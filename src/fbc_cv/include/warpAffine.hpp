// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_WARP_AFFINE_HPP_
#define FBC_CV_WARP_AFFINE_HPP_

/* reference: include/opencv2/imgproc.hpp
	      modules/imgproc/src/imgwarp.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "solve.hpp"
#include "imgproc.hpp"

namespace fbc {

// Calculates an affine transform from three pairs of the corresponding points
FBC_EXPORTS int getAffineTransform(const Point2f src1[], const Point2f src2[], Mat_<double, 1>& dst);

// Applies an affine transformation to an image
// The function cannot operate in - place
template<typename _Tp1, typename _Tp2, int chs1, int chs2>
int warpAffine(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst, const Mat_<_Tp2, chs2>& M_,
	int flags = INTER_LINEAR, int borderMode = BORDER_CONSTANT, const Scalar& borderValue = Scalar())
{
	FBC_Assert(src.data != NULL && dst.data != NULL && M_.data != NULL);
	FBC_Assert(src.cols > 0 && src.rows > 0 && dst.cols > 0 && dst.rows > 0 && M_.cols > 0 && M_.rows > 0);
	FBC_Assert(src.data != dst.data);

	double M[6];
	Mat_<double, 1> matM(2, 3, M);
	int interpolation = flags & INTER_MAX;
	if (interpolation == INTER_AREA)
		interpolation = INTER_LINEAR;

	FBC_Assert(typeid(double) == typeid(_Tp2));
	FBC_Assert(sizeof(_Tp2) == sizeof(double) && M_.rows == 2 && M_.cols == 3);
	M_.convertTo(matM);

	if (!(flags & WARP_INVERSE_MAP)) {
		double D = M[0] * M[4] - M[1] * M[3];
		D = D != 0 ? 1. / D : 0;
		double A11 = M[4] * D, A22 = M[0] * D;
		M[0] = A11; M[1] *= -D;
		M[3] *= -D; M[4] = A22;
		double b1 = -M[0] * M[2] - M[1] * M[5];
		double b2 = -M[3] * M[2] - M[4] * M[5];
		M[2] = b1; M[5] = b2;
	}

	int x;
	AutoBuffer<int> _abdelta(dst.cols * 2);
	int* adelta = &_abdelta[0], *bdelta = adelta + dst.cols;
	const int AB_BITS = MAX(10, (int)INTER_BITS);
	const int AB_SCALE = 1 << AB_BITS;

	for (x = 0; x < dst.cols; x++) {
		adelta[x] = saturate_cast<int>(M[0] * x*AB_SCALE);
		bdelta[x] = saturate_cast<int>(M[3] * x*AB_SCALE);
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_WARP_AFFINE_HPP_
