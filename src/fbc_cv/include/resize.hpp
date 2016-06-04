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
#include "core/utility.hpp"
#include "imgproc.hpp"

namespace fbc {

static const int MAX_ESIZE = 16;

// interpolation formulas and tables
const int INTER_RESIZE_COEF_BITS = 11;
const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;

template<typename _Tp, int chs> static int resize_nearest(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_linear(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_cubic(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_area(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);
template<typename _Tp, int chs> static int resize_lanczos4(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst);

// resize the image src down to or up to the specified size
// support type: uchar/float
template<typename _Tp, int chs>
int resize(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, int interpolation = NTER_LINEAR)
{
	FBC_Assert((interpolation >= 0) && (interpolation < 5));
	FBC_Assert((src.rows >= 4 && src.cols >= 4) && (dst.rows >= 4  && dst.cols >= 4));
	FBC_Assert((sizeof(_Tp) == 1) || sizeof(_Tp) == 4); // uchar || float

	Size ssize = src.size();
	Size dsize = dst.size();

	if (dsize == ssize) {
		// Source and destination are of same size. Use simple copy.
		src.copyTo(dst);
		return 0;
	}

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

template<typename ST, typename DT> struct Cast
{
	typedef ST type1;
	typedef DT rtype;

	DT operator()(ST val) const { return saturate_cast<DT>(val); }
};

template<typename ST, typename DT, int bits> struct FixedPtCast
{
	typedef ST type1;
	typedef DT rtype;
	enum { SHIFT = bits, DELTA = 1 << (bits - 1) };

	DT operator()(ST val) const { return saturate_cast<DT>((val + DELTA) >> SHIFT); }
};

static inline int clip(int x, int a, int b)
{
	return x >= a ? (x < b ? x : b - 1) : a;
}

template<typename T, typename WT, typename AT>
struct HResizeLinear
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const T** src, WT** dst, int count,
		const int* xofs, const AT* alpha,
		int swidth, int dwidth, int cn, int xmin, int xmax, int ONE) const
	{
		int dx, k;
		int dx0 = 0;

		for (k = 0; k <= count - 2; k++) {
			const T *S0 = src[k], *S1 = src[k + 1];
			WT *D0 = dst[k], *D1 = dst[k + 1];
			for (dx = dx0; dx < xmax; dx++) {
				int sx = xofs[dx];
				WT a0 = alpha[dx * 2], a1 = alpha[dx * 2 + 1];
				WT t0 = S0[sx] * a0 + S0[sx + cn] * a1;
				WT t1 = S1[sx] * a0 + S1[sx + cn] * a1;
				D0[dx] = t0; D1[dx] = t1;
			}

			for (; dx < dwidth; dx++) {
				int sx = xofs[dx];
				D0[dx] = WT(S0[sx] * ONE); D1[dx] = WT(S1[sx] * ONE);
			}
		}

		for (; k < count; k++) {
			const T *S = src[k];
			WT *D = dst[k];
			for (dx = 0; dx < xmax; dx++) {
				int sx = xofs[dx];
				D[dx] = S[sx] * alpha[dx * 2] + S[sx + cn] * alpha[dx * 2 + 1];
			}

			for (; dx < dwidth; dx++) {
				D[dx] = WT(S[xofs[dx]] * ONE);
			}
		}
	}
};

template<typename T, typename WT, typename AT, class CastOp>
struct VResizeLinear
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const WT** src, T* dst, const AT* beta, int width) const
	{
		WT b0 = beta[0], b1 = beta[1];
		const WT *S0 = src[0], *S1 = src[1];
		CastOp castOp;
		int x = 0;

		for (; x <= width - 4; x += 4) {
			WT t0, t1;
			t0 = S0[x] * b0 + S1[x] * b1;
			t1 = S0[x + 1] * b0 + S1[x + 1] * b1;
			dst[x] = castOp(t0); dst[x + 1] = castOp(t1);
			t0 = S0[x + 2] * b0 + S1[x + 2] * b1;
			t1 = S0[x + 3] * b0 + S1[x + 3] * b1;
			dst[x + 2] = castOp(t0); dst[x + 3] = castOp(t1);
		}

		for (; x < width; x++) {
			dst[x] = castOp(S0[x] * b0 + S1[x] * b1);
		}
	}
};

template<>
struct VResizeLinear<uchar, int, short, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>>
{
	typedef uchar value_type;
	typedef int buf_type;
	typedef short alpha_type;

	void operator()(const buf_type** src, value_type* dst, const alpha_type* beta, int width) const
	{
		alpha_type b0 = beta[0], b1 = beta[1];
		const buf_type *S0 = src[0], *S1 = src[1];
		int x = 0;

		for (; x <= width - 4; x += 4) {
			dst[x + 0] = uchar((((b0 * (S0[x + 0] >> 4)) >> 16) + ((b1 * (S1[x + 0] >> 4)) >> 16) + 2) >> 2);
			dst[x + 1] = uchar((((b0 * (S0[x + 1] >> 4)) >> 16) + ((b1 * (S1[x + 1] >> 4)) >> 16) + 2) >> 2);
			dst[x + 2] = uchar((((b0 * (S0[x + 2] >> 4)) >> 16) + ((b1 * (S1[x + 2] >> 4)) >> 16) + 2) >> 2);
			dst[x + 3] = uchar((((b0 * (S0[x + 3] >> 4)) >> 16) + ((b1 * (S1[x + 3] >> 4)) >> 16) + 2) >> 2);
		}

		for (; x < width; x++) {
			dst[x] = uchar((((b0 * (S0[x] >> 4)) >> 16) + ((b1 * (S1[x] >> 4)) >> 16) + 2) >> 2);
		}
	}
};

template<typename _Tp, typename value_type, typename buf_type, typename alpha_type, int chs>
static void resizeGeneric_(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst,
	const int* xofs, const void* _alpha, const int* yofs, const void* _beta, int xmin, int xmax, int ksize, int ONE)
{
	Size ssize = src.size(), dsize = dst.size();
	int dy, cn = src.channels;
	ssize.width *= cn;
	dsize.width *= cn;
	xmin *= cn;
	xmax *= cn;
	// image resize is a separable operation. In case of not too strong

	Range range(0, dsize.height);

	int bufstep = (int)alignSize(dsize.width, 16);
	AutoBuffer<buf_type> _buffer(bufstep*ksize);
	const value_type* srows[MAX_ESIZE] = { 0 };
	buf_type* rows[MAX_ESIZE] = { 0 };
	int prev_sy[MAX_ESIZE];

	for (int k = 0; k < ksize; k++) {
		prev_sy[k] = -1;
		rows[k] = (buf_type*)_buffer + bufstep*k;
	}

	const alpha_type* beta = (const alpha_type*)_beta + ksize * range.start;

	HResizeLinear<value_type, buf_type, alpha_type> hresize;
	VResizeLinear<value_type, buf_type, alpha_type, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>> vresize1;
	VResizeLinear<value_type, buf_type, alpha_type, Cast<float, float>> vresize2;

	for (dy = range.start; dy < range.end; dy++, beta += ksize) {
		int sy0 = yofs[dy], k0 = ksize, k1 = 0, ksize2 = ksize / 2;

		for (int k = 0; k < ksize; k++) {
			int sy = clip(sy0 - ksize2 + 1 + k, 0, ssize.height);
			for (k1 = std::max(k1, k); k1 < ksize; k1++) {
				if (sy == prev_sy[k1]) { // if the sy-th row has been computed already, reuse it.
					if (k1 > k) {
						memcpy(rows[k], rows[k1], bufstep*sizeof(rows[0][0]));
					}
					break;
				}
			}
			if (k1 == ksize) {
				k0 = std::min(k0, k); // remember the first row that needs to be computed
			}
			srows[k] = (const value_type*)src.ptr(sy);
			prev_sy[k] = sy;
		}

		if (k0 < ksize) {
			hresize((const value_type**)(srows + k0), (buf_type**)(rows + k0), ksize - k0, xofs, (const alpha_type*)(_alpha),
				ssize.width, dsize.width, cn, xmin, xmax, ONE);
		}
		if (sizeof(_Tp) == 1) { // uchar
			vresize1((const buf_type**)rows, (value_type*)(dst.data + dst.step*dy), beta, dsize.width);
		} else { // float
			vresize2((const buf_type**)rows, (value_type*)(dst.data + dst.step*dy), beta, dsize.width);
		}
	}
}

template<typename _Tp, int chs>
static int resize_nearest(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	Size ssize = src.size();
	Size dsize = dst.size();

	double fx = (double)dsize.width / ssize.width;
	double fy = (double)dsize.height / ssize.height;

	AutoBuffer<int> _x_ofs(dsize.width);
	int* x_ofs = _x_ofs;
	int pix_size = (int)src.elemSize();
	int pix_size4 = (int)(pix_size / sizeof(int));
	double ifx = 1. / fx, ify = 1. / fy;

	for (int x = 0; x < dsize.width; x++) {
		int sx = fbcFloor(x*ifx);
		x_ofs[x] = std::min(sx, ssize.width - 1)*pix_size;
	}

	Range range(0, dsize.height);
	int x, y;

	for (y = range.start; y < range.end; y++) {
		uchar* D = dst.data + dst.step*y;
		int sy = std::min(fbcFloor(y*ify), ssize.height - 1);
		const uchar* S = src.ptr(sy);

		switch (pix_size) {
		case 1:
			for (x = 0; x <= dsize.width - 2; x += 2) {
				uchar t0 = S[x_ofs[x]];
				uchar t1 = S[x_ofs[x + 1]];
				D[x] = t0;
				D[x + 1] = t1;
			}

			for (; x < dsize.width; x++) {
				D[x] = S[x_ofs[x]];
			}
			break;
		case 2:
			for (x = 0; x < dsize.width; x++) {
				*(ushort*)(D + x * 2) = *(ushort*)(S + x_ofs[x]);
			}
			break;
		case 3:
			for (x = 0; x < dsize.width; x++, D += 3) {
				const uchar* _tS = S + x_ofs[x];
				D[0] = _tS[0]; D[1] = _tS[1]; D[2] = _tS[2];
			}
			break;
		case 4:
			for (x = 0; x < dsize.width; x++) {
				*(int*)(D + x * 4) = *(int*)(S + x_ofs[x]);
			}
			break;
		case 6:
			for (x = 0; x < dsize.width; x++, D += 6) {
				const ushort* _tS = (const ushort*)(S + x_ofs[x]);
				ushort* _tD = (ushort*)D;
				_tD[0] = _tS[0]; _tD[1] = _tS[1]; _tD[2] = _tS[2];
			}
			break;
		case 8:
			for (x = 0; x < dsize.width; x++, D += 8) {
				const int* _tS = (const int*)(S + x_ofs[x]);
				int* _tD = (int*)D;
				_tD[0] = _tS[0]; _tD[1] = _tS[1];
			}
			break;
		case 12:
			for (x = 0; x < dsize.width; x++, D += 12) {
				const int* _tS = (const int*)(S + x_ofs[x]);
				int* _tD = (int*)D;
				_tD[0] = _tS[0]; _tD[1] = _tS[1]; _tD[2] = _tS[2];
			}
			break;
		default:
			for (x = 0; x < dsize.width; x++, D += pix_size) {
				const int* _tS = (const int*)(S + x_ofs[x]);
				int* _tD = (int*)D;
				for (int k = 0; k < pix_size4; k++)
					_tD[k] = _tS[k];
			}
		}
	}

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
	if (is_area_fast && iscale_x == 2 && iscale_y == 2) {
		resize_area(src, dst);
		return 0;
	}

	int cn = dst.channels;
	int k, sx, sy, dx, dy;
	int xmin = 0, xmax = dsize.width, width = dsize.width*cn;
	bool fixpt = sizeof(_Tp) == 1 ? true : false;
	float fx, fy;
	int ksize = 2, ksize2;
	ksize2 = ksize / 2;

	AutoBuffer<uchar> _buffer((width + dsize.height)*(sizeof(int) + sizeof(float)*ksize));
	int* xofs = (int*)(uchar*)_buffer;
	int* yofs = xofs + width;
	float* alpha = (float*)(yofs + dsize.height);
	short* ialpha = (short*)alpha;
	float* beta = alpha + width*ksize;
	short* ibeta = ialpha + width*ksize;
	float cbuf[MAX_ESIZE];

	for (dx = 0; dx < dsize.width; dx++) {
		fx = (float)((dx + 0.5)*scale_x - 0.5);
		sx = fbcFloor(fx);
		fx -= sx;

		if (sx < ksize2 - 1) {
			xmin = dx + 1;
			if (sx < 0) {
				fx = 0, sx = 0;
			}
		}

		if (sx + ksize2 >= ssize.width) {
			xmax = std::min(xmax, dx);
			if (sx >= ssize.width - 1) {
				fx = 0, sx = ssize.width - 1;
			}
		}

		for (k = 0, sx *= cn; k < cn; k++) {
			xofs[dx*cn + k] = sx + k;
		}

		cbuf[0] = 1.f - fx;
		cbuf[1] = fx;

		if (fixpt) {
			for (k = 0; k < ksize; k++) {
				ialpha[dx*cn*ksize + k] = saturate_cast<short>(cbuf[k] * INTER_RESIZE_COEF_SCALE);
			}
			for (; k < cn*ksize; k++) {
				ialpha[dx*cn*ksize + k] = ialpha[dx*cn*ksize + k - ksize];
			}
		} else {
			for (k = 0; k < ksize; k++) {
				alpha[dx*cn*ksize + k] = cbuf[k];
			}
			for (; k < cn*ksize; k++) {
				alpha[dx*cn*ksize + k] = alpha[dx*cn*ksize + k - ksize];
			}
		}
	}

	for (dy = 0; dy < dsize.height; dy++) {
		fy = (float)((dy + 0.5)*scale_y - 0.5);
		sy = fbcFloor(fy);
		fy -= sy;

		yofs[dy] = sy;
		cbuf[0] = 1.f - fy;
		cbuf[1] = fy;

		if (fixpt) {
			for (k = 0; k < ksize; k++) {
				ibeta[dy*ksize + k] = saturate_cast<short>(cbuf[k] * INTER_RESIZE_COEF_SCALE);
			}
		} else {
			for (k = 0; k < ksize; k++) {
				beta[dy*ksize + k] = cbuf[k];
			}
		}
	}

	if (sizeof(_Tp) == 1) { // uchar
		typedef uchar value_type; // HResizeLinear/VResizeLinear
		typedef int buf_type;
		typedef short alpha_type;
		int ONE = INTER_RESIZE_COEF_SCALE;

		resizeGeneric_<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize, ONE);
	} else if (sizeof(_Tp) == 4) { // float
		typedef float value_type; // HResizeLinear/VResizeLinear
		typedef float buf_type;
		typedef float alpha_type;
		int ONE = 1;

		resizeGeneric_<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize, ONE);
	} else {
		fprintf(stderr, "not support type\n");
		return -1;
	}

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

