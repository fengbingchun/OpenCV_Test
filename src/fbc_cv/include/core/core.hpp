// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_CORE_HPP_
#define FBC_CV_CORE_CORE_HPP_

/* reference: include/opencv2/core/core_c.h
              include/opencv2/core.hpp
	      modules/core/src/stat.cpp
	      modules/core/include/opencv2/core/private.hpp
	      modules/core/src/matrix.cpp
	      modules/core/src/arithm.cpp
*/

#ifndef __cplusplus
	#error core.hpp header must be compiled as C++
#endif

#include <exception>
#include <string>
#include "core/fbcdef.hpp"
#include "core/mat.hpp"

namespace fbc {

// Fast cubic root calculation
FBC_EXPORTS float fbcCbrt(float value);

// Computes the source location of an extrapolated pixel
FBC_EXPORTS int borderInterpolate(int p, int len, int borderType);

// Transposes a matrix
// \f[\texttt{dst} (i,j) =  \texttt{src} (j,i)\f]
template<typename _Tp, int chs>
int transpose(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	if (src.empty()) {
		dst.release();
		return -1;
	}

	// handle the case of single-column/single-row matrices, stored in STL vectors
	if (src.rows != dst.cols || src.cols != dst.rows) {
		FBC_Assert(src.size() == dst.size() && (src.cols == 1 || src.rows == 1));
		src.copyTo(dst);

		return 0;
	}

	if (dst.data == src.data) {
		FBC_Assert(0); // TODO
	} else {
		Size sz = src.size();
		int i = 0, j, m = sz.width, n = sz.height;
		int sstep = src.step;
		int dstep = dst.step;

		for (; i <= m - 4; i += 4) {
			_Tp* d0 = (_Tp*)(dst.data + dstep*i);
			_Tp* d1 = (_Tp*)(dst.data + dstep*(i + 1));
			_Tp* d2 = (_Tp*)(dst.data + dstep*(i + 2));
			_Tp* d3 = (_Tp*)(dst.data + dstep*(i + 3));

			for (j = 0; j <= n - 4; j += 4) {
				const _Tp* s0 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*j);
				const _Tp* s1 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*(j + 1));
				const _Tp* s2 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*(j + 2));
				const _Tp* s3 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*(j + 3));

				d0[j] = s0[0]; d0[j + 1] = s1[0]; d0[j + 2] = s2[0]; d0[j + 3] = s3[0];
				d1[j] = s0[1]; d1[j + 1] = s1[1]; d1[j + 2] = s2[1]; d1[j + 3] = s3[1];
				d2[j] = s0[2]; d2[j + 1] = s1[2]; d2[j + 2] = s2[2]; d2[j + 3] = s3[2];
				d3[j] = s0[3]; d3[j + 1] = s1[3]; d3[j + 2] = s2[3]; d3[j + 3] = s3[3];
			}

			for (; j < n; j++) {
				const _Tp* s0 = (const _Tp*)(src.data + i*sizeof(_Tp) + j*sstep);
				d0[j] = s0[0]; d1[j] = s0[1]; d2[j] = s0[2]; d3[j] = s0[3];
			}
		}

		for (; i < m; i++) {
			_Tp* d0 = (_Tp*)(dst.data + dstep*i);
			j = 0;

			for (; j <= n - 4; j += 4) {
				const _Tp* s0 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*j);
				const _Tp* s1 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*(j + 1));
				const _Tp* s2 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*(j + 2));
				const _Tp* s3 = (const _Tp*)(src.data + i*sizeof(_Tp) + sstep*(j + 3));

				d0[j] = s0[0]; d0[j + 1] = s1[0]; d0[j + 2] = s2[0]; d0[j + 3] = s3[0];
			}

			for (; j < n; j++) {
				const _Tp* s0 = (const _Tp*)(src.data + i*sizeof(_Tp) + j*sstep);
				d0[j] = s0[0];
			}
		}
	}

	return 0;
}

// Counts non-zero array elements
// \f[\sum _{ I: \; \texttt{ src } (I) \ne0 } 1\f]
template<typename _Tp, int chs>
int countNonZero(const Mat_<_Tp, chs>& src)
{
	FBC_Assert(chs == 1);

	int len = src.rows * src.cols;
	const _Tp* p = (_Tp*)src.data;

	int nz = 0;
	for (int i = 0; i < len; i++) {
		nz += (p[i] != 0);
	}

	return nz;
}

template<typename _Tp, int chs>
void scalarToRawData(const fbc::Scalar& s, void* _buf, int unroll_to = 0)
{
	int i, cn = chs;
	FBC_Assert(chs <= 4);
	int depth = sizeof(_Tp);
	switch (depth) {
		case 1: {
			uchar* buf = (uchar*)_buf;
			for (i = 0; i < cn; i++)
				buf[i] = saturate_cast<uchar>(s.val[i]);
			for (; i < unroll_to; i++)
				buf[i] = buf[i - cn];
		}
			break;
		case 4: {
			float* buf = (float*)_buf;
			for (i = 0; i < cn; i++)
				buf[i] = saturate_cast<float>(s.val[i]);
			for (; i < unroll_to; i++)
				buf[i] = buf[i - cn];
		}
			break;
		default:
			FBC_Error("UnsupportedFormat");
	}
}

// calculates the per - element bit - wise logical conjunction
// \f[\texttt{dst} (I) =  \texttt{src1} (I)  \wedge \texttt{src2} (I) \quad \texttt{if mask} (I) \ne0\f]
// mask optional operation mask, 8-bit single channel array, that specifies elements of the output array to be changed
template<typename _Tp, int chs>
int bitwise_and(const Mat_<_Tp, chs>& src1, const Mat_<_Tp, chs>& src2, Mat_<_Tp, chs>& dst, const Mat_<uchar, 1>& mask = Mat_<uchar, 1>())
{
	FBC_Assert(src1.rows == src2.rows && src1.cols == src2.cols);
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src1.rows, src1.cols);
	} else {
		FBC_Assert(src1.rows == dst.rows && src1.cols == dst.cols);
	}

	if (!mask.empty()) {
		FBC_Assert(src1.rows == mask.rows && src1.cols == mask.cols);
	}

	int bytePerRow = src1.cols * chs * sizeof(_Tp);
	int bypePerPixel = chs * sizeof(_Tp);
	for (int y = 0; y < src1.rows; y++) {
		const uchar* pSrc1 = src1.ptr(y);
		const uchar* pSrc2 = src2.ptr(y);
		uchar* pDst = dst.ptr(y);
		const uchar* pMask = NULL;
		if (!mask.empty()) {
			pMask = mask.ptr(y);

			for (int x = 0; x < src1.cols; x++) {
				if (pMask[x] == 1) {
					int addr = x * bypePerPixel;
					for (int t = 0; t < bypePerPixel; t++) {
						pDst[addr + t] = pSrc1[addr + t] & pSrc2[addr + t];
					}
				}
			}
		} else {
			for (int x = 0; x < bytePerRow; x++) {
				pDst[x] = pSrc1[x] & pSrc2[x];
			}
		}
	}

	return 0;
}

// Inverts every bit of an array
// \f[\texttt{dst} (I) =  \neg \texttt{src} (I)\f]
// mask optional operation mask, 8-bit single channel array, that specifies elements of the output array to be changed
template<typename _Tp, int chs>
int bitwise_not(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, const Mat_<uchar, 1>& mask = Mat_<uchar, 1>())
{
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.rows, src.cols);
	} else {
		FBC_Assert(src.rows == dst.rows && src.cols == dst.cols);
	}

	if (!mask.empty()) {
		FBC_Assert(src.rows == mask.rows && src.cols == mask.cols);
	}

	int bytePerRow = src.cols * chs * sizeof(_Tp);
	int bypePerPixel = chs * sizeof(_Tp);
	for (int y = 0; y < src.rows; y++) {
		const uchar* pSrc = src.ptr(y);
		uchar* pDst = dst.ptr(y);
		const uchar* pMask = NULL;
		if (!mask.empty()) {
			pMask = mask.ptr(y);

			for (int x = 0; x < src.cols; x++) {
				if (pMask[x] == 1) {
					int addr = x * bypePerPixel;
					for (int t = 0; t < bypePerPixel; t++) {
						pDst[addr + t] = ~pSrc[addr + t];
					}
				}
			}
		} else {
			for (int x = 0; x < bytePerRow; x++) {
				pDst[x] = ~pSrc[x];
			}
		}
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_CORE_CORE_HPP_
