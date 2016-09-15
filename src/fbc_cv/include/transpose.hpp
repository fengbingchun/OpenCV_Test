// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_TRANSPOSE_HPP_
#define FBC_CV_TRANSPOSE_HPP_

/* reference: include/opencv2/core.hpp
              modules/core/src/matrix.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"

namespace fbc {

// transposes the matrix
// \f[\texttt{dst} (i,j) =  \texttt{src} (j,i)\f]
// support type: uchar/float, multi-channels
template <typename _Tp, int chs>
int transpose(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.cols, src.rows);
	} else {
		FBC_Assert(src.rows == dst.cols && src.cols == dst.rows);
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

	if (dst.data == src.data) {
		FBC_Assert(dst.cols == dst.rows);
		int n = dst.rows;
		int  step = dst.step;
		uchar* data = dst.ptr();

		for (int i = 0; i < n; i++) {
			_Tp* row = (_Tp*)(data + step*i);
			int i_ = i * chs;

			for (int j = i + 1; j < n; j++) {
				_Tp* data1 = (_Tp*)(data + step * j);
				int j_ = j * chs;

				for (int ch = 0; ch < chs; ch++) {
					std::swap(row[j_ + ch], data1[i_ + ch]);
				}
			}
		}
	} else {
		const uchar* src_ = src.ptr();
		size_t sstep = src.step;
		uchar* dst_ = dst.ptr();
		size_t dstep = dst.step;
		int m = src.cols, n = src.rows;

		for (int i = 0; i < n; i++) {
			const _Tp* s = (const _Tp*)(src_ + sstep*i);
			int i_ = i * chs;

			for (int j = 0; j < m; j++) {
				_Tp* d = (_Tp*)(dst_ + dstep*j);
				int j_ = j * chs;

				for (int ch = 0; ch < chs; ch++) {
					d[i_ + ch] = s[j_ + ch];
				}
			}
		}
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_TRANSPOSE_HPP_
