#include "funset.hpp"
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif
#include <opencv2/opencv.hpp>
#include "common.hpp"

/////////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/132128885
namespace {

bool packet_encode_flag = true;
const size_t block_size = 640 * 480 * 3;
const int height = 480, width = 640;

void set_packet(PacketScaleQueue& packet_encode)
{
    while (packet_encode_flag) {
        static unsigned char v1 = 0, v2 = 0, v3 = 255;
        static const size_t size = height * width;

        Buffer buffer;
        packet_encode.popPacket(buffer);
        memset(buffer.data, v1, size);
        memset(buffer.data + size, v2, size);
        memset(buffer.data + size * 2, v3, size);
        packet_encode.pushScale(buffer);

        ++v1;
        ++v2;
        --v3;
        if (v1 == 255) v1 = 0;
        if (v2 == 255) v2 = 0;
        if (v3 == 0) v3 = 255;

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

} // namespace

int test_ffmpeg_avio_show()
{
    PacketScaleQueue packet_encode;
    packet_encode.init(30, block_size);

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

    AVFormatContext* ifmt_ctx = avformat_alloc_context();
    if (!ifmt_ctx) {
        print_error_string(AVERROR(ENOMEM));
        return -1;
    }
    ifmt_ctx->pb = avio_ctx;

    AVDictionary* dict = nullptr;
    av_dict_set(&dict, "video_size", "640x480", 0);
    av_dict_set(&dict, "pixel_format", "bgr24", 0);

    auto ret = avformat_open_input(&ifmt_ctx, nullptr, av_find_input_format("rawvideo"), &dict);
    if (ret < 0) {
        fprintf(stderr, "Could not open input\n");
        print_error_string(ret);
        return ret;
    }

    ret = avformat_find_stream_info(ifmt_ctx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        print_error_string(ret);
        return ret;
    }

    av_dump_format(ifmt_ctx, 0, "nothing", 0);

    int video_stream_index = -1;
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
        return -1;
    }

    AVCodecParameters* codecpar = ifmt_ctx->streams[video_stream_index]->codecpar;
    if (codecpar->codec_id != AV_CODEC_ID_RAWVIDEO) {
        fprintf(stderr, "error: this test code only support rawvideo encode: %d\n", codecpar->codec_id);
        return -1;
    }

    AVPacket* packet = static_cast<AVPacket*>(av_malloc(sizeof(AVPacket)));
    if (!packet) {
        fprintf(stderr, "fail to av_malloc\n");
        return -1;
    }

    cv::Mat mat(height, width, CV_8UC3);
    const char* winname = "show video";
    cv::namedWindow(winname);

    while (1) {
        ret = av_read_frame(ifmt_ctx, packet);
        if (ret >= 0 && packet->stream_index == video_stream_index && packet->size > 0) {
            mat.data = packet->data;
            cv::imshow(winname, mat);

            av_packet_unref(packet);

            int key = cv::waitKey(30);
            if (key == 27) {
                packet_encode_flag = false;
                break;
            }
        }
    }

    av_freep(packet);
    cv::destroyWindow(winname);
   
    avformat_close_input(&ifmt_ctx);
    // note: the internal buffer could have changed, and be != avio_ctx_buffer
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    //avio_context_free(&avio_ctx); ==> av_freep(&avio_ctx);

    av_dict_free(&dict);
    thread_packet.join();

    fprintf(stdout, "test finish\n");
    return 0;
}

/////////////////////////////////////////////////////////////////
namespace {

void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* outfile)
{
    int ret;

    // send the frame to the encoder
    if (frame)
        printf("Send frame %3lld\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write packet %3lld (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}

} // namespace

int test_ffmpeg_encode()
{
    // reference: ffmpeg/doc/examples/encode_video.c
    const char* filename, * codec_name;
    const AVCodec* codec;
    AVCodecContext* c = NULL;
    int i, ret, x, y;
    FILE* f;
    AVFrame* frame;
    AVPacket* pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    filename = "../../../test_images/encode.mp4";
    codec_name = "mpeg1video";

    // find the mpeg1video encoder
    codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        return -1;
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }

    pkt = av_packet_alloc();
    if (!pkt)
        return -1;

    // put sample parameters
    c->bit_rate = 400000;
    // resolution must be a multiple of two
    c->width = 640;
    c->height = 480;
    // frames per second
    c->time_base = { 1, 25 };
    c->framerate = { 25, 1 };

    // emit one intra frame every ten frames
    // check frame pict_type before passing frame
    // to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
    // then gop_size is ignored and the output of encoder
    // will always be I frame irrespective to gop_size
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

    // open it
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        print_error_string(ret);
        return -1;
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        return -1;
    }

    // encode 1 second of video
    for (int j = 0; j < 20; ++j) {
        for (i = 0; i < 25; i++) {
            fflush(stdout);

            // make sure the frame data is writable
            ret = av_frame_make_writable(frame);
            if (ret < 0)
                return -1;

            // prepare a dummy image
            // Y
            for (y = 0; y < c->height; y++) {
                for (x = 0; x < c->width; x++) {
                    frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
                }
            }

            // Cb and Cr
            for (y = 0; y < c->height / 2; y++) {
                for (x = 0; x < c->width / 2; x++) {
                    frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                    frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
                }
            }

            frame->pts = j * 25 + i;

            // encode the image
            encode(c, frame, pkt, f);
        }
    }

    // flush the encoder
    encode(c, NULL, pkt, f);

    // add sequence end code to have a real MPEG file
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);

    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}

/////////////////////////////////////////////////////////////////
namespace {

void encode2(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, AVFormatContext* ofmt_ctx)
{
    // send the frame to the encoder
    auto ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        print_error_string(ret);
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            print_error_string(ret);
            exit(1);
        }

        //printf("Write packet %3lld (size=%5d)\n", pkt->pts, pkt->size);

        ret = av_interleaved_write_frame(ofmt_ctx, pkt);
        if (ret < 0) {
            print_error_string(ret);
            exit(1);
        }

        av_packet_unref(pkt);
    }
}

AVFormatContext* generate_video_file(const char* filename, const AVCodecContext* c)
{
    AVFormatContext* ofmt_ctx = nullptr;
    auto ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename);
    if (ret < 0 || !ofmt_ctx) {
        fprintf(stderr, "fail to avformat_alloc_output_context2: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
    if (!out_stream) {
        fprintf(stderr, "fail to avformat_new_stream\n");
        return nullptr;
    }

    ret = avcodec_parameters_from_context(out_stream->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "fail to avcodec_parameters_from_context: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    av_dump_format(ofmt_ctx, 0, filename, 1);

    if (!(ofmt_ctx->flags & AVFMT_NOFILE)) { // true
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "fail to avio_open: %d\n", ret);
            print_error_string(ret);
            return nullptr;
        }
    }

    AVDictionary* opt = nullptr;
    av_dict_set_int(&opt, "video_track_timescale", 25, 0);
    ret = avformat_write_header(ofmt_ctx, &opt);
    if (ret < 0) {
        fprintf(stderr, "fail to avformat_write_header: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }
    av_dict_free(&opt);

    return ofmt_ctx;
}

AVCodecContext* get_codec_context(const AVCodec* codec)
{
    // get the number of frames in video: ffprobe.exe -v error -select_streams v:0 -show_entries stream=nb_frames -of default=nokey=1:noprint_wrappers=1 1.mp4
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return nullptr;
    }

    // put sample parameters
    codec_ctx->bit_rate = 400000;
    // resolution must be a multiple of two
    codec_ctx->width = 640;
    codec_ctx->height = 480;
    // frames per second
    codec_ctx->time_base = { 1, 25 };
    codec_ctx->framerate = { 25, 1 };

    // emit one intra frame every ten frames
    // check frame pict_type before passing frame
    // to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
    // then gop_size is ignored and the output of encoder
    // will always be I frame irrespective to gop_size
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);

    // open it
    auto ret = avcodec_open2(codec_ctx, codec, NULL);
    if (ret < 0) {
        print_error_string(ret);
        return nullptr;
    }

    return codec_ctx;
}

} // namespace

int test_ffmpeg_encode_slice()
{
    // reference: ffmpeg/doc/examples/encode_video.c
    const char* path = "../../../test_images/";
    const char* codec_name = "mpeg1video";

    // find the mpeg1video encoder
    const AVCodec* codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        return -1;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }

    AVFormatContext* ofmt_ctx = nullptr;
    AVCodecContext* codec_ctx = nullptr;
    int count = 0;
    int n = 1;

    AVPacket* pkt = av_packet_alloc();
    if (!pkt)
        return -1;

    // encode 1*20 second of video
    for (auto j = 0; j < 20; ++j) {
        for (auto i = 0; i < 25; i++) {
            if (count % 30 == 0) {
                codec_ctx = get_codec_context(codec);
                if (!codec_ctx) {
                    fprintf(stderr, "fail to get codec context\n");
                    return -1;
                }

                std::string name = std::to_string(n) + ".mp4";
                name = std::string(path) + name;
                ofmt_ctx = generate_video_file(name.c_str(), codec_ctx);
                if (!ofmt_ctx) {
                    fprintf(stderr, "fail to generate video file\n");
                    return -1;
                }

                if (n == 1) {
                    frame->format = codec_ctx->pix_fmt;
                    frame->width = codec_ctx->width;
                    frame->height = codec_ctx->height;

                    auto ret = av_frame_get_buffer(frame, 32);
                    if (ret < 0) {
                        fprintf(stderr, "Could not allocate the video frame data\n");
                        return -1;
                    }
                }

                ++n;
            }

            fflush(stdout);

            // make sure the frame data is writable
            auto ret = av_frame_make_writable(frame);
            if (ret < 0)
                return -1;

            // prepare a dummy image
            // Y
            for (auto y = 0; y < codec_ctx->height; y++) {
                for (auto x = 0; x < codec_ctx->width; x++) {
                    frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
                }
            }

            // Cb and Cr
            for (auto y = 0; y < codec_ctx->height / 2; y++) {
                for (auto x = 0; x < codec_ctx->width / 2; x++) {
                    frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                    frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
                }
            }

            frame->pts = j * 25 + i;

            // encode the image
            encode2(codec_ctx, frame, pkt, ofmt_ctx);
            ++count;

            if (count > 1 && count % 30 == 0) {
                encode2(codec_ctx, nullptr, pkt, ofmt_ctx);
                av_write_trailer(ofmt_ctx);
                avio_closep(&ofmt_ctx->pb);
                avformat_free_context(ofmt_ctx);
                avcodec_free_context(&codec_ctx);
            }
        }
    }

    // flush the encoder
    encode2(codec_ctx, nullptr, pkt, ofmt_ctx);
    av_write_trailer(ofmt_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    avcodec_free_context(&codec_ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    fprintf(stdout, "test finish\n");
    return 0;
}
