// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <stdio.h>
#include <string.h>
#include "avcodec.hpp"
#include "avpacket.hpp"
#include "avmem.hpp"
#include "avutil.hpp"
#include "avoption.hpp"
#include "avpixdesc.hpp"
#include "ffmpeg_codec_id.hpp"
#include "ffmpeg_pixel_format.hpp"

// reference: ffmpeg 4.2
//            libavcodec/parser.c

#ifdef _MSC_VER

namespace fbc {

const PixelFormatTag ff_raw_pix_fmt_tags[] = { { AV_PIX_FMT_NONE, 0 } };

static const AVCodec *codec_list[] = { NULL };

enum AVPixelFormat avpriv_find_pix_fmt(const PixelFormatTag *tags,
	unsigned int fourcc)
{
	while (tags->pix_fmt >= 0) {
		if (tags->fourcc == fourcc)
			return tags->pix_fmt;
		tags++;
	}
	return AV_PIX_FMT_NONE;
}

const struct PixelFormatTag *avpriv_get_raw_pix_fmt_tags(void)
{
	return ff_raw_pix_fmt_tags;
}

unsigned int avpriv_toupper4(unsigned int x)
{
	return av_toupper(x & 0xFF) +
		(av_toupper((x >> 8) & 0xFF) << 8) +
		(av_toupper((x >> 16) & 0xFF) << 16) +
		((unsigned)av_toupper((x >> 24) & 0xFF) << 24);
}

static enum AVCodecID remap_deprecated_codec_id(enum AVCodecID id)
{
	switch (id){
		//This is for future deprecatec codec ids, its empty since
		//last major bump but will fill up again over time, please don't remove it
	default: return id;
	}
}

const AVCodec *av_codec_iterate(void **opaque)
{
	ERROR_POS
	return nullptr;
}

static AVCodec *find_codec(enum AVCodecID id, int(*x)(const AVCodec *))
{
	ERROR_POS
	abort();
	return nullptr;
}

AVCodec *avcodec_find_decoder(enum AVCodecID id)
{
	ERROR_POS
	abort();
	return nullptr;
}

static const char* context_to_name(void* ptr) {
	AVCodecContext *avc = static_cast<AVCodecContext*>(ptr);

	if (avc && avc->codec && avc->codec->name)
		return avc->codec->name;
	else
		return "NULL";
}

#define AV_CODEC_DEFAULT_BITRATE 200*1000
#define OFFSET(x) offsetof(AVCodecContext,x)
#define V AV_OPT_FLAG_VIDEO_PARAM
#define A AV_OPT_FLAG_AUDIO_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption avcodec_options[] = { NULL };

static void *codec_child_next(void *obj, void *prev)
{
	AVCodecContext *s = static_cast<AVCodecContext *>(obj);
	if (!prev && s->codec && s->codec->priv_class && s->priv_data)
		return s->priv_data;
	return NULL;
}

static const AVClass *codec_child_class_next(const AVClass *prev)
{
	AVCodec *c = NULL;

	/* find the codec that corresponds to prev */
	while (prev && (c = av_codec_next(c)))
	if (c->priv_class == prev)
		break;

	/* find next codec with priv options */
	while (c = av_codec_next(c))
	if (c->priv_class)
		return c->priv_class;
	return NULL;
}

static AVClassCategory get_category(void *ptr)
{
	AVCodecContext* avctx = static_cast<AVCodecContext*>(ptr);
	if (avctx->codec && avctx->codec->decode) return AV_CLASS_CATEGORY_DECODER;
	else                                     return AV_CLASS_CATEGORY_ENCODER;
}

static const AVClass av_codec_context_class = {
	"AVCodecContext",
	context_to_name,
	avcodec_options,
	LIBAVUTIL_VERSION_INT,
	offsetof(AVCodecContext, log_level_offset),
	0,
	codec_child_next,
	codec_child_class_next,
	AV_CLASS_CATEGORY_ENCODER,
	get_category
};

static int init_context_defaults(AVCodecContext *s, const AVCodec *codec)
{
	int flags = 0;
	memset(s, 0, sizeof(AVCodecContext));

	s->av_class = &av_codec_context_class;

	s->codec_type = codec ? codec->type : AVMEDIA_TYPE_UNKNOWN;
	if (codec) {
		s->codec = codec;
		s->codec_id = codec->id;
	}

	if (s->codec_type == AVMEDIA_TYPE_AUDIO)
		flags = AV_OPT_FLAG_AUDIO_PARAM;
	else if (s->codec_type == AVMEDIA_TYPE_VIDEO)
		flags = AV_OPT_FLAG_VIDEO_PARAM;
	else if (s->codec_type == AVMEDIA_TYPE_SUBTITLE)
		flags = AV_OPT_FLAG_SUBTITLE_PARAM;
	av_opt_set_defaults2(s, flags, flags);

	s->time_base = AVRational{ 0, 1 };
	s->framerate = AVRational{ 0, 1 };
	s->pkt_timebase = AVRational{ 0, 1 };
	s->get_buffer2 = avcodec_default_get_buffer2;
	s->get_format = avcodec_default_get_format;
	s->execute = avcodec_default_execute;
	s->execute2 = avcodec_default_execute2;
	s->sample_aspect_ratio = AVRational{ 0, 1 };
	s->pix_fmt = AV_PIX_FMT_NONE;
	s->sw_pix_fmt = AV_PIX_FMT_NONE;
	s->sample_fmt = AV_SAMPLE_FMT_NONE;

	s->reordered_opaque = AV_NOPTS_VALUE;
	if (codec && codec->priv_data_size){
		if (!s->priv_data){
			s->priv_data = av_mallocz(codec->priv_data_size);
			if (!s->priv_data) {
				return AVERROR(ENOMEM);
			}
		}
		if (codec->priv_class){
			*(const AVClass**)s->priv_data = codec->priv_class;
			av_opt_set_defaults(s->priv_data);
		}
	}
	if (codec && codec->defaults) {
		int ret;
		const AVCodecDefault *d = codec->defaults;
		while (d->key) {
			ret = av_opt_set(s, reinterpret_cast<const char*>(d->key), reinterpret_cast<const char*>(d->value), 0);
			av_assert0(ret >= 0);
			d++;
		}
	}
	return 0;
}

enum AVPixelFormat avcodec_default_get_format(struct AVCodecContext *avctx,
	const enum AVPixelFormat *fmt)
{
	ERROR_POS
	abort();

	// Nothing is usable, give up.
	return AV_PIX_FMT_NONE;
}

int avcodec_default_get_buffer2(AVCodecContext *avctx, AVFrame *frame, int flags)
{
	ERROR_POS
	abort();
	return -1;
}

AVCodecContext *avcodec_alloc_context3(const AVCodec *codec)
{
	AVCodecContext *avctx = static_cast<AVCodecContext *>(av_malloc(sizeof(AVCodecContext)));

	if (!avctx)
		return NULL;

	if (init_context_defaults(avctx, codec) < 0) {
		av_free(avctx);
		return NULL;
	}

	return avctx;
}

static void codec_parameters_reset(AVCodecParameters *par)
{
	av_freep(&par->extradata);

	memset(par, 0, sizeof(*par));

	par->codec_type = AVMEDIA_TYPE_UNKNOWN;
	par->codec_id = AV_CODEC_ID_NONE;
	par->format = -1;
	par->field_order = AV_FIELD_UNKNOWN;
	par->color_range = AVCOL_RANGE_UNSPECIFIED;
	par->color_primaries = AVCOL_PRI_UNSPECIFIED;
	par->color_trc = AVCOL_TRC_UNSPECIFIED;
	par->color_space = AVCOL_SPC_UNSPECIFIED;
	par->chroma_location = AVCHROMA_LOC_UNSPECIFIED;
	par->sample_aspect_ratio = AVRational{ 0, 1 };
	par->profile = FF_PROFILE_UNKNOWN;
	par->level = FF_LEVEL_UNKNOWN;
}

AVCodecParameters *avcodec_parameters_alloc(void)
{
	AVCodecParameters *par = static_cast<AVCodecParameters *>(av_mallocz(sizeof(*par)));

	if (!par)
		return NULL;
	codec_parameters_reset(par);
	return par;
}

void av_parser_close(AVCodecParserContext *s)
{
	if (s) {
		if (s->parser->parser_close)
			s->parser->parser_close(s);
		av_freep(&s->priv_data);
		av_free(s);
	}
}

void avcodec_free_context(AVCodecContext **pavctx)
{
	AVCodecContext *avctx = *pavctx;

	if (!avctx)
		return;

	avcodec_close(avctx);

	av_freep(&avctx->extradata);
	av_freep(&avctx->subtitle_header);
	av_freep(&avctx->intra_matrix);
	av_freep(&avctx->inter_matrix);
	av_freep(&avctx->rc_override);

	av_freep(pavctx);
}

void av_bsf_free(AVBSFContext **pctx)
{
	AVBSFContext *ctx;

	if (!pctx || !*pctx)
		return;
	ctx = *pctx;

	if (ctx->filter->close)
		ctx->filter->close(ctx);
	if (ctx->filter->priv_class && ctx->priv_data)
		av_opt_free(ctx->priv_data);

	av_opt_free(ctx);

	if (ctx->internal)
		av_packet_free(&ctx->internal->buffer_pkt);
	av_freep(&ctx->internal);
	av_freep(&ctx->priv_data);

	avcodec_parameters_free(&ctx->par_in);
	avcodec_parameters_free(&ctx->par_out);

	av_freep(pctx);
}

void avcodec_parameters_free(AVCodecParameters **ppar)
{
	AVCodecParameters *par = *ppar;

	if (!par)
		return;
	codec_parameters_reset(par);

	av_freep(ppar);
}

AVCodec *av_codec_next(const AVCodec *c)
{
	ERROR_POS
	abort();
	return nullptr;
}

int avcodec_default_execute(AVCodecContext *c, int(*func)(AVCodecContext *c2, void *arg2), void *arg, int *ret, int count, int size)
{
	ERROR_POS
	abort();
	return 0;
}

int avcodec_default_execute2(AVCodecContext *c, int(*func)(AVCodecContext *c2, void *arg2, int jobnr, int threadnr), void *arg, int *ret, int count)
{
	ERROR_POS
	abort();
	return 0;
}

int avcodec_close(AVCodecContext *avctx)
{
	int i;

	if (!avctx)
		return 0;

	if (avcodec_is_open(avctx)) {
		ERROR_POS
	}

	for (i = 0; i < avctx->nb_coded_side_data; i++)
		av_freep(&avctx->coded_side_data[i].data);
	av_freep(&avctx->coded_side_data);
	avctx->nb_coded_side_data = 0;

	av_buffer_unref(&avctx->hw_frames_ctx);
	av_buffer_unref(&avctx->hw_device_ctx);

	if (avctx->priv_data && avctx->codec && avctx->codec->priv_class)
		av_opt_free(avctx->priv_data);
	av_opt_free(avctx);
	av_freep(&avctx->priv_data);
	if (av_codec_is_encoder(avctx->codec)) {
		av_freep(&avctx->extradata);
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
	av_frame_free(&avctx->coded_frame);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
	}
	avctx->codec = NULL;
	avctx->active_thread_type = 0;

	return 0;
}

int av_codec_is_encoder(const AVCodec *codec)
{
	return codec && (codec->encode_sub || codec->encode2 || codec->send_frame);
}

int avcodec_is_open(AVCodecContext *s)
{
	return !!s->internal;
}

int avcodec_parameters_to_context(AVCodecContext *codec,
	const AVCodecParameters *par)
{
	codec->codec_type = par->codec_type;
	codec->codec_id = par->codec_id;
	codec->codec_tag = par->codec_tag;

	codec->bit_rate = par->bit_rate;
	codec->bits_per_coded_sample = par->bits_per_coded_sample;
	codec->bits_per_raw_sample = par->bits_per_raw_sample;
	codec->profile = par->profile;
	codec->level = par->level;

	switch (par->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		codec->pix_fmt = static_cast<AVPixelFormat>(par->format);
		codec->width = par->width;
		codec->height = par->height;
		codec->field_order = par->field_order;
		codec->color_range = par->color_range;
		codec->color_primaries = par->color_primaries;
		codec->color_trc = par->color_trc;
		codec->colorspace = par->color_space;
		codec->chroma_sample_location = par->chroma_location;
		codec->sample_aspect_ratio = par->sample_aspect_ratio;
		codec->has_b_frames = par->video_delay;
		break;
	case AVMEDIA_TYPE_AUDIO:
		codec->sample_fmt = static_cast<AVSampleFormat>(par->format);
		codec->channel_layout = par->channel_layout;
		codec->channels = par->channels;
		codec->sample_rate = par->sample_rate;
		codec->block_align = par->block_align;
		codec->frame_size = par->frame_size;
		codec->delay =
			codec->initial_padding = par->initial_padding;
		codec->trailing_padding = par->trailing_padding;
		codec->seek_preroll = par->seek_preroll;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		codec->width = par->width;
		codec->height = par->height;
		break;
	}

	if (par->extradata) {
		av_freep(&codec->extradata);
		codec->extradata = static_cast<uint8_t*>(av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
		if (!codec->extradata)
			return AVERROR(ENOMEM);
		memcpy(codec->extradata, par->extradata, par->extradata_size);
		codec->extradata_size = par->extradata_size;
	}

	return 0;
}

#define MT(...) (const char *const[]){ __VA_ARGS__, NULL }

const AVProfile ff_mjpeg_profiles[] = {
	{ FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT, "Baseline" },
	{ FF_PROFILE_MJPEG_HUFFMAN_EXTENDED_SEQUENTIAL_DCT, "Sequential" },
	{ FF_PROFILE_MJPEG_HUFFMAN_PROGRESSIVE_DCT, "Progressive" },
	{ FF_PROFILE_MJPEG_HUFFMAN_LOSSLESS, "Lossless" },
	{ FF_PROFILE_MJPEG_JPEG_LS, "JPEG LS" },
	{ FF_PROFILE_UNKNOWN }
};

static const AVCodecDescriptor codec_descriptors[] = {
	{ AV_CODEC_ID_MJPEG, AVMEDIA_TYPE_VIDEO, "mjpeg", NULL_IF_CONFIG_SMALL("Motion JPEG"), AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, (const char *const *)("image/jpeg"), NULL_IF_CONFIG_SMALL(ff_mjpeg_profiles) }
};

static int descriptor_compare(const void *key, const void *member)
{
	enum AVCodecID id = *(const enum AVCodecID *) key;
	const AVCodecDescriptor *desc = static_cast<const AVCodecDescriptor *>(member);

	return id - desc->id;
}

const AVCodecDescriptor *avcodec_descriptor_get(enum AVCodecID id)
{
	return static_cast<const AVCodecDescriptor *>(bsearch(&id, codec_descriptors, FF_ARRAY_ELEMS(codec_descriptors), sizeof(codec_descriptors[0]), descriptor_compare));
}

void ff_fetch_timestamp(AVCodecParserContext *s, int off, int remove, int fuzzy)
{
	int i;

	if (!fuzzy) {
		s->dts = s->pts = AV_NOPTS_VALUE;
		s->pos = -1;
		s->offset = 0;
	}
	for (i = 0; i < AV_PARSER_PTS_NB; i++) {
		if (s->cur_offset + off >= s->cur_frame_offset[i] &&
			(s->frame_offset < s->cur_frame_offset[i] ||
			(!s->frame_offset && !s->next_frame_offset)) && // first field/frame
			// check disabled since MPEG-TS does not send complete PES packets
			/*s->next_frame_offset + off <*/  s->cur_frame_end[i]){

			if (!fuzzy || s->cur_frame_dts[i] != AV_NOPTS_VALUE) {
				s->dts = s->cur_frame_dts[i];
				s->pts = s->cur_frame_pts[i];
				s->pos = s->cur_frame_pos[i];
				s->offset = s->next_frame_offset - s->cur_frame_offset[i];
			}
			if (remove)
				s->cur_frame_offset[i] = INT64_MAX;
			if (s->cur_offset + off < s->cur_frame_end[i])
				break;
		}
	}
}

int av_parser_parse2(AVCodecParserContext *s, AVCodecContext *avctx,
	uint8_t **poutbuf, int *poutbuf_size,
	const uint8_t *buf, int buf_size,
	int64_t pts, int64_t dts, int64_t pos)
{
	int index, i;
	uint8_t dummy_buf[AV_INPUT_BUFFER_PADDING_SIZE];

	av_assert1(avctx->codec_id != AV_CODEC_ID_NONE);

	/* Parsers only work for the specified codec ids. */
	av_assert1(avctx->codec_id == s->parser->codec_ids[0] ||
		avctx->codec_id == s->parser->codec_ids[1] ||
		avctx->codec_id == s->parser->codec_ids[2] ||
		avctx->codec_id == s->parser->codec_ids[3] ||
		avctx->codec_id == s->parser->codec_ids[4]);

	if (!(s->flags & PARSER_FLAG_FETCHED_OFFSET)) {
		s->next_frame_offset = s->cur_offset = pos;
		s->flags |= PARSER_FLAG_FETCHED_OFFSET;
	}

	if (buf_size == 0) {
		/* padding is always necessary even if EOF, so we add it here */
		memset(dummy_buf, 0, sizeof(dummy_buf));
		buf = dummy_buf;
	}
	else if (s->cur_offset + buf_size != s->cur_frame_end[s->cur_frame_start_index]) { /* skip remainder packets */
		/* add a new packet descriptor */
		i = (s->cur_frame_start_index + 1) & (AV_PARSER_PTS_NB - 1);
		s->cur_frame_start_index = i;
		s->cur_frame_offset[i] = s->cur_offset;
		s->cur_frame_end[i] = s->cur_offset + buf_size;
		s->cur_frame_pts[i] = pts;
		s->cur_frame_dts[i] = dts;
		s->cur_frame_pos[i] = pos;
	}

	if (s->fetch_timestamp) {
		s->fetch_timestamp = 0;
		s->last_pts = s->pts;
		s->last_dts = s->dts;
		s->last_pos = s->pos;
		ff_fetch_timestamp(s, 0, 0, 0);
	}
	/* WARNING: the returned index can be negative */
	index = s->parser->parser_parse(s, avctx, (const uint8_t **)poutbuf,
		poutbuf_size, buf, buf_size);
	av_assert0(index > -0x20000000); // The API does not allow returning AVERROR codes
#define FILL(name) if(s->name > 0 && avctx->name <= 0) avctx->name = s->name
	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		FILL(field_order);
	}

	/* update the file pointer */
	if (*poutbuf_size) {
		/* fill the data for the current frame */
		s->frame_offset = s->next_frame_offset;

		/* offset of the next frame */
		s->next_frame_offset = s->cur_offset + index;
		s->fetch_timestamp = 1;
	}
	if (index < 0)
		index = 0;
	s->cur_offset += index;
	return index;
}

#undef OFFSET
#undef A
#undef V
#undef E

} // namespace fbc

#endif // _MSC_VER
