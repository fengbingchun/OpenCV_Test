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

template<typename _Tp, int chs> static double getThreshVal_Otsu_8u(const Mat_<_Tp, chs>& src);
template<typename _Tp, int chs> static double getThreshVal_Triangle_8u(const Mat_<_Tp, chs>& src);
template<typename _Tp, int chs> static void thresh_8u(const Mat_<_Tp, chs>& _src, Mat_<_Tp, chs>& _dst, uchar thresh, uchar maxval, int type);
template<typename _Tp, int chs> static void thresh_32f(const Mat_<_Tp, chs>& _src, Mat_<_Tp, chs>& _dst, float thresh, float maxval, int type);

// applies fixed-level thresholding to a single-channel array
// the Otsu's and Triangle methods are implemented only for 8-bit images
// support type: uchar/float, single-channel
template<typename _Tp, int chs>
double threshold(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, double thresh, double maxval, int type)
{
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	if (dst.empty()) {
		dst = Mat_<_Tp, chs>(src.rows, src.cols);
	} else {
		FBC_Assert(src.rows == dst.rows && src.cols == dst.cols);
	}

	int automatic_thresh = (type & ~THRESH_MASK);
	type &= THRESH_MASK;

	FBC_Assert(automatic_thresh != (THRESH_OTSU | THRESH_TRIANGLE));
	if (automatic_thresh == THRESH_OTSU) {
		FBC_Assert(sizeof(_Tp) == 1);
		thresh = getThreshVal_Otsu_8u(src);
	} else if (automatic_thresh == THRESH_TRIANGLE) {
		FBC_Assert(sizeof(_Tp) == 1);
		thresh = getThreshVal_Triangle_8u(src);
	}

	if (sizeof(_Tp) == 1) {
		int ithresh = fbcFloor(thresh);
		thresh = ithresh;
		int imaxval = fbcRound(maxval);
		if (type == THRESH_TRUNC)
			imaxval = ithresh;
		imaxval = saturate_cast<uchar>(imaxval);

		if (ithresh < 0 || ithresh >= 255) {
			if (type == THRESH_BINARY || type == THRESH_BINARY_INV ||
				((type == THRESH_TRUNC || type == THRESH_TOZERO_INV) && ithresh < 0) ||
				(type == THRESH_TOZERO && ithresh >= 255)) {
				int v = type == THRESH_BINARY ? (ithresh >= 255 ? 0 : imaxval) :
					type == THRESH_BINARY_INV ? (ithresh >= 255 ? imaxval : 0) :
					/*type == THRESH_TRUNC ? imaxval :*/ 0;
				dst.setTo(v);
			}
			else
				src.copyTo(dst);
			return thresh;
		}
		thresh = ithresh;
		maxval = imaxval;
	} else if (sizeof(_Tp) == 4) {
	} else {
		FBC_Error("UnsupportedFormat");
	}

	if (sizeof(_Tp) == 1) {
		thresh_8u(src, dst, (uchar)thresh, (uchar)maxval, type);
	} else {
		thresh_32f(src, dst, (float)thresh, (float)maxval, type);
	}

	return 0;
}

template<typename _Tp, int chs>
static double getThreshVal_Otsu_8u(const Mat_<_Tp, chs>& _src)
{
	Size size = _src.size();
	const int N = 256;
	int i, j, h[N] = { 0 };

	for (i = 0; i < size.height; i++) {
		const uchar* src = _src.ptr(i);
		j = 0;
		for (; j <= size.width - 4; j += 4) {
			int v0 = src[j], v1 = src[j + 1];
			h[v0]++; h[v1]++;
			v0 = src[j + 2]; v1 = src[j + 3];
			h[v0]++; h[v1]++;
		}
		for (; j < size.width; j++)
			h[src[j]]++;
	}

	double mu = 0, scale = 1. / (size.width*size.height);
	for (i = 0; i < N; i++)
		mu += i*(double)h[i];

	mu *= scale;
	double mu1 = 0, q1 = 0;
	double max_sigma = 0, max_val = 0;

	for (i = 0; i < N; i++) {
		double p_i, q2, mu2, sigma;

		p_i = h[i] * scale;
		mu1 *= q1;
		q1 += p_i;
		q2 = 1. - q1;

		if (std::min(q1, q2) < FLT_EPSILON || std::max(q1, q2) > 1. - FLT_EPSILON)
			continue;

		mu1 = (mu1 + i*p_i) / q1;
		mu2 = (mu - q1*mu1) / q2;
		sigma = q1*q2*(mu1 - mu2)*(mu1 - mu2);
		if (sigma > max_sigma) {
			max_sigma = sigma;
			max_val = i;
		}
	}

	return max_val;
}

template<typename _Tp, int chs>
static double getThreshVal_Triangle_8u(const Mat_<_Tp, chs>& _src)
{
	Size size = _src.size();
	const int N = 256;
	int i, j, h[N] = { 0 };

	for (i = 0; i < size.height; i++) {
		const uchar* src = _src.ptr(i);
		j = 0;
		for (; j <= size.width - 4; j += 4) {
			int v0 = src[j], v1 = src[j + 1];
			h[v0]++; h[v1]++;
			v0 = src[j + 2]; v1 = src[j + 3];
			h[v0]++; h[v1]++;
		}

		for (; j < size.width; j++)
			h[src[j]]++;
	}

	int left_bound = 0, right_bound = 0, max_ind = 0, max = 0;
	int temp;
	bool isflipped = false;

	for (i = 0; i < N; i++) {
		if (h[i] > 0) {
			left_bound = i;
			break;
		}
	}
	if (left_bound > 0)
		left_bound--;

	for (i = N - 1; i > 0; i--) {
		if (h[i] > 0) {
			right_bound = i;
			break;
		}
	}
	if (right_bound < N - 1)
		right_bound++;

	for (i = 0; i < N; i++) {
		if (h[i] > max) {
			max = h[i];
			max_ind = i;
		}
	}

	if (max_ind - left_bound < right_bound - max_ind) {
		isflipped = true;
		i = 0, j = N - 1;
		while (i < j) {
			temp = h[i]; h[i] = h[j]; h[j] = temp;
			i++; j--;
		}
		left_bound = N - 1 - right_bound;
		max_ind = N - 1 - max_ind;
	}

	double thresh = left_bound;
	double a, b, dist = 0, tempdist;

	// We do not need to compute precise distance here. Distance is maximized, so some constants can
	// be omitted. This speeds up a computation a bit.
	a = max; b = left_bound - max_ind;
	for (i = left_bound + 1; i <= max_ind; i++) {
		tempdist = a*i + b*h[i];
		if (tempdist > dist) {
			dist = tempdist;
			thresh = i;
		}
	}
	thresh--;

	if (isflipped)
		thresh = N - 1 - thresh;

	return thresh;
}

template<typename _Tp, int chs>
static void thresh_8u(const Mat_<_Tp, chs>& _src, Mat_<_Tp, chs>& _dst, uchar thresh, uchar maxval, int type)
{
	int i, j, j_scalar = 0;
	uchar tab[256];
	Size roi = _src.size();
	roi.width *= _src.channels;

	switch (type) {
	case THRESH_BINARY:
		for (i = 0; i <= thresh; i++)
			tab[i] = 0;
		for (; i < 256; i++)
			tab[i] = maxval;
		break;
	case THRESH_BINARY_INV:
		for (i = 0; i <= thresh; i++)
			tab[i] = maxval;
		for (; i < 256; i++)
			tab[i] = 0;
		break;
	case THRESH_TRUNC:
		for (i = 0; i <= thresh; i++)
			tab[i] = (uchar)i;
		for (; i < 256; i++)
			tab[i] = thresh;
		break;
	case THRESH_TOZERO:
		for (i = 0; i <= thresh; i++)
			tab[i] = 0;
		for (; i < 256; i++)
			tab[i] = (uchar)i;
		break;
	case THRESH_TOZERO_INV:
		for (i = 0; i <= thresh; i++)
			tab[i] = (uchar)i;
		for (; i < 256; i++)
			tab[i] = 0;
		break;
	default:
		FBC_Error("Unknown threshold type");
	}

	if (j_scalar < roi.width) {
		for (i = 0; i < roi.height; i++) {
			const uchar* src = _src.ptr(i);
			uchar* dst = _dst.ptr(i);
			j = j_scalar;

			for (; j <= roi.width - 4; j += 4) {
				uchar t0 = tab[src[j]];
				uchar t1 = tab[src[j + 1]];

				dst[j] = t0;
				dst[j + 1] = t1;

				t0 = tab[src[j + 2]];
				t1 = tab[src[j + 3]];

				dst[j + 2] = t0;
				dst[j + 3] = t1;
			}

			for (; j < roi.width; j++)
				dst[j] = tab[src[j]];
		}
	}
}

template<typename _Tp, int chs>
static void thresh_32f(const Mat_<_Tp, chs>& _src, Mat_<_Tp, chs>& _dst, float thresh, float maxval, int type)
{
	int i, j;
	Size roi = _src.size();
	roi.width *= _src.channels;
	const float* src = (const float*)_src.ptr();
	float* dst = (float*)_dst.ptr();
	size_t src_step = _src.step / sizeof(src[0]);
	size_t dst_step = _dst.step / sizeof(dst[0]);

	switch (type) {
	case THRESH_BINARY:
		for (i = 0; i < roi.height; i++, src += src_step, dst += dst_step) {
			for (j = 0; j < roi.width; j++)
				dst[j] = src[j] > thresh ? maxval : 0;
		}
		break;

	case THRESH_BINARY_INV:
		for (i = 0; i < roi.height; i++, src += src_step, dst += dst_step) {
			for (j = 0; j < roi.width; j++)
				dst[j] = src[j] <= thresh ? maxval : 0;
		}
		break;

	case THRESH_TRUNC:
		for (i = 0; i < roi.height; i++, src += src_step, dst += dst_step) {
			for (j = 0; j < roi.width; j++)
				dst[j] = std::min(src[j], thresh);
		}
		break;

	case THRESH_TOZERO:
		for (i = 0; i < roi.height; i++, src += src_step, dst += dst_step) {
			for (j = 0; j < roi.width; j++) {
				float v = src[j];
				dst[j] = v > thresh ? v : 0;
			}
		}
		break;

	case THRESH_TOZERO_INV:
		for (i = 0; i < roi.height; i++, src += src_step, dst += dst_step) {
			for (j = 0; j < roi.width; j++) {
				float v = src[j];
				dst[j] = v <= thresh ? v : 0;
			}
		}
		break;
	default:
		FBC_Error("BadArg");
	}
}

} // namespace fbc

#endif // FBC_CV_THRESHOLD_HPP_
