#ifndef FBC_CV_CORE_FAST_MATH_HPP_
#define FBC_CV_CORE_FAST_MATH_HPP_

// reference: include/opencv2/core/fast_math.hpp

#include "core/fbcdef.hpp"

namespace fbc {

// Rounds floating-point number to the nearest integer
int fbcRound(double value)
{
	return (int)(value + (value >= 0 ? 0.5 : -0.5));
}

int fbcRound(float value)
{
	return (int)(value + (value >= 0 ? 0.5f : -0.5f));
}

int fbcRound(int value)
{
	return value;
}

// Rounds floating-point number to the nearest integer not larger than the original
int fbcFloor(double value)
{
	int i = fbcRound(value);
	float diff = (float)(value - i);
	return i - (diff < 0);
}

int fbcFloor(float value)
{
	int i = fbcRound(value);
	float diff = (float)(value - i);
	return i - (diff < 0);
}

int fbcFloor(int value)
{
	return value;
}

// Rounds floating-point number to the nearest integer not smaller than the original
int fbcCeil(double value)
{
	int i = fbcRound(value);
	float diff = (float)(i - value);
	return i + (diff < 0);
}

int fbcCeil(float value)
{
	int i = fbcRound(value);
	float diff = (float)(i - value);
	return i + (diff < 0);
}

int fbcCeil(int value)
{
	return value;
}

} // fbc

#endif // FBC_CV_CORE_FAST_MATH_HPP_
