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

template<typename _Tp, int chs> static int flipHoriz(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int flipVert(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);

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

	Size size = src.size();

	if (flipCode < 0) {
		if (size.width == 1)
			flipCode = 0;
		if (size.height == 1)
			flipCode = 1;
	}

	if ((size.width == 1 && flipCode > 0) ||
		(size.height == 1 && flipCode == 0) ||
		(size.height == 1 && size.width == 1 && flipCode < 0)) {
		src.copyTo(dst);
		return 0;
	}

	if (flipCode <= 0)
		flipVert(src, dst);
	else
		flipHoriz(src, dst);

	if (flipCode < 0)
		flipHoriz(dst, dst);

	return 0;
}

template<typename _Tp, int chs>
static int flipHoriz(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	Size size = src.size();
	size_t esz = sizeof(_Tp) * chs;
	int i, j, limit = (int)(((size.width + 1) / 2) *esz );
	AutoBuffer<int> _tab(size.width * esz);
	int* tab = _tab;
	size_t sstep = src.step;
	size_t dstep = dst.step;
	const uchar* src_ = src.ptr();
	uchar* dst_ = dst.ptr();

	for (i = 0; i < size.width; i++) {
		for (size_t k = 0; k < esz; k++) {
			tab[i*esz + k] = (int)((size.width - i - 1) * esz + k);
		}
	}

	for (; size.height--; src_ += sstep, dst_ += dstep) {
		for (i = 0; i < limit; i++) {
			j = tab[i];
			uchar t0 = src_[i], t1 = src_[j];
			dst_[i] = t1; dst_[j] = t0;
		}
	}

	return 0;
}

template<typename _Tp, int chs>
static int flipVert(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	const uchar* src0 = src.ptr();
	uchar* dst0 = dst.ptr();
	Size size = src.size();
	size_t sstep = src.step;
	size_t dstep = dst.step;
	size_t esz = sizeof(_Tp) * chs;
	const uchar* src1 = src0 + (size.height - 1)*sstep;
	uchar* dst1 = dst0 + (size.height - 1)*dstep;
	size.width *= (int)esz;

	for (int y = 0; y < (size.height + 1) / 2; y++, src0 += sstep, src1 -= sstep, dst0 += dstep, dst1 -= dstep) {
		int i = 0;
		if (((size_t)src0 | (size_t)dst0 | (size_t)src1 | (size_t)dst1) % sizeof(int) == 0) {
			for (; i <= size.width - 16; i += 16) {
				int t0 = ((int*)(src0 + i))[0];
				int t1 = ((int*)(src1 + i))[0];

				((int*)(dst0 + i))[0] = t1;
				((int*)(dst1 + i))[0] = t0;

				t0 = ((int*)(src0 + i))[1];
				t1 = ((int*)(src1 + i))[1];

				((int*)(dst0 + i))[1] = t1;
				((int*)(dst1 + i))[1] = t0;

				t0 = ((int*)(src0 + i))[2];
				t1 = ((int*)(src1 + i))[2];

				((int*)(dst0 + i))[2] = t1;
				((int*)(dst1 + i))[2] = t0;

				t0 = ((int*)(src0 + i))[3];
				t1 = ((int*)(src1 + i))[3];

				((int*)(dst0 + i))[3] = t1;
				((int*)(dst1 + i))[3] = t0;
			}

			for (; i <= size.width - 4; i += 4) {
				int t0 = ((int*)(src0 + i))[0];
				int t1 = ((int*)(src1 + i))[0];

				((int*)(dst0 + i))[0] = t1;
				((int*)(dst1 + i))[0] = t0;
			}
		}

		for (; i < size.width; i++) {
			uchar t0 = src0[i];
			uchar t1 = src1[i];

			dst0[i] = t1;
			dst1[i] = t0;
		}
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_FLIP_HPP_
