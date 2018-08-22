// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_INTERFACE_HPP_
#define FBC_CV_CORE_INTERFACE_HPP_

// reference: include/opencv2/core/hal/interface.h
#include <stdint.h>

namespace fbc {

/* primitive types */
/*
schar  - signed 1 byte integer
uchar  - unsigned 1 byte integer
short  - signed 2 byte integer
ushort - unsigned 2 byte integer
int    - signed 4 byte integer
uint   - unsigned 4 byte integer
int64  - signed 8 byte integer
uint64 - unsigned 8 byte integer
*/

typedef unsigned int uint;
typedef signed char schar;
typedef unsigned char uchar;
typedef unsigned short ushort;
#ifdef _MSC_VER
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#else
	typedef int64_t int64;
	typedef uint64_t uint64;
#endif

} // fbc

#endif // FBC_CV_CORE_INTERFACE_HPP_
