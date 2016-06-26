// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_MERGE_HPP_
#define FBC_CV_MERGE_HPP_

/* reference: include/opencv2/core.hpp
              core/src/convert.cpp
              core/src/merge.cpp
*/

#include <vector>
#include "core/mat.hpp"

#ifndef __cplusplus
	#error merge.hpp header must be compiled as C++
#endif

namespace fbc {

// merge several arrays to make a single multi-channel array
template<typename _Tp, int chs1, int chs2>
int merge(const std::vector<Mat_<_Tp, chs1>>& src, Mat_<_Tp, chs2>& dst)
{
	FBC_Assert(dst.data != NULL);
	FBC_Assert((src.size() > 0) && (src.size() == dst.channels) && (src.size() <= FBC_CN_MAX));
	int width = src[0].cols;
	int height = src[0].rows;
	FBC_Assert((dst.cols == width) && (dst.rows == height));
	for (int i = 0; i < src.size(); i++) {
		FBC_Assert(src[i].data != NULL);
		FBC_Assert((src[i].cols == width) && src[i].rows == height);
		FBC_Assert(src[i].channels == 1);
	}

	if (src.size() == 1) {
		memcpy(dst.data, src[0].data, dst.step * dst.rows);
		return 0;
	}

	_Tp* pDst = (_Tp*)dst.data;
	int len = width * height;
	int cn = dst.channels;

	for (int i = 0; i < src.size(); i++) {
		_Tp* pSrc = (_Tp*)src[i].data;

		for (int j = 0; j < len; j++) {
			pDst[j * cn + i] = pSrc[j];
		}
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_MERGE_HPP_
