// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVFRAME_HPP_
#define FBC_CV_AVFRAME_HPP_

// reference: ffmpeg 4.2
//            libavutil/frame.h

#ifdef _MSC_VER

#include "ffmpeg_common.hpp"
#include "avbuffer.hpp"
#include "avdictionary.hpp"
#include "avrational.hpp"

namespace fbc {

typedef struct AVFrameSideData {
	enum AVFrameSideDataType type;
	uint8_t *data;
	int      size;
	AVDictionary *metadata;
	AVBufferRef *buf;
} AVFrameSideData;

typedef struct AVFrame {
#define AV_NUM_DATA_POINTERS 8
	/**
	* pointer to the picture/channel planes.
	* This might be different from the first allocated byte
	*
	* Some decoders access areas outside 0,0 - width,height, please
	* see avcodec_align_dimensions2(). Some filters and swscale can read
	* up to 16 bytes beyond the planes, if these filters are to be used,
	* then 16 extra bytes must be allocated.
	*
	* NOTE: Except for hwaccel formats, pointers not needed by the format
	* MUST be set to NULL.
	*/
	uint8_t *data[AV_NUM_DATA_POINTERS];

	/**
	* For video, size in bytes of each picture line.
	* For audio, size in bytes of each plane.
	*
	* For audio, only linesize[0] may be set. For planar audio, each channel
	* plane must be the same size.
	*
	* For video the linesizes should be multiples of the CPUs alignment
	* preference, this is 16 or 32 for modern desktop CPUs.
	* Some code requires such alignment other code can be slower without
	* correct alignment, for yet other it makes no difference.
	*
	* @note The linesize may be larger than the size of usable data -- there
	* may be extra padding present for performance reasons.
	*/
	int linesize[AV_NUM_DATA_POINTERS];

	/**
	* pointers to the data planes/channels.
	*
	* For video, this should simply point to data[].
	*
	* For planar audio, each channel has a separate data pointer, and
	* linesize[0] contains the size of each channel buffer.
	* For packed audio, there is just one data pointer, and linesize[0]
	* contains the total size of the buffer for all channels.
	*
	* Note: Both data and extended_data should always be set in a valid frame,
	* but for planar audio with more channels that can fit in data,
	* extended_data must be used in order to access all channels.
	*/
	uint8_t **extended_data;

	/**
	* @name Video dimensions
	* Video frames only. The coded dimensions (in pixels) of the video frame,
	* i.e. the size of the rectangle that contains some well-defined values.
	*
	* @note The part of the frame intended for display/presentation is further
	* restricted by the @ref cropping "Cropping rectangle".
	* @{
	*/
	int width, height;
	/**
	* @}
	*/

	/**
	* number of audio samples (per channel) described by this frame
	*/
	int nb_samples;

	/**
	* format of the frame, -1 if unknown or unset
	* Values correspond to enum AVPixelFormat for video frames,
	* enum AVSampleFormat for audio)
	*/
	int format;

	/**
	* 1 -> keyframe, 0-> not
	*/
	int key_frame;

	/**
	* Picture type of the frame.
	*/
	enum AVPictureType pict_type;

	/**
	* Sample aspect ratio for the video frame, 0/1 if unknown/unspecified.
	*/
	AVRational sample_aspect_ratio;

	/**
	* Presentation timestamp in time_base units (time when frame should be shown to user).
	*/
	int64_t pts;

#if FF_API_PKT_PTS
	/**
	* PTS copied from the AVPacket that was decoded to produce this frame.
	* @deprecated use the pts field instead
	*/
	attribute_deprecated
	int64_t pkt_pts;
#endif

	/**
	* DTS copied from the AVPacket that triggered returning this frame. (if frame threading isn't used)
	* This is also the Presentation time of this AVFrame calculated from
	* only AVPacket.dts values without pts values.
	*/
	int64_t pkt_dts;

	/**
	* picture number in bitstream order
	*/
	int coded_picture_number;
	/**
	* picture number in display order
	*/
	int display_picture_number;

	/**
	* quality (between 1 (good) and FF_LAMBDA_MAX (bad))
	*/
	int quality;

	/**
	* for some private data of the user
	*/
	void *opaque;

#if FF_API_ERROR_FRAME
	/**
	* @deprecated unused
	*/
	attribute_deprecated
	uint64_t error[AV_NUM_DATA_POINTERS];
#endif

	/**
	* When decoding, this signals how much the picture must be delayed.
	* extra_delay = repeat_pict / (2*fps)
	*/
	int repeat_pict;

	/**
	* The content of the picture is interlaced.
	*/
	int interlaced_frame;

	/**
	* If the content is interlaced, is top field displayed first.
	*/
	int top_field_first;

	/**
	* Tell user application that palette has changed from previous frame.
	*/
	int palette_has_changed;

	/**
	* reordered opaque 64 bits (generally an integer or a double precision float
	* PTS but can be anything).
	* The user sets AVCodecContext.reordered_opaque to represent the input at
	* that time,
	* the decoder reorders values as needed and sets AVFrame.reordered_opaque
	* to exactly one of the values provided by the user through AVCodecContext.reordered_opaque
	*/
	int64_t reordered_opaque;

	/**
	* Sample rate of the audio data.
	*/
	int sample_rate;

	/**
	* Channel layout of the audio data.
	*/
	uint64_t channel_layout;

	/**
	* AVBuffer references backing the data for this frame. If all elements of
	* this array are NULL, then this frame is not reference counted. This array
	* must be filled contiguously -- if buf[i] is non-NULL then buf[j] must
	* also be non-NULL for all j < i.
	*
	* There may be at most one AVBuffer per data plane, so for video this array
	* always contains all the references. For planar audio with more than
	* AV_NUM_DATA_POINTERS channels, there may be more buffers than can fit in
	* this array. Then the extra AVBufferRef pointers are stored in the
	* extended_buf array.
	*/
	AVBufferRef *buf[AV_NUM_DATA_POINTERS];

	/**
	* For planar audio which requires more than AV_NUM_DATA_POINTERS
	* AVBufferRef pointers, this array will hold all the references which
	* cannot fit into AVFrame.buf.
	*
	* Note that this is different from AVFrame.extended_data, which always
	* contains all the pointers. This array only contains the extra pointers,
	* which cannot fit into AVFrame.buf.
	*
	* This array is always allocated using av_malloc() by whoever constructs
	* the frame. It is freed in av_frame_unref().
	*/
	AVBufferRef **extended_buf;
	/**
	* Number of elements in extended_buf.
	*/
	int        nb_extended_buf;

	AVFrameSideData **side_data;
	int            nb_side_data;

	/**
	* @defgroup lavu_frame_flags AV_FRAME_FLAGS
	* @ingroup lavu_frame
	* Flags describing additional frame properties.
	*
	* @{
	*/

	/**
	* The frame data may be corrupted, e.g. due to decoding errors.
	*/
#define AV_FRAME_FLAG_CORRUPT       (1 << 0)
	/**
	* A flag to mark the frames which need to be decoded, but shouldn't be output.
	*/
#define AV_FRAME_FLAG_DISCARD   (1 << 2)
	/**
	* @}
	*/

	/**
	* Frame flags, a combination of @ref lavu_frame_flags
	*/
	int flags;

	/**
	* MPEG vs JPEG YUV range.
	* - encoding: Set by user
	* - decoding: Set by libavcodec
	*/
	enum AVColorRange color_range;

	enum AVColorPrimaries color_primaries;

	enum AVColorTransferCharacteristic color_trc;

	/**
	* YUV colorspace type.
	* - encoding: Set by user
	* - decoding: Set by libavcodec
	*/
	enum AVColorSpace colorspace;

	enum AVChromaLocation chroma_location;

	/**
	* frame timestamp estimated using various heuristics, in stream time base
	* - encoding: unused
	* - decoding: set by libavcodec, read by user.
	*/
	int64_t best_effort_timestamp;

	/**
	* reordered pos from the last AVPacket that has been input into the decoder
	* - encoding: unused
	* - decoding: Read by user.
	*/
	int64_t pkt_pos;

	/**
	* duration of the corresponding packet, expressed in
	* AVStream->time_base units, 0 if unknown.
	* - encoding: unused
	* - decoding: Read by user.
	*/
	int64_t pkt_duration;

	/**
	* metadata.
	* - encoding: Set by user.
	* - decoding: Set by libavcodec.
	*/
	AVDictionary *metadata;

	/**
	* decode error flags of the frame, set to a combination of
	* FF_DECODE_ERROR_xxx flags if the decoder produced a frame, but there
	* were errors during the decoding.
	* - encoding: unused
	* - decoding: set by libavcodec, read by user.
	*/
	int decode_error_flags;
#define FF_DECODE_ERROR_INVALID_BITSTREAM   1
#define FF_DECODE_ERROR_MISSING_REFERENCE   2
#define FF_DECODE_ERROR_CONCEALMENT_ACTIVE  4
#define FF_DECODE_ERROR_DECODE_SLICES       8

	/**
	* number of audio channels, only used for audio.
	* - encoding: unused
	* - decoding: Read by user.
	*/
	int channels;

	/**
	* size of the corresponding packet containing the compressed
	* frame.
	* It is set to a negative value if unknown.
	* - encoding: unused
	* - decoding: set by libavcodec, read by user.
	*/
	int pkt_size;

#if FF_API_FRAME_QP
	/**
	* QP table
	*/
	attribute_deprecated
	int8_t *qscale_table;
	/**
	* QP store stride
	*/
	attribute_deprecated
	int qstride;

	attribute_deprecated
	int qscale_type;

	attribute_deprecated
	AVBufferRef *qp_table_buf;
#endif
	/**
	* For hwaccel-format frames, this should be a reference to the
	* AVHWFramesContext describing the frame.
	*/
	AVBufferRef *hw_frames_ctx;

	/**
	* AVBufferRef for free use by the API user. FFmpeg will never check the
	* contents of the buffer ref. FFmpeg calls av_buffer_unref() on it when
	* the frame is unreferenced. av_frame_copy_props() calls create a new
	* reference with av_buffer_ref() for the target frame's opaque_ref field.
	*
	* This is unrelated to the opaque field, although it serves a similar
	* purpose.
	*/
	AVBufferRef *opaque_ref;

	/**
	* @anchor cropping
	* @name Cropping
	* Video frames only. The number of pixels to discard from the the
	* top/bottom/left/right border of the frame to obtain the sub-rectangle of
	* the frame intended for presentation.
	* @{
	*/
	size_t crop_top;
	size_t crop_bottom;
	size_t crop_left;
	size_t crop_right;
	/**
	* @}
	*/

	/**
	* AVBufferRef for internal use by a single libav* library.
	* Must not be used to transfer data between libraries.
	* Has to be NULL when ownership of the frame leaves the respective library.
	*
	* Code outside the FFmpeg libs should never check or change the contents of the buffer ref.
	*
	* FFmpeg calls av_buffer_unref() on it when the frame is unreferenced.
	* av_frame_copy_props() calls create a new reference with av_buffer_ref()
	* for the target frame's private_ref field.
	*/
	AVBufferRef *private_ref;
} AVFrame;

void av_frame_free(AVFrame **frame);
void av_frame_unref(AVFrame *frame);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVFRAME_HPP_

