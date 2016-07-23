// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_WARP_AFFINE_HPP_
#define FBC_CV_WARP_AFFINE_HPP_

/* reference: include/opencv2/imgproc.hpp
	      modules/imgproc/src/imgwarp.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "imgproc.hpp"
#include "remap.hpp"

namespace fbc {

// Calculates an affine transform from three pairs of the corresponding points
/*
\f[\begin{bmatrix} x'_i \\ y'_i \end{bmatrix} = \texttt{map\_matrix} \cdot \begin{bmatrix} x_i \\ y_i \\ 1 \end{bmatrix}\f]
where
\f[dst(i)=(x'_i,y'_i), src(i)=(x_i, y_i), i=0,1,2\f]
*/
FBC_EXPORTS int getAffineTransform(const Point2f src1[], const Point2f src2[], Mat_<double, 1>& dst);

// Applies an affine transformation to an image
// The function cannot operate in - place
// support type: uchar/float
/*
\f[\texttt{dst} (x,y) =  \texttt{src} ( \texttt{M} _{11} x +  \texttt{M} _{12} y +  \texttt{M} _{13}, \texttt{M} _{21} x +  \texttt{M} _{22} y +  \texttt{M} _{23})\f]
*/
template<typename _Tp1, typename _Tp2, int chs1, int chs2>
int warpAffine(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst, const Mat_<_Tp2, chs2>& M_,
	int flags = INTER_LINEAR, int borderMode = BORDER_CONSTANT, const Scalar& borderValue = Scalar())
{
	FBC_Assert(src.data != NULL && dst.data != NULL && M_.data != NULL);
	FBC_Assert(src.cols > 0 && src.rows > 0 && dst.cols > 0 && dst.rows > 0);
	FBC_Assert(src.data != dst.data);
	FBC_Assert(typeid(double) == typeid(_Tp2) && M_.rows == 2 && M_.cols == 3);
	FBC_Assert((typeid(uchar).name() == typeid(_Tp1).name()) || (typeid(float).name() == typeid(_Tp1).name())); // uchar/float

	double M[6];
	Mat_<double, 1> matM(2, 3, M);
	M_.convertTo(matM);

	int interpolation = flags & INTER_MAX;
	if (interpolation == INTER_AREA)
		interpolation = INTER_LINEAR;

	if (!(flags & WARP_INVERSE_MAP)) {
		double D = M[0] * M[4] - M[1] * M[3];
		D = D != 0 ? 1. / D : 0;
		double A11 = M[4] * D, A22 = M[0] * D;
		M[0] = A11; M[1] *= -D;
		M[3] *= -D; M[4] = A22;
		double b1 = -M[0] * M[2] - M[1] * M[5];
		double b2 = -M[3] * M[2] - M[4] * M[5];
		M[2] = b1; M[5] = b2;
	}

	int x;
	AutoBuffer<int> _abdelta(dst.cols * 2);
	int* adelta = &_abdelta[0], *bdelta = adelta + dst.cols;
	const int AB_BITS = MAX(10, (int)INTER_BITS);
	const int AB_SCALE = 1 << AB_BITS;

	for (x = 0; x < dst.cols; x++) {
		adelta[x] = saturate_cast<int>(M[0] * x*AB_SCALE);
		bdelta[x] = saturate_cast<int>(M[3] * x*AB_SCALE);
	}

	Range range(0, dst.rows);

	const int BLOCK_SZ = 64;
	short XY[BLOCK_SZ*BLOCK_SZ * 2], A[BLOCK_SZ*BLOCK_SZ];;
	int round_delta = interpolation == INTER_NEAREST ? AB_SCALE / 2 : AB_SCALE / INTER_TAB_SIZE / 2, y, x1, y1;

	int bh0 = std::min(BLOCK_SZ / 2, dst.rows);
	int bw0 = std::min(BLOCK_SZ*BLOCK_SZ / bh0, dst.cols);
	bh0 = std::min(BLOCK_SZ*BLOCK_SZ / bw0, dst.rows);

	for (y = range.start; y < range.end; y += bh0) {
		for (x = 0; x < dst.cols; x += bw0) {
			int bw = std::min(bw0, dst.cols - x);
			int bh = std::min(bh0, range.end - y);

			Mat_<short, 2> _XY(bh, bw, XY);
			Mat_<_Tp1, chs1> dpart;
			dst.getROI(dpart, Rect(x, y, bw, bh));

			for (y1 = 0; y1 < bh; y1++) {
				short* xy = XY + y1*bw * 2;
				int X0 = saturate_cast<int>((M[1] * (y + y1) + M[2])*AB_SCALE) + round_delta;
				int Y0 = saturate_cast<int>((M[4] * (y + y1) + M[5])*AB_SCALE) + round_delta;

				if (interpolation == INTER_NEAREST) {
					x1 = 0;
					for (; x1 < bw; x1++) {
						int X = (X0 + adelta[x + x1]) >> AB_BITS;
						int Y = (Y0 + bdelta[x + x1]) >> AB_BITS;
						xy[x1 * 2] = saturate_cast<short>(X);
						xy[x1 * 2 + 1] = saturate_cast<short>(Y);
					}
				} else {
					short* alpha = A + y1*bw;
					x1 = 0;
					for (; x1 < bw; x1++) {
						int X = (X0 + adelta[x + x1]) >> (AB_BITS - INTER_BITS);
						int Y = (Y0 + bdelta[x + x1]) >> (AB_BITS - INTER_BITS);
						xy[x1 * 2] = saturate_cast<short>(X >> INTER_BITS);
						xy[x1 * 2 + 1] = saturate_cast<short>(Y >> INTER_BITS);
						alpha[x1] = (short)((Y & (INTER_TAB_SIZE - 1))*INTER_TAB_SIZE +
							(X & (INTER_TAB_SIZE - 1)));
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

#endif // FBC_CV_WARP_AFFINE_HPP_
