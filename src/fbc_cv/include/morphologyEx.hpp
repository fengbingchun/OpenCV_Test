// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_MORPHOLOGYEX_HPP_
#define FBC_CV_MORPHOLOGYEX_HPP_

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/morph.cpp
*/

#include <typeinfo>
#include "erode.hpp"
#include "dilate.hpp"

namespace fbc {

// perform advanced morphological transformations using an erosion and dilation as basic operations
// In case of multi - channel images, each channel is processed independently.
// morphologyEx can be applied several ( iterations ) times.
// op ==> enum MorphTypes
// support type: uchar/float, multi-channels
template<typename _Tp, int chs>
int morphologyEx(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, int op, const Mat_<uchar, 1>& kernel,
	Point anchor = Point(-1, -1), int iterations = 1, int borderType = BORDER_CONSTANT, const Scalar& borderValue = Scalar::all(DBL_MAX))
{
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.rows, src.cols);
	} else {
		FBC_Assert(src.rows == dst.rows && src.cols == dst.cols);
	}

	Mat_<uchar, 1> kernel_ = kernel;
	if (kernel_.empty()) {
		kernel_ = Mat_<uchar, 1>(3, 3);
		getStructuringElement(kernel_, MORPH_RECT, Size(3, 3), Point(1, 1));
	}

	switch (op) {
		case MORPH_ERODE: {
			erode(src, dst, kernel_, anchor, iterations, borderType, borderValue);
			break;
		}
		case MORPH_DILATE: {
			dilate(src, dst, kernel_, anchor, iterations, borderType, borderValue);
			break;
		}
		case MORPH_OPEN: {
			erode(src, dst, kernel_, anchor, iterations, borderType, borderValue);
			dilate(dst, dst, kernel_, anchor, iterations, borderType, borderValue);
			break;
		}
		case MORPH_CLOSE: {
			dilate(src, dst, kernel_, anchor, iterations, borderType, borderValue);
			erode(dst, dst, kernel_, anchor, iterations, borderType, borderValue);
			break;
		}
		case MORPH_GRADIENT: {
			Mat_<_Tp, chs> temp(src.rows, src.cols);
			erode(src, temp, kernel_, anchor, iterations, borderType, borderValue);
			dilate(src, dst, kernel_, anchor, iterations, borderType, borderValue);
			dst -= temp;
			break;
		}
		case MORPH_TOPHAT: {
			Mat_<_Tp, chs> temp(src.rows, src.cols);
			if (src.data != dst.data)
				temp = dst;
			erode(src, temp, kernel_, anchor, iterations, borderType, borderValue);
			dilate(temp, temp, kernel_, anchor, iterations, borderType, borderValue);
			dst = src - temp;
			break;
		}
		case MORPH_BLACKHAT: {
			Mat_<_Tp, chs> temp(src.rows, src.cols);
			if (src.data != dst.data)
				temp = dst;
			dilate(src, temp, kernel_, anchor, iterations, borderType, borderValue);
			erode(temp, temp, kernel_, anchor, iterations, borderType, borderValue);
			dst = temp - src;
			break;
		}
		case MORPH_HITMISS: {
			FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() && chs == 1);
			Mat_<uchar, 1> k1 = (kernel_ == Mat_<uchar, 1>(kernel_.rows, kernel_.cols, Scalar::all(1)));
			Mat_<uchar, 1> k2 = (kernel_ == Mat_<int, 1>(kernel_.rows, kernel_.cols, Scalar::all(-1)));
			Mat_<_Tp, chs> e1, e2;

			if (countNonZero(k1) <= 0)
				e1 = src;
			else
				erode(src, e1, k1, anchor, iterations, borderType, borderValue);
			if (countNonZero(k2) <= 0) {
				e2 = src;
			} else {
				Mat_<_Tp, chs> src_complement;
				bitwise_not(src, src_complement);
				erode(src_complement, e2, k2, anchor, iterations, borderType, borderValue);
			}
			bitwise_and(e1, e2, dst);
			break;
		}
		default:
			FBC_Assert("unknown morphological operation");
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_MORPHOLOGYEX_HPP_
