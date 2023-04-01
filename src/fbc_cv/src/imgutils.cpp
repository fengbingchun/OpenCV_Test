// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <stdio.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include "imgutils.hpp"
#include "avclass.hpp"
#include "avpixdesc.hpp"

// reference: ffmpeg 4.2
//            libavutil/imgutils.c

#ifdef _MSC_VER

namespace fbc {

typedef struct ImgUtils {
	const AVClass *class_;
	int   log_offset;
	void *log_ctx;
} ImgUtils;

static const AVClass imgutils_class = {
	"IMGUTILS",
	av_default_item_name,
	NULL,
	LIBAVUTIL_VERSION_INT,
	offsetof(ImgUtils, log_offset),
	offsetof(ImgUtils, log_ctx),
};

void av_image_fill_max_pixsteps(int max_pixsteps[4], int max_pixstep_comps[4],
	const AVPixFmtDescriptor *pixdesc)
{
	int i;
	memset(max_pixsteps, 0, 4 * sizeof(max_pixsteps[0]));
	if (max_pixstep_comps)
		memset(max_pixstep_comps, 0, 4 * sizeof(max_pixstep_comps[0]));

	for (i = 0; i < 4; i++) {
		const AVComponentDescriptor *comp = &(pixdesc->comp[i]);
		if (comp->step > max_pixsteps[comp->plane]) {
			max_pixsteps[comp->plane] = comp->step;
			if (max_pixstep_comps)
				max_pixstep_comps[comp->plane] = i;
		}
	}
}

static inline int image_get_linesize(int width, int plane,
			int max_step, int max_step_comp,
			const AVPixFmtDescriptor *desc)
{
	int s, shifted_w, linesize;

	if (!desc)
		return AVERROR(EINVAL);

	if (width < 0)
		return AVERROR(EINVAL);
	s = (max_step_comp == 1 || max_step_comp == 2) ? desc->log2_chroma_w : 0;
	shifted_w = ((width + (1 << s) - 1)) >> s;
	if (shifted_w && max_step > INT_MAX / shifted_w)
		return AVERROR(EINVAL);
	linesize = max_step * shifted_w;

	if (desc->flags & AV_PIX_FMT_FLAG_BITSTREAM)
		linesize = (linesize + 7) >> 3;
	return linesize;
}

int av_image_fill_linesizes(int linesizes[4], enum AVPixelFormat pix_fmt, int width)
{
	int i, ret;
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
	int max_step[4];       /* max pixel step for each plane */
	int max_step_comp[4];       /* the component for each plane which has the max pixel step */

	memset(linesizes, 0, 4 * sizeof(linesizes[0]));

	if (!desc || desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
		return AVERROR(EINVAL);

	av_image_fill_max_pixsteps(max_step, max_step_comp, desc);
	for (i = 0; i < 4; i++) {
		if ((ret = image_get_linesize(width, i, max_step[i], max_step_comp[i], desc)) < 0)
			return ret;
		linesizes[i] = ret;
	}

	return 0;
}

int av_image_fill_pointers(uint8_t *data[4], enum AVPixelFormat pix_fmt, int height,
	uint8_t *ptr, const int linesizes[4])
{
	int i, total_size, size[4] = { 0 }, has_plane[4] = { 0 };

	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
	memset(data, 0, sizeof(data[0]) * 4);

	if (!desc || desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
		return AVERROR(EINVAL);

	data[0] = ptr;
	if (linesizes[0] > (INT_MAX - 1024) / height)
		return AVERROR(EINVAL);
	size[0] = linesizes[0] * height;

	if (desc->flags & AV_PIX_FMT_FLAG_PAL ||
		desc->flags & FF_PSEUDOPAL) {
		data[1] = ptr + size[0]; /* palette is stored here as 256 32 bits words */
		return size[0] + 256 * 4;
	}

	for (i = 0; i < 4; i++)
		has_plane[desc->comp[i].plane] = 1;

	total_size = size[0];
	for (i = 1; i < 4 && has_plane[i]; i++) {
		int h, s = (i == 1 || i == 2) ? desc->log2_chroma_h : 0;
		data[i] = data[i - 1] + size[i - 1];
		h = (height + (1 << s) - 1) >> s;
		if (linesizes[i] > INT_MAX / h)
			return AVERROR(EINVAL);
		size[i] = h * linesizes[i];
		if (total_size > INT_MAX - size[i])
			return AVERROR(EINVAL);
		total_size += size[i];
	}

	return total_size;
}

int av_image_fill_arrays(uint8_t *dst_data[4], int dst_linesize[4],
	const uint8_t *src, enum AVPixelFormat pix_fmt,
	int width, int height, int align)
{
	int ret, i;

	ret = av_image_check_size(width, height, 0, NULL);
	if (ret < 0)
		return ret;

	ret = av_image_fill_linesizes(dst_linesize, pix_fmt, width);
	if (ret < 0)
		return ret;

	for (i = 0; i < 4; i++)
		dst_linesize[i] = FFALIGN(dst_linesize[i], align);

	return av_image_fill_pointers(dst_data, pix_fmt, height, (uint8_t *)src, dst_linesize);
}

int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx)
{
	return av_image_check_size2(w, h, INT64_MAX, AV_PIX_FMT_NONE, log_offset, log_ctx);
}

int av_image_get_linesize(enum AVPixelFormat pix_fmt, int width, int plane)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
	int max_step[4];       /* max pixel step for each plane */
	int max_step_comp[4];       /* the component for each plane which has the max pixel step */

	if (!desc || desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
		return AVERROR(EINVAL);

	av_image_fill_max_pixsteps(max_step, max_step_comp, desc);
	return image_get_linesize(width, plane, max_step[plane], max_step_comp[plane], desc);
}

int av_image_check_size2(unsigned int w, unsigned int h, int64_t max_pixels, enum AVPixelFormat pix_fmt, int log_offset, void *log_ctx)
{
	ImgUtils imgutils = {
		&imgutils_class,
		log_offset,
		log_ctx,
	};
	int64_t stride = av_image_get_linesize(pix_fmt, w, 0);
	if (stride <= 0)
		stride = 8LL * w;
	stride += 128 * 8;

	if ((int)w <= 0 || (int)h <= 0 || stride >= INT_MAX || stride*(uint64_t)(h + 128) >= INT_MAX) {
		fprintf(stderr, "imgutils: Picture size %ux%u is invalid\n", w, h);
		return AVERROR(EINVAL);
	}

	if (max_pixels < INT64_MAX) {
		if (w*(int64_t)h > max_pixels) {
			fprintf(stderr, "imgutils: Picture size %ux%u exceeds specified max pixel count %lld, see the documentation if you wish to increase it\n",
				w, h, max_pixels);
			return AVERROR(EINVAL);
		}
	}

	return 0;
}

} // namespace fbc

#endif // _MSC_VER
