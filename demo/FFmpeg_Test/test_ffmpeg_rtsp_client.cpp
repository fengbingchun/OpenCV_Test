#include "funset.hpp"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

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

