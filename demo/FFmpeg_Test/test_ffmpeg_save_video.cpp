#include "funset.hpp"
#include <memory>
#include <thread>
#include <chrono>
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
// Blog: https://blog.csdn.net/fengbingchun/article/details/132129988
namespace {

bool packet_encode_flag = true;
const size_t block_size = 640 * 480 * 1.5;
const int height = 480, width = 640;
const char* video_size = "640x480";
const char* pixel_format = "yuv420p";
const double frame_rate = 30;
const int queue_size = 30;
const char* filename = "../../../test_images/1.mp4";
int video_stream_index = -1;
const int slice_size = 30;
const char* path = "../../../test_images/";
const int total_frames = 125;

void set_packet(PacketScaleQueue& packet_encode)
{
    while (packet_encode_flag) {
        Buffer buffer;
        packet_encode.popPacket(buffer);
        static int i = 0;
    
        // refrence: ffmpeg/doc/examples/encode_video.c
        // prepare a dummy image: Y
        unsigned char* p1 = buffer.data;
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                p1[y * width + x] = x + y + i * 3;
            }
        }

        // Cb and Cr
        unsigned char* p2 = buffer.data + width * height;
        unsigned char* p3 = buffer.data + width * height + width * height / 4;
        for (auto y = 0; y < height / 2; ++y) {
            for (auto x = 0; x < width / 2; ++x) {
                p2[y * width / 2 + x] = 128 + y + i * 2;
                p3[y * width / 2 + x] = 64 + x + i * 5;
            }
        }

        packet_encode.pushScale(buffer);

        if (++i > 25) i = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
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

AVCodecContext* get_encode_context(AVFormatContext* ifmt_ctx)
{
    AVCodec* encodec = avcodec_find_encoder_by_name("mpeg1video"); // ffmpeg.exe -encoders
    if (!encodec) {
        fprintf(stderr, "fail to avcodec_find_encoder_by_name\n");
        return nullptr;
    }

    AVCodecContext* enc_ctx = avcodec_alloc_context3(encodec);
    if (!enc_ctx) {
        fprintf(stderr, "fail to avcodec_alloc_context3\n");
        return nullptr;
    }

    enc_ctx->bit_rate = 400000;
    enc_ctx->framerate = ifmt_ctx->streams[video_stream_index]->r_frame_rate;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->height = ifmt_ctx->streams[video_stream_index]->codecpar->height;
    enc_ctx->width = ifmt_ctx->streams[video_stream_index]->codecpar->width;
    enc_ctx->time_base = av_inv_q(av_d2q(frame_rate, 4096));
    enc_ctx->max_b_frames = 1;

    //if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) // true
    enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    auto ret = avcodec_open2(enc_ctx, encodec, nullptr);
    if (ret != 0) {
        fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    return enc_ctx;
}

AVFormatContext* get_output_format_context(const AVCodecContext* enc_ctx, const char* filename)
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

    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if (ret < 0) {
        fprintf(stderr, "fail to avcodec_parameters_from_context: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    out_stream->time_base = enc_ctx->time_base;

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) { // true
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "fail to avio_open: %d\n", ret);
            print_error_string(ret);
            return nullptr;
        }
    }

    ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "fail to avformat_write_header: %d\n", ret);
        print_error_string(ret);
        return nullptr;
    }

    av_dump_format(ofmt_ctx, 0, filename, 1);
    return ofmt_ctx;
}

int decode(AVPacket* packet, AVFormatContext* ifmt_ctx, AVCodecContext* dec_ctx, AVFrame* frame)
{
    if (packet)
        av_packet_rescale_ts(packet, ifmt_ctx->streams[video_stream_index]->time_base, dec_ctx->time_base);

    auto ret = avcodec_send_packet(dec_ctx, packet);
    if (ret < 0) {
        fprintf(stderr, "fail to avcodec_send_packet: %d\n", ret);
        print_error_string(ret);
        return -1;
    }

    while (1) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) { // AVERROR(EAGAIN): decode is not yet complete
            //fprintf(stderr, "Warning: avcodec_receive_frame: %d\n", ret);
            //print_error_string(ret);
            break;
        }
        else if (ret < 0) {
            fprintf(stderr, "fail to avcodec_receive_frame: %d\n", ret);
            print_error_string(ret);
            return ret;
        }

        frame->pts = frame->best_effort_timestamp;
        break;
    }

    if (packet)
        av_packet_unref(packet);

    return 0;
}

int encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* packet, AVFormatContext* ifmt_ctx, AVFormatContext* ofmt_ctx)
{
    auto ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            fprintf(stderr, "Warning: avcodec_send_frame: %d\n", ret);
            print_error_string(ret);
            ret = 0;
        }
        else {
            fprintf(stderr, "fail to avcodec_send_frame: %d\n", ret);
            print_error_string(ret);
            return ret;
        }
    }

    while (1) {
        ret = avcodec_receive_packet(enc_ctx, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) { // AVERROR(EAGAIN): encode is not yet complete
            //fprintf(stderr, "Warning: avcodec_receive_packet: %d\n", ret);
            //print_error_string(ret);
            break;
        }

        if (ret < 0) {
            fprintf(stderr, "fail to avcodec_receive_packet: %d\n", ret);
            print_error_string(ret);
            return ret;
        }

        packet->stream_index = 0;
        av_packet_rescale_ts(packet, ifmt_ctx->streams[video_stream_index]->time_base, ofmt_ctx->streams[0]->time_base);
        //packet2->pts = packet2->dts = frame->pts *
        //    ofmt_ctx->streams[0]->time_base.den / ofmt_ctx->streams[0]->time_base.num / 
        //    (enc_ctx->framerate.num / enc_ctx->framerate.den);

        ret = av_interleaved_write_frame(ofmt_ctx, packet);
        if (ret < 0) {
            print_error_string(ret);
            return ret;
        }

        av_packet_unref(packet);
    }

    return 0;
}

} // namespace

int test_ffmpeg_save_video_slice()
{
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

    AVPacket *packet = av_packet_alloc(), *packet2 = av_packet_alloc();
    if (!packet || !packet2) {
        fprintf(stderr, "fail to av_packet_alloc\n");
        return -1;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "fail to av_frame_alloc\n");
        return -1;
    }

    int count = 0, name = 1;
    AVCodecContext* enc_ctx = nullptr;
    AVFormatContext* ofmt_ctx = nullptr;
    AVCodecContext* dec_ctx = nullptr;

    while (1) {
        auto ret = av_read_frame(ifmt_ctx, packet);
        if (ret < 0) {
            break;
        }

        if (packet->stream_index != video_stream_index) {
            av_packet_unref(packet);
            continue;
        }

        if (count % slice_size == 0) {
            dec_ctx = get_decode_context(ifmt_ctx);
            if (!dec_ctx) {
                fprintf(stderr, "fail to get_decode_context\n");
                return -1;
            }

            enc_ctx = get_encode_context(ifmt_ctx);
            if (!enc_ctx) {
                fprintf(stderr, "fail to avcodec_alloc_context3\n");
                return -1;
            }

            std::string filename = std::to_string(name) + ".mp4";
            filename = std::string(path) + filename;
            ofmt_ctx = get_output_format_context(enc_ctx, filename.c_str());
            if (!ofmt_ctx) {
                fprintf(stderr, "fail to get_output_format_context\n");
                return -1;
            }

            //dec_ctx = get_decode_context(ifmt_ctx);
            //if (!dec_ctx) {
            //    fprintf(stderr, "fail to get_decode_context\n");
            //    return -1;
            //}

            ++name;
        }

        if (decode(packet, ifmt_ctx, dec_ctx, frame) != 0) return -1;

        if (encode(enc_ctx, frame, packet2, ifmt_ctx, ofmt_ctx) != 0) return -1;

        //fprintf(stdout, "count: %d\n", count);
        if (count + 1 == total_frames) { // terminate loop
            packet_encode_flag = false;

            // flush the decoder
            decode(nullptr, ifmt_ctx, dec_ctx, frame);
            if (frame->data[0])
                encode(enc_ctx, frame, packet2, ifmt_ctx, ofmt_ctx);

            // flush the encoder
            encode(enc_ctx, nullptr, packet2, ifmt_ctx, ofmt_ctx);

            av_write_trailer(ofmt_ctx);
            break;
        }

        ++count;

        if (count > 1 && count % slice_size == 0) {
            // flush the decoder
            decode(nullptr, ifmt_ctx, dec_ctx, frame);
            if (frame->data[0])
                encode(enc_ctx, frame, packet2, ifmt_ctx, ofmt_ctx);

            // flush the encoder
            encode(enc_ctx, nullptr, packet2, ifmt_ctx, ofmt_ctx);

            av_write_trailer(ofmt_ctx);

            avcodec_free_context(&dec_ctx);
            avcodec_free_context(&enc_ctx);
            avio_closep(&ofmt_ctx->pb);
            avformat_free_context(ofmt_ctx);
        }
    }

    av_packet_free(&packet);
    av_packet_free(&packet2);
    av_frame_free(&frame);

    avcodec_free_context(&dec_ctx);
    avcodec_free_context(&enc_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_close_input(&ofmt_ctx);
    avformat_close_input(&ifmt_ctx);

    // note: the internal buffer could have changed, and be != avio_ctx_buffer
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }

    thread_packet.join();
    fprintf(stdout, "test finish\n");

    return 0;
}

int test_ffmpeg_save_video()
{
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

    AVCodecContext* enc_ctx = get_encode_context(ifmt_ctx);
    if (!enc_ctx) {
        fprintf(stderr, "fail to avcodec_alloc_context3\n");
        return -1;
    }

    AVFormatContext* ofmt_ctx = get_output_format_context(enc_ctx, filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "fail to get_output_format_context\n");
        return -1;
    }

    AVPacket *packet = av_packet_alloc(), *packet2 = av_packet_alloc();
    if (!packet || !packet2) {
        fprintf(stderr, "fail to av_packet_alloc\n");
        return -1;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "fail to av_frame_alloc\n");
        return -1;
    }

    while (1) {
        auto ret = av_read_frame(ifmt_ctx, packet);
        if (ret < 0) {
            break;
        }

        if (packet->stream_index != video_stream_index) {
            av_packet_unref(packet);
            continue;
        }

        if (decode(packet, ifmt_ctx, dec_ctx, frame) != 0) return -1;

        if (encode(enc_ctx, frame, packet2, ifmt_ctx, ofmt_ctx) != 0) return -1;

        fprintf(stdout, "frame: %d\n", frame->pts);
        if (frame->pts == 125) {
            packet_encode_flag = false;

            // flush the decoder
            decode(nullptr, ifmt_ctx, dec_ctx, frame);
            if (frame->data[0])
                encode(enc_ctx, frame, packet2, ifmt_ctx, ofmt_ctx);

            // flush the encoder
            encode(enc_ctx, nullptr, packet2, ifmt_ctx, ofmt_ctx);

            av_write_trailer(ofmt_ctx);
            break;
        }
    }

    av_packet_free(&packet);
    av_packet_free(&packet2);
    av_frame_free(&frame);

    avcodec_free_context(&dec_ctx);
    avcodec_free_context(&enc_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_close_input(&ofmt_ctx);
    avformat_close_input(&ifmt_ctx);

    // note: the internal buffer could have changed, and be != avio_ctx_buffer
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }

    thread_packet.join();
    fprintf(stdout, "test finish\n");

    return 0;
}
