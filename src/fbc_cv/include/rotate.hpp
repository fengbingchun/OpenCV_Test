// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_ROTATE_HPP_
#define FBC_CV_ROTATE_HPP_

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/imgwarp.cpp
*/

#include "core/mat.hpp"
#include "warpAffine.hpp"

namespace fbc {

// Calculates an affine matrix of 2D rotation
// Positive values mean counter-clockwise rotation (the coordinate origin is assumed to be the top - left corner)
/*
\f[\begin{bmatrix} \alpha &  \beta & (1- \alpha )  \cdot \texttt{center.x} -  \beta \cdot \texttt{center.y} \\ - \beta &  \alpha &  \beta \cdot \texttt{center.x} + (1- \alpha )  \cdot \texttt{center.y} \end{bmatrix}\f]
where
\f[\begin{array}{l} \alpha =  \texttt{scale} \cdot \cos \texttt{angle} , \\ \beta =  \texttt{scale} \cdot \sin \texttt{angle} \end{array}\f]
*/
FBC_EXPORTS int getRotationMatrix2D(Point2f center, double angle, double scale, Mat_<double, 1>& dst);

// Applies an rotate to an image
// The function cannot operate in - place
// support type: uchar/float
template<typename _Tp, int chs>
int rotate(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, Point2f center, double angle,
	bool crop = true, int flags = INTER_LINEAR, int borderMode = BORDER_CONSTANT, const Scalar& borderValue = Scalar())
{
	FBC_Assert(typeid(float).name() == typeid(_Tp).name() || typeid(uchar).name() == typeid(_Tp).name());
	FBC_Assert(src.data != NULL && src.rows > 0 && src.cols > 0);

	Mat_<double, 1> rot_matrix(2, 3);
	getRotationMatrix2D(center, angle, 1.0, rot_matrix);

	if (crop) {
		if (dst.data == NULL) {
			dst = Mat_<_Tp, chs>(src.rows, src.cols);
		}
	} else {
		Rect bbox = RotatedRect(center, Size2f(src.cols, src.rows), angle).boundingRect();

		double* p = (double*)rot_matrix.data;
		p[2] += bbox.width / 2.0 - center.x;
		p[5] += bbox.height / 2.0 - center.y;

		if (dst.rows != bbox.height || dst.cols != bbox.width) {
			dst = Mat_<_Tp, chs>(bbox.height, bbox.width);
		}
	}

	warpAffine(src, dst, rot_matrix, flags, borderMode, borderValue);

	return 0;
}

} // namespace fbc

#endif // FBC_CV_ROTATE_HPP_
