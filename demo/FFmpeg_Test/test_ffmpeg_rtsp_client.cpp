#include "funset.hpp"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>
#include "common.hpp"

// Blog: https://blog.csdn.net/fengbingchun/article/details/95920528
int test_ffmpeg_rtsp_client_decode_show()
{
	const char* url = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_115k.mov";
	AVFormatContext* format_ctx = avformat_alloc_context();
	int ret = avformat_open_input(&format_ctx, url, nullptr, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open url: %s, return value: %d\n", url, ret);
		return -1;
	}

	ret = avformat_find_stream_info(format_ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1, audio_stream_index = -1;
	for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
		const AVStream* stream = format_ctx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		} else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_index = i;
			fprintf(stdout, "audio sample format: %d\n", stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	if (audio_stream_index == -1) {
		fprintf(stderr, "no audio stream\n");
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
	codec_ctx->thread_count = 16;
	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to avcodec_open2: %d\n", ret);
		return -1;
	}

	const int width_new = 640, height_new = 480;
	AVFrame* frame = av_frame_alloc();
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	SwsContext* sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, width_new, height_new, AV_PIX_FMT_BGR24, SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (!frame || !packet || !sws_ctx) {
		fprintf(stderr, "fail to alloc\n");
		return -1;
	}

	uint8_t* bgr_data[4];
	int bgr_linesize[4];
	av_image_alloc(bgr_data, bgr_linesize, width_new, height_new, AV_PIX_FMT_BGR24, 1);
	cv::Mat mat(height_new, width_new, CV_8UC3);
	const char* winname = "rtst video";
	cv::namedWindow(winname);
	long long time_begin, time_end;

	while (1) {
		//time_begin = Timer::getNowTime();
		ret = av_read_frame(format_ctx, packet);
		//time_end = Timer::getNowTime();
		//fprintf(stdout, "av_read_frame cost time: %lldms\n", time_end - time_begin);
		if (ret >= 0 && packet->stream_index == video_stream_index && packet->size > 0) {
			//time_begin = Timer::getNowTime();
			ret = avcodec_send_packet(codec_ctx, packet);
			//time_end = Timer::getNowTime();
			//fprintf(stdout, "avcodec_send_packet cost time: %lldms\n", time_end - time_begin);
			if (ret < 0) {
				fprintf(stderr, "##### fail to avcodec_send_packet: %d\n", ret);
				av_packet_unref(packet);
				continue;
			}

			//time_begin = Timer::getNowTime();
			ret = avcodec_receive_frame(codec_ctx, frame);
			//time_end = Timer::getNowTime();
			//fprintf(stdout, "avcodec_receive_frame cost time: %lldms\n", time_end - time_begin);
			if (ret < 0) {
				fprintf(stderr, "##### fail to avcodec_receive_frame: %d\n", ret);
				av_packet_unref(packet);
				continue;
			}

			//time_begin = Timer::getNowTime();
			sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, bgr_data, bgr_linesize);
			//time_end = Timer::getNowTime();
			//fprintf(stdout, "sws_scale cost time: %lldms\n", time_end - time_begin);
			mat.data = bgr_data[0];
			cv::imshow(winname, mat);
		} else if (ret < 0 || packet->size <= 0) {
			fprintf(stderr, "##### fail to av_read_frame: %d, packet size: %d\n", ret, packet->size);
			continue;
		}

		av_packet_unref(packet);

		int key = cv::waitKey(10);
		if (key == 27) break;
	}

	cv::destroyWindow(winname);
	av_frame_free(&frame);
	sws_freeContext(sws_ctx);
	avformat_close_input(&format_ctx);
	av_freep(packet);
	av_freep(&bgr_data[0]);

	fprintf(stdout, "test finish\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/92198857
int test_ffmpeg_rtsp_client()
{
	// Allocate an AVFormatContext
	AVFormatContext* format_ctx = avformat_alloc_context();

	// open rtsp: Open an input stream and read the header. The codecs are not opened
	const char* url = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
	int ret = -1;
	ret = avformat_open_input(&format_ctx, url, nullptr, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open url: %s, return value: %d\n", url, ret);
		return -1;
	}

	// Read packets of a media file to get stream information
	ret = avformat_find_stream_info(format_ctx, nullptr);
	if ( ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	// audio/video stream index
	int video_stream_index = -1;
	int audio_stream_index = -1;
	fprintf(stdout, "Number of elements in AVFormatContext.streams: %d\n", format_ctx->nb_streams);
	for (int i = 0; i < format_ctx->nb_streams; ++i) {
		const AVStream* stream = format_ctx->streams[i];
		fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		} else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_index = i;
			fprintf(stdout, "audio sample format: %d\n", stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "no video stream\n");
		return -1;
	}

	if (audio_stream_index == -1) {
		fprintf(stderr, "no audio stream\n");
	}

	int cnt = 0;
	AVPacket pkt;
	while (1) {
		if (++cnt > 100) break;

		ret = av_read_frame(format_ctx, &pkt);
		if (ret < 0) {
			fprintf(stderr, "error or end of file: %d\n", ret);
			continue;
		}

		if (pkt.stream_index == video_stream_index) {
			fprintf(stdout, "video stream, packet size: %d\n", pkt.size);
		}

		if (pkt.stream_index == audio_stream_index) {
			fprintf(stdout, "audio stream, packet size: %d\n", pkt.size);
		}

		av_packet_unref(&pkt);
	}

	avformat_free_context(format_ctx);

	return 0;
}

