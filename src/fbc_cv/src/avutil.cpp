// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

// reference: ffmpeg 4.2
//            libavutil/parseutils.c

#ifdef _MSC_VER

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include "avutil.hpp"
#include "ffmpeg_common.hpp"
#include "avclass.hpp"
#include "avrational.hpp"
#include "mathematics.hpp"
#include "avmem.hpp"
#include "ffmpeg_pixel_format.hpp"

namespace fbc {

typedef struct VideoSizeAbbr {
	const char *abbr;
	int width, height;
} VideoSizeAbbr;

typedef struct VideoRateAbbr {
	const char *abbr;
	AVRational rate;
} VideoRateAbbr;

static const VideoRateAbbr video_rate_abbrs[] = {
	{ "ntsc", { 30000, 1001 } },
	{ "pal", { 25, 1 } },
	{ "qntsc", { 30000, 1001 } }, /* VCD compliant NTSC */
	{ "qpal", { 25, 1 } }, /* VCD compliant PAL */
	{ "sntsc", { 30000, 1001 } }, /* square pixel NTSC */
	{ "spal", { 25, 1 } }, /* square pixel PAL */
	{ "film", { 24, 1 } },
	{ "ntsc-film", { 24000, 1001 } },
};

static const VideoSizeAbbr video_size_abbrs[] = {
	{ "ntsc", 720, 480 },
	{ "pal", 720, 576 },
	{ "qntsc", 352, 240 }, /* VCD compliant NTSC */
	{ "qpal", 352, 288 }, /* VCD compliant PAL */
	{ "sntsc", 640, 480 }, /* square pixel NTSC */
	{ "spal", 768, 576 }, /* square pixel PAL */
	{ "film", 352, 240 },
	{ "ntsc-film", 352, 240 },
	{ "sqcif", 128, 96 },
	{ "qcif", 176, 144 },
	{ "cif", 352, 288 },
	{ "4cif", 704, 576 },
	{ "16cif", 1408, 1152 },
	{ "qqvga", 160, 120 },
	{ "qvga", 320, 240 },
	{ "vga", 640, 480 },
	{ "svga", 800, 600 },
	{ "xga", 1024, 768 },
	{ "uxga", 1600, 1200 },
	{ "qxga", 2048, 1536 },
	{ "sxga", 1280, 1024 },
	{ "qsxga", 2560, 2048 },
	{ "hsxga", 5120, 4096 },
	{ "wvga", 852, 480 },
	{ "wxga", 1366, 768 },
	{ "wsxga", 1600, 1024 },
	{ "wuxga", 1920, 1200 },
	{ "woxga", 2560, 1600 },
	{ "wqsxga", 3200, 2048 },
	{ "wquxga", 3840, 2400 },
	{ "whsxga", 6400, 4096 },
	{ "whuxga", 7680, 4800 },
	{ "cga", 320, 200 },
	{ "ega", 640, 350 },
	{ "hd480", 852, 480 },
	{ "hd720", 1280, 720 },
	{ "hd1080", 1920, 1080 },
	{ "2k", 2048, 1080 }, /* Digital Cinema System Specification */
	{ "2kdci", 2048, 1080 },
	{ "2kflat", 1998, 1080 },
	{ "2kscope", 2048, 858 },
	{ "4k", 4096, 2160 }, /* Digital Cinema System Specification */
	{ "4kdci", 4096, 2160 },
	{ "4kflat", 3996, 2160 },
	{ "4kscope", 4096, 1716 },
	{ "nhd", 640, 360 },
	{ "hqvga", 240, 160 },
	{ "wqvga", 400, 240 },
	{ "fwqvga", 432, 240 },
	{ "hvga", 480, 320 },
	{ "qhd", 960, 540 },
	{ "uhd2160", 3840, 2160 },
	{ "uhd4320", 7680, 4320 },
};

typedef struct Parser {
	const AVClass *class_;
	int stack_index;
	char *s;
	const double *const_values;
	const char * const *const_names;          // NULL terminated
	double(*const *funcs1)(void *, double a);           // NULL terminated
	const char * const *func1_names;          // NULL terminated
	double(*const *funcs2)(void *, double a, double b); // NULL terminated
	const char * const *func2_names;          // NULL terminated
	void *opaque;
	int log_offset;
	void *log_ctx;
#define VARS 10
	double *var;
} Parser;

struct AVExpr {
	typedef enum type_ {
		e_value, e_const, e_func0, e_func1, e_func2,
		e_squish, e_gauss, e_ld, e_isnan, e_isinf,
		e_mod, e_max, e_min, e_eq, e_gt, e_gte, e_lte, e_lt,
		e_pow, e_mul, e_div, e_add,
		e_last, e_st, e_while, e_taylor, e_root, e_floor, e_ceil, e_trunc, e_round,
		e_sqrt, e_not, e_random, e_hypot, e_gcd,
		e_if, e_ifnot, e_print, e_bitand, e_bitor, e_between, e_clip, e_atan2, e_lerp,
		e_sgn,
	} type_;
	type_ type;
	double value; // is sign in other types
	union {
		int const_index;
		double(*func0)(double);
		double(*func1)(void *, double);
		double(*func2)(void *, double, double);
	} a;
	struct AVExpr *param[3];
	double *var;
};

const uint8_t ff_reverse[256] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

static const AVClass eval_class = {
	"Eval",
	av_default_item_name,
	NULL,
	LIBAVUTIL_VERSION_INT,
	offsetof(Parser, log_offset),
	offsetof(Parser, log_ctx),
};

static const struct {
	double bin_val;
	double dec_val;
	int8_t exp;
} si_prefixes['z' - 'E' + 1] = { // 'z' - 'E' = 53
	{ 1.152921504606847e18, 1e18, 18 }, {}, { 1.073741824e9, 1e9, 9 }, {}, {}, {}, { 1.024e3, 1e3, 3 }, {}, { 1.048576e6, 1e6, 6 }, {},
	{}, { 1.125899906842624e15, 1e15, 15 }, {}, {}, {}, { 1.099511627776e12, 1e12, 12 }, {}, {}, {}, {},
	{ 1.2089258196146292e24, 1e24, 24 }, { 1.1805916207174113e21, 1e21, 21 }, {}, {}, {}, {}, {}, {}, { 8.6736173798840355e-19, 1e-18, -18 }, {},
	{ 9.8431332023036951e-3, 1e-2, -2 }, { 9.921256574801246e-2, 1e-1, -1 }, {}, { 8.8817841970012523e-16, 1e-15, -15 }, {}, { 1.0159366732596479e2, 1e2, 2 }, {}, {}, { 1.024e3, 1e3, 3 }, {},
	{ 9.765625e-4, 1e-3, -3 }, { 9.3132257461547852e-10, 1e-9, -9 }, {}, { 9.0949470177292824e-13, 1e-12, -12 }, {}, {}, {}, {}, { 9.5367431640625e-7, 1e-6, -6 }, {},
	{}, {}, { 8.271806125530276749e-25, 1e-24, -24 }, { 8.4703294725430034e-22, 1e-21, -21 },
};

static const struct {
	const char *name;
	double value;
} constants[] = {
	{ "E", M_E },
	{ "PI", M_PI },
	{ "PHI", M_PHI },
	{ "QP2LAMBDA", FF_QP2LAMBDA },
};

int av_parse_video_rate(AVRational *rate, const char *arg)
{
	int i, ret;
	int n = FF_ARRAY_ELEMS(video_rate_abbrs);

	/* First, we check our abbreviation table */
	for (i = 0; i < n; ++i)
	if (!strcmp(video_rate_abbrs[i].abbr, arg)) {
		*rate = video_rate_abbrs[i].rate;
		return 0;
	}

	/* Then, we try to parse it as fraction */
	if ((ret = av_parse_ratio_quiet(rate, arg, 1001000)) < 0)
		return ret;
	if (rate->num <= 0 || rate->den <= 0)
		return AVERROR(EINVAL);
	return 0;
}

int av_parse_ratio(AVRational *q, const char *str, int max,
	int log_offset, void *log_ctx)
{
	char c;
	int ret;

	if (sscanf(str, "%d:%d%c", &q->num, &q->den, &c) != 2) {
		double d;
		ret = av_expr_parse_and_eval(&d, str, NULL, NULL,
			NULL, NULL, NULL, NULL,
			NULL, log_offset, log_ctx);
		if (ret < 0)
			return ret;
		*q = av_d2q(d, max);
	}
	else {
		av_reduce(&q->num, &q->den, q->num, q->den, max);
	}

	return 0;
}

static double eval_expr(Parser *p, AVExpr *e)
{
	switch (e->type) {
	case AVExpr::e_value:  return e->value;
	case AVExpr::e_const:  return e->value * p->const_values[e->a.const_index];
	case AVExpr::e_func0:  return e->value * e->a.func0(eval_expr(p, e->param[0]));
	case AVExpr::e_func1:  return e->value * e->a.func1(p->opaque, eval_expr(p, e->param[0]));
	case AVExpr::e_func2:  return e->value * e->a.func2(p->opaque, eval_expr(p, e->param[0]), eval_expr(p, e->param[1]));
	case AVExpr::e_squish: return 1 / (1 + exp(4 * eval_expr(p, e->param[0])));
	case AVExpr::e_gauss: { double d = eval_expr(p, e->param[0]); return exp(-d*d / 2) / sqrt(2 * M_PI); }
	case AVExpr::e_ld:     return e->value * p->var[av_clip(eval_expr(p, e->param[0]), 0, VARS - 1)];
	case AVExpr::e_isnan:  return e->value * !!isnan(eval_expr(p, e->param[0]));
	case AVExpr::e_isinf:  return e->value * !!isinf(eval_expr(p, e->param[0]));
	case AVExpr::e_floor:  return e->value * floor(eval_expr(p, e->param[0]));
	case AVExpr::e_ceil:  return e->value * ceil(eval_expr(p, e->param[0]));
	case AVExpr::e_trunc:  return e->value * trunc(eval_expr(p, e->param[0]));
	case AVExpr::e_round:  return e->value * round(eval_expr(p, e->param[0]));
	case AVExpr::e_sgn:    return e->value * FFDIFFSIGN(eval_expr(p, e->param[0]), 0);
	case AVExpr::e_sqrt:   return e->value * sqrt(eval_expr(p, e->param[0]));
	case AVExpr::e_not:    return e->value * (eval_expr(p, e->param[0]) == 0);
	case AVExpr::e_if:     return e->value * (eval_expr(p, e->param[0]) ? eval_expr(p, e->param[1]) :
		e->param[2] ? eval_expr(p, e->param[2]) : 0);
	case AVExpr::e_ifnot:  return e->value * (!eval_expr(p, e->param[0]) ? eval_expr(p, e->param[1]) :
		e->param[2] ? eval_expr(p, e->param[2]) : 0);
	case AVExpr::e_clip: {
			     double x = eval_expr(p, e->param[0]);
			     double min = eval_expr(p, e->param[1]), max = eval_expr(p, e->param[2]);
			     if (isnan(min) || isnan(max) || isnan(x) || min > max)
				     return NAN;
			     return e->value * av_clipd(eval_expr(p, e->param[0]), min, max);
	}
	case AVExpr::e_between: {
				double d = eval_expr(p, e->param[0]);
				return e->value * (d >= eval_expr(p, e->param[1]) &&
					d <= eval_expr(p, e->param[2]));
	}
	case AVExpr::e_lerp: {
			     double v0 = eval_expr(p, e->param[0]);
			     double v1 = eval_expr(p, e->param[1]);
			     double f = eval_expr(p, e->param[2]);
			     return v0 + (v1 - v0) * f;
	}
	case AVExpr::e_print: {
			      double x = eval_expr(p, e->param[0]);
			      int level = e->param[1] ? av_clip(eval_expr(p, e->param[1]), INT_MIN, INT_MAX) : AV_LOG_INFO;
			      fprintf(stderr, "p: %f\n", x);
			      return x;
	}
	case AVExpr::e_random:{
			      int idx = av_clip(eval_expr(p, e->param[0]), 0, VARS - 1);
			      uint64_t r = isnan(p->var[idx]) ? 0 : p->var[idx];
			      r = r * 1664525 + 1013904223;
			      p->var[idx] = r;
			      return e->value * (r * (1.0 / UINT64_MAX));
	}
	case AVExpr::e_while: {
			      double d = NAN;
			      while (eval_expr(p, e->param[0]))
				      d = eval_expr(p, e->param[1]);
			      return d;
	}
	case AVExpr::e_taylor: {
			       double t = 1, d = 0, v;
			       double x = eval_expr(p, e->param[1]);
			       int id = e->param[2] ? av_clip(eval_expr(p, e->param[2]), 0, VARS - 1) : 0;
			       int i;
			       double var0 = p->var[id];
			       for (i = 0; i<1000; i++) {
				       double ld = d;
				       p->var[id] = i;
				       v = eval_expr(p, e->param[0]);
				       d += t*v;
				       if (ld == d && v)
					       break;
				       t *= x / (i + 1);
			       }
			       p->var[id] = var0;
			       return d;
	}
	case AVExpr::e_root: {
			     int i, j;
			     double low = -1, high = -1, v, low_v = -DBL_MAX, high_v = DBL_MAX;
			     double var0 = p->var[0];
			     double x_max = eval_expr(p, e->param[1]);
			     for (i = -1; i<1024; i++) {
				     if (i<255) {
					     p->var[0] = ff_reverse[i & 255] * x_max / 255;
				     }
				     else {
					     p->var[0] = x_max*pow(0.9, i - 255);
					     if (i & 1) p->var[0] *= -1;
					     if (i & 2) p->var[0] += low;
					     else     p->var[0] += high;
				     }
				     v = eval_expr(p, e->param[0]);
				     if (v <= 0 && v>low_v) {
					     low = p->var[0];
					     low_v = v;
				     }
				     if (v >= 0 && v<high_v) {
					     high = p->var[0];
					     high_v = v;
				     }
				     if (low >= 0 && high >= 0){
					     for (j = 0; j<1000; j++) {
						     p->var[0] = (low + high)*0.5;
						     if (low == p->var[0] || high == p->var[0])
							     break;
						     v = eval_expr(p, e->param[0]);
						     if (v <= 0) low = p->var[0];
						     if (v >= 0) high = p->var[0];
						     if (isnan(v)) {
							     low = high = v;
							     break;
						     }
					     }
					     break;
				     }
			     }
			     p->var[0] = var0;
			     return -low_v<high_v ? low : high;
	}
	default: {
			 double d = eval_expr(p, e->param[0]);
			 double d2 = eval_expr(p, e->param[1]);
			 switch (e->type) {
			 case AVExpr::e_mod: return e->value * (d - floor((!CONFIG_FTRAPV || d2) ? d / d2 : d * INFINITY) * d2);
			 case AVExpr::e_gcd: return e->value * av_gcd(d, d2);
			 case AVExpr::e_max: return e->value * (d >  d2 ? d : d2);
			 case AVExpr::e_min: return e->value * (d <  d2 ? d : d2);
			 case AVExpr::e_eq:  return e->value * (d == d2 ? 1.0 : 0.0);
			 case AVExpr::e_gt:  return e->value * (d >  d2 ? 1.0 : 0.0);
			 case AVExpr::e_gte: return e->value * (d >= d2 ? 1.0 : 0.0);
			 case AVExpr::e_lt:  return e->value * (d <  d2 ? 1.0 : 0.0);
			 case AVExpr::e_lte: return e->value * (d <= d2 ? 1.0 : 0.0);
			 case AVExpr::e_pow: return e->value * pow(d, d2);
			 case AVExpr::e_mul: return e->value * (d * d2);
			 case AVExpr::e_div: return e->value * ((!CONFIG_FTRAPV || d2) ? (d / d2) : d * INFINITY);
			 case AVExpr::e_add: return e->value * (d + d2);
			 case AVExpr::e_last:return e->value * d2;
			 case AVExpr::e_st: return e->value * (p->var[av_clip(d, 0, VARS - 1)] = d2);
			 case AVExpr::e_hypot:return e->value * hypot(d, d2);
			 case AVExpr::e_atan2:return e->value * atan2(d, d2);
			 case AVExpr::e_bitand: return isnan(d) || isnan(d2) ? NAN : e->value * ((long int)d & (long int)d2);
			 case AVExpr::e_bitor:  return isnan(d) || isnan(d2) ? NAN : e->value * ((long int)d | (long int)d2);
			 }
	}
	}
	return NAN;
}

int av_expr_parse_and_eval(double *d, const char *s,
	const char * const *const_names, const double *const_values,
	const char * const *func1_names, double(*const *funcs1)(void *, double),
	const char * const *func2_names, double(*const *funcs2)(void *, double, double),
	void *opaque, int log_offset, void *log_ctx)
{
	AVExpr *e = NULL;
	int ret = av_expr_parse(&e, s, const_names, func1_names, funcs1, func2_names, funcs2, log_offset, log_ctx);

	if (ret < 0) {
		*d = NAN;
		return ret;
	}
	*d = av_expr_eval(e, const_values, opaque);
	av_expr_free(e);
	return isnan(*d) ? AVERROR(EINVAL) : 0;
}

void av_expr_free(AVExpr *e)
{
	if (!e) return;
	av_expr_free(e->param[0]);
	av_expr_free(e->param[1]);
	av_expr_free(e->param[2]);
	av_freep(&e->var);
	av_freep(&e);
}

double av_expr_eval(AVExpr *e, const double *const_values, void *opaque)
{
	Parser p = { 0 };
	p.var = e->var;

	p.const_values = const_values;
	p.opaque = opaque;
	return eval_expr(&p, e);
}

double av_strtod(const char *numstr, char **tail)
{
	double d;
	char *next;
	if (numstr[0] == '0' && (numstr[1] | 0x20) == 'x') {
		d = strtoul(numstr, &next, 16);
	}
	else
		d = strtod(numstr, &next);
	/* if parsing succeeded, check for and interpret postfixes */
	if (next != numstr) {
		if (next[0] == 'd' && next[1] == 'B') {
			/* treat dB as decibels instead of decibytes */
			d = ff_exp10(d / 20);
			next += 2;
		}
		else if (*next >= 'E' && *next <= 'z') {
			int e = si_prefixes[*next - 'E'].exp;
			if (e) {
				if (next[1] == 'i') {
					d *= si_prefixes[*next - 'E'].bin_val;
					next += 2;
				}
				else {
					d *= si_prefixes[*next - 'E'].dec_val;
					next++;
				}
			}
		}

		if (*next == 'B') {
			d *= 8;
			next++;
		}
	}
	/* if requested, fill in tail with the position after the last parsed
	character */
	if (tail)
		*tail = next;
	return d;
}

#define IS_IDENTIFIER_CHAR(c) ((c) - '0' <= 9U || (c) - 'a' <= 25U || (c) - 'A' <= 25U || (c) == '_')

static int strmatch(const char *s, const char *prefix)
{
	int i;
	for (i = 0; prefix[i]; i++) {
		if (prefix[i] != s[i]) return 0;
	}
	/* return 1 only if the s identifier is terminated */
	return !IS_IDENTIFIER_CHAR(s[i]);
}

static double etime(double v)
{
	return av_gettime() * 0.000001;
}

static int parse_expr(AVExpr **e, Parser *p);

static int parse_primary(AVExpr **e, Parser *p)
{
	AVExpr *d = static_cast<AVExpr *>(av_mallocz(sizeof(AVExpr)));
	char *next = p->s, *s0 = p->s;
	int ret, i;

	if (!d)
		return AVERROR(ENOMEM);

	/* number */
	d->value = av_strtod(p->s, &next);
	if (next != p->s) {
		d->type = AVExpr::e_value;
		p->s = next;
		*e = d;
		return 0;
	}
	d->value = 1;

	/* named constants */
	for (i = 0; p->const_names && p->const_names[i]; i++) {
		if (strmatch(p->s, p->const_names[i])) {
			p->s += strlen(p->const_names[i]);
			d->type = AVExpr::e_const;
			d->a.const_index = i;
			*e = d;
			return 0;
		}
	}
	for (i = 0; i < FF_ARRAY_ELEMS(constants); i++) {
		if (strmatch(p->s, constants[i].name)) {
			p->s += strlen(constants[i].name);
			d->type = AVExpr::e_value;
			d->value = constants[i].value;
			*e = d;
			return 0;
		}
	}

	p->s = strchr(p->s, '(');
	if (!p->s) {
		fprintf(stderr, "p: Undefined constant or missing '(' in '%s'\n", s0);
		p->s = next;
		av_expr_free(d);
		return AVERROR(EINVAL);
	}
	p->s++; // "("
	if (*next == '(') { // special case do-nothing
		av_freep(&d);
		if ((ret = parse_expr(&d, p)) < 0)
			return ret;
		if (p->s[0] != ')') {
			fprintf(stderr, "p: Missing ')' in '%s'\n", s0);
			av_expr_free(d);
			return AVERROR(EINVAL);
		}
		p->s++; // ")"
		*e = d;
		return 0;
	}
	if ((ret = parse_expr(&(d->param[0]), p)) < 0) {
		av_expr_free(d);
		return ret;
	}
	if (p->s[0] == ',') {
		p->s++; // ","
		parse_expr(&d->param[1], p);
	}
	if (p->s[0] == ',') {
		p->s++; // ","
		parse_expr(&d->param[2], p);
	}
	if (p->s[0] != ')') {
		fprintf(stderr, "p: Missing ')' or too many args in '%s'\n", s0);
		av_expr_free(d);
		return AVERROR(EINVAL);
	}
	p->s++; // ")"

	d->type = AVExpr::e_func0;
	if (strmatch(next, "sinh")) d->a.func0 = sinh;
	else if (strmatch(next, "cosh")) d->a.func0 = cosh;
	else if (strmatch(next, "tanh")) d->a.func0 = tanh;
	else if (strmatch(next, "sin")) d->a.func0 = sin;
	else if (strmatch(next, "cos")) d->a.func0 = cos;
	else if (strmatch(next, "tan")) d->a.func0 = tan;
	else if (strmatch(next, "atan")) d->a.func0 = atan;
	else if (strmatch(next, "asin")) d->a.func0 = asin;
	else if (strmatch(next, "acos")) d->a.func0 = acos;
	else if (strmatch(next, "exp")) d->a.func0 = exp;
	else if (strmatch(next, "log")) d->a.func0 = log;
	else if (strmatch(next, "abs")) d->a.func0 = fabs;
	else if (strmatch(next, "time")) d->a.func0 = etime;
	else if (strmatch(next, "squish")) d->type = AVExpr::e_squish;
	else if (strmatch(next, "gauss")) d->type = AVExpr::e_gauss;
	else if (strmatch(next, "mod")) d->type = AVExpr::e_mod;
	else if (strmatch(next, "max")) d->type = AVExpr::e_max;
	else if (strmatch(next, "min")) d->type = AVExpr::e_min;
	else if (strmatch(next, "eq")) d->type = AVExpr::e_eq;
	else if (strmatch(next, "gte")) d->type = AVExpr::e_gte;
	else if (strmatch(next, "gt")) d->type = AVExpr::e_gt;
	else if (strmatch(next, "lte")) d->type = AVExpr::e_lte;
	else if (strmatch(next, "lt")) d->type = AVExpr::e_lt;
	else if (strmatch(next, "ld")) d->type = AVExpr::e_ld;
	else if (strmatch(next, "isnan")) d->type = AVExpr::e_isnan;
	else if (strmatch(next, "isinf")) d->type = AVExpr::e_isinf;
	else if (strmatch(next, "st")) d->type = AVExpr::e_st;
	else if (strmatch(next, "while")) d->type = AVExpr::e_while;
	else if (strmatch(next, "taylor")) d->type = AVExpr::e_taylor;
	else if (strmatch(next, "root")) d->type = AVExpr::e_root;
	else if (strmatch(next, "floor")) d->type = AVExpr::e_floor;
	else if (strmatch(next, "ceil")) d->type = AVExpr::e_ceil;
	else if (strmatch(next, "trunc")) d->type = AVExpr::e_trunc;
	else if (strmatch(next, "round")) d->type = AVExpr::e_round;
	else if (strmatch(next, "sqrt")) d->type = AVExpr::e_sqrt;
	else if (strmatch(next, "not")) d->type = AVExpr::e_not;
	else if (strmatch(next, "pow")) d->type = AVExpr::e_pow;
	else if (strmatch(next, "print")) d->type = AVExpr::e_print;
	else if (strmatch(next, "random")) d->type = AVExpr::e_random;
	else if (strmatch(next, "hypot")) d->type = AVExpr::e_hypot;
	else if (strmatch(next, "gcd")) d->type = AVExpr::e_gcd;
	else if (strmatch(next, "if")) d->type = AVExpr::e_if;
	else if (strmatch(next, "ifnot")) d->type = AVExpr::e_ifnot;
	else if (strmatch(next, "bitand")) d->type = AVExpr::e_bitand;
	else if (strmatch(next, "bitor")) d->type = AVExpr::e_bitor;
	else if (strmatch(next, "between"))d->type = AVExpr::e_between;
	else if (strmatch(next, "clip")) d->type = AVExpr::e_clip;
	else if (strmatch(next, "atan2")) d->type = AVExpr::e_atan2;
	else if (strmatch(next, "lerp")) d->type = AVExpr::e_lerp;
	else if (strmatch(next, "sgn")) d->type = AVExpr::e_sgn;
	else {
		for (i = 0; p->func1_names && p->func1_names[i]; i++) {
			if (strmatch(next, p->func1_names[i])) {
				d->a.func1 = p->funcs1[i];
				d->type = AVExpr::e_func1;
				*e = d;
				return 0;
			}
		}

		for (i = 0; p->func2_names && p->func2_names[i]; i++) {
			if (strmatch(next, p->func2_names[i])) {
				d->a.func2 = p->funcs2[i];
				d->type = AVExpr::e_func2;
				*e = d;
				return 0;
			}
		}

		fprintf(stderr, "p: Unknown function in '%s'\n", s0);
		av_expr_free(d);
		return AVERROR(EINVAL);
	}

	*e = d;
	return 0;
}

static AVExpr *make_eval_expr(int type, int value, AVExpr *p0, AVExpr *p1)
{
	AVExpr *e = static_cast<AVExpr *>(av_mallocz(sizeof(AVExpr)));
	if (!e)
		return NULL;
	e->type = static_cast<AVExpr::type_>(type);
	e->value = value;
	e->param[0] = p0;
	e->param[1] = p1;
	return e;
}

static int parse_pow(AVExpr **e, Parser *p, int *sign)
{
	*sign = (*p->s == '+') - (*p->s == '-');
	p->s += *sign & 1;
	return parse_primary(e, p);
}

static int parse_dB(AVExpr **e, Parser *p, int *sign)
{
	/* do not filter out the negative sign when parsing a dB value.
	for example, -3dB is not the same as -(3dB) */
	if (*p->s == '-') {
		char *next;
		double ignored = strtod(p->s, &next);
		if (next != p->s && next[0] == 'd' && next[1] == 'B') {
			*sign = 0;
			return parse_primary(e, p);
		}
	}
	return parse_pow(e, p, sign);
}

static int parse_factor(AVExpr **e, Parser *p)
{
	int sign, sign2, ret;
	AVExpr *e0, *e1, *e2;
	if ((ret = parse_dB(&e0, p, &sign)) < 0)
		return ret;
	while (p->s[0] == '^'){
		e1 = e0;
		p->s++;
		if ((ret = parse_dB(&e2, p, &sign2)) < 0) {
			av_expr_free(e1);
			return ret;
		}
		e0 = make_eval_expr(AVExpr::e_pow, 1, e1, e2);
		if (!e0) {
			av_expr_free(e1);
			av_expr_free(e2);
			return AVERROR(ENOMEM);
		}
		if (e0->param[1]) e0->param[1]->value *= (sign2 | 1);
	}
	if (e0) e0->value *= (sign | 1);

	*e = e0;
	return 0;
}

static int parse_term(AVExpr **e, Parser *p)
{
	int ret;
	AVExpr *e0, *e1, *e2;
	if ((ret = parse_factor(&e0, p)) < 0)
		return ret;
	while (p->s[0] == '*' || p->s[0] == '/') {
		int c = *p->s++;
		e1 = e0;
		if ((ret = parse_factor(&e2, p)) < 0) {
			av_expr_free(e1);
			return ret;
		}
		e0 = make_eval_expr(c == '*' ? AVExpr::e_mul : AVExpr::e_div, 1, e1, e2);
		if (!e0) {
			av_expr_free(e1);
			av_expr_free(e2);
			return AVERROR(ENOMEM);
		}
	}
	*e = e0;
	return 0;
}

static int parse_subexpr(AVExpr **e, Parser *p)
{
	int ret;
	AVExpr *e0, *e1, *e2;
	if ((ret = parse_term(&e0, p)) < 0)
		return ret;
	while (*p->s == '+' || *p->s == '-') {
		e1 = e0;
		if ((ret = parse_term(&e2, p)) < 0) {
			av_expr_free(e1);
			return ret;
		}
		e0 = make_eval_expr(AVExpr::e_add, 1, e1, e2);
		if (!e0) {
			av_expr_free(e1);
			av_expr_free(e2);
			return AVERROR(ENOMEM);
		}
	};

	*e = e0;
	return 0;
}

static int parse_expr(AVExpr **e, Parser *p)
{
	int ret;
	AVExpr *e0, *e1, *e2;
	if (p->stack_index <= 0) //protect against stack overflows
		return AVERROR(EINVAL);
	p->stack_index--;

	if ((ret = parse_subexpr(&e0, p)) < 0)
		return ret;
	while (*p->s == ';') {
		p->s++;
		e1 = e0;
		if ((ret = parse_subexpr(&e2, p)) < 0) {
			av_expr_free(e1);
			return ret;
		}
		e0 = make_eval_expr(AVExpr::e_last, 1, e1, e2);
		if (!e0) {
			av_expr_free(e1);
			av_expr_free(e2);
			return AVERROR(ENOMEM);
		}
	};

	p->stack_index++;
	*e = e0;
	return 0;
}

static int verify_expr(AVExpr *e)
{
	if (!e) return 0;
	switch (e->type) {
	case AVExpr::e_value:
	case AVExpr::e_const: return 1;
	case AVExpr::e_func0:
	case AVExpr::e_func1:
	case AVExpr::e_squish:
	case AVExpr::e_ld:
	case AVExpr::e_gauss:
	case AVExpr::e_isnan:
	case AVExpr::e_isinf:
	case AVExpr::e_floor:
	case AVExpr::e_ceil:
	case AVExpr::e_trunc:
	case AVExpr::e_round:
	case AVExpr::e_sqrt:
	case AVExpr::e_not:
	case AVExpr::e_random:
	case AVExpr::e_sgn:
		return verify_expr(e->param[0]) && !e->param[1];
	case AVExpr::e_print:
		return verify_expr(e->param[0])
			&& (!e->param[1] || verify_expr(e->param[1]));
	case AVExpr::e_if:
	case AVExpr::e_ifnot:
	case AVExpr::e_taylor:
		return verify_expr(e->param[0]) && verify_expr(e->param[1])
			&& (!e->param[2] || verify_expr(e->param[2]));
	case AVExpr::e_between:
	case AVExpr::e_clip:
	case AVExpr::e_lerp:
		return verify_expr(e->param[0]) &&
			verify_expr(e->param[1]) &&
			verify_expr(e->param[2]);
	default: return verify_expr(e->param[0]) && verify_expr(e->param[1]) && !e->param[2];
	}
}

int av_expr_parse(AVExpr **expr, const char *s,
	const char * const *const_names,
	const char * const *func1_names, double(*const *funcs1)(void *, double),
	const char * const *func2_names, double(*const *funcs2)(void *, double, double),
	int log_offset, void *log_ctx)
{
	Parser p = { 0 };
	AVExpr *e = NULL;
	char *w = static_cast<char*>(av_malloc(strlen(s) + 1));
	char *wp = w;
	const char *s0 = s;
	int ret = 0;

	if (!w)
		return AVERROR(ENOMEM);

	while (*s)
	if (!av_isspace(*s++)) *wp++ = s[-1];
	*wp++ = 0;

	p.class_ = &eval_class;
	p.stack_index = 100;
	p.s = w;
	p.const_names = const_names;
	p.funcs1 = funcs1;
	p.func1_names = func1_names;
	p.funcs2 = funcs2;
	p.func2_names = func2_names;
	p.log_offset = log_offset;
	p.log_ctx = log_ctx;

	if ((ret = parse_expr(&e, &p)) < 0)
		goto end;
	if (*p.s) {
		fprintf(stderr, "p: Invalid chars '%s' at the end of expression '%s'\n", p.s, s0);
		ret = AVERROR(EINVAL);
		goto end;
	}
	if (!verify_expr(e)) {
		ret = AVERROR(EINVAL);
		goto end;
	}
	e->var = static_cast<double*>(av_mallocz(sizeof(double)*VARS));
	if (!e->var) {
		ret = AVERROR(ENOMEM);
		goto end;
	}
	*expr = e;
	e = NULL;
end:
	av_expr_free(e);
	av_free(w);
	return ret;
}

int64_t av_gettime(void)
{
#if HAVE_GETTIMEOFDAY
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#elif HAVE_GETSYSTEMTIMEASFILETIME
	FILETIME ft;
	int64_t t;
	GetSystemTimeAsFileTime(&ft);
	t = (int64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
	return t / 10 - 11644473600000000; /* Jan 1, 1601 */
#else
	return -1;
#endif
}

typedef struct ColorEntry {
	const char *name;            ///< a string representing the name of the color
	uint8_t     rgb_color[3];    ///< RGB values for the color
} ColorEntry;

#define ALPHA_SEP '@'

int av_parse_color(uint8_t *rgba_color, const char *color_string, int slen,
	void *log_ctx)
{
	ERROR_POS
	return 0;
}

size_t av_strlcpy(char *dst, const char *src, size_t size)
{
	size_t len = 0;
	while (++len < size && *src)
		*dst++ = *src++;
	if (len <= size)
		*dst = 0;
	return len + strlen(src) - 1;
}

int av_match_name(const char *name, const char *names)
{
	const char *p;
	int len, namelen;

	if (!name || !names)
		return 0;

	namelen = strlen(name);
	while (*names) {
		int negate = '-' == *names;
		p = strchr(names, ',');
		if (!p)
			p = names + strlen(names);
		names += negate;
		len = FFMAX(p - names, namelen);
		if (!av_strncasecmp(name, names, len) || !strncmp("ALL", names, FFMAX(3, p - names)))
			return !negate;
		names = p + (*p == ',');
	}
	return 0;
}

int av_strncasecmp(const char *a, const char *b, size_t n)
{
	uint8_t c1, c2;
	if (n <= 0)
		return 0;
	do {
		c1 = av_tolower(*a++);
		c2 = av_tolower(*b++);
	} while (--n && c1 && c1 == c2);
	return c1 - c2;
}

int av_parse_video_size(int *width_ptr, int *height_ptr, const char *str)
{
	int i;
	int n = FF_ARRAY_ELEMS(video_size_abbrs);
	const char *p;
	int width = 0, height = 0;

	for (i = 0; i < n; i++) {
		if (!strcmp(video_size_abbrs[i].abbr, str)) {
			width = video_size_abbrs[i].width;
			height = video_size_abbrs[i].height;
			break;
		}
	}
	if (i == n) {
		width = strtol(str, const_cast<char**>(&p), 10);
		if (*p)
			p++;
		height = strtol(p, const_cast<char**>(&p), 10);

		/* trailing extraneous data detected, like in 123x345foobar */
		if (*p)
			return AVERROR(EINVAL);
	}
	if (width <= 0 || height <= 0)
		return AVERROR(EINVAL);
	*width_ptr = width;
	*height_ptr = height;
	return 0;
}

size_t av_strlcat(char *dst, const char *src, size_t size)
{
	size_t len = strlen(dst);
	if (size <= len + 1)
		return len + strlen(src);
	return len + av_strlcpy(dst + len, src, size - len);
}

} // namespace fbc

#endif // _MSC_VER
