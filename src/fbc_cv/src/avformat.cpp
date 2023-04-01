// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <stdio.h>
#include <inttypes.h>
#include <cstdint>
#include <string>
#include "thread.hpp"
#include "avformat.hpp"
#include "avmem.hpp"
#include "avutil.hpp"
#include "avcodec.hpp"
#include "avoption.hpp"
#include "avpacket.hpp"
#include "id3v2.hpp"
#include "mathematics.hpp"

// reference: ffmpeg 4.2
//            libavformat/utils.c

#ifdef _MSC_VER

FF_DISABLE_DEPRECATION_WARNINGS
#include "options_table.hpp"
FF_ENABLE_DEPRECATION_WARNINGS

namespace fbc {

static const AVInputFormat * const *indev_list = NULL;
static const AVOutputFormat * const *outdev_list = NULL;

static const AVOutputFormat *muxer_list[] = { NULL };
static const AVInputFormat *demuxer_list[] = { NULL };

static AVMutex avpriv_register_devices_mutex = AV_MUTEX_INITIALIZER;

const AVCodecTag ff_codec_bmp_tags[] = {
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'J', 'P', 'G') },
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'S', 'C', '2') }, /* Multiscope II */
	{ AV_CODEC_ID_MJPEG, MKTAG('L', 'J', 'P', 'G') },
	{ AV_CODEC_ID_MJPEG, MKTAG('d', 'm', 'b', '1') },
	{ AV_CODEC_ID_MJPEG, MKTAG('m', 'j', 'p', 'a') },
	{ AV_CODEC_ID_MJPEG, MKTAG('J', 'R', '2', '4') }, /* Quadrox Mjpeg */
	{ AV_CODEC_ID_LJPEG, MKTAG('L', 'J', 'P', 'G') },
	/* Pegasus lossless JPEG */
	{ AV_CODEC_ID_MJPEG, MKTAG('J', 'P', 'G', 'L') },
	/* JPEG-LS custom FOURCC for AVI - encoder */
	{ AV_CODEC_ID_JPEGLS, MKTAG('M', 'J', 'L', 'S') },
	{ AV_CODEC_ID_JPEGLS, MKTAG('M', 'J', 'P', 'G') },
	/* JPEG-LS custom FOURCC for AVI - decoder */
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'J', 'L', 'S') },
	{ AV_CODEC_ID_MJPEG, MKTAG('j', 'p', 'e', 'g') },
	{ AV_CODEC_ID_MJPEG, MKTAG('I', 'J', 'P', 'G') },
	{ AV_CODEC_ID_AVRN, MKTAG('A', 'V', 'R', 'n') },
	{ AV_CODEC_ID_MJPEG, MKTAG('A', 'C', 'D', 'V') },
	{ AV_CODEC_ID_MJPEG, MKTAG('Q', 'I', 'V', 'G') },
	/* SL M-JPEG */
	{ AV_CODEC_ID_MJPEG, MKTAG('S', 'L', 'M', 'J') },
	/* Creative Webcam JPEG */
	{ AV_CODEC_ID_MJPEG, MKTAG('C', 'J', 'P', 'G') },
	/* Intel JPEG Library Video Codec */
	{ AV_CODEC_ID_MJPEG, MKTAG('I', 'J', 'L', 'V') },
	/* Midvid JPEG Video Codec */
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'V', 'J', 'P') },
	{ AV_CODEC_ID_MJPEG, MKTAG('A', 'V', 'I', '1') },
	{ AV_CODEC_ID_MJPEG, MKTAG('A', 'V', 'I', '2') },
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'T', 'S', 'J') },
	/* Paradigm Matrix M-JPEG Codec */
	{ AV_CODEC_ID_MJPEG, MKTAG('Z', 'J', 'P', 'G') },
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'M', 'J', 'P') },
	{ AV_CODEC_ID_NONE, 0 }
};

const struct AVCodecTag *avformat_get_riff_video_tags(void)
{
	return ff_codec_bmp_tags;
}

enum AVCodecID av_codec_get_id(const AVCodecTag *const *tags, unsigned int tag)
{
	int i;
	for (i = 0; tags && tags[i]; i++) {
		enum AVCodecID id = ff_codec_get_id(tags[i], tag);
		if (id != AV_CODEC_ID_NONE)
			return id;
	}
	return AV_CODEC_ID_NONE;
}

enum AVCodecID ff_codec_get_id(const AVCodecTag *tags, unsigned int tag)
{
	int i;
	for (i = 0; tags[i].id != AV_CODEC_ID_NONE; i++)
	if (tag == tags[i].tag)
		return tags[i].id;
	for (i = 0; tags[i].id != AV_CODEC_ID_NONE; i++)
	if (avpriv_toupper4(tag) == avpriv_toupper4(tags[i].tag))
		return tags[i].id;
	return AV_CODEC_ID_NONE;
}

#define RELATIVE_TS_BASE (INT64_MAX - (1LL<<48))

static void free_stream(AVStream **pst)
{
	AVStream *st = *pst;
	int i;

	if (!st)
		return;

	for (i = 0; i < st->nb_side_data; i++)
		av_freep(&st->side_data[i].data);
	av_freep(&st->side_data);

	if (st->parser)
		av_parser_close(st->parser);

	if (st->attached_pic.data)
		av_packet_unref(&st->attached_pic);

	if (st->internal) {
		avcodec_free_context(&st->internal->avctx);
		for (i = 0; i < st->internal->nb_bsfcs; i++) {
			av_bsf_free(&st->internal->bsfcs[i]);
			av_freep(&st->internal->bsfcs);
		}
		av_freep(&st->internal->priv_pts);
		av_bsf_free(&st->internal->extract_extradata.bsf);
		av_packet_free(&st->internal->extract_extradata.pkt);
	}
	av_freep(&st->internal);

	av_dict_free(&st->metadata);
	avcodec_parameters_free(&st->codecpar);
	av_freep(&st->probe_data.buf);
	av_freep(&st->index_entries);
#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
	avcodec_free_context(&st->codec);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
	av_freep(&st->priv_data);
	if (st->info)
		av_freep(&st->info->duration_error);
	av_freep(&st->info);
#if FF_API_LAVF_FFSERVER
FF_DISABLE_DEPRECATION_WARNINGS
	av_freep(&st->recommended_encoder_configuration);
FF_ENABLE_DEPRECATION_WARNINGS
#endif

	av_freep(pst);
}

AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c)
{
	AVStream *st;
	int i;
	AVStream **streams;

	if (s->nb_streams >= FFMIN(s->max_streams, INT_MAX / sizeof(*streams))) {
		if (s->max_streams < INT_MAX / sizeof(*streams))
			fprintf(stderr, "s: Number of streams exceeds max_streams parameter (%d), see the documentation if you wish to increase it\n", s->max_streams);
		return NULL;
	}
	streams = static_cast<AVStream**>(av_realloc_array(s->streams, s->nb_streams + 1, sizeof(*streams)));
	if (!streams)
		return NULL;
	s->streams = streams;

	st = static_cast<AVStream*>(av_mallocz(sizeof(AVStream)));
	if (!st)
		return NULL;
	if (!(st->info = static_cast<AVStream::info_*>(av_mallocz(sizeof(*st->info))))) {
		av_free(st);
		return NULL;
	}
	st->info->last_dts = AV_NOPTS_VALUE;

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
	st->codec = avcodec_alloc_context3(c);
	if (!st->codec) {
		av_free(st->info);
		av_free(st);
		return NULL;
	}
FF_ENABLE_DEPRECATION_WARNINGS
#endif

	st->internal = static_cast<AVStreamInternal*>(av_mallocz(sizeof(*st->internal)));
	if (!st->internal)
		goto fail;

	st->codecpar = avcodec_parameters_alloc();
	if (!st->codecpar)
		goto fail;

	st->internal->avctx = avcodec_alloc_context3(NULL);
	if (!st->internal->avctx)
		goto fail;

	if (s->iformat) {
#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
	/* no default bitrate if decoding */
	st->codec->bit_rate = 0;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

		/* default pts setting is MPEG-like */
		avpriv_set_pts_info(st, 33, 1, 90000);
		/* we set the current DTS to 0 so that formats without any timestamps
		* but durations get some timestamps, formats with some unknown
		* timestamps have their first few packets buffered and the
		* timestamps corrected before they are returned to the user */
		st->cur_dts = RELATIVE_TS_BASE;
	} else {
		st->cur_dts = AV_NOPTS_VALUE;
	}

	st->index = s->nb_streams;
	st->start_time = AV_NOPTS_VALUE;
	st->duration = AV_NOPTS_VALUE;
	st->first_dts = AV_NOPTS_VALUE;
	st->probe_packets = s->max_probe_packets;
	st->pts_wrap_reference = AV_NOPTS_VALUE;
	st->pts_wrap_behavior = AV_PTS_WRAP_IGNORE;

	st->last_IP_pts = AV_NOPTS_VALUE;
	st->last_dts_for_order_check = AV_NOPTS_VALUE;
	for (i = 0; i < MAX_REORDER_DELAY + 1; i++)
		st->pts_buffer[i] = AV_NOPTS_VALUE;

	st->sample_aspect_ratio = AVRational{ 0, 1 };

#if FF_API_R_FRAME_RATE
	st->info->last_dts = AV_NOPTS_VALUE;
#endif
	st->info->fps_first_dts = AV_NOPTS_VALUE;
	st->info->fps_last_dts = AV_NOPTS_VALUE;

	st->inject_global_side_data = s->internal->inject_global_side_data;

	st->internal->need_context_update = 1;

	s->streams[s->nb_streams++] = st;
	return st;
fail:
	free_stream(&st);
	return NULL;
}

void avpriv_set_pts_info(AVStream *s, int pts_wrap_bits,
	unsigned int pts_num, unsigned int pts_den)
{
	AVRational new_tb;
	if (av_reduce(&new_tb.num, &new_tb.den, pts_num, pts_den, INT_MAX)) {
		if (new_tb.num != pts_num)
			fprintf(stderr, "NULL: st:%d removing common factor %d from timebase\n", s->index, pts_num / new_tb.num);
	}
	else
		fprintf(stderr, "NULL: st:%d has too large timebase, reducing\n", s->index);

	if (new_tb.num <= 0 || new_tb.den <= 0) {
		fprintf(stderr, "NULL: Ignoring attempt to set invalid timebase %d/%d for st:%d\n", new_tb.num, new_tb.den, s->index);
		return;
	}
	s->time_base = new_tb;
#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
	s->codec->pkt_timebase = new_tb;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
	s->internal->avctx->pkt_timebase = new_tb;
	s->pts_wrap_bits = pts_wrap_bits;
}

static void av_format_init_next(void)
{
	AVOutputFormat *prevout = NULL, *out;
	AVInputFormat *previn = NULL, *in;

	ff_mutex_lock(&avpriv_register_devices_mutex);

	for (int i = 0; (out = (AVOutputFormat*)muxer_list[i]); i++) {
		if (prevout)
			prevout->next = out;
		prevout = out;
	}

	if (outdev_list) {
		for (int i = 0; (out = (AVOutputFormat*)outdev_list[i]); i++) {
			if (prevout)
				prevout->next = out;
			prevout = out;
		}
	}

	for (int i = 0; (in = (AVInputFormat*)demuxer_list[i]); i++) {
		if (previn)
			previn->next = in;
		previn = in;
	}

	if (indev_list) {
		for (int i = 0; (in = (AVInputFormat*)indev_list[i]); i++) {
			if (previn)
				previn->next = in;
			previn = in;
		}
	}

	ff_mutex_unlock(&avpriv_register_devices_mutex);
}

void avpriv_register_devices(const AVOutputFormat * const o[], const AVInputFormat * const i[])
{
	ff_mutex_lock(&avpriv_register_devices_mutex);
	outdev_list = o;
	indev_list = i;
	ff_mutex_unlock(&avpriv_register_devices_mutex);
#if FF_API_NEXT
	av_format_init_next();
#endif
}

static const char* format_to_name(void* ptr)
{
	AVFormatContext* fc = (AVFormatContext*)ptr;
	if (fc->iformat) return fc->iformat->name;
	else if (fc->oformat) return fc->oformat->name;
	else return "NULL";
}

static void *format_child_next(void *obj, void *prev)
{
	AVFormatContext *s = static_cast<AVFormatContext *>(obj);
	if (!prev && s->priv_data &&
		((s->iformat && s->iformat->priv_class) ||
		s->oformat && s->oformat->priv_class))
		return s->priv_data;
	if (s->pb && s->pb->av_class && prev != s->pb)
		return s->pb;
	return NULL;
}

static const AVClass *format_child_class_next(const AVClass *prev)
{
	ERROR_POS
	return NULL;
}

static AVClassCategory get_category(void *ptr)
{
	AVFormatContext* s = static_cast<AVFormatContext*>(ptr);
	if (s->iformat) return AV_CLASS_CATEGORY_DEMUXER;
	else           return AV_CLASS_CATEGORY_MUXER;
}

static const AVClass av_format_context_class = {
	"AVFormatContext",
	format_to_name,
	avformat_options,
	LIBAVUTIL_VERSION_INT,
	0, 0,
	format_child_next,
	format_child_class_next,
	AV_CLASS_CATEGORY_MUXER,
	get_category
};

static int io_open_default(AVFormatContext *s, AVIOContext **pb,
	const char *url, int flags, AVDictionary **options)
{
	ERROR_POS
	return -1;
}

static void io_close_default(AVFormatContext *s, AVIOContext *pb)
{
	ERROR_POS
}

static void avformat_get_context_defaults(AVFormatContext *s)
{
	memset(s, 0, sizeof(AVFormatContext));

	s->av_class = &av_format_context_class;

	s->io_open = io_open_default;
	s->io_close = io_close_default;

	av_opt_set_defaults(s);
}

AVFormatContext *avformat_alloc_context(void)
{
	AVFormatContext *ic;
	AVFormatInternal *internal;
	ic = static_cast<AVFormatContext *>(av_malloc(sizeof(AVFormatContext)));
	if (!ic) return ic;

	internal = static_cast<AVFormatInternal *>(av_mallocz(sizeof(*internal)));
	if (!internal) {
		av_free(ic);
		return NULL;
	}
	avformat_get_context_defaults(ic);
	ic->internal = internal;
	ic->internal->offset = AV_NOPTS_VALUE;
	ic->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
	ic->internal->shortest_end = AV_NOPTS_VALUE;

	return ic;
}

const AVInputFormat *av_demuxer_iterate(void **opaque)
{
	static const uintptr_t size = sizeof(demuxer_list) / sizeof(demuxer_list[0]) - 1;
	uintptr_t i = (uintptr_t)*opaque;
	const AVInputFormat *f = NULL;

	if (i < size) {
		f = demuxer_list[i];
	}
	else if (outdev_list) {
		f = indev_list[i - size];
	}

	if (f)
		*opaque = (void*)(i + 1);
	return f;
}

AVInputFormat *av_find_input_format(const char *short_name)
{
	const AVInputFormat *fmt = NULL;
	void *i = 0;
	while ((fmt = av_demuxer_iterate(&i))) {
		if (av_match_name(short_name, fmt->name))
			return (AVInputFormat*)fmt;
	}
	return NULL;
}

/* Open input file and probe the format if necessary. */
static int init_input(AVFormatContext *s, const char *filename, AVDictionary **options)
{
	int ret;
	AVProbeData pd = { filename, NULL, 0 };
	int score = AVPROBE_SCORE_RETRY;

	//if (s->pb) {
	//	s->flags |= AVFMT_FLAG_CUSTOM_IO;
	//	if (!s->iformat)
	//		return av_probe_input_buffer2(s->pb, &s->iformat, filename,
	//		s, 0, s->format_probesize);
	//	else if (s->iformat->flags & AVFMT_NOFILE)
	//		av_log(s, AV_LOG_WARNING, "Custom AVIOContext makes no sense and "
	//		"will be ignored with AVFMT_NOFILE format.\n");
	//	return 0;
	//}

	if ((s->iformat && s->iformat->flags & AVFMT_NOFILE)/* ||
		(!s->iformat && (s->iformat = av_probe_input_format2(&pd, 0, &score)))*/)
		return score;

	//if ((ret = s->io_open(s, &s->pb, filename, AVIO_FLAG_READ | s->avio_flags, options)) < 0)
	//	return ret;

	//if (s->iformat)
	//	return 0;
	//return av_probe_input_buffer2(s->pb, &s->iformat, filename, s, 0, s->format_probesize);
}

int ff_packet_list_put(AVPacketList **packet_buffer, AVPacketList **plast_pktl, AVPacket *pkt, int flags)
{
	AVPacketList *pktl = static_cast<AVPacketList *>(av_mallocz(sizeof(AVPacketList)));
	int ret;

	if (!pktl)
		return AVERROR(ENOMEM);

	if (flags & FF_PACKETLIST_FLAG_REF_PACKET) {
		if ((ret = av_packet_ref(&pktl->pkt, pkt)) < 0) {
			av_free(pktl);
			return ret;
		}
	} else {
		ret = av_packet_make_refcounted(pkt);
		if (ret < 0) {
			av_free(pktl);
			return ret;
		}
		av_packet_move_ref(&pktl->pkt, pkt);
	}

	if (*packet_buffer)
		(*plast_pktl)->next = pktl;
	else
		*packet_buffer = pktl;

	/* Add the packet in the buffered packet list. */
	*plast_pktl = pktl;
	return 0;
}

int avformat_queue_attached_pictures(AVFormatContext *s)
{
	int i, ret;
	for (i = 0; i < s->nb_streams; i++)
	if (s->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC &&
		s->streams[i]->discard < AVDISCARD_ALL) {
		if (s->streams[i]->attached_pic.size <= 0) {
			fprintf(stderr, "AVFormatContext, Attached picture on stream %d has invalid size, ignoring\n", i);
			continue;
		}

		ret = ff_packet_list_put(&s->internal->raw_packet_buffer,
					&s->internal->raw_packet_buffer_end,
					&s->streams[i]->attached_pic,
					FF_PACKETLIST_FLAG_REF_PACKET);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int update_stream_avctx(AVFormatContext *s)
{
	int i, ret;
	for (i = 0; i < s->nb_streams; i++) {
		AVStream *st = s->streams[i];

		if (!st->internal->need_context_update)
			continue;

		/* close parser, because it depends on the codec */
		if (st->parser && st->internal->avctx->codec_id != st->codecpar->codec_id) {
			av_parser_close(st->parser);
			st->parser = NULL;
		}

		/* update internal codec context, for the parser */
		ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
		if (ret < 0)
			return ret;

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
		/* update deprecated public codec context */
		ret = avcodec_parameters_to_context(st->codec, st->codecpar);
		if (ret < 0)
			return ret;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

		st->internal->need_context_update = 0;
	}
	return 0;
}

int avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options)
{
	AVFormatContext *s = *ps;
	int i, ret = 0;
	AVDictionary *tmp = NULL;
	ID3v2ExtraMeta *id3v2_extra_meta = NULL;
	
	if (!s && !(s = avformat_alloc_context()))
		return AVERROR(ENOMEM);
	if (!s->av_class) {
		fprintf(stderr, "NULL: Input context has not been properly allocated by avformat_alloc_context() and is not NULL either\n");
		return AVERROR(EINVAL);
	}
	if (fmt)
		s->iformat = fmt;
	
	if (options)
		av_dict_copy(&tmp, *options, 0);
	
	//if (s->pb) // must be before any goto fail
	//	s->flags |= AVFMT_FLAG_CUSTOM_IO;
	
	if ((ret = av_opt_set_dict(s, &tmp)) < 0)
		goto fail;
	
	if (!(s->url = av_strdup(filename ? filename : ""))) {
		ret = AVERROR(ENOMEM);
		goto fail;
	}
	
#if FF_API_FORMAT_FILENAME
FF_DISABLE_DEPRECATION_WARNINGS
	av_strlcpy(s->filename, filename ? filename : "", sizeof(s->filename));
FF_ENABLE_DEPRECATION_WARNINGS
#endif
	if ((ret = init_input(s, filename, &tmp)) < 0)
		goto fail;
	s->probe_score = ret;
	
	//if (!s->protocol_whitelist && s->pb && s->pb->protocol_whitelist) {
	//	s->protocol_whitelist = av_strdup(s->pb->protocol_whitelist);
	//	if (!s->protocol_whitelist) {
	//		ret = AVERROR(ENOMEM);
	//		goto fail;
	//	}
	//}

	//if (!s->protocol_blacklist && s->pb && s->pb->protocol_blacklist) {
	//	s->protocol_blacklist = av_strdup(s->pb->protocol_blacklist);
	//	if (!s->protocol_blacklist) {
	//		ret = AVERROR(ENOMEM);
	//		goto fail;
	//	}
	//}
	
	//if (s->format_whitelist && av_match_list(s->iformat->name, s->format_whitelist, ',') <= 0) {
	//	fprintf(stderr, "error: AVFormatContext, Format not on whitelist \'%s\'\n", s->format_whitelist);
	//	ret = AVERROR(EINVAL);
	//	goto fail;
	//}
	
	//avio_skip(s->pb, s->skip_initial_bytes);
	
	/* Check filename in case an image number is expected. */
	//if (s->iformat->flags & AVFMT_NEEDNUMBER) {
	//	if (!av_filename_number_test(filename)) {
	//		ret = AVERROR(EINVAL);
	//		goto fail;
	//	}
	//}
	
	s->duration = s->start_time = AV_NOPTS_VALUE;
	
	/* Allocate private data. */
	if (s->iformat->priv_data_size > 0) {
		if (!(s->priv_data = av_mallocz(s->iformat->priv_data_size))) {
			ret = AVERROR(ENOMEM);
			goto fail;
		}
		if (s->iformat->priv_class) {
			*(const AVClass **)s->priv_data = s->iformat->priv_class;
			av_opt_set_defaults(s->priv_data);
			if ((ret = av_opt_set_dict(s->priv_data, &tmp)) < 0)
				goto fail;
		}
	}
	
	/* e.g. AVFMT_NOFILE formats will not have a AVIOContext */
	//if (s->pb)
	//	ff_id3v2_read_dict(s->pb, &s->internal->id3v2_meta, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta);
	
	
	if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->iformat->read_header)
		if ((ret = s->iformat->read_header(s)) < 0)
			goto fail;
	
	if (!s->metadata) {
		s->metadata = s->internal->id3v2_meta;
		s->internal->id3v2_meta = NULL;
	}
	//else if (s->internal->id3v2_meta) {
	//	int level = AV_LOG_WARNING;
	//	if (s->error_recognition & AV_EF_COMPLIANT)
	//		level = AV_LOG_ERROR;
	//	fprintf(stderr, "error: AVFormatContext, Discarding ID3 tags because more suitable tags were found.\n");
	//	av_dict_free(&s->internal->id3v2_meta);
	//	if (s->error_recognition & AV_EF_EXPLODE)
	//		return AVERROR_INVALIDDATA;
	//}
	
	//if (id3v2_extra_meta) {
	//	if (!strcmp(s->iformat->name, "mp3") || !strcmp(s->iformat->name, "aac") ||
	//		!strcmp(s->iformat->name, "tta") || !strcmp(s->iformat->name, "wav")) {
	//		if ((ret = ff_id3v2_parse_apic(s, &id3v2_extra_meta)) < 0)
	//			goto fail;
	//		if ((ret = ff_id3v2_parse_chapters(s, &id3v2_extra_meta)) < 0)
	//			goto fail;
	//		if ((ret = ff_id3v2_parse_priv(s, &id3v2_extra_meta)) < 0)
	//			goto fail;
	//	}
	//	else
	//		fprintf(stderr, "error: AVFormatContext, demuxer does not support additional id3 data, skipping\n");
	//}
	ff_id3v2_free_extra_meta(&id3v2_extra_meta);
	
	if ((ret = avformat_queue_attached_pictures(s)) < 0)
		goto fail;
	
	if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->pb && !s->internal->data_offset)
		s->internal->data_offset = avio_tell(s->pb);
	
	s->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
	
	update_stream_avctx(s);
	
	for (i = 0; i < s->nb_streams; i++)
		s->streams[i]->internal->orig_codec_id = s->streams[i]->codecpar->codec_id;
	
	if (options) {
		av_dict_free(options);
		*options = tmp;
	}
	*ps = s;
	return 0;

fail:
	ff_id3v2_free_extra_meta(&id3v2_extra_meta);
	av_dict_free(&tmp);
	//if (s->pb && !(s->flags & AVFMT_FLAG_CUSTOM_IO))
	//	avio_closep(&s->pb);
	avformat_free_context(s);
	*ps = NULL;
	return ret;
}

void ff_free_stream(AVFormatContext *s, AVStream *st)
{
	av_assert0(s->nb_streams>0);
	av_assert0(s->streams[s->nb_streams - 1] == st);

	free_stream(&s->streams[--s->nb_streams]);
}

void ff_packet_list_free(AVPacketList **pkt_buf, AVPacketList **pkt_buf_end)
{
	AVPacketList *tmp = *pkt_buf;

	while (tmp) {
		AVPacketList *pktl = tmp;
		tmp = pktl->next;
		av_packet_unref(&pktl->pkt);
		av_freep(&pktl);
	}
	*pkt_buf = NULL;
	*pkt_buf_end = NULL;
}

/* XXX: suppress the packet queue */
static void flush_packet_queue(AVFormatContext *s)
{
	if (!s->internal)
		return;
	ff_packet_list_free(&s->internal->parse_queue, &s->internal->parse_queue_end);
	ff_packet_list_free(&s->internal->packet_buffer, &s->internal->packet_buffer_end);
	ff_packet_list_free(&s->internal->raw_packet_buffer, &s->internal->raw_packet_buffer_end);

	s->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
}

void avformat_free_context(AVFormatContext *s)
{
	int i;

	if (!s)
		return;

	if (s->oformat && s->oformat->deinit && s->internal->initialized)
		s->oformat->deinit(s);

	av_opt_free(s);
	if (s->iformat && s->iformat->priv_class && s->priv_data)
		av_opt_free(s->priv_data);
	if (s->oformat && s->oformat->priv_class && s->priv_data)
		av_opt_free(s->priv_data);

	for (i = s->nb_streams - 1; i >= 0; i--)
		ff_free_stream(s, s->streams[i]);

	for (i = s->nb_programs - 1; i >= 0; i--) {
		av_dict_free(&s->programs[i]->metadata);
		av_freep(&s->programs[i]->stream_index);
		av_freep(&s->programs[i]);
	}
	av_freep(&s->programs);
	av_freep(&s->priv_data);
	while (s->nb_chapters--) {
		av_dict_free(&s->chapters[s->nb_chapters]->metadata);
		av_freep(&s->chapters[s->nb_chapters]);
	}
	av_freep(&s->chapters);
	av_dict_free(&s->metadata);
	av_dict_free(&s->internal->id3v2_meta);
	av_freep(&s->streams);
	flush_packet_queue(s);
	av_freep(&s->internal);
	av_freep(&s->url);
	av_free(s);
}

int ff_packet_list_get(AVPacketList **pkt_buffer, AVPacketList **pkt_buffer_end, AVPacket *pkt)
{
	AVPacketList *pktl;
	av_assert0(*pkt_buffer);
	pktl = *pkt_buffer;
	*pkt = pktl->pkt;
	*pkt_buffer = pktl->next;
	if (!pktl->next)
		*pkt_buffer_end = NULL;
	av_freep(&pktl);
	return 0;
}

int av_match_ext(const char *filename, const char *extensions)
{
	const char *ext;

	if (!filename)
		return 0;

	ext = strrchr(filename, '.');
	if (ext)
		return av_match_name(ext + 1, extensions);
	return 0;
}

AVInputFormat *av_probe_input_format3(AVProbeData *pd, int is_opened, int *score_ret)
{
	AVProbeData lpd = *pd;
	const AVInputFormat *fmt1 = NULL;
	AVInputFormat *fmt = NULL;
	int score, score_max = 0;
	void *i = 0;
	const static uint8_t zerobuffer[AVPROBE_PADDING_SIZE];
	enum nodat {
		NO_ID3,
		ID3_ALMOST_GREATER_PROBE,
		ID3_GREATER_PROBE,
		ID3_GREATER_MAX_PROBE,
	} nodat = NO_ID3;

	if (!lpd.buf)
		lpd.buf = (unsigned char *)zerobuffer;

	if (lpd.buf_size > 10 && ff_id3v2_match(lpd.buf, ID3v2_DEFAULT_MAGIC)) {
		int id3len = ff_id3v2_tag_len(lpd.buf);
		if (lpd.buf_size > id3len + 16) {
			if (lpd.buf_size < 2LL * id3len + 16)
				nodat = ID3_ALMOST_GREATER_PROBE;
			lpd.buf += id3len;
			lpd.buf_size -= id3len;
		}
		else if (id3len >= PROBE_BUF_MAX) {
			nodat = ID3_GREATER_MAX_PROBE;
		}
		else
			nodat = ID3_GREATER_PROBE;
	}

	while ((fmt1 = av_demuxer_iterate(&i))) {
		if (!is_opened == !(fmt1->flags & AVFMT_NOFILE) && strcmp(fmt1->name, "image2"))
			continue;
		score = 0;
		if (fmt1->read_probe) {
			score = fmt1->read_probe(&lpd);
			if (score) {
				//fprintf(stdout, "AV_LOG_TRACE, Probing %s score:%d size:%d\n", fmt1->name, score, lpd.buf_size);
			}
			if (fmt1->extensions && av_match_ext(lpd.filename, fmt1->extensions)) {
				switch (nodat) {
				case NO_ID3:
					score = FFMAX(score, 1);
					break;
				case ID3_GREATER_PROBE:
				case ID3_ALMOST_GREATER_PROBE:
					score = FFMAX(score, AVPROBE_SCORE_EXTENSION / 2 - 1);
					break;
				case ID3_GREATER_MAX_PROBE:
					score = FFMAX(score, AVPROBE_SCORE_EXTENSION);
					break;
				}
			}
		}
		else if (fmt1->extensions) {
			if (av_match_ext(lpd.filename, fmt1->extensions))
				score = AVPROBE_SCORE_EXTENSION;
		}
		if (av_match_name(lpd.mime_type, fmt1->mime_type)) {
			if (AVPROBE_SCORE_MIME > score) {
				fprintf(stdout, "AV_LOG_DEBUG, Probing %s score:%d increased to %d due to MIME type\n", fmt1->name, score, AVPROBE_SCORE_MIME);
				score = AVPROBE_SCORE_MIME;
			}
		}
		if (score > score_max) {
			score_max = score;
			fmt = (AVInputFormat*)fmt1;
		}
		else if (score == score_max)
			fmt = NULL;
	}
	if (nodat == ID3_GREATER_PROBE)
		score_max = FFMIN(AVPROBE_SCORE_EXTENSION / 2 - 1, score_max);
	*score_ret = score_max;

	return fmt;
}

static int set_codec_from_probe_data(AVFormatContext *s, AVStream *st,
	AVProbeData *pd)
{
	static const struct {
		const char *name;
		enum AVCodecID id;
		enum AVMediaType type;
	} fmt_id_type[] = {
		{ "aac", AV_CODEC_ID_AAC, AVMEDIA_TYPE_AUDIO },
		{ "ac3", AV_CODEC_ID_AC3, AVMEDIA_TYPE_AUDIO },
		{ "aptx", AV_CODEC_ID_APTX, AVMEDIA_TYPE_AUDIO },
		{ "dts", AV_CODEC_ID_DTS, AVMEDIA_TYPE_AUDIO },
		{ "dvbsub", AV_CODEC_ID_DVB_SUBTITLE, AVMEDIA_TYPE_SUBTITLE },
		{ "dvbtxt", AV_CODEC_ID_DVB_TELETEXT, AVMEDIA_TYPE_SUBTITLE },
		{ "eac3", AV_CODEC_ID_EAC3, AVMEDIA_TYPE_AUDIO },
		{ "h264", AV_CODEC_ID_H264, AVMEDIA_TYPE_VIDEO },
		{ "hevc", AV_CODEC_ID_HEVC, AVMEDIA_TYPE_VIDEO },
		{ "loas", AV_CODEC_ID_AAC_LATM, AVMEDIA_TYPE_AUDIO },
		{ "m4v", AV_CODEC_ID_MPEG4, AVMEDIA_TYPE_VIDEO },
		{ "mjpeg_2000", AV_CODEC_ID_JPEG2000, AVMEDIA_TYPE_VIDEO },
		{ "mp3", AV_CODEC_ID_MP3, AVMEDIA_TYPE_AUDIO },
		{ "mpegvideo", AV_CODEC_ID_MPEG2VIDEO, AVMEDIA_TYPE_VIDEO },
		{ "truehd", AV_CODEC_ID_TRUEHD, AVMEDIA_TYPE_AUDIO },
		{ 0 }
	};
	int score;
	const AVInputFormat *fmt = av_probe_input_format3(pd, 1, &score);

	if (fmt) {
		int i;
		//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, Probe with size=%d, packets=%d detected %s with score=%d\n",
		//	pd->buf_size, s->max_probe_packets - st->probe_packets, fmt->name, score);
		for (i = 0; fmt_id_type[i].name; i++) {
			if (!strcmp(fmt->name, fmt_id_type[i].name)) {
				if (fmt_id_type[i].type != AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
					continue;
				if (st->request_probe > score && st->codecpar->codec_id != fmt_id_type[i].id)
					continue;
				st->codecpar->codec_id = fmt_id_type[i].id;
				st->codecpar->codec_type = fmt_id_type[i].type;
				st->internal->need_context_update = 1;
#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
				st->codec->codec_type = st->codecpar->codec_type;
				st->codec->codec_id = st->codecpar->codec_id;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
				return score;
			}
		}
	}
	return 0;
}

static void force_codec_ids(AVFormatContext *s, AVStream *st)
{
	switch (st->codecpar->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		if (s->video_codec_id)
			st->codecpar->codec_id = s->video_codec_id;
		break;
	case AVMEDIA_TYPE_AUDIO:
		if (s->audio_codec_id)
			st->codecpar->codec_id = s->audio_codec_id;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		if (s->subtitle_codec_id)
			st->codecpar->codec_id = s->subtitle_codec_id;
		break;
	case AVMEDIA_TYPE_DATA:
		if (s->data_codec_id)
			st->codecpar->codec_id = s->data_codec_id;
		break;
	}
}

static int probe_codec(AVFormatContext *s, AVStream *st, const AVPacket *pkt)
{
	if (st->request_probe>0) {
		AVProbeData *pd = &st->probe_data;
		int end;
		//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, probing stream %d pp:%d\n", st->index, st->probe_packets);
		--st->probe_packets;

		if (pkt) {
			uint8_t *new_buf = static_cast<uint8_t*>(av_realloc(pd->buf, pd->buf_size + pkt->size + AVPROBE_PADDING_SIZE));
			if (!new_buf) {
				fprintf(stdout, "AV_LOG_WARNING, AVFormatContext, Failed to reallocate probe buffer for stream %d\n", st->index);
				goto no_packet;
			}
			pd->buf = new_buf;
			memcpy(pd->buf + pd->buf_size, pkt->data, pkt->size);
			pd->buf_size += pkt->size;
			memset(pd->buf + pd->buf_size, 0, AVPROBE_PADDING_SIZE);
		}
		else {
		no_packet:
			st->probe_packets = 0;
			if (!pd->buf_size) {
				fprintf(stdout, "AV_LOG_WARNING, AVFormatContext, nothing to probe for stream %d\n", st->index);
			}
		}

		end = s->internal->raw_packet_buffer_remaining_size <= 0
			|| st->probe_packets <= 0;

		if (end || av_log2(pd->buf_size) != av_log2(pd->buf_size - pkt->size)) {
			int score = set_codec_from_probe_data(s, st, pd);
			if ((st->codecpar->codec_id != AV_CODEC_ID_NONE && score > AVPROBE_SCORE_STREAM_RETRY)
				|| end) {
				pd->buf_size = 0;
				av_freep(&pd->buf);
				st->request_probe = -1;
				if (st->codecpar->codec_id != AV_CODEC_ID_NONE) {
					//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, probed stream %d\n", st->index);
				}
				else
					fprintf(stdout, "AV_LOG_WARNING, AVFormatContext, probed stream %d failed\n", st->index);
			}
			force_codec_ids(s, st);
		}
	}
	return 0;
}

AVProgram *av_find_program_from_stream(AVFormatContext *ic, AVProgram *last, int s)
{
	int i, j;

	for (i = 0; i < ic->nb_programs; i++) {
		if (ic->programs[i] == last) {
			last = NULL;
		}
		else {
			if (!last)
			for (j = 0; j < ic->programs[i]->nb_stream_indexes; j++)
			if (ic->programs[i]->stream_index[j] == s)
				return ic->programs[i];
		}
	}
	return NULL;
}

int av_find_default_stream_index(AVFormatContext *s)
{
	int i;
	AVStream *st;
	int best_stream = 0;
	int best_score = INT_MIN;

	if (s->nb_streams <= 0)
		return -1;
	for (i = 0; i < s->nb_streams; i++) {
		int score = 0;
		st = s->streams[i];
		if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (st->disposition & AV_DISPOSITION_ATTACHED_PIC)
				score -= 400;
			if (st->codecpar->width && st->codecpar->height)
				score += 50;
			score += 25;
		}
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (st->codecpar->sample_rate)
				score += 50;
		}
		if (st->codec_info_nb_frames)
			score += 12;

		if (st->discard != AVDISCARD_ALL)
			score += 200;

		if (score > best_score) {
			best_score = score;
			best_stream = i;
		}
	}
	return best_stream;
}

static int update_wrap_reference(AVFormatContext *s, AVStream *st, int stream_index, AVPacket *pkt)
{
	int64_t ref = pkt->dts;
	int i, pts_wrap_behavior;
	int64_t pts_wrap_reference;
	AVProgram *first_program;

	if (ref == AV_NOPTS_VALUE)
		ref = pkt->pts;
	if (st->pts_wrap_reference != AV_NOPTS_VALUE || st->pts_wrap_bits >= 63 || ref == AV_NOPTS_VALUE || !s->correct_ts_overflow)
		return 0;
	ref &= (1LL << st->pts_wrap_bits) - 1;

	// reference time stamp should be 60 s before first time stamp
	pts_wrap_reference = ref - av_rescale(60, st->time_base.den, st->time_base.num);
	// if first time stamp is not more than 1/8 and 60s before the wrap point, subtract rather than add wrap offset
	pts_wrap_behavior = (ref < (1LL << st->pts_wrap_bits) - (1LL << st->pts_wrap_bits - 3)) ||
		(ref < (1LL << st->pts_wrap_bits) - av_rescale(60, st->time_base.den, st->time_base.num)) ?
	AV_PTS_WRAP_ADD_OFFSET : AV_PTS_WRAP_SUB_OFFSET;

	first_program = av_find_program_from_stream(s, NULL, stream_index);

	if (!first_program) {
		int default_stream_index = av_find_default_stream_index(s);
		if (s->streams[default_stream_index]->pts_wrap_reference == AV_NOPTS_VALUE) {
			for (i = 0; i < s->nb_streams; i++) {
				if (av_find_program_from_stream(s, NULL, i))
					continue;
				s->streams[i]->pts_wrap_reference = pts_wrap_reference;
				s->streams[i]->pts_wrap_behavior = pts_wrap_behavior;
			}
		}
		else {
			st->pts_wrap_reference = s->streams[default_stream_index]->pts_wrap_reference;
			st->pts_wrap_behavior = s->streams[default_stream_index]->pts_wrap_behavior;
		}
	}
	else {
		AVProgram *program = first_program;
		while (program) {
			if (program->pts_wrap_reference != AV_NOPTS_VALUE) {
				pts_wrap_reference = program->pts_wrap_reference;
				pts_wrap_behavior = program->pts_wrap_behavior;
				break;
			}
			program = av_find_program_from_stream(s, program, stream_index);
		}

		// update every program with differing pts_wrap_reference
		program = first_program;
		while (program) {
			if (program->pts_wrap_reference != pts_wrap_reference) {
				for (i = 0; i<program->nb_stream_indexes; i++) {
					s->streams[program->stream_index[i]]->pts_wrap_reference = pts_wrap_reference;
					s->streams[program->stream_index[i]]->pts_wrap_behavior = pts_wrap_behavior;
				}

				program->pts_wrap_reference = pts_wrap_reference;
				program->pts_wrap_behavior = pts_wrap_behavior;
			}
			program = av_find_program_from_stream(s, program, stream_index);
		}
	}
	return 1;
}

#define RELATIVE_TS_BASE (INT64_MAX - (1LL<<48))

static int is_relative(int64_t ts) {
	return ts > (RELATIVE_TS_BASE - (1LL << 48));
}

static int64_t wrap_timestamp(const AVStream *st, int64_t timestamp)
{
	if (st->pts_wrap_behavior != AV_PTS_WRAP_IGNORE &&
		st->pts_wrap_reference != AV_NOPTS_VALUE && timestamp != AV_NOPTS_VALUE) {
		if (st->pts_wrap_behavior == AV_PTS_WRAP_ADD_OFFSET &&
			timestamp < st->pts_wrap_reference)
			return timestamp + (1ULL << st->pts_wrap_bits);
		else if (st->pts_wrap_behavior == AV_PTS_WRAP_SUB_OFFSET &&
			timestamp >= st->pts_wrap_reference)
			return timestamp - (1ULL << st->pts_wrap_bits);
	}
	return timestamp;
}

int ff_read_packet(AVFormatContext *s, AVPacket *pkt)
{
	int ret, i, err;
	AVStream *st;

	pkt->data = NULL;
	pkt->size = 0;
	av_init_packet(pkt);

	for (;;) {
		AVPacketList *pktl = s->internal->raw_packet_buffer;
		const AVPacket *pkt1;

		if (pktl) {
			st = s->streams[pktl->pkt.stream_index];
			if (s->internal->raw_packet_buffer_remaining_size <= 0)
			if ((err = probe_codec(s, st, NULL)) < 0)
				return err;
			if (st->request_probe <= 0) {
				ff_packet_list_get(&s->internal->raw_packet_buffer,
					&s->internal->raw_packet_buffer_end, pkt);
				s->internal->raw_packet_buffer_remaining_size += pkt->size;
				return 0;
			}
		}

		ret = s->iformat->read_packet(s, pkt);
		if (ret < 0) {
			av_packet_unref(pkt);

			/* Some demuxers return FFERROR_REDO when they consume
			data and discard it (ignored streams, junk, extradata).
			We must re-call the demuxer to get the real packet. */
			if (ret == FFERROR_REDO)
				continue;
			if (!pktl || ret == AVERROR(EAGAIN))
				return ret;
			for (i = 0; i < s->nb_streams; i++) {
				st = s->streams[i];
				if (st->probe_packets || st->request_probe > 0)
				if ((err = probe_codec(s, st, NULL)) < 0)
					return err;
				av_assert0(st->request_probe <= 0);
			}
			continue;
		}

		err = av_packet_make_refcounted(pkt);
		if (err < 0) {
			av_packet_unref(pkt);
			return err;
		}

		if ((s->flags & AVFMT_FLAG_DISCARD_CORRUPT) &&
			(pkt->flags & AV_PKT_FLAG_CORRUPT)) {
			fprintf(stderr, "AV_LOG_WARNING, AVFormatContext, Dropped corrupted packet (stream = %d)\n", pkt->stream_index);
			av_packet_unref(pkt);
			continue;
		}

		av_assert0(pkt->stream_index < (unsigned)s->nb_streams &&
			"Invalid stream index.\n");

		st = s->streams[pkt->stream_index];

		if (update_wrap_reference(s, st, pkt->stream_index, pkt) && st->pts_wrap_behavior == AV_PTS_WRAP_SUB_OFFSET) {
			// correct first time stamps to negative values
			if (!is_relative(st->first_dts))
				st->first_dts = wrap_timestamp(st, st->first_dts);
			if (!is_relative(st->start_time))
				st->start_time = wrap_timestamp(st, st->start_time);
			if (!is_relative(st->cur_dts))
				st->cur_dts = wrap_timestamp(st, st->cur_dts);
		}

		pkt->dts = wrap_timestamp(st, pkt->dts);
		pkt->pts = wrap_timestamp(st, pkt->pts);

		force_codec_ids(s, st);

		/* TODO: audio: time filter; video: frame reordering (pts != dts) */
		if (s->use_wallclock_as_timestamps)
			pkt->dts = pkt->pts = av_rescale_q(av_gettime(), AV_TIME_BASE_Q, st->time_base);

		if (!pktl && st->request_probe <= 0)
			return ret;

		err = ff_packet_list_put(&s->internal->raw_packet_buffer,
			&s->internal->raw_packet_buffer_end,
			pkt, 0);
		if (err < 0) {
			av_packet_unref(pkt);
			return err;
		}
		pkt1 = &s->internal->raw_packet_buffer_end->pkt;
		s->internal->raw_packet_buffer_remaining_size -= pkt1->size;

		if ((err = probe_codec(s, st, pkt1)) < 0)
			return err;
	}
}

void ff_compute_frame_duration(AVFormatContext *s, int *pnum, int *pden, AVStream *st,
	AVCodecParserContext *pc, AVPacket *pkt)
{
	AVRational codec_framerate = s->iformat ? st->internal->avctx->framerate :
			av_mul_q(av_inv_q(st->internal->avctx->time_base), AVRational{ 1, st->internal->avctx->ticks_per_frame });
	int frame_size, sample_rate;

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
	if ((!codec_framerate.den || !codec_framerate.num) && st->codec->time_base.den && st->codec->time_base.num)
		codec_framerate = av_mul_q(av_inv_q(st->codec->time_base), AVRational{ 1, st->codec->ticks_per_frame });
FF_ENABLE_DEPRECATION_WARNINGS
#endif

	*pnum = 0;
	*pden = 0;
	switch (st->codecpar->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		if (st->r_frame_rate.num && !pc && s->iformat) {
			*pnum = st->r_frame_rate.den;
			*pden = st->r_frame_rate.num;
		}
		else if (st->time_base.num * 1000LL > st->time_base.den) {
			*pnum = st->time_base.num;
			*pden = st->time_base.den;
		}
		else if (codec_framerate.den * 1000LL > codec_framerate.num) {
			av_assert0(st->internal->avctx->ticks_per_frame);
			av_reduce(pnum, pden,
				codec_framerate.den,
				codec_framerate.num * (int64_t)st->internal->avctx->ticks_per_frame,
				INT_MAX);

			if (pc && pc->repeat_pict) {
				av_assert0(s->iformat); // this may be wrong for interlaced encoding but its not used for that case
				av_reduce(pnum, pden,
					(*pnum) * (1LL + pc->repeat_pict),
					(*pden),
					INT_MAX);
			}
			/* If this codec can be interlaced or progressive then we need
			* a parser to compute duration of a packet. Thus if we have
			* no parser in such case leave duration undefined. */
			if (st->internal->avctx->ticks_per_frame > 1 && !pc)
				*pnum = *pden = 0;
		}
		break;
	case AVMEDIA_TYPE_AUDIO:
		ERROR_POS
		if (st->internal->avctx_inited) {
			//frame_size = av_get_audio_frame_duration(st->internal->avctx, pkt->size);
			sample_rate = st->internal->avctx->sample_rate;
		}
		else {
			//frame_size = av_get_audio_frame_duration2(st->codecpar, pkt->size);
			sample_rate = st->codecpar->sample_rate;
		}
		if (frame_size <= 0 || sample_rate <= 0)
			break;
		*pnum = frame_size;
		*pden = sample_rate;
		break;
	default:
		break;
	}
}

static AVPacketList *get_next_pkt(AVFormatContext *s, AVStream *st, AVPacketList *pktl)
{
	if (pktl->next)
		return pktl->next;
	if (pktl == s->internal->packet_buffer_end)
		return s->internal->parse_queue;
	return NULL;
}

static void update_initial_durations(AVFormatContext *s, AVStream *st,
	int stream_index, int duration)
{
	AVPacketList *pktl = s->internal->packet_buffer ? s->internal->packet_buffer : s->internal->parse_queue;
	int64_t cur_dts = RELATIVE_TS_BASE;

	if (st->first_dts != AV_NOPTS_VALUE) {
		if (st->update_initial_durations_done)
			return;
		st->update_initial_durations_done = 1;
		cur_dts = st->first_dts;
		for (; pktl; pktl = get_next_pkt(s, st, pktl)) {
			if (pktl->pkt.stream_index == stream_index) {
				if (pktl->pkt.pts != pktl->pkt.dts ||
					pktl->pkt.dts != AV_NOPTS_VALUE ||
					pktl->pkt.duration)
					break;
				cur_dts -= duration;
			}
		}
		if (pktl && pktl->pkt.dts != st->first_dts) {
			//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, first_dts %s not matching first dts %s (pts %s, duration %"PRId64") in the queue\n",
			//	av_ts2str(st->first_dts), av_ts2str(pktl->pkt.dts), av_ts2str(pktl->pkt.pts), pktl->pkt.duration);
			return;
		}
		if (!pktl) {
			//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, first_dts %s but no packet with dts in the queue\n", av_ts2str(st->first_dts));
			return;
		}
		pktl = s->internal->packet_buffer ? s->internal->packet_buffer : s->internal->parse_queue;
		st->first_dts = cur_dts;
	}
	else if (st->cur_dts != RELATIVE_TS_BASE)
		return;

	for (; pktl; pktl = get_next_pkt(s, st, pktl)) {
		if (pktl->pkt.stream_index != stream_index)
			continue;
		if ((pktl->pkt.pts == pktl->pkt.dts ||
			pktl->pkt.pts == AV_NOPTS_VALUE) &&
			(pktl->pkt.dts == AV_NOPTS_VALUE ||
			pktl->pkt.dts == st->first_dts ||
			pktl->pkt.dts == RELATIVE_TS_BASE) &&
			!pktl->pkt.duration) {
			pktl->pkt.dts = cur_dts;
			if (!st->internal->avctx->has_b_frames)
				pktl->pkt.pts = cur_dts;
			//if (st->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
			pktl->pkt.duration = duration;
		}
		else
			break;
		cur_dts = pktl->pkt.dts + pktl->pkt.duration;
	}
	if (!pktl)
		st->cur_dts = cur_dts;
}

static int has_decode_delay_been_guessed(AVStream *st)
{
	if (st->codecpar->codec_id != AV_CODEC_ID_H264) return 1;
	if (!st->info) // if we have left find_stream_info then nb_decoded_frames won't increase anymore for stream copy
		return 1;
#if CONFIG_H264_DECODER
	if (st->internal->avctx->has_b_frames &&
		avpriv_h264_has_num_reorder_frames(st->internal->avctx) == st->internal->avctx->has_b_frames)
		return 1;
#endif
	if (st->internal->avctx->has_b_frames<3)
		return st->nb_decoded_frames >= 7;
	else if (st->internal->avctx->has_b_frames<4)
		return st->nb_decoded_frames >= 18;
	else
		return st->nb_decoded_frames >= 20;
}

static int64_t select_from_pts_buffer(AVStream *st, int64_t *pts_buffer, int64_t dts) {
	int onein_oneout = st->codecpar->codec_id != AV_CODEC_ID_H264 &&
		st->codecpar->codec_id != AV_CODEC_ID_HEVC;

	if (!onein_oneout) {
		int delay = st->internal->avctx->has_b_frames;
		int i;

		if (dts == AV_NOPTS_VALUE) {
			int64_t best_score = INT64_MAX;
			for (i = 0; i<delay; i++) {
				if (st->pts_reorder_error_count[i]) {
					int64_t score = st->pts_reorder_error[i] / st->pts_reorder_error_count[i];
					if (score < best_score) {
						best_score = score;
						dts = pts_buffer[i];
					}
				}
			}
		}
		else {
			for (i = 0; i<delay; i++) {
				if (pts_buffer[i] != AV_NOPTS_VALUE) {
					int64_t diff = FFABS(pts_buffer[i] - dts)
						+ (uint64_t)st->pts_reorder_error[i];
					diff = FFMAX(diff, st->pts_reorder_error[i]);
					st->pts_reorder_error[i] = diff;
					st->pts_reorder_error_count[i]++;
					if (st->pts_reorder_error_count[i] > 250) {
						st->pts_reorder_error[i] >>= 1;
						st->pts_reorder_error_count[i] >>= 1;
					}
				}
			}
		}
	}

	if (dts == AV_NOPTS_VALUE)
		dts = pts_buffer[0];

	return dts;
}

static void update_dts_from_pts(AVFormatContext *s, int stream_index,
	AVPacketList *pkt_buffer)
{
	AVStream *st = s->streams[stream_index];
	int delay = st->internal->avctx->has_b_frames;
	int i;

	int64_t pts_buffer[MAX_REORDER_DELAY + 1];

	for (i = 0; i<MAX_REORDER_DELAY + 1; i++)
		pts_buffer[i] = AV_NOPTS_VALUE;

	for (; pkt_buffer; pkt_buffer = get_next_pkt(s, st, pkt_buffer)) {
		if (pkt_buffer->pkt.stream_index != stream_index)
			continue;

		if (pkt_buffer->pkt.pts != AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY) {
			pts_buffer[0] = pkt_buffer->pkt.pts;
			for (i = 0; i<delay && pts_buffer[i] > pts_buffer[i + 1]; i++)
				FFSWAP(int64_t, pts_buffer[i], pts_buffer[i + 1]);

			pkt_buffer->pkt.dts = select_from_pts_buffer(st, pts_buffer, pkt_buffer->pkt.dts);
		}
	}
}

static void update_initial_timestamps(AVFormatContext *s, int stream_index,
	int64_t dts, int64_t pts, AVPacket *pkt)
{
	AVStream *st = s->streams[stream_index];
	AVPacketList *pktl = s->internal->packet_buffer ? s->internal->packet_buffer : s->internal->parse_queue;
	AVPacketList *pktl_it;

	uint64_t shift;

	if (st->first_dts != AV_NOPTS_VALUE ||
		dts == AV_NOPTS_VALUE ||
		st->cur_dts == AV_NOPTS_VALUE ||
		st->cur_dts < INT_MIN + RELATIVE_TS_BASE ||
		is_relative(dts))
		return;

	st->first_dts = dts - (st->cur_dts - RELATIVE_TS_BASE);
	st->cur_dts = dts;
	shift = (uint64_t)st->first_dts - RELATIVE_TS_BASE;

	if (is_relative(pts))
		pts += shift;

	for (pktl_it = pktl; pktl_it; pktl_it = get_next_pkt(s, st, pktl_it)) {
		if (pktl_it->pkt.stream_index != stream_index)
			continue;
		if (is_relative(pktl_it->pkt.pts))
			pktl_it->pkt.pts += shift;

		if (is_relative(pktl_it->pkt.dts))
			pktl_it->pkt.dts += shift;

		if (st->start_time == AV_NOPTS_VALUE && pktl_it->pkt.pts != AV_NOPTS_VALUE) {
			st->start_time = pktl_it->pkt.pts;
			if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
				st->start_time += av_rescale_q(st->skip_samples, AVRational{ 1, st->codecpar->sample_rate }, st->time_base);
		}
	}

	if (has_decode_delay_been_guessed(st)) {
		update_dts_from_pts(s, stream_index, pktl);
	}

	if (st->start_time == AV_NOPTS_VALUE) {
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || !(pkt->flags & AV_PKT_FLAG_DISCARD)) {
			st->start_time = pts;
		}
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && st->codecpar->sample_rate)
			st->start_time += av_rescale_q(st->skip_samples, AVRational{ 1, st->codecpar->sample_rate }, st->time_base);
	}
}

static int is_intra_only(enum AVCodecID id)
{
	const AVCodecDescriptor *d = avcodec_descriptor_get(id);
	if (!d)
		return 0;
	if ((d->type == AVMEDIA_TYPE_VIDEO || d->type == AVMEDIA_TYPE_AUDIO) &&
		!(d->props & AV_CODEC_PROP_INTRA_ONLY))
		return 0;
	return 1;
}

static void compute_pkt_fields(AVFormatContext *s, AVStream *st,
	AVCodecParserContext *pc, AVPacket *pkt,
	int64_t next_dts, int64_t next_pts)
{
	int num, den, presentation_delayed, delay, i;
	int64_t offset;
	AVRational duration;
	int onein_oneout = st->codecpar->codec_id != AV_CODEC_ID_H264 &&
		st->codecpar->codec_id != AV_CODEC_ID_HEVC;

	if (s->flags & AVFMT_FLAG_NOFILLIN)
		return;

	if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && pkt->dts != AV_NOPTS_VALUE) {
		if (pkt->dts == pkt->pts && st->last_dts_for_order_check != AV_NOPTS_VALUE) {
			if (st->last_dts_for_order_check <= pkt->dts) {
				st->dts_ordered++;
			}
			else {
				fprintf(stderr, "st->dts_misordered ? AV_LOG_DEBUG : AV_LOG_WARNING, DTS %lld < %lld out of order\n",
					pkt->dts, st->last_dts_for_order_check);
				st->dts_misordered++;
			}
			if (st->dts_ordered + st->dts_misordered > 250) {
				st->dts_ordered >>= 1;
				st->dts_misordered >>= 1;
			}
		}

		st->last_dts_for_order_check = pkt->dts;
		if (st->dts_ordered < 8 * st->dts_misordered && pkt->dts == pkt->pts)
			pkt->dts = AV_NOPTS_VALUE;
	}

	if ((s->flags & AVFMT_FLAG_IGNDTS) && pkt->pts != AV_NOPTS_VALUE)
		pkt->dts = AV_NOPTS_VALUE;

	if (pc && pc->pict_type == AV_PICTURE_TYPE_B
		&& !st->internal->avctx->has_b_frames)
		//FIXME Set low_delay = 0 when has_b_frames = 1
		st->internal->avctx->has_b_frames = 1;

	/* do we have a video B-frame ? */
	delay = st->internal->avctx->has_b_frames;
	presentation_delayed = 0;

	/* XXX: need has_b_frame, but cannot get it if the codec is
	*  not initialized */
	if (delay &&
		pc && pc->pict_type != AV_PICTURE_TYPE_B)
		presentation_delayed = 1;

	if (pkt->pts != AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE &&
		st->pts_wrap_bits < 63 &&
		pkt->dts - (1LL << (st->pts_wrap_bits - 1)) > pkt->pts) {
		if (is_relative(st->cur_dts) || pkt->dts - (1LL << (st->pts_wrap_bits - 1)) > st->cur_dts) {
			pkt->dts -= 1LL << st->pts_wrap_bits;
		}
		else
			pkt->pts += 1LL << st->pts_wrap_bits;
	}

	/* Some MPEG-2 in MPEG-PS lack dts (issue #171 / input_file.mpg).
	* We take the conservative approach and discard both.
	* Note: If this is misbehaving for an H.264 file, then possibly
	* presentation_delayed is not set correctly. */
	if (delay == 1 && pkt->dts == pkt->pts &&
		pkt->dts != AV_NOPTS_VALUE && presentation_delayed) {
		//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, invalid dts/pts combination %"PRIi64"\n", pkt->dts);
		if (strcmp(s->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2")
			&& strcmp(s->iformat->name, "flv")) // otherwise we discard correct timestamps for vc1-wmapro.ism
			pkt->dts = AV_NOPTS_VALUE;
	}

	duration = av_mul_q(AVRational{ static_cast<int>(pkt->duration), 1 }, st->time_base);
	if (pkt->duration <= 0) {
		ff_compute_frame_duration(s, &num, &den, st, pc, pkt);
		if (den && num) {
			duration = AVRational{ num, den };
			pkt->duration = av_rescale_rnd(1,
				num * (int64_t)st->time_base.den,
				den * (int64_t)st->time_base.num,
				AV_ROUND_DOWN);
		}
	}

	if (pkt->duration > 0 && (s->internal->packet_buffer || s->internal->parse_queue))
		update_initial_durations(s, st, pkt->stream_index, pkt->duration);

	/* Correct timestamps with byte offset if demuxers only have timestamps
	* on packet boundaries */
	if (pc && st->need_parsing == AVSTREAM_PARSE_TIMESTAMPS && pkt->size) {
		/* this will estimate bitrate based on this frame's duration and size */
		offset = av_rescale(pc->offset, pkt->duration, pkt->size);
		if (pkt->pts != AV_NOPTS_VALUE)
			pkt->pts += offset;
		if (pkt->dts != AV_NOPTS_VALUE)
			pkt->dts += offset;
	}

	/* This may be redundant, but it should not hurt. */
	if (pkt->dts != AV_NOPTS_VALUE &&
		pkt->pts != AV_NOPTS_VALUE &&
		pkt->pts > pkt->dts)
		presentation_delayed = 1;

	if (s->debug & FF_FDEBUG_TS) {
		//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, IN delayed:%d pts:%s, dts:%s cur_dts:%s st:%d pc:%p duration:%"PRId64" delay:%d onein_oneout:%d\n",
		//	presentation_delayed, av_ts2str(pkt->pts), av_ts2str(pkt->dts), av_ts2str(st->cur_dts),
		//	pkt->stream_index, pc, pkt->duration, delay, onein_oneout);
	}

	/* Interpolate PTS and DTS if they are not present. We skip H264
	* currently because delay and has_b_frames are not reliably set. */
	if ((delay == 0 || (delay == 1 && pc)) &&
		onein_oneout) {
		if (presentation_delayed) {
			/* DTS = decompression timestamp */
			/* PTS = presentation timestamp */
			if (pkt->dts == AV_NOPTS_VALUE)
				pkt->dts = st->last_IP_pts;
			update_initial_timestamps(s, pkt->stream_index, pkt->dts, pkt->pts, pkt);
			if (pkt->dts == AV_NOPTS_VALUE)
				pkt->dts = st->cur_dts;

			/* This is tricky: the dts must be incremented by the duration
			* of the frame we are displaying, i.e. the last I- or P-frame. */
			if (st->last_IP_duration == 0 && (uint64_t)pkt->duration <= INT32_MAX)
				st->last_IP_duration = pkt->duration;
			if (pkt->dts != AV_NOPTS_VALUE)
				st->cur_dts = pkt->dts + st->last_IP_duration;
			if (pkt->dts != AV_NOPTS_VALUE &&
				pkt->pts == AV_NOPTS_VALUE &&
				st->last_IP_duration > 0 &&
				((uint64_t)st->cur_dts - (uint64_t)next_dts + 1) <= 2 &&
				next_dts != next_pts &&
				next_pts != AV_NOPTS_VALUE)
				pkt->pts = next_dts;

			if ((uint64_t)pkt->duration <= INT32_MAX)
				st->last_IP_duration = pkt->duration;
			st->last_IP_pts = pkt->pts;
			/* Cannot compute PTS if not present (we can compute it only
			* by knowing the future. */
		}
		else if (pkt->pts != AV_NOPTS_VALUE ||
			pkt->dts != AV_NOPTS_VALUE ||
			pkt->duration > 0) {

			/* presentation is not delayed : PTS and DTS are the same */
			if (pkt->pts == AV_NOPTS_VALUE)
				pkt->pts = pkt->dts;
			update_initial_timestamps(s, pkt->stream_index, pkt->pts,
				pkt->pts, pkt);
			if (pkt->pts == AV_NOPTS_VALUE)
				pkt->pts = st->cur_dts;
			pkt->dts = pkt->pts;
			if (pkt->pts != AV_NOPTS_VALUE && duration.num >= 0)
				st->cur_dts = av_add_stable(st->time_base, pkt->pts, duration, 1);
		}
	}

	if (pkt->pts != AV_NOPTS_VALUE && delay <= MAX_REORDER_DELAY) {
		st->pts_buffer[0] = pkt->pts;
		for (i = 0; i<delay && st->pts_buffer[i] > st->pts_buffer[i + 1]; i++)
			FFSWAP(int64_t, st->pts_buffer[i], st->pts_buffer[i + 1]);

		if (has_decode_delay_been_guessed(st))
			pkt->dts = select_from_pts_buffer(st, st->pts_buffer, pkt->dts);
	}
	// We skipped it above so we try here.
	if (!onein_oneout)
		// This should happen on the first packet
		update_initial_timestamps(s, pkt->stream_index, pkt->dts, pkt->pts, pkt);
	if (pkt->dts > st->cur_dts)
		st->cur_dts = pkt->dts;

	if (s->debug & FF_FDEBUG_TS) {
		//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, OUTdelayed:%d/%d pts:%s, dts:%s cur_dts:%s st:%d (%d)\n",
		//	presentation_delayed, delay, av_ts2str(pkt->pts), av_ts2str(pkt->dts), av_ts2str(st->cur_dts), st->index, st->id);
	}

	/* update flags */
	if (st->codecpar->codec_type == AVMEDIA_TYPE_DATA || is_intra_only(st->codecpar->codec_id))
		pkt->flags |= AV_PKT_FLAG_KEY;
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
	if (pc)
		pkt->convergence_duration = pc->convergence_duration;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
}

static int parse_packet(AVFormatContext *s, AVPacket *pkt,
	int stream_index, int flush)
{
	AVPacket out_pkt;
	AVStream *st = s->streams[stream_index];
	uint8_t *data = pkt->data;
	int size = pkt->size;
	int ret = 0, got_output = flush;

	if (size || flush) {
		av_init_packet(&out_pkt);
	}
	else if (st->parser->flags & PARSER_FLAG_COMPLETE_FRAMES) {
		// preserve 0-size sync packets
		compute_pkt_fields(s, st, st->parser, pkt, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
	}

	while (size > 0 || (flush && got_output)) {
		int len;
		int64_t next_pts = pkt->pts;
		int64_t next_dts = pkt->dts;

		len = av_parser_parse2(st->parser, st->internal->avctx,
			&out_pkt.data, &out_pkt.size, data, size,
			pkt->pts, pkt->dts, pkt->pos);

		pkt->pts = pkt->dts = AV_NOPTS_VALUE;
		pkt->pos = -1;
		/* increment read pointer */
		data += len;
		size -= len;

		got_output = !!out_pkt.size;

		if (!out_pkt.size)
			continue;

		if (pkt->buf && out_pkt.data == pkt->data) {
			/* reference pkt->buf only when out_pkt.data is guaranteed to point
			* to data in it and not in the parser's internal buffer. */
			/* XXX: Ensure this is the case with all parsers when st->parser->flags
			* is PARSER_FLAG_COMPLETE_FRAMES and check for that instead? */
			out_pkt.buf = av_buffer_ref(pkt->buf);
			if (!out_pkt.buf) {
				ret = AVERROR(ENOMEM);
				goto fail;
			}
		}
		else {
			ret = av_packet_make_refcounted(&out_pkt);
			if (ret < 0)
				goto fail;
		}

		if (pkt->side_data) {
			out_pkt.side_data = pkt->side_data;
			out_pkt.side_data_elems = pkt->side_data_elems;
			pkt->side_data = NULL;
			pkt->side_data_elems = 0;
		}

		/* set the duration */
		out_pkt.duration = (st->parser->flags & PARSER_FLAG_COMPLETE_FRAMES) ? pkt->duration : 0;
		if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (st->internal->avctx->sample_rate > 0) {
				out_pkt.duration =
					av_rescale_q_rnd(st->parser->duration,
							AVRational{ 1, st->internal->avctx->sample_rate},
							st->time_base,
							AV_ROUND_DOWN);
			}
		}

		out_pkt.stream_index = st->index;
		out_pkt.pts = st->parser->pts;
		out_pkt.dts = st->parser->dts;
		out_pkt.pos = st->parser->pos;
		out_pkt.flags |= pkt->flags & AV_PKT_FLAG_DISCARD;

		if (st->need_parsing == AVSTREAM_PARSE_FULL_RAW)
			out_pkt.pos = st->parser->frame_offset;

		if (st->parser->key_frame == 1 ||
			(st->parser->key_frame == -1 &&
			st->parser->pict_type == AV_PICTURE_TYPE_I))
			out_pkt.flags |= AV_PKT_FLAG_KEY;

		if (st->parser->key_frame == -1 && st->parser->pict_type == AV_PICTURE_TYPE_NONE && (pkt->flags&AV_PKT_FLAG_KEY))
			out_pkt.flags |= AV_PKT_FLAG_KEY;

		compute_pkt_fields(s, st, st->parser, &out_pkt, next_dts, next_pts);

		ret = ff_packet_list_put(&s->internal->parse_queue,
			&s->internal->parse_queue_end,
			&out_pkt, 0);
		if (ret < 0) {
			av_packet_unref(&out_pkt);
			goto fail;
		}
	}

	/* end of the stream => close and free the parser */
	if (flush) {
		av_parser_close(st->parser);
		st->parser = NULL;
	}

fail:
	av_packet_unref(pkt);
	return ret;
}

static int read_frame_internal(AVFormatContext *s, AVPacket *pkt)
{
	int ret, i, got_packet = 0;
	AVDictionary *metadata = NULL;

	while (!got_packet && !s->internal->parse_queue) {
		AVStream *st;

		/* read next packet */
		ret = ff_read_packet(s, pkt);
		if (ret < 0) {
			if (ret == AVERROR(EAGAIN))
				return ret;
			/* flush the parsers */
			for (i = 0; i < s->nb_streams; i++) {
				st = s->streams[i];
				if (st->parser && st->need_parsing)
					parse_packet(s, pkt, st->index, 1);
			}
			/* all remaining packets are now in parse_queue =>
			* really terminate parsing */
			break;
		}
		ret = 0;
		st = s->streams[pkt->stream_index];

		/* update context if required */
		if (st->internal->need_context_update) {
			if (avcodec_is_open(st->internal->avctx)) {
				//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, Demuxer context update while decoder is open, closing and trying to re-open\n");
				avcodec_close(st->internal->avctx);
				st->info->found_decoder = 0;
			}

			/* close parser, because it depends on the codec */
			if (st->parser && st->internal->avctx->codec_id != st->codecpar->codec_id) {
				av_parser_close(st->parser);
				st->parser = NULL;
			}

			ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
			if (ret < 0) {
				av_packet_unref(pkt);
				return ret;
			}

#if FF_API_LAVF_AVCTX
			FF_DISABLE_DEPRECATION_WARNINGS
				/* update deprecated public codec context */
				ret = avcodec_parameters_to_context(st->codec, st->codecpar);
			if (ret < 0) {
				av_packet_unref(pkt);
				return ret;
			}
			FF_ENABLE_DEPRECATION_WARNINGS
#endif

			st->internal->need_context_update = 0;
		}

		if (pkt->pts != AV_NOPTS_VALUE &&
			pkt->dts != AV_NOPTS_VALUE &&
			pkt->pts < pkt->dts) {
			fprintf(stderr, "AV_LOG_WARNING, AVFormatContext, Invalid timestamps stream=%d, pts=%s, dts=%s, size=%d\n",
				pkt->stream_index, std::to_string(pkt->pts).c_str(), std::to_string(pkt->dts).c_str(), pkt->size);
		}
		if (s->debug & FF_FDEBUG_TS) {
			//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, ff_read_packet stream=%d, pts=%s, dts=%s, size=%d, duration=%"PRId64", flags=%d\n",
			//	pkt->stream_index, av_ts2str(pkt->pts), av_ts2str(pkt->dts), pkt->size, pkt->duration, pkt->flags);
		}

		if (st->need_parsing && !st->parser && !(s->flags & AVFMT_FLAG_NOPARSE)) {
			ERROR_POS
		}

		if (!st->need_parsing || !st->parser) {
			/* no parsing needed: we just output the packet as is */
			compute_pkt_fields(s, st, NULL, pkt, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
			if ((s->iformat->flags & AVFMT_GENERIC_INDEX) &&
				(pkt->flags & AV_PKT_FLAG_KEY) && pkt->dts != AV_NOPTS_VALUE) {
				ERROR_POS
			}
			got_packet = 1;
		}
		else if (st->discard < AVDISCARD_ALL) {
			if ((ret = parse_packet(s, pkt, pkt->stream_index, 0)) < 0)
				return ret;
			st->codecpar->sample_rate = st->internal->avctx->sample_rate;
			st->codecpar->bit_rate = st->internal->avctx->bit_rate;
			st->codecpar->channels = st->internal->avctx->channels;
			st->codecpar->channel_layout = st->internal->avctx->channel_layout;
			st->codecpar->codec_id = st->internal->avctx->codec_id;
		}
		else {
			/* free packet */
			av_packet_unref(pkt);
		}
		if (pkt->flags & AV_PKT_FLAG_KEY)
			st->skip_to_keyframe = 0;
		if (st->skip_to_keyframe) {
			av_packet_unref(pkt);
			got_packet = 0;
		}
	}

	if (!got_packet && s->internal->parse_queue)
		ret = ff_packet_list_get(&s->internal->parse_queue, &s->internal->parse_queue_end, pkt);

	if (ret >= 0) {
		AVStream *st = s->streams[pkt->stream_index];
		int discard_padding = 0;
		if (st->first_discard_sample && pkt->pts != AV_NOPTS_VALUE) {
			ERROR_POS
		}
		if (st->start_skip_samples && (pkt->pts == 0 || pkt->pts == RELATIVE_TS_BASE))
			st->skip_samples = st->start_skip_samples;
		if (st->skip_samples || discard_padding) {
			ERROR_POS
		}

		if (st->inject_global_side_data) {
			ERROR_POS
		}
	}

	av_opt_get_dict_val(s, "metadata", AV_OPT_SEARCH_CHILDREN, &metadata);
	if (metadata) {
		s->event_flags |= AVFMT_EVENT_FLAG_METADATA_UPDATED;
		av_dict_copy(&s->metadata, metadata, 0);
		av_dict_free(&metadata);
		av_opt_set_dict_val(s, "metadata", NULL, AV_OPT_SEARCH_CHILDREN);
	}

#if FF_API_LAVF_AVCTX
	update_stream_avctx(s);
#endif

	if (s->debug & FF_FDEBUG_TS) {
		//fprintf(stdout, "AV_LOG_DEBUG, AVFormatContext, read_frame_internal stream=%d, pts=%s, dts=%s, size=%d, duration=%"PRId64", flags=%d\n",
		//	pkt->stream_index, av_ts2str(pkt->pts), av_ts2str(pkt->dts), pkt->size, pkt->duration, pkt->flags);
	}

	/* A demuxer might have returned EOF because of an IO error, let's
	* propagate this back to the user. */
	if (ret == AVERROR_EOF && s->pb && s->pb->error < 0 && s->pb->error != AVERROR(EAGAIN))
		ret = s->pb->error;

	return ret;
}

void ff_reduce_index(AVFormatContext *s, int stream_index)
{
	AVStream *st = s->streams[stream_index];
	unsigned int max_entries = s->max_index_size / sizeof(AVIndexEntry);

	if ((unsigned)st->nb_index_entries >= max_entries) {
		int i;
		for (i = 0; 2 * i < st->nb_index_entries; i++)
			st->index_entries[i] = st->index_entries[2 * i];
		st->nb_index_entries = i;
	}
}

int ff_index_search_timestamp(const AVIndexEntry *entries, int nb_entries,
	int64_t wanted_timestamp, int flags)
{
	int a, b, m;
	int64_t timestamp;

	a = -1;
	b = nb_entries;

	// Optimize appending index entries at the end.
	if (b && entries[b - 1].timestamp < wanted_timestamp)
		a = b - 1;

	while (b - a > 1) {
		m = (a + b) >> 1;

		// Search for the next non-discarded packet.
		while ((entries[m].flags & AVINDEX_DISCARD_FRAME) && m < b && m < nb_entries - 1) {
			m++;
			if (m == b && entries[m].timestamp >= wanted_timestamp) {
				m = b - 1;
				break;
			}
		}

		timestamp = entries[m].timestamp;
		if (timestamp >= wanted_timestamp)
			b = m;
		if (timestamp <= wanted_timestamp)
			a = m;
	}
	m = (flags & AVSEEK_FLAG_BACKWARD) ? a : b;

	if (!(flags & AVSEEK_FLAG_ANY))
	while (m >= 0 && m < nb_entries &&
		!(entries[m].flags & AVINDEX_KEYFRAME))
		m += (flags & AVSEEK_FLAG_BACKWARD) ? -1 : 1;

	if (m == nb_entries)
		return -1;
	return m;
}

int ff_add_index_entry(AVIndexEntry **index_entries,
	int *nb_index_entries,
	unsigned int *index_entries_allocated_size,
	int64_t pos, int64_t timestamp,
	int size, int distance, int flags)
{
	AVIndexEntry *entries, *ie;
	int index;

	if ((unsigned)*nb_index_entries + 1 >= UINT_MAX / sizeof(AVIndexEntry))
		return -1;

	if (timestamp == AV_NOPTS_VALUE)
		return AVERROR(EINVAL);

	if (size < 0 || size > 0x3FFFFFFF)
		return AVERROR(EINVAL);

	if (is_relative(timestamp)) //FIXME this maintains previous behavior but we should shift by the correct offset once known
		timestamp -= RELATIVE_TS_BASE;

	entries = static_cast<AVIndexEntry *>(av_fast_realloc(*index_entries,
				index_entries_allocated_size,
				(*nb_index_entries + 1) *
				sizeof(AVIndexEntry)));
	if (!entries)
		return -1;

	*index_entries = entries;

	index = ff_index_search_timestamp(*index_entries, *nb_index_entries,
					timestamp, AVSEEK_FLAG_ANY);

	if (index < 0) {
		index = (*nb_index_entries)++;
		ie = &entries[index];
		av_assert0(index == 0 || ie[-1].timestamp < timestamp);
	}
	else {
		ie = &entries[index];
		if (ie->timestamp != timestamp) {
			if (ie->timestamp <= timestamp)
				return -1;
			memmove(entries + index + 1, entries + index,
				sizeof(AVIndexEntry)* (*nb_index_entries - index));
			(*nb_index_entries)++;
		}
		else if (ie->pos == pos && distance < ie->min_distance)
			// do not reduce the distance
			distance = ie->min_distance;
	}

	ie->pos = pos;
	ie->timestamp = timestamp;
	ie->min_distance = distance;
	ie->size = size;
	ie->flags = flags;

	return index;
}

int av_add_index_entry(AVStream *st, int64_t pos, int64_t timestamp,
	int size, int distance, int flags)
{
	timestamp = wrap_timestamp(st, timestamp);
	return ff_add_index_entry(&st->index_entries, &st->nb_index_entries,
				&st->index_entries_allocated_size, pos,
				timestamp, size, distance, flags);
}

int av_read_frame(AVFormatContext *s, AVPacket *pkt)
{
	const int genpts = s->flags & AVFMT_FLAG_GENPTS;
	int eof = 0;
	int ret;
	AVStream *st;

	if (!genpts) {
		ret = s->internal->packet_buffer ? ff_packet_list_get(&s->internal->packet_buffer, &s->internal->packet_buffer_end, pkt) : read_frame_internal(s, pkt);
		if (ret < 0)
			return ret;
		goto return_packet;
	}

	ERROR_POS

return_packet:
	st = s->streams[pkt->stream_index];
	if ((s->iformat->flags & AVFMT_GENERIC_INDEX) && pkt->flags & AV_PKT_FLAG_KEY) {
		ff_reduce_index(s, st->index);
		av_add_index_entry(st, pkt->pos, pkt->dts, 0, 0, AVINDEX_KEYFRAME);
	}

	if (is_relative(pkt->dts))
		pkt->dts -= RELATIVE_TS_BASE;
	if (is_relative(pkt->pts))
		pkt->pts -= RELATIVE_TS_BASE;

	return ret;
}

void avformat_close_input(AVFormatContext **ps)
{
	AVFormatContext *s;
	AVIOContext *pb;

	if (!ps || !*ps)
		return;

	s = *ps;
	pb = s->pb;

	if ((s->iformat && strcmp(s->iformat->name, "image2") && s->iformat->flags & AVFMT_NOFILE) ||
		(s->flags & AVFMT_FLAG_CUSTOM_IO))
		pb = NULL;

	flush_packet_queue(s);

	if (s->iformat)
		if (s->iformat->read_close)
			s->iformat->read_close(s);

	avformat_free_context(s);

	*ps = NULL;

	avio_close(pb);
}

} // namespace fbc

#endif // _MSC_VER
