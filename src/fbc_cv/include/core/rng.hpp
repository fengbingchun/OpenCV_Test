// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_RNG_HPP_
#define FBC_CV_CORE_RNG_HPP_

/* reference: core/include/opencv2/core.hpp
              core/include/opencv2/core/operations.hpp
	      modules/core/src/rand.cpp
*/

#include "mat.hpp"

namespace fbc {

// Random Number Generator
// It encapsulates the state (currently, a 64-bit integer) and has methods to return scalar random values and to fill arrays with random values.
// Currently it supports uniform and Gaussian (normal) distributions.
template<typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
class RNG {
public:
	enum {
		UNIFORM = 0,
		NORMAL = 1
	};

	// constructor
	RNG() { state = 0xffffffff; }
	// 64-bit value used to initialize the RNG
	RNG(uint64 _state) { state = _state ? _state : 0xffffffff; }
	// updates the state using the MWC algorithm and returns the next 32 - bit random number
	unsigned next()
	{
		state = (uint64)(unsigned)state * 4164903690U + (unsigned)(state >> 32);
		return (unsigned)state;
	}
	// Each of the methods updates the state using the MWC algorithm and returns the next random number of the specified type
	// In case of integer types, the returned number is from the available value range for the specified type
	// In case of floating-point types, the returned value is from[0, 1) range
	operator uchar() { return (uchar)next(); }
	operator schar() { return (schar)next(); }
	operator ushort() { return (ushort)next(); }
	operator short() { return (short)next(); }
	operator unsigned() { return next(); }
	operator int() { return (int)next(); }
	operator float() { return next()*2.3283064365386962890625e-10f; }
	operator double() { unsigned t = next(); return (((uint64)t << 32) | next()) * 5.4210108624275221700372640043497e-20; }

	// returns a random integer sampled uniformly from [0, N)
	unsigned operator ()() { return next(); }
	unsigned operator ()(unsigned N) { return (unsigned)uniform(0, N); }

	// returns uniformly distributed integer random number from [a,b) range
	int uniform(int a, int b) { return a == b ? a : (int)(next() % (b - a) + a); }
	float uniform(float a, float b) { return ((float)*this)*(b - a) + a; }
	double uniform(double a, double b) { return ((double)*this)*(b - a) + a; }

	// Fills arrays with random numbers
	void fill(Mat_<_Tp1, chs1>& _mat, int distType, const Mat_<_Tp2, chs2>& _param1arg, const Mat_<_Tp2, chs3>& _param2arg, bool saturateRange = false); // TODO

	// Returns the next random number sampled from the Gaussian distribution
	double gaussian(double sigma); // TODO

	uint64 state;
};

} // namespace fbc

#endif // FBC_CV_CORE_RNG_HPP_
