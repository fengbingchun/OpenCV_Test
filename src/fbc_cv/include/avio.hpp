// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVIO_HPP_
#define FBC_CV_AVIO_HPP_

// reference: ffmpeg 4.2
//            libavformat/avio.h

#ifdef _MSC_VER

#include <stdio.h>
#include "ffmpeg_common.hpp"
#include "avclass.hpp"

namespace fbc {

enum AVIODataMarkerType {
	/**
	* Header data; this needs to be present for the stream to be decodeable.
	*/
	AVIO_DATA_MARKER_HEADER,
	/**
	* A point in the output bytestream where a decoder can start decoding
	* (i.e. a keyframe). A demuxer/decoder given the data flagged with
	* AVIO_DATA_MARKER_HEADER, followed by any AVIO_DATA_MARKER_SYNC_POINT,
	* should give decodeable results.
	*/
	AVIO_DATA_MARKER_SYNC_POINT,
	/**
	* A point in the output bytestream where a demuxer can start parsing
	* (for non self synchronizing bytestream formats). That is, any
	* non-keyframe packet start point.
	*/
	AVIO_DATA_MARKER_BOUNDARY_POINT,
	/**
	* This is any, unlabelled data. It can either be a muxer not marking
	* any positions at all, it can be an actual boundary/sync point
	* that the muxer chooses not to mark, or a later part of a packet/fragment
	* that is cut into multiple write callbacks due to limited IO buffer size.
	*/
	AVIO_DATA_MARKER_UNKNOWN,
	/**
	* Trailer data, which doesn't contain actual content, but only for
	* finalizing the output file.
	*/
	AVIO_DATA_MARKER_TRAILER,
	/**
	* A point in the output bytestream where the underlying AVIOContext might
	* flush the buffer depending on latency or buffering requirements. Typically
	* means the end of a packet.
	*/
	AVIO_DATA_MARKER_FLUSH_POINT,
};

typedef struct AVIOContext {
	/**
	* A class for private options.
	*
	* If this AVIOContext is created by avio_open2(), av_class is set and
	* passes the options down to protocols.
	*
	* If this AVIOContext is manually allocated, then av_class may be set by
	* the caller.
	*
	* warning -- this field can be NULL, be sure to not pass this AVIOContext
	* to any av_opt_* functions in that case.
	*/
	const AVClass *av_class;

	/*
	* The following shows the relationship between buffer, buf_ptr,
	* buf_ptr_max, buf_end, buf_size, and pos, when reading and when writing
	* (since AVIOContext is used for both):
	*
	**********************************************************************************
	*                                   READING
	**********************************************************************************
	*
	*                            |              buffer_size              |
	*                            |---------------------------------------|
	*                            |                                       |
	*
	*                         buffer          buf_ptr       buf_end
	*                            +---------------+-----------------------+
	*                            |/ / / / / / / /|/ / / / / / /|         |
	*  read buffer:              |/ / consumed / | to be read /|         |
	*                            |/ / / / / / / /|/ / / / / / /|         |
	*                            +---------------+-----------------------+
	*
	*                                                         pos
	*              +-------------------------------------------+-----------------+
	*  input file: |                                           |                 |
	*              +-------------------------------------------+-----------------+
	*
	*
	**********************************************************************************
	*                                   WRITING
	**********************************************************************************
	*
	*                             |          buffer_size                 |
	*                             |--------------------------------------|
	*                             |                                      |
	*
	*                                                buf_ptr_max
	*                          buffer                 (buf_ptr)       buf_end
	*                             +-----------------------+--------------+
	*                             |/ / / / / / / / / / / /|              |
	*  write buffer:              | / / to be flushed / / |              |
	*                             |/ / / / / / / / / / / /|              |
	*                             +-----------------------+--------------+
	*                               buf_ptr can be in this
	*                               due to a backward seek
	*
	*                            pos
	*               +-------------+----------------------------------------------+
	*  output file: |             |                                              |
	*               +-------------+----------------------------------------------+
	*
	*/
	unsigned char *buffer;  /**< Start of the buffer. */
	int buffer_size;        /**< Maximum buffer size */
	unsigned char *buf_ptr; /**< Current position in the buffer */
	unsigned char *buf_end; /**< End of the data, may be less than
				buffer+buffer_size if the read function returned
				less data than requested, e.g. for streams where
				no more data has been received yet. */
	void *opaque;           /**< A private pointer, passed to the read/write/seek/...
				functions. */
	int(*read_packet)(void *opaque, uint8_t *buf, int buf_size);
	int(*write_packet)(void *opaque, uint8_t *buf, int buf_size);
	int64_t(*seek)(void *opaque, int64_t offset, int whence);
	int64_t pos;            /**< position in the file of the current buffer */
	int eof_reached;        /**< true if was unable to read due to error or eof */
	int write_flag;         /**< true if open for writing */
	int max_packet_size;
	unsigned long checksum;
	unsigned char *checksum_ptr;
	unsigned long(*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
	int error;              /**< contains the error code or 0 if no error happened */
	/**
	* Pause or resume playback for network streaming protocols - e.g. MMS.
	*/
	int(*read_pause)(void *opaque, int pause);
	/**
	* Seek to a given timestamp in stream with the specified stream_index.
	* Needed for some network streaming protocols which don't support seeking
	* to byte position.
	*/
	int64_t(*read_seek)(void *opaque, int stream_index,
		int64_t timestamp, int flags);
	/**
	* A combination of AVIO_SEEKABLE_ flags or 0 when the stream is not seekable.
	*/
	int seekable;

	/**
	* max filesize, used to limit allocations
	* This field is internal to libavformat and access from outside is not allowed.
	*/
	int64_t maxsize;

	/**
	* avio_read and avio_write should if possible be satisfied directly
	* instead of going through a buffer, and avio_seek will always
	* call the underlying seek function directly.
	*/
	int direct;

	/**
	* Bytes read statistic
	* This field is internal to libavformat and access from outside is not allowed.
	*/
	int64_t bytes_read;

	/**
	* seek statistic
	* This field is internal to libavformat and access from outside is not allowed.
	*/
	int seek_count;

	/**
	* writeout statistic
	* This field is internal to libavformat and access from outside is not allowed.
	*/
	int writeout_count;

	/**
	* Original buffer size
	* used internally after probing and ensure seekback to reset the buffer size
	* This field is internal to libavformat and access from outside is not allowed.
	*/
	int orig_buffer_size;

	/**
	* Threshold to favor readahead over seek.
	* This is current internal only, do not use from outside.
	*/
	int short_seek_threshold;

	/**
	* ',' separated list of allowed protocols.
	*/
	const char *protocol_whitelist;

	/**
	* ',' separated list of disallowed protocols.
	*/
	const char *protocol_blacklist;

	/**
	* A callback that is used instead of write_packet.
	*/
	int(*write_data_type)(void *opaque, uint8_t *buf, int buf_size,
	enum AVIODataMarkerType type, int64_t time);
	/**
	* If set, don't call write_data_type separately for AVIO_DATA_MARKER_BOUNDARY_POINT,
	* but ignore them and treat them as AVIO_DATA_MARKER_UNKNOWN (to avoid needlessly
	* small chunks of data returned from the callback).
	*/
	int ignore_boundary_point;

	/**
	* Internal, not meant to be used from outside of AVIOContext.
	*/
	enum AVIODataMarkerType current_type;
	int64_t last_time;

	/**
	* A callback that is used instead of short_seek_threshold.
	* This is current internal only, do not use from outside.
	*/
	int(*short_seek_get)(void *opaque);

	int64_t written;

	/**
	* Maximum reached position before a backward seek in the write buffer,
	* used keeping track of already written data for a later flush.
	*/
	unsigned char *buf_ptr_max;

	/**
	* Try to buffer at least this amount of data before flushing it
	*/
	int min_packet_size;
} AVIOContext;

#define AVSEEK_FORCE 0x20000
#define AVSEEK_SIZE 0x10000
#define AVIO_SEEKABLE_NORMAL (1 << 0)

#define AVIO_FLAG_READ  1                                      /**< read-only */
#define AVIO_FLAG_WRITE 2                                      /**< write-only */
#define AVIO_FLAG_READ_WRITE (AVIO_FLAG_READ|AVIO_FLAG_WRITE)  /**< read-write pseudo flag */

#define AVIO_FLAG_DIRECT 0x8000

int64_t avio_skip(AVIOContext *s, int64_t offset);
int64_t avio_seek(AVIOContext *s, int64_t offset, int whence);

int ffio_set_buf_size(AVIOContext *s, int buf_size);

static av_always_inline int64_t avio_tell(AVIOContext *s)
{
	return avio_seek(s, 0, SEEK_CUR);
}

int avio_close(AVIOContext *s);

} // namespace fbc


#endif // _MSC_VER
#endif // FBC_CV_AVIO_HPP_
