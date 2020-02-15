// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVRATIONAL_HPP_
#define FBC_CV_AVRATIONAL_HPP_

// reference: ffmpeg 4.2
//            libavutil/rational.h

#ifdef _MSC_VER

#include <limits.h>
#include "ffmpeg_common.hpp"

namespace fbc {

typedef struct AVRational{
	int num; ///< Numerator
	int den; ///< Denominator
} AVRational;

AVRational av_mul_q(AVRational b, AVRational c);
int av_reduce(int *dst_num, int *dst_den, int64_t num, int64_t den, int64_t max);

static inline int av_cmp_q(AVRational a, AVRational b){
	const int64_t tmp = a.num * (int64_t)b.den - b.num * (int64_t)a.den;

	if (tmp) return (int)((tmp ^ a.den ^ b.den) >> 63) | 1;
	else if (b.den && a.den) return 0;
	else if (a.num && b.num) return (a.num >> 31) - (b.num >> 31);
	else                    return INT_MIN;
}

static av_always_inline AVRational av_inv_q(AVRational q)
{
	AVRational r = { q.den, q.num };
	return r;
}

AVRational av_d2q(double d, int max);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVRATIONAL_HPP_
