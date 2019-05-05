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

#include "fbcdef.hpp"
#include "interface.hpp"

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

/** norm types
- For one array:
\f[norm =  \forkthree{\|\texttt{src1}\|_{L_{\infty}} =  \max _I | \texttt{src1} (I)|}{if  \(\texttt{normType} = \texttt{NORM_INF}\) }
{ \| \texttt{src1} \| _{L_1} =  \sum _I | \texttt{src1} (I)|}{if  \(\texttt{normType} = \texttt{NORM_L1}\) }
{ \| \texttt{src1} \| _{L_2} =  \sqrt{\sum_I \texttt{src1}(I)^2} }{if  \(\texttt{normType} = \texttt{NORM_L2}\) }\f]

- Absolute norm for two arrays
\f[norm =  \forkthree{\|\texttt{src1}-\texttt{src2}\|_{L_{\infty}} =  \max _I | \texttt{src1} (I) -  \texttt{src2} (I)|}{if  \(\texttt{normType} = \texttt{NORM_INF}\) }
{ \| \texttt{src1} - \texttt{src2} \| _{L_1} =  \sum _I | \texttt{src1} (I) -  \texttt{src2} (I)|}{if  \(\texttt{normType} = \texttt{NORM_L1}\) }
{ \| \texttt{src1} - \texttt{src2} \| _{L_2} =  \sqrt{\sum_I (\texttt{src1}(I) - \texttt{src2}(I))^2} }{if  \(\texttt{normType} = \texttt{NORM_L2}\) }\f]

- Relative norm for two arrays
\f[norm =  \forkthree{\frac{\|\texttt{src1}-\texttt{src2}\|_{L_{\infty}}    }{\|\texttt{src2}\|_{L_{\infty}} }}{if  \(\texttt{normType} = \texttt{NORM_RELATIVE_INF}\) }
{ \frac{\|\texttt{src1}-\texttt{src2}\|_{L_1} }{\|\texttt{src2}\|_{L_1}} }{if  \(\texttt{normType} = \texttt{NORM_RELATIVE_L1}\) }
{ \frac{\|\texttt{src1}-\texttt{src2}\|_{L_2} }{\|\texttt{src2}\|_{L_2}} }{if  \(\texttt{normType} = \texttt{NORM_RELATIVE_L2}\) }\f]

As example for one array consider the function \f$r(x)= \begin{pmatrix} x \\ 1-x \end{pmatrix}, x \in [-1;1]\f$.
The \f$ L_{1}, L_{2} \f$ and \f$ L_{\infty} \f$ norm for the sample value \f$r(-1) = \begin{pmatrix} -1 \\ 2 \end{pmatrix}\f$
is calculated as follows
\f{align*}
\| r(-1) \|_{L_1} &= |-1| + |2| = 3 \\
\| r(-1) \|_{L_2} &= \sqrt{(-1)^{2} + (2)^{2}} = \sqrt{5} \\
\| r(-1) \|_{L_\infty} &= \max(|-1|,|2|) = 2
\f}
and for \f$r(0.5) = \begin{pmatrix} 0.5 \\ 0.5 \end{pmatrix}\f$ the calculation is
\f{align*}
\| r(0.5) \|_{L_1} &= |0.5| + |0.5| = 1 \\
\| r(0.5) \|_{L_2} &= \sqrt{(0.5)^{2} + (0.5)^{2}} = \sqrt{0.5} \\
\| r(0.5) \|_{L_\infty} &= \max(|0.5|,|0.5|) = 0.5.
\f}
The following graphic shows all values for the three norm functions \f$\| r(x) \|_{L_1}, \| r(x) \|_{L_2}\f$ and \f$\| r(x) \|_{L_\infty}\f$.
It is notable that the \f$ L_{1} \f$ norm forms the upper and the \f$ L_{\infty} \f$ norm forms the lower border for the example function \f$ r(x) \f$.
![Graphs for the different norm functions from the above example](pics/NormTypes_OneArray_1-2-INF.png)
*/
enum NormTypes {
	NORM_INF = 1,
	NORM_L1 = 2,
	NORM_L2 = 4,
	NORM_L2SQR = 5,
	NORM_HAMMING = 6,
	NORM_HAMMING2 = 7,
	NORM_TYPE_MASK = 7,
	NORM_RELATIVE = 8, // flag
	NORM_MINMAX = 32 // flag
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

enum DftFlags {
	/** performs an inverse 1D or 2D transform instead of the default forward transform. */
	DFT_INVERSE = 1,
	/** scales the result: divide it by the number of array elements. Normally, it is
	combined with DFT_INVERSE. */
	DFT_SCALE = 2,
	/** performs a forward or inverse transform of every individual row of the input
	matrix; this flag enables you to transform multiple vectors simultaneously and can be used to
	decrease the overhead (which is sometimes several times larger than the processing itself) to
	perform 3D and higher-dimensional transformations and so forth.*/
	DFT_ROWS = 4,
	/** performs a forward transformation of 1D or 2D real array; the result,
	though being a complex array, has complex-conjugate symmetry (*CCS*, see the function
	description below for details), and such an array can be packed into a real array of the same
	size as input, which is the fastest option and which is what the function does by default;
	however, you may wish to get a full complex array (for simpler spectrum analysis, and so on) -
	pass the flag to enable the function to produce a full-size complex output array. */
	DFT_COMPLEX_OUTPUT = 16,
	/** performs an inverse transformation of a 1D or 2D complex array; the
	result is normally a complex array of the same size, however, if the input array has
	conjugate-complex symmetry (for example, it is a result of forward transformation with
	DFT_COMPLEX_OUTPUT flag), the output is a real array; while the function itself does not
	check whether the input is symmetrical or not, you can pass the flag and then the function
	will assume the symmetry and produce the real output array (note that when the input is packed
	into a real array and inverse transformation is executed, the function treats the input as a
	packed complex-conjugate symmetrical array, and the output will also be a real array). */
	DFT_REAL_OUTPUT = 32,
	/** performs an inverse 1D or 2D transform instead of the default forward transform. */
	DCT_INVERSE = DFT_INVERSE,
	/** performs a forward or inverse transform of every individual row of the input
	matrix. This flag enables you to transform multiple vectors simultaneously and can be used to
	decrease the overhead (which is sometimes several times larger than the processing itself) to
	perform 3D and higher-dimensional transforms and so forth.*/
	DCT_ROWS = DFT_ROWS
};

} //fbc

#endif //FBC_CV_CORE_BASE_HPP_
