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

} // namespace fbc

#endif // FBC_CV_CORE_CORE_HPP_
