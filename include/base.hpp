#ifndef FBC_OPENCV_TEST_BASE_HPP_
#define FBC_OPENCV_TEST_BASE_HPP_

#include <algorithm>

namespace fbc {

// reference: opencv/include/opencv2/core/types.hpp
template<typename _Tp, int cn> class Vec;
template<typename _Tp> class Point_;
template<typename _Tp> class Size_;
template<typename _Tp> class Point3_;
template<typename _Tp> class Rect_;
template<typename _Tp> class Scalar_;

// Image_
template<typename _Tp, int cn>
struct Image_ {
	typedef _Tp value_type;
	enum {
		channels = cn;
	};

	_Tp* data;
	int width;
	int height;
};

typedef Image_<unsigned char, 1> ImageGray;
typedef Image_<unsigned char, 3> ImageBGR;
typedef Image_<unsigned char, 4> ImageBGRA;
typedef Image_<float, 1> ImageFloat;
typedef Image_<float, 3> ImageFloat3;
typedef Image_<float, 4> ImageFloat4;

// Vec
template<typename _Tp, int cn> class Vec {
public:
	typedef _Tp value_type;
	enum {
		channels = cn,
	};

	//! default constructor
	Vec();

	Vec(_Tp v0); //!< 1-element vector constructor
	Vec(_Tp v0, _Tp v1); //!< 2-element vector constructor
	Vec(_Tp v0, _Tp v1, _Tp v2); //!< 3-element vector constructor
	Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3); //!< 4-element vector constructor
	Vec(_Tp v0, _Tp v1, _Tp v2, _Tp v3, _Tp v4); //!< 5-element vector constructor

	explicit Vec(const _Tp* values);

	Vec(const Vec<_Tp, cn>& v);

	static Vec all(_Tp alpha);

	//! per-element multiplication
	Vec mul(const Vec<_Tp, cn>& v) const;

	//! conjugation (makes sense for complex numbers and quaternions)
	Vec conj() const;

	/*!
	cross product of the two 3D vectors.

	For other dimensionalities the exception is raised
	*/
	Vec cross(const Vec& v) const;
	//! conversion to another data type
	template<typename T2> operator Vec<T2, cn>() const;

	/*! element access */
	const _Tp& operator [](int i) const;
	_Tp& operator[](int i);
	const _Tp& operator ()(int i) const;
	_Tp& operator ()(int i);
};

// Size_
template<typename _Tp> class Size_ {
public:
	typedef _Tp value_type;

	//! various constructors
	Size_();
	Size_(_Tp _width, _Tp _height);
	Size_(const Size_& sz);
	Size_(const Point_<_Tp>& pt);

	Size_& operator = (const Size_& sz);
	//! the area (width*height)
	_Tp area() const;

	//! conversion of another data type.
	template<typename _Tp2> operator Size_<_Tp2>() const;

	_Tp width, height; // the width and the height
};

typedef Size_<int> Size2i;
typedef Size_<float> Size2f;
typedef Size_<double> Size2d;
typedef Size2i Size;

// Point_
template<typename _Tp> class Point_ {
public:
	typedef _Tp value_type;

	// various constructors
	Point_();
	Point_(_Tp _x, _Tp _y);
	Point_(const Point_& pt);
	Point_(const Size_<_Tp>& sz);
	Point_(const Vec<_Tp, 2>& v);

	Point_& operator = (const Point_& pt);
	//! conversion to another data type
	template<typename _Tp2> operator Point_<_Tp2>() const;

	//! conversion to the old-style C structures
	operator Vec<_Tp, 2>() const;

	//! dot product
	_Tp dot(const Point_& pt) const;
	//! dot product computed in double-precision arithmetics
	double ddot(const Point_& pt) const;
	//! cross-product
	double cross(const Point_& pt) const;
	//! checks whether the point is inside the specified rectangle
	bool inside(const Rect_<_Tp>& r) const;

	_Tp x, y; //< the point coordinates
};

typedef Point_<int> Point2i;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Point2i Point;

// Point3_
template<typename _Tp> class Point3_ {
public:
	typedef _Tp value_type;

	// various constructors
	Point3_();
	Point3_(_Tp _x, _Tp _y, _Tp _z);
	Point3_(const Point3_& pt);
	explicit Point3_(const Point_<_Tp>& pt);
	Point3_(const Vec<_Tp, 3>& v);

	Point3_& operator = (const Point3_& pt);
	//! conversion to another data type
	template<typename _Tp2> operator Point3_<_Tp2>() const;
	//! conversion to cv::Vec<>
	operator Vec<_Tp, 3>() const;

	//! dot product
	_Tp dot(const Point3_& pt) const;
	//! dot product computed in double-precision arithmetics
	double ddot(const Point3_& pt) const;
	//! cross product of the 2 3D points
	Point3_ cross(const Point3_& pt) const;

	_Tp x, y, z; //< the point coordinates
};

typedef Point3_<int> Point3i;
typedef Point3_<float> Point3f;
typedef Point3_<double> Point3d;

// Rect_
template<typename _Tp> class Rect_ {
public:
	typedef _Tp value_type;

	//! various constructors
	Rect_();
	Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
	Rect_(const Rect_& r);
	Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz);
	Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2);

	Rect_& operator = (const Rect_& r);
	//! the top-left corner
	Point_<_Tp> tl() const;
	//! the bottom-right corner
	Point_<_Tp> br() const;

	//! size (width, height) of the rectangle
	Size_<_Tp> size() const;
	//! area (width*height) of the rectangle
	_Tp area() const;

	//! conversion to another data type
	template<typename _Tp2> operator Rect_<_Tp2>() const;

	//! checks whether the rectangle contains the point
	bool contains(const Point_<_Tp>& pt) const;

	_Tp x, y, width, height; //< the top-left corner, as well as width and height of the rectangle
};

typedef Rect_<int> Rect2i;
typedef Rect_<float> Rect2f;
typedef Rect_<double> Rect2d;
typedef Rect2i Rect;

// Range
class Range {
public:
	Range();
	Range(int _start, int _end);
	int size() const;
	bool empty() const;
	static Range all();

	int start, end;
};

// Scalar
template<typename _Tp> class Scalar_ : public Vec<_Tp, 4>
{
public:
	//! various constructors
	Scalar_();
	Scalar_(_Tp v0, _Tp v1, _Tp v2 = 0, _Tp v3 = 0);
	Scalar_(_Tp v0);

	template<typename _Tp2, int cn>
	Scalar_(const Vec<_Tp2, cn>& v);

	//! returns a scalar with all elements set to v0
	static Scalar_<_Tp> all(_Tp v0);

	//! conversion to another data type
	template<typename T2> operator Scalar_<T2>() const;

	//! per-element product
	Scalar_<_Tp> mul(const Scalar_<_Tp>& a, double scale = 1) const;

	// returns (v0, -v1, -v2, -v3)
	Scalar_<_Tp> conj() const;

	// returns true iff v1 == v2 == v3 == 0
	bool isReal() const;
};

typedef Scalar_<double> Scalar;

//////////////// Implementation //////////////////
// Point_
template<typename _Tp> inline
Point_<_Tp>::Point_()
: x(0), y(0) {}

template<typename _Tp> inline
Point_<_Tp>::Point_(_Tp _x, _Tp _y)
: x(_x), y(_y) {}

template<typename _Tp> inline
Point_<_Tp>::Point_(const Point_& pt)
: x(pt.x), y(pt.y) {}

template<typename _Tp> inline
Point_<_Tp>::Point_(const Size_<_Tp>& sz)
: x(sz.width), y(sz.height) {}

template<typename _Tp> inline
Point_<_Tp>::Point_(const Vec<_Tp, 2>& v)
: x(v[0]), y(v[1]) {}

template<typename _Tp> inline
Point_<_Tp>& Point_<_Tp>::operator = (const Point_& pt)
{
	x = pt.x; y = pt.y;
	return *this;
}

template<typename _Tp> template<typename _Tp2> inline
Point_<_Tp>::operator Point_<_Tp2>() const
{
	return Point_<_Tp2>(static_cast<_Tp2>(x), static_cast<_Tp2>(y));
}

template<typename _Tp> inline
Point_<_Tp>::operator Vec<_Tp, 2>() const
{
	return Vec<_Tp, 2>(x, y);
}

template<typename _Tp> inline
_Tp Point_<_Tp>::dot(const Point_& pt) const
{
	return static_cast<_Tp>(x*pt.x + y*pt.y);
}

template<typename _Tp> inline
double Point_<_Tp>::ddot(const Point_& pt) const
{
	return (double)x*pt.x + (double)y*pt.y;
}

template<typename _Tp> inline
double Point_<_Tp>::cross(const Point_& pt) const
{
	return (double)x*pt.y - (double)y*pt.x;
}

template<typename _Tp> inline bool
Point_<_Tp>::inside(const Rect_<_Tp>& r) const
{
	return r.contains(*this);
}


template<typename _Tp> static inline
Point_<_Tp>& operator += (Point_<_Tp>& a, const Point_<_Tp>& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator -= (Point_<_Tp>& a, const Point_<_Tp>& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator *= (Point_<_Tp>& a, int b)
{
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator *= (Point_<_Tp>& a, float b)
{
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator *= (Point_<_Tp>& a, double b)
{
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator /= (Point_<_Tp>& a, int b)
{
	a.x = static_cast<_Tp>(a.x / b);
	a.y = static_cast<_Tp>(a.y / b);
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator /= (Point_<_Tp>& a, float b)
{
	a.x = static_cast<_Tp>(a.x / b);
	a.y = static_cast<_Tp>(a.y / b);
	return a;
}

template<typename _Tp> static inline
Point_<_Tp>& operator /= (Point_<_Tp>& a, double b)
{
	a.x = static_cast<_Tp>(a.x / b);
	a.y = static_cast<_Tp>(a.y / b);
	return a;
}

template<typename _Tp> static inline
double norm(const Point_<_Tp>& pt)
{
	return std::sqrt((double)pt.x*pt.x + (double)pt.y*pt.y);
}

template<typename _Tp> static inline
bool operator == (const Point_<_Tp>& a, const Point_<_Tp>& b)
{
	return a.x == b.x && a.y == b.y;
}

template<typename _Tp> static inline
bool operator != (const Point_<_Tp>& a, const Point_<_Tp>& b)
{
	return a.x != b.x || a.y != b.y;
}

template<typename _Tp> static inline
Point_<_Tp> operator + (const Point_<_Tp>& a, const Point_<_Tp>& b)
{
	return Point_<_Tp>(static_cast<_Tp>(a.x + b.x), static_cast<_Tp>(a.y + b.y));
}

template<typename _Tp> static inline
Point_<_Tp> operator - (const Point_<_Tp>& a, const Point_<_Tp>& b)
{
	return Point_<_Tp>(static_cast<_Tp>(a.x - b.x), static_cast<_Tp>(a.y - b.y));
}

template<typename _Tp> static inline
Point_<_Tp> operator - (const Point_<_Tp>& a)
{
	return Point_<_Tp>(static_cast<_Tp>(-a.x), static_cast<_Tp>(-a.y));
}

template<typename _Tp> static inline
Point_<_Tp> operator * (const Point_<_Tp>& a, int b)
{
	return Point_<_Tp>(static_cast<_Tp>(a.x*b), static_cast<_Tp>(a.y*b));
}

template<typename _Tp> static inline
Point_<_Tp> operator * (int a, const Point_<_Tp>& b)
{
	return Point_<_Tp>(static_cast<_Tp>(b.x*a), static_cast<_Tp>(b.y*a));
}

template<typename _Tp> static inline
Point_<_Tp> operator * (const Point_<_Tp>& a, float b)
{
	return Point_<_Tp>(static_cast<_Tp>(a.x*b), static_cast<_Tp>(a.y*b));
}

template<typename _Tp> static inline
Point_<_Tp> operator * (float a, const Point_<_Tp>& b)
{
	return Point_<_Tp>(static_cast<_Tp>(b.x*a), static_cast<_Tp>(b.y*a));
}

template<typename _Tp> static inline
Point_<_Tp> operator * (const Point_<_Tp>& a, double b)
{
	return Point_<_Tp>(static_cast<_Tp>(a.x*b), static_cast<_Tp>(a.y*b));
}

template<typename _Tp> static inline
Point_<_Tp> operator * (double a, const Point_<_Tp>& b)
{
	return Point_<_Tp>(static_cast<_Tp>(b.x*a), static_cast<_Tp>(b.y*a));
}

template<typename _Tp> static inline
Point_<_Tp> operator / (const Point_<_Tp>& a, int b)
{
	Point_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

template<typename _Tp> static inline
Point_<_Tp> operator / (const Point_<_Tp>& a, float b)
{
	Point_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

template<typename _Tp> static inline
Point_<_Tp> operator / (const Point_<_Tp>& a, double b)
{
	Point_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

// Point3_
template<typename _Tp> inline
Point3_<_Tp>::Point3_()
: x(0), y(0), z(0) {}

template<typename _Tp> inline
Point3_<_Tp>::Point3_(_Tp _x, _Tp _y, _Tp _z)
: x(_x), y(_y), z(_z) {}

template<typename _Tp> inline
Point3_<_Tp>::Point3_(const Point3_& pt)
: x(pt.x), y(pt.y), z(pt.z) {}

template<typename _Tp> inline
Point3_<_Tp>::Point3_(const Point_<_Tp>& pt)
: x(pt.x), y(pt.y), z(_Tp()) {}

template<typename _Tp> inline
Point3_<_Tp>::Point3_(const Vec<_Tp, 3>& v)
: x(v[0]), y(v[1]), z(v[2]) {}

template<typename _Tp> template<typename _Tp2> inline
Point3_<_Tp>::operator Point3_<_Tp2>() const
{
	return Point3_<_Tp2>(static_cast<_Tp2>(x), static_cast<_Tp2>(y), static_cast<_Tp2>(z));
}

template<typename _Tp> inline
Point3_<_Tp>::operator Vec<_Tp, 3>() const
{
	return Vec<_Tp, 3>(x, y, z);
}

template<typename _Tp> inline
Point3_<_Tp>& Point3_<_Tp>::operator = (const Point3_& pt)
{
	x = pt.x; y = pt.y; z = pt.z;
	return *this;
}

template<typename _Tp> inline
_Tp Point3_<_Tp>::dot(const Point3_& pt) const
{
	return static_cast<_Tp>(x*pt.x + y*pt.y + z*pt.z);
}

template<typename _Tp> inline
double Point3_<_Tp>::ddot(const Point3_& pt) const
{
	return (double)x*pt.x + (double)y*pt.y + (double)z*pt.z;
}

template<typename _Tp> inline
Point3_<_Tp> Point3_<_Tp>::cross(const Point3_<_Tp>& pt) const
{
	return Point3_<_Tp>(y*pt.z - z*pt.y, z*pt.x - x*pt.z, x*pt.y - y*pt.x);
}


template<typename _Tp> static inline
Point3_<_Tp>& operator += (Point3_<_Tp>& a, const Point3_<_Tp>& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator -= (Point3_<_Tp>& a, const Point3_<_Tp>& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator *= (Point3_<_Tp>& a, int b)
{
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	a.z = static_cast<_Tp>(a.z * b);
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator *= (Point3_<_Tp>& a, float b)
{
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	a.z = static_cast<_Tp>(a.z * b);
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator *= (Point3_<_Tp>& a, double b)
{
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	a.z = static_cast<_Tp>(a.z * b);
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator /= (Point3_<_Tp>& a, int b)
{
	a.x = static_cast<_Tp>(a.x / b);
	a.y = static_cast<_Tp>(a.y / b);
	a.z = static_cast<_Tp>(a.z / b);
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator /= (Point3_<_Tp>& a, float b)
{
	a.x = static_cast<_Tp>(a.x / b);
	a.y = static_cast<_Tp>(a.y / b);
	a.z = static_cast<_Tp>(a.z / b);
	return a;
}

template<typename _Tp> static inline
Point3_<_Tp>& operator /= (Point3_<_Tp>& a, double b)
{
	a.x = static_cast<_Tp>(a.x / b);
	a.y = static_cast<_Tp>(a.y / b);
	a.z = static_cast<_Tp>(a.z / b);
	return a;
}

template<typename _Tp> static inline
double norm(const Point3_<_Tp>& pt)
{
	return std::sqrt((double)pt.x*pt.x + (double)pt.y*pt.y + (double)pt.z*pt.z);
}

template<typename _Tp> static inline
bool operator == (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<typename _Tp> static inline
bool operator != (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

template<typename _Tp> static inline
Point3_<_Tp> operator + (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{
	return Point3_<_Tp>(static_cast<_Tp>(a.x + b.x), static_cast<_Tp>(a.y + b.y), static_cast<_Tp>(a.z + b.z));
}

template<typename _Tp> static inline
Point3_<_Tp> operator - (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{
	return Point3_<_Tp>(static_cast<_Tp>(a.x - b.x), static_cast<_Tp>(a.y - b.y), static_cast<_Tp>(a.z - b.z));
}

template<typename _Tp> static inline
Point3_<_Tp> operator - (const Point3_<_Tp>& a)
{
	return Point3_<_Tp>(static_cast<_Tp>(-a.x), static_cast<_Tp>(-a.y), static_cast<_Tp>(-a.z));
}

template<typename _Tp> static inline
Point3_<_Tp> operator * (const Point3_<_Tp>& a, int b)
{
	return Point3_<_Tp>(static_cast<_Tp>(a.x*b), static_cast<_Tp>(a.y*b), static_cast<_Tp>(a.z*b));
}

template<typename _Tp> static inline
Point3_<_Tp> operator * (int a, const Point3_<_Tp>& b)
{
	return Point3_<_Tp>(static_cast<_Tp>(b.x * a), static_cast<_Tp>(b.y * a), static_cast<_Tp>(b.z * a));
}

template<typename _Tp> static inline
Point3_<_Tp> operator * (const Point3_<_Tp>& a, float b)
{
	return Point3_<_Tp>(static_cast<_Tp>(a.x * b), static_cast<_Tp>(a.y * b), static_cast<_Tp>(a.z * b));
}

template<typename _Tp> static inline
Point3_<_Tp> operator * (float a, const Point3_<_Tp>& b)
{
	return Point3_<_Tp>(static_cast<_Tp>(b.x * a), static_cast<_Tp>(b.y * a), static_cast<_Tp>(b.z * a));
}

template<typename _Tp> static inline
Point3_<_Tp> operator * (const Point3_<_Tp>& a, double b)
{
	return Point3_<_Tp>(static_cast<_Tp>(a.x * b), static_cast<_Tp>(a.y * b), static_cast<_Tp>(a.z * b));
}

template<typename _Tp> static inline
Point3_<_Tp> operator * (double a, const Point3_<_Tp>& b)
{
	return Point3_<_Tp>(static_cast<_Tp>(b.x * a), static_cast<_Tp>(b.y * a), static_cast<_Tp>(b.z * a));
}

template<typename _Tp> static inline
Point3_<_Tp> operator / (const Point3_<_Tp>& a, int b)
{
	Point3_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

template<typename _Tp> static inline
Point3_<_Tp> operator / (const Point3_<_Tp>& a, float b)
{
	Point3_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

template<typename _Tp> static inline
Point3_<_Tp> operator / (const Point3_<_Tp>& a, double b)
{
	Point3_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

// Size_
template<typename _Tp> inline
Size_<_Tp>::Size_()
: width(0), height(0) {}

template<typename _Tp> inline
Size_<_Tp>::Size_(_Tp _width, _Tp _height)
: width(_width), height(_height) {}

template<typename _Tp> inline
Size_<_Tp>::Size_(const Size_& sz)
: width(sz.width), height(sz.height) {}

template<typename _Tp> inline
Size_<_Tp>::Size_(const Point_<_Tp>& pt)
: width(pt.x), height(pt.y) {}

template<typename _Tp> template<typename _Tp2> inline
Size_<_Tp>::operator Size_<_Tp2>() const
{
	return Size_<_Tp2>(static_cast<_Tp2>(width), static_cast<_Tp2>(height));
}

template<typename _Tp> inline
Size_<_Tp>& Size_<_Tp>::operator = (const Size_<_Tp>& sz)
{
	width = sz.width; height = sz.height;
	return *this;
}

template<typename _Tp> inline
_Tp Size_<_Tp>::area() const
{
	return width * height;
}

template<typename _Tp> static inline
Size_<_Tp>& operator *= (Size_<_Tp>& a, _Tp b)
{
	a.width *= b;
	a.height *= b;
	return a;
}

template<typename _Tp> static inline
Size_<_Tp> operator * (const Size_<_Tp>& a, _Tp b)
{
	Size_<_Tp> tmp(a);
	tmp *= b;
	return tmp;
}

template<typename _Tp> static inline
Size_<_Tp>& operator /= (Size_<_Tp>& a, _Tp b)
{
	a.width /= b;
	a.height /= b;
	return a;
}

template<typename _Tp> static inline
Size_<_Tp> operator / (const Size_<_Tp>& a, _Tp b)
{
	Size_<_Tp> tmp(a);
	tmp /= b;
	return tmp;
}

template<typename _Tp> static inline
Size_<_Tp>& operator += (Size_<_Tp>& a, const Size_<_Tp>& b)
{
	a.width += b.width;
	a.height += b.height;
	return a;
}

template<typename _Tp> static inline
Size_<_Tp> operator + (const Size_<_Tp>& a, const Size_<_Tp>& b)
{
	Size_<_Tp> tmp(a);
	tmp += b;
	return tmp;
}

template<typename _Tp> static inline
Size_<_Tp>& operator -= (Size_<_Tp>& a, const Size_<_Tp>& b)
{
	a.width -= b.width;
	a.height -= b.height;
	return a;
}

template<typename _Tp> static inline
Size_<_Tp> operator - (const Size_<_Tp>& a, const Size_<_Tp>& b)
{
	Size_<_Tp> tmp(a);
	tmp -= b;
	return tmp;
}

template<typename _Tp> static inline
bool operator == (const Size_<_Tp>& a, const Size_<_Tp>& b)
{
	return a.width == b.width && a.height == b.height;
}

template<typename _Tp> static inline
bool operator != (const Size_<_Tp>& a, const Size_<_Tp>& b)
{
	return !(a == b);
}

// Rect_
template<typename _Tp> inline
Rect_<_Tp>::Rect_()
: x(0), y(0), width(0), height(0) {}

template<typename _Tp> inline
Rect_<_Tp>::Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height)
: x(_x), y(_y), width(_width), height(_height) {}

template<typename _Tp> inline
Rect_<_Tp>::Rect_(const Rect_<_Tp>& r)
: x(r.x), y(r.y), width(r.width), height(r.height) {}

template<typename _Tp> inline
Rect_<_Tp>::Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz)
: x(org.x), y(org.y), width(sz.width), height(sz.height) {}

template<typename _Tp> inline
Rect_<_Tp>::Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2)
{
	x = std::min(pt1.x, pt2.x);
	y = std::min(pt1.y, pt2.y);
	width = std::max(pt1.x, pt2.x) - x;
	height = std::max(pt1.y, pt2.y) - y;
}

template<typename _Tp> inline
Rect_<_Tp>& Rect_<_Tp>::operator = (const Rect_<_Tp>& r)
{
	x = r.x;
	y = r.y;
	width = r.width;
	height = r.height;
	return *this;
}

template<typename _Tp> inline
Point_<_Tp> Rect_<_Tp>::tl() const
{
	return Point_<_Tp>(x, y);
}

template<typename _Tp> inline
Point_<_Tp> Rect_<_Tp>::br() const
{
	return Point_<_Tp>(x + width, y + height);
}

template<typename _Tp> inline
Size_<_Tp> Rect_<_Tp>::size() const
{
	return Size_<_Tp>(width, height);
}

template<typename _Tp> inline
_Tp Rect_<_Tp>::area() const
{
	return width * height;
}

template<typename _Tp> template<typename _Tp2> inline
Rect_<_Tp>::operator Rect_<_Tp2>() const
{
	return Rect_<_Tp2>(static_cast<_Tp2>(x), static_cast<_Tp2>(y), static_cast<_Tp2>(width), static_cast<_Tp2>(height));
}

template<typename _Tp> inline
bool Rect_<_Tp>::contains(const Point_<_Tp>& pt) const
{
	return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height;
}


template<typename _Tp> static inline
Rect_<_Tp>& operator += (Rect_<_Tp>& a, const Point_<_Tp>& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

template<typename _Tp> static inline
Rect_<_Tp>& operator -= (Rect_<_Tp>& a, const Point_<_Tp>& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

template<typename _Tp> static inline
Rect_<_Tp>& operator += (Rect_<_Tp>& a, const Size_<_Tp>& b)
{
	a.width += b.width;
	a.height += b.height;
	return a;
}

template<typename _Tp> static inline
Rect_<_Tp>& operator -= (Rect_<_Tp>& a, const Size_<_Tp>& b)
{
	a.width -= b.width;
	a.height -= b.height;
	return a;
}

template<typename _Tp> static inline
Rect_<_Tp>& operator &= (Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
	_Tp x1 = std::max(a.x, b.x);
	_Tp y1 = std::max(a.y, b.y);
	a.width = std::min(a.x + a.width, b.x + b.width) - x1;
	a.height = std::min(a.y + a.height, b.y + b.height) - y1;
	a.x = x1;
	a.y = y1;
	if (a.width <= 0 || a.height <= 0)
		a = Rect();
	return a;
}

template<typename _Tp> static inline
Rect_<_Tp>& operator |= (Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
	_Tp x1 = std::min(a.x, b.x);
	_Tp y1 = std::min(a.y, b.y);
	a.width = std::max(a.x + a.width, b.x + b.width) - x1;
	a.height = std::max(a.y + a.height, b.y + b.height) - y1;
	a.x = x1;
	a.y = y1;
	return a;
}

template<typename _Tp> static inline
bool operator == (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
	return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

template<typename _Tp> static inline
bool operator != (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
	return a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height;
}

template<typename _Tp> static inline
Rect_<_Tp> operator + (const Rect_<_Tp>& a, const Point_<_Tp>& b)
{
	return Rect_<_Tp>(a.x + b.x, a.y + b.y, a.width, a.height);
}

template<typename _Tp> static inline
Rect_<_Tp> operator - (const Rect_<_Tp>& a, const Point_<_Tp>& b)
{
	return Rect_<_Tp>(a.x - b.x, a.y - b.y, a.width, a.height);
}

template<typename _Tp> static inline
Rect_<_Tp> operator + (const Rect_<_Tp>& a, const Size_<_Tp>& b)
{
	return Rect_<_Tp>(a.x, a.y, a.width + b.width, a.height + b.height);
}

template<typename _Tp> static inline
Rect_<_Tp> operator & (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
	Rect_<_Tp> c = a;
	return c &= b;
}

template<typename _Tp> static inline
Rect_<_Tp> operator | (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
	Rect_<_Tp> c = a;
	return c |= b;
}

// Range
inline
Range::Range()
: start(0), end(0) {}

inline
Range::Range(int _start, int _end)
: start(_start), end(_end) {}

inline
int Range::size() const
{
	return end - start;
}

inline
bool Range::empty() const
{
	return start == end;
}

inline
Range Range::all()
{
	return Range(INT_MIN, INT_MAX);
}

static inline
bool operator == (const Range& r1, const Range& r2)
{
	return r1.start == r2.start && r1.end == r2.end;
}

static inline
bool operator != (const Range& r1, const Range& r2)
{
	return !(r1 == r2);
}

static inline
bool operator !(const Range& r)
{
	return r.start == r.end;
}

static inline
Range operator & (const Range& r1, const Range& r2)
{
	Range r(std::max(r1.start, r2.start), std::min(r1.end, r2.end));
	r.end = std::max(r.end, r.start);
	return r;
}

static inline
Range& operator &= (Range& r1, const Range& r2)
{
	r1 = r1 & r2;
	return r1;
}

static inline
Range operator + (const Range& r1, int delta)
{
	return Range(r1.start + delta, r1.end + delta);
}

static inline
Range operator + (int delta, const Range& r1)
{
	return Range(r1.start + delta, r1.end + delta);
}

static inline
Range operator - (const Range& r1, int delta)
{
	return r1 + (-delta);
}

// Scalar_
template<typename _Tp> inline
Scalar_<_Tp>::Scalar_()
{
	this->val[0] = this->val[1] = this->val[2] = this->val[3] = 0;
}

template<typename _Tp> inline
Scalar_<_Tp>::Scalar_(_Tp v0, _Tp v1, _Tp v2, _Tp v3)
{
	this->val[0] = v0;
	this->val[1] = v1;
	this->val[2] = v2;
	this->val[3] = v3;
}

template<typename _Tp> template<typename _Tp2, int cn> inline
Scalar_<_Tp>::Scalar_(const Vec<_Tp2, cn>& v)
{
	int i;
	for (i = 0; i < (cn < 4 ? cn : 4); i++)
		this->val[i] = cv::static_cast<_Tp>(v.val[i]);
	for (; i < 4; i++)
		this->val[i] = 0;
}

template<typename _Tp> inline
Scalar_<_Tp>::Scalar_(_Tp v0)
{
	this->val[0] = v0;
	this->val[1] = this->val[2] = this->val[3] = 0;
}

template<typename _Tp> inline
Scalar_<_Tp> Scalar_<_Tp>::all(_Tp v0)
{
	return Scalar_<_Tp>(v0, v0, v0, v0);
}


template<typename _Tp> inline
Scalar_<_Tp> Scalar_<_Tp>::mul(const Scalar_<_Tp>& a, double scale) const
{
	return Scalar_<_Tp>(static_cast<_Tp>(this->val[0] * a.val[0] * scale),
		static_cast<_Tp>(this->val[1] * a.val[1] * scale),
		static_cast<_Tp>(this->val[2] * a.val[2] * scale),
		static_cast<_Tp>(this->val[3] * a.val[3] * scale));
}

template<typename _Tp> inline
Scalar_<_Tp> Scalar_<_Tp>::conj() const
{
	return Scalar_<_Tp>(static_cast<_Tp>(this->val[0]),
		static_cast<_Tp>(-this->val[1]),
		static_cast<_Tp>(-this->val[2]),
		static_cast<_Tp>(-this->val[3]));
}

template<typename _Tp> inline
bool Scalar_<_Tp>::isReal() const
{
	return this->val[1] == 0 && this->val[2] == 0 && this->val[3] == 0;
}


template<typename _Tp> template<typename T2> inline
Scalar_<_Tp>::operator Scalar_<T2>() const
{
	return Scalar_<T2>(static_cast<T2>(this->val[0]),
		static_cast<T2>(this->val[1]),
		static_cast<T2>(this->val[2]),
		static_cast<T2>(this->val[3]));
}


template<typename _Tp> static inline
Scalar_<_Tp>& operator += (Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	a.val[0] += b.val[0];
	a.val[1] += b.val[1];
	a.val[2] += b.val[2];
	a.val[3] += b.val[3];
	return a;
}

template<typename _Tp> static inline
Scalar_<_Tp>& operator -= (Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	a.val[0] -= b.val[0];
	a.val[1] -= b.val[1];
	a.val[2] -= b.val[2];
	a.val[3] -= b.val[3];
	return a;
}

template<typename _Tp> static inline
Scalar_<_Tp>& operator *= (Scalar_<_Tp>& a, _Tp v)
{
	a.val[0] *= v;
	a.val[1] *= v;
	a.val[2] *= v;
	a.val[3] *= v;
	return a;
}

template<typename _Tp> static inline
bool operator == (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	return a.val[0] == b.val[0] && a.val[1] == b.val[1] &&
		a.val[2] == b.val[2] && a.val[3] == b.val[3];
}

template<typename _Tp> static inline
bool operator != (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	return a.val[0] != b.val[0] || a.val[1] != b.val[1] ||
		a.val[2] != b.val[2] || a.val[3] != b.val[3];
}

template<typename _Tp> static inline
Scalar_<_Tp> operator + (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	return Scalar_<_Tp>(a.val[0] + b.val[0],
		a.val[1] + b.val[1],
		a.val[2] + b.val[2],
		a.val[3] + b.val[3]);
}

template<typename _Tp> static inline
Scalar_<_Tp> operator - (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	return Scalar_<_Tp>(static_cast<_Tp>(a.val[0] - b.val[0]),
		static_cast<_Tp>(a.val[1] - b.val[1]),
		static_cast<_Tp>(a.val[2] - b.val[2]),
		static_cast<_Tp>(a.val[3] - b.val[3]));
}

template<typename _Tp> static inline
Scalar_<_Tp> operator * (const Scalar_<_Tp>& a, _Tp alpha)
{
	return Scalar_<_Tp>(a.val[0] * alpha,
		a.val[1] * alpha,
		a.val[2] * alpha,
		a.val[3] * alpha);
}

template<typename _Tp> static inline
Scalar_<_Tp> operator * (_Tp alpha, const Scalar_<_Tp>& a)
{
	return a*alpha;
}

template<typename _Tp> static inline
Scalar_<_Tp> operator - (const Scalar_<_Tp>& a)
{
	return Scalar_<_Tp>(static_cast<_Tp>(-a.val[0]),
		static_cast<_Tp>(-a.val[1]),
		static_cast<_Tp>(-a.val[2]),
		static_cast<_Tp>(-a.val[3]));
}

template<typename _Tp> static inline
Scalar_<_Tp> operator * (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	return Scalar_<_Tp>(static_cast<_Tp>(a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3]),
		static_cast<_Tp>(a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2]),
		static_cast<_Tp>(a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1]),
		static_cast<_Tp>(a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0]));
}

template<typename _Tp> static inline
Scalar_<_Tp>& operator *= (Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	a = a * b;
	return a;
}

template<typename _Tp> static inline
Scalar_<_Tp> operator / (const Scalar_<_Tp>& a, _Tp alpha)
{
	return Scalar_<_Tp>(a.val[0] / alpha,
		a.val[1] / alpha,
		a.val[2] / alpha,
		a.val[3] / alpha);
}

template<typename _Tp> static inline
Scalar_<float> operator / (const Scalar_<float>& a, float alpha)
{
	float s = 1 / alpha;
	return Scalar_<float>(a.val[0] * s, a.val[1] * s, a.val[2] * s, a.val[3] * s);
}

template<typename _Tp> static inline
Scalar_<double> operator / (const Scalar_<double>& a, double alpha)
{
	double s = 1 / alpha;
	return Scalar_<double>(a.val[0] * s, a.val[1] * s, a.val[2] * s, a.val[3] * s);
}

template<typename _Tp> static inline
Scalar_<_Tp>& operator /= (Scalar_<_Tp>& a, _Tp alpha)
{
	a = a / alpha;
	return a;
}

template<typename _Tp> static inline
Scalar_<_Tp> operator / (_Tp a, const Scalar_<_Tp>& b)
{
	_Tp s = a / (b[0] * b[0] + b[1] * b[1] + b[2] * b[2] + b[3] * b[3]);
	return b.conj() * s;
}

template<typename _Tp> static inline
Scalar_<_Tp> operator / (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	return a * ((_Tp)1 / b);
}

template<typename _Tp> static inline
Scalar_<_Tp>& operator /= (Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
	a = a / b;
	return a;
}

} //fbc
#endif //FBC_OPENCV_TEST_BASE_HPP_
