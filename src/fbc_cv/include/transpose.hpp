// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_TRANSPOSE_HPP_
#define FBC_CV_TRANSPOSE_HPP_

/* reference: include/opencv2/core.hpp
              modules/core/src/matrix.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "imgproc.hpp"

namespace fbc {

// transposes the matrix
// \f[\texttt{dst} (i,j) =  \texttt{src} (j,i)\f]
// support type: uchar/float, multi-channels
template <typename _Tp, int chs>
int transpose(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.rows, src.cols);
	} else {
		FBC_Assert(src.rows == dst.rows && src.cols == dst.cols);
	}

	if (src.empty()) {
		dst.release();
		return 0;
	}

	// handle the case of single-column/single-row matrices, stored in STL vectors.
	if (src.rows != dst.cols || src.cols != dst.rows) {
		FBC_Assert(src.size() == dst.size() && (src.cols == 1 || src.rows == 1));
		src.copyTo(dst);
		return 0;
	}

	//if (dst.data == src.data) {
	//	FBC_Assert(dst.cols == dst.rows);
	//	int n = dst.rows;
	//	int  step = dst.step;
	//	uchar* data = dst.ptr();

	//	int i, j;
	//	for (i = 0; i < n; i++) {
	//		_Tp* row = (_Tp*)(data + step*i);
	//		uchar* data1 = data + i*sizeof(_Tp);
	//		for (j = i + 1; j < n; j++)
	//			std::swap(row[j], *(_Tp*)(data1 + step*j));
	//	}
	//} else {
	//	const uchar* src_ = src.ptr();
	//	size_t sstep = src.step;
	//	uchar* dst_ = dst.ptr();
	//	size_t dstep = dst.step;
	//	Size sz = src.size();

	//	int i = 0, j, m = sz.width, n = sz.height;

	//	for (; i <= m - 4; i += 4) {
	//		_Tp* d0 = (_Tp*)(dst_ + dstep*i);
	//		_Tp* d1 = (_Tp*)(dst_ + dstep*(i + 1));
	//		_Tp* d2 = (_Tp*)(dst_ + dstep*(i + 2));
	//		_Tp* d3 = (_Tp*)(dst_ + dstep*(i + 3));

	//		for (j = 0; j <= n - 4; j += 4) {
	//			const _Tp* s0 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*j);
	//			const _Tp* s1 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*(j + 1));
	//			const _Tp* s2 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*(j + 2));
	//			const _Tp* s3 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*(j + 3));

	//			d0[j] = s0[0]; d0[j + 1] = s1[0]; d0[j + 2] = s2[0]; d0[j + 3] = s3[0];
	//			d1[j] = s0[1]; d1[j + 1] = s1[1]; d1[j + 2] = s2[1]; d1[j + 3] = s3[1];
	//			d2[j] = s0[2]; d2[j + 1] = s1[2]; d2[j + 2] = s2[2]; d2[j + 3] = s3[2];
	//			d3[j] = s0[3]; d3[j + 1] = s1[3]; d3[j + 2] = s2[3]; d3[j + 3] = s3[3];
	//		}

	//		for (; j < n; j++) {
	//			const _Tp* s0 = (const _Tp*)(src_ + i*sizeof(_Tp) + j*sstep);
	//			d0[j] = s0[0]; d1[j] = s0[1]; d2[j] = s0[2]; d3[j] = s0[3];
	//		}
	//	}

	//	for (; i < m; i++) {
	//		_Tp* d0 = (_Tp*)(dst_ + dstep*i);
	//		j = 0;

	//		for (; j <= n - 4; j += 4) {
	//			const _Tp* s0 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*j);
	//			const _Tp* s1 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*(j + 1));
	//			const _Tp* s2 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*(j + 2));
	//			const _Tp* s3 = (const _Tp*)(src_ + i*sizeof(_Tp) + sstep*(j + 3));

	//			d0[j] = s0[0]; d0[j + 1] = s1[0]; d0[j + 2] = s2[0]; d0[j + 3] = s3[0];
	//		}

	//		for (; j < n; j++) {
	//			const _Tp* s0 = (const _Tp*)(src_ + i*sizeof(_Tp) + j*sstep);
	//			d0[j] = s0[0];
	//		}
	//	}
	//}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_TRANSPOSE_HPP_
