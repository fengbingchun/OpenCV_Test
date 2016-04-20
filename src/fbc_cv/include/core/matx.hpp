#ifndef FBC_CV_CORE_MATX_HPP_
#define FBC_CV_CORE_MATX_HPP_

// reference: include/opencv2/core/matx.hpp

#include "core/fbcdef.hpp"
#include "core/base.hpp"

namespace fbc {
////////////////////////////// Small Matrix ///////////////////////////
template<typename _Tp, int m, int n> class Matx
{
public:
	enum {
		rows = m,
		cols = n,
		channels = rows*cols,
		shortdim = (m < n ? m : n)
	};

	typedef _Tp value_type;
	typedef Matx<_Tp, m, n> mat_type;
	typedef Matx<_Tp, shortdim, 1> diag_type;

	//! default constructor
	Matx();

	Matx(_Tp v0); //!< 1x1 matrix
	Matx(_Tp v0, _Tp v1); //!< 1x2 or 2x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2); //!< 1x3 or 3x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 1x4, 2x2 or 4x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4); //!< 1x5 or 5x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5); //!< 1x6, 2x3, 3x2 or 6x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6); //!< 1x7 or 7x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7); //!< 1x8, 2x4, 4x2 or 8x1 matrix
	Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8); //!< 1x9, 3x3 or 9x1 matrix
	explicit Matx(const _Tp* vals); //!< initialize from a plain array

	static Matx all(_Tp alpha);
	static Matx zeros();
	static Matx ones();
	static Matx eye();
	static Matx diag(const diag_type& d);

	//! dot product computed with the default precision
	_Tp dot(const Matx<_Tp, m, n>& v) const;

	//! dot product computed in double-precision arithmetics
	double ddot(const Matx<_Tp, m, n>& v) const;

	//! conversion to another data type
	template<typename T2> operator Matx<T2, m, n>() const;

	//! change the matrix shape
	template<int m1, int n1> Matx<_Tp, m1, n1> reshape() const;

	//! extract part of the matrix
	template<int m1, int n1> Matx<_Tp, m1, n1> get_minor(int i, int j) const;

	//! extract the matrix row
	Matx<_Tp, 1, n> row(int i) const;

	//! extract the matrix column
	Matx<_Tp, m, 1> col(int i) const;

	//! extract the matrix diagonal
	diag_type diag() const;

	//! element access
	const _Tp& operator ()(int i, int j) const;
	_Tp& operator ()(int i, int j);

	//! 1D element access
	const _Tp& operator ()(int i) const;
	_Tp& operator ()(int i);

	_Tp val[m*n]; //< matrix elements
};

typedef Matx<float, 1, 2> Matx12f;
typedef Matx<double, 1, 2> Matx12d;
typedef Matx<float, 1, 3> Matx13f;
typedef Matx<double, 1, 3> Matx13d;
typedef Matx<float, 1, 4> Matx14f;
typedef Matx<double, 1, 4> Matx14d;
typedef Matx<float, 1, 6> Matx16f;
typedef Matx<double, 1, 6> Matx16d;

typedef Matx<float, 2, 1> Matx21f;
typedef Matx<double, 2, 1> Matx21d;
typedef Matx<float, 3, 1> Matx31f;
typedef Matx<double, 3, 1> Matx31d;
typedef Matx<float, 4, 1> Matx41f;
typedef Matx<double, 4, 1> Matx41d;
typedef Matx<float, 6, 1> Matx61f;
typedef Matx<double, 6, 1> Matx61d;

typedef Matx<float, 2, 2> Matx22f;
typedef Matx<double, 2, 2> Matx22d;
typedef Matx<float, 2, 3> Matx23f;
typedef Matx<double, 2, 3> Matx23d;
typedef Matx<float, 3, 2> Matx32f;
typedef Matx<double, 3, 2> Matx32d;

typedef Matx<float, 3, 3> Matx33f;
typedef Matx<double, 3, 3> Matx33d;

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx()
{
	for (int i = 0; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0)
{
	val[0] = v0;
	for (int i = 1; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1)
{
	FBC_StaticAssert(channels >= 2, "Matx should have at least 2 elements.");
	val[0] = v0; val[1] = v1;
	for (int i = 2; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2)
{
	FBC_StaticAssert(channels >= 3, "Matx should have at least 3 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2;
	for (int i = 3; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3)
{
	FBC_StaticAssert(channels >= 4, "Matx should have at least 4 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
	for (int i = 4; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4)
{
	FBC_StaticAssert(channels >= 5, "Matx should have at least 5 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3; val[4] = v4;
	for (int i = 5; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5)
{
	FBC_StaticAssert(channels >= 6, "Matx should have at least 6 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
	val[4] = v4; val[5] = v5;
	for (int i = 6; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6)
{
	FBC_StaticAssert(channels >= 7, "Matx should have at least 7 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
	val[4] = v4; val[5] = v5; val[6] = v6;
	for (int i = 7; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7)
{
	FBC_StaticAssert(channels >= 8, "Matx should have at least 8 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
	val[4] = v4; val[5] = v5; val[6] = v6; val[7] = v7;
	for (int i = 8; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8)
{
	FBC_StaticAssert(channels >= 9, "Matx should have at least 9 elements.");
	val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
	val[4] = v4; val[5] = v5; val[6] = v6; val[7] = v7;
	val[8] = v8;
	for (int i = 9; i < channels; i++) val[i] = _Tp(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n>::Matx(const _Tp* values)
{
	for (int i = 0; i < channels; i++) val[i] = values[i];
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n> Matx<_Tp, m, n>::all(_Tp alpha)
{
	Matx<_Tp, m, n> M;
	for (int i = 0; i < m*n; i++) M.val[i] = alpha;
	return M;
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n> Matx<_Tp, m, n>::zeros()
{
	return all(0);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n> Matx<_Tp, m, n>::ones()
{
	return all(1);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n> Matx<_Tp, m, n>::eye()
{
	Matx<_Tp, m, n> M;
	for (int i = 0; i < shortdim; i++)
		M(i, i) = 1;
	return M;
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, n> Matx<_Tp, m, n>::diag(const typename Matx<_Tp, m, n>::diag_type& d)
{
	Matx<_Tp, m, n> M;
	for (int i = 0; i < shortdim; i++)
		M(i, i) = d(i, 0);
	return M;
}

template<typename _Tp, int m, int n> inline
_Tp Matx<_Tp, m, n>::dot(const Matx<_Tp, m, n>& M) const
{
	_Tp s = 0;
	for (int i = 0; i < channels; i++) s += val[i] * M.val[i];
	return s;
}

template<typename _Tp, int m, int n> inline
double Matx<_Tp, m, n>::ddot(const Matx<_Tp, m, n>& M) const
{
	double s = 0;
	for (int i = 0; i < channels; i++) s += (double)val[i] * M.val[i];
	return s;
}

template<typename _Tp, int m, int n> template<typename T2>
inline Matx<_Tp, m, n>::operator Matx<T2, m, n>() const
{
	Matx<T2, m, n> M;
	for (int i = 0; i < m*n; i++) M.val[i] = saturate_cast<T2>(val[i]);
	return M;
}

template<typename _Tp, int m, int n> template<int m1, int n1> inline
Matx<_Tp, m1, n1> Matx<_Tp, m, n>::reshape() const
{
	FBC_StaticAssert(m1*n1 == m*n, "Input and destnarion matrices must have the same number of elements");
	return (const Matx<_Tp, m1, n1>&)*this;
}

template<typename _Tp, int m, int n>
template<int m1, int n1> inline
Matx<_Tp, m1, n1> Matx<_Tp, m, n>::get_minor(int i, int j) const
{
	FBC_Assert(0 <= i && i + m1 <= m && 0 <= j && j + n1 <= n);
	Matx<_Tp, m1, n1> s;
	for (int di = 0; di < m1; di++)
		for (int dj = 0; dj < n1; dj++)
			s(di, dj) = (*this)(i + di, j + dj);
	return s;
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, 1, n> Matx<_Tp, m, n>::row(int i) const
{
	FBC_Assert((unsigned)i < (unsigned)m);
	return Matx<_Tp, 1, n>(&val[i*n]);
}

template<typename _Tp, int m, int n> inline
Matx<_Tp, m, 1> Matx<_Tp, m, n>::col(int j) const
{
	FBC_Assert((unsigned)j < (unsigned)n);
	Matx<_Tp, m, 1> v;
	for (int i = 0; i < m; i++)
		v.val[i] = val[i*n + j];
	return v;
}

template<typename _Tp, int m, int n> inline
typename Matx<_Tp, m, n>::diag_type Matx<_Tp, m, n>::diag() const
{
	diag_type d;
	for (int i = 0; i < shortdim; i++)
		d.val[i] = val[i*n + i];
	return d;
}

template<typename _Tp, int m, int n> inline
const _Tp& Matx<_Tp, m, n>::operator()(int i, int j) const
{
	FBC_Assert((unsigned)i < (unsigned)m && (unsigned)j < (unsigned)n);
	return this->val[i*n + j];
}

template<typename _Tp, int m, int n> inline
_Tp& Matx<_Tp, m, n>::operator ()(int i, int j)
{
	FBC_Assert((unsigned)i < (unsigned)m && (unsigned)j < (unsigned)n);
	return val[i*n + j];
}

template<typename _Tp, int m, int n> inline
const _Tp& Matx<_Tp, m, n>::operator ()(int i) const
{
	FBC_StaticAssert(m == 1 || n == 1, "Single index indexation requires matrix to be a column or a row");
	FBC_Assert((unsigned)i < (unsigned)(m + n - 1));
	return val[i];
}

template<typename _Tp, int m, int n> inline
_Tp& Matx<_Tp, m, n>::operator ()(int i)
{
	FBC_StaticAssert(m == 1 || n == 1, "Single index indexation requires matrix to be a column or a row");
	FBC_Assert((unsigned)i < (unsigned)(m + n - 1));
	return val[i];
}

///////////////////////////////////////// Vec ///////////////////////////////////
//template<typename _Tp, int cn> class Vec {
//public:
//	typedef _Tp value_type;
//	enum {
//		channels = cn,
//	};
//
//	//! default constructor
//	Vec();
//
//	Vec(_Tp v0); //!< 1-element vector constructor
//	Vec(_Tp v0, _Tp v1); //!< 2-element vector constructor
//	Vec(_Tp v0, _Tp v1, _Tp v2); //!< 3-element vector constructor
//	Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 4-element vector constructor
//	explicit Vec(const _Tp* values);
//
//	Vec(const Vec<_Tp, cn>& v);
//
//	static Vec all(_Tp alpha);
//
//	//! per-element multiplication
//	Vec mul(const Vec<_Tp, cn>& v) const;
//
//	//! conjugation (makes sense for complex numbers and quaternions)
//	Vec conj() const;
//
//	/*!
//	cross product of the two 3D vectors.
//	For other dimensionalities the exception is raised
//	*/
//	Vec cross(const Vec& v) const;
//	//! conversion to another data type
//	template<typename T2> operator Vec<T2, cn>() const;
//
//	/*! element access */
//	const _Tp& operator [](int i) const;
//	_Tp& operator[](int i);
//	const _Tp& operator ()(int i) const;
//	_Tp& operator ()(int i);
//};
//
///** @name Shorter aliases for the most popular specializations of Vec<T,n>
//@{
//*/
//typedef Vec<uchar, 2> Vec2b;
//typedef Vec<uchar, 3> Vec3b;
//typedef Vec<uchar, 4> Vec4b;
//
//typedef Vec<short, 2> Vec2s;
//typedef Vec<short, 3> Vec3s;
//typedef Vec<short, 4> Vec4s;
//
//typedef Vec<ushort, 2> Vec2w;
//typedef Vec<ushort, 3> Vec3w;
//typedef Vec<ushort, 4> Vec4w;
//
//typedef Vec<int, 2> Vec2i;
//typedef Vec<int, 3> Vec3i;
//typedef Vec<int, 4> Vec4i;
//
//typedef Vec<float, 2> Vec2f;
//typedef Vec<float, 3> Vec3f;
//typedef Vec<float, 4> Vec4f;
//
//typedef Vec<double, 2> Vec2d;
//typedef Vec<double, 3> Vec3d;
//typedef Vec<double, 4> Vec4d;
///** @} */
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec() {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0)
//: Matx<_Tp, cn, 1>(v0) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1)
//: Matx<_Tp, cn, 1>(v0, v1) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2)
//: Matx<_Tp, cn, 1>(v0, v1, v2) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4, v5) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4, v5, v6) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4, v5, v6, v7) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4, v5, v6, v7, v8) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4, _Tp v5, _Tp v6, _Tp v7, _Tp v8, _Tp v9, _Tp v10, _Tp v11, _Tp v12, _Tp v13)
//: Matx<_Tp, cn, 1>(v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(const _Tp* values)
//: Matx<_Tp, cn, 1>(values) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(const Vec<_Tp, cn>& m)
//: Matx<_Tp, cn, 1>(m.val) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(const Matx<_Tp, cn, 1>& a, const Matx<_Tp, cn, 1>& b, Matx_AddOp op)
//: Matx<_Tp, cn, 1>(a, b, op) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn>::Vec(const Matx<_Tp, cn, 1>& a, const Matx<_Tp, cn, 1>& b, Matx_SubOp op)
//: Matx<_Tp, cn, 1>(a, b, op) {}
//
//template<typename _Tp, int cn> template<typename _T2> inline
//Vec<_Tp, cn>::Vec(const Matx<_Tp, cn, 1>& a, _T2 alpha, Matx_ScaleOp op)
//: Matx<_Tp, cn, 1>(a, alpha, op) {}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn> Vec<_Tp, cn>::all(_Tp alpha)
//{
//	Vec v;
//	for (int i = 0; i < cn; i++) v.val[i] = alpha;
//	return v;
//}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn> Vec<_Tp, cn>::mul(const Vec<_Tp, cn>& v) const
//{
//	Vec<_Tp, cn> w;
//	for (int i = 0; i < cn; i++) w.val[i] = saturate_cast<_Tp>(this->val[i] * v.val[i]);
//	return w;
//}
//
//template<> inline
//Vec<float, 2> Vec<float, 2>::conj() const
//{
//	return cv::internal::conjugate(*this);
//}
//
//template<> inline
//Vec<double, 2> Vec<double, 2>::conj() const
//{
//	return cv::internal::conjugate(*this);
//}
//
//template<> inline
//Vec<float, 4> Vec<float, 4>::conj() const
//{
//	return cv::internal::conjugate(*this);
//}
//
//template<> inline
//Vec<double, 4> Vec<double, 4>::conj() const
//{
//	return cv::internal::conjugate(*this);
//}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn> Vec<_Tp, cn>::cross(const Vec<_Tp, cn>&) const
//{
//	CV_StaticAssert(cn == 3, "for arbitrary-size vector there is no cross-product defined");
//	return Vec<_Tp, cn>();
//}
//
//template<> inline
//Vec<float, 3> Vec<float, 3>::cross(const Vec<float, 3>& v) const
//{
//	return Vec<float, 3>(val[1] * v.val[2] - val[2] * v.val[1],
//		val[2] * v.val[0] - val[0] * v.val[2],
//		val[0] * v.val[1] - val[1] * v.val[0]);
//}
//
//template<> inline
//Vec<double, 3> Vec<double, 3>::cross(const Vec<double, 3>& v) const
//{
//	return Vec<double, 3>(val[1] * v.val[2] - val[2] * v.val[1],
//		val[2] * v.val[0] - val[0] * v.val[2],
//		val[0] * v.val[1] - val[1] * v.val[0]);
//}
//
//template<typename _Tp, int cn> template<typename T2> inline
//Vec<_Tp, cn>::operator Vec<T2, cn>() const
//{
//	Vec<T2, cn> v;
//	for (int i = 0; i < cn; i++) v.val[i] = saturate_cast<T2>(this->val[i]);
//	return v;
//}
//
//template<typename _Tp, int cn> inline
//const _Tp& Vec<_Tp, cn>::operator [](int i) const
//{
//	CV_DbgAssert((unsigned)i < (unsigned)cn);
//	return this->val[i];
//}
//
//template<typename _Tp, int cn> inline
//_Tp& Vec<_Tp, cn>::operator [](int i)
//{
//	CV_DbgAssert((unsigned)i < (unsigned)cn);
//	return this->val[i];
//}
//
//template<typename _Tp, int cn> inline
//const _Tp& Vec<_Tp, cn>::operator ()(int i) const
//{
//	CV_DbgAssert((unsigned)i < (unsigned)cn);
//	return this->val[i];
//}
//
//template<typename _Tp, int cn> inline
//_Tp& Vec<_Tp, cn>::operator ()(int i)
//{
//	CV_DbgAssert((unsigned)i < (unsigned)cn);
//	return this->val[i];
//}
//
//template<typename _Tp, int cn> inline
//Vec<_Tp, cn> normalize(const Vec<_Tp, cn>& v)
//{
//	double nv = norm(v);
//	return v * (nv ? 1. / nv : 0.);
//}
} //fbc
#endif //FBC_CV_CORE_MATX_HPP_
