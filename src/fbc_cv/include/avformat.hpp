// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVFORMAT_HPP_
#define FBC_CV_AVFORMAT_HPP_

// reference: ffmpeg 4.2
//            libavformat/avformat.h

#ifdef _MSC_VER

#include "core/fbcdef.hpp"
#include "avframe.hpp"
#include "avclass.hpp"
#include "avformat.hpp"
#include "avio.hpp"
#include "avstream.hpp"
#include "ffmpeg_codec_id.hpp"
#include "averror.hpp"

namespace fbc {

#define AVPROBE_SCORE_RETRY (AVPROBE_SCORE_MAX/4)
#define AVPROBE_SCORE_STREAM_RETRY (AVPROBE_SCORE_MAX/4-1)

#define AVPROBE_SCORE_EXTENSION  50 ///< score for file extension
#define AVPROBE_SCORE_MIME       75 ///< score for file mime type
#define AVPROBE_SCORE_MAX       100 ///< maximum score

#define AVPROBE_PADDING_SIZE 32             ///< extra allocated bytes at the end of the probe buffer

/// Demuxer will use avio_open, no opened file should be provided by the caller.
#define AVFMT_NOFILE        0x0001
#define AVFMT_NEEDNUMBER    0x0002 /**< Needs '%d' in filename. */
#define AVFMT_SHOW_IDS      0x0008 /**< Show format stream IDs numbers. */
#define AVFMT_GLOBALHEADER  0x0040 /**< Format wants global header. */
#define AVFMT_NOTIMESTAMPS  0x0080 /**< Format does not need / have any timestamps. */
#define AVFMT_GENERIC_INDEX 0x0100 /**< Use generic index building code. */
#define AVFMT_TS_DISCONT    0x0200 /**< Format allows timestamp discontinuities. Note, muxers always require valid (monotone) timestamps */
#define AVFMT_VARIABLE_FPS  0x0400 /**< Format allows variable fps. */
#define AVFMT_NODIMENSIONS  0x0800 /**< Format does not need width/height */
#define AVFMT_NOSTREAMS     0x1000 /**< Format does not require any streams */
#define AVFMT_NOBINSEARCH   0x2000 /**< Format does not allow to fall back on binary search via read_timestamp */
#define AVFMT_NOGENSEARCH   0x4000 /**< Format does not allow to fall back on generic search */
#define AVFMT_NO_BYTE_SEEK  0x8000 /**< Format does not allow seeking by bytes */
#define AVFMT_ALLOW_FLUSH  0x10000 /**< Format allows flushing. If not set, the muxer will not receive a NULL packet in the write_packet function. */
#define AVFMT_TS_NONSTRICT 0x20000 /**< Format does not require strictly
					increasing timestamps, but they must
					still be monotonic */
#define AVFMT_TS_NEGATIVE  0x40000 /**< Format allows muxing negative
					timestamps. If not set the timestamp
					will be shifted in av_write_frame and
					av_interleaved_write_frame so they
					start from 0.
					The user or muxer can override this through
					AVFormatContext.avoid_negative_ts
					*/

#define AVFMT_SEEK_TO_PTS   0x4000000 /**< Seeking is based on PTS */

#define AV_DISPOSITION_ATTACHED_PIC      0x0400

#define FF_PACKETLIST_FLAG_REF_PACKET (1 << 0) /**< Create a new reference for the packet instead of
						transferring the ownership of the existing one to the
						list. */

#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX (1 << 20)

#define FFERROR_REDO FFERRTAG('R','E','D','O')

typedef int(*av_format_control_message)(struct AVFormatContext *s, int type, void *data, size_t data_size);

typedef struct AVProgram {
	int            id;
	int            flags;
	enum AVDiscard discard;        ///< selects which program to discard and which to feed to the caller
	unsigned int   *stream_index;
	unsigned int   nb_stream_indexes;
	AVDictionary *metadata;

	int program_num;
	int pmt_pid;
	int pcr_pid;
	int pmt_version;

	/*****************************************************************
	* All fields below this line are not part of the public API. They
	* may not be used outside of libavformat and can be changed and
	* removed at will.
	* New public fields should be added right above.
	*****************************************************************
	*/
	int64_t start_time;
	int64_t end_time;

	int64_t pts_wrap_reference;    ///< reference dts for wrap detection
	int pts_wrap_behavior;         ///< behavior on wrap detection
} AVProgram;

typedef struct AVChapter {
	int id;                 ///< unique ID to identify the chapter
	AVRational time_base;   ///< time base in which the start/end timestamps are specified
	int64_t start, end;     ///< chapter start/end time in time_base units
	AVDictionary *metadata;
} AVChapter;

struct AVFormatInternal {
	/**
	* Number of streams relevant for interleaving.
	* Muxing only.
	*/
	int nb_interleaved_streams;

	/**
	* This buffer is only needed when packets were already buffered but
	* not decoded, for example to get the codec parameters in MPEG
	* streams.
	*/
	struct AVPacketList *packet_buffer;
	struct AVPacketList *packet_buffer_end;

	/* av_seek_frame() support */
	int64_t data_offset; /**< offset of the first packet */

	/**
	* Raw packets from the demuxer, prior to parsing and decoding.
	* This buffer is used for buffering packets until the codec can
	* be identified, as parsing cannot be done without knowing the
	* codec.
	*/
	struct AVPacketList *raw_packet_buffer;
	struct AVPacketList *raw_packet_buffer_end;
	/**
	* Packets split by the parser get queued here.
	*/
	struct AVPacketList *parse_queue;
	struct AVPacketList *parse_queue_end;
	/**
	* Remaining size available for raw_packet_buffer, in bytes.
	*/
#define RAW_PACKET_BUFFER_SIZE 2500000
	int raw_packet_buffer_remaining_size;

	/**
	* Offset to remap timestamps to be non-negative.
	* Expressed in timebase units.
	* @see AVStream.mux_ts_offset
	*/
	int64_t offset;

	/**
	* Timebase for the timestamp offset.
	*/
	AVRational offset_timebase;

#if FF_API_COMPUTE_PKT_FIELDS2
	int missing_ts_warning;
#endif

	int inject_global_side_data;

	int avoid_negative_ts_use_pts;

	/**
	* Timestamp of the end of the shortest stream.
	*/
	int64_t shortest_end;

	/**
	* Whether or not avformat_init_output has already been called
	*/
	int initialized;

	/**
	* Whether or not avformat_init_output fully initialized streams
	*/
	int streams_initialized;

	/**
	* ID3v2 tag useful for MP3 demuxing
	*/
	AVDictionary *id3v2_meta;

	/*
	* Prefer the codec framerate for avg_frame_rate computation.
	*/
	int prefer_codec_framerate;
};

typedef struct AVFormatContext {
	/**
	* A class for logging and @ref avoptions. Set by avformat_alloc_context().
	* Exports (de)muxer private options if they exist.
	*/
	const AVClass *av_class;

	/**
	* The input container format.
	*
	* Demuxing only, set by avformat_open_input().
	*/
	struct AVInputFormat *iformat;

	/**
	* The output container format.
	*
	* Muxing only, must be set by the caller before avformat_write_header().
	*/
	struct AVOutputFormat *oformat;

	/**
	* Format private data. This is an AVOptions-enabled struct
	* if and only if iformat/oformat.priv_class is not NULL.
	*
	* - muxing: set by avformat_write_header()
	* - demuxing: set by avformat_open_input()
	*/
	void *priv_data;

	/**
	* I/O context.
	*
	* - demuxing: either set by the user before avformat_open_input() (then
	*             the user must close it manually) or set by avformat_open_input().
	* - muxing: set by the user before avformat_write_header(). The caller must
	*           take care of closing / freeing the IO context.
	*
	* Do NOT set this field if AVFMT_NOFILE flag is set in
	* iformat/oformat.flags. In such a case, the (de)muxer will handle
	* I/O in some other way and this field will be NULL.
	*/
	AVIOContext *pb;

	/* stream info */
	/**
	* Flags signalling stream properties. A combination of AVFMTCTX_*.
	* Set by libavformat.
	*/
	int ctx_flags;

	/**
	* Number of elements in AVFormatContext.streams.
	*
	* Set by avformat_new_stream(), must not be modified by any other code.
	*/
	unsigned int nb_streams;
	/**
	* A list of all streams in the file. New streams are created with
	* avformat_new_stream().
	*
	* - demuxing: streams are created by libavformat in avformat_open_input().
	*             If AVFMTCTX_NOHEADER is set in ctx_flags, then new streams may also
	*             appear in av_read_frame().
	* - muxing: streams are created by the user before avformat_write_header().
	*
	* Freed by libavformat in avformat_free_context().
	*/
	AVStream **streams;

#if FF_API_FORMAT_FILENAME
	/**
	* input or output filename
	*
	* - demuxing: set by avformat_open_input()
	* - muxing: may be set by the caller before avformat_write_header()
	*
	* @deprecated Use url instead.
	*/
	attribute_deprecated
	char filename[1024];
#endif

	/**
	* input or output URL. Unlike the old filename field, this field has no
	* length restriction.
	*
	* - demuxing: set by avformat_open_input(), initialized to an empty
	*             string if url parameter was NULL in avformat_open_input().
	* - muxing: may be set by the caller before calling avformat_write_header()
	*           (or avformat_init_output() if that is called first) to a string
	*           which is freeable by av_free(). Set to an empty string if it
	*           was NULL in avformat_init_output().
	*
	* Freed by libavformat in avformat_free_context().
	*/
	char *url;

	/**
	* Position of the first frame of the component, in
	* AV_TIME_BASE fractional seconds. NEVER set this value directly:
	* It is deduced from the AVStream values.
	*
	* Demuxing only, set by libavformat.
	*/
	int64_t start_time;

	/**
	* Duration of the stream, in AV_TIME_BASE fractional
	* seconds. Only set this value if you know none of the individual stream
	* durations and also do not set any of them. This is deduced from the
	* AVStream values if not set.
	*
	* Demuxing only, set by libavformat.
	*/
	int64_t duration;

	/**
	* Total stream bitrate in bit/s, 0 if not
	* available. Never set it directly if the file_size and the
	* duration are known as FFmpeg can compute it automatically.
	*/
	int64_t bit_rate;

	unsigned int packet_size;
	int max_delay;

	/**
	* Flags modifying the (de)muxer behaviour. A combination of AVFMT_FLAG_*.
	* Set by the user before avformat_open_input() / avformat_write_header().
	*/
	int flags;
#define AVFMT_FLAG_GENPTS       0x0001 ///< Generate missing pts even if it requires parsing future frames.
#define AVFMT_FLAG_IGNIDX       0x0002 ///< Ignore index.
#define AVFMT_FLAG_NONBLOCK     0x0004 ///< Do not block when reading packets from input.
#define AVFMT_FLAG_IGNDTS       0x0008 ///< Ignore DTS on frames that contain both DTS & PTS
#define AVFMT_FLAG_NOFILLIN     0x0010 ///< Do not infer any values from other values, just return what is stored in the container
#define AVFMT_FLAG_NOPARSE      0x0020 ///< Do not use AVParsers, you also must set AVFMT_FLAG_NOFILLIN as the fillin code works on frames and no parsing -> no frames. Also seeking to frames can not work if parsing to find frame boundaries has been disabled
#define AVFMT_FLAG_NOBUFFER     0x0040 ///< Do not buffer frames when possible
#define AVFMT_FLAG_CUSTOM_IO    0x0080 ///< The caller has supplied a custom AVIOContext, don't avio_close() it.
#define AVFMT_FLAG_DISCARD_CORRUPT  0x0100 ///< Discard frames marked corrupted
#define AVFMT_FLAG_FLUSH_PACKETS    0x0200 ///< Flush the AVIOContext every packet.
	/**
	* When muxing, try to avoid writing any random/volatile data to the output.
	* This includes any random IDs, real-time timestamps/dates, muxer version, etc.
	*
	* This flag is mainly intended for testing.
	*/
#define AVFMT_FLAG_BITEXACT         0x0400
#if FF_API_LAVF_MP4A_LATM
#define AVFMT_FLAG_MP4A_LATM    0x8000 ///< Deprecated, does nothing.
#endif
#define AVFMT_FLAG_SORT_DTS    0x10000 ///< try to interleave outputted packets by dts (using this flag can slow demuxing down)
#define AVFMT_FLAG_PRIV_OPT    0x20000 ///< Enable use of private options by delaying codec open (this could be made default once all code is converted)
#if FF_API_LAVF_KEEPSIDE_FLAG
#define AVFMT_FLAG_KEEP_SIDE_DATA 0x40000 ///< Deprecated, does nothing.
#endif
#define AVFMT_FLAG_FAST_SEEK   0x80000 ///< Enable fast, but inaccurate seeks for some formats
#define AVFMT_FLAG_SHORTEST   0x100000 ///< Stop muxing when the shortest stream stops.
#define AVFMT_FLAG_AUTO_BSF   0x200000 ///< Add bitstream filters as requested by the muxer

	/**
	* Maximum size of the data read from input for determining
	* the input container format.
	* Demuxing only, set by the caller before avformat_open_input().
	*/
	int64_t probesize;

	/**
	* Maximum duration (in AV_TIME_BASE units) of the data read
	* from input in avformat_find_stream_info().
	* Demuxing only, set by the caller before avformat_find_stream_info().
	* Can be set to 0 to let avformat choose using a heuristic.
	*/
	int64_t max_analyze_duration;

	const uint8_t *key;
	int keylen;

	unsigned int nb_programs;
	AVProgram **programs;

	/**
	* Forced video codec_id.
	* Demuxing: Set by user.
	*/
	enum AVCodecID video_codec_id;

	/**
	* Forced audio codec_id.
	* Demuxing: Set by user.
	*/
	enum AVCodecID audio_codec_id;

	/**
	* Forced subtitle codec_id.
	* Demuxing: Set by user.
	*/
	enum AVCodecID subtitle_codec_id;

	/**
	* Maximum amount of memory in bytes to use for the index of each stream.
	* If the index exceeds this size, entries will be discarded as
	* needed to maintain a smaller size. This can lead to slower or less
	* accurate seeking (depends on demuxer).
	* Demuxers for which a full in-memory index is mandatory will ignore
	* this.
	* - muxing: unused
	* - demuxing: set by user
	*/
	unsigned int max_index_size;

	/**
	* Maximum amount of memory in bytes to use for buffering frames
	* obtained from realtime capture devices.
	*/
	unsigned int max_picture_buffer;

	/**
	* Number of chapters in AVChapter array.
	* When muxing, chapters are normally written in the file header,
	* so nb_chapters should normally be initialized before write_header
	* is called. Some muxers (e.g. mov and mkv) can also write chapters
	* in the trailer.  To write chapters in the trailer, nb_chapters
	* must be zero when write_header is called and non-zero when
	* write_trailer is called.
	* - muxing: set by user
	* - demuxing: set by libavformat
	*/
	unsigned int nb_chapters;
	AVChapter **chapters;

	/**
	* Metadata that applies to the whole file.
	*
	* - demuxing: set by libavformat in avformat_open_input()
	* - muxing: may be set by the caller before avformat_write_header()
	*
	* Freed by libavformat in avformat_free_context().
	*/
	AVDictionary *metadata;

	/**
	* Start time of the stream in real world time, in microseconds
	* since the Unix epoch (00:00 1st January 1970). That is, pts=0 in the
	* stream was captured at this real world time.
	* - muxing: Set by the caller before avformat_write_header(). If set to
	*           either 0 or AV_NOPTS_VALUE, then the current wall-time will
	*           be used.
	* - demuxing: Set by libavformat. AV_NOPTS_VALUE if unknown. Note that
	*             the value may become known after some number of frames
	*             have been received.
	*/
	int64_t start_time_realtime;

	/**
	* The number of frames used for determining the framerate in
	* avformat_find_stream_info().
	* Demuxing only, set by the caller before avformat_find_stream_info().
	*/
	int fps_probe_size;

	/**
	* Error recognition; higher values will detect more errors but may
	* misdetect some more or less valid parts as errors.
	* Demuxing only, set by the caller before avformat_open_input().
	*/
	int error_recognition;

	/**
	* Custom interrupt callbacks for the I/O layer.
	*
	* demuxing: set by the user before avformat_open_input().
	* muxing: set by the user before avformat_write_header()
	* (mainly useful for AVFMT_NOFILE formats). The callback
	* should also be passed to avio_open2() if it's used to
	* open the file.
	*/
	//AVIOInterruptCB interrupt_callback;

	/**
	* Flags to enable debugging.
	*/
	int debug;
#define FF_FDEBUG_TS        0x0001

	/**
	* Maximum buffering duration for interleaving.
	*
	* To ensure all the streams are interleaved correctly,
	* av_interleaved_write_frame() will wait until it has at least one packet
	* for each stream before actually writing any packets to the output file.
	* When some streams are "sparse" (i.e. there are large gaps between
	* successive packets), this can result in excessive buffering.
	*
	* This field specifies the maximum difference between the timestamps of the
	* first and the last packet in the muxing queue, above which libavformat
	* will output a packet regardless of whether it has queued a packet for all
	* the streams.
	*
	* Muxing only, set by the caller before avformat_write_header().
	*/
	int64_t max_interleave_delta;

	/**
	* Allow non-standard and experimental extension
	* @see AVCodecContext.strict_std_compliance
	*/
	int strict_std_compliance;

	/**
	* Flags for the user to detect events happening on the file. Flags must
	* be cleared by the user once the event has been handled.
	* A combination of AVFMT_EVENT_FLAG_*.
	*/
	int event_flags;
#define AVFMT_EVENT_FLAG_METADATA_UPDATED 0x0001 ///< The call resulted in updated metadata.

	/**
	* Maximum number of packets to read while waiting for the first timestamp.
	* Decoding only.
	*/
	int max_ts_probe;

	/**
	* Avoid negative timestamps during muxing.
	* Any value of the AVFMT_AVOID_NEG_TS_* constants.
	* Note, this only works when using av_interleaved_write_frame. (interleave_packet_per_dts is in use)
	* - muxing: Set by user
	* - demuxing: unused
	*/
	int avoid_negative_ts;
#define AVFMT_AVOID_NEG_TS_AUTO             -1 ///< Enabled when required by target format
#define AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE 1 ///< Shift timestamps so they are non negative
#define AVFMT_AVOID_NEG_TS_MAKE_ZERO         2 ///< Shift timestamps so that they start at 0

	/**
	* Transport stream id.
	* This will be moved into demuxer private options. Thus no API/ABI compatibility
	*/
	int ts_id;

	/**
	* Audio preload in microseconds.
	* Note, not all formats support this and unpredictable things may happen if it is used when not supported.
	* - encoding: Set by user
	* - decoding: unused
	*/
	int audio_preload;

	/**
	* Max chunk time in microseconds.
	* Note, not all formats support this and unpredictable things may happen if it is used when not supported.
	* - encoding: Set by user
	* - decoding: unused
	*/
	int max_chunk_duration;

	/**
	* Max chunk size in bytes
	* Note, not all formats support this and unpredictable things may happen if it is used when not supported.
	* - encoding: Set by user
	* - decoding: unused
	*/
	int max_chunk_size;

	/**
	* forces the use of wallclock timestamps as pts/dts of packets
	* This has undefined results in the presence of B frames.
	* - encoding: unused
	* - decoding: Set by user
	*/
	int use_wallclock_as_timestamps;

	/**
	* avio flags, used to force AVIO_FLAG_DIRECT.
	* - encoding: unused
	* - decoding: Set by user
	*/
	int avio_flags;

	/**
	* The duration field can be estimated through various ways, and this field can be used
	* to know how the duration was estimated.
	* - encoding: unused
	* - decoding: Read by user
	*/
	enum AVDurationEstimationMethod duration_estimation_method;

	/**
	* Skip initial bytes when opening stream
	* - encoding: unused
	* - decoding: Set by user
	*/
	int64_t skip_initial_bytes;

	/**
	* Correct single timestamp overflows
	* - encoding: unused
	* - decoding: Set by user
	*/
	unsigned int correct_ts_overflow;

	/**
	* Force seeking to any (also non key) frames.
	* - encoding: unused
	* - decoding: Set by user
	*/
	int seek2any;

	/**
	* Flush the I/O context after each packet.
	* - encoding: Set by user
	* - decoding: unused
	*/
	int flush_packets;

	/**
	* format probing score.
	* The maximal score is AVPROBE_SCORE_MAX, its set when the demuxer probes
	* the format.
	* - encoding: unused
	* - decoding: set by avformat, read by user
	*/
	int probe_score;

	/**
	* number of bytes to read maximally to identify format.
	* - encoding: unused
	* - decoding: set by user
	*/
	int format_probesize;

	/**
	* ',' separated list of allowed decoders.
	* If NULL then all are allowed
	* - encoding: unused
	* - decoding: set by user
	*/
	char *codec_whitelist;

	/**
	* ',' separated list of allowed demuxers.
	* If NULL then all are allowed
	* - encoding: unused
	* - decoding: set by user
	*/
	char *format_whitelist;

	/**
	* An opaque field for libavformat internal usage.
	* Must not be accessed in any way by callers.
	*/
	AVFormatInternal *internal;

	/**
	* IO repositioned flag.
	* This is set by avformat when the underlaying IO context read pointer
	* is repositioned, for example when doing byte based seeking.
	* Demuxers can use the flag to detect such changes.
	*/
	int io_repositioned;

	/**
	* Forced video codec.
	* This allows forcing a specific decoder, even when there are multiple with
	* the same codec_id.
	* Demuxing: Set by user
	*/
	AVCodec *video_codec;

	/**
	* Forced audio codec.
	* This allows forcing a specific decoder, even when there are multiple with
	* the same codec_id.
	* Demuxing: Set by user
	*/
	AVCodec *audio_codec;

	/**
	* Forced subtitle codec.
	* This allows forcing a specific decoder, even when there are multiple with
	* the same codec_id.
	* Demuxing: Set by user
	*/
	AVCodec *subtitle_codec;

	/**
	* Forced data codec.
	* This allows forcing a specific decoder, even when there are multiple with
	* the same codec_id.
	* Demuxing: Set by user
	*/
	AVCodec *data_codec;

	/**
	* Number of bytes to be written as padding in a metadata header.
	* Demuxing: Unused.
	* Muxing: Set by user via av_format_set_metadata_header_padding.
	*/
	int metadata_header_padding;

	/**
	* User data.
	* This is a place for some private data of the user.
	*/
	void *opaque;

	/**
	* Callback used by devices to communicate with application.
	*/
	av_format_control_message control_message_cb;

	/**
	* Output timestamp offset, in microseconds.
	* Muxing: set by user
	*/
	int64_t output_ts_offset;

	/**
	* dump format separator.
	* can be ", " or "\n      " or anything else
	* - muxing: Set by user.
	* - demuxing: Set by user.
	*/
	uint8_t *dump_separator;

	/**
	* Forced Data codec_id.
	* Demuxing: Set by user.
	*/
	enum AVCodecID data_codec_id;

#if FF_API_OLD_OPEN_CALLBACKS
	/**
	* Called to open further IO contexts when needed for demuxing.
	*
	* This can be set by the user application to perform security checks on
	* the URLs before opening them.
	* The function should behave like avio_open2(), AVFormatContext is provided
	* as contextual information and to reach AVFormatContext.opaque.
	*
	* If NULL then some simple checks are used together with avio_open2().
	*
	* Must not be accessed directly from outside avformat.
	* @See av_format_set_open_cb()
	*
	* Demuxing: Set by user.
	*
	* @deprecated Use io_open and io_close.
	*/
	//attribute_deprecated
	//	int(*open_cb)(struct AVFormatContext *s, AVIOContext **p, const char *url, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options);
#endif

	/**
	* ',' separated list of allowed protocols.
	* - encoding: unused
	* - decoding: set by user
	*/
	char *protocol_whitelist;

	/**
	* A callback for opening new IO streams.
	*
	* Whenever a muxer or a demuxer needs to open an IO stream (typically from
	* avformat_open_input() for demuxers, but for certain formats can happen at
	* other times as well), it will call this callback to obtain an IO context.
	*
	* @param s the format context
	* @param pb on success, the newly opened IO context should be returned here
	* @param url the url to open
	* @param flags a combination of AVIO_FLAG_*
	* @param options a dictionary of additional options, with the same
	*                semantics as in avio_open2()
	* @return 0 on success, a negative AVERROR code on failure
	*
	* @note Certain muxers and demuxers do nesting, i.e. they open one or more
	* additional internal format contexts. Thus the AVFormatContext pointer
	* passed to this callback may be different from the one facing the caller.
	* It will, however, have the same 'opaque' field.
	*/
	int(*io_open)(struct AVFormatContext *s, AVIOContext **pb, const char *url,
		int flags, AVDictionary **options);

	/**
	* A callback for closing the streams opened with AVFormatContext.io_open().
	*/
	void(*io_close)(struct AVFormatContext *s, AVIOContext *pb);

	/**
	* ',' separated list of disallowed protocols.
	* - encoding: unused
	* - decoding: set by user
	*/
	char *protocol_blacklist;

	/**
	* The maximum number of streams.
	* - encoding: unused
	* - decoding: set by user
	*/
	int max_streams;

	/**
	* Skip duration calcuation in estimate_timings_from_pts.
	* - encoding: unused
	* - decoding: set by user
	*/
	int skip_estimate_duration_from_pts;

	/**
	* Maximum number of packets that can be probed
	* - encoding: unused
	* - decoding: set by user
	*/
	int max_probe_packets;
} AVFormatContext;

typedef struct AVOutputFormat {
	const char *name;
	/**
	* Descriptive name for the format, meant to be more human-readable
	* than name. You should use the NULL_IF_CONFIG_SMALL() macro
	* to define it.
	*/
	const char *long_name;
	const char *mime_type;
	const char *extensions; /**< comma-separated filename extensions */
	/* output support */
	enum AVCodecID audio_codec;    /**< default audio codec */
	enum AVCodecID video_codec;    /**< default video codec */
	enum AVCodecID subtitle_codec; /**< default subtitle codec */
	/**
	* can use flags: AVFMT_NOFILE, AVFMT_NEEDNUMBER,
	* AVFMT_GLOBALHEADER, AVFMT_NOTIMESTAMPS, AVFMT_VARIABLE_FPS,
	* AVFMT_NODIMENSIONS, AVFMT_NOSTREAMS, AVFMT_ALLOW_FLUSH,
	* AVFMT_TS_NONSTRICT, AVFMT_TS_NEGATIVE
	*/
	int flags;

	/**
	* List of supported codec_id-codec_tag pairs, ordered by "better
	* choice first". The arrays are all terminated by AV_CODEC_ID_NONE.
	*/
	const struct AVCodecTag * const *codec_tag;


	const AVClass *priv_class; ///< AVClass for the private context

	/*****************************************************************
	* No fields below this line are part of the public API. They
	* may not be used outside of libavformat and can be changed and
	* removed at will.
	* New public fields should be added right above.
	*****************************************************************
	*/
	/**
	* The ff_const59 define is not part of the public API and will
	* be removed without further warning.
	*/
	struct AVOutputFormat *next;
	/**
	* size of private data so that it can be allocated in the wrapper
	*/
	int priv_data_size;

	int(*write_header)(struct AVFormatContext *);
	/**
	* Write a packet. If AVFMT_ALLOW_FLUSH is set in flags,
	* pkt can be NULL in order to flush data buffered in the muxer.
	* When flushing, return 0 if there still is more data to flush,
	* or 1 if everything was flushed and there is no more buffered
	* data.
	*/
	int(*write_packet)(struct AVFormatContext *, AVPacket *pkt);
	int(*write_trailer)(struct AVFormatContext *);
	/**
	* Currently only used to set pixel format if not YUV420P.
	*/
	int(*interleave_packet)(struct AVFormatContext *, AVPacket *out,
		AVPacket *in, int flush);
	/**
	* Test if the given codec can be stored in this container.
	*
	* @return 1 if the codec is supported, 0 if it is not.
	*         A negative number if unknown.
	*         MKTAG('A', 'P', 'I', 'C') if the codec is only supported as AV_DISPOSITION_ATTACHED_PIC
	*/
	int(*query_codec)(enum AVCodecID id, int std_compliance);

	void(*get_output_timestamp)(struct AVFormatContext *s, int stream,
		int64_t *dts, int64_t *wall);
	/**
	* Allows sending messages from application to device.
	*/
	int(*control_message)(struct AVFormatContext *s, int type,
		void *data, size_t data_size);

	/**
	* Write an uncoded AVFrame.
	*
	* See av_write_uncoded_frame() for details.
	*
	* The library will free *frame afterwards, but the muxer can prevent it
	* by setting the pointer to NULL.
	*/
	int(*write_uncoded_frame)(struct AVFormatContext *, int stream_index,
		AVFrame **frame, unsigned flags);
	/**
	* Returns device list with it properties.
	* @see avdevice_list_devices() for more details.
	*/
	int(*get_device_list)(struct AVFormatContext *s, struct AVDeviceInfoList *device_list);
	/**
	* Initialize device capabilities submodule.
	* @see avdevice_capabilities_create() for more details.
	*/
	int(*create_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
	/**
	* Free device capabilities submodule.
	* @see avdevice_capabilities_free() for more details.
	*/
	int(*free_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
	enum AVCodecID data_codec; /**< default data codec */
	/**
	* Initialize format. May allocate data here, and set any AVFormatContext or
	* AVStream parameters that need to be set before packets are sent.
	* This method must not write output.
	*
	* Return 0 if streams were fully configured, 1 if not, negative AVERROR on failure
	*
	* Any allocations made here must be freed in deinit().
	*/
	int(*init)(struct AVFormatContext *);
	/**
	* Deinitialize format. If present, this is called whenever the muxer is being
	* destroyed, regardless of whether or not the header has been written.
	*
	* If a trailer is being written, this is called after write_trailer().
	*
	* This is called if init() fails as well.
	*/
	void(*deinit)(struct AVFormatContext *);
	/**
	* Set up any necessary bitstream filtering and extract any extra data needed
	* for the global header.
	* Return 0 if more packets from this stream must be checked; 1 if not.
	*/
	int(*check_bitstream)(struct AVFormatContext *, const AVPacket *pkt);
} AVOutputFormat;

typedef struct AVInputFormat {
	/**
	* A comma separated list of short names for the format. New names
	* may be appended with a minor bump.
	*/
	const char *name;

	/**
	* Descriptive name for the format, meant to be more human-readable
	* than name. You should use the NULL_IF_CONFIG_SMALL() macro
	* to define it.
	*/
	const char *long_name;

	/**
	* Can use flags: AVFMT_NOFILE, AVFMT_NEEDNUMBER, AVFMT_SHOW_IDS,
	* AVFMT_NOTIMESTAMPS, AVFMT_GENERIC_INDEX, AVFMT_TS_DISCONT, AVFMT_NOBINSEARCH,
	* AVFMT_NOGENSEARCH, AVFMT_NO_BYTE_SEEK, AVFMT_SEEK_TO_PTS.
	*/
	int flags;

	/**
	* If extensions are defined, then no probe is done. You should
	* usually not use extension format guessing because it is not
	* reliable enough
	*/
	const char *extensions;

	const struct AVCodecTag * const *codec_tag;

	const AVClass *priv_class; ///< AVClass for the private context

	/**
	* Comma-separated list of mime types.
	* It is used check for matching mime types while probing.
	* @see av_probe_input_format2
	*/
	const char *mime_type;

	/*****************************************************************
	* No fields below this line are part of the public API. They
	* may not be used outside of libavformat and can be changed and
	* removed at will.
	* New public fields should be added right above.
	*****************************************************************
	*/
	struct AVInputFormat *next;

	/**
	* Raw demuxers store their codec ID here.
	*/
	int raw_codec_id;

	/**
	* Size of private data so that it can be allocated in the wrapper.
	*/
	int priv_data_size;

	/**
	* Tell if a given file has a chance of being parsed as this format.
	* The buffer provided is guaranteed to be AVPROBE_PADDING_SIZE bytes
	* big so you do not have to check for that unless you need more.
	*/
	int(*read_probe)(const AVProbeData *);

	/**
	* Read the format header and initialize the AVFormatContext
	* structure. Return 0 if OK. 'avformat_new_stream' should be
	* called to create new streams.
	*/
	int(*read_header)(struct AVFormatContext *);

	/**
	* Read one packet and put it in 'pkt'. pts and flags are also
	* set. 'avformat_new_stream' can be called only if the flag
	* AVFMTCTX_NOHEADER is used and only in the calling thread (not in a
	* background thread).
	* @return 0 on success, < 0 on error.
	*         When returning an error, pkt must not have been allocated
	*         or must be freed before returning
	*/
	int(*read_packet)(struct AVFormatContext *, AVPacket *pkt);

	/**
	* Close the stream. The AVFormatContext and AVStreams are not
	* freed by this function
	*/
	int(*read_close)(struct AVFormatContext *);

	/**
	* Seek to a given timestamp relative to the frames in
	* stream component stream_index.
	* @param stream_index Must not be -1.
	* @param flags Selects which direction should be preferred if no exact
	*              match is available.
	* @return >= 0 on success (but not necessarily the new offset)
	*/
	int(*read_seek)(struct AVFormatContext *,
		int stream_index, int64_t timestamp, int flags);

	/**
	* Get the next timestamp in stream[stream_index].time_base units.
	* @return the timestamp or AV_NOPTS_VALUE if an error occurred
	*/
	int64_t(*read_timestamp)(struct AVFormatContext *s, int stream_index,
		int64_t *pos, int64_t pos_limit);

	/**
	* Start/resume playing - only meaningful if using a network-based format
	* (RTSP).
	*/
	int(*read_play)(struct AVFormatContext *);

	/**
	* Pause playing - only meaningful if using a network-based format
	* (RTSP).
	*/
	int(*read_pause)(struct AVFormatContext *);

	/**
	* Seek to timestamp ts.
	* Seeking will be done so that the point from which all active streams
	* can be presented successfully will be closest to ts and within min/max_ts.
	* Active streams are all streams that have AVStream.discard < AVDISCARD_ALL.
	*/
	int(*read_seek2)(struct AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags);

	/**
	* Returns device list with it properties.
	* @see avdevice_list_devices() for more details.
	*/
	int(*get_device_list)(struct AVFormatContext *s, struct AVDeviceInfoList *device_list);

	/**
	* Initialize device capabilities submodule.
	* @see avdevice_capabilities_create() for more details.
	*/
	int(*create_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);

	/**
	* Free device capabilities submodule.
	* @see avdevice_capabilities_free() for more details.
	*/
	int(*free_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
} AVInputFormat;

enum AVWriteUncodedFrameFlags {
	/**
	* Query whether the feature is possible on this stream.
	* The frame argument is ignored.
	*/
	AV_WRITE_UNCODED_FRAME_QUERY = 0x0001,
};

#define AV_PTS_WRAP_IGNORE      0   ///< ignore the wrap
#define AV_PTS_WRAP_ADD_OFFSET  1   ///< add the format specific offset on wrap detection
#define AV_PTS_WRAP_SUB_OFFSET  -1  ///< subtract the format specific offset on wrap detection

#define AVSEEK_FLAG_BACKWARD 1 ///< seek backward
#define AVSEEK_FLAG_BYTE     2 ///< seeking based on position in bytes
#define AVSEEK_FLAG_ANY      4 ///< seek to any frame, even non-keyframes
#define AVSEEK_FLAG_FRAME    8 ///< seeking based on frame number

const struct AVCodecTag *avformat_get_riff_video_tags(void);
enum AVCodecID av_codec_get_id(const struct AVCodecTag * const *tags, unsigned int tag);
enum AVCodecID ff_codec_get_id(const AVCodecTag *tags, unsigned int tag);
AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
void avpriv_set_pts_info(AVStream *s, int pts_wrap_bits, unsigned int pts_num, unsigned int pts_den);

void avpriv_register_devices(const AVOutputFormat * const o[], const AVInputFormat * const i[]);
FBC_EXPORTS AVFormatContext *avformat_alloc_context(void);

FBC_EXPORTS AVInputFormat *av_find_input_format(const char *short_name);
const AVInputFormat *av_demuxer_iterate(void **opaque);

FBC_EXPORTS int avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
int avformat_queue_attached_pictures(AVFormatContext *s);
int ff_packet_list_put(AVPacketList **head, AVPacketList **tail, AVPacket *pkt, int flags);
void avformat_free_context(AVFormatContext *s);

void ff_free_stream(AVFormatContext *s, AVStream *st);
void ff_packet_list_free(AVPacketList **head, AVPacketList **tail);

FBC_EXPORTS int av_read_frame(AVFormatContext *s, AVPacket *pkt);

int ff_packet_list_get(AVPacketList **head, AVPacketList **tail, AVPacket *pkt);
int ff_read_packet(AVFormatContext *s, AVPacket *pkt);

AVInputFormat *av_probe_input_format3(AVProbeData *pd, int is_opened, int *score_ret);
int av_match_ext(const char *filename, const char *extensions);
AVProgram *av_find_program_from_stream(AVFormatContext *ic, AVProgram *last, int s);
int av_find_default_stream_index(AVFormatContext *s);
void ff_compute_frame_duration(AVFormatContext *s, int *pnum, int *pden, AVStream *st, AVCodecParserContext *pc, AVPacket *pkt);
void ff_reduce_index(AVFormatContext *s, int stream_index);
int av_add_index_entry(AVStream *st, int64_t pos, int64_t timestamp, int size, int distance, int flags);
int ff_add_index_entry(AVIndexEntry **index_entries, int *nb_index_entries, unsigned int *index_entries_allocated_size, int64_t pos, int64_t timestamp, int size, int distance, int flags);

int ff_index_search_timestamp(const AVIndexEntry *entries, int nb_entries, int64_t wanted_timestamp, int flags);
FBC_EXPORTS void avformat_close_input(AVFormatContext **s);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVFORMAT_HPP_
