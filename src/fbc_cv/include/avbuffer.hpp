// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVBUFFER_HPP_
#define FBC_CV_AVBUFFER_HPP_

// reference: ffmpeg 4.2
//            libavutil/buffer_internal.h

#ifdef _MSC_VER
#include "ffmpeg_common.hpp"

namespace fbc {

struct AVBuffer {
	uint8_t *data; /**< data described by this buffer */
	int      size; /**< size of data in bytes */

	/**
	*  number of existing AVBufferRef instances referring to this buffer
	*/
	atomic_uint refcount;

	/**
	* a callback for freeing the data
	*/
	void(*free)(void *opaque, uint8_t *data);

	/**
	* an opaque pointer, to be used by the freeing callback
	*/
	void *opaque;

	/**
	* A combination of BUFFER_FLAG_*
	*/
	int flags;
};

typedef struct AVBufferRef {
	AVBuffer *buffer;

	/**
	* The data buffer. It is considered writable if and only if
	* this is the only reference to the buffer, in which case
	* av_buffer_is_writable() returns 1.
	*/
	uint8_t *data;
	/**
	* Size of data in bytes.
	*/
	int      size;
} AVBufferRef;

void av_buffer_unref(AVBufferRef **buf);
int av_buffer_realloc(AVBufferRef **buf, int size);
AVBufferRef *av_buffer_create(uint8_t *data, int size, void(*free)(void *opaque, uint8_t *data), void *opaque, int flags);
void av_buffer_default_free(void *opaque, uint8_t *data);
int av_buffer_is_writable(const AVBufferRef *buf);
AVBufferRef *av_buffer_ref(AVBufferRef *buf);

#define AV_BUFFER_FLAG_READONLY			(1 << 0)
#define BUFFER_FLAG_READONLY			(1 << 0)
#define BUFFER_FLAG_REALLOCATABLE		(1 << 1)

} // namespace fbc

#endif // _MSC_VER

#endif // FBC_CV_AVBUFFER_HPP_
