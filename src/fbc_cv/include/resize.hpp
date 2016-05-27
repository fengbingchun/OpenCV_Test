// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_RESIZE_HPP_
#define FBC_CV_RESIZE_HPP_

/* reference: imgproc/include/opencv2/imgproc.hpp
              imgproc/src/imgwarp.cpp
*/

#include "core/mat.hpp"
#include "core/base.hpp"
#include "core/saturate.hpp"

namespace fbc {

// interpolation algorithm
enum InterpolationFlags{
	/** nearest neighbor interpolation */
	INTER_NEAREST = 0,
	/** bilinear interpolation */
	INTER_LINEAR = 1,
	/** bicubic interpolation */
	INTER_CUBIC = 2,
	/** resampling using pixel area relation. It may be a preferred method for image decimation, as
	it gives moire'-free results. But when the image is zoomed, it is similar to the INTER_NEAREST method. */
	INTER_AREA = 3,
	/** Lanczos interpolation over 8x8 neighborhood */
	INTER_LANCZOS4 = 4,
};

template<typename _Tp, int chs> static int resize_nearest(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_linear(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_cubic(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_area(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_lanczos4(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);

// resize the image src down to or up to the specified size
template<typename _Tp, int chs>
int resize(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, int interpolation = NTER_LINEAR)
{
	FBC_Assert((interpolation >= 0) && (interpolation < 5));
	FBC_Assert((src.rows >= 4 && src.cols >= 4) && (dst.rows >= 4  && dst.cols >= 4));

	switch (interpolation) {
		case 0: {
			resize_nearest(src, dst);
			break;
		}
		case 1: {
			resize_linear(src, dst);
			break;
		}
		case 2: {
			resize_cubic(src, dst);
			break;
		}
		case 3: {
			resize_area(src, dst);
			break;
		}
		case 4: {
			resize_lanczos4(src, dst);
			break;
		}
		default:
			return -1;
	}

	return 0;
}

template<typename _Tp, int chs>
static int resize_nearest(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	Size ssize = src.size();
	Size dsize = dst.size();

	if (dsize == ssize) {
		// Source and destination are of same size. Use simple copy.
		src.copyTo(dst);
		return 0;
	}

	double inv_scale_x = (double)dsize.width / ssize.width;
	double inv_scale_y = (double)dsize.height / ssize.height;

	



	return 0;
}

template<typename _Tp, int chs>
static int resize_linear(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	Size ssize = src.size();
	Size dsize = dst.size();

	double inv_scale_x = (double)dsize.width / ssize.width;
	double inv_scale_y = (double)dsize.height / ssize.height;

	double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;

	int iscale_x = saturate_cast<int>(scale_x);
	int iscale_y = saturate_cast<int>(scale_y);

	bool is_area_fast = std::abs(scale_x - iscale_x) < DBL_EPSILON && std::abs(scale_y - iscale_y) < DBL_EPSILON;

	return 0;
}

template<typename _Tp, int chs>
static int resize_cubic(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{

	return 0;
}

template<typename _Tp, int chs>
static int resize_area(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{

	return 0;
}

template<typename _Tp, int chs>
static int resize_lanczos4(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{

	return 0;
}


} // namespace fbc

#endif // FBC_CV_RESIZE_HPP_

