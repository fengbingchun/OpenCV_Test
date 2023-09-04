#include "common.hpp"
#include <string.h>

namespace {

int read_packet(void* opaque, uint8_t* buf, int buf_size)
{
	PacketScaleQueue* raw_packet = static_cast<PacketScaleQueue*>(opaque);
	Buffer buffer;
	raw_packet->popScale(buffer);
	memcpy(buf, buffer.data, buf_size);
	raw_packet->pushPacket(buffer);
	return buf_size;
}

char* err2str(int errnum)
{
	char errbuf[AV_ERROR_MAX_STRING_SIZE];
	memset(errbuf, 0, AV_ERROR_MAX_STRING_SIZE);
	return av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
}

int encode_write_one_frame(CodecQueue& codec_queue, CodecCtx* codec_ctx)
{
	AVFrame* filt_frame = nullptr;
	codec_queue.popEncode(&filt_frame);
	if (!filt_frame) return -1;
	//fprintf(stderr, "#### line: %d; linesize: %d, %d, %d, %d; width: %d, height: %d\n", __LINE__,
	//	filt_frame->linesize[0], filt_frame->linesize[1], filt_frame->linesize[2], filt_frame->linesize[3],
	//	filt_frame->width, filt_frame->height);

	// encode filtered frame
	AVPacket* enc_pkt = codec_ctx->enc_pkt;
	auto ret = avcodec_send_frame(codec_ctx->enc_ctx, filt_frame);
	if (ret < 0) {
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			fprintf(stderr, "#### line: %d, warning: %s\n", __LINE__, err2str(ret));
			ret = 0;
		}
		else {
			fprintf(stderr, "Error sending a frame for encoding. Error code: %s\n", err2str(ret));
			return -1;
		}
	}

	while (1) {
		ret = avcodec_receive_packet(codec_ctx->enc_ctx, enc_pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		if (ret < 0) {
			av_log(nullptr, AV_LOG_FATAL, "Error during encoding %d ret = %d \n", __LINE__, ret);
			return -1;
		}

		enc_pkt->stream_index = 0;
		enc_pkt->pts = enc_pkt->dts = codec_ctx->frame_count *
			codec_ctx->ofmt_ctx->streams[0]->time_base.den / codec_ctx->ofmt_ctx->streams[0]->time_base.num
			/ (codec_ctx->enc_ctx->framerate.num / codec_ctx->enc_ctx->framerate.den);
		// av_packet_rescale_ts(
		//     &enc_pkt, codec_ctx->stream_ctx[stream_index].enc_ctx->time_base,
		//     codec_ctx->ofmt_ctx->streams[0]->time_base);

		ret = av_interleaved_write_frame(codec_ctx->ofmt_ctx, enc_pkt);
		if (ret < 0) {
			fprintf(stderr, "Error during writing data to output file. Error code: %s\n", err2str(ret));
			return -1;
		}

		codec_ctx->frame_count++;
	}

	av_frame_free(&filt_frame);
	filt_frame = nullptr;
	codec_queue.pushDecode(&filt_frame);
	return 0;
}

void encode_write_frame(CodecQueue& codec_queue, CodecCtx* codec_ctx)
{
	while (!codec_ctx->encode_thread_end) {
		auto ret = encode_write_one_frame(codec_queue, codec_ctx);
		if (ret != 0) {
			fprintf(stderr, "#### Warning: encode_write_one_frame: %d\n", ret);
			//break;
		}
	}
	codec_ctx->exit_encode_thread = true;
}

} // namespace

int VideoCodec::openEncode()
{
	codec_ctx_ = static_cast<CodecCtx*>(malloc(sizeof(CodecCtx)));
	if (!codec_ctx_) {
		fprintf(stderr, "Error: fail to malloc CodecCtx\n");
		return -1;
	}
	memset(codec_ctx_, 0, sizeof(CodecCtx));

	codec_ctx_->frame_rate = av_d2q(30, 4096);
	strcpy(codec_ctx_->outfile_name, outfile_name_.c_str());
	strcpy(codec_ctx_->video_size, video_size_.c_str());
	strcpy(codec_ctx_->bitrate_str, "50000000");
	strcpy(codec_ctx_->pixel_format, pixel_format_.c_str());
	strcpy(codec_ctx_->filter_descr, filter_descr_.c_str());

	uint8_t* avio_ctx_buffer = static_cast<uint8_t*>(av_malloc(block_size_));
	if (!avio_ctx_buffer) {
		fprintf(stderr, "Error: avio_ctx_buffer malloc failed\n");
		return -1;
	}

	avio_ctx_ = avio_alloc_context(avio_ctx_buffer, block_size_, 0, &raw_packet_queue_, &read_packet, nullptr, nullptr);
	if (!avio_ctx_) {
		fprintf(stderr, "Error: fail to avio_alloc_context\n");
		return -1;
	}

	codec_ctx_->ifmt_ctx = avformat_alloc_context();
	if (!codec_ctx_->ifmt_ctx) {
		fprintf(stderr, "Error: codec_ctx_->ifmt_ctx malloc failed\n");
		return -1;
	}
	codec_ctx_->ifmt_ctx->pb = avio_ctx_;

	AVDictionary* format_opts = nullptr;
	av_dict_set(&format_opts, "video_size", codec_ctx_->video_size, 0);
	av_dict_set(&format_opts, "pixel_format", codec_ctx_->pixel_format, 0);

	auto ret = avformat_open_input(&codec_ctx_->ifmt_ctx, nullptr, av_find_input_format("rawvideo"), &format_opts);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(codec_ctx_->ifmt_ctx, nullptr)) < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
		return ret;
	}

	av_dump_format(codec_ctx_->ifmt_ctx, 0, "nothing", 0);
	return 0;
}

int VideoCodec::get_decode_context()
{
	AVCodec* decoder = nullptr;
	auto ret = av_find_best_stream(codec_ctx_->ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
	if (ret < 0) {
		fprintf(stderr, "Cannot find a video stream in the input file. Error code: %s\n", err2str(ret));
		return ret;
	}

	codec_ctx_->stream_index = ret;
	AVStream* stream = codec_ctx_->ifmt_ctx->streams[codec_ctx_->stream_index];

	AVCodecContext* avcodec_ctx = avcodec_alloc_context3(decoder);
	if (!avcodec_ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", codec_ctx_->stream_index);
		return AVERROR(ENOMEM);
	}
	ret = avcodec_parameters_to_context(avcodec_ctx, stream->codecpar);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context for stream #%u\n", codec_ctx_->stream_index);
		return ret;
	}

	// Reencode video & audio and remux subtitles etc
	stream->r_frame_rate = codec_ctx_->frame_rate;
	avcodec_ctx->framerate = av_guess_frame_rate(codec_ctx_->ifmt_ctx, stream, nullptr);
	// Open decoder
	ret = avcodec_open2(avcodec_ctx, decoder, nullptr);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", codec_ctx_->stream_index);
		return ret;
	}
	codec_ctx_->dec_ctx = avcodec_ctx;
	codec_ctx_->dec_frame = av_frame_alloc();
	if (!codec_ctx_->dec_frame)
		return AVERROR(ENOMEM);
	return 0;
}

int VideoCodec::init_filters()
{
	AVFilterContext* buffersrc_ctx = nullptr;
	AVFilterContext* buffersink_ctx = nullptr;
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();

	auto ret = AVERROR(ENOMEM);
	codec_ctx_->filter_graph = avfilter_graph_alloc();
	if (!outputs || !inputs || !codec_ctx_->filter_graph) {
		return ret;
	}

	const AVFilter* buffersrc = avfilter_get_by_name("buffer");
	const AVFilter* buffersink = avfilter_get_by_name("buffersink");
	if (!buffersrc || !buffersink) {
		av_log(nullptr, AV_LOG_ERROR, "filtering source or sink element not found\n");
		ret = AVERROR_UNKNOWN;
		return ret;
	}

	char args[512];
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		codec_ctx_->dec_ctx->width, codec_ctx_->dec_ctx->height, codec_ctx_->dec_ctx->pix_fmt,
		codec_ctx_->dec_ctx->time_base.num, codec_ctx_->dec_ctx->time_base.den,
		codec_ctx_->dec_ctx->sample_aspect_ratio.num, codec_ctx_->dec_ctx->sample_aspect_ratio.den);
	ret = avfilter_graph_create_filter(&codec_ctx_->buffersrc_ctx, buffersrc, "in", args, nullptr, codec_ctx_->filter_graph);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot create buffer source\n");
		return ret;
	}

	ret = avfilter_graph_create_filter(&codec_ctx_->buffersink_ctx, buffersink, "out", nullptr, nullptr, codec_ctx_->filter_graph);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
		print_error_string(ret);
		return ret;
	}

	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	ret = av_opt_set_int_list(codec_ctx_->buffersink_ctx, "pix_fmts", pix_fmts,
		AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
		print_error_string(ret);
		return ret;
	}

	outputs->name = av_strdup("in");
	outputs->filter_ctx = codec_ctx_->buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = nullptr;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = codec_ctx_->buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = nullptr;

	if ((ret = avfilter_graph_parse_ptr(codec_ctx_->filter_graph, filter_descr_.c_str(), &inputs, &outputs, nullptr)) < 0) {
		print_error_string(ret);
		return ret;
	}

	if ((ret = avfilter_graph_config(codec_ctx_->filter_graph, nullptr)) < 0) {
		print_error_string(ret);
		return ret;
	}

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	codec_ctx_->enc_pkt = av_packet_alloc();
	if (!codec_ctx_->enc_pkt)
		return AVERROR(ENOMEM);

	return 0;
}

int VideoCodec::get_encode_context()
{
	AVCodecContext* dec_ctx = codec_ctx_->dec_ctx;
	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		// in this example, we choose transcoding to same codec
		AVCodec* encoder = avcodec_find_encoder_by_name("mpeg4");
		if (!encoder) {
			av_log(nullptr, AV_LOG_FATAL, "Necessary encoder not found\n");
			return AVERROR_INVALIDDATA;
		}

		AVCodecContext* enc_ctx = avcodec_alloc_context3(encoder);
		if (!enc_ctx) {
			av_log(nullptr, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
			return AVERROR(ENOMEM);
		}

		enc_ctx->bit_rate = 400000;
		enc_ctx->framerate = dec_ctx->framerate;
		// In this example, we transcode to same properties (picture size, sample rate etc.). 
		// These properties can be changed for output streams easily using filters
		enc_ctx->height = codec_ctx_->buffersink_ctx->inputs[0]->h;
		enc_ctx->width = codec_ctx_->buffersink_ctx->inputs[0]->w;
		enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
		// take first format from list of supported formats
		enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
		enc_ctx->sw_pix_fmt = AV_PIX_FMT_NV12;

		// video time_base can be set to whatever is handy and supported by encoder
		if (codec_ctx_->frame_rate.num)
			enc_ctx->time_base = av_inv_q(codec_ctx_->frame_rate);
		else
			enc_ctx->time_base = av_inv_q(dec_ctx->framerate);

		// if (codec_ctx_->ofmt_ctx->oformat->flags & 
		//     AVFMT_GLOBALHEADER) // feng: it seems to be true all the time
		enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		// Third parameter can be used to pass settings to encoder
		auto ret = avcodec_open2(enc_ctx, encoder, nullptr);
		if (ret < 0) {
			av_log(nullptr, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", 0);
			return ret;
		}
		codec_ctx_->enc_ctx = enc_ctx;
	}
	else {
		// if this stream must be remuxed
		av_log(nullptr, AV_LOG_ERROR, "Error stream not supported\n");
	}
	return 0;
}

int VideoCodec::get_output_format_context()
{
	codec_ctx_->ofmt_ctx = nullptr;
	if (strcmp(outfile_name_.c_str(), "-"))
		avformat_alloc_output_context2(&codec_ctx_->ofmt_ctx, nullptr, nullptr, outfile_name_.c_str());
	else
		avformat_alloc_output_context2(&codec_ctx_->ofmt_ctx, nullptr, "nullptr", outfile_name_.c_str());
	if (!codec_ctx_->ofmt_ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Could not create output context\n");
		return AVERROR_UNKNOWN;
	}
	AVStream* out_stream = avformat_new_stream(codec_ctx_->ofmt_ctx, nullptr);
	if (!out_stream) {
		av_log(nullptr, AV_LOG_ERROR, "Failed allocating output stream\n");
		return AVERROR_UNKNOWN;
	}

	AVCodecContext* dec_ctx = codec_ctx_->dec_ctx;
	int ret = -1;
	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		AVCodecContext* enc_ctx = codec_ctx_->enc_ctx;
		ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
		if (ret < 0) {
			av_log(nullptr, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", ret);
			return ret;
		}

		out_stream->time_base = enc_ctx->time_base;
	}
	else {
		// if this stream must be remuxed
		av_log(nullptr, AV_LOG_ERROR, "Error stream not supported\n");
	}

	if (!(codec_ctx_->ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&codec_ctx_->ofmt_ctx->pb, outfile_name_.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			av_log(nullptr, AV_LOG_ERROR, "Could not open output file '%s'", outfile_name_.c_str());
			return ret;
		}
	}

	// init muxer, write output file header
	ret = avformat_write_header(codec_ctx_->ofmt_ctx, nullptr);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Error occurred when opening output file\n");
		return ret;
	}
	av_dump_format(codec_ctx_->ofmt_ctx, 0, outfile_name_.c_str(), 1);
	return 0;
}

int VideoCodec::filter_encode_write_frame(AVFrame* frame)
{
	// push the decoded frame into the filtergraph
	auto ret = av_buffersrc_add_frame_flags(codec_ctx_->buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_PUSH);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
		return ret;
	}

	// pull filtered frames from the filtergraph
	while (1) {
		AVFrame* filt_frame = av_frame_alloc();
		if (!filt_frame) {
			ret = AVERROR(ENOMEM);
			break;
		}

		ret = av_buffersink_get_frame_flags(codec_ctx_->buffersink_ctx, filt_frame, AV_BUFFERSINK_FLAG_NO_REQUEST);
		if (ret < 0) {
			// if no more frames for output - returns AVERROR(EAGAIN)
			// if flushed and no more frames for output - returns AVERROR_EOF
			// rewrite retcode to 0 to show it as normal procedure completion
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				ret = 0;
			av_frame_free(&filt_frame);
			break;
		}

		filt_frame->pict_type = AV_PICTURE_TYPE_NONE;

		AVFrame* tmp = nullptr;
		codec_queue_.popDecode(&tmp);
		codec_queue_.pushEncode(&filt_frame);
	}

	return ret;
}

int VideoCodec::flush_decoder()
{
	auto ret = avcodec_send_packet(codec_ctx_->dec_ctx, nullptr);
	if (ret < 0) {
		fprintf(stderr, "Error during decoding. Error code: %s\n", err2str(ret));
		return ret;
	}

	while (ret >= 0) {
		AVFrame* frame = av_frame_alloc();
		if (!frame) {
			return AVERROR(ENOMEM);
		}

		ret = avcodec_receive_frame(codec_ctx_->dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&frame);
			ret = 0;
			break;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error: while decoding, Error code: %s\n", err2str(ret));
			return ret;
		}

		ret = filter_encode_write_frame(frame);
		if (ret) {
			if (!codec_ctx_->term_status)
				av_log(nullptr, AV_LOG_ERROR, "filter_encode_write_frame \n");
			return ret;
		}

		av_frame_free(&frame);
		if (ret < 0)
			return ret;
	}

	return ret;
}

int VideoCodec::flush_encode_write_frame()
{
	auto ret = avcodec_send_frame(codec_ctx_->enc_ctx, nullptr);
	if (ret < 0) {
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			ret = 0;
		}
		else {
			return ret;
		}
	}

	AVPacket* enc_pkt = av_packet_alloc();
	if (!enc_pkt) {
		fprintf(stderr, "fail to av_packet_alloc\n");
		return -1;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(codec_ctx_->enc_ctx, enc_pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			//ret = 0;
			break;
		}
		if (ret < 0) {
			av_log(nullptr, AV_LOG_FATAL, "Error during encoding %d\n", __LINE__);
			return -1;
		}

		enc_pkt->stream_index = 0;
		// av_packet_rescale_ts(
		//     &enc_pkt, codec_ctx->ifmt_ctx->streams[stream_index]->time_base,
		//     codec_ctx->ofmt_ctx->streams[0]->time_base);

		enc_pkt->pts = enc_pkt->dts = codec_ctx_->frame_count *
			codec_ctx_->ofmt_ctx->streams[0]->time_base.den / codec_ctx_->ofmt_ctx->streams[0]->time_base.num
			/ (codec_ctx_->enc_ctx->framerate.num / codec_ctx_->enc_ctx->framerate.den);

		ret = av_interleaved_write_frame(codec_ctx_->ofmt_ctx, enc_pkt);
		if (ret < 0) {
			fprintf(stderr, "Error during writing data to output file, Error code: %s\n", err2str(ret));
			return -1;
		}
		codec_ctx_->frame_count++;
	}

	//fprintf(stderr, "#### line: %d, %d\n", __LINE__, codec_ctx_->frame_count);
	av_packet_unref(enc_pkt);
	av_packet_free(&enc_pkt);
	if (ret == AVERROR_EOF)
		return 0;
	ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
	return ret;
}

int VideoCodec::flush_encoder()
{
	//if (!(codec_ctx_->enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY))
	//	return 0;

	av_log(nullptr, AV_LOG_INFO, "Flushing stream video encoder\n");
	auto ret = flush_encode_write_frame();
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Error: Flushing encoder failed\n");
		return ret;
	}

	return ret;
}

void VideoCodec::flush_codec()
{
	auto ret = flush_decoder();
	if (ret != 0)
		fprintf(stderr, "Warning: flush_decoder: %d\n", ret);

	while (codec_queue_.getEncodeSize() > 0) {
		if (encode_write_one_frame(codec_queue_, codec_ctx_) != 0)
			break;
	}

	if ((ret = flush_encoder()) != 0)
		fprintf(stderr, "Warning: flush_encoder: %d\n", ret);

	av_write_trailer(codec_ctx_->ofmt_ctx);
}

int VideoCodec::processEncode()
{
	AVFormatContext* ifmt_ctx = codec_ctx_->ifmt_ctx;
	av_log_set_level(AV_LOG_ERROR);

	AVPacket* packet = av_packet_alloc();
	if (!packet) {
		fprintf(stderr, "Error: fail to av_packet_alloc\n");
		return -1;
	}

	codec_queue_.init(10);
	codec_ctx_->encode_thread_end = false;
	encode_thread_ = std::thread(encode_write_frame, std::ref(codec_queue_), codec_ctx_);

	codec_ctx_->ofmt_ctx = (AVFormatContext*)malloc(sizeof(AVFormatContext));
	if (!codec_ctx_->ofmt_ctx) {
		fprintf(stderr, "Error: fail to codec_ctx->ofmt_ctx malloc\n");
		return -1;
	}
	memset(codec_ctx_->ofmt_ctx, 0, sizeof(AVFormatContext));

	auto ret = get_decode_context();
	if (ret != 0) {
		fprintf(stderr, "Error: fail to get_decode_context: %d\n", ret);
		return -1;
	}

	ret = init_filters();
	if (ret != 0) {
		fprintf(stderr, "Error: fail to init_filters: %d\n", ret);
		return -1;
	}
	ret = get_encode_context();
	if (ret != 0) {
		fprintf(stderr, "Error: fail to get_encode_context: %d\n", ret);
		return -1;
	}

	ret = get_output_format_context();
	if (ret != 0) {
		fprintf(stderr, "Error: fail to get_output_format_context: %d\n", ret);
		return -1;
	}


	int total_frames = 0;
	while (!codec_ctx_->term_status) {
		if ((ret = av_read_frame(ifmt_ctx, packet)) < 0) {
			break;
		}

		if (codec_ctx_->stream_index != packet->stream_index) {
			av_packet_unref(packet);
			fprintf(stderr, "Error: codec_ctx_->stream_index != packet->stream_index \n");
			continue;
		}

		av_packet_rescale_ts(packet,
			ifmt_ctx->streams[codec_ctx_->stream_index]->time_base,
			codec_ctx_->dec_ctx->time_base);

		ret = avcodec_send_packet(codec_ctx_->dec_ctx, packet);
		if (ret < 0) {
			fprintf(stderr, "Error: during decoding, Error code: %s\n", err2str(ret));
			return -1;
		}

		while (1) {
			AVFrame* frame = codec_ctx_->dec_frame;
			ret = avcodec_receive_frame(codec_ctx_->dec_ctx, frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				break;
			}
			else if (ret < 0) {
				fprintf(stderr, "Error: avcodec_receive_frame: %d\n", ret);
				return -1;
			}

			frame->pts = frame->best_effort_timestamp;
			ret = filter_encode_write_frame(frame);
			if (ret) {
				if (!codec_ctx_->term_status)
					av_log(nullptr, AV_LOG_ERROR, "filter_encode_write_frame \n");
				return -1;
			}

			if (ret < 0)
				return -1;
			break;
		}

		++total_frames;
		//fprintf(stderr, "#### total frames: %d\n", total_frames);
		av_packet_unref(packet);
	}
		
	codec_ctx_->encode_thread_end = true;
	while (!codec_ctx_->exit_encode_thread)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	flush_codec();
	av_packet_free(&packet);

	if (ret < 0)
		av_log(nullptr, AV_LOG_ERROR, "Error occurred: %s\n", err2str(ret));
	return ret;
}

int VideoCodec::closeEncode()
{
	encode_thread_.join();
	codec_queue_.release();

	avcodec_free_context(&codec_ctx_->dec_ctx);
	if (codec_ctx_->ofmt_ctx && codec_ctx_->ofmt_ctx->streams && codec_ctx_->enc_ctx)
		avcodec_free_context(&codec_ctx_->enc_ctx);

	av_frame_free(&codec_ctx_->dec_frame);

	if (codec_ctx_->ofmt_ctx && !(codec_ctx_->ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		avio_closep(&codec_ctx_->ofmt_ctx->pb);
	}
	avformat_free_context(codec_ctx_->ofmt_ctx);
	codec_ctx_->ofmt_ctx = nullptr;

	if (codec_ctx_->ofmt_ctx) {
		free(codec_ctx_->ofmt_ctx);
		codec_ctx_->ofmt_ctx = nullptr;
	}

	codec_ctx_->frame_count = 0;

	av_packet_free(&codec_ctx_->enc_pkt);

	if (codec_ctx_->filter_graph)
		avfilter_graph_free(&codec_ctx_->filter_graph);

	avformat_close_input(&codec_ctx_->ifmt_ctx);

	if (avio_ctx_) {
		av_freep(&avio_ctx_->buffer);
		av_freep(&avio_ctx_);
		avio_ctx_ = nullptr;
	}

	if (codec_ctx_) {
		free(codec_ctx_);
		codec_ctx_ = nullptr;
	}

	return 0;
}
