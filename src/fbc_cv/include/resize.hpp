// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_RESIZE_HPP_
#define FBC_CV_RESIZE_HPP_

/* reference: imgproc/include/opencv2/imgproc.hpp
              imgproc/src/imgwarp.cpp
*/

#include <typeinfo>
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
int resize(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst, int interpolation = INTER_LINEAR)
{
	FBC_Assert((interpolation >= 0) && (interpolation < 5));
	FBC_Assert((src.rows >= 4 && src.cols >= 4) && (dst.rows >= 4  && dst.cols >= 4));
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float

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

struct DecimateAlpha
{
	int si, di;
	float alpha;
};

template<typename type>
static int computeResizeAreaTab(int ssize, int dsize, int cn, double scale, DecimateAlpha* tab)
{
	int k = 0;
	for (int dx = 0; dx < dsize; dx++) {
		double fsx1 = dx * scale;
		double fsx2 = fsx1 + scale;
		double cellWidth = std::min(scale, ssize - fsx1);

		int sx1 = fbcCeil(fsx1), sx2 = fbcFloor(fsx2);

		sx2 = std::min(sx2, ssize - 1);
		sx1 = std::min(sx1, sx2);

		if (sx1 - fsx1 > 1e-3) {
			assert(k < ssize * 2);
			tab[k].di = dx * cn;
			tab[k].si = (sx1 - 1) * cn;
			tab[k++].alpha = (float)((sx1 - fsx1) / cellWidth);
		}

		for (int sx = sx1; sx < sx2; sx++) {
			assert(k < ssize * 2);
			tab[k].di = dx * cn;
			tab[k].si = sx * cn;
			tab[k++].alpha = float(1.0 / cellWidth);
		}

		if (fsx2 - sx2 > 1e-3) {
			assert(k < ssize * 2);
			tab[k].di = dx * cn;
			tab[k].si = sx2 * cn;
			tab[k++].alpha = (float)(std::min(std::min(fsx2 - sx2, 1.), cellWidth) / cellWidth);
		}
	}
	return k;
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

template<typename type>
static type clip(type x, type a, type b)
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

template<typename T, typename WT, typename AT>
struct HResizeCubic
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const T** src, WT** dst, int count,
		const int* xofs, const AT* alpha,
		int swidth, int dwidth, int cn, int xmin, int xmax) const
	{
		for (int k = 0; k < count; k++) {
			const T *S = src[k];
			WT *D = dst[k];
			int dx = 0, limit = xmin;
			for (;;) {
				for (; dx < limit; dx++, alpha += 4) {
					int j, sx = xofs[dx] - cn;
					WT v = 0;
					for (j = 0; j < 4; j++) {
						int sxj = sx + j*cn;
						if ((unsigned)sxj >= (unsigned)swidth) {
							while (sxj < 0)
								sxj += cn;
							while (sxj >= swidth)
								sxj -= cn;
						}
						v += S[sxj] * alpha[j];
					}
					D[dx] = v;
				}
				if (limit == dwidth)
					break;
				for (; dx < xmax; dx++, alpha += 4) {
					int sx = xofs[dx];
					D[dx] = S[sx - cn] * alpha[0] + S[sx] * alpha[1] +
						S[sx + cn] * alpha[2] + S[sx + cn * 2] * alpha[3];
				}
				limit = dwidth;
			}
			alpha -= dwidth * 4;
		}
	}
};

template<typename T, typename WT, typename AT, class CastOp>
struct VResizeCubic
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const WT** src, T* dst, const AT* beta, int width) const
	{
		WT b0 = beta[0], b1 = beta[1], b2 = beta[2], b3 = beta[3];
		const WT *S0 = src[0], *S1 = src[1], *S2 = src[2], *S3 = src[3];
		CastOp castOp;

		int x = 0;
		for (; x < width; x++) {
			dst[x] = castOp(S0[x] * b0 + S1[x] * b1 + S2[x] * b2 + S3[x] * b3);
		}
	}
};

template<typename T, typename WT, typename AT>
struct HResizeLanczos4
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const T** src, WT** dst, int count,
		const int* xofs, const AT* alpha,
		int swidth, int dwidth, int cn, int xmin, int xmax) const
	{
		for (int k = 0; k < count; k++) {
			const T *S = src[k];
			WT *D = dst[k];
			int dx = 0, limit = xmin;
			for (;;) {
				for (; dx < limit; dx++, alpha += 8) {
					int j, sx = xofs[dx] - cn * 3;
					WT v = 0;
					for (j = 0; j < 8; j++) {
						int sxj = sx + j*cn;
						if ((unsigned)sxj >= (unsigned)swidth) {
							while (sxj < 0)
								sxj += cn;
							while (sxj >= swidth)
								sxj -= cn;
						}
						v += S[sxj] * alpha[j];
					}
					D[dx] = v;
				}
				if (limit == dwidth)
					break;
				for (; dx < xmax; dx++, alpha += 8) {
					int sx = xofs[dx];
					D[dx] = S[sx - cn * 3] * alpha[0] + S[sx - cn * 2] * alpha[1] +
						S[sx - cn] * alpha[2] + S[sx] * alpha[3] +
						S[sx + cn] * alpha[4] + S[sx + cn * 2] * alpha[5] +
						S[sx + cn * 3] * alpha[6] + S[sx + cn * 4] * alpha[7];
				}
				limit = dwidth;
			}
			alpha -= dwidth * 8;
		}
	}
};

template<typename T, typename WT, typename AT, class CastOp>
struct VResizeLanczos4
{
	typedef T value_type;
	typedef WT buf_type;
	typedef AT alpha_type;

	void operator()(const WT** src, T* dst, const AT* beta, int width) const
	{
		CastOp castOp;
		int k, x = 0;

		for (; x <= width - 4; x += 4) {
			WT b = beta[0];
			const WT* S = src[0];
			WT s0 = S[x] * b, s1 = S[x + 1] * b, s2 = S[x + 2] * b, s3 = S[x + 3] * b;

			for (k = 1; k < 8; k++) {
				b = beta[k]; S = src[k];
				s0 += S[x] * b; s1 += S[x + 1] * b;
				s2 += S[x + 2] * b; s3 += S[x + 3] * b;
			}

			dst[x] = castOp(s0); dst[x + 1] = castOp(s1);
			dst[x + 2] = castOp(s2); dst[x + 3] = castOp(s3);
		}

		for (; x < width; x++) {
			dst[x] = castOp(src[0][x] * beta[0] + src[1][x] * beta[1] +
				src[2][x] * beta[2] + src[3][x] * beta[3] + src[4][x] * beta[4] +
				src[5][x] * beta[5] + src[6][x] * beta[6] + src[7][x] * beta[7]);
		}
	}
};

template<typename T>
struct ResizeAreaFastVec
{
	ResizeAreaFastVec(int _scale_x, int _scale_y, int _cn, int _step) :
		scale_x(_scale_x), scale_y(_scale_y), cn(_cn), step(_step)
	{
		fast_mode = scale_x == 2 && scale_y == 2 && (cn == 1 || cn == 3 || cn == 4);
	}

	int operator() (const T* S, T* D, int w) const
	{
		if (!fast_mode) {
			return 0;
		}

		const T* nextS = (const T*)((const uchar*)S + step);
		int dx = 0;

		if (cn == 1) {
			for (; dx < w; ++dx) {
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 1] + nextS[index] + nextS[index + 1] + 2) >> 2);
			}
		}
		else if (cn == 3) {
			for (; dx < w; dx += 3) {
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 3] + nextS[index] + nextS[index + 3] + 2) >> 2);
				D[dx + 1] = (T)((S[index + 1] + S[index + 4] + nextS[index + 1] + nextS[index + 4] + 2) >> 2);
				D[dx + 2] = (T)((S[index + 2] + S[index + 5] + nextS[index + 2] + nextS[index + 5] + 2) >> 2);
			}
		} else {
			FBC_Assert(cn == 4);
			for (; dx < w; dx += 4) {
				int index = dx * 2;
				D[dx] = (T)((S[index] + S[index + 4] + nextS[index] + nextS[index + 4] + 2) >> 2);
				D[dx + 1] = (T)((S[index + 1] + S[index + 5] + nextS[index + 1] + nextS[index + 5] + 2) >> 2);
				D[dx + 2] = (T)((S[index + 2] + S[index + 6] + nextS[index + 2] + nextS[index + 6] + 2) >> 2);
				D[dx + 3] = (T)((S[index + 3] + S[index + 7] + nextS[index + 3] + nextS[index + 7] + 2) >> 2);
			}
		}

		return dx;
	}

private:
	int scale_x, scale_y;
	int cn;
	bool fast_mode;
	int step;
};

template<typename _Tp, typename value_type, typename buf_type, typename alpha_type, int chs>
static void resizeGeneric_Linear(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst,
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
			int sy = clip<int>(sy0 - ksize2 + 1 + k, 0, ssize.height);
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

template<typename _Tp, typename value_type, typename buf_type, typename alpha_type, int chs>
static void resizeGeneric_Cubic(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst,
	const int* xofs, const void* _alpha, const int* yofs, const void* _beta, int xmin, int xmax, int ksize)
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

	HResizeCubic<value_type, buf_type, alpha_type> hresize;
	VResizeCubic<value_type, buf_type, alpha_type, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>> vresize1;
	VResizeCubic<value_type, buf_type, alpha_type, Cast<float, float>> vresize2;

	for (dy = range.start; dy < range.end; dy++, beta += ksize) {
		int sy0 = yofs[dy], k0 = ksize, k1 = 0, ksize2 = ksize / 2;

		for (int k = 0; k < ksize; k++) {
			int sy = clip<int>(sy0 - ksize2 + 1 + k, 0, ssize.height);
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
				ssize.width, dsize.width, cn, xmin, xmax);
		}
		if (sizeof(_Tp) == 1) { // uchar
			vresize1((const buf_type**)rows, (value_type*)(dst.data + dst.step*dy), beta, dsize.width);
		} else { // float
			vresize2((const buf_type**)rows, (value_type*)(dst.data + dst.step*dy), beta, dsize.width);
		}
	}
}

template<typename _Tp, typename value_type, typename buf_type, typename alpha_type, int chs>
static void resizeGeneric_Lanczos4(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst,
	const int* xofs, const void* _alpha, const int* yofs, const void* _beta, int xmin, int xmax, int ksize)
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

	HResizeLanczos4<value_type, buf_type, alpha_type> hresize;
	VResizeLanczos4<value_type, buf_type, alpha_type, FixedPtCast<int, uchar, INTER_RESIZE_COEF_BITS * 2>> vresize1;
	VResizeLanczos4<value_type, buf_type, alpha_type, Cast<float, float>> vresize2;

	for (dy = range.start; dy < range.end; dy++, beta += ksize) {
		int sy0 = yofs[dy], k0 = ksize, k1 = 0, ksize2 = ksize / 2;

		for (int k = 0; k < ksize; k++) {
			int sy = clip<int>(sy0 - ksize2 + 1 + k, 0, ssize.height);
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
				ssize.width, dsize.width, cn, xmin, xmax);
		}
		if (sizeof(_Tp) == 1) { // uchar
			vresize1((const buf_type**)rows, (value_type*)(dst.data + dst.step*dy), beta, dsize.width);
		}
		else { // float
			vresize2((const buf_type**)rows, (value_type*)(dst.data + dst.step*dy), beta, dsize.width);
		}
	}

}

template<typename _Tp, typename T, typename WT, int chs>
static void resizeGeneric_Area(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst,
	const DecimateAlpha* xtab0, int xtab_size0, const DecimateAlpha* ytab, int ytab_size, const int* tabofs)
{
	Size dsize = dst.size();
	int cn = dst.channels;
	Range range(0, dsize.height);
	dsize.width *= cn;
	AutoBuffer<WT> _buffer(dsize.width * 2);
	const DecimateAlpha* xtab = xtab0;
	int xtab_size = xtab_size0;
	WT *buf = _buffer, *sum = buf + dsize.width;
	int j_start = tabofs[range.start], j_end = tabofs[range.end], j, k, dx, prev_dy = ytab[j_start].di;

	for (dx = 0; dx < dsize.width; dx++) {
		sum[dx] = (WT)0;
	}

	for (j = j_start; j < j_end; j++) {
		WT beta = ytab[j].alpha;
		int dy = ytab[j].di;
		int sy = ytab[j].si;

		const T* S = (const T*)src.ptr(sy);
		for (dx = 0; dx < dsize.width; dx++) {
			buf[dx] = (WT)0;
		}

		if (cn == 1) {
			for (k = 0; k < xtab_size; k++) {
				int dxn = xtab[k].di;
				WT alpha = xtab[k].alpha;
				buf[dxn] += S[xtab[k].si] * alpha;
			}
		} else if (cn == 2) {
			for (k = 0; k < xtab_size; k++) {
				int sxn = xtab[k].si;
				int dxn = xtab[k].di;
				WT alpha = xtab[k].alpha;
				WT t0 = buf[dxn] + S[sxn] * alpha;
				WT t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
				buf[dxn] = t0; buf[dxn + 1] = t1;
			}
		} else if (cn == 3) {
			for (k = 0; k < xtab_size; k++) {
				int sxn = xtab[k].si;
				int dxn = xtab[k].di;
				WT alpha = xtab[k].alpha;
				WT t0 = buf[dxn] + S[sxn] * alpha;
				WT t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
				WT t2 = buf[dxn + 2] + S[sxn + 2] * alpha;
				buf[dxn] = t0; buf[dxn + 1] = t1; buf[dxn + 2] = t2;
			}
		} else if (cn == 4) {
			for (k = 0; k < xtab_size; k++) {
				int sxn = xtab[k].si;
				int dxn = xtab[k].di;
				WT alpha = xtab[k].alpha;
				WT t0 = buf[dxn] + S[sxn] * alpha;
				WT t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
				buf[dxn] = t0; buf[dxn + 1] = t1;
				t0 = buf[dxn + 2] + S[sxn + 2] * alpha;
				t1 = buf[dxn + 3] + S[sxn + 3] * alpha;
				buf[dxn + 2] = t0; buf[dxn + 3] = t1;
			}
		} else {
			for (k = 0; k < xtab_size; k++) {
				int sxn = xtab[k].si;
				int dxn = xtab[k].di;
				WT alpha = xtab[k].alpha;
				for (int c = 0; c < cn; c++)
					buf[dxn + c] += S[sxn + c] * alpha;
			}
		}

		if (dy != prev_dy) {
			T* D = (T*)dst.ptr(prev_dy);

			for (dx = 0; dx < dsize.width; dx++) {
				D[dx] = saturate_cast<T>(sum[dx]);
				sum[dx] = beta*buf[dx];
			}
			prev_dy = dy;
		} else {
			for (dx = 0; dx < dsize.width; dx++) {
				sum[dx] += beta*buf[dx];
			}
		}
	}

	T* D = (T*)dst.ptr(prev_dy);
	for (dx = 0; dx < dsize.width; dx++) {
		D[dx] = saturate_cast<T>(sum[dx]);
	}
}

template<typename _Tp, typename T, typename WT, int chs>
static void resizeGeneric_AreaFast(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst,
	const int* ofs, const int* xofs, int scale_x, int scale_y)
{
	Size ssize = src.size(), dsize = dst.size();
	int cn = src.channels;
	Range range(0, dsize.height);
	int area = scale_x*scale_y;
	float scale = 1.f / (area);
	int dwidth1 = (ssize.width / scale_x)*cn;
	dsize.width *= cn;
	ssize.width *= cn;
	int dy, dx, k = 0;

	ResizeAreaFastVec<uchar> vop(scale_x, scale_y, src.channels, (int)src.step);

	for (dy = range.start; dy < range.end; dy++) {
		T* D = (T*)(dst.data + dst.step*dy);
		int sy0 = dy*scale_y;
		int w = sy0 + scale_y <= ssize.height ? dwidth1 : 0;

		if (sy0 >= ssize.height) {
			for (dx = 0; dx < dsize.width; dx++) {
				D[dx] = 0;
			}
			continue;
		}

		dx = sizeof(_Tp) == 1 ? vop(src.ptr(sy0), (uchar*)D, w) : 0;
		for (; dx < w; dx++) {
			const T* S = (const T*)src.ptr(sy0) +xofs[dx];
			WT sum = 0;
			k = 0;

			for (; k <= area - 4; k += 4) {
				sum += S[ofs[k]] + S[ofs[k + 1]] + S[ofs[k + 2]] + S[ofs[k + 3]];
			}

			for (; k < area; k++) {
				sum += S[ofs[k]];
			}

			D[dx] = saturate_cast<T>(sum * scale);
		}

		for (; dx < dsize.width; dx++) {
			WT sum = 0;
			int count = 0, sx0 = xofs[dx];
			if (sx0 >= ssize.width) {
				D[dx] = 0;
			}

			for (int sy = 0; sy < scale_y; sy++) {
				if (sy0 + sy >= ssize.height) {
					break;
				}
				const T* S = (const T*)src.ptr(sy0 + sy) + sx0;
				for (int sx = 0; sx < scale_x*cn; sx += cn) {
					if (sx0 + sx >= ssize.width) {
						break;
					}
					sum += S[sx];
					count++;
				}
			}

			D[dx] = saturate_cast<T>((float)sum / count);
		}
	}
}

template<typename _Tp>
static void interpolateCubic(_Tp x, _Tp* coeffs)
{
	const float A = -0.75f;

	coeffs[0] = ((A*(x + 1) - 5 * A)*(x + 1) + 8 * A)*(x + 1) - 4 * A;
	coeffs[1] = ((A + 2)*x - (A + 3))*x*x + 1;
	coeffs[2] = ((A + 2)*(1 - x) - (A + 3))*(1 - x)*(1 - x) + 1;
	coeffs[3] = 1.f - coeffs[0] - coeffs[1] - coeffs[2];
}

template<typename _Tp>
static void interpolateLanczos4(_Tp x, _Tp* coeffs)
{
	static const double s45 = 0.70710678118654752440084436210485;
	static const double cs[][2] = { { 1, 0 }, { -s45, -s45 }, { 0, 1 }, { s45, -s45 }, { -1, 0 }, { s45, s45 }, { 0, -1 }, { -s45, s45 } };

	if (x < FLT_EPSILON) {
		for (int i = 0; i < 8; i++) {
			coeffs[i] = 0;
		}
		coeffs[3] = 1;
		return;
	}

	float sum = 0;
	double y0 = -(x + 3)*FBC_PI*0.25, s0 = sin(y0), c0 = cos(y0);
	for (int i = 0; i < 8; i++) {
		double y = -(x + 3 - i)*FBC_PI*0.25;
		coeffs[i] = (float)((cs[i][0] * s0 + cs[i][1] * c0) / (y*y));
		sum += coeffs[i];
	}

	sum = 1.f / sum;
	for (int i = 0; i < 8; i++) {
		coeffs[i] *= sum;
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
	// in case of scale_x && scale_y is equal to 2
	// INTER_AREA (fast) also is equal to INTER_LINEAR
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

		resizeGeneric_Linear<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize, ONE);
	} else if (sizeof(_Tp) == 4) { // float
		typedef float value_type; // HResizeLinear/VResizeLinear
		typedef float buf_type;
		typedef float alpha_type;
		int ONE = 1;

		resizeGeneric_Linear<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
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
	Size ssize = src.size();
	Size dsize = dst.size();

	double inv_scale_x = (double)dsize.width / ssize.width;
	double inv_scale_y = (double)dsize.height / ssize.height;
	double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;

	int cn = dst.channels;
	int k, sx, sy, dx, dy;
	int xmin = 0, xmax = dsize.width, width = dsize.width*cn;
	bool fixpt = sizeof(_Tp) == 1 ? true : false;
	float fx, fy;
	int ksize = 4, ksize2;
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
		}

		if (sx + ksize2 >= ssize.width) {
			xmax = std::min(xmax, dx);
		}

		for (k = 0, sx *= cn; k < cn; k++) {
			xofs[dx*cn + k] = sx + k;
		}

		interpolateCubic<float>(fx, cbuf);

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
		interpolateCubic<float>(fy, cbuf);

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
		typedef uchar value_type; // HResizeCubic/VResizeCubic
		typedef int buf_type;
		typedef short alpha_type;

		resizeGeneric_Cubic<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize);
	} else if (sizeof(_Tp) == 4) { // float
		typedef float value_type; // HResizeCubic/VResizeCubic
		typedef float buf_type;
		typedef float alpha_type;

		resizeGeneric_Cubic<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize);
	} else {
		fprintf(stderr, "not support type\n");
		return -1;
	}

	return 0;
}

template<typename _Tp, int chs>
static int resize_area(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	Size ssize = src.size();
	Size dsize = dst.size();
	int cn = dst.channels;

	double inv_scale_x = (double)dsize.width / ssize.width;
	double inv_scale_y = (double)dsize.height / ssize.height;
	double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;

	int iscale_x = saturate_cast<int>(scale_x);
	int iscale_y = saturate_cast<int>(scale_y);

	bool is_area_fast = std::abs(scale_x - iscale_x) < DBL_EPSILON && std::abs(scale_y - iscale_y) < DBL_EPSILON;

	int k, sx, sy, dx, dy;

	// true "area" interpolation is only implemented for the case (scale_x <= 1 && scale_y <= 1).
	// In other cases it is emulated using some variant of bilinear interpolation
	if (scale_x >= 1 && scale_y >= 1) {
		if (is_area_fast) {
			int area = iscale_x*iscale_y;
			size_t srcstep = src.step / sizeof(_Tp);
			AutoBuffer<int> _ofs(area + dsize.width*cn);
			int* ofs = _ofs;
			int* xofs = ofs + area;

			for (sy = 0, k = 0; sy < iscale_y; sy++) {
				for (sx = 0; sx < iscale_x; sx++) {
					ofs[k++] = (int)(sy*srcstep + sx*cn);
				}
			}

			for (dx = 0; dx < dsize.width; dx++) {
				int j = dx * cn;
				sx = iscale_x * j;
				for (k = 0; k < cn; k++) {
					xofs[j + k] = sx + k;
				}
			}

			if (sizeof(_Tp) == 1) { // uchar
				typedef uchar T;
				typedef int WT;

				resizeGeneric_AreaFast<_Tp, T, WT, chs>(src, dst, ofs, xofs, iscale_x, iscale_y);
			} else if (sizeof(_Tp) == 4) { // float
				typedef float T;
				typedef float WT;

				resizeGeneric_AreaFast<_Tp, T, WT, chs>(src, dst, ofs, xofs, iscale_x, iscale_y);
			} else {
				fprintf(stderr, "not support type\n");
				return -1;
			}

			return 0;
		}

		FBC_Assert(cn <= 4);

		AutoBuffer<DecimateAlpha> _xytab((ssize.width + ssize.height) * 2);
		DecimateAlpha* xtab = _xytab, *ytab = xtab + ssize.width * 2;

		int xtab_size = computeResizeAreaTab<int>(ssize.width, dsize.width, cn, scale_x, xtab);
		int ytab_size = computeResizeAreaTab<int>(ssize.height, dsize.height, 1, scale_y, ytab);

		AutoBuffer<int> _tabofs(dsize.height + 1);
		int* tabofs = _tabofs;
		for (k = 0, dy = 0; k < ytab_size; k++) {
			if (k == 0 || ytab[k].di != ytab[k - 1].di) {
				assert(ytab[k].di == dy);
				tabofs[dy++] = k;
			}
		}
		tabofs[dy] = ytab_size;

		if (sizeof(_Tp) == 1) { // uchar
			typedef uchar T;
			typedef float WT;

			resizeGeneric_Area<_Tp, T, WT, chs>(src, dst, xtab, xtab_size, ytab, ytab_size, tabofs);
		} else if (sizeof(_Tp) == 4) { // float
			typedef float T;
			typedef float WT;

			resizeGeneric_Area<_Tp, T, WT, chs>(src, dst, xtab, xtab_size, ytab, ytab_size, tabofs);
		} else {
			fprintf(stderr, "not support type\n");
			return -1;
		}

		return 0;
	}

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
		sx = fbcFloor(dx*scale_x);
		fx = (float)((dx + 1) - (sx + 1)*inv_scale_x);
		fx = fx <= 0 ? 0.f : fx - fbcFloor(fx);

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
		sy = fbcFloor(dy*scale_y);
		fy = (float)((dy + 1) - (sy + 1)*inv_scale_y);
		fy = fy <= 0 ? 0.f : fy - fbcFloor(fy);

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

		resizeGeneric_Linear<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize, ONE);
	} else if (sizeof(_Tp) == 4) { // float
		typedef float value_type; // HResizeLinear/VResizeLinear
		typedef float buf_type;
		typedef float alpha_type;
		int ONE = 1;

		resizeGeneric_Linear<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize, ONE);
	} else {
		fprintf(stderr, "not support type\n");
		return -1;
	}

	return 0;
}

template<typename _Tp, int chs>
static int resize_lanczos4(const Mat_<_Tp, chs>& src, Mat_<_Tp, chs>& dst)
{
	Size ssize = src.size();
	Size dsize = dst.size();

	double inv_scale_x = (double)dsize.width / ssize.width;
	double inv_scale_y = (double)dsize.height / ssize.height;
	double scale_x = 1. / inv_scale_x, scale_y = 1. / inv_scale_y;

	int cn = dst.channels;
	int k, sx, sy, dx, dy;
	int xmin = 0, xmax = dsize.width, width = dsize.width*cn;
	bool fixpt = sizeof(_Tp) == 1 ? true : false;
	float fx, fy;
	int ksize = 8, ksize2;
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
		}

		if (sx + ksize2 >= ssize.width) {
			xmax = std::min(xmax, dx);
		}

		for (k = 0, sx *= cn; k < cn; k++) {
			xofs[dx*cn + k] = sx + k;
		}

		interpolateLanczos4<float>(fx, cbuf);

		if (fixpt) {
			for (k = 0; k < ksize; k++)
				ialpha[dx*cn*ksize + k] = saturate_cast<short>(cbuf[k] * INTER_RESIZE_COEF_SCALE);
			for (; k < cn*ksize; k++)
				ialpha[dx*cn*ksize + k] = ialpha[dx*cn*ksize + k - ksize];
		} else {
			for (k = 0; k < ksize; k++)
				alpha[dx*cn*ksize + k] = cbuf[k];
			for (; k < cn*ksize; k++)
				alpha[dx*cn*ksize + k] = alpha[dx*cn*ksize + k - ksize];
		}
	}

	for (dy = 0; dy < dsize.height; dy++) {
		fy = (float)((dy + 0.5)*scale_y - 0.5);
		sy = fbcFloor(fy);
		fy -= sy;

		yofs[dy] = sy;

		interpolateLanczos4<float>(fy, cbuf);

		if (fixpt){
			for (k = 0; k < ksize; k++)
				ibeta[dy*ksize + k] = saturate_cast<short>(cbuf[k] * INTER_RESIZE_COEF_SCALE);
		} else {
			for (k = 0; k < ksize; k++)
				beta[dy*ksize + k] = cbuf[k];
		}
	}

	if (sizeof(_Tp) == 1) { // uchar
		typedef uchar value_type; // HResizeLanczos4/VResizeLanczos4
		typedef int buf_type;
		typedef short alpha_type;

		resizeGeneric_Lanczos4<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize);
	} else if (sizeof(_Tp) == 4) { // float
		typedef float value_type; // HResizeLanczos4/VResizeLanczos4
		typedef float buf_type;
		typedef float alpha_type;

		resizeGeneric_Lanczos4<_Tp, value_type, buf_type, alpha_type, chs>(src, dst,
			xofs, fixpt ? (void*)ialpha : (void*)alpha, yofs, fixpt ? (void*)ibeta : (void*)beta, xmin, xmax, ksize);
	} else {
		fprintf(stderr, "not support type\n");
		return -1;
	}

	return 0;
}

} // namespace fbc

#endif // FBC_CV_RESIZE_HPP_

