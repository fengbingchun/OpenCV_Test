// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_SOLVE_HPP_
#define FBC_CV_SOLVE_HPP_

/* reference: core/include/opencv2/core.hpp
              modules/core/src/lapack.cpp
*/

#ifndef __cplusplus
	#error solve.hpp header must be compiled as C++
#endif

#include "core/base.hpp"
#include "core/mat.hpp"
#include "core/hal.hpp"

namespace fbc {

#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

#define det2(m)   ((double)m(0,0)*m(1,1) - (double)m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*((double)m(1,1)*m(2,2) - (double)m(1,2)*m(2,1)) -  \
                   m(0,1)*((double)m(1,0)*m(2,2) - (double)m(1,2)*m(2,0)) +  \
                   m(0,2)*((double)m(1,0)*m(2,1) - (double)m(1,1)*m(2,0)))

// Solves one or more linear systems or least-squares problems
// The function solve solves a linear system or least-squares problem (the
// latter is possible with SVD or QR methods, or by specifying the flag DECOMP_NORMAL)
template<typename _Tp, int chs>
bool solve(const Mat_<_Tp, chs>& src1, const Mat_<_Tp, chs>& src2, Mat_<_Tp, chs>& dst, int method = DECOMP_LU)
{
	FBC_Assert(src1.data != NULL && src2.data != NULL && dst.data != NULL);
	FBC_Assert(chs == 1);

	bool result = true;
	bool is_normal = (method & DECOMP_NORMAL) != 0;

	FBC_Assert(sizeof(_Tp) == sizeof(float) || sizeof(_Tp) == sizeof(double)); // float(4) || double(8)
	method &= ~DECOMP_NORMAL;
	FBC_Assert((method != DECOMP_LU && method != DECOMP_CHOLESKY) || is_normal || src1.rows == src1.cols);

	// check case of a single equation and small matrix
	if ((method == DECOMP_LU || method == DECOMP_CHOLESKY) && !is_normal &&
		src1.rows <= 3 && src1.rows == src1.cols && src2.cols == 1) {
		FBC_Assert(dst.rows == src1.cols && dst.cols == src2.cols);

		#define bf(y) ((float*)(bdata + y*src2step))[0]
		#define bd(y) ((double*)(bdata + y*src2step))[0]

		const uchar* srcdata = src1.ptr();
		const uchar* bdata = src2.ptr();
		uchar* dstdata = (uchar*)dst.ptr();
		size_t srcstep = src1.step;
		size_t src2step = src2.step;
		size_t dststep = dst.step;

		if (src1.rows == 2) {
			if (sizeof(_Tp) == sizeof(float)) {
				double d = det2(Sf);
				if (d != 0.) {
					double t;
					d = 1. / d;
					t = (float)(((double)bf(0)*Sf(1, 1) - (double)bf(1)*Sf(0, 1))*d);
					Df(1, 0) = (float)(((double)bf(1)*Sf(0, 0) - (double)bf(0)*Sf(1, 0))*d);
					Df(0, 0) = (float)t;
				} else {
					result = false;
				}
			} else {
				double d = det2(Sd);
				if (d != 0.) {
					double t;
					d = 1. / d;
					t = (bd(0)*Sd(1, 1) - bd(1)*Sd(0, 1))*d;
					Dd(1, 0) = (bd(1)*Sd(0, 0) - bd(0)*Sd(1, 0))*d;
					Dd(0, 0) = t;
				} else {
					result = false;
				}
			}
		} else if (src1.rows == 3) {
			if (sizeof(_Tp) == sizeof(float)) {
				double d = det3(Sf);
				if (d != 0.) {
					float t[3];
					d = 1. / d;

					t[0] = (float)(d*
						(bf(0)*((double)Sf(1, 1)*Sf(2, 2) - (double)Sf(1, 2)*Sf(2, 1)) -
						Sf(0, 1)*((double)bf(1)*Sf(2, 2) - (double)Sf(1, 2)*bf(2)) +
						Sf(0, 2)*((double)bf(1)*Sf(2, 1) - (double)Sf(1, 1)*bf(2))));

					t[1] = (float)(d*
						(Sf(0, 0)*(double)(bf(1)*Sf(2, 2) - (double)Sf(1, 2)*bf(2)) -
						bf(0)*((double)Sf(1, 0)*Sf(2, 2) - (double)Sf(1, 2)*Sf(2, 0)) +
						Sf(0, 2)*((double)Sf(1, 0)*bf(2) - (double)bf(1)*Sf(2, 0))));

					t[2] = (float)(d*
						(Sf(0, 0)*((double)Sf(1, 1)*bf(2) - (double)bf(1)*Sf(2, 1)) -
						Sf(0, 1)*((double)Sf(1, 0)*bf(2) - (double)bf(1)*Sf(2, 0)) +
						bf(0)*((double)Sf(1, 0)*Sf(2, 1) - (double)Sf(1, 1)*Sf(2, 0))));

					Df(0, 0) = t[0];
					Df(1, 0) = t[1];
					Df(2, 0) = t[2];
				} else {
					result = false;
				}
			} else {
				double d = det3(Sd);
				if (d != 0.) {
					double t[9];

					d = 1. / d;

					t[0] = ((Sd(1, 1) * Sd(2, 2) - Sd(1, 2) * Sd(2, 1))*bd(0) +
						(Sd(0, 2) * Sd(2, 1) - Sd(0, 1) * Sd(2, 2))*bd(1) +
						(Sd(0, 1) * Sd(1, 2) - Sd(0, 2) * Sd(1, 1))*bd(2))*d;

					t[1] = ((Sd(1, 2) * Sd(2, 0) - Sd(1, 0) * Sd(2, 2))*bd(0) +
						(Sd(0, 0) * Sd(2, 2) - Sd(0, 2) * Sd(2, 0))*bd(1) +
						(Sd(0, 2) * Sd(1, 0) - Sd(0, 0) * Sd(1, 2))*bd(2))*d;

					t[2] = ((Sd(1, 0) * Sd(2, 1) - Sd(1, 1) * Sd(2, 0))*bd(0) +
						(Sd(0, 1) * Sd(2, 0) - Sd(0, 0) * Sd(2, 1))*bd(1) +
						(Sd(0, 0) * Sd(1, 1) - Sd(0, 1) * Sd(1, 0))*bd(2))*d;

					Dd(0, 0) = t[0];
					Dd(1, 0) = t[1];
					Dd(2, 0) = t[2];
				} else {
					result = false;
				}
			}
		} else {
			FBC_Assert(src1.rows == 1);

			if (sizeof(_Tp) == sizeof(float)) {
				double d = Sf(0, 0);
				if (d != 0.)
					Df(0, 0) = (float)(bf(0) / d);
				else
					result = false;
			} else {
				double d = Sd(0, 0);
				if (d != 0.)
					Dd(0, 0) = (bd(0) / d);
				else
					result = false;
			}
		}
		return result;
	}

	if (method == DECOMP_QR)
		method = DECOMP_SVD;

	int m = src1.rows, m_ = m, n = src1.cols, nb = src2.cols;
	size_t esz = sizeof(_Tp), bufsize = 0;
	size_t vstep = alignSize(n*esz, 16);
	size_t astep = method == DECOMP_SVD && !is_normal ? alignSize(m*esz, 16) : vstep;
	AutoBuffer<uchar> buffer;

	FBC_Assert(dst.rows == src1.cols && dst.cols == src2.cols);

	if (m < n) {
		FBC_Error("The function can not solve under-determined linear systems");
	}

	if (m == n) {
		is_normal = false;
	} else if (is_normal) {
		m_ = n;
		if (method == DECOMP_SVD)
			method = DECOMP_EIG;
	}

	size_t asize = astep*(method == DECOMP_SVD || is_normal ? n : m);
	bufsize += asize + 32;

	if (is_normal)
		bufsize += n*nb*esz;

	if (method == DECOMP_SVD || method == DECOMP_EIG)
		bufsize += n * 5 * esz + n*vstep + nb*sizeof(double) + 32;

	buffer.allocate(bufsize);
	uchar* ptr = alignPtr((uchar*)buffer, 16);

	Mat_<_Tp, chs> a(m_, n, ptr);

	if (is_normal) {
		//mulTransposed(src, a, true); // TODO
		FBC_Assert(0);
	} else if (method != DECOMP_SVD) {
		src1.copyTo(a);
	} else {
		a = Mat_<_Tp, chs>(n, m_, ptr);
		//transpose(src, a); // TODO
		FBC_Assert(0);
	}
	ptr += asize;

	if (!is_normal) {
		if (method == DECOMP_LU || method == DECOMP_CHOLESKY)
			src2.copyTo(dst);
	} else {
		FBC_Assert(0);
		// a'*b
		if (method == DECOMP_LU || method == DECOMP_CHOLESKY) {
			//gemm(src, src2, 1, Mat(), 0, dst, GEMM_1_T); // TODO
		} else {
			//Mat tmp(n, nb, type, ptr);
			//ptr += n*nb*esz;
			//gemm(src, src2, 1, Mat(), 0, tmp, GEMM_1_T); // TODO
			//src2 = tmp;
		}
	}

	if (method == DECOMP_LU) {
		if (sizeof(_Tp) == sizeof(float))
			result = hal::LU32f((float*)a.ptr(), a.step, n, (float*)dst.ptr(), dst.step, nb) != 0;
		else
			result = hal::LU64f((double*)a.ptr(), a.step, n, (double*)dst.ptr(), dst.step, nb) != 0;
	} else if (method == DECOMP_CHOLESKY) {
		if (sizeof(_Tp) == sizeof(float))
			result = hal::Cholesky32f((float*)a.ptr(), a.step, n, (float*)dst.ptr(), dst.step, nb);
		else
			result = hal::Cholesky64f((double*)a.ptr(), a.step, n, (double*)dst.ptr(), dst.step, nb);
	} else {
		FBC_Assert(0);
		//ptr = alignPtr(ptr, 16); // TODO
		//Mat v(n, n, type, ptr, vstep), w(n, 1, type, ptr + vstep*n), u;
		//ptr += n*(vstep + esz);

		//if (method == DECOMP_EIG) {
		//	if (type == CV_32F)
		//		Jacobi(a.ptr<float>(), a.step, w.ptr<float>(), v.ptr<float>(), v.step, n, ptr);
		//	else
		//		Jacobi(a.ptr<double>(), a.step, w.ptr<double>(), v.ptr<double>(), v.step, n, ptr);
		//	u = v;
		//} else {
		//	if (type == CV_32F)
		//		JacobiSVD(a.ptr<float>(), a.step, w.ptr<float>(), v.ptr<float>(), v.step, m_, n);
		//	else
		//		JacobiSVD(a.ptr<double>(), a.step, w.ptr<double>(), v.ptr<double>(), v.step, m_, n);
		//	u = a;
		//}

		//if (type == CV_32F) {
		//	SVBkSb(m_, n, w.ptr<float>(), 0, u.ptr<float>(), u.step, true,
		//		v.ptr<float>(), v.step, true, src2.ptr<float>(),
		//		src2.step, nb, dst.ptr<float>(), dst.step, ptr);
		//} else {
		//	SVBkSb(m_, n, w.ptr<double>(), 0, u.ptr<double>(), u.step, true,
		//		v.ptr<double>(), v.step, true, src2.ptr<double>(),
		//		src2.step, nb, dst.ptr<double>(), dst.step, ptr);
		//}
		//result = true;
	}

	return result;
}

} // namespace fbc

#endif // FBC_CV_SOLVE_HPP_
