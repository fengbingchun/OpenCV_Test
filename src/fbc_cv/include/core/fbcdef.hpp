// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_FBCDEF_HPP_
#define FBC_CV_CORE_FBCDEF_HPP_

/* reference: include/opencv2/core/cvdef.h
              include/opencv2/core/typedef_c.h
*/

#include "interface.hpp"

#ifdef _MSC_VER
	#define FBC_EXPORTS __declspec(dllexport)
	#define FBC_DECL_ALIGNED(x) __declspec(align(x))
#else
	#define FBC_EXPORTS __attribute__((visibility("default")))
	#define FBC_DECL_ALIGNED(x) __attribute__((aligned(x)))
#endif

namespace fbc {

#define FBC_CN_MAX		512
#define FBC_CN_SHIFT		3
#define FBC_DEPTH_MAX		(1 << FBC_CN_SHIFT)

#define FBC_MAT_TYPE_MASK	(FBC_DEPTH_MAX*FBC_CN_MAX - 1)
#define FBC_MAT_TYPE(flags)	((flags) & FBC_MAT_TYPE_MASK)

#ifndef MIN
	#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
	#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#define FBC_CN_MAX  512

// Common macros and inline functions
#define FBC_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

/** min & max without jumps */
#define  FBC_IMIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))
#define  FBC_IMAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

// fundamental constants
#define FBC_PI 3.1415926535897932384626433832795

// Note: No practical significance
class dump {};

typedef union Cv32suf {
	int i;
	unsigned u;
	float f;
} Cv32suf;

typedef union Cv64suf {
	int64 i;
	fbc::uint64 u;
	double f;
} Cv64suf;

} // namespace fbc

#endif // FBC_CV_CORE_FBCDEF_HPP_
