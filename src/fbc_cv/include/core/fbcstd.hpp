// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_FBCSTD_HPP_
#define FBC_CV_CORE_FBCSTD_HPP_

// reference: include/opencv2/core/cvstd.hpp

#include "fbcdef.hpp"

#ifndef __cplusplus
	#error fbcstd.hpp header must be compiled as C++
#endif

namespace fbc {

/* the alignment of all the allocated buffers */
#define  FBC_MALLOC_ALIGN    16

// Allocates an aligned memory buffer
FBC_EXPORTS void* fastMalloc(size_t size);
// Deallocates a memory buffer
FBC_EXPORTS void fastFree(void* ptr);

} // namespace fbc

#endif // FBC_CV_CORE_FBCSTD_HPP_
