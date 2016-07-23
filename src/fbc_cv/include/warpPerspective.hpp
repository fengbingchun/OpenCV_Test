// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_WARP_PERSPECTIVE_HPP_
#define FBC_CV_WARP_PERSPECTIVE_HPP_

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/imgwarp.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "core/invert.hpp"
#include "imgproc.hpp"
#include "remap.hpp"

namespace fbc {

// Calculates a perspective transform from four pairs of the corresponding points
/*
\f[\begin{bmatrix} t_i x'_i \\ t_i y'_i \\ t_i \end{bmatrix} = \texttt{map\_matrix} \cdot \begin{bmatrix} x_i \\ y_i \\ 1 \end{bmatrix}\f]
where
\f[dst(i)=(x'_i,y'_i), src(i)=(x_i, y_i), i=0,1,2,3\f]
*/
FBC_EXPORTS int getPerspectiveTransform(const Point2f src1[], const Point2f src2[], Mat_<double, 1>& dst);

// Applies a perspective transformation to an image
// The function cannot operate in - place
// support type: uchar/float
/* 
\f[\texttt{ dst } (x, y) = \texttt{ src } \left(\frac{ M_{ 11 } x + M_{ 12 } y + M_{ 13 } }{M_{ 31 } x + M_{ 32 } y + M_{ 33 }},
	\frac{ M_{ 21 } x + M_{ 22 } y + M_{ 23 } }{M_{ 31 } x + M_{ 32 } y + M_{ 33 }} \right)\f]
*/
template<typename _Tp1, typename _Tp2, int chs1, int chs2>
int warpPerspective(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst, const Mat_<_Tp2, chs2>& M_,
	int flags = INTER_LINEAR, int borderMode = BORDER_CONSTANT, const Scalar& borderValue = Scalar())
{
	FBC_Assert(src.data != NULL && dst.data != NULL && M_.data != NULL);
	FBC_Assert(src.cols > 0 && src.rows > 0 && dst.cols > 0 && dst.rows > 0);
	FBC_Assert(src.data != dst.data);
	FBC_Assert(typeid(double).name() == typeid(_Tp2).name() && M_.rows == 3 && M_.cols == 3);
	FBC_Assert((typeid(uchar).name() == typeid(_Tp1).name()) || (typeid(float).name() == typeid(_Tp1).name())); // uchar/float

	double M[9];
	Mat_<double, 1> matM(3, 3, M);

	int interpolation = flags & INTER_MAX;
	if (interpolation == INTER_AREA)
		interpolation = INTER_LINEAR;

	if (!(flags & WARP_INVERSE_MAP))
		invert(M_, matM);

	Range range(0, dst.rows);

	const int BLOCK_SZ = 32;
	short XY[BLOCK_SZ*BLOCK_SZ * 2], A[BLOCK_SZ*BLOCK_SZ];
	int x, y, x1, y1, width = dst.cols, height = dst.rows;

	int bh0 = std::min(BLOCK_SZ / 2, height);
	int bw0 = std::min(BLOCK_SZ*BLOCK_SZ / bh0, width);
	bh0 = std::min(BLOCK_SZ*BLOCK_SZ / bw0, height);

	for (y = range.start; y < range.end; y += bh0) {
		for (x = 0; x < width; x += bw0) {
			int bw = std::min(bw0, width - x);
			int bh = std::min(bh0, range.end - y); // height

			Mat_<short, 2> _XY(bh, bw, XY), matA;
			Mat_<_Tp1, chs1> dpart;
			dst.getROI(dpart, Rect(x, y, bw, bh));

			for (y1 = 0; y1 < bh; y1++) {
				short* xy = XY + y1*bw * 2;
				double X0 = M[0] * x + M[1] * (y + y1) + M[2];
				double Y0 = M[3] * x + M[4] * (y + y1) + M[5];
				double W0 = M[6] * x + M[7] * (y + y1) + M[8];

				if (interpolation == INTER_NEAREST) {
					x1 = 0;
					for (; x1 < bw; x1++) {
						double W = W0 + M[6] * x1;
						W = W ? 1. / W : 0;
						double fX = std::max((double)INT_MIN, std::min((double)INT_MAX, (X0 + M[0] * x1)*W));
						double fY = std::max((double)INT_MIN, std::min((double)INT_MAX, (Y0 + M[3] * x1)*W));
						int X = saturate_cast<int>(fX);
						int Y = saturate_cast<int>(fY);

						xy[x1 * 2] = saturate_cast<short>(X);
						xy[x1 * 2 + 1] = saturate_cast<short>(Y);
					}
				} else {
					short* alpha = A + y1*bw;
					x1 = 0;
					for (; x1 < bw; x1++) {
						double W = W0 + M[6] * x1;
						W = W ? INTER_TAB_SIZE / W : 0;
						double fX = std::max((double)INT_MIN, std::min((double)INT_MAX, (X0 + M[0] * x1)*W));
						double fY = std::max((double)INT_MIN, std::min((double)INT_MAX, (Y0 + M[3] * x1)*W));
						int X = saturate_cast<int>(fX);
						int Y = saturate_cast<int>(fY);

						xy[x1 * 2] = saturate_cast<short>(X >> INTER_BITS);
						xy[x1 * 2 + 1] = saturate_cast<short>(Y >> INTER_BITS);
						alpha[x1] = (short)((Y & (INTER_TAB_SIZE - 1))*INTER_TAB_SIZE + (X & (INTER_TAB_SIZE - 1)));
					}
				}
			}

			if (interpolation == INTER_NEAREST) {
				remap(src, dpart, _XY, Mat_<float, 1>(), interpolation, borderMode, borderValue);
			} else {
				Mat_<ushort, 1> _matA(bh, bw, A);
				remap(src, dpart, _XY, _matA, interpolation, borderMode, borderValue);
			}
		}
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_WARP_PERSPECTIVE_HPP_
