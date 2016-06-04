// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_FAST_MATH_HPP_
#define FBC_CV_CORE_FAST_MATH_HPP_

// reference: include/opencv2/core/fast_math.hpp

#include "core/fbcdef.hpp"

#if (defined _MSC_VER && defined _M_X64 && _MSC_VER >= 1500)
	#include <nmmintrin.h>
#endif

namespace fbc {

// Rounds floating-point number to the nearest integer
static inline int fbcRound(double value)
{
#if (defined _MSC_VER && defined _M_X64)
	__m128d t = _mm_set_sd( value );
	return _mm_cvtsd_si32(t);
#else
	// it's ok if round does not comply with IEEE754 standard;
	// it should allow +/-1 difference when the other functions use round
	return (int)(value + (value >= 0 ? 0.5 : -0.5));
#endif
}

static inline int fbcRound(float value)
{
#if (defined _MSC_VER && defined _M_X64)
	__m128 t = _mm_set_ss(value);
	return _mm_cvtss_si32(t);
#else
	// it's ok if round does not comply with IEEE754 standard;
	// it should allow +/-1 difference when the other functions use round
	return (int)(value + (value >= 0 ? 0.5f : -0.5f));
#endif
}

static inline int fbcRound(int value)
{
	return value;
}

// Rounds floating-point number to the nearest integer not larger than the original
static inline int fbcFloor(double value)
{
#if (defined _MSC_VER && defined _M_X64)
	__m128d t = _mm_set_sd(value);
	int i = _mm_cvtsd_si32(t);
	return i - _mm_movemask_pd(_mm_cmplt_sd(t, _mm_cvtsi32_sd(t,i)));
#else
	int i = fbcRound(value);
	float diff = (float)(value - i);
	return i - (diff < 0);
#endif
}

static inline int fbcFloor(float value)
{
#if (defined _MSC_VER && defined _M_X64)
	__m128 t = _mm_set_ss(value);
	int i = _mm_cvtss_si32(t);
	return i - _mm_movemask_ps(_mm_cmplt_ss(t, _mm_cvtsi32_ss(t, i)));
#else
	int i = fbcRound(value);
	float diff = (float)(value - i);
	return i - (diff < 0);
#endif
}

static inline int fbcFloor(int value)
{
	return value;
}

// Rounds floating-point number to the nearest integer not smaller than the original
static inline int fbcCeil(double value)
{
#if (defined _MSC_VER && defined _M_X64)
	__m128d t = _mm_set_sd(value);
	int i = _mm_cvtsd_si32(t);
	return i + _mm_movemask_pd(_mm_cmplt_sd(_mm_cvtsi32_sd(t, i), t));
#else
	int i = fbcRound(value);
	float diff = (float)(i - value);
	return i + (diff < 0);
#endif 
}

static inline int fbcCeil(float value)
{
#if (defined _MSC_VER && defined _M_X64)
	__m128 t = _mm_set_ss(value);
	int i = _mm_cvtss_si32(t);
	return i + _mm_movemask_ps(_mm_cmplt_ss(_mm_cvtsi32_ss(t, i), t));
#else
	int i = fbcRound(value);
	float diff = (float)(i - value);
	return i + (diff < 0);
#endif
}

static inline int fbcCeil(int value)
{
	return value;
}

} // fbc

#endif // FBC_CV_CORE_FAST_MATH_HPP_
