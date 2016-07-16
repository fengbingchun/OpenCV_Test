// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_BASE_HPP_
#define FBC_CV_CORE_BASE_HPP_

// reference: include/opencv2/core/base.hpp

#ifndef __cplusplus
	#error base.hpp header must be compiled as C++
#endif

#include <assert.h>
#include <algorithm>

#include "core/fbcdef.hpp"
#include "core/interface.hpp"

namespace fbc {

#define FBC_StaticAssert(condition, reason)	static_assert((condition), reason " " #condition)
#define FBC_Assert(expr) assert(expr)
#define FBC_Error(msg) \
	fprintf(stderr, "Error: "#msg", file: %s, func: %s, line: %d \n", __FILE__, __FUNCTION__, __LINE__); \
	assert(0);

/////////////////////////////////// inline norms ////////////////////////////////////
template<typename _Tp> inline _Tp fbc_abs(_Tp x) { return std::abs(x); }
inline int fbc_abs(uchar x) { return x; }
inline int fbc_abs(schar x) { return std::abs(x); }
inline int fbc_abs(ushort x) { return x; }
inline int fbc_abs(short x) { return std::abs(x); }

template<typename _Tp, typename _AccTp> static inline
_AccTp normL2Sqr(const _Tp* a, int n)
{
	_AccTp s = 0;
	int i = 0;

	for (; i <= n - 4; i += 4) {
		_AccTp v0 = a[i], v1 = a[i + 1], v2 = a[i + 2], v3 = a[i + 3];
		s += v0*v0 + v1*v1 + v2*v2 + v3*v3;
	}

	for (; i < n; i++) {
		_AccTp v = a[i];
		s += v*v;
	}

	return s;
}

template<typename _Tp, typename _AccTp> static inline
_AccTp normL1(const _Tp* a, int n)
{
	_AccTp s = 0;
	int i = 0;

	for (; i <= n - 4; i += 4) {
		s += (_AccTp)cv_abs(a[i]) + (_AccTp)cv_abs(a[i + 1]) +
			(_AccTp)cv_abs(a[i + 2]) + (_AccTp)cv_abs(a[i + 3]);
	}

	for (; i < n; i++)
		s += cv_abs(a[i]);

	return s;
}

template<typename _Tp, typename _AccTp> static inline
_AccTp normInf(const _Tp* a, int n)
{
	_AccTp s = 0;
	for (int i = 0; i < n; i++)
		s = std::max(s, (_AccTp)cv_abs(a[i]));

	return s;
}

template<typename _Tp, typename _AccTp> static inline
_AccTp normL2Sqr(const _Tp* a, const _Tp* b, int n)
{
	_AccTp s = 0;
	int i = 0;

	for (; i <= n - 4; i += 4) {
		_AccTp v0 = _AccTp(a[i] - b[i]), v1 = _AccTp(a[i + 1] - b[i + 1]), v2 = _AccTp(a[i + 2] - b[i + 2]), v3 = _AccTp(a[i + 3] - b[i + 3]);
		s += v0*v0 + v1*v1 + v2*v2 + v3*v3;
	}

	for (; i < n; i++) {
		_AccTp v = _AccTp(a[i] - b[i]);
		s += v*v;
	}

	return s;
}

static inline float normL2Sqr(const float* a, const float* b, int n)
{
	float s = 0.f;
	for (int i = 0; i < n; i++) {
		float v = a[i] - b[i];
		s += v*v;
	}

	return s;
}

template<typename _Tp, typename _AccTp> static inline
_AccTp normL1(const _Tp* a, const _Tp* b, int n)
{
	_AccTp s = 0;
	int i = 0;

	for (; i <= n - 4; i += 4) {
		_AccTp v0 = _AccTp(a[i] - b[i]), v1 = _AccTp(a[i + 1] - b[i + 1]), v2 = _AccTp(a[i + 2] - b[i + 2]), v3 = _AccTp(a[i + 3] - b[i + 3]);
		s += std::abs(v0) + std::abs(v1) + std::abs(v2) + std::abs(v3);
	}

	for (; i < n; i++) {
		_AccTp v = _AccTp(a[i] - b[i]);
		s += std::abs(v);
	}

	return s;
}

inline float normL1(const float* a, const float* b, int n)
{
	float s = 0.f;
	for (int i = 0; i < n; i++) {
		s += std::abs(a[i] - b[i]);
	}

	return s;
}

inline int normL1(const uchar* a, const uchar* b, int n)
{
	int s = 0;
	for (int i = 0; i < n; i++) {
		s += std::abs(a[i] - b[i]);
	}

	return s;
}

template<typename _Tp, typename _AccTp> static inline
_AccTp normInf(const _Tp* a, const _Tp* b, int n)
{
	_AccTp s = 0;
	for (int i = 0; i < n; i++) {
		_AccTp v0 = a[i] - b[i];
		s = std::max(s, std::abs(v0));
	}

	return s;
}

//! comparison types
enum CmpTypes {
	CMP_EQ = 0, //!< src1 is equal to src2.
	CMP_GT = 1, //!< src1 is greater than src2.
	CMP_GE = 2, //!< src1 is greater than or equal to src2.
	CMP_LT = 3, //!< src1 is less than src2.
	CMP_LE = 4, //!< src1 is less than or equal to src2.
	CMP_NE = 5  //!< src1 is unequal to src2.
};

//! matrix decomposition types
enum DecompTypes {
	/** Gaussian elimination with the optimal pivot element chosen. */
	DECOMP_LU = 0,
	/** singular value decomposition (SVD) method; the system can be over-defined and/or the matrix
	src1 can be singular */
	DECOMP_SVD = 1,
	/** eigenvalue decomposition; the matrix src1 must be symmetrical */
	DECOMP_EIG = 2,
	/** Cholesky \f$LL^T\f$ factorization; the matrix src1 must be symmetrical and positively
	defined */
	DECOMP_CHOLESKY = 3,
	/** QR factorization; the system can be over-defined and/or the matrix src1 can be singular */
	DECOMP_QR = 4,
	/** while all the previous flags are mutually exclusive, this flag can be used together with
	any of the previous; it means that the normal equations
	\f$\texttt{src1}^T\cdot\texttt{src1}\cdot\texttt{dst}=\texttt{src1}^T\texttt{src2}\f$ are
	solved instead of the original system
	\f$\texttt{src1}\cdot\texttt{dst}=\texttt{src2}\f$ */
	DECOMP_NORMAL = 16
};

//! Various border types, image boundaries are denoted with `|`
enum BorderTypes {
	BORDER_CONSTANT = 0, //!< `iiiiii|abcdefgh|iiiiiii`  with some specified `i`
	BORDER_REPLICATE = 1, //!< `aaaaaa|abcdefgh|hhhhhhh`
	BORDER_REFLECT = 2, //!< `fedcba|abcdefgh|hgfedcb`
	BORDER_WRAP = 3, //!< `cdefgh|abcdefgh|abcdefg`
	BORDER_REFLECT_101 = 4, //!< `gfedcb|abcdefgh|gfedcba`
	BORDER_TRANSPARENT = 5, //!< `uvwxyz|absdefgh|ijklmno`

	BORDER_REFLECT101 = BORDER_REFLECT_101, //!< same as BORDER_REFLECT_101
	BORDER_DEFAULT = BORDER_REFLECT_101, //!< same as BORDER_REFLECT_101
	BORDER_ISOLATED = 16 //!< do not look outside of ROI
};

} //fbc

#endif //FBC_CV_CORE_BASE_HPP_
