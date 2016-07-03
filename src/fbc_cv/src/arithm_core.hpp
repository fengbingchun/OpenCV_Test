// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_SRC_ARITHM_CORE_HPP_
#define FBC_CV_SRC_ARITHM_CORE_HPP_

// reference: modules/core/src/arithm_core.cpp

#include "core/saturate.hpp"

namespace fbc {

struct NOP {};
#define IF_SIMD(op) NOP

template <typename T>
struct Cmp_SIMD
{
	explicit Cmp_SIMD(int)
	{
	}

	int operator () (const T *, const T *, uchar *, int) const
	{
		return 0;
	}
};

template <typename T, typename WT>
struct Mul_SIMD
{
	int operator() (const T *, const T *, T *, int, WT) const
	{
		return 0;
	}
};

template <typename T>
struct Div_SIMD
{
	int operator() (const T *, const T *, T *, int, double) const
	{
		return 0;
	}
};

template <typename T>
struct Recip_SIMD
{
	int operator() (const T *, T *, int, double) const
	{
		return 0;
	}
};

template <typename T, typename WT>
struct AddWeighted_SIMD
{
	int operator() (const T *, const T *, T *, int, WT, WT, WT) const
	{
		return 0;
	}
};

template<typename T1, typename T2 = T1, typename T3 = T1> struct OpAdd
{
	typedef T1 type1;
	typedef T2 type2;
	typedef T3 rtype;
	T3 operator ()(const T1 a, const T2 b) const { return saturate_cast<T3>(a + b); }
};

template<typename T1, typename T2 = T1, typename T3 = T1> struct OpSub
{
	typedef T1 type1;
	typedef T2 type2;
	typedef T3 rtype;
	T3 operator ()(const T1 a, const T2 b) const { return saturate_cast<T3>(a - b); }
};

template<typename T1, typename T2 = T1, typename T3 = T1> struct OpRSub
{
	typedef T1 type1;
	typedef T2 type2;
	typedef T3 rtype;
	T3 operator ()(const T1 a, const T2 b) const { return saturate_cast<T3>(b - a); }
};

template<typename T> struct OpMin
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator ()(const T a, const T b) const { return std::min(a, b); }
};

template<typename T> struct OpMax
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator ()(const T a, const T b) const { return std::max(a, b); }
};

template<typename T> struct OpAbsDiff
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator()(T a, T b) const { return a > b ? a - b : b - a; }
};

template<typename T> struct OpAnd
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator()(T a, T b) const { return a & b; }
};

template<typename T> struct OpOr
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator()(T a, T b) const { return a | b; }
};

template<typename T> struct OpXor
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator()(T a, T b) const { return a ^ b; }
};

template<typename T> struct OpNot
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator()(T a, T) const { return ~a; }
};

////////////////////////////////////////////////////////////////////////

template<typename T, class Op, class VOp>
void vBinOp(const T* src1, size_t step1, const T* src2, size_t step2, T* dst, size_t step, int width, int height)
{
	Op op;

	for (; height--; src1 = (const T *)((const uchar *)src1 + step1),
		src2 = (const T *)((const uchar *)src2 + step2),
		dst = (T *)((uchar *)dst + step)) {
		int x = 0;

		for (; x <= width - 4; x += 4) {
			T v0 = op(src1[x], src2[x]);
			T v1 = op(src1[x + 1], src2[x + 1]);
			dst[x] = v0; dst[x + 1] = v1;
			v0 = op(src1[x + 2], src2[x + 2]);
			v1 = op(src1[x + 3], src2[x + 3]);
			dst[x + 2] = v0; dst[x + 3] = v1;
		}

		for (; x < width; x++)
			dst[x] = op(src1[x], src2[x]);
	}
}

template<typename T, class Op, class Op32>
void vBinOp32(const T* src1, size_t step1, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height)
{
	Op op;

	for (; height--; src1 = (const T *)((const uchar *)src1 + step1),
		src2 = (const T *)((const uchar *)src2 + step2),
		dst = (T *)((uchar *)dst + step)) {
		int x = 0;

		for (; x <= width - 4; x += 4) {
			T v0 = op(src1[x], src2[x]);
			T v1 = op(src1[x + 1], src2[x + 1]);
			dst[x] = v0; dst[x + 1] = v1;
			v0 = op(src1[x + 2], src2[x + 2]);
			v1 = op(src1[x + 3], src2[x + 3]);
			dst[x + 2] = v0; dst[x + 3] = v1;
		}

		for (; x < width; x++)
			dst[x] = op(src1[x], src2[x]);
	}
}

template<typename T, class Op, class Op64>
void vBinOp64(const T* src1, size_t step1, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height)
{
	Op op;

	for (; height--; src1 = (const T *)((const uchar *)src1 + step1),
		src2 = (const T *)((const uchar *)src2 + step2),
		dst = (T *)((uchar *)dst + step)) {
		int x = 0;

		for (; x <= width - 4; x += 4) {
			T v0 = op(src1[x], src2[x]);
			T v1 = op(src1[x + 1], src2[x + 1]);
			dst[x] = v0; dst[x + 1] = v1;
			v0 = op(src1[x + 2], src2[x + 2]);
			v1 = op(src1[x + 3], src2[x + 3]);
			dst[x + 2] = v0; dst[x + 3] = v1;
		}

		for (; x < width; x++)
			dst[x] = op(src1[x], src2[x]);
	}
}

template<typename T> static void
cmp_(const T* src1, size_t step1, const T* src2, size_t step2,
	uchar* dst, size_t step, int width, int height, int code)
{
	step1 /= sizeof(src1[0]);
	step2 /= sizeof(src2[0]);
	if (code == CMP_GE || code == CMP_LT) {
		std::swap(src1, src2);
		std::swap(step1, step2);
		code = code == CMP_GE ? CMP_LE : CMP_GT;
	}

	Cmp_SIMD<T> vop(code);

	if (code == CMP_GT || code == CMP_LE) {
		int m = code == CMP_GT ? 0 : 255;
		for (; height--; src1 += step1, src2 += step2, dst += step) {
			int x = vop(src1, src2, dst, width);

			for (; x <= width - 4; x += 4) {
				int t0, t1;
				t0 = -(src1[x] > src2[x]) ^ m;
				t1 = -(src1[x + 1] > src2[x + 1]) ^ m;
				dst[x] = (uchar)t0; dst[x + 1] = (uchar)t1;
				t0 = -(src1[x + 2] > src2[x + 2]) ^ m;
				t1 = -(src1[x + 3] > src2[x + 3]) ^ m;
				dst[x + 2] = (uchar)t0; dst[x + 3] = (uchar)t1;
			}

			for (; x < width; x++)
				dst[x] = (uchar)(-(src1[x] > src2[x]) ^ m);
		}
	} else if (code == CMP_EQ || code == CMP_NE) {
		int m = code == CMP_EQ ? 0 : 255;
		for (; height--; src1 += step1, src2 += step2, dst += step) {
			int x = 0;
			for (; x <= width - 4; x += 4) {
				int t0, t1;
				t0 = -(src1[x] == src2[x]) ^ m;
				t1 = -(src1[x + 1] == src2[x + 1]) ^ m;
				dst[x] = (uchar)t0; dst[x + 1] = (uchar)t1;
				t0 = -(src1[x + 2] == src2[x + 2]) ^ m;
				t1 = -(src1[x + 3] == src2[x + 3]) ^ m;
				dst[x + 2] = (uchar)t0; dst[x + 3] = (uchar)t1;
			}

			for (; x < width; x++)
				dst[x] = (uchar)(-(src1[x] == src2[x]) ^ m);
		}
	}
}

template<typename T, typename WT> static void
mul_(const T* src1, size_t step1, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height, WT scale)
{
	step1 /= sizeof(src1[0]);
	step2 /= sizeof(src2[0]);
	step /= sizeof(dst[0]);

	Mul_SIMD<T, WT> vop;

	if (scale == (WT)1.) {
		for (; height--; src1 += step1, src2 += step2, dst += step) {
			int i = vop(src1, src2, dst, width, scale);

			for (; i <= width - 4; i += 4) {
				T t0;
				T t1;
				t0 = saturate_cast<T>(src1[i] * src2[i]);
				t1 = saturate_cast<T>(src1[i + 1] * src2[i + 1]);
				dst[i] = t0;
				dst[i + 1] = t1;

				t0 = saturate_cast<T>(src1[i + 2] * src2[i + 2]);
				t1 = saturate_cast<T>(src1[i + 3] * src2[i + 3]);
				dst[i + 2] = t0;
				dst[i + 3] = t1;
			}

			for (; i < width; i++)
				dst[i] = saturate_cast<T>(src1[i] * src2[i]);
		}
	} else {
		for (; height--; src1 += step1, src2 += step2, dst += step) {
			int i = vop(src1, src2, dst, width, scale);

			for (; i <= width - 4; i += 4) {
				T t0 = saturate_cast<T>(scale*(WT)src1[i] * src2[i]);
				T t1 = saturate_cast<T>(scale*(WT)src1[i + 1] * src2[i + 1]);
				dst[i] = t0; dst[i + 1] = t1;

				t0 = saturate_cast<T>(scale*(WT)src1[i + 2] * src2[i + 2]);
				t1 = saturate_cast<T>(scale*(WT)src1[i + 3] * src2[i + 3]);
				dst[i + 2] = t0; dst[i + 3] = t1;
			}

			for (; i < width; i++)
				dst[i] = saturate_cast<T>(scale*(WT)src1[i] * src2[i]);
		}
	}
}

template<typename T> static void
div_i(const T* src1, size_t step1, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height, double scale)
{
	step1 /= sizeof(src1[0]);
	step2 /= sizeof(src2[0]);
	step /= sizeof(dst[0]);

	Div_SIMD<T> vop;
	float scale_f = (float)scale;

	for (; height--; src1 += step1, src2 += step2, dst += step) {
		int i = vop(src1, src2, dst, width, scale);
		for (; i < width; i++) {
			T num = src1[i], denom = src2[i];
			dst[i] = denom != 0 ? saturate_cast<T>(num*scale_f / denom) : (T)0;
		}
	}
}

template<typename T> static void
div_f(const T* src1, size_t step1, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height, double scale)
{
	T scale_f = (T)scale;
	step1 /= sizeof(src1[0]);
	step2 /= sizeof(src2[0]);
	step /= sizeof(dst[0]);

	Div_SIMD<T> vop;

	for (; height--; src1 += step1, src2 += step2, dst += step) {
		int i = vop(src1, src2, dst, width, scale);
		for (; i < width; i++) {
			T num = src1[i], denom = src2[i];
			dst[i] = denom != 0 ? saturate_cast<T>(num*scale_f / denom) : (T)0;
		}
	}
}

template<typename T> static void
recip_i(const T*, size_t, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height, double scale)
{
	step2 /= sizeof(src2[0]);
	step /= sizeof(dst[0]);

	Recip_SIMD<T> vop;
	float scale_f = (float)scale;

	for (; height--; src2 += step2, dst += step) {
		int i = vop(src2, dst, width, scale);
		for (; i < width; i++) {
			T denom = src2[i];
			dst[i] = denom != 0 ? saturate_cast<T>(scale_f / denom) : (T)0;
		}
	}
}

template<typename T> static void
recip_f(const T*, size_t, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height, double scale)
{
	T scale_f = (T)scale;
	step2 /= sizeof(src2[0]);
	step /= sizeof(dst[0]);

	Recip_SIMD<T> vop;

	for (; height--; src2 += step2, dst += step) {
		int i = vop(src2, dst, width, scale);
		for (; i < width; i++) {
			T denom = src2[i];
			dst[i] = denom != 0 ? saturate_cast<T>(scale_f / denom) : (T)0;
		}
	}
}

template<typename T, typename WT> static void
addWeighted_(const T* src1, size_t step1, const T* src2, size_t step2,
	T* dst, size_t step, int width, int height, void* _scalars)
{
	const double* scalars = (const double*)_scalars;
	WT alpha = (WT)scalars[0], beta = (WT)scalars[1], gamma = (WT)scalars[2];
	step1 /= sizeof(src1[0]);
	step2 /= sizeof(src2[0]);
	step /= sizeof(dst[0]);

	AddWeighted_SIMD<T, WT> vop;

	for (; height--; src1 += step1, src2 += step2, dst += step) {
		int x = vop(src1, src2, dst, width, alpha, beta, gamma);

		for (; x <= width - 4; x += 4) {
			T t0 = saturate_cast<T>(src1[x] * alpha + src2[x] * beta + gamma);
			T t1 = saturate_cast<T>(src1[x + 1] * alpha + src2[x + 1] * beta + gamma);
			dst[x] = t0; dst[x + 1] = t1;

			t0 = saturate_cast<T>(src1[x + 2] * alpha + src2[x + 2] * beta + gamma);
			t1 = saturate_cast<T>(src1[x + 3] * alpha + src2[x + 3] * beta + gamma);
			dst[x + 2] = t0; dst[x + 3] = t1;
		}

		for (; x < width; x++)
			dst[x] = saturate_cast<T>(src1[x] * alpha + src2[x] * beta + gamma);
	}
}

} // namespace fbc

#endif // FBC_CV_SRC_ARITHM_CORE_HPP_
