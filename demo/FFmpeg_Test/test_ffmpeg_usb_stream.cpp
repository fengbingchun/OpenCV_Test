#include "funset.hpp"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavdevice/avdevice.h>

#ifdef __cplusplus
}
#endif

// Blog: https://blog.csdn.net/fengbingchun/article/details/93325025

int test_ffmpeg_usb_stream()
{
	avdevice_register_all();

#ifdef _MSC_VER
	const char* input_format_name = "vfwcap";
	const char* url = "";
#else
	const char* input_format_name = "v4l2";
	const char* url = "/dev/video0";
#endif
	AVInputFormat* fmt = av_find_input_format(input_format_name);

	AVFormatContext* ctx = avformat_alloc_context();
	int ret = avformat_open_input(&ctx, url, fmt, nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to open input stream: %d\n", ret);
		return -1;
	}

	// read packets of a media file to get stream information
	ret = avformat_find_stream_info(ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "fail to get stream information: %d\n", ret);
		return -1;
	}

	// find audio/video stream index
	int video_stream_index = -1;
	int audio_stream_index = -1;
	fprintf(stdout, "Number of elements in AVFormatContext.streams: %d\n", ctx->nb_streams);
	for (int i = 0; i < ctx->nb_streams; ++i) {
		const AVStream* stream = ctx->streams[i];
		fprintf(stdout, "type of the encoded data: %d\n", stream->codecpar->codec_id);
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
		}
		else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_index = i;
			fprintf(stdout, "audio sample format: %d\n", stream->codecpar->format);
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "Error: no video stream\n");
		return -1;
	}

	if (audio_stream_index == -1) {
		fprintf(stdout, "Warning: no audio stream\n");
	}

	int cnt = 0;
	AVPacket pkt;
	while (1) {
		if (++cnt > 100) break;

		ret = av_read_frame(ctx, &pkt);
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

	avformat_close_input(&ctx);

	fprintf(stdout, "test finish\n");
	return 0;
}
