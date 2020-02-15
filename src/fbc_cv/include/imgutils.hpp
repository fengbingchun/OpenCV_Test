// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_IMGUTILS_HPP_
#define FBC_CV_IMGUTILS_HPP_

// reference: ffmpeg 4.2
//            libavutil/imgutils.h

#ifdef _MSC_VER

#include "ffmpeg_common.hpp"
#include "ffmpeg_pixel_format.hpp"

namespace fbc {

int av_image_fill_arrays(uint8_t *dst_data[4], int dst_linesize[4],
			const uint8_t *src,
			enum AVPixelFormat pix_fmt, int width, int height, int align);
int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx);
int av_image_check_size2(unsigned int w, unsigned int h, int64_t max_pixels, enum AVPixelFormat pix_fmt, int log_offset, void *log_ctx);
int av_image_fill_linesizes(int linesizes[4], enum AVPixelFormat pix_fmt, int width);
int av_image_fill_pointers(uint8_t *data[4], enum AVPixelFormat pix_fmt, int height, uint8_t *ptr, const int linesizes[4]);
int av_image_get_linesize(enum AVPixelFormat pix_fmt, int width, int plane);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_IMGUTILS_HPP_
