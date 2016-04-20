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

template<typename _Tp> inline _Tp cv_abs(_Tp x) { return std::abs(x); }
inline int cv_abs(uchar x) { return x; }
inline int cv_abs(schar x) { return std::abs(x); }
inline int cv_abs(ushort x) { return x; }
inline int cv_abs(short x) { return std::abs(x); }

} //fbc

#endif //FBC_CV_CORE_BASE_HPP_
