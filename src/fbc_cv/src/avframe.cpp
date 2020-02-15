// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <string.h>
#include "avframe.hpp"
#include "avmem.hpp"
#include "avutil.hpp"
#include "ffmpeg_pixel_format.hpp"

// reference: ffmpeg 4.2
//            libavutil/frame.c

#ifdef _MSC_VER

namespace fbc {

void av_frame_free(AVFrame **frame)
{
	if (!frame || !*frame)
		return;

	av_frame_unref(*frame);
	av_freep(frame);
}

static void free_side_data(AVFrameSideData **ptr_sd)
{
	AVFrameSideData *sd = *ptr_sd;

	av_buffer_unref(&sd->buf);
	av_dict_free(&sd->metadata);
	av_freep(ptr_sd);
}

static void wipe_side_data(AVFrame *frame)
{
	int i;

	for (i = 0; i < frame->nb_side_data; i++) {
		free_side_data(&frame->side_data[i]);
	}
	frame->nb_side_data = 0;

	av_freep(&frame->side_data);
}

static void get_frame_defaults(AVFrame *frame)
{
	if (frame->extended_data != frame->data)
		av_freep(&frame->extended_data);

	memset(frame, 0, sizeof(*frame));

	frame->pts =
		frame->pkt_dts = AV_NOPTS_VALUE;
#if FF_API_PKT_PTS
FF_DISABLE_DEPRECATION_WARNINGS
	frame->pkt_pts = AV_NOPTS_VALUE;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
		frame->best_effort_timestamp = AV_NOPTS_VALUE;
	frame->pkt_duration = 0;
	frame->pkt_pos = -1;
	frame->pkt_size = -1;
	frame->key_frame = 1;
	frame->sample_aspect_ratio = AVRational{ 0, 1 };
	frame->format = -1; /* unknown */
	frame->extended_data = frame->data;
	frame->color_primaries = AVCOL_PRI_UNSPECIFIED;
	frame->color_trc = AVCOL_TRC_UNSPECIFIED;
	frame->colorspace = AVCOL_SPC_UNSPECIFIED;
	frame->color_range = AVCOL_RANGE_UNSPECIFIED;
	frame->chroma_location = AVCHROMA_LOC_UNSPECIFIED;
	frame->flags = 0;
}

void av_frame_unref(AVFrame *frame)
{
	int i;

	if (!frame)
		return;

	wipe_side_data(frame);

	for (i = 0; i < FF_ARRAY_ELEMS(frame->buf); i++)
		av_buffer_unref(&frame->buf[i]);
	for (i = 0; i < frame->nb_extended_buf; i++)
		av_buffer_unref(&frame->extended_buf[i]);
	av_freep(&frame->extended_buf);
	av_dict_free(&frame->metadata);
#if FF_API_FRAME_QP
FF_DISABLE_DEPRECATION_WARNINGS
	av_buffer_unref(&frame->qp_table_buf);
FF_ENABLE_DEPRECATION_WARNINGS
#endif

	av_buffer_unref(&frame->hw_frames_ctx);

	av_buffer_unref(&frame->opaque_ref);
	av_buffer_unref(&frame->private_ref);

	get_frame_defaults(frame);
}

} // namespace fbc

#endif // _MSC_VER
