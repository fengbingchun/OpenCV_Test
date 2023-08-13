#include "funset.hpp"
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>
#include "common.hpp"

namespace {

bool packet_encode_flag = true;
const size_t block_size = 640 * 480 * 3;
const int height = 480, width = 640;
int video_stream_index = -1;
const double frame_rate = 30;
const int queue_size = 30;
const char* video_size = "640x480";
const char* pixel_format = "bgr24";

void set_packet(PacketScaleQueue& packet_encode)
{
    while (packet_encode_flag) {
        static unsigned char value = 0;

        Buffer buffer;
        packet_encode.popPacket(buffer);
        memset(buffer.data, value, block_size);
        packet_encode.pushScale(buffer);

        value += 5;
        if (value >= 255) value = 0;

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

int read_packet(void* opaque, uint8_t* buf, int buf_size)
{
    PacketScaleQueue* packet_encode = static_cast<PacketScaleQueue*>(opaque);
    Buffer buffer;
    packet_encode->popScale(buffer);
    memcpy(buf, buffer.data, buf_size);
    packet_encode->pushPacket(buffer);

    return buf_size;
}

AVFormatContext* get_input_format_context(AVIOContext* avio_ctx)
{
    AVFormatContext* ifmt_ctx = avformat_alloc_context();
    if (!ifmt_ctx) {
        print_error_string(AVERROR(ENOMEM));
        return nullptr;
    }
    ifmt_ctx->pb = avio_ctx;

    AVDictionary* dict = nullptr;
    av_dict_set(&dict, "video_size", video_size, 0);
    av_dict_set(&dict, "pixel_format", pixel_format, 0);

    auto ret = avformat_open_input(&ifmt_ctx, nullptr, av_find_input_format("rawvideo"), &dict);
    if (ret < 0) {
        fprintf(stderr, "Could not open input\n");
        print_error_string(ret);
        return nullptr;
    }
    av_dict_free(&dict);

    ret = avformat_find_stream_info(ifmt_ctx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        print_error_string(ret);
        return nullptr;
    }

    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; ++i) {
        const AVStream* stream = ifmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
                stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
        }
    }

    if (video_stream_index == -1) {
        fprintf(stderr, "error: no video stream\n");
        return nullptr;
    }

    if (ifmt_ctx->streams[video_stream_index]->codecpar->codec_id != AV_CODEC_ID_RAWVIDEO) {
        fprintf(stderr, "error: this test code only support rawvideo encode: %d\n", ifmt_ctx->streams[video_stream_index]->codecpar->codec_id);
        return nullptr;
    }

    av_dump_format(ifmt_ctx, 0, "nothing", 0);
    return ifmt_ctx;
}

AVCodecContext* get_decode_context(AVFormatContext* ifmt_ctx)
{
    AVCodec* decoder = nullptr;
    auto ret = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (ret < 0) {
        fprintf(stderr, "fail to av_find_best_stream: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
    if (!dec_ctx) {
        fprintf(stderr, "fail to avcodec_alloc_context3\n");
        return nullptr;
    }

    ret = avcodec_parameters_to_context(dec_ctx, ifmt_ctx->streams[video_stream_index]->codecpar);
    if (ret < 0) {
        fprintf(stderr, "fail to avcodec_parameters_to_context: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    ifmt_ctx->streams[video_stream_index]->r_frame_rate = av_d2q(frame_rate, 4096);
    dec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, ifmt_ctx->streams[video_stream_index], nullptr);
    ret = avcodec_open2(dec_ctx, decoder, nullptr);
    if (ret != 0) {
        fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    return dec_ctx;
}

int init_filters(const char* filters_descr, AVFormatContext* fmt_ctx, AVCodecContext* dec_ctx, AVFilterGraph** filter_graph, AVFilterContext** buffersrc_ctx, AVFilterContext** buffersink_ctx)
{
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");

    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    *filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        print_error_string(AVERROR(ENOMEM));
        return -1;
    }

    // buffer video source: the decoded frames from the decoder will be inserted here.
    AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
    char args[512];
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
        time_base.num, time_base.den,
        dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);
    auto ret = avfilter_graph_create_filter(buffersrc_ctx, buffersrc, "in", args, nullptr, *filter_graph);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer source\n");
        return -1;
    }

    // buffer video sink: to terminate the filter chain.
    ret = avfilter_graph_create_filter(buffersink_ctx, buffersink, "out", nullptr, nullptr, *filter_graph);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer sink\n");
        return -1;
    }

    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_BGR24, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(*buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot set output pixel format\n");
        return -1;
    }

    // Set the endpoints for the filter graph. The filter_graph will be linked to the graph described by filters_descr.

    // The buffer source output must be connected to the input pad of the first filter described by filters_descr; 
    // since the first filter input label is not specified, it is set to "in" by default.
    outputs->name = av_strdup("in");
    outputs->filter_ctx = *buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    // The buffer sink input must be connected to the output pad of the last filter described by filters_descr; 
    // since the last filter output label is not specified, it is set to "out" by default.
    inputs->name = av_strdup("out");
    inputs->filter_ctx = *buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    if ((ret = avfilter_graph_parse_ptr(*filter_graph, filters_descr, &inputs, &outputs, nullptr)) < 0)
        return -1;

    if ((ret = avfilter_graph_config(*filter_graph, nullptr)) < 0)
        return -1;

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return 0;
}

} // namespace

int test_ffmpeg_libavfilter()
{
	// reference: doc/examples/filtering_video.c
    PacketScaleQueue packet_encode;
    packet_encode.init(queue_size, block_size);

    std::thread thread_packet(set_packet, std::ref(packet_encode));

    uint8_t* avio_ctx_buffer = static_cast<uint8_t*>(av_malloc(block_size));
    if (!avio_ctx_buffer) {
        print_error_string(AVERROR(ENOMEM));
        return -1;
    }

    AVIOContext* avio_ctx = avio_alloc_context(avio_ctx_buffer, block_size, 0, &packet_encode, &read_packet, nullptr, nullptr);
    if (!avio_ctx) {
        print_error_string(AVERROR(ENOMEM));
        return -1;
    }

    AVFormatContext* ifmt_ctx = get_input_format_context(avio_ctx);
    if (!ifmt_ctx) {
        fprintf(stderr, "fail to get_input_format_context\n");
        return -1;
    }

    AVCodecContext* dec_ctx = get_decode_context(ifmt_ctx);
    if (!dec_ctx) {
        fprintf(stderr, "fail to get_decode_context\n");
        return -1;
    }

    // ffmpeg.exe -i abc.mp4 -i lena.png -filter_complex "overlay=x=10:y=10" out.mp4
    const char* filter_descr = "movie=../../../test_images/lena.png[logo];[in][logo]overlay=20:10[out]"; // error ? build ffmpeg miss option
    //const char* filter_descr = "scale=w=320:h=200,transpose=cclock";
    AVFilterContext* buffersink_ctx = nullptr;
    AVFilterContext* buffersrc_ctx = nullptr;
    AVFilterGraph* filter_graph = nullptr;
    auto ret = init_filters(filter_descr, ifmt_ctx, dec_ctx, &filter_graph, &buffersink_ctx, &buffersink_ctx);
    if (ret != 0 || !filter_graph || !buffersrc_ctx || !buffersink_ctx) {
        fprintf(stderr, "fail to init_filters: %d\n", ret);
        return -1;
    }
	
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    AVFrame* filt_frame = av_frame_alloc();
    if (!frame || !filt_frame) {
        fprintf(stderr, "fail to av_frame_alloc\n");
        return -1;
    }

    cv::Mat mat(height, width, CV_8UC3);
    const char* winname = "show video";
    cv::namedWindow(winname);

    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;

        if (packet.stream_index == video_stream_index) {
            ret = avcodec_send_packet(dec_ctx, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                else if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
                    return -1;
                }

                frame->pts = frame->best_effort_timestamp;

                // push the decoded frame into the filtergraph
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                    return -1;
                }

                // pull filtered frames from the filtergraph
                while (1) {
                    ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0)
                        return -1;

                    //display_frame(filt_frame, buffersink_ctx->inputs[0]->time_base);
                    mat.data = filt_frame->data[0];
                    cv::imshow(winname, mat);

                    av_frame_unref(filt_frame);

                    int key = cv::waitKey(30);
                    if (key == 27) {
                        packet_encode_flag = false;
                        break;
                    }
                }
                av_frame_unref(frame);
            }
        }
        av_packet_unref(&packet);
    }

	return 0;
}

namespace {

const char* filter_descr = "scale=78:24,transpose=cclock";
/* other way:
    scal/////////////////////////////////////////////////////////////////
e=78:24 [scl]; [scl] transpose=cclock // assumes "[in]" and "[out]" to be input output pads respectively
*/

static AVFormatContext* fmt_ctx;
static AVCodecContext* dec_ctx;
AVFilterContext* buffersink_ctx;
AVFilterContext* buffersrc_ctx;
AVFilterGraph* filter_graph;
//static int video_stream_index = -1;
static int64_t last_pts = AV_NOPTS_VALUE;

static int open_input_file(const char* filename)
{
    int ret;
    AVCodec* dec;

    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    /* select the video stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    video_stream_index = ret;

    /* create decoding context */
    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);

    /* init the video decoder */
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    return 0;
}

static int init_filters(const char* filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
        time_base.num, time_base.den,
        dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
        args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
        NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
        AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
        * Set the endpoints for the filter graph. The filter_graph will
        * be linked to the graph described by filters_descr.
        */

        /*
        * The buffer source output must be connected to the input pad of
        * the first filter described by filters_descr; since the first
        * filter input label is not specified, it is set to "in" by
        * default.
        */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
        * The buffer sink input must be connected to the output pad of
        * the last filter described by filters_descr; since the last
        * filter output label is not specified, it is set to "out" by
        * default.
        */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

static void display_frame(const AVFrame* frame, AVRational time_base)
{
    int x, y;
    uint8_t* p0, * p;
    int64_t delay;

    if (frame->pts != AV_NOPTS_VALUE) {
        if (last_pts != AV_NOPTS_VALUE) {
            /* sleep roughly the right amount of time;
                * usleep is in microseconds, just like AV_TIME_BASE. */
            delay = av_rescale_q(frame->pts - last_pts,
                time_base, { 1, AV_TIME_BASE });
            if (delay > 0 && delay < 1000000)
                std::this_thread::sleep_for(std::chrono::microseconds(delay));
        }
        last_pts = frame->pts;
    }

    /* Trivial ASCII grayscale display. */
    p0 = frame->data[0];
    puts("\033c");
    for (y = 0; y < frame->height; y++) {
        p = p0;
        for (x = 0; x < frame->width; x++)
            putchar(" .-+#"[*(p++) / 52]);
        putchar('\n');
        p0 += frame->linesize[0];
    }
    fflush(stdout);
}

} // namespace

int test_ffmpeg_libavfilter_scale()
{
    int ret;
    AVPacket packet;
    AVFrame* frame;
    AVFrame* filt_frame;

    frame = av_frame_alloc();
    filt_frame = av_frame_alloc();
    if (!frame || !filt_frame) {
        perror("Could not allocate frame");
        exit(1);
    }

    const char* filename = "../../../test_images/abc.mp4";
    if ((ret = open_input_file(filename) < 0))
        goto end;
    if ((ret = init_filters(filter_descr)) < 0)
        goto end;

    /* read all packets */
    while (1) {
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
            break;

        if (packet.stream_index == video_stream_index) {
            ret = avcodec_send_packet(dec_ctx, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                else if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
                    goto end;
                }

                frame->pts = frame->best_effort_timestamp;

                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                    break;
                }

                /* pull filtered frames from the filtergraph */
                while (1) {
                    ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0)
                        goto end;
                    display_frame(filt_frame, buffersink_ctx->inputs[0]->time_base);
                    av_frame_unref(filt_frame);
                }
                av_frame_unref(frame);
            }
        }
        av_packet_unref(&packet);
    }

end:
    avfilter_graph_free(&filter_graph);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);

    if (ret < 0 && ret != AVERROR_EOF) {
        print_error_string(ret);
        exit(1);
    }

    return 0;
}

