// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <stdio.h>
#include "avpixdesc.hpp"
#include "ffmpeg_common.hpp"
#include "ffmpeg_pixel_format.hpp"

// reference: ffmpeg 4.2
//            libavutil/pixdesc.c

#ifdef _MSC_VER

namespace fbc {

#if FF_API_PLUS1_MINUS1
FF_DISABLE_DEPRECATION_WARNINGS
#endif
static const AVPixFmtDescriptor av_pix_fmt_descriptors[AV_PIX_FMT_NB] = {
	{ "yuv420p", 3, 1, 1, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "yuyv422", 3, 1, 0, 0, { { 0, 2, 0, 0, 8, 1, 7, 1 }, { 0, 4, 1, 0, 8, 3, 7, 2 }, { 0, 4, 3, 0, 8, 3, 7, 4 } } },
	{ "rgb24", 3, 0, 0, AV_PIX_FMT_FLAG_RGB, { { 0, 3, 0, 0, 8, 2, 7, 1 }, { 0, 3, 1, 0, 8, 2, 7, 2 }, { 0, 3, 2, 0, 8, 2, 7, 3 } } },
	{ "bgr24", 3, 0, 0, AV_PIX_FMT_FLAG_RGB, { { 0, 3, 2, 0, 8, 2, 7, 3 }, { 0, 3, 1, 0, 8, 2, 7, 2 }, { 0, 3, 0, 0, 8, 2, 7, 1 } } },
	{ "yuv422p", 3, 1, 0, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "yuv444p", 3, 0, 0, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "yuv410p", 3, 2, 2, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "yuv411p", 3, 2, 0, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "gray", 1, 0, 0, FF_PSEUDOPAL, { { 0, 1, 0, 0, 8, 0, 7, 1 }, {}, {} }, "gray8,y8" }, // AV_PIX_FMT_GRAY8
	{ "monow", 1, 0, 0, AV_PIX_FMT_FLAG_BITSTREAM, { { 0, 1, 0, 0, 1, 0, 0, 1 }, {}, {} } },// AV_PIX_FMT_MONOWHITE
	{ "monob", 1, 0, 0, AV_PIX_FMT_FLAG_BITSTREAM, { { 0, 1, 0, 7, 1, 0, 0, 1 }, {}, {} } },// AV_PIX_FMT_MONOBLACK
	{ "pal8", 1, 0, 0, AV_PIX_FMT_FLAG_PAL | AV_PIX_FMT_FLAG_ALPHA, { { 0, 1, 0, 0, 8, 0, 7, 1 }, {}, {} } },
	{ "yuvj420p", 3, 1, 1, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "yuvj422p", 3, 1, 0, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{ "yuvj444p", 3, 0, 0, AV_PIX_FMT_FLAG_PLANAR, { { 0, 1, 0, 0, 8, 0, 7, 1 }, { 1, 1, 0, 0, 8, 0, 7, 1 }, { 2, 1, 0, 0, 8, 0, 7, 1 } } },
	{}, // AV_PIX_FMT_UYVY422
	{}, // AV_PIX_FMT_UYYVYY411
};
#if FF_API_PLUS1_MINUS1
FF_ENABLE_DEPRECATION_WARNINGS
#endif

const AVPixFmtDescriptor *av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt)
{
	ERROR_POS
	if (pix_fmt < 0 || pix_fmt >= AV_PIX_FMT_NB)
		return NULL;
	return &av_pix_fmt_descriptors[pix_fmt];
}

const char *av_get_pix_fmt_name(enum AVPixelFormat pix_fmt)
{
	return (unsigned)pix_fmt < AV_PIX_FMT_NB ?
		av_pix_fmt_descriptors[pix_fmt].name : NULL;
}

} // namespace fbc

#endif // _MSC_VER
