// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_REMAP_HPP_
#define FBC_CV_REMAP_HPP_

/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/imgwarp.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "core/base.hpp"
#include "imgproc.hpp"
#include "resize.hpp"

namespace fbc {

const int INTER_REMAP_COEF_BITS = 15;
const int INTER_REMAP_COEF_SCALE = 1 << INTER_REMAP_COEF_BITS;

static uchar NNDeltaTab_i[INTER_TAB_SIZE2][2];

static float BilinearTab_f[INTER_TAB_SIZE2][2][2];
static short BilinearTab_i[INTER_TAB_SIZE2][2][2];

static float BicubicTab_f[INTER_TAB_SIZE2][4][4];
static short BicubicTab_i[INTER_TAB_SIZE2][4][4];

static float Lanczos4Tab_f[INTER_TAB_SIZE2][8][8];
static short Lanczos4Tab_i[INTER_TAB_SIZE2][8][8];

template<typename _Tp1, typename _Tp2, int chs1, int chs2> static int remap_nearest(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue);
template<typename _Tp1, typename _Tp2, int chs1, int chs2> static int remap_linear(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue);
template<typename _Tp1, typename _Tp2, int chs1, int chs2> static int remap_cubic(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue);
template<typename _Tp1, typename _Tp2, int chs1, int chs2> static int remap_lanczos4(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue);

// Applies a generic geometrical transformation to an image
// transforms the source image using the specified map, this function cannot operate in-place
template<typename _Tp1, typename _Tp2, int chs1, int chs2>
int remap(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst, const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2,
	int interpolation, int borderMode = BORDER_CONSTANT, const Scalar& borderValue = Scalar())
{
	FBC_Assert(map1.size().area() > 0 && map1.size() == map2.size());
	FBC_Assert(map1.data != NULL && map2.data != NULL);
	FBC_Assert(src.size() == map1.size() && src.size() == dst.size());
	FBC_Assert(src.data != dst.data);
	FBC_Assert(typeid(short).name() == typeid(_Tp2).name() || typeid(float).name() == typeid(_Tp2).name());
	FBC_Assert(sizeof(_Tp2) == sizeof(short) || sizeof(_Tp2) == sizeof(float)); // short/float
	FBC_Assert(chs2 == 1);

	switch (interpolation) {
		case 0: {
			remap_nearest(src, dst, map1, map2, borderMode, borderValue);
			break;
		}
		case 1:
		case 3: {
			remap_linear(src, dst, map1, map2, borderMode, borderValue);
			break;
		}
		case 2: {
			remap_cubic(src, dst, map1, map2, borderMode, borderValue);
			break;
		}
		case 4: {
			remap_lanczos4(src, dst, map1, map2, borderMode, borderValue);
			break;
		}
		default:
			return -1;
	}

	return 0;
}

template<typename _Tp>
static inline void interpolateLinear(_Tp x, _Tp* coeffs)
{
	coeffs[0] = 1.f - x;
	coeffs[1] = x;
}

template<typename _Tp>
static void initInterTab1D(int method, float* tab, int tabsz)
{
	float scale = 1.f / tabsz;
	if (method == INTER_LINEAR) {
		for (int i = 0; i < tabsz; i++, tab += 2)
			interpolateLinear<float>(i*scale, tab);
	} else if (method == INTER_CUBIC) {
		for (int i = 0; i < tabsz; i++, tab += 4)
			interpolateCubic<float>(i*scale, tab);
	} else if (method == INTER_LANCZOS4) {
		for (int i = 0; i < tabsz; i++, tab += 8)
			interpolateLanczos4<float>(i*scale, tab);
	} else {
		FBC_Error("Unknown interpolation method");
	}
}

template<typename _Tp>
static const void* initInterTab2D(int method, bool fixpt)
{
	static bool inittab[INTER_MAX + 1] = { false };
	float* tab = 0;
	short* itab = 0;
	int ksize = 0;
	if (method == INTER_LINEAR) {
		tab = BilinearTab_f[0][0], itab = BilinearTab_i[0][0], ksize = 2;
	} else if (method == INTER_CUBIC) {
		tab = BicubicTab_f[0][0], itab = BicubicTab_i[0][0], ksize = 4;
	} else if (method == INTER_LANCZOS4) {
		tab = Lanczos4Tab_f[0][0], itab = Lanczos4Tab_i[0][0], ksize = 8;
	} else {
		FBC_Error("Unknown/unsupported interpolation type");
	}

	if (!inittab[method]) {
		AutoBuffer<float> _tab(8 * INTER_TAB_SIZE);
		int i, j, k1, k2;
		initInterTab1D<float>(method, _tab, INTER_TAB_SIZE);
		for (i = 0; i < INTER_TAB_SIZE; i++) {
			for (j = 0; j < INTER_TAB_SIZE; j++, tab += ksize*ksize, itab += ksize*ksize) {
				int isum = 0;
				NNDeltaTab_i[i*INTER_TAB_SIZE + j][0] = j < INTER_TAB_SIZE / 2;
				NNDeltaTab_i[i*INTER_TAB_SIZE + j][1] = i < INTER_TAB_SIZE / 2;

				for (k1 = 0; k1 < ksize; k1++) {
					float vy = _tab[i*ksize + k1];
					for (k2 = 0; k2 < ksize; k2++) {
						float v = vy*_tab[j*ksize + k2];
						tab[k1*ksize + k2] = v;
						isum += itab[k1*ksize + k2] = saturate_cast<short>(v*INTER_REMAP_COEF_SCALE);
					}
				}

				if (isum != INTER_REMAP_COEF_SCALE) {
					int diff = isum - INTER_REMAP_COEF_SCALE;
					int ksize2 = ksize / 2, Mk1 = ksize2, Mk2 = ksize2, mk1 = ksize2, mk2 = ksize2;
					for (k1 = ksize2; k1 < ksize2 + 2; k1++) {
						for (k2 = ksize2; k2 < ksize2 + 2; k2++) {
							if (itab[k1*ksize + k2] < itab[mk1*ksize + mk2])
								mk1 = k1, mk2 = k2;
							else if (itab[k1*ksize + k2] > itab[Mk1*ksize + Mk2])
								Mk1 = k1, Mk2 = k2;
						}
					}
					if (diff < 0)
						itab[Mk1*ksize + Mk2] = (short)(itab[Mk1*ksize + Mk2] - diff);
					else
						itab[mk1*ksize + mk2] = (short)(itab[mk1*ksize + mk2] - diff);
				}
			}
		}
		tab -= INTER_TAB_SIZE2*ksize*ksize;
		itab -= INTER_TAB_SIZE2*ksize*ksize;
		inittab[method] = true;
	}

	return fixpt ? (const void*)itab : (const void*)tab;
}

template<typename _Tp>
static bool initAllInterTab2D()
{
	return  initInterTab2D<uchar>(INTER_LINEAR, false) &&
		initInterTab2D<uchar>(INTER_LINEAR, true) &&
		initInterTab2D<uchar>(INTER_CUBIC, false) &&
		initInterTab2D<uchar>(INTER_CUBIC, true) &&
		initInterTab2D<uchar>(INTER_LANCZOS4, false) &&
		initInterTab2D<uchar>(INTER_LANCZOS4, true);
}

static volatile bool doInitAllInterTab2D = initAllInterTab2D<uchar>();

template<typename _Tp1, typename _Tp2, int chs1, int chs2>
static void remapNearest(const Mat_<_Tp1, chs1>& _src, Mat_<_Tp1, chs1>& _dst, const Mat_<_Tp2, chs2>& _xy, int borderType, const Scalar& _borderValue)
{

}

template<typename _Tp1, typename _Tp2, int chs1, int chs2>
static int remap_nearest(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue)
{
	const void* ctab = 0;
	bool fixpt = sizeof(_Tp1) == 1; // uchar
	bool planar_input = map1.channels == 1;
	Range range(0, dst.rows);

	int x, y, x1, y1;
	const int buf_size = 1 << 14;
	int brows0 = std::min(128, dst.rows);
	int bcols0 = std::min(buf_size / brows0, dst.cols);
	brows0 = std::min(buf_size / bcols0, dst.rows);

	Mat_<short, 2> _bufxy(brows0, bcols0);

	for (y = range.start; y < range.end; y += brows0) {

		for (x = 0; x < dst.cols; x += bcols0) {
			int brows = std::min(brows0, range.end - y);
			int bcols = std::min(bcols0, dst.cols - x);
			Mat_<_Tp1, chs1> dpart;
			dst.getROI(dpart, Rect(x, y, bcols, brows));
			Mat_<short, 2> bufxy;
			_bufxy.getROI(bufxy, Rect(0, 0, bcols, brows));

			if (sizeof(_Tp2) == sizeof(short)) { // short
				for (y1 = 0; y1 < brows; y1++) {
					short* XY = (short*)bufxy.ptr(y1);
					const short* sXY = (const short*)map1.ptr(y + y1) + x * 2;
					const ushort* sA = (const ushort*)map2.ptr(y + y1) + x;

					for (x1 = 0; x1 < bcols; x1++) {
						int a = sA[x1] & (INTER_TAB_SIZE2 - 1);
						XY[x1 * 2] = sXY[x1 * 2] + NNDeltaTab_i[a][0];
						XY[x1 * 2 + 1] = sXY[x1 * 2 + 1] + NNDeltaTab_i[a][1];
					}
				}
			} else { // float
				for (y1 = 0; y1 < brows; y1++) {
					short* XY = (short*)bufxy.ptr(y1);
					const float* sX = (const float*)map1.ptr(y + y1) + x;
					const float* sY = (const float*)map2.ptr(y + y1) + x;

					x1 = 0;
					for (; x1 < bcols; x1++) {
						XY[x1 * 2] = saturate_cast<short>(sX[x1]);
						XY[x1 * 2 + 1] = saturate_cast<short>(sY[x1]);
					}
				}
			}

			
		}
	}

	return 0;
}

template<typename _Tp1, typename _Tp2, int chs1, int chs2>
static int remap_linear(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue)
{
	const void* ctab = 0;
	bool fixpt = sizeof(_Tp1) == 1; // uchar
	bool planar_input = map1.channels == 1;
	Range range(0, dst.rows);


	return 0;
}

template<typename _Tp1, typename _Tp2, int chs1, int chs2>
static int remap_cubic(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue)
{
	const void* ctab = 0;
	bool fixpt = sizeof(_Tp1) == 1; // uchar
	bool planar_input = map1.channels == 1;
	Range range(0, dst.rows);


	return 0;
}

template<typename _Tp1, typename _Tp2, int chs1, int chs2>
static int remap_lanczos4(const Mat_<_Tp1, chs1>& src, Mat_<_Tp1, chs1>& dst,
	const Mat_<_Tp2, chs2>& map1, const Mat_<_Tp2, chs2>& map2, int borderMode, const Scalar& borderValue)
{
	const void* ctab = 0;
	bool fixpt = sizeof(_Tp1) == 1; // uchar
	bool planar_input = map1.channels == 1;
	Range range(0, dst.rows);


	return 0;
}

} // namespace fbc

#endif // FBC_CV_REMAP_HPP_
