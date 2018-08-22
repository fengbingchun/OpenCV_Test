// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CVTCOLOR_HPP_
#define FBC_CV_CVTCOLOR_HPP_

/* reference: include/opencv2/imgproc.hpp
              imgproc/src/color.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "core/saturate.hpp"
#include "imgproc.hpp"
#include "core/core.hpp"

namespace fbc {
#define  FBC_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))

template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2Gray(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_Gray2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2YCrCb(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs_f, const int* coeffs_i);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_YCrCb2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs_f, const int* coeffs_i);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2XYZ(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_XYZ2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2HSV(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2HLS(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_HSV2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_HLS2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2Lab(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_RGB2Luv(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_Lab2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb);
template<typename _Tp, int chs1, int chs2> static int CvtColorLoop_Luv2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb);
template<typename _Tp, int chs, int bIdx, int uIdx> inline void cvtYUV420sp2RGB(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _uv);
template<typename _Tp, int chs, int bIdx, int uIdx> inline void cvtYUV420sp2RGBA(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _uv);
template<typename _Tp, int chs, int bIdx> inline void cvtYUV420p2RGB(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _u, const uchar* _v, int ustepIdx, int vstepIdx);
template<typename _Tp, int chs, int bIdx> inline void cvtYUV420p2RGBA(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _u, const uchar* _v, int ustepIdx, int vstepIdx);
template<typename _Tp, int chs1, int chs2, int bIdx, int uIdx> static void cvtRGBtoYUV420p(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst);


#undef R2Y
#undef G2Y
#undef B2Y

enum {
	yuv_shift = 14,
	xyz_shift = 12,
	R2Y = 4899,
	G2Y = 9617,
	B2Y = 1868,
	BLOCK_SIZE = 256
};

// Converts an image from one color space to another
// support type: uchar/ushort/float
template<typename _Tp, int chs1, int chs2>
int cvtColor(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int code)
{
	FBC_Assert(src.cols > 0 &&  src.rows > 0 && dst.cols > 0 && dst.rows > 0);
	FBC_Assert(src.cols == dst.cols);
	FBC_Assert(src.data != NULL && dst.data != NULL);
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() ||
		typeid(ushort).name() == typeid(_Tp).name() ||
		typeid(float).name() == typeid(_Tp).name());
	FBC_Assert((sizeof(_Tp) == 1) || sizeof(_Tp) == 2 || sizeof(_Tp) == 4); // uchar || ushort || float

	int scn = src.channels;
	int dcn = dst.channels; // number of channels in the destination image
	Size sz = src.size();
	Size dz = dst.size();
	int bidx;

	switch (code) {
		case CV_BGR2BGRA: case CV_RGB2BGRA: case CV_BGRA2BGR:
		case CV_RGBA2BGR: case CV_RGB2BGR: case CV_BGRA2RGBA: {
			FBC_Assert(scn == 3 || scn == 4);
			dcn = code == CV_BGR2BGRA || code == CV_RGB2BGRA || code == CV_BGRA2RGBA ? 4 : 3;
			FBC_Assert(dst.channels == dcn);
			bidx = code == CV_BGR2BGRA || code == CV_BGRA2BGR ? 0 : 2;

			CvtColorLoop_RGB2RGB(src, dst, bidx); // uchar/ushort/float
			break;
		}
		case CV_BGR2GRAY: case CV_BGRA2GRAY: case CV_RGB2GRAY: case CV_RGBA2GRAY: {
			FBC_Assert(scn == 3 || scn == 4);
			FBC_Assert(dst.channels == 1);
			bidx = code == CV_BGR2GRAY || code == CV_BGRA2GRAY ? 0 : 2;

			CvtColorLoop_RGB2Gray(src, dst, bidx);
			break;
		}
		case CV_GRAY2BGR: case CV_GRAY2BGRA: {
			FBC_Assert(scn == 1 && (dcn == 3 || dcn == 4));

			CvtColorLoop_Gray2RGB(src, dst);
			break;
		}
		case CV_BGR2YCrCb: case CV_RGB2YCrCb:
		case CV_BGR2YUV: case CV_RGB2YUV: {
			FBC_Assert(scn == 3 || scn == 4);
			bidx = code == CV_BGR2YCrCb || code == CV_BGR2YUV ? 0 : 2;
			static const float yuv_f[] = { 0.114f, 0.587f, 0.299f, 0.492f, 0.877f };
			static const int yuv_i[] = { B2Y, G2Y, R2Y, 8061, 14369 };
			const float* coeffs_f = code == CV_BGR2YCrCb || code == CV_RGB2YCrCb ? 0 : yuv_f;
			const int* coeffs_i = code == CV_BGR2YCrCb || code == CV_RGB2YCrCb ? 0 : yuv_i;

			CvtColorLoop_RGB2YCrCb(src, dst, bidx, coeffs_f, coeffs_i);
			break;
		}
		case CV_YCrCb2BGR: case CV_YCrCb2RGB:
		case CV_YUV2BGR: case CV_YUV2RGB: {
			FBC_Assert(scn == 3 && (dcn == 3 || dcn == 4));
			bidx = code == CV_YCrCb2BGR || code == CV_YUV2BGR ? 0 : 2;
			static const float yuv_f[] = { 2.032f, -0.395f, -0.581f, 1.140f };
			static const int yuv_i[] = { 33292, -6472, -9519, 18678 };
			const float* coeffs_f = code == CV_YCrCb2BGR || code == CV_YCrCb2RGB ? 0 : yuv_f;
			const int* coeffs_i = code == CV_YCrCb2BGR || code == CV_YCrCb2RGB ? 0 : yuv_i;

			CvtColorLoop_YCrCb2RGB(src, dst, bidx, coeffs_f, coeffs_i);
			break;
		}
		case CV_BGR2XYZ: case CV_RGB2XYZ: {
			FBC_Assert(scn == 3 || scn == 4);
			bidx = code == CV_BGR2XYZ ? 0 : 2;

			CvtColorLoop_RGB2XYZ(src, dst, bidx);
			break;
		}
		case CV_XYZ2BGR: case CV_XYZ2RGB: {
			FBC_Assert(scn == 3 && (dcn == 3 || dcn == 4));
			bidx = code == CV_XYZ2BGR ? 0 : 2;

			CvtColorLoop_XYZ2RGB(src, dst, bidx);
			break;
		}
		case CV_BGR2HSV: case CV_RGB2HSV: case CV_BGR2HSV_FULL: case CV_RGB2HSV_FULL:
		case CV_BGR2HLS: case CV_RGB2HLS: case CV_BGR2HLS_FULL: case CV_RGB2HLS_FULL: {
			FBC_Assert(scn == 3 || scn == 4);
			FBC_Assert(sizeof(_Tp) == 1 || sizeof(_Tp) == 4);
			bidx = code == CV_BGR2HSV || code == CV_BGR2HLS ||
				code == CV_BGR2HSV_FULL || code == CV_BGR2HLS_FULL ? 0 : 2;
			int hrange = sizeof(_Tp) == 4 ? 360 : code == CV_BGR2HSV || code == CV_RGB2HSV ||
				code == CV_BGR2HLS || code == CV_RGB2HLS ? 180 : 256;

			if (code == CV_BGR2HSV || code == CV_RGB2HSV || code == CV_BGR2HSV_FULL || code == CV_RGB2HSV_FULL) {
				CvtColorLoop_RGB2HSV(src, dst, bidx, hrange);
			} else {
				CvtColorLoop_RGB2HLS(src, dst, bidx, hrange);
			}
			break;
		}
		case CV_HSV2BGR: case CV_HSV2RGB: case CV_HSV2BGR_FULL: case CV_HSV2RGB_FULL:
		case CV_HLS2BGR: case CV_HLS2RGB: case CV_HLS2BGR_FULL: case CV_HLS2RGB_FULL: {
			FBC_Assert(scn == 3 && (dcn == 3 || dcn == 4));
			FBC_Assert(sizeof(_Tp) == 1 || sizeof(_Tp) == 4);
			bidx = code == CV_HSV2BGR || code == CV_HLS2BGR ||
				code == CV_HSV2BGR_FULL || code == CV_HLS2BGR_FULL ? 0 : 2;
			int hrange = sizeof(_Tp) == 4 ? 360 : code == CV_HSV2BGR || code == CV_HSV2RGB ||
				code == CV_HLS2BGR || code == CV_HLS2RGB ? 180 : 255;

			if (code == CV_HSV2BGR || code == CV_HSV2RGB || code == CV_HSV2BGR_FULL || code == CV_HSV2RGB_FULL) {
				CvtColorLoop_HSV2RGB(src, dst, bidx, hrange);
			} else {
				CvtColorLoop_HLS2RGB(src, dst, bidx, hrange);
			}
			break;
		}
		case CV_BGR2Lab: case CV_RGB2Lab:
		case CV_BGR2Luv: case CV_RGB2Luv: {
			FBC_Assert(scn == 3 || scn == 4);
			FBC_Assert(sizeof(_Tp) == 1 || sizeof(_Tp) == 4);
			bidx = code == CV_BGR2Lab || code == CV_BGR2Luv ? 0 : 2;
			bool srgb = code == CV_BGR2Lab || code == CV_RGB2Lab || code == CV_BGR2Luv || code == CV_RGB2Luv;

			if (code == CV_BGR2Lab || code == CV_RGB2Lab) {
				CvtColorLoop_RGB2Lab(src, dst, bidx, 0, 0, srgb);
			} else {
				CvtColorLoop_RGB2Luv(src, dst, bidx, 0, 0, srgb);
			}
			break;
		}
		case CV_Lab2BGR: case CV_Lab2RGB:
		case CV_Luv2BGR: case CV_Luv2RGB: {
			FBC_Assert(scn == 3 && (dcn == 3 || dcn == 4));
			FBC_Assert(sizeof(_Tp) == 1 || sizeof(_Tp) == 4);
			bidx = code == CV_Lab2BGR || code == CV_Luv2BGR ? 0 : 2;
			bool srgb = code == CV_Lab2BGR || code == CV_Lab2RGB || code == CV_Luv2BGR || code == CV_Luv2RGB;

			if (code == CV_Lab2BGR || code == CV_Lab2RGB) {
				CvtColorLoop_Lab2RGB(src, dst, bidx, 0, 0, srgb);
			} else {
				CvtColorLoop_Luv2RGB(src, dst, bidx, 0, 0, srgb);
			}
			break;
		}
		case CV_YUV2BGR_NV21:  case CV_YUV2RGB_NV21:  case CV_YUV2BGR_NV12:  case CV_YUV2RGB_NV12:
		case CV_YUV2BGRA_NV21: case CV_YUV2RGBA_NV21: case CV_YUV2BGRA_NV12: case CV_YUV2RGBA_NV12: {
			// http://www.fourcc.org/yuv.php#NV21 == yuv420sp -> a plane of 8 bit Y samples followed by an interleaved V/U plane containing 8 bit 2x2 subsampled chroma samples
			// http://www.fourcc.org/yuv.php#NV12 -> a plane of 8 bit Y samples followed by an interleaved U/V plane containing 8 bit 2x2 subsampled colour difference samples

			dcn = (code == CV_YUV420sp2BGRA || code == CV_YUV420sp2RGBA || code == CV_YUV2BGRA_NV12 || code == CV_YUV2RGBA_NV12) ? 4 : 3;
			FBC_Assert(dcn == dst.channels);
			const int bIdx = (code == CV_YUV2BGR_NV21 || code == CV_YUV2BGRA_NV21 || code == CV_YUV2BGR_NV12 || code == CV_YUV2BGRA_NV12) ? 0 : 2;
			const int uIdx = (code == CV_YUV2BGR_NV21 || code == CV_YUV2BGRA_NV21 || code == CV_YUV2RGB_NV21 || code == CV_YUV2RGBA_NV21) ? 1 : 0;

			FBC_Assert(dcn == 3 || dcn == 4);
			FBC_Assert(sz.width % 2 == 0 && sz.height % 3 == 0);
			FBC_Assert(sizeof(_Tp) == 1);
			//Size dstSz(sz.width, sz.height * 2 / 3);
			FBC_Assert((sz.width == dz.width) && (dz.height = sz.height * 2 / 3));

			int srcstep = (int)src.step;
			const uchar* y = src.ptr();
			const uchar* uv = y + srcstep * dz.height;

			switch (dcn * 100 + bIdx * 10 + uIdx) {
				case 300: cvtYUV420sp2RGB<_Tp, chs2, 0, 0>(dst, srcstep, y, uv); break;
				case 301: cvtYUV420sp2RGB<_Tp, chs2, 0, 1>(dst, srcstep, y, uv); break;
				case 320: cvtYUV420sp2RGB<_Tp, chs2, 2, 0>(dst, srcstep, y, uv); break;
				case 321: cvtYUV420sp2RGB<_Tp, chs2, 2, 1>(dst, srcstep, y, uv); break;
				case 400: cvtYUV420sp2RGBA<_Tp, chs2, 0, 0>(dst, srcstep, y, uv); break;
				case 401: cvtYUV420sp2RGBA<_Tp, chs2, 0, 1>(dst, srcstep, y, uv); break;
				case 420: cvtYUV420sp2RGBA<_Tp, chs2, 2, 0>(dst, srcstep, y, uv); break;
				case 421: cvtYUV420sp2RGBA<_Tp, chs2, 2, 1>(dst, srcstep, y, uv); break;
				default: FBC_Error("Unknown/unsupported color conversion code"); break;
			};
			break;
		}
		case CV_YUV2BGR_YV12: case CV_YUV2RGB_YV12: case CV_YUV2BGRA_YV12: case CV_YUV2RGBA_YV12:
		case CV_YUV2BGR_IYUV: case CV_YUV2RGB_IYUV: case CV_YUV2BGRA_IYUV: case CV_YUV2RGBA_IYUV: {
			//http://www.fourcc.org/yuv.php#YV12 == yuv420p -> It comprises an NxM Y plane followed by (N/2)x(M/2) V and U planes.
			//http://www.fourcc.org/yuv.php#IYUV == I420 -> It comprises an NxN Y plane followed by (N/2)x(N/2) U and V planes

			dcn = (code == CV_YUV2BGRA_YV12 || code == CV_YUV2RGBA_YV12 || code == CV_YUV2RGBA_IYUV || code == CV_YUV2BGRA_IYUV) ? 4 : 3;
			FBC_Assert(dcn == dst.channels);
			const int bIdx = (code == CV_YUV2BGR_YV12 || code == CV_YUV2BGRA_YV12 || code == CV_YUV2BGR_IYUV || code == CV_YUV2BGRA_IYUV) ? 0 : 2;
			const int uIdx = (code == CV_YUV2BGR_YV12 || code == CV_YUV2RGB_YV12 || code == CV_YUV2BGRA_YV12 || code == CV_YUV2RGBA_YV12) ? 1 : 0;

			FBC_Assert(dcn == 3 || dcn == 4);
			FBC_Assert(sz.width % 2 == 0 && sz.height % 3 == 0);
			FBC_Assert(sizeof(_Tp) == 1);

			//Size dstSz(sz.width, sz.height * 2 / 3);
			FBC_Assert((sz.width == dz.width) && (dz.height = sz.height * 2 / 3));

			int srcstep = (int)src.step;
			const uchar* y = src.ptr();
			const uchar* u = y + srcstep * dz.height;
			const uchar* v = y + srcstep * (dz.height + dz.height / 4) + (dz.width / 2) * ((dz.height % 4) / 2);

			int ustepIdx = 0;
			int vstepIdx = dz.height % 4 == 2 ? 1 : 0;

			if (uIdx == 1) { std::swap(u, v), std::swap(ustepIdx, vstepIdx); }

			switch (dcn * 10 + bIdx) {
				case 30: cvtYUV420p2RGB<_Tp, chs2, 0>(dst, srcstep, y, u, v, ustepIdx, vstepIdx); break;
				case 32: cvtYUV420p2RGB<_Tp, chs2, 2>(dst, srcstep, y, u, v, ustepIdx, vstepIdx); break;
				case 40: cvtYUV420p2RGBA<_Tp, chs2, 0>(dst, srcstep, y, u, v, ustepIdx, vstepIdx); break;
				case 42: cvtYUV420p2RGBA<_Tp, chs2, 2>(dst, srcstep, y, u, v, ustepIdx, vstepIdx); break;
				default: FBC_Error("Unknown/unsupported color conversion code"); break;
			};
			break;
		}
		case CV_YUV2GRAY_420: {
			FBC_Assert(dcn == 1 && scn == 1);
			FBC_Assert(sz.width % 2 == 0 && sz.height % 3 == 0);
			FBC_Assert(sizeof(_Tp) == 1);

			//Size dstSz(sz.width, sz.height * 2 / 3);
			FBC_Assert((sz.width == dz.width) && (dz.height = sz.height * 2 / 3));

			//src.copyTo(dst, Rect(0, 0, sz.width, dz.height));
			memcpy(dst.data, src.data, dz.area());

			break;
		}
		case CV_RGB2YUV_YV12: case CV_BGR2YUV_YV12: case CV_RGBA2YUV_YV12: case CV_BGRA2YUV_YV12:
		case CV_RGB2YUV_IYUV: case CV_BGR2YUV_IYUV: case CV_RGBA2YUV_IYUV: case CV_BGRA2YUV_IYUV: {
			const int bIdx = (code == CV_BGR2YUV_IYUV || code == CV_BGRA2YUV_IYUV || code == CV_BGR2YUV_YV12 || code == CV_BGRA2YUV_YV12) ? 0 : 2;
			const int uIdx = (code == CV_BGR2YUV_IYUV || code == CV_BGRA2YUV_IYUV || code == CV_RGB2YUV_IYUV || code == CV_RGBA2YUV_IYUV) ? 1 : 2;

			FBC_Assert(scn == 3 || scn == 4);
			FBC_Assert(sizeof(_Tp) == 1);
			FBC_Assert(dcn == 1);
			FBC_Assert(sz.width % 2 == 0 && sz.height % 2 == 0);

			//Size dstSz(sz.width, sz.height / 2 * 3);
			FBC_Assert((dz.width == sz.width) && (sz.height / 2 * 3 == dz.height));

			switch (bIdx + uIdx * 10) {
				case 10: cvtRGBtoYUV420p<_Tp, chs1, chs2, 0, 1>(src, dst); break;
				case 12: cvtRGBtoYUV420p<_Tp, chs1, chs2, 2, 1>(src, dst); break;
				case 20: cvtRGBtoYUV420p<_Tp, chs1, chs2, 0, 2>(src, dst); break;
				case 22: cvtRGBtoYUV420p<_Tp, chs1, chs2, 2, 2>(src, dst); break;
				default: FBC_Error("Unknown/unsupported color conversion code"); break;
			};

			break;
		}
		default:
			FBC_Error("Unknown/unsupported color conversion code");
	}

	return 0;
}

// computes cubic spline coefficients for a function: (xi=i, yi=f[i]), i=0..n
template<typename _Tp> static void splineBuild(const _Tp* f, int n, _Tp* tab)
{
	_Tp cn = 0;
	int i;
	tab[0] = tab[1] = (_Tp)0;

	for (i = 1; i < n - 1; i++) {
		_Tp t = 3 * (f[i + 1] - 2 * f[i] + f[i - 1]);
		_Tp l = 1 / (4 - tab[(i - 1) * 4]);
		tab[i * 4] = l; tab[i * 4 + 1] = (t - tab[(i - 1) * 4 + 1])*l;
	}

	for (i = n - 1; i >= 0; i--) {
		_Tp c = tab[i * 4 + 1] - tab[i * 4] * cn;
		_Tp b = f[i + 1] - f[i] - (cn + c * 2)*(_Tp)0.3333333333333333;
		_Tp d = (cn - c)*(_Tp)0.3333333333333333;
		tab[i * 4] = f[i]; tab[i * 4 + 1] = b;
		tab[i * 4 + 2] = c; tab[i * 4 + 3] = d;
		cn = c;
	}
}

// interpolates value of a function at x, 0 <= x <= n using a cubic spline.
template<typename _Tp> static inline _Tp splineInterpolate(_Tp x, const _Tp* tab, int n)
{
	// don't touch this function without urgent need - some versions of gcc fail to inline it correctly
	int ix = std::min(std::max(int(x), 0), n - 1);
	x -= ix;
	tab += ix * 4;
	return ((tab[3] * x + tab[2])*x + tab[1])*x + tab[0];
}

template<typename _Tp> struct ColorChannel
{
	typedef float worktype_f;
	static _Tp max() { return std::numeric_limits<_Tp>::max(); }
	static _Tp half() { return (_Tp)(max() / 2 + 1); }
};

template<> struct ColorChannel<float>
{
	typedef float worktype_f;
	static float max() { return 1.f; }
	static float half() { return 0.5f; }
};

template<typename _Tp> struct RGB2Gray
{
	typedef _Tp channel_type;

	RGB2Gray(int _srccn, int blueIdx, const float* _coeffs) : srccn(_srccn)
	{
		static const float coeffs0[] = { 0.299f, 0.587f, 0.114f };
		memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 3 * sizeof(coeffs[0]));
		if (blueIdx == 0)
			std::swap(coeffs[0], coeffs[2]);
	}

	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int scn = srccn;
		float cb = coeffs[0], cg = coeffs[1], cr = coeffs[2];
		for (int i = 0; i < n; i++, src += scn)
			dst[i] = saturate_cast<_Tp>(src[0] * cb + src[1] * cg + src[2] * cr);
	}
	int srccn;
	float coeffs[3];
};

template<> struct RGB2Gray<uchar>
{
	typedef uchar channel_type;

	RGB2Gray(int _srccn, int blueIdx, const int* coeffs) : srccn(_srccn)
	{
		const int coeffs0[] = { R2Y, G2Y, B2Y };
		if (!coeffs) coeffs = coeffs0;

		int b = 0, g = 0, r = (1 << (yuv_shift - 1));
		int db = coeffs[blueIdx ^ 2], dg = coeffs[1], dr = coeffs[blueIdx];

		for (int i = 0; i < 256; i++, b += db, g += dg, r += dr) {
			tab[i] = b;
			tab[i + 256] = g;
			tab[i + 512] = r;
		}
	}
	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int scn = srccn;
		const int* _tab = tab;
		for (int i = 0; i < n; i++, src += scn)
			dst[i] = (uchar)((_tab[src[0]] + _tab[src[1] + 256] + _tab[src[2] + 512]) >> yuv_shift);
	}
	int srccn;
	int tab[256 * 3];
};

template<> struct RGB2Gray<ushort>
{
	typedef ushort channel_type;

	RGB2Gray(int _srccn, int blueIdx, const int* _coeffs) : srccn(_srccn)
	{
		static const int coeffs0[] = { R2Y, G2Y, B2Y };
		memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 3 * sizeof(coeffs[0]));
		if (blueIdx == 0)
			std::swap(coeffs[0], coeffs[2]);
	}

	void operator()(const ushort* src, ushort* dst, int n) const
	{
		int scn = srccn, cb = coeffs[0], cg = coeffs[1], cr = coeffs[2];
		for (int i = 0; i < n; i++, src += scn)
			dst[i] = (ushort)FBC_DESCALE((unsigned)(src[0] * cb + src[1] * cg + src[2] * cr), yuv_shift);
	}
	int srccn;
	int coeffs[3];
};

template<typename _Tp>
struct Gray2RGB
{
	typedef _Tp channel_type;

	Gray2RGB(int _dstcn) : dstcn(_dstcn) {}
	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		if (dstcn == 3) {
			for (int i = 0; i < n; i++, dst += 3)
				dst[0] = dst[1] = dst[2] = src[i];
		} else {
			_Tp alpha = ColorChannel<_Tp>::max();
			for (int i = 0; i < n; i++, dst += 4) {
				dst[0] = dst[1] = dst[2] = src[i];
				dst[3] = alpha;
			}
		}
	}

	int dstcn;
};

template<typename _Tp> struct RGB2YCrCb_f
{
	typedef _Tp channel_type;

	RGB2YCrCb_f(int _srccn, int _blueIdx, const float* _coeffs) : srccn(_srccn), blueIdx(_blueIdx)
	{
		static const float coeffs0[] = { 0.299f, 0.587f, 0.114f, 0.713f, 0.564f };
		memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 5 * sizeof(coeffs[0]));
		if (blueIdx == 0) std::swap(coeffs[0], coeffs[2]);
	}

	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int scn = srccn, bidx = blueIdx;
		const _Tp delta = ColorChannel<_Tp>::half();
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2], C3 = coeffs[3], C4 = coeffs[4];
		n *= 3;
		for (int i = 0; i < n; i += 3, src += scn) {
			_Tp Y = saturate_cast<_Tp>(src[0] * C0 + src[1] * C1 + src[2] * C2);
			_Tp Cr = saturate_cast<_Tp>((src[bidx ^ 2] - Y)*C3 + delta);
			_Tp Cb = saturate_cast<_Tp>((src[bidx] - Y)*C4 + delta);
			dst[i] = Y; dst[i + 1] = Cr; dst[i + 2] = Cb;
		}
	}
	int srccn, blueIdx;
	float coeffs[5];
};

template<typename _Tp> struct RGB2YCrCb_i
{
	typedef _Tp channel_type;

	RGB2YCrCb_i(int _srccn, int _blueIdx, const int* _coeffs) : srccn(_srccn), blueIdx(_blueIdx)
	{
		static const int coeffs0[] = { R2Y, G2Y, B2Y, 11682, 9241 };
		memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 5 * sizeof(coeffs[0]));
		if (blueIdx == 0) std::swap(coeffs[0], coeffs[2]);
	}

	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int scn = srccn, bidx = blueIdx;
		int C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2], C3 = coeffs[3], C4 = coeffs[4];
		int delta = ColorChannel<_Tp>::half()*(1 << yuv_shift);
		n *= 3;
		for (int i = 0; i < n; i += 3, src += scn) {
			int Y = FBC_DESCALE(src[0] * C0 + src[1] * C1 + src[2] * C2, yuv_shift);
			int Cr = FBC_DESCALE((src[bidx ^ 2] - Y)*C3 + delta, yuv_shift);
			int Cb = FBC_DESCALE((src[bidx] - Y)*C4 + delta, yuv_shift);
			dst[i] = saturate_cast<_Tp>(Y);
			dst[i + 1] = saturate_cast<_Tp>(Cr);
			dst[i + 2] = saturate_cast<_Tp>(Cb);
		}
	}
	int srccn, blueIdx;
	int coeffs[5];
};

template<typename _Tp> struct YCrCb2RGB_f
{
	typedef _Tp channel_type;

	YCrCb2RGB_f(int _dstcn, int _blueIdx, const float* _coeffs)
		: dstcn(_dstcn), blueIdx(_blueIdx)
	{
		static const float coeffs0[] = { 1.403f, -0.714f, -0.344f, 1.773f };
		memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 4 * sizeof(coeffs[0]));
	}
	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int dcn = dstcn, bidx = blueIdx;
		const _Tp delta = ColorChannel<_Tp>::half(), alpha = ColorChannel<_Tp>::max();
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2], C3 = coeffs[3];
		n *= 3;
		for (int i = 0; i < n; i += 3, dst += dcn) {
			_Tp Y = src[i];
			_Tp Cr = src[i + 1];
			_Tp Cb = src[i + 2];

			_Tp b = saturate_cast<_Tp>(Y + (Cb - delta)*C3);
			_Tp g = saturate_cast<_Tp>(Y + (Cb - delta)*C2 + (Cr - delta)*C1);
			_Tp r = saturate_cast<_Tp>(Y + (Cr - delta)*C0);

			dst[bidx] = b; dst[1] = g; dst[bidx ^ 2] = r;
			if (dcn == 4)
				dst[3] = alpha;
		}
	}
	int dstcn, blueIdx;
	float coeffs[4];
};

template<typename _Tp> struct YCrCb2RGB_i
{
	typedef _Tp channel_type;

	YCrCb2RGB_i(int _dstcn, int _blueIdx, const int* _coeffs)
		: dstcn(_dstcn), blueIdx(_blueIdx)
	{
		static const int coeffs0[] = { 22987, -11698, -5636, 29049 };
		memcpy(coeffs, _coeffs ? _coeffs : coeffs0, 4 * sizeof(coeffs[0]));
	}

	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int dcn = dstcn, bidx = blueIdx;
		const _Tp delta = ColorChannel<_Tp>::half(), alpha = ColorChannel<_Tp>::max();
		int C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2], C3 = coeffs[3];
		n *= 3;
		for (int i = 0; i < n; i += 3, dst += dcn) {
			_Tp Y = src[i];
			_Tp Cr = src[i + 1];
			_Tp Cb = src[i + 2];

			int b = Y + FBC_DESCALE((Cb - delta)*C3, yuv_shift);
			int g = Y + FBC_DESCALE((Cb - delta)*C2 + (Cr - delta)*C1, yuv_shift);
			int r = Y + FBC_DESCALE((Cr - delta)*C0, yuv_shift);

			dst[bidx] = saturate_cast<_Tp>(b);
			dst[1] = saturate_cast<_Tp>(g);
			dst[bidx ^ 2] = saturate_cast<_Tp>(r);
			if (dcn == 4)
				dst[3] = alpha;
		}
	}
	int dstcn, blueIdx;
	int coeffs[4];
};

static const float sRGB2XYZ_D65[] =
{
	0.412453f, 0.357580f, 0.180423f,
	0.212671f, 0.715160f, 0.072169f,
	0.019334f, 0.119193f, 0.950227f
};

static const float XYZ2sRGB_D65[] =
{
	3.240479f, -1.53715f, -0.498535f,
	-0.969256f, 1.875991f, 0.041556f,
	0.055648f, -0.204043f, 1.057311f
};

template<typename _Tp> struct RGB2XYZ_f
{
	typedef _Tp channel_type;

	RGB2XYZ_f(int _srccn, int blueIdx, const float* _coeffs) : srccn(_srccn)
	{
		memcpy(coeffs, _coeffs ? _coeffs : sRGB2XYZ_D65, 9 * sizeof(coeffs[0]));
		if (blueIdx == 0) {
			std::swap(coeffs[0], coeffs[2]);
			std::swap(coeffs[3], coeffs[5]);
			std::swap(coeffs[6], coeffs[8]);
		}
	}
	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int scn = srccn;
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];

		n *= 3;
		for (int i = 0; i < n; i += 3, src += scn) {
			_Tp X = saturate_cast<_Tp>(src[0] * C0 + src[1] * C1 + src[2] * C2);
			_Tp Y = saturate_cast<_Tp>(src[0] * C3 + src[1] * C4 + src[2] * C5);
			_Tp Z = saturate_cast<_Tp>(src[0] * C6 + src[1] * C7 + src[2] * C8);
			dst[i] = X; dst[i + 1] = Y; dst[i + 2] = Z;
		}
	}
	int srccn;
	float coeffs[9];
};

template<typename _Tp> struct RGB2XYZ_i
{
	typedef _Tp channel_type;

	RGB2XYZ_i(int _srccn, int blueIdx, const float* _coeffs) : srccn(_srccn)
	{
		static const int coeffs0[] = {
			1689, 1465, 739,
			871, 2929, 296,
			79, 488, 3892
		};
		for (int i = 0; i < 9; i++)
			coeffs[i] = _coeffs ? fbcRound(_coeffs[i] * (1 << xyz_shift)) : coeffs0[i];
		if (blueIdx == 0) {
			std::swap(coeffs[0], coeffs[2]);
			std::swap(coeffs[3], coeffs[5]);
			std::swap(coeffs[6], coeffs[8]);
		}
	}
	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int scn = srccn;
		int C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		n *= 3;

		for (int i = 0; i < n; i += 3, src += scn) {
			int X = FBC_DESCALE(src[0] * C0 + src[1] * C1 + src[2] * C2, xyz_shift);
			int Y = FBC_DESCALE(src[0] * C3 + src[1] * C4 + src[2] * C5, xyz_shift);
			int Z = FBC_DESCALE(src[0] * C6 + src[1] * C7 + src[2] * C8, xyz_shift);
			dst[i] = saturate_cast<_Tp>(X); dst[i + 1] = saturate_cast<_Tp>(Y);
			dst[i + 2] = saturate_cast<_Tp>(Z);
		}
	}
	int srccn;
	int coeffs[9];
};

template<typename _Tp> struct XYZ2RGB_f
{
	typedef _Tp channel_type;

	XYZ2RGB_f(int _dstcn, int _blueIdx, const float* _coeffs) : dstcn(_dstcn), blueIdx(_blueIdx)
	{
		memcpy(coeffs, _coeffs ? _coeffs : XYZ2sRGB_D65, 9 * sizeof(coeffs[0]));
		if (blueIdx == 0) {
			std::swap(coeffs[0], coeffs[6]);
			std::swap(coeffs[1], coeffs[7]);
			std::swap(coeffs[2], coeffs[8]);
		}
	}

	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int dcn = dstcn;
		_Tp alpha = ColorChannel<_Tp>::max();
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		n *= 3;
		for (int i = 0; i < n; i += 3, dst += dcn) {
			_Tp B = saturate_cast<_Tp>(src[i] * C0 + src[i + 1] * C1 + src[i + 2] * C2);
			_Tp G = saturate_cast<_Tp>(src[i] * C3 + src[i + 1] * C4 + src[i + 2] * C5);
			_Tp R = saturate_cast<_Tp>(src[i] * C6 + src[i + 1] * C7 + src[i + 2] * C8);
			dst[0] = B; dst[1] = G; dst[2] = R;
			if (dcn == 4)
				dst[3] = alpha;
		}
	}
	int dstcn, blueIdx;
	float coeffs[9];
};

template<typename _Tp> struct XYZ2RGB_i
{
	typedef _Tp channel_type;

	XYZ2RGB_i(int _dstcn, int _blueIdx, const int* _coeffs)
		: dstcn(_dstcn), blueIdx(_blueIdx)
	{
		static const int coeffs0[] = {
			13273, -6296, -2042,
			-3970, 7684, 170,
			228, -836, 4331
		};
		for (int i = 0; i < 9; i++)
			coeffs[i] = _coeffs ? fbcRound(_coeffs[i] * (1 << xyz_shift)) : coeffs0[i];

		if (blueIdx == 0) {
			std::swap(coeffs[0], coeffs[6]);
			std::swap(coeffs[1], coeffs[7]);
			std::swap(coeffs[2], coeffs[8]);
		}
	}
	void operator()(const _Tp* src, _Tp* dst, int n) const
	{
		int dcn = dstcn;
		_Tp alpha = ColorChannel<_Tp>::max();
		int C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		n *= 3;
		for (int i = 0; i < n; i += 3, dst += dcn) {
			int B = FBC_DESCALE(src[i] * C0 + src[i + 1] * C1 + src[i + 2] * C2, xyz_shift);
			int G = FBC_DESCALE(src[i] * C3 + src[i + 1] * C4 + src[i + 2] * C5, xyz_shift);
			int R = FBC_DESCALE(src[i] * C6 + src[i + 1] * C7 + src[i + 2] * C8, xyz_shift);
			dst[0] = saturate_cast<_Tp>(B); dst[1] = saturate_cast<_Tp>(G);
			dst[2] = saturate_cast<_Tp>(R);
			if (dcn == 4)
				dst[3] = alpha;
		}
	}
	int dstcn, blueIdx;
	int coeffs[9];
};

struct RGB2HSV_b
{
	typedef uchar channel_type;

	RGB2HSV_b(int _srccn, int _blueIdx, int _hrange) : srccn(_srccn), blueIdx(_blueIdx), hrange(_hrange)
	{
		FBC_Assert(hrange == 180 || hrange == 256);
	}

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, bidx = blueIdx, scn = srccn;
		const int hsv_shift = 12;

		static int sdiv_table[256];
		static int hdiv_table180[256];
		static int hdiv_table256[256];
		static volatile bool initialized = false;

		int hr = hrange;
		const int* hdiv_table = hr == 180 ? hdiv_table180 : hdiv_table256;
		n *= 3;

		if (!initialized) {
			sdiv_table[0] = hdiv_table180[0] = hdiv_table256[0] = 0;
			for (i = 1; i < 256; i++) {
				sdiv_table[i] = saturate_cast<int>((255 << hsv_shift) / (1.*i));
				hdiv_table180[i] = saturate_cast<int>((180 << hsv_shift) / (6.*i));
				hdiv_table256[i] = saturate_cast<int>((256 << hsv_shift) / (6.*i));
			}
			initialized = true;
		}

		for (i = 0; i < n; i += 3, src += scn) {
			int b = src[bidx], g = src[1], r = src[bidx ^ 2];
			int h, s, v = b;
			int vmin = b, diff;
			int vr, vg;

			FBC_CALC_MAX_8U(v, g);
			FBC_CALC_MAX_8U(v, r);
			FBC_CALC_MIN_8U(vmin, g);
			FBC_CALC_MIN_8U(vmin, r);

			diff = v - vmin;
			vr = v == r ? -1 : 0;
			vg = v == g ? -1 : 0;

			s = (diff * sdiv_table[v] + (1 << (hsv_shift - 1))) >> hsv_shift;
			h = (vr & (g - b)) + (~vr & ((vg & (b - r + 2 * diff)) + ((~vg) & (r - g + 4 * diff))));
			h = (h * hdiv_table[diff] + (1 << (hsv_shift - 1))) >> hsv_shift;
			h += h < 0 ? hr : 0;

			dst[i] = saturate_cast<uchar>(h);
			dst[i + 1] = (uchar)s;
			dst[i + 2] = (uchar)v;
		}
	}

	int srccn, blueIdx, hrange;
};

struct RGB2HSV_f
{
	typedef float channel_type;

	RGB2HSV_f(int _srccn, int _blueIdx, float _hrange) : srccn(_srccn), blueIdx(_blueIdx), hrange(_hrange) {}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, bidx = blueIdx, scn = srccn;
		float hscale = hrange*(1.f / 360.f);
		n *= 3;

		for (i = 0; i < n; i += 3, src += scn) {
			float b = src[bidx], g = src[1], r = src[bidx ^ 2];
			float h, s, v;

			float vmin, diff;

			v = vmin = r;
			if (v < g) v = g;
			if (v < b) v = b;
			if (vmin > g) vmin = g;
			if (vmin > b) vmin = b;

			diff = v - vmin;
			s = diff / (float)(fabs(v) + FLT_EPSILON);
			diff = (float)(60. / (diff + FLT_EPSILON));
			if (v == r)
				h = (g - b)*diff;
			else if (v == g)
				h = (b - r)*diff + 120.f;
			else
				h = (r - g)*diff + 240.f;

			if (h < 0) h += 360.f;

			dst[i] = h*hscale;
			dst[i + 1] = s;
			dst[i + 2] = v;
		}
	}

	int srccn, blueIdx;
	float hrange;
};

struct RGB2HLS_f
{
	typedef float channel_type;

	RGB2HLS_f(int _srccn, int _blueIdx, float _hrange) : srccn(_srccn), blueIdx(_blueIdx), hrange(_hrange) {}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, bidx = blueIdx, scn = srccn;
		float hscale = hrange*(1.f / 360.f);
		n *= 3;

		for (i = 0; i < n; i += 3, src += scn) {
			float b = src[bidx], g = src[1], r = src[bidx ^ 2];
			float h = 0.f, s = 0.f, l;
			float vmin, vmax, diff;

			vmax = vmin = r;
			if (vmax < g) vmax = g;
			if (vmax < b) vmax = b;
			if (vmin > g) vmin = g;
			if (vmin > b) vmin = b;

			diff = vmax - vmin;
			l = (vmax + vmin)*0.5f;

			if (diff > FLT_EPSILON) {
				s = l < 0.5f ? diff / (vmax + vmin) : diff / (2 - vmax - vmin);
				diff = 60.f / diff;

				if (vmax == r)
					h = (g - b)*diff;
				else if (vmax == g)
					h = (b - r)*diff + 120.f;
				else
					h = (r - g)*diff + 240.f;

				if (h < 0.f) h += 360.f;
			}

			dst[i] = h*hscale;
			dst[i + 1] = l;
			dst[i + 2] = s;
		}
	}

	int srccn, blueIdx;
	float hrange;
};

struct RGB2HLS_b
{
	typedef uchar channel_type;

	RGB2HLS_b(int _srccn, int _blueIdx, int _hrange) : srccn(_srccn), cvt(3, _blueIdx, (float)_hrange) { }

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, j, scn = srccn;
		float FBC_DECL_ALIGNED(16) buf[3 * BLOCK_SIZE];

		for (i = 0; i < n; i += BLOCK_SIZE, dst += BLOCK_SIZE * 3) {
			int dn = std::min(n - i, (int)BLOCK_SIZE);
			j = 0;

			for (; j < dn * 3; j += 3, src += scn) {
				buf[j] = src[0] * (1.f / 255.f);
				buf[j + 1] = src[1] * (1.f / 255.f);
				buf[j + 2] = src[2] * (1.f / 255.f);
			}
			cvt(buf, buf, dn);

			j = 0;
			for (; j < dn * 3; j += 3) {
				dst[j] = saturate_cast<uchar>(buf[j]);
				dst[j + 1] = saturate_cast<uchar>(buf[j + 1] * 255.f);
				dst[j + 2] = saturate_cast<uchar>(buf[j + 2] * 255.f);
			}
		}
	}

	int srccn;
	RGB2HLS_f cvt;
};

struct HSV2RGB_f
{
	typedef float channel_type;

	HSV2RGB_f(int _dstcn, int _blueIdx, float _hrange)
		: dstcn(_dstcn), blueIdx(_blueIdx), hscale(6.f / _hrange) {}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, bidx = blueIdx, dcn = dstcn;
		float _hscale = hscale;
		float alpha = ColorChannel<float>::max();
		n *= 3;

		for (i = 0; i < n; i += 3, dst += dcn) {
			float h = src[i], s = src[i + 1], v = src[i + 2];
			float b, g, r;

			if (s == 0)
				b = g = r = v;
			else {
				static const int sector_data[][3] =
				{ { 1, 3, 0 }, { 1, 0, 2 }, { 3, 0, 1 }, { 0, 2, 1 }, { 0, 1, 3 }, { 2, 1, 0 } };
				float tab[4];
				int sector;
				h *= _hscale;
				if (h < 0)
					do h += 6; while (h < 0);
				else if (h >= 6)
					do h -= 6; while (h >= 6);
				sector = fbcFloor(h);
				h -= sector;
				if ((unsigned)sector >= 6u) {
					sector = 0;
					h = 0.f;
				}

				tab[0] = v;
				tab[1] = v*(1.f - s);
				tab[2] = v*(1.f - s*h);
				tab[3] = v*(1.f - s*(1.f - h));

				b = tab[sector_data[sector][0]];
				g = tab[sector_data[sector][1]];
				r = tab[sector_data[sector][2]];
			}

			dst[bidx] = b;
			dst[1] = g;
			dst[bidx ^ 2] = r;
			if (dcn == 4)
				dst[3] = alpha;
		}
	}

	int dstcn, blueIdx;
	float hscale;
};

struct HSV2RGB_b
{
	typedef uchar channel_type;

	HSV2RGB_b(int _dstcn, int _blueIdx, int _hrange) : dstcn(_dstcn), cvt(3, _blueIdx, (float)_hrange) {}

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, j, dcn = dstcn;
		uchar alpha = ColorChannel<uchar>::max();
		float FBC_DECL_ALIGNED(16) buf[3 * BLOCK_SIZE];

		for (i = 0; i < n; i += BLOCK_SIZE, src += BLOCK_SIZE * 3) {
			int dn = std::min(n - i, (int)BLOCK_SIZE);
			j = 0;

			for (; j < dn * 3; j += 3) {
				buf[j] = src[j];
				buf[j + 1] = src[j + 1] * (1.f / 255.f);
				buf[j + 2] = src[j + 2] * (1.f / 255.f);
			}
			cvt(buf, buf, dn);

			j = 0;
			for (; j < dn * 3; j += 3, dst += dcn) {
				dst[0] = saturate_cast<uchar>(buf[j] * 255.f);
				dst[1] = saturate_cast<uchar>(buf[j + 1] * 255.f);
				dst[2] = saturate_cast<uchar>(buf[j + 2] * 255.f);
				if (dcn == 4)
					dst[3] = alpha;
			}
		}
	}

	int dstcn;
	HSV2RGB_f cvt;
};

struct HLS2RGB_f
{
	typedef float channel_type;

	HLS2RGB_f(int _dstcn, int _blueIdx, float _hrange) : dstcn(_dstcn), blueIdx(_blueIdx), hscale(6.f / _hrange) {}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, bidx = blueIdx, dcn = dstcn;
		float _hscale = hscale;
		float alpha = ColorChannel<float>::max();
		n *= 3;

		for (i = 0; i < n; i += 3, dst += dcn) {
			float h = src[i], l = src[i + 1], s = src[i + 2];
			float b, g, r;

			if (s == 0)
				b = g = r = l;
			else {
				static const int sector_data[][3] =
				{ { 1, 3, 0 }, { 1, 0, 2 }, { 3, 0, 1 }, { 0, 2, 1 }, { 0, 1, 3 }, { 2, 1, 0 } };
				float tab[4];
				int sector;

				float p2 = l <= 0.5f ? l*(1 + s) : l + s - l*s;
				float p1 = 2 * l - p2;

				h *= _hscale;
				if (h < 0)
					do h += 6; while (h < 0);
				else if (h >= 6)
					do h -= 6; while (h >= 6);

				assert(0 <= h && h < 6);
				sector = fbcFloor(h);
				h -= sector;

				tab[0] = p2;
				tab[1] = p1;
				tab[2] = p1 + (p2 - p1)*(1 - h);
				tab[3] = p1 + (p2 - p1)*h;

				b = tab[sector_data[sector][0]];
				g = tab[sector_data[sector][1]];
				r = tab[sector_data[sector][2]];
			}

			dst[bidx] = b;
			dst[1] = g;
			dst[bidx ^ 2] = r;
			if (dcn == 4)
				dst[3] = alpha;
		}
	}

	int dstcn, blueIdx;
	float hscale;
};

struct HLS2RGB_b
{
	typedef uchar channel_type;

	HLS2RGB_b(int _dstcn, int _blueIdx, int _hrange) : dstcn(_dstcn), cvt(3, _blueIdx, (float)_hrange) { }

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, j, dcn = dstcn;
		uchar alpha = ColorChannel<uchar>::max();
		float FBC_DECL_ALIGNED(16) buf[3 * BLOCK_SIZE];

		for (i = 0; i < n; i += BLOCK_SIZE, src += BLOCK_SIZE * 3) {
			int dn = std::min(n - i, (int)BLOCK_SIZE);
			j = 0;

			for (; j < dn * 3; j += 3) {
				buf[j] = src[j];
				buf[j + 1] = src[j + 1] * (1.f / 255.f);
				buf[j + 2] = src[j + 2] * (1.f / 255.f);
			}
			cvt(buf, buf, dn);

			j = 0;
			for (; j < dn * 3; j += 3, dst += dcn) {
				dst[0] = saturate_cast<uchar>(buf[j] * 255.f);
				dst[1] = saturate_cast<uchar>(buf[j + 1] * 255.f);
				dst[2] = saturate_cast<uchar>(buf[j + 2] * 255.f);
				if (dcn == 4)
					dst[3] = alpha;
			}
		}
	}

	int dstcn;
	HLS2RGB_f cvt;
};

static const float D65[] = { 0.950456f, 1.f, 1.088754f };

enum { LAB_CBRT_TAB_SIZE = 1024, GAMMA_TAB_SIZE = 1024 };
static float LabCbrtTab[LAB_CBRT_TAB_SIZE * 4];
static const float LabCbrtTabScale = LAB_CBRT_TAB_SIZE / 1.5f;

static float sRGBGammaTab[GAMMA_TAB_SIZE * 4], sRGBInvGammaTab[GAMMA_TAB_SIZE * 4];
static const float GammaTabScale = (float)GAMMA_TAB_SIZE;

static ushort sRGBGammaTab_b[256], linearGammaTab_b[256];
#undef lab_shift
#define lab_shift xyz_shift
#define gamma_shift 3
#define lab_shift2 (lab_shift + gamma_shift)
#define LAB_CBRT_TAB_SIZE_B (256*3/2*(1<<gamma_shift))
static ushort LabCbrtTab_b[LAB_CBRT_TAB_SIZE_B];

static void initLabTabs()
{
	static bool initialized = false;
	if (!initialized) {
		float f[LAB_CBRT_TAB_SIZE + 1], g[GAMMA_TAB_SIZE + 1], ig[GAMMA_TAB_SIZE + 1], scale = 1.f / LabCbrtTabScale;
		int i;
		for (i = 0; i <= LAB_CBRT_TAB_SIZE; i++) {
			float x = i*scale;
			f[i] = x < 0.008856f ? x*7.787f + 0.13793103448275862f : fbcCbrt(x);
		}
		splineBuild(f, LAB_CBRT_TAB_SIZE, LabCbrtTab);

		scale = 1.f / GammaTabScale;
		for (i = 0; i <= GAMMA_TAB_SIZE; i++) {
			float x = i*scale;
			g[i] = x <= 0.04045f ? x*(1.f / 12.92f) : (float)std::pow((double)(x + 0.055)*(1. / 1.055), 2.4);
			ig[i] = x <= 0.0031308 ? x*12.92f : (float)(1.055*std::pow((double)x, 1. / 2.4) - 0.055);
		}
		splineBuild(g, GAMMA_TAB_SIZE, sRGBGammaTab);
		splineBuild(ig, GAMMA_TAB_SIZE, sRGBInvGammaTab);

		for (i = 0; i < 256; i++) {
			float x = i*(1.f / 255.f);
			sRGBGammaTab_b[i] = saturate_cast<ushort>(255.f*(1 << gamma_shift)*(x <= 0.04045f ? x*(1.f / 12.92f) : (float)std::pow((double)(x + 0.055)*(1. / 1.055), 2.4)));
			linearGammaTab_b[i] = (ushort)(i*(1 << gamma_shift));
		}

		for (i = 0; i < LAB_CBRT_TAB_SIZE_B; i++) {
			float x = i*(1.f / (255.f*(1 << gamma_shift)));
			LabCbrtTab_b[i] = saturate_cast<ushort>((1 << lab_shift2)*(x < 0.008856f ? x*7.787f + 0.13793103448275862f : fbcCbrt(x)));
		}
		initialized = true;
	}
}

struct RGB2Lab_b
{
	typedef uchar channel_type;

	RGB2Lab_b(int _srccn, int blueIdx, const float* _coeffs, const float* _whitept, bool _srgb) : srccn(_srccn), srgb(_srgb)
	{
		static volatile int _3 = 3;
		initLabTabs();

		if (!_coeffs)
			_coeffs = sRGB2XYZ_D65;
		if (!_whitept)
			_whitept = D65;

		float scale[] = {
			(1 << lab_shift) / _whitept[0],
			(float)(1 << lab_shift),
			(1 << lab_shift) / _whitept[2]
		};

		for (int i = 0; i < _3; i++) {
			coeffs[i * 3 + (blueIdx ^ 2)] = fbcRound(_coeffs[i * 3] * scale[i]);
			coeffs[i * 3 + 1] = fbcRound(_coeffs[i * 3 + 1] * scale[i]);
			coeffs[i * 3 + blueIdx] = fbcRound(_coeffs[i * 3 + 2] * scale[i]);

			FBC_Assert(coeffs[i] >= 0 && coeffs[i * 3 + 1] >= 0 && coeffs[i * 3 + 2] >= 0 &&
				coeffs[i * 3] + coeffs[i * 3 + 1] + coeffs[i * 3 + 2] < 2 * (1 << lab_shift));
		}
	}

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		const int Lscale = (116 * 255 + 50) / 100;
		const int Lshift = -((16 * 255 * (1 << lab_shift2) + 50) / 100);
		const ushort* tab = srgb ? sRGBGammaTab_b : linearGammaTab_b;
		int i, scn = srccn;
		int C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		n *= 3;

		for (i = 0; i < n; i += 3, src += scn) {
			int R = tab[src[0]], G = tab[src[1]], B = tab[src[2]];
			int fX = LabCbrtTab_b[FBC_DESCALE(R*C0 + G*C1 + B*C2, lab_shift)];
			int fY = LabCbrtTab_b[FBC_DESCALE(R*C3 + G*C4 + B*C5, lab_shift)];
			int fZ = LabCbrtTab_b[FBC_DESCALE(R*C6 + G*C7 + B*C8, lab_shift)];

			int L = FBC_DESCALE(Lscale*fY + Lshift, lab_shift2);
			int a = FBC_DESCALE(500 * (fX - fY) + 128 * (1 << lab_shift2), lab_shift2);
			int b = FBC_DESCALE(200 * (fY - fZ) + 128 * (1 << lab_shift2), lab_shift2);

			dst[i] = saturate_cast<uchar>(L);
			dst[i + 1] = saturate_cast<uchar>(a);
			dst[i + 2] = saturate_cast<uchar>(b);
		}
	}

	int srccn;
	int coeffs[9];
	bool srgb;
};

template<typename _Tp> static _Tp clip(_Tp value)
{
	return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
}

struct RGB2Lab_f
{
	typedef float channel_type;

	RGB2Lab_f(int _srccn, int blueIdx, const float* _coeffs, const float* _whitept, bool _srgb) : srccn(_srccn), srgb(_srgb)
	{
		volatile int _3 = 3;
		initLabTabs();

		if (!_coeffs)
			_coeffs = sRGB2XYZ_D65;
		if (!_whitept)
			_whitept = D65;

		float scale[] = { 1.0f / _whitept[0], 1.0f, 1.0f / _whitept[2] };

		for (int i = 0; i < _3; i++) {
			int j = i * 3;
			coeffs[j + (blueIdx ^ 2)] = _coeffs[j] * scale[i];
			coeffs[j + 1] = _coeffs[j + 1] * scale[i];
			coeffs[j + blueIdx] = _coeffs[j + 2] * scale[i];

			FBC_Assert(coeffs[j] >= 0 && coeffs[j + 1] >= 0 && coeffs[j + 2] >= 0 &&
				coeffs[j] + coeffs[j + 1] + coeffs[j + 2] < 1.5f*LabCbrtTabScale);
		}
	}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, scn = srccn;
		float gscale = GammaTabScale;
		const float* gammaTab = srgb ? sRGBGammaTab : 0;
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		n *= 3;

		static const float _1_3 = 1.0f / 3.0f;
		static const float _a = 16.0f / 116.0f;
		for (i = 0; i < n; i += 3, src += scn) {
			float R = clip<float>(src[0]);
			float G = clip<float>(src[1]);
			float B = clip<float>(src[2]);

			if (gammaTab) {
				R = splineInterpolate(R * gscale, gammaTab, GAMMA_TAB_SIZE);
				G = splineInterpolate(G * gscale, gammaTab, GAMMA_TAB_SIZE);
				B = splineInterpolate(B * gscale, gammaTab, GAMMA_TAB_SIZE);
			}
			float X = R*C0 + G*C1 + B*C2;
			float Y = R*C3 + G*C4 + B*C5;
			float Z = R*C6 + G*C7 + B*C8;

			float FX = X > 0.008856f ? std::pow(X, _1_3) : (7.787f * X + _a);
			float FY = Y > 0.008856f ? std::pow(Y, _1_3) : (7.787f * Y + _a);
			float FZ = Z > 0.008856f ? std::pow(Z, _1_3) : (7.787f * Z + _a);

			float L = Y > 0.008856f ? (116.f * FY - 16.f) : (903.3f * Y);
			float a = 500.f * (FX - FY);
			float b = 200.f * (FY - FZ);

			dst[i] = L;
			dst[i + 1] = a;
			dst[i + 2] = b;
		}
	}

	int srccn;
	float coeffs[9];
	bool srgb;
};

struct RGB2Luv_f
{
	typedef float channel_type;

	RGB2Luv_f(int _srccn, int blueIdx, const float* _coeffs, const float* whitept, bool _srgb) : srccn(_srccn), srgb(_srgb)
	{
		volatile int i;
		initLabTabs();

		if (!_coeffs) _coeffs = sRGB2XYZ_D65;
		if (!whitept) whitept = D65;

		for (i = 0; i < 3; i++) {
			coeffs[i * 3] = _coeffs[i * 3];
			coeffs[i * 3 + 1] = _coeffs[i * 3 + 1];
			coeffs[i * 3 + 2] = _coeffs[i * 3 + 2];
			if (blueIdx == 0)
				std::swap(coeffs[i * 3], coeffs[i * 3 + 2]);
			FBC_Assert(coeffs[i * 3] >= 0 && coeffs[i * 3 + 1] >= 0 && coeffs[i * 3 + 2] >= 0 &&
				coeffs[i * 3] + coeffs[i * 3 + 1] + coeffs[i * 3 + 2] < 1.5f);
		}

		float d = 1.f / (whitept[0] + whitept[1] * 15 + whitept[2] * 3);
		un = 4 * whitept[0] * d;
		vn = 9 * whitept[1] * d;

		FBC_Assert(whitept[1] == 1.f);
	}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, scn = srccn;
		float gscale = GammaTabScale;
		const float* gammaTab = srgb ? sRGBGammaTab : 0;
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		float _un = 13 * un, _vn = 13 * vn;
		n *= 3;

		for (i = 0; i < n; i += 3, src += scn) {
			float R = src[0], G = src[1], B = src[2];
			if (gammaTab) {
				R = splineInterpolate(R*gscale, gammaTab, GAMMA_TAB_SIZE);
				G = splineInterpolate(G*gscale, gammaTab, GAMMA_TAB_SIZE);
				B = splineInterpolate(B*gscale, gammaTab, GAMMA_TAB_SIZE);
			}

			float X = R*C0 + G*C1 + B*C2;
			float Y = R*C3 + G*C4 + B*C5;
			float Z = R*C6 + G*C7 + B*C8;

			float L = splineInterpolate(Y*LabCbrtTabScale, LabCbrtTab, LAB_CBRT_TAB_SIZE);
			L = 116.f*L - 16.f;

			float d = (4 * 13) / std::max(X + 15 * Y + 3 * Z, FLT_EPSILON);
			float u = L*(X*d - _un);
			float v = L*((9 * 0.25f)*Y*d - _vn);

			dst[i] = L; dst[i + 1] = u; dst[i + 2] = v;
		}
	}

	int srccn;
	float coeffs[9], un, vn;
	bool srgb;
};

struct RGB2Luv_b
{
	typedef uchar channel_type;

	RGB2Luv_b(int _srccn, int blueIdx, const float* _coeffs, const float* _whitept, bool _srgb)
		: srccn(_srccn), cvt(3, blueIdx, _coeffs, _whitept, _srgb) { }

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, j, scn = srccn;
		float FBC_DECL_ALIGNED(16) buf[3 * BLOCK_SIZE];

		for (i = 0; i < n; i += BLOCK_SIZE, dst += BLOCK_SIZE * 3) {
			int dn = std::min(n - i, (int)BLOCK_SIZE);
			j = 0;
			for (; j < dn * 3; j += 3, src += scn) {
				buf[j] = src[0] * (1.f / 255.f);
				buf[j + 1] = (float)(src[1] * (1.f / 255.f));
				buf[j + 2] = (float)(src[2] * (1.f / 255.f));
			}
			cvt(buf, buf, dn);

			j = 0;
			for (; j < dn * 3; j += 3) {
				dst[j] = saturate_cast<uchar>(buf[j] * 2.55f);
				dst[j + 1] = saturate_cast<uchar>(buf[j + 1] * 0.72033898305084743f + 96.525423728813564f);
				dst[j + 2] = saturate_cast<uchar>(buf[j + 2] * 0.9732824427480916f + 136.259541984732824f);
			}
		}
	}

	int srccn;
	RGB2Luv_f cvt;
};

struct Lab2RGB_f
{
	typedef float channel_type;

	Lab2RGB_f(int _dstcn, int blueIdx, const float* _coeffs, const float* _whitept, bool _srgb) : dstcn(_dstcn), srgb(_srgb)
	{
		initLabTabs();

		if (!_coeffs)
			_coeffs = XYZ2sRGB_D65;
		if (!_whitept)
			_whitept = D65;

		for (int i = 0; i < 3; i++) {
			coeffs[i + (blueIdx ^ 2) * 3] = _coeffs[i] * _whitept[i];
			coeffs[i + 3] = _coeffs[i + 3] * _whitept[i];
			coeffs[i + blueIdx * 3] = _coeffs[i + 6] * _whitept[i];
		}
	}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, dcn = dstcn;
		const float* gammaTab = srgb ? sRGBInvGammaTab : 0;
		float gscale = GammaTabScale;
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		float alpha = ColorChannel<float>::max();
		n *= 3;

		static const float lThresh = 0.008856f * 903.3f;
		static const float fThresh = 7.787f * 0.008856f + 16.0f / 116.0f;
		for (i = 0; i < n; i += 3, dst += dcn) {
			float li = src[i];
			float ai = src[i + 1];
			float bi = src[i + 2];

			float y, fy;
			if (li <= lThresh) {
				y = li / 903.3f;
				fy = 7.787f * y + 16.0f / 116.0f;
			} else {
				fy = (li + 16.0f) / 116.0f;
				y = fy * fy * fy;
			}

			float fxz[] = { ai / 500.0f + fy, fy - bi / 200.0f };

			for (int j = 0; j < 2; j++)
				if (fxz[j] <= fThresh)
					fxz[j] = (fxz[j] - 16.0f / 116.0f) / 7.787f;
				else
					fxz[j] = fxz[j] * fxz[j] * fxz[j];


			float x = fxz[0], z = fxz[1];
			float ro = C0 * x + C1 * y + C2 * z;
			float go = C3 * x + C4 * y + C5 * z;
			float bo = C6 * x + C7 * y + C8 * z;
			ro = clip(ro);
			go = clip(go);
			bo = clip(bo);

			if (gammaTab) {
				ro = splineInterpolate(ro * gscale, gammaTab, GAMMA_TAB_SIZE);
				go = splineInterpolate(go * gscale, gammaTab, GAMMA_TAB_SIZE);
				bo = splineInterpolate(bo * gscale, gammaTab, GAMMA_TAB_SIZE);
			}

			dst[0] = ro, dst[1] = go, dst[2] = bo;
			if (dcn == 4)
				dst[3] = alpha;
		}
	}

	int dstcn;
	float coeffs[9];
	bool srgb;
};

struct Lab2RGB_b
{
	typedef uchar channel_type;

	Lab2RGB_b(int _dstcn, int blueIdx, const float* _coeffs, const float* _whitept, bool _srgb)
		: dstcn(_dstcn), cvt(3, blueIdx, _coeffs, _whitept, _srgb) { }

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, j, dcn = dstcn;
		uchar alpha = ColorChannel<uchar>::max();
		float FBC_DECL_ALIGNED(16) buf[3 * BLOCK_SIZE];

		for (i = 0; i < n; i += BLOCK_SIZE, src += BLOCK_SIZE * 3)
		{
			int dn = std::min(n - i, (int)BLOCK_SIZE);
			j = 0;
			for (; j < dn * 3; j += 3) {
				buf[j] = src[j] * (100.f / 255.f);
				buf[j + 1] = (float)(src[j + 1] - 128);
				buf[j + 2] = (float)(src[j + 2] - 128);
			}
			cvt(buf, buf, dn);
			j = 0;
			for (; j < dn * 3; j += 3, dst += dcn) {
				dst[0] = saturate_cast<uchar>(buf[j] * 255.f);
				dst[1] = saturate_cast<uchar>(buf[j + 1] * 255.f);
				dst[2] = saturate_cast<uchar>(buf[j + 2] * 255.f);
				if (dcn == 4)
					dst[3] = alpha;
			}
		}
	}

	int dstcn;
	Lab2RGB_f cvt;
};

struct Luv2RGB_f
{
	typedef float channel_type;

	Luv2RGB_f(int _dstcn, int blueIdx, const float* _coeffs, const float* whitept, bool _srgb) : dstcn(_dstcn), srgb(_srgb)
	{
		initLabTabs();

		if (!_coeffs) _coeffs = XYZ2sRGB_D65;
		if (!whitept) whitept = D65;

		for (int i = 0; i < 3; i++) {
			coeffs[i + (blueIdx ^ 2) * 3] = _coeffs[i];
			coeffs[i + 3] = _coeffs[i + 3];
			coeffs[i + blueIdx * 3] = _coeffs[i + 6];
		}

		float d = 1.f / (whitept[0] + whitept[1] * 15 + whitept[2] * 3);
		un = 4 * whitept[0] * d;
		vn = 9 * whitept[1] * d;

		FBC_Assert(whitept[1] == 1.f);
	}

	void operator()(const float* src, float* dst, int n) const
	{
		int i, dcn = dstcn;
		const float* gammaTab = srgb ? sRGBInvGammaTab : 0;
		float gscale = GammaTabScale;
		float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
			C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
			C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
		float alpha = ColorChannel<float>::max();
		float _un = un, _vn = vn;
		n *= 3;

		for (i = 0; i < n; i += 3, dst += dcn) {
			float L = src[i], u = src[i + 1], v = src[i + 2], d, X, Y, Z;
			Y = (L + 16.f) * (1.f / 116.f);
			Y = Y*Y*Y;
			d = (1.f / 13.f) / L;
			u = u*d + _un;
			v = v*d + _vn;
			float iv = 1.f / v;
			X = 2.25f * u * Y * iv;
			Z = (12 - 3 * u - 20 * v) * Y * 0.25f * iv;

			float R = X*C0 + Y*C1 + Z*C2;
			float G = X*C3 + Y*C4 + Z*C5;
			float B = X*C6 + Y*C7 + Z*C8;

			R = std::min(std::max(R, 0.f), 1.f);
			G = std::min(std::max(G, 0.f), 1.f);
			B = std::min(std::max(B, 0.f), 1.f);

			if (gammaTab) {
				R = splineInterpolate(R*gscale, gammaTab, GAMMA_TAB_SIZE);
				G = splineInterpolate(G*gscale, gammaTab, GAMMA_TAB_SIZE);
				B = splineInterpolate(B*gscale, gammaTab, GAMMA_TAB_SIZE);
			}

			dst[0] = R; dst[1] = G; dst[2] = B;
			if (dcn == 4)
				dst[3] = alpha;
		}
	}

	int dstcn;
	float coeffs[9], un, vn;
	bool srgb;
};

struct Luv2RGB_b
{
	typedef uchar channel_type;

	Luv2RGB_b(int _dstcn, int blueIdx, const float* _coeffs, const float* _whitept, bool _srgb)
		: dstcn(_dstcn), cvt(3, blueIdx, _coeffs, _whitept, _srgb) { }

	void operator()(const uchar* src, uchar* dst, int n) const
	{
		int i, j, dcn = dstcn;
		uchar alpha = ColorChannel<uchar>::max();
		float FBC_DECL_ALIGNED(16) buf[3 * BLOCK_SIZE];

		for (i = 0; i < n; i += BLOCK_SIZE, src += BLOCK_SIZE * 3) {
			int dn = std::min(n - i, (int)BLOCK_SIZE);
			j = 0;
			for (; j < dn * 3; j += 3) {
				buf[j] = src[j] * (100.f / 255.f);
				buf[j + 1] = (float)(src[j + 1] * 1.388235294117647f - 134.f);
				buf[j + 2] = (float)(src[j + 2] * 1.027450980392157f - 140.f);
			}
			cvt(buf, buf, dn);

			j = 0;
			for (; j < dn * 3; j += 3, dst += dcn) {
				dst[0] = saturate_cast<uchar>(buf[j] * 255.f);
				dst[1] = saturate_cast<uchar>(buf[j + 1] * 255.f);
				dst[2] = saturate_cast<uchar>(buf[j + 2] * 255.f);
				if (dcn == 4)
					dst[3] = alpha;
			}
		}
	}

	int dstcn;
	Luv2RGB_f cvt;
};

const int ITUR_BT_601_CY = 1220542;
const int ITUR_BT_601_CUB = 2116026;
const int ITUR_BT_601_CUG = -409993;
const int ITUR_BT_601_CVG = -852492;
const int ITUR_BT_601_CVR = 1673527;
const int ITUR_BT_601_SHIFT = 20;

// Coefficients for RGB to YUV420p conversion
const int ITUR_BT_601_CRY = 269484;
const int ITUR_BT_601_CGY = 528482;
const int ITUR_BT_601_CBY = 102760;
const int ITUR_BT_601_CRU = -155188;
const int ITUR_BT_601_CGU = -305135;
const int ITUR_BT_601_CBU = 460324;
const int ITUR_BT_601_CGV = -385875;
const int ITUR_BT_601_CBV = -74448;

template<typename _Tp, int chs, int bIdx, int uIdx>
struct YUV420sp2RGB888Invoker
{
	Mat_<_Tp, chs>* dst;
	const uchar* my1, *muv;
	int width, stride;

	YUV420sp2RGB888Invoker(Mat_<_Tp, chs>* _dst, int _stride, const uchar* _y1, const uchar* _uv)
		: dst(_dst), my1(_y1), muv(_uv), width(_dst->cols), stride(_stride) {}

	void operator()(const Range& range) const
	{
		int rangeBegin = range.start * 2;
		int rangeEnd = range.end * 2;

		//R = 1.164(Y - 16) + 1.596(V - 128)
		//G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
		//B = 1.164(Y - 16)                  + 2.018(U - 128)

		//R = (1220542(Y - 16) + 1673527(V - 128)                  + (1 << 19)) >> 20
		//G = (1220542(Y - 16) - 852492(V - 128) - 409993(U - 128) + (1 << 19)) >> 20
		//B = (1220542(Y - 16)                  + 2116026(U - 128) + (1 << 19)) >> 20

		const uchar* y1 = my1 + rangeBegin * stride, *uv = muv + rangeBegin * stride / 2;

		for (int j = rangeBegin; j < rangeEnd; j += 2, y1 += stride * 2, uv += stride) {
			uchar* row1 = (uchar*)dst->ptr(j);
			uchar* row2 = (uchar*)dst->ptr(j + 1);
			const uchar* y2 = y1 + stride;

			for (int i = 0; i < width; i += 2, row1 += 6, row2 += 6) {
				int u = int(uv[i + 0 + uIdx]) - 128;
				int v = int(uv[i + 1 - uIdx]) - 128;

				int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
				int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
				int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;

				int y00 = std::max(0, int(y1[i]) - 16) * ITUR_BT_601_CY;
				row1[2 - bIdx] = saturate_cast<uchar>((y00 + ruv) >> ITUR_BT_601_SHIFT);
				row1[1] = saturate_cast<uchar>((y00 + guv) >> ITUR_BT_601_SHIFT);
				row1[bIdx] = saturate_cast<uchar>((y00 + buv) >> ITUR_BT_601_SHIFT);

				int y01 = std::max(0, int(y1[i + 1]) - 16) * ITUR_BT_601_CY;
				row1[5 - bIdx] = saturate_cast<uchar>((y01 + ruv) >> ITUR_BT_601_SHIFT);
				row1[4] = saturate_cast<uchar>((y01 + guv) >> ITUR_BT_601_SHIFT);
				row1[3 + bIdx] = saturate_cast<uchar>((y01 + buv) >> ITUR_BT_601_SHIFT);

				int y10 = std::max(0, int(y2[i]) - 16) * ITUR_BT_601_CY;
				row2[2 - bIdx] = saturate_cast<uchar>((y10 + ruv) >> ITUR_BT_601_SHIFT);
				row2[1] = saturate_cast<uchar>((y10 + guv) >> ITUR_BT_601_SHIFT);
				row2[bIdx] = saturate_cast<uchar>((y10 + buv) >> ITUR_BT_601_SHIFT);

				int y11 = std::max(0, int(y2[i + 1]) - 16) * ITUR_BT_601_CY;
				row2[5 - bIdx] = saturate_cast<uchar>((y11 + ruv) >> ITUR_BT_601_SHIFT);
				row2[4] = saturate_cast<uchar>((y11 + guv) >> ITUR_BT_601_SHIFT);
				row2[3 + bIdx] = saturate_cast<uchar>((y11 + buv) >> ITUR_BT_601_SHIFT);
			}
		}
	}
};

template<typename _Tp, int chs, int bIdx, int uIdx>
struct YUV420sp2RGBA8888Invoker
{
	Mat_<_Tp, chs>* dst;
	const uchar* my1, *muv;
	int width, stride;

	YUV420sp2RGBA8888Invoker(Mat_<_Tp, chs>* _dst, int _stride, const uchar* _y1, const uchar* _uv)
		: dst(_dst), my1(_y1), muv(_uv), width(_dst->cols), stride(_stride) {}

	void operator()(const Range& range) const
	{
		int rangeBegin = range.start * 2;
		int rangeEnd = range.end * 2;

		//R = 1.164(Y - 16) + 1.596(V - 128)
		//G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
		//B = 1.164(Y - 16)                  + 2.018(U - 128)

		//R = (1220542(Y - 16) + 1673527(V - 128)                  + (1 << 19)) >> 20
		//G = (1220542(Y - 16) - 852492(V - 128) - 409993(U - 128) + (1 << 19)) >> 20
		//B = (1220542(Y - 16)                  + 2116026(U - 128) + (1 << 19)) >> 20

		const uchar* y1 = my1 + rangeBegin * stride, *uv = muv + rangeBegin * stride / 2;

		for (int j = rangeBegin; j < rangeEnd; j += 2, y1 += stride * 2, uv += stride) {
			uchar* row1 = (uchar*)dst->ptr(j);
			uchar* row2 = (uchar*)dst->ptr(j + 1);
			const uchar* y2 = y1 + stride;

			for (int i = 0; i < width; i += 2, row1 += 8, row2 += 8) {
				int u = int(uv[i + 0 + uIdx]) - 128;
				int v = int(uv[i + 1 - uIdx]) - 128;

				int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
				int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
				int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;

				int y00 = std::max(0, int(y1[i]) - 16) * ITUR_BT_601_CY;
				row1[2 - bIdx] = saturate_cast<uchar>((y00 + ruv) >> ITUR_BT_601_SHIFT);
				row1[1] = saturate_cast<uchar>((y00 + guv) >> ITUR_BT_601_SHIFT);
				row1[bIdx] = saturate_cast<uchar>((y00 + buv) >> ITUR_BT_601_SHIFT);
				row1[3] = uchar(0xff);

				int y01 = std::max(0, int(y1[i + 1]) - 16) * ITUR_BT_601_CY;
				row1[6 - bIdx] = saturate_cast<uchar>((y01 + ruv) >> ITUR_BT_601_SHIFT);
				row1[5] = saturate_cast<uchar>((y01 + guv) >> ITUR_BT_601_SHIFT);
				row1[4 + bIdx] = saturate_cast<uchar>((y01 + buv) >> ITUR_BT_601_SHIFT);
				row1[7] = uchar(0xff);

				int y10 = std::max(0, int(y2[i]) - 16) * ITUR_BT_601_CY;
				row2[2 - bIdx] = saturate_cast<uchar>((y10 + ruv) >> ITUR_BT_601_SHIFT);
				row2[1] = saturate_cast<uchar>((y10 + guv) >> ITUR_BT_601_SHIFT);
				row2[bIdx] = saturate_cast<uchar>((y10 + buv) >> ITUR_BT_601_SHIFT);
				row2[3] = uchar(0xff);

				int y11 = std::max(0, int(y2[i + 1]) - 16) * ITUR_BT_601_CY;
				row2[6 - bIdx] = saturate_cast<uchar>((y11 + ruv) >> ITUR_BT_601_SHIFT);
				row2[5] = saturate_cast<uchar>((y11 + guv) >> ITUR_BT_601_SHIFT);
				row2[4 + bIdx] = saturate_cast<uchar>((y11 + buv) >> ITUR_BT_601_SHIFT);
				row2[7] = uchar(0xff);
			}
		}
	}
};

template<typename _Tp, int chs, int bIdx>
struct YUV420p2RGB888Invoker
{
	Mat_<_Tp, chs>* dst;
	const uchar* my1, *mu, *mv;
	int width, stride;
	int ustepIdx, vstepIdx;

	YUV420p2RGB888Invoker(Mat_<_Tp, chs>* _dst, int _stride, const uchar* _y1, const uchar* _u, const uchar* _v, int _ustepIdx, int _vstepIdx)
		: dst(_dst), my1(_y1), mu(_u), mv(_v), width(_dst->cols), stride(_stride), ustepIdx(_ustepIdx), vstepIdx(_vstepIdx) {}

	void operator()(const Range& range) const
	{
		const int rangeBegin = range.start * 2;
		const int rangeEnd = range.end * 2;

		int uvsteps[2] = { width / 2, stride - width / 2 };
		int usIdx = ustepIdx, vsIdx = vstepIdx;

		const uchar* y1 = my1 + rangeBegin * stride;
		const uchar* u1 = mu + (range.start / 2) * stride;
		const uchar* v1 = mv + (range.start / 2) * stride;

		if (range.start % 2 == 1) {
			u1 += uvsteps[(usIdx++) & 1];
			v1 += uvsteps[(vsIdx++) & 1];
		}

		for (int j = rangeBegin; j < rangeEnd; j += 2, y1 += stride * 2, u1 += uvsteps[(usIdx++) & 1], v1 += uvsteps[(vsIdx++) & 1]) {
			uchar* row1 = (uchar*)dst->ptr(j);
			uchar* row2 = (uchar*)dst->ptr(j + 1);
			const uchar* y2 = y1 + stride;

			for (int i = 0; i < width / 2; i += 1, row1 += 6, row2 += 6) {
				int u = int(u1[i]) - 128;
				int v = int(v1[i]) - 128;

				int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
				int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
				int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;

				int y00 = std::max(0, int(y1[2 * i]) - 16) * ITUR_BT_601_CY;
				row1[2 - bIdx] = saturate_cast<uchar>((y00 + ruv) >> ITUR_BT_601_SHIFT);
				row1[1] = saturate_cast<uchar>((y00 + guv) >> ITUR_BT_601_SHIFT);
				row1[bIdx] = saturate_cast<uchar>((y00 + buv) >> ITUR_BT_601_SHIFT);

				int y01 = std::max(0, int(y1[2 * i + 1]) - 16) * ITUR_BT_601_CY;
				row1[5 - bIdx] = saturate_cast<uchar>((y01 + ruv) >> ITUR_BT_601_SHIFT);
				row1[4] = saturate_cast<uchar>((y01 + guv) >> ITUR_BT_601_SHIFT);
				row1[3 + bIdx] = saturate_cast<uchar>((y01 + buv) >> ITUR_BT_601_SHIFT);

				int y10 = std::max(0, int(y2[2 * i]) - 16) * ITUR_BT_601_CY;
				row2[2 - bIdx] = saturate_cast<uchar>((y10 + ruv) >> ITUR_BT_601_SHIFT);
				row2[1] = saturate_cast<uchar>((y10 + guv) >> ITUR_BT_601_SHIFT);
				row2[bIdx] = saturate_cast<uchar>((y10 + buv) >> ITUR_BT_601_SHIFT);

				int y11 = std::max(0, int(y2[2 * i + 1]) - 16) * ITUR_BT_601_CY;
				row2[5 - bIdx] = saturate_cast<uchar>((y11 + ruv) >> ITUR_BT_601_SHIFT);
				row2[4] = saturate_cast<uchar>((y11 + guv) >> ITUR_BT_601_SHIFT);
				row2[3 + bIdx] = saturate_cast<uchar>((y11 + buv) >> ITUR_BT_601_SHIFT);
			}
		}
	}
};

template<typename _Tp, int chs, int bIdx>
struct YUV420p2RGBA8888Invoker
{
	Mat_<_Tp, chs>* dst;
	const uchar* my1, *mu, *mv;
	int width, stride;
	int ustepIdx, vstepIdx;

	YUV420p2RGBA8888Invoker(Mat_<_Tp, chs>* _dst, int _stride, const uchar* _y1, const uchar* _u, const uchar* _v, int _ustepIdx, int _vstepIdx)
		: dst(_dst), my1(_y1), mu(_u), mv(_v), width(_dst->cols), stride(_stride), ustepIdx(_ustepIdx), vstepIdx(_vstepIdx) {}

	void operator()(const Range& range) const
	{
		int rangeBegin = range.start * 2;
		int rangeEnd = range.end * 2;

		int uvsteps[2] = { width / 2, stride - width / 2 };
		int usIdx = ustepIdx, vsIdx = vstepIdx;

		const uchar* y1 = my1 + rangeBegin * stride;
		const uchar* u1 = mu + (range.start / 2) * stride;
		const uchar* v1 = mv + (range.start / 2) * stride;

		if (range.start % 2 == 1) {
			u1 += uvsteps[(usIdx++) & 1];
			v1 += uvsteps[(vsIdx++) & 1];
		}

		for (int j = rangeBegin; j < rangeEnd; j += 2, y1 += stride * 2, u1 += uvsteps[(usIdx++) & 1], v1 += uvsteps[(vsIdx++) & 1]) {
			uchar* row1 = (uchar*)dst->ptr(j);
			uchar* row2 = (uchar*)dst->ptr(j + 1);
			const uchar* y2 = y1 + stride;

			for (int i = 0; i < width / 2; i += 1, row1 += 8, row2 += 8) {
				int u = int(u1[i]) - 128;
				int v = int(v1[i]) - 128;

				int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
				int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
				int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;

				int y00 = std::max(0, int(y1[2 * i]) - 16) * ITUR_BT_601_CY;
				row1[2 - bIdx] = saturate_cast<uchar>((y00 + ruv) >> ITUR_BT_601_SHIFT);
				row1[1] = saturate_cast<uchar>((y00 + guv) >> ITUR_BT_601_SHIFT);
				row1[bIdx] = saturate_cast<uchar>((y00 + buv) >> ITUR_BT_601_SHIFT);
				row1[3] = uchar(0xff);

				int y01 = std::max(0, int(y1[2 * i + 1]) - 16) * ITUR_BT_601_CY;
				row1[6 - bIdx] = saturate_cast<uchar>((y01 + ruv) >> ITUR_BT_601_SHIFT);
				row1[5] = saturate_cast<uchar>((y01 + guv) >> ITUR_BT_601_SHIFT);
				row1[4 + bIdx] = saturate_cast<uchar>((y01 + buv) >> ITUR_BT_601_SHIFT);
				row1[7] = uchar(0xff);

				int y10 = std::max(0, int(y2[2 * i]) - 16) * ITUR_BT_601_CY;
				row2[2 - bIdx] = saturate_cast<uchar>((y10 + ruv) >> ITUR_BT_601_SHIFT);
				row2[1] = saturate_cast<uchar>((y10 + guv) >> ITUR_BT_601_SHIFT);
				row2[bIdx] = saturate_cast<uchar>((y10 + buv) >> ITUR_BT_601_SHIFT);
				row2[3] = uchar(0xff);

				int y11 = std::max(0, int(y2[2 * i + 1]) - 16) * ITUR_BT_601_CY;
				row2[6 - bIdx] = saturate_cast<uchar>((y11 + ruv) >> ITUR_BT_601_SHIFT);
				row2[5] = saturate_cast<uchar>((y11 + guv) >> ITUR_BT_601_SHIFT);
				row2[4 + bIdx] = saturate_cast<uchar>((y11 + buv) >> ITUR_BT_601_SHIFT);
				row2[7] = uchar(0xff);
			}
		}
	}
};

template<typename _Tp, int chs, int bIdx, int uIdx>
inline void cvtYUV420sp2RGB(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _uv)
{
	YUV420sp2RGB888Invoker<_Tp, chs, bIdx, uIdx> converter(&_dst, _stride, _y1, _uv);
	converter(Range(0, _dst.rows / 2));
}

template<typename _Tp, int chs, int bIdx, int uIdx>
inline void cvtYUV420sp2RGBA(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _uv)
{
	YUV420sp2RGBA8888Invoker<_Tp, chs, bIdx, uIdx> converter(&_dst, _stride, _y1, _uv);
	converter(Range(0, _dst.rows / 2));
}

template<typename _Tp, int chs, int bIdx>
inline void cvtYUV420p2RGB(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _u, const uchar* _v, int ustepIdx, int vstepIdx)
{
	YUV420p2RGB888Invoker<_Tp, chs, bIdx> converter(&_dst, _stride, _y1, _u, _v, ustepIdx, vstepIdx);
	converter(Range(0, _dst.rows / 2));
}

template<typename _Tp, int chs, int bIdx>
inline void cvtYUV420p2RGBA(Mat_<_Tp, chs>& _dst, int _stride, const uchar* _y1, const uchar* _u, const uchar* _v, int ustepIdx, int vstepIdx)
{
	YUV420p2RGBA8888Invoker<_Tp, chs, bIdx> converter(&_dst, _stride, _y1, _u, _v, ustepIdx, vstepIdx);
	converter(Range(0, _dst.rows / 2));
}

template<typename _Tp, int chs1, int chs2, int bIdx>
struct RGB888toYUV420pInvoker
{
	RGB888toYUV420pInvoker(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>* dst, const int uIdx)
		: src_(src), dst_(dst), uIdx_(uIdx) { }

	void operator()(const Range& rowRange) const
	{
		const int w = src_.cols;
		const int h = src_.rows;

		const int cn = src_.channels;
		for (int i = rowRange.start; i < rowRange.end; i++) {
			const uchar* row0 = src_.ptr(2 * i);
			const uchar* row1 = src_.ptr(2 * i + 1);

			uchar* y = (uchar*)dst_->ptr(2 * i);
			uchar* u = (uchar*)dst_->ptr(h + i / 2) + (i % 2) * (w / 2);
			uchar* v = (uchar*)dst_->ptr(h + (i + h / 2) / 2) + ((i + h / 2) % 2) * (w / 2);
			if (uIdx_ == 2) std::swap(u, v);

			for (int j = 0, k = 0; j < w * cn; j += 2 * cn, k++) {
				int r00 = row0[2 - bIdx + j];      int g00 = row0[1 + j];      int b00 = row0[bIdx + j];
				int r01 = row0[2 - bIdx + cn + j]; int g01 = row0[1 + cn + j]; int b01 = row0[bIdx + cn + j];
				int r10 = row1[2 - bIdx + j];      int g10 = row1[1 + j];      int b10 = row1[bIdx + j];
				int r11 = row1[2 - bIdx + cn + j]; int g11 = row1[1 + cn + j]; int b11 = row1[bIdx + cn + j];

				const int shifted16 = (16 << ITUR_BT_601_SHIFT);
				const int halfShift = (1 << (ITUR_BT_601_SHIFT - 1));
				int y00 = ITUR_BT_601_CRY * r00 + ITUR_BT_601_CGY * g00 + ITUR_BT_601_CBY * b00 + halfShift + shifted16;
				int y01 = ITUR_BT_601_CRY * r01 + ITUR_BT_601_CGY * g01 + ITUR_BT_601_CBY * b01 + halfShift + shifted16;
				int y10 = ITUR_BT_601_CRY * r10 + ITUR_BT_601_CGY * g10 + ITUR_BT_601_CBY * b10 + halfShift + shifted16;
				int y11 = ITUR_BT_601_CRY * r11 + ITUR_BT_601_CGY * g11 + ITUR_BT_601_CBY * b11 + halfShift + shifted16;

				y[2 * k + 0] = saturate_cast<uchar>(y00 >> ITUR_BT_601_SHIFT);
				y[2 * k + 1] = saturate_cast<uchar>(y01 >> ITUR_BT_601_SHIFT);
				y[2 * k + dst_->step + 0] = saturate_cast<uchar>(y10 >> ITUR_BT_601_SHIFT);
				y[2 * k + dst_->step + 1] = saturate_cast<uchar>(y11 >> ITUR_BT_601_SHIFT);

				const int shifted128 = (128 << ITUR_BT_601_SHIFT);
				int u00 = ITUR_BT_601_CRU * r00 + ITUR_BT_601_CGU * g00 + ITUR_BT_601_CBU * b00 + halfShift + shifted128;
				int v00 = ITUR_BT_601_CBU * r00 + ITUR_BT_601_CGV * g00 + ITUR_BT_601_CBV * b00 + halfShift + shifted128;

				u[k] = saturate_cast<uchar>(u00 >> ITUR_BT_601_SHIFT);
				v[k] = saturate_cast<uchar>(v00 >> ITUR_BT_601_SHIFT);
			}
		}
	}

private:
	RGB888toYUV420pInvoker& operator=(const RGB888toYUV420pInvoker&);

	const Mat_<_Tp, chs1>& src_;
	Mat_<_Tp, chs2>* const dst_;
	const int uIdx_;
};

template<typename _Tp, int chs1, int chs2, int bIdx, int uIdx>
static void cvtRGBtoYUV420p(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst)
{
	RGB888toYUV420pInvoker<_Tp, chs1, chs2, bIdx> colorConverter(src, &dst, uIdx);
	colorConverter(Range(0, src.rows / 2));
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx)
{
	Range range(0, src.rows);

	const uchar* yS_ = src.ptr(range.start);
	uchar* yD_ = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	for (int h = range.start; h < range.end; ++h, yS_ += src.step, yD_ += dst.step) {
		int n = src.cols;
		const _Tp* yS = (const _Tp*)yS_;
		_Tp* yD = (_Tp*)yD_;

		if (dcn == 3) {
			n *= 3;
			for (int i = 0; i < n; i += 3, yS += scn) {
				_Tp t0 = yS[bidx], t1 = yS[1], t2 = yS[bidx ^ 2];
				yD[i] = t0; yD[i + 1] = t1; yD[i + 2] = t2;
			}
		} else if (scn == 3) {
			n *= 3;
			_Tp alpha = ColorChannel<_Tp>::max(); // Note: _Tp = float: alpha = 1.0f
			for (int i = 0; i < n; i += 3, yD += 4) {
				_Tp t0 = yS[i], t1 = yS[i + 1], t2 = yS[i + 2];
				yD[bidx] = t0; yD[1] = t1; yD[bidx ^ 2] = t2; yD[3] = alpha;
			}
		} else {
			n *= 4;
			for (int i = 0; i < n; i += 4) {
				_Tp t0 = yS[i], t1 = yS[i + 1], t2 = yS[i + 2], t3 = yS[i + 3];
				yD[i] = t2; yD[i + 1] = t1; yD[i + 2] = t0; yD[i + 3] = t3;
			}
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2Gray(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	RGB2Gray<_Tp> rgb2gray(scn, bidx, 0);

	for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
		rgb2gray((const _Tp*)yS, (_Tp*)yD, src.cols);
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_Gray2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	Gray2RGB<_Tp> gray2rgb(dcn);

	for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
		gray2rgb((const _Tp*)yS, (_Tp*)yD, src.cols);
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2YCrCb(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs_f, const int* coeffs_i)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 4) {
		RGB2YCrCb_f<_Tp> rgb2ycrcb(scn, bidx, coeffs_f);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2ycrcb((const _Tp*)yS, (_Tp*)yD, src.cols);
		}
	} else {
		if (sizeof(_Tp) == 1) {
			RGB2YCrCb_i<uchar> rgb2ycrcb(scn, bidx, coeffs_i);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				rgb2ycrcb((const uchar*)yS, (uchar*)yD, src.cols);
			}
		} else {
			RGB2YCrCb_i<ushort> rgb2ycrcb(scn, bidx, coeffs_i);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				rgb2ycrcb((const ushort*)yS, (ushort*)yD, src.cols);
			}
		}

	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_YCrCb2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs_f, const int* coeffs_i)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 4) {
		YCrCb2RGB_f<_Tp> ycrcb2rgb(dcn, bidx, coeffs_f);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			ycrcb2rgb((const _Tp*)yS, (_Tp*)yD, src.cols);
		}
	} else {
		if (sizeof(_Tp) == 1) {
			YCrCb2RGB_i<uchar> ycrcb2rgb(dcn, bidx, coeffs_i);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				ycrcb2rgb((const uchar*)yS, (uchar*)yD, src.cols);
			}
		}
		else {
			YCrCb2RGB_i<ushort> ycrcb2rgb(dcn, bidx, coeffs_i);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				ycrcb2rgb((const ushort*)yS, (ushort*)yD, src.cols);
			}
		}

	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2XYZ(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 4) {
		RGB2XYZ_f<_Tp> rgb2xyz(scn, bidx, 0);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2xyz((const _Tp*)yS, (_Tp*)yD, src.cols);
		}
	} else {
		if (sizeof(_Tp) == 1) {
			RGB2XYZ_i<uchar> rgb2xyz(scn, bidx, 0);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				rgb2xyz((const uchar*)yS, (uchar*)yD, src.cols);
			}
		} else {
			RGB2XYZ_i<ushort> rgb2xyz(scn, bidx, 0);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				rgb2xyz((const ushort*)yS, (ushort*)yD, src.cols);
			}
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_XYZ2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 4) {
		XYZ2RGB_f<_Tp> xyz2rgb(dcn, bidx, 0);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			xyz2rgb((const _Tp*)yS, (_Tp*)yD, src.cols);
		}
	} else {
		if (sizeof(_Tp) == 1) {
			XYZ2RGB_i<uchar> xyz2rgb(dcn, bidx, 0);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				xyz2rgb((const uchar*)yS, (uchar*)yD, src.cols);
			}
		} else {
			XYZ2RGB_i<ushort> xyz2rgb(dcn, bidx, 0);

			for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
				xyz2rgb((const ushort*)yS, (ushort*)yD, src.cols);
			}
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2HSV(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		RGB2HSV_b rgb2hsv(scn, bidx, hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2hsv((const uchar*)yS, (uchar*)yD, src.cols);
		}
	} else {
		RGB2HSV_f rgb2hsv(scn, bidx, (float)hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2hsv((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2HLS(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		RGB2HLS_b rgb2hls(scn, bidx, hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2hls((const uchar*)yS, (uchar*)yD, src.cols);
		}
	} else {
		RGB2HLS_f rgb2hls(scn, bidx, (float)hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2hls((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_HSV2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		HSV2RGB_b hsv2rgb(dcn, bidx, hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			hsv2rgb((const uchar*)yS, (uchar*)yD, src.cols);
		}
	} else {
		HSV2RGB_f hsv2rgb(dcn, bidx, (float)hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			hsv2rgb((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_HLS2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, int hrange)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		HLS2RGB_b hls2rgb(dcn, bidx, hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			hls2rgb((const uchar*)yS, (uchar*)yD, src.cols);
		}
	} else {
		HLS2RGB_f hls2rgb(dcn, bidx, (float)hrange);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			hls2rgb((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2Lab(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		RGB2Lab_b rgb2lab(scn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2lab((const uchar*)yS, (uchar*)yD, src.cols);
		}
	}
	else {
		RGB2Lab_f rgb2lab(scn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2lab((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_RGB2Luv(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		RGB2Luv_b rgb2luv(scn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2luv((const uchar*)yS, (uchar*)yD, src.cols);
		}
	}
	else {
		RGB2Luv_f rgb2luv(scn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			rgb2luv((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_Lab2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		Lab2RGB_b lab2rgb(dcn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			lab2rgb((const uchar*)yS, (uchar*)yD, src.cols);
		}
	}
	else {
		Lab2RGB_f lab2rgb(dcn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			lab2rgb((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}

template<typename _Tp, int chs1, int chs2>
static int CvtColorLoop_Luv2RGB(const Mat_<_Tp, chs1>& src, Mat_<_Tp, chs2>& dst, int bidx, const float* coeffs, const float* whitept, bool srgb)
{
	Range range(0, src.rows);
	const uchar* yS = src.ptr(range.start);
	uchar* yD = (uchar*)dst.ptr(range.start);
	int scn = src.channels, dcn = dst.channels;

	if (sizeof(_Tp) == 1) {
		Luv2RGB_b luv2rgb(dcn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			luv2rgb((const uchar*)yS, (uchar*)yD, src.cols);
		}
	}
	else {
		Luv2RGB_f luv2rgb(dcn, bidx, coeffs, whitept, srgb);

		for (int i = range.start; i < range.end; ++i, yS += src.step, yD += dst.step) {
			luv2rgb((const float*)yS, (float*)yD, src.cols);
		}
	}

	return 0;
}


} // namespace fbc
#endif // FBC_CV_CVTCOLOR_HPP_
