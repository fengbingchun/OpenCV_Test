// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#include "warpAffine.hpp"
#include "rotate.hpp"

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/imgwarp.cpp
*/

namespace fbc {

/* Calculates coefficients of affine transformation
* which maps (xi,yi) to (ui,vi), (i=1,2,3):
*
* ui = c00*xi + c01*yi + c02
*
* vi = c10*xi + c11*yi + c12
*
* Coefficients are calculated by solving linear system:
* / x0 y0  1  0  0  0 \ /c00\ /u0\
* | x1 y1  1  0  0  0 | |c01| |u1|
* | x2 y2  1  0  0  0 | |c02| |u2|
* |  0  0  0 x0 y0  1 | |c10| |v0|
* |  0  0  0 x1 y1  1 | |c11| |v1|
* \  0  0  0 x2 y2  1 / |c12| |v2|
*
* where:
*   cij - matrix coefficients
*/
int getAffineTransform(const Point2f src1[], const Point2f src2[], Mat_<double, 1>& dst)
{
	FBC_Assert(dst.rows == 2 && dst.cols == 3);

	Mat_<double, 1> X(6, 1, dst.data);
	double a[6 * 6], b[6];
	Mat_<double, 1> A(6, 6, a), B(6, 1, b);

	for (int i = 0; i < 3; i++) {
		int j = i * 12;
		int k = i * 12 + 6;
		a[j] = a[k + 3] = src1[i].x;
		a[j + 1] = a[k + 4] = src1[i].y;
		a[j + 2] = a[k + 5] = 1;
		a[j + 3] = a[j + 4] = a[j + 5] = 0;
		a[k] = a[k + 1] = a[k + 2] = 0;
		b[i * 2] = src2[i].x;
		b[i * 2 + 1] = src2[i].y;
	}

	bool ret = solve(A, B, X);
	FBC_Assert(ret == true);

	return 0;
}

int getRotationMatrix2D(Point2f center, double angle, double scale, Mat_<double, 1>& dst)
{
	FBC_Assert(dst.rows == 2 && dst.cols == 3);

	angle *= FBC_PI / 180;
	double alpha = cos(angle)*scale;
	double beta = sin(angle)*scale;

	double* m = (double*)dst.data;

	m[0] = alpha;
	m[1] = beta;
	m[2] = (1 - alpha)*center.x - beta*center.y;
	m[3] = -beta;
	m[4] = alpha;
	m[5] = beta*center.x + (1 - alpha)*center.y;

	return 0;
}

} // namespace fbc
