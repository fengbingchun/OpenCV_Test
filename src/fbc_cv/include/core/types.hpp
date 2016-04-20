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



} // fbc

#endif // FBC_CV_CORE_TYPES_HPP_
