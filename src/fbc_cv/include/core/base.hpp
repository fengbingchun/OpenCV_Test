// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_BASE_HPP_
#define FBC_CV_CORE_BASE_HPP_

// reference: include/opencv2/core/base.hpp

#ifndef __cplusplus
	#error base.hpp header must be compiled as C++
#endif

#include <assert.h>
#include <algorithm>

#include "core/fbcdef.hpp"
#include "core/interface.hpp"

namespace fbc {

#define FBC_StaticAssert(condition, reason)	static_assert((condition), reason " " #condition)
#define FBC_Assert(expr) assert(expr)
#define FBC_Error(msg) \
	fprintf(stderr, "Error: "#msg", file: %s, func: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__); \
	assert(0);

template<typename _Tp> inline _Tp fbc_abs(_Tp x) { return std::abs(x); }
inline int fbc_abs(uchar x) { return x; }
inline int fbc_abs(schar x) { return std::abs(x); }
inline int fbc_abs(ushort x) { return x; }
inline int fbc_abs(short x) { return std::abs(x); }

} //fbc

#endif //FBC_CV_CORE_BASE_HPP_
