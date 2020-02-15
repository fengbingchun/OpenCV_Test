// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_MATHEMATICS_HPP_
#define FBC_CV_MATHEMATICS_HPP_

// reference: ffmpeg 4.2
//            libavutil/mathematics.h

#ifdef _MSC_VER

#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include "ffmpeg_common.hpp"
#include "avrational.hpp"

namespace fbc {

#ifndef M_E
#define M_E            2.7182818284590452354   /* e */
#endif
#ifndef M_LN2
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif
#ifndef M_LN10
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#endif
#ifndef M_LOG2_10
#define M_LOG2_10      3.32192809488736234787  /* log_2 10 */
#endif
#ifndef M_PHI
#define M_PHI          1.61803398874989484820   /* phi / golden ratio */
#endif
#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif
#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif
#ifndef NAN
#define NAN            av_int2float(0x7fc00000)
#endif
#ifndef INFINITY
#define INFINITY       av_int2float(0x7f800000)
#endif

#define ff_ctzll(v) _tzcnt_u64(v)

enum AVRounding {
	AV_ROUND_ZERO = 0, ///< Round toward zero.
	AV_ROUND_INF = 1, ///< Round away from zero.
	AV_ROUND_DOWN = 2, ///< Round toward -infinity.
	AV_ROUND_UP = 3, ///< Round toward +infinity.
	AV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
	/**
	* Flag telling rescaling functions to pass `INT64_MIN`/`MAX` through
	* unchanged, avoiding special cases for #AV_NOPTS_VALUE.
	*
	* Unlike other values of the enumeration AVRounding, this value is a
	* bitmask that must be used in conjunction with another value of the
	* enumeration through a bitwise OR, in order to set behavior for normal
	* cases.
	*
	* @code{.c}
	* av_rescale_rnd(3, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
	* // Rescaling 3:
	* //     Calculating 3 * 1 / 2
	* //     3 / 2 is rounded up to 2
	* //     => 2
	*
	* av_rescale_rnd(AV_NOPTS_VALUE, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
	* // Rescaling AV_NOPTS_VALUE:
	* //     AV_NOPTS_VALUE == INT64_MIN
	* //     AV_NOPTS_VALUE is passed through
	* //     => AV_NOPTS_VALUE
	* @endcode
	*/
	AV_ROUND_PASS_MINMAX = 8192,
};

int64_t  av_gcd(int64_t a, int64_t b);
int64_t av_rescale(int64_t a, int64_t b, int64_t c);
int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd);
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq);
int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq, enum AVRounding rnd);

static av_always_inline double ff_exp10(double x)
{
	return exp2(M_LOG2_10 * x);
}

static av_always_inline float ff_exp10f(float x)
{
	return exp2f(M_LOG2_10 * x);
}

int64_t av_add_stable(AVRational ts_tb, int64_t ts, AVRational inc_tb, int64_t inc);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_MATHEMATICS_HPP_
