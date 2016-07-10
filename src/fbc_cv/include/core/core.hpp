// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_CORE_HPP_
#define FBC_CV_CORE_CORE_HPP_

/* reference: include/opencv2/core/core_c.h
              include/opencv2/core.hpp
*/

#ifndef __cplusplus
	#error core.hpp header must be compiled as C++
#endif

#include <exception>
#include <string>
#include "core/fbcdef.hpp"

namespace fbc {

// Fast cubic root calculation
FBC_EXPORTS float fbcCbrt(float value);

// Computes the source location of an extrapolated pixel
FBC_EXPORTS int borderInterpolate(int p, int len, int borderType);

} // namespace fbc

#endif // FBC_CV_CORE_CORE_HPP_
