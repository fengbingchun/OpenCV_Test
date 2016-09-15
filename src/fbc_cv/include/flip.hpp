// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_FLIP_HPP_
#define FBC_CV_FLIP_HPP_

/* reference: include/opencv2/core.hpp
              modules/core/src/copy.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"

namespace fbc {

// Flips a 2D array around vertical, horizontal, or both axes
// flipCode: 0 means flipping around the x - axis and positive value means flipping around y - axis.
//	     Negative value means flipping around both axes
// support type: uchar/float, multi-channels
template <typename _Tp, int chs>
int flip(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, int flipCode)
{
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.rows, src.cols);
	} else {
		FBC_Assert(src.rows == dst.rows && src.cols == dst.cols);
	}


	return 0;
}

} // namespace fbc

#endif // FBC_CV_FLIP_HPP_
