// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_THRESHOLD_HPP_
#define FBC_CV_THRESHOLD_HPP_

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/thresh.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "imgproc.hpp"

namespace fbc {

// applies fixed-level thresholding to a single-channel array
// the Otsu's and Triangle methods are implemented only for 8-bit images
// support type: uchar/float, single-channel
template<typename _Tp, int chs>
double threshold(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, double thresh, double maxval, int type)
{

	return 0;
}

} // namespace fbc

#endif // FBC_CV_THRESHOLD_HPP_
