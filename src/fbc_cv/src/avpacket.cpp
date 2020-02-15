// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <string.h>
#include "avpacket.hpp"
#include "ffmpeg_common.hpp"
#include "avmem.hpp"
#include "avutil.hpp"

// reference: ffmpeg 4.2
//            libavcodec/avpacket.c

#ifdef _MSC_VER

namespace fbc {

void av_packet_free(AVPacket **pkt)
{
	if (!pkt || !*pkt)
		return;

	av_packet_unref(*pkt);
	av_freep(pkt);
}

void av_packet_unref(AVPacket *pkt)
{
	av_packet_free_side_data(pkt);
	av_buffer_unref(&pkt->buf);
	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;
}

void av_packet_free_side_data(AVPacket *pkt)
{
	int i;
	for (i = 0; i < pkt->side_data_elems; i++)
		av_freep(&pkt->side_data[i].data);
	av_freep(&pkt->side_data);
	pkt->side_data_elems = 0;
}

void av_init_packet(AVPacket *pkt)
{
	pkt->pts = AV_NOPTS_VALUE;
	pkt->dts = AV_NOPTS_VALUE;
	pkt->pos = -1;
	pkt->duration = 0;
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
	pkt->convergence_duration = 0;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
	pkt->flags = 0;
	pkt->stream_index = 0;
	pkt->buf = NULL;
	pkt->side_data = NULL;
	pkt->side_data_elems = 0;
}

static int packet_alloc(AVBufferRef **buf, int size)
{
	int ret;
	if (size < 0 || size >= INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
		return AVERROR(EINVAL);

	ret = av_buffer_realloc(buf, size + AV_INPUT_BUFFER_PADDING_SIZE);
	if (ret < 0)
		return ret;

	memset((*buf)->data + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	return 0;
}

int av_new_packet(AVPacket *pkt, int size)
{
	AVBufferRef *buf = NULL;
	int ret = packet_alloc(&buf, size);
	if (ret < 0)
		return ret;

	av_init_packet(pkt);
	pkt->buf = buf;
	pkt->data = buf->data;
	pkt->size = size;

	return 0;
}

int av_packet_copy_props(AVPacket *dst, const AVPacket *src)
{
	int i;

	dst->pts = src->pts;
	dst->dts = src->dts;
	dst->pos = src->pos;
	dst->duration = src->duration;
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
	dst->convergence_duration = src->convergence_duration;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
	dst->flags = src->flags;
	dst->stream_index = src->stream_index;

	dst->side_data = NULL;
	dst->side_data_elems = 0;
	for (i = 0; i < src->side_data_elems; i++) {
		enum AVPacketSideDataType type = src->side_data[i].type;
		int size = src->side_data[i].size;
		uint8_t *src_data = src->side_data[i].data;
		uint8_t *dst_data = av_packet_new_side_data(dst, type, size);

		if (!dst_data) {
			av_packet_free_side_data(dst);
			return AVERROR(ENOMEM);
		}
		memcpy(dst_data, src_data, size);
	}

	return 0;
}

int av_packet_ref(AVPacket *dst, const AVPacket *src)
{
	int ret;

	ret = av_packet_copy_props(dst, src);
	if (ret < 0)
		return ret;

	if (!src->buf) {
		ret = packet_alloc(&dst->buf, src->size);
		if (ret < 0)
			goto fail;
		av_assert1(!src->size || src->data);
		if (src->size)
			memcpy(dst->buf->data, src->data, src->size);

		dst->data = dst->buf->data;
	}
	else {
		dst->buf = av_buffer_ref(src->buf);
		if (!dst->buf) {
			ret = AVERROR(ENOMEM);
			goto fail;
		}
		dst->data = src->data;
	}

	dst->size = src->size;

	return 0;
fail:
	av_packet_free_side_data(dst);
	return ret;
}

uint8_t *av_packet_new_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
	int size)
{
	int ret;
	uint8_t *data;

	if ((unsigned)size > INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
		return NULL;
	data = static_cast<uint8_t*>(av_mallocz(size + AV_INPUT_BUFFER_PADDING_SIZE));
	if (!data)
		return NULL;

	ret = av_packet_add_side_data(pkt, type, data, size);
	if (ret < 0) {
		av_freep(&data);
		return NULL;
	}

	return data;
}

int av_packet_add_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
	uint8_t *data, size_t size)
{
	AVPacketSideData *tmp;
	int i, elems = pkt->side_data_elems;

	for (i = 0; i < elems; i++) {
		AVPacketSideData *sd = &pkt->side_data[i];

		if (sd->type == type) {
			av_free(sd->data);
			sd->data = data;
			sd->size = size;
			return 0;
		}
	}

	if ((unsigned)elems + 1 > AV_PKT_DATA_NB)
		return AVERROR(ERANGE);

	tmp = static_cast<AVPacketSideData*>(av_realloc(pkt->side_data, (elems + 1) * sizeof(*tmp)));
	if (!tmp)
		return AVERROR(ENOMEM);

	pkt->side_data = tmp;
	pkt->side_data[elems].data = data;
	pkt->side_data[elems].size = size;
	pkt->side_data[elems].type = type;
	pkt->side_data_elems++;

	return 0;
}

int av_packet_make_refcounted(AVPacket *pkt)
{
	int ret;

	if (pkt->buf)
		return 0;

	ret = packet_alloc(&pkt->buf, pkt->size);
	if (ret < 0)
		return ret;
	av_assert1(!pkt->size || pkt->data);
	if (pkt->size)
		memcpy(pkt->buf->data, pkt->data, pkt->size);

	pkt->data = pkt->buf->data;

	return 0;
}

void av_packet_move_ref(AVPacket *dst, AVPacket *src)
{
	*dst = *src;
	av_init_packet(src);
	src->data = NULL;
	src->size = 0;
}

} // namespace fbc

#endif // _MSC_VER
