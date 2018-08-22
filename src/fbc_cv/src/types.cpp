// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

/* reference: include/opencv2/core/types.hpp
              modules/core/src/matrix.cpp
*/

#include <float.h>
#include "core/types.hpp"
#include "core/base.hpp"
#include "core/fast_math.hpp"

namespace fbc {
///////////////////////////// RotatedRect /////////////////////////////
RotatedRect::RotatedRect() : center(), size(), angle(0) {}

RotatedRect::RotatedRect(const Point2f& _center, const Size2f& _size, float _angle) : center(_center), size(_size), angle(_angle) {}

RotatedRect::RotatedRect(const Point2f& _point1, const Point2f& _point2, const Point2f& _point3)
{
	Point2f _center = 0.5f * (_point1 + _point3);
	Vec2f vecs[2];
	vecs[0] = Vec2f(_point1 - _point2);
	vecs[1] = Vec2f(_point2 - _point3);
	// check that given sides are perpendicular
	FBC_Assert(std::abs(vecs[0].dot(vecs[1])) / (norm(vecs[0]) * norm(vecs[1])) <= FLT_EPSILON);

	// wd_i stores which vector (0,1) or (1,2) will make the width
	// One of them will definitely have slope within -1 to 1
	int wd_i = 0;
	if (std::abs(vecs[1][1]) < std::abs(vecs[1][0])) wd_i = 1;
	int ht_i = (wd_i + 1) % 2;

	float _angle = atan(vecs[wd_i][1] / vecs[wd_i][0]) * 180.0f / (float)FBC_PI;
	float _width = (float)norm(vecs[wd_i]);
	float _height = (float)norm(vecs[ht_i]);

	center = _center;
	size = Size2f(_width, _height);
	angle = _angle;
}

void RotatedRect::points(Point2f pt[]) const
{
	double _angle = angle*FBC_PI / 180.;
	float b = (float)cos(_angle)*0.5f;
	float a = (float)sin(_angle)*0.5f;

	pt[0].x = center.x - a*size.height - b*size.width;
	pt[0].y = center.y + b*size.height - a*size.width;
	pt[1].x = center.x + a*size.height - b*size.width;
	pt[1].y = center.y - b*size.height - a*size.width;
	pt[2].x = 2 * center.x - pt[0].x;
	pt[2].y = 2 * center.y - pt[0].y;
	pt[3].x = 2 * center.x - pt[1].x;
	pt[3].y = 2 * center.y - pt[1].y;
}

Rect RotatedRect::boundingRect() const
{
	Point2f pt[4];
	points(pt);
	Rect r(fbcFloor(std::min(std::min(std::min(pt[0].x, pt[1].x), pt[2].x), pt[3].x)),
		fbcFloor(std::min(std::min(std::min(pt[0].y, pt[1].y), pt[2].y), pt[3].y)),
		fbcCeil(std::max(std::max(std::max(pt[0].x, pt[1].x), pt[2].x), pt[3].x)),
		fbcCeil(std::max(std::max(std::max(pt[0].y, pt[1].y), pt[2].y), pt[3].y)));
	r.width -= r.x - 1;
	r.height -= r.y - 1;

	return r;
}

} // namespace fbc
