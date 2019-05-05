// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_FAST_MATH_HPP_
#define FBC_CV_CORE_FAST_MATH_HPP_

// reference: include/opencv2/core/fast_math.hpp

#include "fbcdef.hpp"

namespace fbc {

// Rounds floating-point number to the nearest integer
static inline int fbcRound(double value)
{
	// it's ok if round does not comply with IEEE754 standard;
	// it should allow +/-1 difference when the other functions use round
	return (int)(value + (value >= 0 ? 0.5 : -0.5));
}

static inline int fbcRound(float value)
{
	// it's ok if round does not comply with IEEE754 standard;
	// it should allow +/-1 difference when the other functions use round
	return (int)(value + (value >= 0 ? 0.5f : -0.5f));
}

static inline int fbcRound(int value)
{
	return value;
}

// Rounds floating-point number to the nearest integer not larger than the original
static inline int fbcFloor(double value)
{
	int i = fbcRound(value);
	float diff = (float)(value - i);
	return i - (diff < 0);
}

static inline int fbcFloor(float value)
{
	int i = fbcRound(value);
	float diff = (float)(value - i);
	return i - (diff < 0);
}

static inline int fbcFloor(int value)
{
	return value;
}

// Rounds floating-point number to the nearest integer not smaller than the original
static inline int fbcCeil(double value)
{
	int i = fbcRound(value);
	float diff = (float)(i - value);
	return i + (diff < 0);
}

static inline int fbcCeil(float value)
{
	int i = fbcRound(value);
	float diff = (float)(i - value);
	return i + (diff < 0);
}

static inline int fbcCeil(int value)
{
	return value;
}

} // fbc

#endif // FBC_CV_CORE_FAST_MATH_HPP_
