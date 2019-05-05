// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_NARY_MAT_ITERATOR_HPP_
#define FBC_CV_CORE_NARY_MAT_ITERATOR_HPP_

/* reference: include/opencv2/core/map.hpp
              modules/core/src/matrix.cpp
*/

#include "mat.hpp"

namespace fbc {
// n-ary multi-dimensional array iterator
// Use the class to implement unary, binary, and, generally, n-ary element-wise operations on multi - dimensional arrays.
template<typename _Tp, int chs> class NAryMatIterator {
public:
	// the default constructor
	NAryMatIterator();
	// the full constructor taking arbitrary number of n-dim matrices
	NAryMatIterator(const Mat_<_Tp, chs>** arrays, uchar** ptrs, int narrays = -1);
	// the full constructor taking arbitrary number of n-dim matrices
	NAryMatIterator(const Mat_<_Tp, chs>** arrays, Mat_<_Tp, chs>* planes, int narrays = -1);
	// the separate iterator initialization method
	void init(const Mat_<_Tp, chs>** arrays, Mat_<_Tp, chs>* planes, uchar** ptrs, int narrays = -1);

	// proceeds to the next plane of every iterated matrix
	NAryMatIterator& operator ++();
	// proceeds to the next plane of every iterated matrix (postfix increment operator)
	NAryMatIterator operator ++(int);

	// the iterated arrays
	const Mat_<_Tp, chs>** arrays;
	// the current planes
	Mat_<_Tp, chs>* planes;
	// data pointers
	uchar** ptrs;
	// the number of arrays
	int narrays;
	// the number of hyper-planes that the iterator steps through
	size_t nplanes;
	// the size of each segment (in elements)
	size_t size;
protected:
	int iterdepth;
	size_t idx;
};

template<typename _Tp, int chs> inline
NAryMatIterator<_Tp, chs>::NAryMatIterator()
	: arrays(0), planes(0), ptrs(0), narrays(0), nplanes(0), size(0), iterdepth(0), idx(0)
{
}

template<typename _Tp, int chs>
NAryMatIterator<_Tp, chs>::NAryMatIterator(const Mat_<_Tp, chs>** _arrays, Mat_<_Tp, chs>* _planes, int _narrays)
	: arrays(0), planes(0), ptrs(0), narrays(0), nplanes(0), size(0), iterdepth(0), idx(0)
{
	init(_arrays, _planes, 0, _narrays);
}

template<typename _Tp, int chs>
NAryMatIterator<_Tp, chs>::NAryMatIterator(const Mat_<_Tp, chs>** _arrays, uchar** _ptrs, int _narrays)
	: arrays(0), planes(0), ptrs(0), narrays(0), nplanes(0), size(0), iterdepth(0), idx(0)
{
	init(_arrays, 0, _ptrs, _narrays);
}

template<typename _Tp, int chs>
void NAryMatIterator<_Tp, chs>::init(const Mat_<_Tp, chs>** _arrays, Mat_<_Tp, chs>* _planes, uchar** _ptrs, int _narrays)
{
	fprintf(stderr, "NAryMatIterator no impl\n");
	FBC_Error("null"); // TODO
	/*FBC_Assert(_arrays && (_ptrs || _planes));
	int i, j, d1 = 0, i0 = -1, d = -1;

	arrays = _arrays;
	ptrs = _ptrs;
	planes = _planes;
	narrays = _narrays;
	nplanes = 0;
	size = 0;

	if (narrays < 0) {
		for (i = 0; _arrays[i] != 0; i++)
			;
		narrays = i;
		FBC_Assert(narrays <= 1000);
	}

	iterdepth = 0;

	for (i = 0; i < narrays; i++) {
		FBC_Assert(arrays[i] != 0);
		const Mat_<_Tp, chs>& A = *arrays[i];
		if (ptrs)
			ptrs[i] = A.data;

		if (!A.data)
			continue;

		if (i0 < 0) {
			i0 = i;
			d = 2; // A.dims;

			// find the first dimensionality which is different from 1;
			// in any of the arrays the first "d1" step do not affect the continuity
			for (d1 = 0; d1 < d; d1++)
				if (A.size[d1] > 1)
					break;
		} else {
			FBC_Assert(A.size == arrays[i0]->size);
		}

		if (!A.isContinuous()) {
			FBC_Assert(A.step[d - 1] == A.elemSize());
			for (j = d - 1; j > d1; j--)
				if (A.step[j] * A.size[j] < A.step[j - 1])
					break;
			iterdepth = std::max(iterdepth, j);
		}
	}

	if (i0 >= 0) {
		size = arrays[i0]->size[d - 1];
		for (j = d - 1; j > iterdepth; j--) {
			int64 total1 = (int64)size*arrays[i0]->size[j - 1];
			if (total1 != (int)total1)
				break;
			size = (int)total1;
		}

		iterdepth = j;
		if (iterdepth == d1)
			iterdepth = 0;

		nplanes = 1;
		for (j = iterdepth - 1; j >= 0; j--)
			nplanes *= arrays[i0]->size[j];
	} else {
		iterdepth = 0;
	}

	idx = 0;

	if (!planes)
		return;

	for (i = 0; i < narrays; i++) {
		FBC_Assert(arrays[i] != 0);
		const Mat_<_Tp, chs>& A = *arrays[i];

		if (!A.data) {
			planes[i] = Mat_<_Tp, chs>();
			continue;
		}

		planes[i] = Mat_<_Tp, chs>(1, (int)size, A.data);
	}*/
}

template<typename _Tp, int chs>
NAryMatIterator<_Tp, chs>& NAryMatIterator<_Tp, chs>::operator ++()
{
	fprintf(stderr, "NAryMatIterator no impl\n");
	/*if (idx >= nplanes - 1)
		return *this;
	++idx;

	if (iterdepth == 1) {
		if (ptrs) {
			for (int i = 0; i < narrays; i++) {
				if (!ptrs[i])
					continue;
				ptrs[i] = arrays[i]->data + arrays[i]->step[0] * idx;
			}
		}
		if (planes) {
			for (int i = 0; i < narrays; i++) {
				if (!planes[i].data)
					continue;
				planes[i].data = arrays[i]->data + arrays[i]->step[0] * idx;
			}
		}
	} else {
		for (int i = 0; i < narrays; i++) {
			const Mat_<_Tp, chs>& A = *arrays[i];
			if (!A.data)
				continue;
			int _idx = (int)idx;
			uchar* data = A.data;
			for (int j = iterdepth - 1; j >= 0 && _idx > 0; j--) {
				int szi = A.size[j], t = _idx / szi;
				data += (_idx - t * szi)*A.step[j];
				_idx = t;
			}
			if (ptrs)
				ptrs[i] = data;
			if (planes)
				planes[i].data = data;
		}
	}*/

	return *this;
}

template<typename _Tp, int chs>
NAryMatIterator<_Tp, chs> NAryMatIterator<_Tp, chs>::operator ++(int)
{
	NAryMatIterator<_Tp, chs> it = *this;
	++*this;
	return it;
}

} // namespace fbc

#endif // FBC_CV_CORE_NARY_MAT_ITERATOR_HPP_
