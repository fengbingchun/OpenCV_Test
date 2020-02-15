// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVUTIL_HPP_
#define FBC_CV_AVUTIL_HPP_

// reference: ffmpeg 4.2
//            libavutil/avutil.h

#ifdef _MSC_VER

#include <stdio.h>
#include "avrational.hpp"

namespace fbc {

enum AVMediaType {
	AVMEDIA_TYPE_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
	AVMEDIA_TYPE_VIDEO,
	AVMEDIA_TYPE_AUDIO,
	AVMEDIA_TYPE_DATA,          ///< Opaque data information usually continuous
	AVMEDIA_TYPE_SUBTITLE,
	AVMEDIA_TYPE_ATTACHMENT,    ///< Opaque data information usually sparse
	AVMEDIA_TYPE_NB
};

enum AVSampleFormat {
	AV_SAMPLE_FMT_NONE = -1,
	AV_SAMPLE_FMT_U8,          ///< unsigned 8 bits
	AV_SAMPLE_FMT_S16,         ///< signed 16 bits
	AV_SAMPLE_FMT_S32,         ///< signed 32 bits
	AV_SAMPLE_FMT_FLT,         ///< float
	AV_SAMPLE_FMT_DBL,         ///< double

	AV_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
	AV_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
	AV_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
	AV_SAMPLE_FMT_FLTP,        ///< float, planar
	AV_SAMPLE_FMT_DBLP,        ///< double, planar
	AV_SAMPLE_FMT_S64,         ///< signed 64 bits
	AV_SAMPLE_FMT_S64P,        ///< signed 64 bits, planar

	AV_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};

enum AVPictureType {
	AV_PICTURE_TYPE_NONE = 0, ///< Undefined
	AV_PICTURE_TYPE_I,     ///< Intra
	AV_PICTURE_TYPE_P,     ///< Predicted
	AV_PICTURE_TYPE_B,     ///< Bi-dir predicted
	AV_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG-4
	AV_PICTURE_TYPE_SI,    ///< Switching Intra
	AV_PICTURE_TYPE_SP,    ///< Switching Predicted
	AV_PICTURE_TYPE_BI,    ///< BI type
};

int av_parse_video_rate(AVRational *rate, const char *str);
int av_parse_ratio(AVRational *q, const char *str, int max, int log_offset, void *log_ctx);

#define av_parse_ratio_quiet(rate, str, max) \
	av_parse_ratio(rate, str, max, AV_LOG_MAX_OFFSET, NULL)

typedef struct AVExpr AVExpr;

int av_expr_parse_and_eval(double *res, const char *s,
			const char * const *const_names, const double *const_values,
			const char * const *func1_names, double(*const *funcs1)(void *, double),
			const char * const *func2_names, double(*const *funcs2)(void *, double, double),
			void *opaque, int log_offset, void *log_ctx);
int av_expr_parse(AVExpr **expr, const char *s,
			const char * const *const_names,
			const char * const *func1_names, double(*const *funcs1)(void *, double),
			const char * const *func2_names, double(*const *funcs2)(void *, double, double),
			int log_offset, void *log_ctx);
double av_expr_eval(AVExpr *e, const double *const_values, void *opaque);
void av_expr_free(AVExpr *e);

static av_always_inline int av_clip_c(int a, int amin, int amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
	if (amin > amax) abort();
#endif
	if (a < amin) return amin;
	else if (a > amax) return amax;
	else               return a;
}

static av_always_inline double av_clipd_c(double a, double amin, double amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
	if (amin > amax) abort();
#endif
	if (a < amin) return amin;
	else if (a > amax) return amax;
	else               return a;
}

#ifndef av_clip
#   define av_clip          av_clip_c
#endif

#ifndef av_clipd
#   define av_clipd         av_clipd_c
#endif

static inline int av_isspace(int c)
{
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

double av_strtod(const char *numstr, char **tail);
int64_t av_gettime(void);

#define FF_LAMBDA_SHIFT 7
#define FF_LAMBDA_SCALE (1<<FF_LAMBDA_SHIFT)
#define FF_QP2LAMBDA 118 ///< factor to convert from H.263 QP to lambda
#define FF_LAMBDA_MAX (256*128-1)

#define AV_NOPTS_VALUE		((int64_t)UINT64_C(0x8000000000000000))

int av_parse_color(uint8_t *rgba_color, const char *color_string, int slen, void *log_ctx);

size_t av_strlcpy(char *dst, const char *src, size_t size);
//int av_strcasecmp(const char *a, const char *b);
size_t av_strlcat(char *dst, const char *src, size_t size);

uint32_t av_get_random_seed(void);
int av_parse_time(int64_t *timeval, const char *timestr, int duration);
int av_match_name(const char *name, const char *names);
int av_strncasecmp(const char *a, const char *b, size_t n);
int av_parse_video_size(int *width_ptr, int *height_ptr, const char *str);

static inline int av_tolower(int c)
{
	if (c >= 'A' && c <= 'Z')
		c ^= 0x20;
	return c;
}

enum AVPixelFormat av_get_pix_fmt(const char *name);

#define ff_log2		ff_log2_x86
static av_always_inline int ff_log2_x86(unsigned int v)
{
	unsigned long n;
	_BitScanReverse(&n, v | 1);
	return n;
}

#define av_log2		ff_log2

#define AV_TIME_BASE            1000000
#define AV_TIME_BASE_Q          AVRational{1, AV_TIME_BASE}

#define AV_TS_MAX_STRING_SIZE 32

} // namespae codec

#endif // _MSC_VER
#endif // FBC_CV_AVUTIL_HPP_
