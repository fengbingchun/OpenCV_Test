// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "avio.hpp"
#include <stdio.h>
#include <inttypes.h>
#include "averror.hpp"
#include "avutil.hpp"
#include "avmem.hpp"

// reference: ffmpeg 4.2
//            libavformat/aviobuf.c

#ifdef _MSC_VER

namespace fbc {

#define IO_BUFFER_SIZE 32768

int64_t avio_skip(AVIOContext *s, int64_t offset)
{
	return avio_seek(s, offset, SEEK_CUR);
}

static int url_resetbuf(AVIOContext *s, int flags)
{
	av_assert1(flags == AVIO_FLAG_WRITE || flags == AVIO_FLAG_READ);

	if (flags & AVIO_FLAG_WRITE) {
		s->buf_end = s->buffer + s->buffer_size;
		s->write_flag = 1;
	}
	else {
		s->buf_end = s->buffer;
		s->write_flag = 0;
	}
	return 0;
}

int ffio_set_buf_size(AVIOContext *s, int buf_size)
{
	uint8_t *buffer;
	buffer = static_cast<uint8_t*>(av_malloc(buf_size));
	if (!buffer)
		return AVERROR(ENOMEM);

	av_free(s->buffer);
	s->buffer = buffer;
	s->orig_buffer_size =
		s->buffer_size = buf_size;
	s->buf_ptr = s->buf_ptr_max = buffer;
	url_resetbuf(s, s->write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);
	return 0;
}

static int read_packet_wrapper(AVIOContext *s, uint8_t *buf, int size)
{
	int ret;

	if (!s->read_packet)
		return AVERROR(EINVAL);
	ret = s->read_packet(s->opaque, buf, size);
#if FF_API_OLD_AVIO_EOF_0
	if (!ret && !s->max_packet_size) {
		fprintf(stderr, "NULL, Invalid return value 0 for stream protocol\n");
		ret = AVERROR_EOF;
	}
#else
	av_assert2(ret || s->max_packet_size);
#endif
	return ret;
}

static void fill_buffer(AVIOContext *s)
{
	int max_buffer_size = s->max_packet_size ?
		s->max_packet_size : IO_BUFFER_SIZE;
	uint8_t *dst = s->buf_end - s->buffer + max_buffer_size < s->buffer_size ?
		s->buf_end : s->buffer;
	int len = s->buffer_size - (dst - s->buffer);

	/* can't fill the buffer without read_packet, just set EOF if appropriate */
	if (!s->read_packet && s->buf_ptr >= s->buf_end)
		s->eof_reached = 1;

	/* no need to do anything if EOF already reached */
	if (s->eof_reached)
		return;

	if (s->update_checksum && dst == s->buffer) {
		if (s->buf_end > s->checksum_ptr)
			s->checksum = s->update_checksum(s->checksum, s->checksum_ptr,
			s->buf_end - s->checksum_ptr);
		s->checksum_ptr = s->buffer;
	}

	/* make buffer smaller in case it ended up large after probing */
	if (s->read_packet && s->orig_buffer_size && s->buffer_size > s->orig_buffer_size && len >= s->orig_buffer_size) {
		if (dst == s->buffer && s->buf_ptr != dst) {
			int ret = ffio_set_buf_size(s, s->orig_buffer_size);
			if (ret < 0)
				fprintf(stderr, "AVIOContext, Failed to decrease buffer size\n");

			s->checksum_ptr = dst = s->buffer;
		}
		len = s->orig_buffer_size;
	}

	len = read_packet_wrapper(s, dst, len);
	if (len == AVERROR_EOF) {
		/* do not modify buffer if EOF reached so that a seek back can
		be done without rereading data */
		s->eof_reached = 1;
	}
	else if (len < 0) {
		s->eof_reached = 1;
		s->error = len;
	}
	else {
		s->pos += len;
		s->buf_ptr = dst;
		s->buf_end = dst + len;
		s->bytes_read += len;
	}
}

static void writeout(AVIOContext *s, const uint8_t *data, int len)
{
	if (!s->error) {
		int ret = 0;
		if (s->write_data_type)
			ret = s->write_data_type(s->opaque, (uint8_t *)data, len, s->current_type, s->last_time);
		else if (s->write_packet)
			ret = s->write_packet(s->opaque, (uint8_t *)data, len);
		if (ret < 0) {
			s->error = ret;
		}
		else {
			if (s->pos + len > s->written)
				s->written = s->pos + len;
		}
	}
	if (s->current_type == AVIO_DATA_MARKER_SYNC_POINT ||
		s->current_type == AVIO_DATA_MARKER_BOUNDARY_POINT) {
		s->current_type = AVIO_DATA_MARKER_UNKNOWN;
	}
	s->last_time = AV_NOPTS_VALUE;
	s->writeout_count++;
	s->pos += len;
}

static void flush_buffer(AVIOContext *s)
{
	s->buf_ptr_max = FFMAX(s->buf_ptr, s->buf_ptr_max);
	if (s->write_flag && s->buf_ptr_max > s->buffer) {
		writeout(s, s->buffer, s->buf_ptr_max - s->buffer);
		if (s->update_checksum) {
			s->checksum = s->update_checksum(s->checksum, s->checksum_ptr,
				s->buf_ptr_max - s->checksum_ptr);
			s->checksum_ptr = s->buffer;
		}
	}
	s->buf_ptr = s->buf_ptr_max = s->buffer;
	if (!s->write_flag)
		s->buf_end = s->buffer;
}

int64_t avio_seek(AVIOContext *s, int64_t offset, int whence)
{
	int64_t offset1;
	int64_t pos;
	int force = whence & AVSEEK_FORCE;
	int buffer_size;
	int short_seek;
	whence &= ~AVSEEK_FORCE;

	if (!s)
		return AVERROR(EINVAL);

	if ((whence & AVSEEK_SIZE))
		return s->seek ? s->seek(s->opaque, offset, AVSEEK_SIZE) : AVERROR(ENOSYS);

	buffer_size = s->buf_end - s->buffer;
	// pos is the absolute position that the beginning of s->buffer corresponds to in the file
	pos = s->pos - (s->write_flag ? 0 : buffer_size);

	if (whence != SEEK_CUR && whence != SEEK_SET)
		return AVERROR(EINVAL);

	if (whence == SEEK_CUR) {
		offset1 = pos + (s->buf_ptr - s->buffer);
		if (offset == 0)
			return offset1;
		if (offset > INT64_MAX - offset1)
			return AVERROR(EINVAL);
		offset += offset1;
	}
	if (offset < 0)
		return AVERROR(EINVAL);

	if (s->short_seek_get) {
		short_seek = s->short_seek_get(s->opaque);
		/* fallback to default short seek */
		if (short_seek <= 0)
			short_seek = s->short_seek_threshold;
	}
	else
		short_seek = s->short_seek_threshold;

	offset1 = offset - pos; // "offset1" is the relative offset from the beginning of s->buffer
	s->buf_ptr_max = FFMAX(s->buf_ptr_max, s->buf_ptr);
	if ((!s->direct || !s->seek) &&
		offset1 >= 0 && offset1 <= (s->write_flag ? s->buf_ptr_max - s->buffer : buffer_size)) {
		/* can do the seek inside the buffer */
		s->buf_ptr = s->buffer + offset1;
	}
	else if ((!(s->seekable & AVIO_SEEKABLE_NORMAL) ||
		offset1 <= buffer_size + short_seek) &&
		!s->write_flag && offset1 >= 0 &&
		(!s->direct || !s->seek) &&
		(whence != SEEK_END || force)) {
		while (s->pos < offset && !s->eof_reached)
			fill_buffer(s);
		if (s->eof_reached)
			return AVERROR_EOF;
		s->buf_ptr = s->buf_end - (s->pos - offset);
	}
	else if (!s->write_flag && offset1 < 0 && -offset1 < buffer_size>>1 && s->seek && offset > 0) {
		int64_t res;

		pos -= FFMIN(buffer_size >> 1, pos);
		if ((res = s->seek(s->opaque, pos, SEEK_SET)) < 0)
			return res;
		s->buf_end =
			s->buf_ptr = s->buffer;
		s->pos = pos;
		s->eof_reached = 0;
		fill_buffer(s);
		return avio_seek(s, offset, SEEK_SET | force);
	}
	else {
		int64_t res;
		if (s->write_flag) {
			flush_buffer(s);
		}
		if (!s->seek)
			return AVERROR(EPIPE);
		if ((res = s->seek(s->opaque, offset, SEEK_SET)) < 0)
			return res;
		s->seek_count++;
		if (!s->write_flag)
			s->buf_end = s->buffer;
		s->buf_ptr = s->buf_ptr_max = s->buffer;
		s->pos = offset;
	}
	s->eof_reached = 0;
	return offset;
}

int avio_close(AVIOContext *s)
{
	//AVIOInternal *internal;
	//URLContext *h;

	if (!s)
		return 0;

	ERROR_POS
	return -1;
}

} // namespace fbc

#endif // _MSC_VER
