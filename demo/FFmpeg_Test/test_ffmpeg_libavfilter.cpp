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
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>
#include "common.hpp"

/////////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/132389734
namespace {

int open_input_file(const char* filename, AVFormatContext** fmt_ctx, AVCodecContext** dec_ctx, int& video_stream_index)
{
    auto ret = avformat_open_input(fmt_ctx, filename, nullptr, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        print_error_string(ret);
        return ret;
    }

    if ((ret = avformat_find_stream_info(*fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        print_error_string(ret);
        return ret;
    }

    // select the video stream
    AVCodec* dec = nullptr;
    ret = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        print_error_string(ret);
        return ret;
    }
    video_stream_index = ret;

    // create decoding context
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx) {
        ret = AVERROR(ENOMEM);
        print_error_string(ret);
        return ret;
    }

    ret = avcodec_parameters_to_context(*dec_ctx, (*fmt_ctx)->streams[video_stream_index]->codecpar);
    if (ret < 0) {
        print_error_string(ret);
        return ret;
    }

    // init the video decoder
    if ((ret = avcodec_open2(*dec_ctx, dec, nullptr)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    return 0;
}

int init_filters(const char* filters_descr, const AVCodecContext* dec_ctx, const AVRational& time_base, AVFilterGraph** filter_graph, AVFilterContext** buffersink_ctx, AVFilterContext** buffersrc_ctx)
{
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    *filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph || !buffersrc || !buffersink) {
        return AVERROR(ENOMEM);
    }

    // buffer video source: the decoded frames from the decoder will be inserted here.
    char args[512];
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
        time_base.num, time_base.den,
        dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);
    auto ret = avfilter_graph_create_filter(buffersrc_ctx, buffersrc, "in",
        args, nullptr, *filter_graph);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer source\n");
        print_error_string(ret);
        return ret;
    }

    // buffer video sink: to terminate the filter chain.
    ret = avfilter_graph_create_filter(buffersink_ctx, buffersink, "out",
        nullptr, nullptr, *filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        print_error_string(ret);
        return ret;
    }

    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(*buffersink_ctx, "pix_fmts", pix_fmts,
        AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        print_error_string(ret);
        return ret;
    }

    //Set the endpoints for the filter graph. The filter_graph will be linked to the graph described by filters_descr.

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

    if ((ret = avfilter_graph_parse_ptr(*filter_graph, filters_descr,
        &inputs, &outputs, nullptr)) < 0) {
        print_error_string(ret);
        return ret;
    }

    if ((ret = avfilter_graph_config(*filter_graph, nullptr)) < 0) {
        print_error_string(ret);
        return ret;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;
}

} // namespace

int test_ffmpeg_libavfilter_movie(const char* filename)
{
    AVFormatContext* fmt_ctx = nullptr;
    AVCodecContext* dec_ctx = nullptr;
    int video_stream_index = -1;
    auto ret = open_input_file(filename, &fmt_ctx, &dec_ctx, video_stream_index);
    if (ret < 0 || !fmt_ctx || !dec_ctx) {
        fprintf(stderr, "fail to open_input_file: %d\n", ret);
        return -1;
    }

    // ffmpeg.exe -i abc.mp4 -i 1.jpg -filter_complex "overlay=x=10:y=20" out.mp4
    const char* filter_descr = "movie=1.jpg[logo];[in][logo]overlay=10:20[out]";
    AVFilterGraph* filter_graph = nullptr;
    AVFilterContext* buffersink_ctx = nullptr;
    AVFilterContext* buffersrc_ctx = nullptr;
    AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
    ret = init_filters(filter_descr, dec_ctx, time_base, &filter_graph, &buffersink_ctx, &buffersrc_ctx);
    if (ret < 0) {
        fprintf(stderr, "fail to init_filters: %d\n", ret);
        return -1;
    }

    AVFrame* frame = av_frame_alloc();
    AVFrame* filt_frame = av_frame_alloc();
    if (!frame || !filt_frame) {
        fprintf(stderr, "fail to av_frame_alloc\n");
        return -1;
    }

    const int width_src = dec_ctx->width, height_src = dec_ctx->height;
    const int width_dst = dec_ctx->width / 2, height_dst = dec_ctx->height / 2;
    SwsContext* sws_ctx = sws_getContext(width_src, height_src, AV_PIX_FMT_YUV420P,
        width_dst, height_dst, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        fprintf(stderr, "fail to sws_getContext\n");
        return -1;
    }
    const int stride_src[4] = { width_src, width_src / 2, width_src / 2, 0 };
    const int stride_dst[4] = { width_dst * 3, 0, 0, 0 };

    AVPacket packet;
    bool exit = false;
    cv::Mat mat(height_dst, width_dst, CV_8UC3);
    const char* winname = "show video";
    cv::namedWindow(winname);

    // read all packets
    while (1) {
        if (exit) break;

        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
            break;

        if (packet.stream_index == video_stream_index) {
            ret = avcodec_send_packet(dec_ctx, &packet);
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                else if (ret < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
                    print_error_string(ret);
                    return -1;
                }

                frame->pts = frame->best_effort_timestamp;

                // push the decoded frame into the filtergraph
                if ((ret = av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                    print_error_string(ret);
                    break;
                }

                // pull filtered frames from the filtergraph
                while (1) {
                    ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0) {
                        fprintf(stderr, "fail to av_buffersink_get_frame: %d\n", ret);
                        return -1;
                    }

                    sws_scale(sws_ctx, filt_frame->data, stride_src, 0, height_src, &mat.data, stride_dst);
                    cv::imshow(winname, mat);

                    av_frame_unref(filt_frame);

                    int key = cv::waitKey(30);
                    if (key == 27) {
                        exit = true;
                        break;
                    }

                    av_frame_unref(filt_frame);
                }
                av_frame_unref(frame);
            }
        }
        av_packet_unref(&packet);
    }

    avfilter_graph_free(&filter_graph);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    sws_freeContext(sws_ctx);

    //if (ret < 0 && ret != AVERROR_EOF) {
    //    print_error_string(ret);
    //    return ret;
    //}

    return 0;
}

