// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_IMGPROC_HPP_
#define FBC_CV_IMGPROC_HPP_

// reference: include/opencv2/imgproc.hpp

#include "core/fbcdef.hpp"
#include "core/interface.hpp"
#include "core/mat.hpp"
#include "core/core.hpp"

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
	/** mask for interpolation codes */
	INTER_MAX = 7,
	/** flag, fills all of the destination image pixels. If some of them correspond to outliers in the
	source image, they are set to zero */
	WARP_FILL_OUTLIERS = 8,
	/** flag, inverse transformation

	For example, polar transforms:
	- flag is __not__ set: \f$dst( \phi , \rho ) = src(x,y)\f$
	- flag is set: \f$dst(x,y) = src( \phi , \rho )\f$
	*/
	WARP_INVERSE_MAP = 16
};

enum InterpolationMasks {
	INTER_BITS = 5,
	INTER_BITS2 = INTER_BITS * 2,
	INTER_TAB_SIZE = 1 << INTER_BITS,
	INTER_TAB_SIZE2 = INTER_TAB_SIZE * INTER_TAB_SIZE
};

// Constants for color conversion
enum ColorConversionFlags {
	CV_BGR2BGRA = 0,
	CV_RGB2RGBA = CV_BGR2BGRA,

	CV_BGRA2BGR = 1,
	CV_RGBA2RGB = CV_BGRA2BGR,

	CV_BGR2RGBA = 2,
	CV_RGB2BGRA = CV_BGR2RGBA,

	CV_RGBA2BGR = 3,
	CV_BGRA2RGB = CV_RGBA2BGR,

	CV_BGR2RGB = 4,
	CV_RGB2BGR = CV_BGR2RGB,

	CV_BGRA2RGBA = 5,
	CV_RGBA2BGRA = CV_BGRA2RGBA,

	CV_BGR2GRAY = 6,
	CV_RGB2GRAY = 7,
	CV_GRAY2BGR = 8,
	CV_GRAY2RGB = CV_GRAY2BGR,
	CV_GRAY2BGRA = 9,
	CV_GRAY2RGBA = CV_GRAY2BGRA,
	CV_BGRA2GRAY = 10,
	CV_RGBA2GRAY = 11,

	// CV_BGR2BGR565 // 12 - 31

	CV_BGR2XYZ = 32,
	CV_RGB2XYZ = 33,
	CV_XYZ2BGR = 34,
	CV_XYZ2RGB = 35,

	CV_BGR2YCrCb = 36,
	CV_RGB2YCrCb = 37,
	CV_YCrCb2BGR = 38,
	CV_YCrCb2RGB = 39,

	CV_BGR2HSV = 40,
	CV_RGB2HSV = 41,

	CV_BGR2Lab = 44,
	CV_RGB2Lab = 45,

	// CV_BayerBG2BGR // 46 - 49

	CV_BGR2Luv = 50,
	CV_RGB2Luv = 51,
	CV_BGR2HLS = 52,
	CV_RGB2HLS = 53,

	CV_HSV2BGR = 54,
	CV_HSV2RGB = 55,

	CV_Lab2BGR = 56,
	CV_Lab2RGB = 57,
	CV_Luv2BGR = 58,
	CV_Luv2RGB = 59,
	CV_HLS2BGR = 60,
	CV_HLS2RGB = 61,

	// CV_BayerBG2BGR_VNG // 62 - 65

	CV_BGR2HSV_FULL = 66,
	CV_RGB2HSV_FULL = 67,
	CV_BGR2HLS_FULL = 68,
	CV_RGB2HLS_FULL = 69,

	CV_HSV2BGR_FULL = 70,
	CV_HSV2RGB_FULL = 71,
	CV_HLS2BGR_FULL = 72,
	CV_HLS2RGB_FULL = 73,

	// CV_LBGR2Lab // 74 - 81

	CV_BGR2YUV = 82,
	CV_RGB2YUV = 83,
	CV_YUV2BGR = 84,
	CV_YUV2RGB = 85,

	// CV_BayerBG2GRAY // 86 - 89

	//YUV 4:2:0 formats family
	CV_YUV2RGB_NV12 = 90,
	CV_YUV2BGR_NV12 = 91,
	CV_YUV2RGB_NV21 = 92,
	CV_YUV2BGR_NV21 = 93,
	CV_YUV420sp2RGB = CV_YUV2RGB_NV21,
	CV_YUV420sp2BGR = CV_YUV2BGR_NV21,

	CV_YUV2RGBA_NV12 = 94,
	CV_YUV2BGRA_NV12 = 95,
	CV_YUV2RGBA_NV21 = 96,
	CV_YUV2BGRA_NV21 = 97,
	CV_YUV420sp2RGBA = CV_YUV2RGBA_NV21,
	CV_YUV420sp2BGRA = CV_YUV2BGRA_NV21,

	CV_YUV2RGB_YV12 = 98,
	CV_YUV2BGR_YV12 = 99,
	CV_YUV2RGB_IYUV = 100,
	CV_YUV2BGR_IYUV = 101,
	CV_YUV2RGB_I420 = CV_YUV2RGB_IYUV,
	CV_YUV2BGR_I420 = CV_YUV2BGR_IYUV,
	CV_YUV420p2RGB = CV_YUV2RGB_YV12,
	CV_YUV420p2BGR = CV_YUV2BGR_YV12,

	CV_YUV2RGBA_YV12 = 102,
	CV_YUV2BGRA_YV12 = 103,
	CV_YUV2RGBA_IYUV = 104,
	CV_YUV2BGRA_IYUV = 105,
	CV_YUV2RGBA_I420 = CV_YUV2RGBA_IYUV,
	CV_YUV2BGRA_I420 = CV_YUV2BGRA_IYUV,
	CV_YUV420p2RGBA = CV_YUV2RGBA_YV12,
	CV_YUV420p2BGRA = CV_YUV2BGRA_YV12,

	CV_YUV2GRAY_420 = 106,
	CV_YUV2GRAY_NV21 = CV_YUV2GRAY_420,
	CV_YUV2GRAY_NV12 = CV_YUV2GRAY_420,
	CV_YUV2GRAY_YV12 = CV_YUV2GRAY_420,
	CV_YUV2GRAY_IYUV = CV_YUV2GRAY_420,
	CV_YUV2GRAY_I420 = CV_YUV2GRAY_420,
	CV_YUV420sp2GRAY = CV_YUV2GRAY_420,
	CV_YUV420p2GRAY = CV_YUV2GRAY_420,

	// YUV 4:2:2 formats family // 107 - 124

	// alpha premultiplication // 125 - 126

	CV_RGB2YUV_I420 = 127,
	CV_BGR2YUV_I420 = 128,
	CV_RGB2YUV_IYUV = CV_RGB2YUV_I420,
	CV_BGR2YUV_IYUV = CV_BGR2YUV_I420,

	CV_RGBA2YUV_I420 = 129,
	CV_BGRA2YUV_I420 = 130,
	CV_RGBA2YUV_IYUV = CV_RGBA2YUV_I420,
	CV_BGRA2YUV_IYUV = CV_BGRA2YUV_I420,
	CV_RGB2YUV_YV12 = 131,
	CV_BGR2YUV_YV12 = 132,
	CV_RGBA2YUV_YV12 = 133,
	CV_BGRA2YUV_YV12 = 134

	// Edge-Aware Demosaicing // 135 - 139
};

// type of morphological operation
enum MorphTypes{
	MORPH_ERODE = 0, // see cv::erode
	MORPH_DILATE = 1, // see cv::dilate
	MORPH_OPEN = 2, // an opening operation
	// \f[\texttt{dst} = \mathrm{open} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \mathrm{erode} ( \texttt{src} , \texttt{element} ))\f]
	MORPH_CLOSE = 3, // a closing operation
	// \f[\texttt{dst} = \mathrm{close} ( \texttt{src} , \texttt{element} )= \mathrm{erode} ( \mathrm{dilate} ( \texttt{src} , \texttt{element} ))\f]
	MORPH_GRADIENT = 4, // a morphological gradient
	// \f[\texttt{dst} = \mathrm{morph\_grad} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \texttt{src} , \texttt{element} )- \mathrm{erode} ( \texttt{src} , \texttt{element} )\f]
	MORPH_TOPHAT = 5, // "top hat"
	// \f[\texttt{dst} = \mathrm{tophat} ( \texttt{src} , \texttt{element} )= \texttt{src} - \mathrm{open} ( \texttt{src} , \texttt{element} )\f]
	MORPH_BLACKHAT = 6, // "black hat"
	// \f[\texttt{dst} = \mathrm{blackhat} ( \texttt{src} , \texttt{element} )= \mathrm{close} ( \texttt{src} , \texttt{element} )- \texttt{src}\f]
	MORPH_HITMISS = 7  // "hit and miss"
	// Only supported for CV_8UC1 binary images. Tutorial can be found in [this page](http://opencv-code.com/tutorials/hit-or-miss-transform-in-opencv/)
};

// shape of the structuring element
enum MorphShapes {
	MORPH_RECT = 0, // a rectangular structuring element:  \f[E_{ij}=1\f]
	MORPH_CROSS = 1, // a cross-shaped structuring element:
	//!< \f[E_{ij} =  \fork{1}{if i=\texttt{anchor.y} or j=\texttt{anchor.x}}{0}{otherwise}\f]
	MORPH_ELLIPSE = 2 // an elliptic structuring element, that is, a filled ellipse inscribed
	// into the rectangle Rect(0, 0, esize.width, 0.esize.height)
};

// type of the threshold operation
enum ThresholdTypes {
	THRESH_BINARY = 0, // \f[\texttt{dst} (x,y) =  \fork{\texttt{maxval}}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{0}{otherwise}\f]
	THRESH_BINARY_INV = 1, // \f[\texttt{dst} (x,y) =  \fork{0}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{maxval}}{otherwise}\f]
	THRESH_TRUNC = 2, // \f[\texttt{dst} (x,y) =  \fork{\texttt{threshold}}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{src}(x,y)}{otherwise}\f]
	THRESH_TOZERO = 3, // \f[\texttt{dst} (x,y) =  \fork{\texttt{src}(x,y)}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{0}{otherwise}\f]
	THRESH_TOZERO_INV = 4, // \f[\texttt{dst} (x,y) =  \fork{0}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{src}(x,y)}{otherwise}\f]
	THRESH_MASK = 7,
	THRESH_OTSU = 8, // flag, use Otsu algorithm to choose the optimal threshold value, combine the flag with one of the above THRESH_* values
	THRESH_TRIANGLE = 16 // flag, use Triangle algorithm to choose the optimal threshold value, combine the flag with one of the above THRESH_* values, but not with THRESH_OTSU
};

// adaptive threshold algorithm
enum AdaptiveThresholdTypes {
	/** the threshold value \f$T(x,y)\f$ is a mean of the \f$\texttt{blockSize} \times
	\texttt{blockSize}\f$ neighborhood of \f$(x, y)\f$ minus C */
	ADAPTIVE_THRESH_MEAN_C = 0,
	/** the threshold value \f$T(x, y)\f$ is a weighted sum (cross-correlation with a Gaussian
	window) of the \f$\texttt{blockSize} \times \texttt{blockSize}\f$ neighborhood of \f$(x, y)\f$
	minus C . The default sigma (standard deviation) is used for the specified blockSize . See cv::getGaussianKernel*/
	ADAPTIVE_THRESH_GAUSSIAN_C = 1
};

// helper tables
const uchar icvSaturate8u_cv[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255
};

#define FBC_FAST_CAST_8U(t)  (assert(-256 <= (t) && (t) <= 512), icvSaturate8u_cv[(t)+256])
#define FBC_CALC_MIN_8U(a,b) (a) -= FBC_FAST_CAST_8U((a) - (b))
#define FBC_CALC_MAX_8U(a,b) (a) += FBC_FAST_CAST_8U((b) - (a))

// cal a structuring element of the specified size and shape for morphological operations
FBC_EXPORTS int getStructuringElement(Mat_<uchar, 1>& dst, int shape, Size ksize, Point anchor = Point(-1, -1));

// Returns the optimal DFT size for a given vector size
// Arrays whose size is a power-of-two (2, 4, 8, 16, 32, ...) are the fastest to process.
// Though, the arrays whose size is a product of 2's, 3's, and 5's are also processed quite efficiently
FBC_EXPORTS int getOptimalDFTSize(int vecsize);

static void copyMakeBorder_8u(const uchar* src, size_t srcstep, Size srcroi, uchar* dst, size_t dststep, Size dstroi, int top, int left, int cn, int borderType)
{
	const int isz = (int)sizeof(int);
	int i, j, k, elemSize = 1;
	bool intMode = false;

	if ((cn | srcstep | dststep | (size_t)src | (size_t)dst) % isz == 0) {
		cn /= isz;
		elemSize = isz;
		intMode = true;
	}

	AutoBuffer<int> _tab((dstroi.width - srcroi.width)*cn);
	int* tab = _tab;
	int right = dstroi.width - srcroi.width - left;
	int bottom = dstroi.height - srcroi.height - top;

	for (i = 0; i < left; i++) {
		j = borderInterpolate<int>(i - left, srcroi.width, borderType)*cn;
		for (k = 0; k < cn; k++)
			tab[i*cn + k] = j + k;
	}

	for (i = 0; i < right; i++)
	{
		j = borderInterpolate<int>(srcroi.width + i, srcroi.width, borderType)*cn;
		for (k = 0; k < cn; k++)
			tab[(i + left)*cn + k] = j + k;
	}

	srcroi.width *= cn;
	dstroi.width *= cn;
	left *= cn;
	right *= cn;

	uchar* dstInner = dst + dststep*top + left*elemSize;

	for (i = 0; i < srcroi.height; i++, dstInner += dststep, src += srcstep) {
		if (dstInner != src)
			memcpy(dstInner, src, srcroi.width*elemSize);

		if (intMode) {
			const int* isrc = (int*)src;
			int* idstInner = (int*)dstInner;
			for (j = 0; j < left; j++)
				idstInner[j - left] = isrc[tab[j]];
			for (j = 0; j < right; j++)
				idstInner[j + srcroi.width] = isrc[tab[j + left]];
		} else {
			for (j = 0; j < left; j++)
				dstInner[j - left] = src[tab[j]];
			for (j = 0; j < right; j++)
				dstInner[j + srcroi.width] = src[tab[j + left]];
		}
	}

	dstroi.width *= elemSize;
	dst += dststep*top;

	for (i = 0; i < top; i++) {
		j = borderInterpolate<int>(i - top, srcroi.height, borderType);
		memcpy(dst + (i - top)*dststep, dst + j*dststep, dstroi.width);
	}

	for (i = 0; i < bottom; i++) {
		j = borderInterpolate<int>(i + srcroi.height, srcroi.height, borderType);
		memcpy(dst + (i + srcroi.height)*dststep, dst + j*dststep, dstroi.width);
	}
}

static void copyMakeConstBorder_8u(const uchar* src, size_t srcstep, Size srcroi, uchar* dst, size_t dststep, Size dstroi, int top, int left, int cn, const uchar* value)
{
	int i, j;
	AutoBuffer<uchar> _constBuf(dstroi.width*cn);
	uchar* constBuf = _constBuf;
	int right = dstroi.width - srcroi.width - left;
	int bottom = dstroi.height - srcroi.height - top;

	for (i = 0; i < dstroi.width; i++) {
		for (j = 0; j < cn; j++)
			constBuf[i*cn + j] = value[j];
	}

	srcroi.width *= cn;
	dstroi.width *= cn;
	left *= cn;
	right *= cn;

	uchar* dstInner = dst + dststep*top + left;

	for (i = 0; i < srcroi.height; i++, dstInner += dststep, src += srcstep) {
		if (dstInner != src)
			memcpy(dstInner, src, srcroi.width);
		memcpy(dstInner - left, constBuf, left);
		memcpy(dstInner + srcroi.width, constBuf, right);
	}

	dst += dststep*top;

	for (i = 0; i < top; i++)
		memcpy(dst + (i - top)*dststep, constBuf, dstroi.width);

	for (i = 0; i < bottom; i++)
		memcpy(dst + (i + srcroi.height)*dststep, constBuf, dstroi.width);
}

// Forms a border around an image
// Note: if src is a part(ROI) of a bigger image, src'size will be change
template<typename _Tp, int chs>
int copyMakeBorder(/*const*/ Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, int top, int bottom, int left, int right, int borderType, const Scalar& value = Scalar())
{
	FBC_Assert(top >= 0 && bottom >= 0 && left >= 0 && right >= 0);

	if (src.isSubmatrix() && (borderType & BORDER_ISOLATED) == 0) {
		Size wholeSize;
		Point ofs;
		src.locateROI(wholeSize, ofs);
		int dtop = std::min(ofs.y, top);
		int dbottom = std::min(wholeSize.height - src.rows - ofs.y, bottom);
		int dleft = std::min(ofs.x, left);
		int dright = std::min(wholeSize.width - src.cols - ofs.x, right);
		src.adjustROI(dtop, dbottom, dleft, dright);
		top -= dtop;
		left -= dleft;
		bottom -= dbottom;
		right -= dright;
	}

	if (dst.empty() || dst.rows != (src.rows + top + bottom) || dst.cols != (src.cols + left + right)) {
		dst.release();
		dst = Mat_<_Tp, chs>(src.rows + top + bottom, src.cols + left + right);
	}

	if (top == 0 && left == 0 && bottom == 0 && right == 0) {
		if (src.data != dst.data || src.step != dst.step)
			src.copyTo(dst);
		return 0;
	}

	borderType &= ~BORDER_ISOLATED;

	if (borderType != BORDER_CONSTANT) {
		copyMakeBorder_8u(src.ptr(), src.step, src.size(), dst.ptr(), dst.step, dst.size(), top, left, src.elemSize(), borderType);
	} else {
		int cn = src.channels, cn1 = cn;
		AutoBuffer<double> buf(cn);

		scalarToRawData<_Tp, chs>(value, buf, cn);
		copyMakeConstBorder_8u(src.ptr(), src.step, src.size(), dst.ptr(), dst.step, dst.size(), top, left, (int)src.elemSize(), (uchar*)(double*)buf);
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_IMGPROC_HPP_