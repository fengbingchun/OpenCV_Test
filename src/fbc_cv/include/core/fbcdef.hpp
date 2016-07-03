// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_FBCDEF_HPP_
#define FBC_CV_CORE_FBCDEF_HPP_

// reference: include/opencv2/core/cvdef.h

#include "core/interface.hpp"

#define FBC_EXPORTS __declspec(dllexport)
#define FBC_DECL_ALIGNED(x) __declspec(align(x))

namespace fbc {

#ifndef MIN
	#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
	#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#define FBC_CN_MAX  512

// fundamental constants
#define FBC_PI 3.1415926535897932384626433832795

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
