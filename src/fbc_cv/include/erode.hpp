// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_ERODE_HPP_
#define FBC_CV_ERODE_HPP_

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/morph.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "imgproc.hpp"
#include "filterengine.hpp"
#include "core/core.hpp"
#include "morph.hpp"

namespace fbc {

// Erodes an image by using a specific structuring element
// \f[\texttt{ dst } (x, y) = \min _{ (x',y') : \, \texttt{ element } (x',y') \ne0 } \texttt{ src } (x + x',y+y')\f]
// In case of multi - channel images, each channel is processed independently.
// Erosion can be applied several ( iterations ) times.
// support type: uchar/float, multi-channels
template<typename _Tp, int chs>
int erode(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, const Mat_<uchar, 1>& kernel,
	Point anchor = Point(-1, -1), int iterations = 1, int borderType = BORDER_CONSTANT, const Scalar& borderValue = Scalar::all(DBL_MAX))
{
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.rows, src.cols);
	} else {
		FBC_Assert(src.rows == dst.rows && src.cols == dst.cols);
	}

	Size ksize = !kernel.empty() ? kernel.size() : Size(3, 3);
	anchor = normalizeAnchor(anchor, ksize);

	if (iterations == 0 || kernel.rows * kernel.cols == 1) {
		src.copyTo(dst);
		return 0;
	}

	Mat_<uchar, 1> kernel_ = kernel;
	if (kernel_.empty()) {
		kernel_ = Mat_<uchar, 1>(1 + iterations * 2, 1 + iterations * 2);
		getStructuringElement(kernel_, MORPH_RECT, Size(1 + iterations * 2, 1 + iterations * 2));
		anchor = Point(iterations, iterations);
		iterations = 1;
	} else if (iterations > 1 && countNonZero(kernel_) == kernel_.rows * kernel_.cols) {
		anchor = Point(anchor.x*iterations, anchor.y*iterations);
		kernel_ = Mat_<uchar, 1>(ksize.height + (iterations - 1)*(ksize.height - 1), ksize.width + (iterations - 1)*(ksize.width - 1));
		getStructuringElement(kernel_, MORPH_RECT,
			Size(ksize.width + (iterations - 1)*(ksize.width - 1), ksize.height + (iterations - 1)*(ksize.height - 1)), anchor);
		iterations = 1;
	}

	anchor = normalizeAnchor(anchor, kernel_.size());

	Ptr<BaseRowFilter> rowFilter;
	Ptr<BaseColumnFilter> columnFilter;
	Ptr<BaseFilter> filter2D;

	if (countNonZero(kernel_) == kernel_.rows*kernel_.cols) {
		// rectangular structuring element
		rowFilter = getMorphologyRowFilter<_Tp, chs>(0, kernel_.cols, anchor.x);
		columnFilter = getMorphologyColumnFilter<_Tp, chs>(0, kernel_.rows, anchor.y);
	} else {
		filter2D = getMorphologyFilter<_Tp, chs>(0, kernel_, anchor);
	}

	Scalar borderValue_ = borderValue;
	if (borderType == BORDER_CONSTANT && borderValue_ == Scalar::all(DBL_MAX)) {
		if (sizeof(_Tp) == 1) // CV_8U
			borderValue_ = Scalar::all((double)UCHAR_MAX);
		else // CV_32F
			borderValue_ = Scalar::all((double)FLT_MAX);
	}

	Ptr<FilterEngine<_Tp, _Tp, _Tp, chs, chs, chs>> f = makePtr<FilterEngine<_Tp, _Tp, _Tp, chs, chs, chs>>(filter2D, rowFilter, columnFilter, borderType, borderType, borderValue_);
	f->apply(src, dst);
	for (int i = 1; i < iterations; i++)
		f->apply(dst, dst);

	return 0;
}

} // namespace fbc

#endif // FBC_CV_ERODE_HPP_
