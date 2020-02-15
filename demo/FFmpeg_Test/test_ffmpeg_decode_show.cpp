#include "funset.hpp"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <thread>
#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/mem.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>
#ifdef _MSC_VER
#include <libyuv/convert_argb.h>
#include <libyuv/convert.h>
#endif

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/103583548
#ifdef _MSC_VER
int test_ffmpeg_dshow_mjpeg_encode_libyuv_decode()
{
	avdevice_register_all();

	AVCodecID id = AV_CODEC_ID_MJPEG;
	AVCodec* encoder_id = avcodec_find_encoder(id);
	if (!encoder_id) {
		fprintf(stderr, "codec not found: %d\n", id);
		return -1;
	}

	AVFormatContext* format_context = avformat_alloc_context();
	format_context->video_codec_id = id;

	AVInputFormat* input_format = av_find_input_format("dshow");
	AVDictionary* dict = nullptr;
	if (av_dict_set(&dict, "video_size", "960x540", 0) < 0) fprintf(stderr, "fail to av_dict_set: line: %d\n", __LINE__);
	int ret = avformat_open_input(&format_context, "video=Integrated Webcam", input_format, &dict);
	if (ret != 0) {
		fprintf(stderr, "fail to avformat_open_input: %d\n", ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_context, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (unsigned int i = 0; i < format_context->nb_streams; ++i) {
		const AVStream* stream = format_context->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	AVCodecParameters* codecpar = format_context->streams[video_stream_index]->codecpar;
	if (codecpar->codec_id != id) {
		fprintf(stderr, "this test code only support mjpeg encode: %d\n", codecpar->codec_id);
		return -1;
	}

	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	if (!packet) {
		fprintf(stderr, "fail to alloc\n");
		return -1;
	}

	std::unique_ptr<unsigned char[]> data(new unsigned char[codecpar->width * codecpar->height * 4]);
	cv::Mat mat(codecpar->height,codecpar->width, CV_8UC4, data.get());
	const char* winname = "dshow mjpeg encode and libyuv decode";
	cv::namedWindow(winname);
	AVFrame* frame = av_frame_alloc();

	while (1) {
		ret = av_read_frame(format_context, packet);
		if (ret >= 0 && packet->stream_index == video_stream_index && packet->size > 0) {
			int dst_width, dst_height;
			libyuv::MJPGSize(packet->data, packet->size, &dst_width, &dst_height);
			if (dst_width != codecpar->width || dst_height != codecpar->height) {
				fprintf(stderr, "width or height dismatch: (%d, %d), (%d, %d)\n",
					dst_width, dst_height, codecpar->width, codecpar->height);
				return -1;
			}

			libyuv::MJPGToARGB(packet->data, packet->size, data.get(), dst_width * 4,
				codecpar->width, codecpar->height, dst_width, dst_height);
			cv::imshow(winname, mat);

		} else if (ret < 0 || packet->size <= 0) {
			fprintf(stderr, "fail to av_read_frame: %d, packet size: %d\n", ret, packet->size);
			continue;
		}

		av_packet_unref(packet);

		int key = cv::waitKey(30);
		if (key == 27) break;
	}

	cv::destroyWindow(winname);
	av_freep(packet);
	avformat_close_input(&format_context);
	av_dict_free(&dict);

	fprintf(stdout, "test finish\n");
	return 0;
}
#else
int test_ffmpeg_dshow_mjpeg_encode_libyuv_decode()
{
	fprintf(stderr, "Error: this test code only support windows platform\n");
	return -1;
}
#endif

////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/103444891
#ifdef _MSC_VER
int test_ffmpeg_decode_dshow()
{
	avdevice_register_all();

{ // show dshow device name
	AVFormatContext* format_context = avformat_alloc_context();
	AVDictionary* dict = nullptr;
	av_dict_set(&dict, "list_devices", "true", 0);
	AVInputFormat* input_format = av_find_input_format("dshow");
	avformat_open_input(&format_context, "", input_format, &dict);
	avformat_close_input(&format_context);
	av_dict_free(&dict);
}

{ // show support codec type and support video size
	AVFormatContext* format_context = avformat_alloc_context();
	AVDictionary* dict = nullptr;
	av_dict_set(&dict, "list_options", "true", 0);
	AVInputFormat* input_format = av_find_input_format("dshow");
	avformat_open_input(&format_context, "video=Integrated Webcam", input_format, &dict); // video=video device name
	avformat_close_input(&format_context);
	av_dict_free(&dict);
}

{ // get dshow encode and decode
	AVCodecID id = AV_CODEC_ID_MJPEG;
	AVCodec* encoder_id = avcodec_find_encoder(id);
	AVCodec* decoder_id = avcodec_find_decoder(id);
	if (!encoder_id || !decoder_id) {
		fprintf(stderr, "codec not found: %d\n", id);
		return -1;
	}

	AVFormatContext* format_context = avformat_alloc_context();
	format_context->video_codec_id = id; // 指定编解码格式

	AVInputFormat* input_format = av_find_input_format("dshow");
	AVDictionary* dict = nullptr;
	//if (av_dict_set(&dict, "vcodec"/*"input_format"*/, "mjpeg", 0) < 0) fprintf(stderr, "fail to av_dict_set: line: %d\n", __LINE__); // 通过av_dict_set设置编解码格式好像不起作用
	if (av_dict_set(&dict, "video_size", "320x240", 0) < 0) fprintf(stderr, "fail to av_dict_set: line: %d\n", __LINE__);
	//if (av_dict_set(&dict, "r", "25", 0) < 0) fprintf(stderr, "fail to av_dict_set: line: %d\n", __LINE__); // 通过av_dict_set设置帧率好像不起作用
	int ret = avformat_open_input(&format_context, "video=Integrated Webcam", input_format, &dict);
	if (ret != 0) {
		fprintf(stderr, "fail to avformat_open_input: %d\n", ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_context, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (unsigned int i = 0; i < format_context->nb_streams; ++i) {
		const AVStream* stream = format_context->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	fprintf(stdout, "frame rate: %f\n", av_q2d(format_context->streams[video_stream_index]->r_frame_rate));

	AVCodecParameters* codecpar = format_context->streams[video_stream_index]->codecpar;
	const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "fail to avcodec_find_decoder\n");
		return -1;
	}

	if (codecpar->codec_id != id) {
		fprintf(stderr, "this test code only support mjpeg encode: %d\n", codecpar->codec_id);
		return -1;
	}

	AVCodecContext* codec_context = avcodec_alloc_context3(codec);
	if (!codec_context) {
		fprintf(stderr, "fail to avcodec_alloc_context3\n");
		return -1;
	}

	codec_context->pix_fmt = AVPixelFormat(codecpar->format);
	codec_context->height = codecpar->height;
	codec_context->width = codecpar->width;
	codec_context->thread_count = 16;
	ret = avcodec_open2(codec_context, codec, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
		return -1;
	}

	AVPixelFormat dst_pixel_format = AV_PIX_FMT_BGR24;
	AVFrame* frame = av_frame_alloc();
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	SwsContext* sws_context = sws_getContext(codec_context->width, codec_context->height, codec_context->pix_fmt, codec_context->width, codec_context->height, dst_pixel_format, 0, nullptr, nullptr, nullptr);
	if (!frame || !packet || !sws_context) {
		fprintf(stderr, "fail to alloc\n");
		return -1;
	}

	uint8_t *bgr_data[4];
	int bgr_linesize[4];
	av_image_alloc(bgr_data, bgr_linesize, codec_context->width, codec_context->height, dst_pixel_format, 1);
	cv::Mat mat(codec_context->height, codec_context->width, CV_8UC3);
	const char* winname = "dshow mjpeg video";
	cv::namedWindow(winname);

	while (1) {
		ret = av_read_frame(format_context, packet);
		if (ret >= 0 && packet->stream_index == video_stream_index && packet->size > 0) {
			ret = avcodec_send_packet(codec_context, packet);
			if (ret < 0) {
				fprintf(stderr, "##### fail to avcodec_send_packet: %d\n", ret);
				av_packet_unref(packet);
				continue;
			}

			ret = avcodec_receive_frame(codec_context, frame);
			if (ret < 0) {
				fprintf(stderr, "##### fail to avcodec_receive_frame: %d\n", ret);
				av_packet_unref(packet);
				continue;
			}

			sws_scale(sws_context, frame->data, frame->linesize, 0, codec_context->height, bgr_data, bgr_linesize);
			mat.data = bgr_data[0];
			cv::imshow(winname, mat);
		} else if (ret < 0 || packet->size <= 0) {
			fprintf(stderr, "##### fail to av_read_frame: %d, packet size: %d\n", ret, packet->size);
			continue;
		}

		av_packet_unref(packet);

		int key = cv::waitKey(30);
		if (key == 27) break;
	}

	cv::destroyWindow(winname);
	sws_freeContext(sws_context);
	av_frame_free(&frame);
	av_freep(packet);
	av_freep(&bgr_data[0]);
	avformat_close_input(&format_context);
	av_dict_free(&dict);
}

	fprintf(stdout, "test finish\n");
	return 0;
}
#else
int test_ffmpeg_decode_dshow()
{
	fprintf(stderr, "Error: this test code only support windows platform\n");
	return -1;
}
#endif

////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/95021281

namespace {

bool packet_scale_flag = true;

void get_packet(AVFormatContext* format_ctx, int video_stream_index, PacketScaleQueue& packet_scale)
{
	//for (int i = 0; i < 100; ++i) {
	while (packet_scale_flag) {
		AVPacket packet;
		//long long t1 = Timer::getNowTime();
		int ret = av_read_frame(format_ctx, &packet);
		//long long t2 = Timer::getNowTime();
		//fprintf(stdout, "av_read frame cost time: %lldms\n", t2 - t1);
		if (ret >= 0 && packet.stream_index == video_stream_index && packet.size > 0) {
			Buffer buffer;
			packet_scale.popPacket(buffer);
			memcpy(buffer.data, packet.data, packet.size);
			packet_scale.pushScale(buffer);
			av_packet_unref(&packet);
		} else {
			fprintf(stderr, "##### fail to av_read_frame: %d, %d\n", ret, packet.size);
		}
	}
}

void get_scale(AVCodecContext* codec_ctx, PacketScaleQueue& packet_scale)
{
	SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		fprintf(stderr, "##### fail to sws_getContext\n");
	}

	uint8_t *bgr_data[4], *yuyv422_data[4];
	int bgr_linesize[4], yuyv422_linesize[4];
	av_image_alloc(bgr_data, bgr_linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 1);
	av_image_alloc(yuyv422_data, yuyv422_linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_YUYV422, 1);

	cv::Mat mat(codec_ctx->height, codec_ctx->width, CV_8UC3);
	const char* winname = "usb video";
	cv::namedWindow(winname);

	//for (int i = 0; i < 100; ++i) {
	while (packet_scale_flag) {
		Buffer buffer;
		packet_scale.popScale(buffer);
		const uint8_t *srcSlice[1];
		srcSlice[0] = buffer.data;
		//long long t1 = Timer::getNowTime();
		sws_scale(sws_ctx, srcSlice, yuyv422_linesize, 0, codec_ctx->height, bgr_data, bgr_linesize);
		//long long t2 = Timer::getNowTime();
		//fprintf(stdout, "sws_scale cost time: %lldms\n", t2 - t1);
		packet_scale.pushPacket(buffer);

		mat.data = bgr_data[0];
		cv::imshow(winname, mat);
		//cv::imwrite("xxx.jpg", mat);

		int key = cv::waitKey(10);
		if (key == 27) { 
			packet_scale_flag = false;
			break;
		}
	}

	cv::destroyWindow(winname);
	sws_freeContext(sws_ctx);
	av_freep(&bgr_data[0]);
	av_freep(&yuyv422_data[0]);
}

} // namespace

int test_ffmpeg_stream_show_two_thread()
{
	avdevice_register_all();

#ifdef _MSC_VER
	const char* input_format_name = "vfwcap";
	const char* url = "";
#else
	const char* input_format_name = "video4linux2";
	const char* url = "/dev/video0";
#endif

	AVInputFormat* input_fmt = av_find_input_format(input_format_name);
	AVFormatContext* format_ctx = avformat_alloc_context();

	int ret = avformat_open_input(&format_ctx, url, input_fmt, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open url: %s, return value: %d\n", url, ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
		const AVStream* stream = format_ctx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	AVCodecParameters* codecpar = format_ctx->streams[video_stream_index]->codecpar;
	const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "fail to avcodec_find_decoder\n");
		return -1;
	}

	if (codecpar->codec_id != AV_CODEC_ID_RAWVIDEO) {
		fprintf(stderr, "this test code only support rawvideo encode: %d\n", codecpar->codec_id);
		return -1;
	}

	AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "fail to avcodec_alloc_context3\n");
		return -1;
	}

	codec_ctx->pix_fmt = AVPixelFormat(codecpar->format);
	codec_ctx->height = codecpar->height;
	codec_ctx->width = codecpar->width;
	codec_ctx->thread_count = 16;
	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
		return -1;
	}

	PacketScaleQueue packet_scale;
	packet_scale.init(16, 1024*1024*4);

	std::thread thread_packet(get_packet, format_ctx, video_stream_index, std::ref(packet_scale));
	std::thread thread_scale(get_scale, codec_ctx, std::ref(packet_scale));

	thread_packet.join();
	thread_scale.join();

	avformat_close_input(&format_ctx);

	fprintf(stdout, "test finish\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/94712986
int test_ffmpeg_stream_show()
{
	avdevice_register_all();

#ifdef _MSC_VER
	const char* input_format_name = "vfwcap";
	const char* url = "";
#else
	const char* input_format_name = "video4linux2";
	const char* url = "/dev/video0";
#endif

	AVInputFormat* input_fmt = av_find_input_format(input_format_name);
	AVFormatContext* format_ctx = avformat_alloc_context();

	int ret = avformat_open_input(&format_ctx, url, input_fmt, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open url: %s, return value: %d\n", url, ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
		const AVStream* stream = format_ctx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	AVCodecParameters* codecpar = format_ctx->streams[video_stream_index]->codecpar;
	const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "fail to avcodec_find_decoder\n");
		return -1;
	}

	if (codecpar->codec_id != AV_CODEC_ID_RAWVIDEO) {
		fprintf(stderr, "this test code only support rawvideo encode: %d\n", codecpar->codec_id);
		return -1;
	}

	AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "fail to avcodec_alloc_context3\n");
		return -1;
	}

	codec_ctx->pix_fmt = AVPixelFormat(codecpar->format);
	codec_ctx->height = codecpar->height;
	codec_ctx->width = codecpar->width;
	codec_ctx->thread_count = 16;
	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
		return -1;
	}

	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
	if (!packet || !sws_ctx) {
		fprintf(stderr, "fail to alloc\n");
		return -1;
	}

	uint8_t *bgr_data[4], *yuyv422_data[4];
	int bgr_linesize[4], yuyv422_linesize[4];
	av_image_alloc(bgr_data, bgr_linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 1);
	av_image_alloc(yuyv422_data, yuyv422_linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_YUYV422, 1);
	cv::Mat mat(codec_ctx->height, codec_ctx->width, CV_8UC3);
	const char* winname = "usb video";
	cv::namedWindow(winname);
	const uint8_t *srcSlice[1];

	while (1) {
		ret = av_read_frame(format_ctx, packet);
		if (ret >= 0 && packet->stream_index == video_stream_index) {
			srcSlice[0] = packet->data;
			sws_scale(sws_ctx, srcSlice, yuyv422_linesize, 0, codec_ctx->height, bgr_data, bgr_linesize);
			mat.data = bgr_data[0];
			cv::imshow(winname, mat);
		}

		av_packet_unref(packet);

		int key = cv::waitKey(25);
		if (key == 27) break;
	}

	cv::destroyWindow(winname);
	sws_freeContext(sws_ctx);
	avformat_close_input(&format_ctx);
	av_freep(packet);
	av_freep(&bgr_data[0]);
	av_freep(&yuyv422_data[0]);

	fprintf(stdout, "test finish\n");
	return 0;
}

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/93975325
int test_ffmpeg_decode_show_old()
{
	avdevice_register_all();

#ifdef _MSC_VER
	const char* input_format_name = "vfwcap";
	const char* url = "";
#else
	const char* input_format_name = "video4linux2";
	const char* url = "/dev/video0";
#endif

	AVInputFormat* input_fmt = av_find_input_format(input_format_name);
	AVFormatContext* format_ctx = avformat_alloc_context();
	int ret = avformat_open_input(&format_ctx, url, input_fmt, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open url: %s, return value: %d\n", url, ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (int i = 0; i < format_ctx->nb_streams; ++i) {
		const AVStream* stream = format_ctx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	AVCodecContext* codec_ctx = format_ctx->streams[video_stream_index]->codec;
	AVCodec* codec = avcodec_find_decoder(codec_ctx->codec_id);
	if (!codec) {
		fprintf(stderr, "no decoder was found\n");
		return -1;
	}

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to init AVCodecContext: %d\n", ret);
		return -1;
	}

	AVFrame* frame = av_frame_alloc();
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
	if (!frame || !packet || !sws_ctx) {
		fprintf(stderr, "fail to alloc\n");
		return -1;
	}

	int got_picture = -1;
	std::unique_ptr<uint8_t[]> data(new uint8_t[codec_ctx->width * codec_ctx->height * 3]);
	cv::Mat mat(codec_ctx->height, codec_ctx->width, CV_8UC3);
	int width_new = 320, height_new = 240;
	cv::Mat dst(height_new, width_new, CV_8UC3);
	const char* winname = "usb video1";
	cv::namedWindow(winname);

	while (1) {
		ret = av_read_frame(format_ctx, packet);
		if (ret < 0) {
			fprintf(stderr, "fail to av_read_frame: %d\n", ret);
			continue;
		}

		if (packet->stream_index == video_stream_index) {
			ret = avcodec_decode_video2(codec_ctx, frame, &got_picture, packet);
			if (ret < 0) {
				fprintf(stderr, "fail to avcodec_decode_video2: %d\n", ret);
				av_free_packet(packet);
				continue;
			}

			if (got_picture) {
				uint8_t* p[1] = { data.get() };
				int dst_stride[1] = { frame->width * 3 };
				sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, p, dst_stride);
				mat.data = data.get();
				cv::resize(mat, dst, cv::Size(width_new, height_new));
				cv::imshow(winname, dst);
			}
		}

		av_free_packet(packet);

		int key = cv::waitKey(25);
		if (key == 27) break;
	}

	cv::destroyWindow(winname);
	av_frame_free(&frame);
	sws_freeContext(sws_ctx);
	av_free(packet);
	avformat_close_input(&format_ctx);

	fprintf(stdout, "test finish\n");
	return 0;
}

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/93975844
int test_ffmpeg_decode_show_new()
{
	avdevice_register_all();

	AVDictionary* options = nullptr;
#ifdef _MSC_VER
	const char* input_format_name = "vfwcap";
	const char* url = "";
#else
	const char* input_format_name = "video4linux2";
	const char* url = "/dev/video0";
	av_dict_set(&options, "video_size", "640x480", 0);
	av_dict_set(&options, "input_format", "mjpeg", 0);
#endif

	AVInputFormat* input_fmt = av_find_input_format(input_format_name);
	AVFormatContext* format_ctx = avformat_alloc_context();

	int ret = avformat_open_input(&format_ctx, url, input_fmt, &options);
	if (ret != 0) {
		fprintf(stderr, "fail to open url: %s, return value: %d\n", url, ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
		const AVStream* stream = format_ctx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	AVCodecParameters* codecpar = format_ctx->streams[video_stream_index]->codecpar;
	const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "fail to avcodec_find_decoder\n");
		return -1;
	}

	AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "fail to avcodec_alloc_context3\n");
		return -1;
	}

	codec_ctx->pix_fmt = AVPixelFormat(codecpar->format);
	codec_ctx->height = codecpar->height;
	codec_ctx->width = codecpar->width;
	codec_ctx->thread_count = 4;
	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
		return -1;
	}

	AVFrame* frame = av_frame_alloc();
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
	if (!frame || !packet || !sws_ctx) {
		fprintf(stderr, "fail to alloc\n");
		return -1;
	}

	uint8_t* bgr_data[4];
	int bgr_linesize[4];
	av_image_alloc(bgr_data, bgr_linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, 1);
	cv::Mat mat(codec_ctx->height, codec_ctx->width, CV_8UC3);
	const char* winname = "usb video2";
	cv::namedWindow(winname);

	while (1) {
		ret = av_read_frame(format_ctx, packet);
		if (ret >= 0 && packet->stream_index == video_stream_index) {
			ret = avcodec_send_packet(codec_ctx, packet);
			if (ret < 0) {
				fprintf(stderr, "fail to avcodec_send_packet: %d\n", ret);
				av_packet_unref(packet);
				continue;
			}

			ret = avcodec_receive_frame(codec_ctx, frame);
			if (ret < 0) {
				fprintf(stderr, "fail to avcodec_receive_frame\n");
				av_packet_unref(packet);
				continue;
			}

			sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, bgr_data, bgr_linesize);
			mat.data = bgr_data[0];
			cv::imshow(winname, mat);
		}

		av_packet_unref(packet);

		int key = cv::waitKey(25);
		if (key == 27) break;
	}

	cv::destroyWindow(winname);
	av_frame_free(&frame);
	sws_freeContext(sws_ctx);
	av_dict_free(&options);
	avformat_close_input(&format_ctx);
	av_freep(packet);
	av_freep(&bgr_data[0]);

	fprintf(stdout, "test finish\n");
	return 0;
}
