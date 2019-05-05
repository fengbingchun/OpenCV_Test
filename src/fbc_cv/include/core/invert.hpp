// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_INVERT_HPP_
#define FBC_CV_CORE_INVERT_HPP_

/* reference: core/include/opencv2/core.hpp
              modules/core/src/lapack.cpp
*/

#include "mat.hpp"
#include "solve.hpp"

namespace fbc {

// Finds the inverse or pseudo-inverse of a matrix
template<typename _Tp, int chs>
bool invert(const Mat_<_Tp, chs>&src, Mat_<_Tp, chs>& dst, int method = DECOMP_LU)
{
	FBC_Assert(src.data != NULL && dst.data != NULL);
	FBC_Assert(src.cols > 0 && src.rows > 0 && dst.cols > 0 && dst.rows > 0);
	FBC_Assert(typeid(double).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name());
	FBC_Assert(src.cols == dst.rows && src.rows == dst.cols);

	bool result = false;
	size_t esz = sizeof(_Tp) * chs; // size_t esz = CV_ELEM_SIZE(type);
	int m = src.rows, n = src.cols;

	if (method == DECOMP_SVD) { // TODO
		FBC_Assert(0);
	}

	FBC_Assert(m == n);

	if (method == DECOMP_EIG) { // TODO
		FBC_Assert(0);
	}

	FBC_Assert(method == DECOMP_LU || method == DECOMP_CHOLESKY);

	if (n <= 3) {
		const uchar* srcdata = src.ptr();
		uchar* dstdata = const_cast<uchar*>(dst.ptr());
		size_t srcstep = src.step;
		size_t dststep = dst.step;

		if (n == 2) { // TODO
			FBC_Assert(0);
		} else if (n == 3) {
			if (typeid(float).name() == typeid(_Tp).name() && chs == 1) {
				double d = det3(Sf);

				if (d != 0.) {
					double t[12];

					result = true;
					d = 1./d;
					t[0] = (((double)Sf(1,1) * Sf(2,2) - (double)Sf(1,2) * Sf(2,1)) * d);
					t[1] = (((double)Sf(0,2) * Sf(2,1) - (double)Sf(0,1) * Sf(2,2)) * d);
					t[2] = (((double)Sf(0,1) * Sf(1,2) - (double)Sf(0,2) * Sf(1,1)) * d);

					t[3] = (((double)Sf(1,2) * Sf(2,0) - (double)Sf(1,0) * Sf(2,2)) * d);
					t[4] = (((double)Sf(0,0) * Sf(2,2) - (double)Sf(0,2) * Sf(2,0)) * d);
					t[5] = (((double)Sf(0,2) * Sf(1,0) - (double)Sf(0,0) * Sf(1,2)) * d);

					t[6] = (((double)Sf(1,0) * Sf(2,1) - (double)Sf(1,1) * Sf(2,0)) * d);
					t[7] = (((double)Sf(0,1) * Sf(2,0) - (double)Sf(0,0) * Sf(2,1)) * d);
					t[8] = (((double)Sf(0,0) * Sf(1,1) - (double)Sf(0,1) * Sf(1,0)) * d);

					Df(0,0) = (float)t[0]; Df(0,1) = (float)t[1]; Df(0,2) = (float)t[2];
					Df(1,0) = (float)t[3]; Df(1,1) = (float)t[4]; Df(1,2) = (float)t[5];
					Df(2, 0) = (float)t[6]; Df(2, 1) = (float)t[7]; Df(2, 2) = (float)t[8];
				}
			} else {
				double d = det3(Sd);
				if (d != 0.) {
					result = true;
					d = 1. / d;
					double t[9];

					t[0] = (Sd(1, 1) * Sd(2, 2) - Sd(1, 2) * Sd(2, 1)) * d;
					t[1] = (Sd(0, 2) * Sd(2, 1) - Sd(0, 1) * Sd(2, 2)) * d;
					t[2] = (Sd(0, 1) * Sd(1, 2) - Sd(0, 2) * Sd(1, 1)) * d;

					t[3] = (Sd(1, 2) * Sd(2, 0) - Sd(1, 0) * Sd(2, 2)) * d;
					t[4] = (Sd(0, 0) * Sd(2, 2) - Sd(0, 2) * Sd(2, 0)) * d;
					t[5] = (Sd(0, 2) * Sd(1, 0) - Sd(0, 0) * Sd(1, 2)) * d;

					t[6] = (Sd(1, 0) * Sd(2, 1) - Sd(1, 1) * Sd(2, 0)) * d;
					t[7] = (Sd(0, 1) * Sd(2, 0) - Sd(0, 0) * Sd(2, 1)) * d;
					t[8] = (Sd(0, 0) * Sd(1, 1) - Sd(0, 1) * Sd(1, 0)) * d;

					Dd(0, 0) = t[0]; Dd(0, 1) = t[1]; Dd(0, 2) = t[2];
					Dd(1, 0) = t[3]; Dd(1, 1) = t[4]; Dd(1, 2) = t[5];
					Dd(2, 0) = t[6]; Dd(2, 1) = t[7]; Dd(2, 2) = t[8];
				}
			}
		} else {
			assert(n == 1);

			if (typeid(float).name() == typeid(_Tp).name() && chs == 1)
			{
				double d = Sf(0, 0);
				if (d != 0.) {
					result = true;
					Df(0, 0) = (float)(1. / d);
				}
			} else {
				double d = Sd(0, 0);
				if (d != 0.) {
					result = true;
					Dd(0, 0) = 1. / d;
				}
			}
		}

		if (!result)
			dst.setTo(Scalar(0));
		return result;
	}

	FBC_Assert(0); // TODO

	return result;
}

} // namespace fbc

#endif // FBC_CV_CORE_INVERT_HPP_
