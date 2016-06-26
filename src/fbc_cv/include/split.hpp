// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_SPLIT_HPP_
#define FBC_CV_SPLIT_HPP_

/* reference: include/opencv2/core.hpp
              core/src/convert.cpp
	      core/src/split.cpp
*/

#include <vector>
#include "core/mat.hpp"

#ifndef __cplusplus
	#error split.hpp header must be compiled as C++
#endif

namespace fbc {

// split a multi-channel array into separate single-channel arrays
template<typename _Tp, int chs1, int chs2>
int split(const Mat_<_Tp, chs1>& src, std::vector<Mat_<_Tp, chs2>>& dst)
{
	FBC_Assert(src.data != NULL);
	FBC_Assert((dst.size() == chs1) && (chs2 == 1));
	for (int i = 0; i < dst.size(); i++) {
		FBC_Assert((dst[i].data != NULL) && (dst[i].rows == src.rows) && (dst[i].cols == src.cols));
	}

	int cn = src.channels;
	if (cn == 1) {
		memcpy(dst[0].data, src.data, src.step * src.rows);
		return 0;
	}

	_Tp* pSrc = (_Tp*)src.data;
	int len = src.rows * src.cols;

	for (int i = 0; i < cn; i++) {
		_Tp* pDst = (_Tp*)dst[i].data;

		for (int j = 0; j < len; j++) {
			pDst[j] = pSrc[j * cn + i];
		}
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_SPLIT_HPP_
