// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVSTREAM_HPP_
#define FBC_CV_AVSTREAM_HPP_

// reference: ffmpeg 4.2
//            libavformat/avformat.h

#ifdef _MSC_VER

#include "ffmpeg_common.hpp"
#include "avformat.hpp"
#include "avcodec.hpp"
#include "avpacket.hpp"

namespace fbc {

enum AVStreamParseType {
	AVSTREAM_PARSE_NONE,
	AVSTREAM_PARSE_FULL,       /**< full parsing and repack */
	AVSTREAM_PARSE_HEADERS,    /**< Only parse headers, do not repack. */
	AVSTREAM_PARSE_TIMESTAMPS, /**< full parsing and interpolation of timestamps for frames not starting on a packet boundary */
	AVSTREAM_PARSE_FULL_ONCE,  /**< full parsing and repack of the first frame only, only implemented for H.264 currently */
	AVSTREAM_PARSE_FULL_RAW,   /**< full parsing and repack with timestamp and position generation by parser for raw
					this assumes that each packet in the file contains no demuxer level headers and
					just codec level data, otherwise position generation would fail */
};

typedef struct FFFrac {
	int64_t val, num, den;
} FFFrac;

struct AVStreamInternal {
	/**
	* Set to 1 if the codec allows reordering, so pts can be different
	* from dts.
	*/
	int reorder;

	/**
	* bitstream filters to run on stream
	* - encoding: Set by muxer using ff_stream_add_bitstream_filter
	* - decoding: unused
	*/
	AVBSFContext **bsfcs;
	int nb_bsfcs;

	/**
	* Whether or not check_bitstream should still be run on each packet
	*/
	int bitstream_checked;

	/**
	* The codec context used by avformat_find_stream_info, the parser, etc.
	*/
	AVCodecContext *avctx;
	/**
	* 1 if avctx has been initialized with the values from the codec parameters
	*/
	int avctx_inited;

	enum AVCodecID orig_codec_id;

	/* the context for extracting extradata in find_stream_info()
	* inited=1/bsf=NULL signals that extracting is not possible (codec not
	* supported) */
	struct {
		AVBSFContext *bsf;
		AVPacket     *pkt;
		int inited;
	} extract_extradata;

	/**
	* Whether the internal avctx needs to be updated from codecpar (after a late change to codecpar)
	*/
	int need_context_update;

	FFFrac *priv_pts;
};

typedef struct AVProbeData {
	const char *filename;
	unsigned char *buf; /**< Buffer must have AVPROBE_PADDING_SIZE of extra allocated bytes filled with zero. */
	int buf_size;       /**< Size of buf except extra allocated bytes */
	const char *mime_type; /**< mime_type, when known. */
} AVProbeData;

typedef struct AVIndexEntry {
	int64_t pos;
	int64_t timestamp;        /**<
				  * Timestamp in AVStream.time_base units, preferably the time from which on correctly decoded frames are available
				  * when seeking to this entry. That means preferable PTS on keyframe based formats.
				  * But demuxers can choose to store a different timestamp, if it is more convenient for the implementation or nothing better
				  * is known
				  */
#define AVINDEX_KEYFRAME 0x0001
#define AVINDEX_DISCARD_FRAME  0x0002    /**
					  * Flag is used to indicate which frame should be discarded after decoding.
					  */
	int flags : 2;
	int size : 30; //Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
	int min_distance;         /**< Minimum distance between this and the previous keyframe, used to avoid unneeded searching. */
} AVIndexEntry;

typedef struct AVStream {
	int index;    /**< stream index in AVFormatContext */
	/**
	* Format-specific stream ID.
	* decoding: set by libavformat
	* encoding: set by the user, replaced by libavformat if left unset
	*/
	int id;
#if FF_API_LAVF_AVCTX
	/**
	* @deprecated use the codecpar struct instead
	*/
	attribute_deprecated
	AVCodecContext *codec;
#endif
	void *priv_data;

	/**
	* This is the fundamental unit of time (in seconds) in terms
	* of which frame timestamps are represented.
	*
	* decoding: set by libavformat
	* encoding: May be set by the caller before avformat_write_header() to
	*           provide a hint to the muxer about the desired timebase. In
	*           avformat_write_header(), the muxer will overwrite this field
	*           with the timebase that will actually be used for the timestamps
	*           written into the file (which may or may not be related to the
	*           user-provided one, depending on the format).
	*/
	AVRational time_base;

	/**
	* Decoding: pts of the first frame of the stream in presentation order, in stream time base.
	* Only set this if you are absolutely 100% sure that the value you set
	* it to really is the pts of the first frame.
	* This may be undefined (AV_NOPTS_VALUE).
	* @note The ASF header does NOT contain a correct start_time the ASF
	* demuxer must NOT set this.
	*/
	int64_t start_time;

	/**
	* Decoding: duration of the stream, in stream time base.
	* If a source file does not specify a duration, but does specify
	* a bitrate, this value will be estimated from bitrate and file size.
	*
	* Encoding: May be set by the caller before avformat_write_header() to
	* provide a hint to the muxer about the estimated duration.
	*/
	int64_t duration;

	int64_t nb_frames;                 ///< number of frames in this stream if known or 0

	int disposition; /**< AV_DISPOSITION_* bit field */

	enum AVDiscard discard; ///< Selects which packets can be discarded at will and do not need to be demuxed.

	/**
	* sample aspect ratio (0 if unknown)
	* - encoding: Set by user.
	* - decoding: Set by libavformat.
	*/
	AVRational sample_aspect_ratio;

	AVDictionary *metadata;

	/**
	* Average framerate
	*
	* - demuxing: May be set by libavformat when creating the stream or in
	*             avformat_find_stream_info().
	* - muxing: May be set by the caller before avformat_write_header().
	*/
	AVRational avg_frame_rate;

	/**
	* For streams with AV_DISPOSITION_ATTACHED_PIC disposition, this packet
	* will contain the attached picture.
	*
	* decoding: set by libavformat, must not be modified by the caller.
	* encoding: unused
	*/
	AVPacket attached_pic;

	/**
	* An array of side data that applies to the whole stream (i.e. the
	* container does not allow it to change between packets).
	*
	* There may be no overlap between the side data in this array and side data
	* in the packets. I.e. a given side data is either exported by the muxer
	* (demuxing) / set by the caller (muxing) in this array, then it never
	* appears in the packets, or the side data is exported / sent through
	* the packets (always in the first packet where the value becomes known or
	* changes), then it does not appear in this array.
	*
	* - demuxing: Set by libavformat when the stream is created.
	* - muxing: May be set by the caller before avformat_write_header().
	*
	* Freed by libavformat in avformat_free_context().
	*
	* @see av_format_inject_global_side_data()
	*/
	AVPacketSideData *side_data;
	/**
	* The number of elements in the AVStream.side_data array.
	*/
	int            nb_side_data;

	/**
	* Flags for the user to detect events happening on the stream. Flags must
	* be cleared by the user once the event has been handled.
	* A combination of AVSTREAM_EVENT_FLAG_*.
	*/
	int event_flags;
#define AVSTREAM_EVENT_FLAG_METADATA_UPDATED 0x0001 ///< The call resulted in updated metadata.

	/**
	* Real base framerate of the stream.
	* This is the lowest framerate with which all timestamps can be
	* represented accurately (it is the least common multiple of all
	* framerates in the stream). Note, this value is just a guess!
	* For example, if the time base is 1/90000 and all frames have either
	* approximately 3600 or 1800 timer ticks, then r_frame_rate will be 50/1.
	*/
	AVRational r_frame_rate;

#if FF_API_LAVF_FFSERVER
	/**
	* String containing pairs of key and values describing recommended encoder configuration.
	* Pairs are separated by ','.
	* Keys are separated from values by '='.
	*
	* @deprecated unused
	*/
	attribute_deprecated
	char *recommended_encoder_configuration;
#endif

	/**
	* Codec parameters associated with this stream. Allocated and freed by
	* libavformat in avformat_new_stream() and avformat_free_context()
	* respectively.
	*
	* - demuxing: filled by libavformat on stream creation or in
	*             avformat_find_stream_info()
	* - muxing: filled by the caller before avformat_write_header()
	*/
	AVCodecParameters *codecpar;

	/*****************************************************************
	* All fields below this line are not part of the public API. They
	* may not be used outside of libavformat and can be changed and
	* removed at will.
	* Internal note: be aware that physically removing these fields
	* will break ABI. Replace removed fields with dummy fields, and
	* add new fields to AVStreamInternal.
	*****************************************************************
	*/

#define MAX_STD_TIMEBASES (30*12+30+3+6)
	/**
	* Stream information used internally by avformat_find_stream_info()
	*/
	typedef struct info_{
		int64_t last_dts;
		int64_t duration_gcd;
		int duration_count;
		int64_t rfps_duration_sum;
		double(*duration_error)[2][MAX_STD_TIMEBASES];
		int64_t codec_info_duration;
		int64_t codec_info_duration_fields;
		int frame_delay_evidence;

		/**
		* 0  -> decoder has not been searched for yet.
		* >0 -> decoder found
		* <0 -> decoder with codec_id == -found_decoder has not been found
		*/
		int found_decoder;

		int64_t last_duration;

		/**
		* Those are used for average framerate estimation.
		*/
		int64_t fps_first_dts;
		int     fps_first_dts_idx;
		int64_t fps_last_dts;
		int     fps_last_dts_idx;

	} info_;
	info_ *info;

	int pts_wrap_bits; /**< number of bits in pts (used for wrapping control) */

	// Timestamp generation support:
	/**
	* Timestamp corresponding to the last dts sync point.
	*
	* Initialized when AVCodecParserContext.dts_sync_point >= 0 and
	* a DTS is received from the underlying container. Otherwise set to
	* AV_NOPTS_VALUE by default.
	*/
	int64_t first_dts;
	int64_t cur_dts;
	int64_t last_IP_pts;
	int last_IP_duration;

	/**
	* Number of packets to buffer for codec probing
	*/
	int probe_packets;

	/**
	* Number of frames that have been demuxed during avformat_find_stream_info()
	*/
	int codec_info_nb_frames;

	/* av_read_frame() support */
	enum AVStreamParseType need_parsing;
	struct AVCodecParserContext *parser;

	/**
	* last packet in packet_buffer for this stream when muxing.
	*/
	struct AVPacketList *last_in_packet_buffer;
	AVProbeData probe_data;
#define MAX_REORDER_DELAY 16
	int64_t pts_buffer[MAX_REORDER_DELAY + 1];

	AVIndexEntry *index_entries; /**< Only used if the format does not
				     support seeking natively. */
	int nb_index_entries;
	unsigned int index_entries_allocated_size;

	/**
	* Stream Identifier
	* This is the MPEG-TS stream identifier +1
	* 0 means unknown
	*/
	int stream_identifier;

	/**
	* Details of the MPEG-TS program which created this stream.
	*/
	int program_num;
	int pmt_version;
	int pmt_stream_idx;

	int64_t interleaver_chunk_size;
	int64_t interleaver_chunk_duration;

	/**
	* stream probing state
	* -1   -> probing finished
	*  0   -> no probing requested
	* rest -> perform probing with request_probe being the minimum score to accept.
	* NOT PART OF PUBLIC API
	*/
	int request_probe;
	/**
	* Indicates that everything up to the next keyframe
	* should be discarded.
	*/
	int skip_to_keyframe;

	/**
	* Number of samples to skip at the start of the frame decoded from the next packet.
	*/
	int skip_samples;

	/**
	* If not 0, the number of samples that should be skipped from the start of
	* the stream (the samples are removed from packets with pts==0, which also
	* assumes negative timestamps do not happen).
	* Intended for use with formats such as mp3 with ad-hoc gapless audio
	* support.
	*/
	int64_t start_skip_samples;

	/**
	* If not 0, the first audio sample that should be discarded from the stream.
	* This is broken by design (needs global sample count), but can't be
	* avoided for broken by design formats such as mp3 with ad-hoc gapless
	* audio support.
	*/
	int64_t first_discard_sample;

	/**
	* The sample after last sample that is intended to be discarded after
	* first_discard_sample. Works on frame boundaries only. Used to prevent
	* early EOF if the gapless info is broken (considered concatenated mp3s).
	*/
	int64_t last_discard_sample;

	/**
	* Number of internally decoded frames, used internally in libavformat, do not access
	* its lifetime differs from info which is why it is not in that structure.
	*/
	int nb_decoded_frames;

	/**
	* Timestamp offset added to timestamps before muxing
	* NOT PART OF PUBLIC API
	*/
	int64_t mux_ts_offset;

	/**
	* Internal data to check for wrapping of the time stamp
	*/
	int64_t pts_wrap_reference;

	/**
	* Options for behavior, when a wrap is detected.
	*
	* Defined by AV_PTS_WRAP_ values.
	*
	* If correction is enabled, there are two possibilities:
	* If the first time stamp is near the wrap point, the wrap offset
	* will be subtracted, which will create negative time stamps.
	* Otherwise the offset will be added.
	*/
	int pts_wrap_behavior;

	/**
	* Internal data to prevent doing update_initial_durations() twice
	*/
	int update_initial_durations_done;

	/**
	* Internal data to generate dts from pts
	*/
	int64_t pts_reorder_error[MAX_REORDER_DELAY + 1];
	uint8_t pts_reorder_error_count[MAX_REORDER_DELAY + 1];

	/**
	* Internal data to analyze DTS and detect faulty mpeg streams
	*/
	int64_t last_dts_for_order_check;
	uint8_t dts_ordered;
	uint8_t dts_misordered;

	/**
	* Internal data to inject global side data
	*/
	int inject_global_side_data;

	/**
	* display aspect ratio (0 if unknown)
	* - encoding: unused
	* - decoding: Set by libavformat to calculate sample_aspect_ratio internally
	*/
	AVRational display_aspect_ratio;

	/**
	* An opaque field for libavformat internal usage.
	* Must not be accessed in any way by callers.
	*/
	AVStreamInternal *internal;
} AVStream;

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVSTREAM_HPP_
