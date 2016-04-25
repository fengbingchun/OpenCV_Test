#ifndef FBC_CV_CORE_TYPES_HPP_
#define FBC_CV_CORE_TYPES_HPP_

// reference: include/opencv2/core/types.hpp

#ifndef __cplusplus
	#error types.hpp header must be compiled as C++
#endif

#include "core/fbcdef.hpp"
#include "core/matx.hpp"

namespace fbc {
template<typename _Tp> class Size_;
template<typename _Tp> class Rect_;

//////////////////////////////// Point_ ////////////////////////////////
template<typename _Tp> class Point_
{
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

	//! checks whether the point is inside the specified rectangle
	bool inside(const Rect_<_Tp>& r) const;

	_Tp x, y; //< the point coordinates
};

typedef Point_<int> Point2i;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Point2i Point;

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
	return Point_<_Tp2>(saturate_cast<_Tp2>(x), saturate_cast<_Tp2>(y));
}

template<typename _Tp> inline
Point_<_Tp>::operator Vec<_Tp, 2>() const
{
	return Vec<_Tp, 2>(x, y);
}

template<typename _Tp> inline bool
Point_<_Tp>::inside(const Rect_<_Tp>& r) const
{
	return r.contains(*this);
}

//////////////////////////////// Point3_ ////////////////////////////////
template<typename _Tp> class Point3_
{
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

	_Tp x, y, z; //< the point coordinates
};

typedef Point3_<int> Point3i;
typedef Point3_<float> Point3f;
typedef Point3_<double> Point3d;

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
	return Point3_<_Tp2>(saturate_cast<_Tp2>(x), saturate_cast<_Tp2>(y), saturate_cast<_Tp2>(z));
}

template<typename _Tp> inline
Point3_<_Tp>::operator Vec<_Tp, 3>() const
{
	return Vec<_Tp, 3>(x, y, z);
}

//////////////////////////////// Size_ ////////////////////////////////
template<typename _Tp> class Size_
{
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
	return Size_<_Tp2>(saturate_cast<_Tp2>(width), saturate_cast<_Tp2>(height));
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

//////////////////////////////// Rect_ ////////////////////////////////
template<typename _Tp> class Rect_
{
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
	return Rect_<_Tp2>(saturate_cast<_Tp2>(x), saturate_cast<_Tp2>(y), saturate_cast<_Tp2>(width), saturate_cast<_Tp2>(height));
}

template<typename _Tp> inline
bool Rect_<_Tp>::contains(const Point_<_Tp>& pt) const
{
	return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height;
}

//////////////////////////////// Range /////////////////////////////////
class Range
{
public:
	Range();
	Range(int _start, int _end);
	int size() const;
	bool empty() const;
	static Range all();

	int start, end;
};

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


//////////////////////////////// Scalar_ ///////////////////////////////
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
};

typedef Scalar_<double> Scalar;

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
		this->val[i] = saturate_cast<_Tp>(v.val[i]);
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

template<typename _Tp> template<typename T2> inline
Scalar_<_Tp>::operator Scalar_<T2>() const
{
	return Scalar_<T2>(saturate_cast<T2>(this->val[0]),
		saturate_cast<T2>(this->val[1]),
		saturate_cast<T2>(this->val[2]),
		saturate_cast<T2>(this->val[3]));
}

} // fbc

#endif // FBC_CV_CORE_TYPES_HPP_
