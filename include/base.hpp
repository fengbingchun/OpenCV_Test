#ifndef FBC_OPENCV_TEST_BASE_HPP_
#define FBC_OPENCV_TEST_BASE_HPP_

namespace fbc {

template<typename _Tp, int cn> class Vec;
template<typename _Tp> class Point_;
template<typename _Tp> class Size_;
template<typename _Tp> class Point3_;
template<typename _Tp> class Rect_;
template<typename _Tp> class Scalar_;
class Range;

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

class Range {
public:
	Range();
	Range(int _start, int _end);
	int size() const;
	bool empty() const;
	static Range all();

	int start, end;
};

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

} //fbc
#endif //FBC_OPENCV_TEST_BASE_HPP_
