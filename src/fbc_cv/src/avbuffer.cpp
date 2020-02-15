// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "stdatomic.hpp"
#include "avbuffer.hpp"
#include "avmem.hpp"

// reference: ffmpeg 4.2
//            libavutil/buffer.c

#ifdef _MSC_VER

namespace fbc {

static void buffer_replace(AVBufferRef **dst, AVBufferRef **src)
{
	AVBuffer *b;

	b = (*dst)->buffer;

	if (src) {
		**dst = **src;
		av_freep(src);
	}
	else
		av_freep(dst);

	if (atomic_fetch_sub_explicit(&b->refcount, 1, memory_order_acq_rel) == 1) {
		b->free(b->opaque, b->data);
		av_freep(&b);
	}
}

void av_buffer_unref(AVBufferRef **buf)
{
	if (!buf || !*buf)
		return;

	buffer_replace(buf, NULL);
}

void av_buffer_default_free(void *opaque, uint8_t *data)
{
	av_free(data);
}

AVBufferRef *av_buffer_create(uint8_t *data, int size,
	void(*free)(void *opaque, uint8_t *data),
	void *opaque, int flags)
{
	AVBufferRef *ref = NULL;
	AVBuffer    *buf = NULL;

	buf = static_cast<AVBuffer*>(av_mallocz(sizeof(*buf)));
	if (!buf)
		return NULL;

	buf->data = data;
	buf->size = size;
	buf->free = free ? free : av_buffer_default_free;
	buf->opaque = opaque;

	atomic_init(&buf->refcount, 1);

	if (flags & AV_BUFFER_FLAG_READONLY)
		buf->flags |= BUFFER_FLAG_READONLY;

	ref = static_cast<AVBufferRef *>(av_mallocz(sizeof(*ref)));
	if (!ref) {
		av_freep(&buf);
		return NULL;
	}

	ref->buffer = buf;
	ref->data = data;
	ref->size = size;

	return ref;
}

int av_buffer_realloc(AVBufferRef **pbuf, int size)
{
	AVBufferRef *buf = *pbuf;
	uint8_t *tmp;

	if (!buf) {
		/* allocate a new buffer with av_realloc(), so it will be reallocatable
		* later */
		uint8_t *data = static_cast<uint8_t*>(av_realloc(NULL, size));
		if (!data)
			return AVERROR(ENOMEM);

		buf = av_buffer_create(data, size, av_buffer_default_free, NULL, 0);
		if (!buf) {
			av_freep(&data);
			return AVERROR(ENOMEM);
		}

		buf->buffer->flags |= BUFFER_FLAG_REALLOCATABLE;
		*pbuf = buf;

		return 0;
	}
	else if (buf->size == size)
		return 0;

	if (!(buf->buffer->flags & BUFFER_FLAG_REALLOCATABLE) ||
		!av_buffer_is_writable(buf) || buf->data != buf->buffer->data) {
		/* cannot realloc, allocate a new reallocable buffer and copy data */
		AVBufferRef *newx = NULL;

		av_buffer_realloc(&newx, size);
		if (!newx)
			return AVERROR(ENOMEM);

		memcpy(newx->data, buf->data, FFMIN(size, buf->size));

		buffer_replace(pbuf, &newx);
		return 0;
	}

	tmp = static_cast<uint8_t*>(av_realloc(buf->buffer->data, size));
	if (!tmp)
		return AVERROR(ENOMEM);

	buf->buffer->data = buf->data = tmp;
	buf->buffer->size = buf->size = size;
	return 0;
}

int av_buffer_is_writable(const AVBufferRef *buf)
{
	if (buf->buffer->flags & AV_BUFFER_FLAG_READONLY)
		return 0;

	return atomic_load(&buf->buffer->refcount) == 1;
}

AVBufferRef *av_buffer_ref(AVBufferRef *buf)
{
	AVBufferRef *ret = static_cast<AVBufferRef*>(av_mallocz(sizeof(*ret)));

	if (!ret)
		return NULL;

	*ret = *buf;

	atomic_fetch_add_explicit(&buf->buffer->refcount, 1, memory_order_relaxed);

	return ret;
}

} // namespace fbc

#endif // _MSC_VER
