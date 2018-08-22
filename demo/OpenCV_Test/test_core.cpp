#include <assert.h>

#include <core/fast_math.hpp>
#include <core/base.hpp>
#include <core/saturate.hpp>
#include <core/matx.hpp>
#include <core/types.hpp>
#include <core/mat.hpp>
#include <core/Ptr.hpp>

#include <opencv2/opencv.hpp>
#include "fbc_cv_funset.hpp"

int test_fast_math()
{
	int ret1 = 0, ret2 = 0, i = 5;
	float f1 = 5.11, f2 = 5.99, f3 = -5.11, f4 = -5.99;
	double d1 = 5.11, d2 = 5.99, d3 = -5.11, d4 = -5.99;

	assert(fbc::fbcRound(i) == cvRound(i));
	assert(fbc::fbcRound(f1) == cvRound(f1));
	assert(fbc::fbcRound(f2) == cvRound(f2));
	assert(fbc::fbcRound(f3) == cvRound(f3));
	assert(fbc::fbcRound(f4) == cvRound(f4));
	
	assert(fbc::fbcFloor(i) == cvFloor(i));
	assert(fbc::fbcFloor(f1) == cvFloor(f1));
	assert(fbc::fbcFloor(f2) == cvFloor(f2));
	assert(fbc::fbcFloor(f3) == cvFloor(f3));
	assert(fbc::fbcFloor(f4) == cvFloor(f4));

	assert(fbc::fbcCeil(i) == cvCeil(i));
	assert(fbc::fbcCeil(f1) == cvCeil(f1));
	assert(fbc::fbcCeil(f2) == cvCeil(f2));
	assert(fbc::fbcCeil(f3) == cvCeil(f3));
	assert(fbc::fbcCeil(f4) == cvCeil(f4));

	return 0;
}

int test_base()
{
	FBC_StaticAssert(sizeof(void *) == 8, "64-bit code generation is not supported."); // sizeof(void *) = 8/4 ?
	CV_StaticAssert(sizeof(void *) == 8, "64-bit code generation is not supported.");

	double d1 = 1.0, d2 = 1.9, d3 = -1.0, d4 = -1.9;

	FBC_Assert(d1 > 0);
	CV_Assert(d1 > 0);

	assert(fbc::fbc_abs<double>(d1) == cv::cv_abs<double>(d1));
	assert(fbc::fbc_abs<double>(d2) == cv::cv_abs<double>(d2));
	assert(fbc::fbc_abs<double>(d3) == cv::cv_abs<double>(d3));
	assert(fbc::fbc_abs<double>(d4) == cv::cv_abs<double>(d4));

	fbc::uchar uch = 10;
	fbc::schar sch = -5;
	fbc::ushort ush = 10;
	short sh = -5;

	assert(fbc::fbc_abs(uch) == cv::cv_abs(uch));
	assert(fbc::fbc_abs(sch) == cv::cv_abs(sch));
	assert(fbc::fbc_abs(ush) == cv::cv_abs(ush));
	assert(fbc::fbc_abs(sh) == cv::cv_abs(sh));

	return 0;
}

int test_saturate()
{
	fbc::uchar uch1 = 10;
	fbc::uint ui = 1000;
	fbc::schar sch = -2;
	fbc::ushort ush = 500;
	int i = -1435;
	float f = -2323.3;
	double d = 33333.33333;

	assert(fbc::saturate_cast<fbc::uchar>(ui) == cv::saturate_cast<uchar>(ui));
	assert(fbc::saturate_cast<fbc::uchar>(sch) == cv::saturate_cast<uchar>(sch));
	assert(fbc::saturate_cast<fbc::uchar>(ush) == cv::saturate_cast<uchar>(ush));
	assert(fbc::saturate_cast<fbc::uchar>(i) == cv::saturate_cast<uchar>(i));
	assert(fbc::saturate_cast<fbc::uchar>(f) == cv::saturate_cast<uchar>(f));
	assert(fbc::saturate_cast<fbc::uchar>(d) == cv::saturate_cast<uchar>(d));
	assert(fbc::saturate_cast<fbc::uint>(f) == cv::saturate_cast<uint>(f));
	assert(fbc::saturate_cast<fbc::schar>(ush) == cv::saturate_cast<schar>(ush));
	assert(fbc::saturate_cast<fbc::ushort>(d) == cv::saturate_cast<ushort>(d));
	assert(fbc::saturate_cast<unsigned>(f) == cv::saturate_cast<unsigned>(f));
	assert(fbc::saturate_cast<int>(d) == cv::saturate_cast<int>(d));

	return 0;
}

int test_Matx()
{
	fbc::Matx22f matx1(1.1, 2.2, 3.3, 4.4);
	fbc::Matx22f matx3(matx1);
	fbc::Matx22f matx4 = fbc::Matx22f::all(-1.1);
	fbc::Matx22f matx5 = fbc::Matx22f::ones();
	fbc::Matx22f matx6 = fbc::Matx22f::eye();
	fbc::Matx22f::diag_type diag_(9, 9);
	fbc::Matx22f matx7 = fbc::Matx22f::diag(diag_);
	float ret1 = matx1.dot(matx3);
	double ret2 = matx3.ddot(matx4);
	fbc::Matx<int, 2, 2> matx8 = fbc::Matx<int, 2, 2>(matx1);
	fbc::Matx12f matx9 = matx1.row(1);
	fbc::Matx21f matx10 = matx1.col(1);
	float ret3 = matx1(1, 1);
	fbc::Matx22f matx11 = matx1 + matx4;
	fbc::Matx22f matx12 = matx1 - matx6;
	fbc::Matx22f matx13 = matx1 * ret1;
	fbc::Matx22f matx14 = matx1 * matx4;
	fbc::Matx41f matx15 = matx1.reshape<4, 1>();
	fbc::Matx21f matx16 = matx1.get_minor<2, 1>(0, 1);

	cv::Matx22f matx1_(1.1, 2.2, 3.3, 4.4);
	cv::Matx22f matx3_(matx1_);
	cv::Matx22f matx4_ = cv::Matx22f::all(-1.1);
	cv::Matx22f matx5_ = cv::Matx22f::ones();
	cv::Matx22f matx6_ = cv::Matx22f::eye();
	cv::Matx22f::diag_type diag__(9, 9);
	cv::Matx22f matx7_ = cv::Matx22f::diag(diag__);
	float ret1_ = matx1_.dot(matx3_);
	double ret2_ = matx3_.ddot(matx4_);
	cv::Matx<int, 2, 2> matx8_ = cv::Matx<int, 2, 2>(matx1_);
	cv::Matx12f matx9_ = matx1_.row(1);
	cv::Matx21f matx10_ = matx1_.col(1);
	float ret3_ = matx1_(1, 1);
	cv::Matx22f matx11_ = matx1_ + matx4_;
	cv::Matx22f matx12_ = matx1_ - matx6_;
	cv::Matx22f matx13_ = matx1_ * ret1_;
	cv::Matx22f matx14_ = matx1_ * matx4_;
	cv::Matx41f matx15_ = matx1_.reshape<4, 1>();
	cv::Matx21f matx16_ = matx1_.get_minor<2, 1>(0, 1);

	const float eps = 0.000001;

	for (int i = 0; i < 4; i++) {
		assert(fabs(matx1.val[i] - matx1_.val[i]) < eps);
		assert(fabs(matx3.val[i] - matx3_.val[i]) < eps);
		assert(fabs(matx4.val[i] - matx4_.val[i]) < eps);
		assert(fabs(matx5.val[i] - matx5_.val[i]) < eps);
		assert(fabs(matx6.val[i] - matx6_.val[i]) < eps);
		assert(fabs(matx7.val[i] - matx7_.val[i]) < eps);
		assert(matx8.val[i] == matx8_.val[i]);
		assert(fabs(matx11.val[i] - matx11_.val[i]) < eps);
		assert(fabs(matx12.val[i] - matx12_.val[i]) < eps);
		assert(fabs(matx13.val[i] - matx13_.val[i]) < eps);
		assert(fabs(matx14.val[i] - matx14_.val[i]) < eps);
	}

	assert(fabs(ret1 - ret1_) < eps);
	assert(fabs(ret2 - ret2_) < eps);
	assert(fabs(ret3 - ret3_) < eps);

	for (int i = 0; i < 2; i++) {
		assert(fabs(matx9.val[i] - matx9_.val[i]) < eps);
		assert(fabs(matx10.val[i] - matx10_.val[i]) < eps);
	}

	assert(matx15.rows == 4 && matx15.cols == 1 && matx15_.rows == 4 && matx15_.cols == 1);
	assert(fabs(matx16.val[0] - 2.2) < eps);
	assert(fabs(matx16.val[1] - 4.4) < eps);
	assert(fabs(matx16.val[0] - matx16_.val[0]) < eps);
	assert(fabs(matx16.val[1] - matx16_.val[1]) < eps);

	return 0;
}

int test_Vec()
{
	fbc::Vec6f vec1;
	for (int i = 0; i < 6; i++) {
		vec1.val[i] = i * 1.1;
	}
	fbc::Vec6f vec2(-7.7, -8.8);
	float* tmp = new float[6];
	for (int i = 0; i < 6; i++) {
		tmp[i] = i + 9.9;
	}
	fbc::Vec6f vec3(tmp);
	fbc::Vec6f vec4(vec3);
	fbc::Vec6f vec5 = fbc::Vec6f::all(-3.3);
	vec1 = vec1.mul(vec5);
	fbc::Vec6i vec6;
	vec6 = fbc::Vec<int, 6>(vec1);
	int i1 = vec6.val[1];
	
	fbc::Vec6f vec7 = vec1 + vec2;
	fbc::Vec6f vec8(-3.4, -0.5, 9.1);
	vec8 += vec7;
	fbc::Vec6f vec9(vec8);
	vec9 *= 2;
	fbc::Vec6f vec10(vec9);
	vec10 /= 1.2;
	fbc::Vec6f vec11(vec10);
	fbc::Vec6f vec12 = vec11 * 5;

	cv::Vec6f vec1_;
	for (int i = 0; i < 6; i++) {
		vec1_.val[i] = i * 1.1;
	}
	cv::Vec6f vec2_(-7.7, -8.8);
	cv::Vec6f vec3_(tmp);
	cv::Vec6f vec4_(vec3_);
	cv::Vec6f vec5_ = cv::Vec6f::all(-3.3);
	vec1_ = vec1_.mul(vec5_);
	cv::Vec6i vec6_;
	vec6_ = cv::Vec<int, 6>(vec1_);
	int i1_ = vec6_.val[1];

	cv::Vec6f vec7_ = vec1_ + vec2_;
	cv::Vec6f vec8_(-3.4, -0.5, 9.1);
	vec8_ += vec7_;
	cv::Vec6f vec9_(vec8_);
	vec9_ *= 2;
	cv::Vec6f vec10_(vec9_);
	vec10_ /= 1.2;
	cv::Vec6f vec11_(vec10_);
	cv::Vec6f vec12_ = vec11_ * 5;

	const float eps = 0.000001;

	for (int i = 0; i < 6; i++) {
		assert(fabs(vec1.val[i] - vec1_.val[i]) < eps);
		assert(fabs(vec2.val[i] - vec2_.val[i]) < eps);
		assert(fabs(vec3.val[i] - vec3_.val[i]) < eps);
		assert(fabs(vec3.val[i] - tmp[i]) < eps);
		assert(fabs(vec4.val[i] - vec4_.val[i]) < eps);
		assert(fabs(vec5.val[i] - vec5_.val[i]) < eps);
		assert(vec6.val[i] == vec6_.val[i]);
		assert(fabs(vec7.val[i] - vec7_.val[i]) < eps);
		assert(fabs(vec8.val[i] - vec8_.val[i]) < eps);
		assert(fabs(vec9.val[i] - vec9_.val[i]) < eps);
		assert(fabs(vec10.val[i] - vec10_.val[i]) < eps);
		assert(fabs(vec11.val[i] - vec11_.val[i]) < eps);
		assert(fabs(vec12.val[i] - vec12_.val[i]) < eps);
	}
	assert(i1 == i1_);
	assert(i1 == -4);

	delete [] tmp;
	return 0;
}

int test_Point()
{
	const float eps = 0.000001;

	fbc::Point point1;
	fbc::Point point2(2, 3);
	fbc::Point point3(point2);
	fbc::Size size(10, 20);
	fbc::Point point4(size);
	assert(point4.x == 10 && point4.y == 20);
	fbc::Vec2f vec(5.1, 2.2);
	fbc::Point2f point5(vec);
	fbc::Point point6 = point3;
	fbc::Point point7 = fbc::Point(point5);
	assert(point7.x == 5 && point7.y == 2);
	int ret1 = point2.dot(point4);
	assert(ret1 == 80);
	double ret2 = point5.cross(point4);
	assert(fabs(ret2 - 79.9999976) < eps);
	fbc::Rect rect(1, 0, 10, 20);
	bool ret3 = point3.inside(rect);
	assert(ret3 == true);
	point1 += point3;
	point2 -= point1;
	point3 *= ret1;
	point4 /= ret2;
	double ret4 = fbc::norm(point3);
	assert(fabs(ret4 - 288.444102037) < eps);
	point6 = point1 + point7;
	point1 = point2 * ret1;
	point7 = point3 / ret4;

	cv::Point point1_;
	cv::Point point2_(2, 3);
	cv::Point point3_(point2_);
	cv::Size size_(10, 20);
	cv::Point point4_(size_);
	assert(point4_.x == 10 && point4_.y == 20);
	cv::Vec2f vec_(5.1, 2.2);
	cv::Point2f point5_(vec_);
	cv::Point point6_ = point3_;
	cv::Point point7_ = cv::Point(point5_);
	assert(point7_.x == 5 && point7_.y == 2);
	int ret1_ = point2_.dot(point4_);
	assert(ret1_ == 80);
	double ret2_ = point5_.cross(point4_);
	assert(fabs(ret2_ - 79.9999976) < eps);
	cv::Rect rect_(1, 0, 10, 20);
	bool ret3_ = point3_.inside(rect_);
	assert(ret3_ == true);
	point1_ += point3_;
	point2_ -= point1_;
	point3_ *= ret1_;
	point4_ /= ret2_;
	double ret4_ = cv::norm(point3_);
	assert(fabs(ret4_ - 288.444102037) < eps);
	point6_ = point1_ + point7_;
	point1_ = point2_ * ret1_;
	point7_ = point3_ / ret4_;

	assert(point1.x == point1_.x && point1.y == point1_.y);
	assert(point2.x == point2_.x && point2.y == point2_.y);
	assert(point3.x == point3_.x && point3.y == point3_.y);
	assert(point4.x == point4_.x && point4.y == point4_.y);
	assert(fabs(point5.x - point5_.x) < eps && fabs(point5.y - point5_.y) < eps);
	assert(point6.x == point6_.x && point6.y == point6_.y);
	assert(point7.x == point7_.x && point7.y == point7_.y);
	assert(ret1 == ret1_);
	assert(fabs(ret2 - ret2_) < eps);
	assert(ret3 == ret3_);
	assert(fabs(ret4 - ret4_) < eps);

	return 0;
}

int test_Point3()
{
	const float eps = 0.000001;

	fbc::Point3i point1;
	fbc::Point3i point2(2, 3, 4);
	fbc::Point3i point3(point2);
	fbc::Vec3f vec1(5.1, 2.2, 3.4);
	fbc::Point3f point5(vec1);
	fbc::Point3f point6 = point3;
	fbc::Point3i point7 = fbc::Point3i(point5);
	assert(point7.x == 5 && point7.y == 2 && point7.z == 3);
	fbc::Vec3i vec2(point7);
	int ret1 = point2.dot(point7);
	assert(ret1 == 28);
	fbc::Point3i point8 = point7.cross(point3);
	assert(point8.x == -1 && point8.y == -14 && point8.z == 11);
	point1 += point3;
	point2 -= point1;
	point3 *= ret1;
	point5 /= ret1;
	double ret4 = fbc::norm(point6);
	assert(fabs(ret4 - 5.3851648) < eps);
	point6 = point1 + point7;
	point1 = point2 * ret1;
	point7 = point3 / ret4;

	cv::Point3i point1_;
	cv::Point3i point2_(2, 3, 4);
	cv::Point3i point3_(point2_);
	cv::Vec3f vec1_(5.1, 2.2, 3.4);
	cv::Point3f point5_(vec1_);
	cv::Point3f point6_ = point3_;
	cv::Point3i point7_ = cv::Point3i(point5_);
	assert(point7_.x == 5 && point7_.y == 2 && point7_.z == 3);
	cv::Vec3i vec2_(point7_);
	int ret1_ = point2_.dot(point7_);
	assert(ret1 == 28);
	cv::Point3i point8_ = point7_.cross(point3_);
	assert(point8_.x == -1 && point8_.y == -14 && point8_.z == 11);
	point1_ += point3_;
	point2_ -= point1_;
	point3_ *= ret1_;
	point5_ /= ret1_;
	double ret4_ = cv::norm(point6_);
	assert(fabs(ret4_ - 5.3851648) < eps);
	point6_ = point1_ + point7_;
	point1_ = point2_ * ret1_;
	point7_ = point3_ / ret4_;

	assert(point1.x == point1_.x && point1.y == point1_.y && point1.z == point1_.z);
	assert(point2.x == point2_.x && point2.y == point2_.y && point2.z == point2_.z);
	assert(point3.x == point3_.x && point3.y == point3_.y && point3.z == point3_.z);
	assert(fabs(point5.x - point5_.x) < eps && fabs(point5.y - point5_.y) < eps && fabs(point5.z - point5_.z) < eps);
	assert(fabs(point6.x - point6_.x) < eps && fabs(point6.y - point6_.y) < eps && fabs(point6.z - point6_.z) < eps);
	assert(point7.x == point7_.x && point7.y == point7_.y && point7.z == point7_.z);
	assert(ret1 == ret1_);
	assert(fabs(ret4 - ret4_) < eps);

	return 0;
}

int test_Size()
{
	const float eps = 0.000001;

	fbc::Size size1;
	fbc::Size size2(10, 15);
	fbc::Size size3(size2);
	fbc::Point2f point1(2, 3);
	fbc::Size2f size4(point1);
	fbc::Size size5 = size3;
	int ret1 = size5.area();
	assert(ret1 == 150);
	fbc::Size size6 = fbc::Size(size4);
	size1 *= ret1;
	size2 = size1 * 4;
	size4 /= 1.2f;
	size5 += size1;
	size3 = size2 - size5;

	cv::Size size1_;
	cv::Size size2_(10, 15);
	cv::Size size3_(size2_);
	cv::Point2f point1_(2, 3);
	cv::Size2f size4_(point1_);
	cv::Size size5_ = size3_;
	int ret1_ = size5_.area();
	assert(ret1_ == 150);
	cv::Size size6_ = cv::Size(size4_);
	size1_ *= ret1_;
	size2_ = size1_ * 4;
	size4_ /= 1.2f;
	size5_ += size1_;
	size3_ = size2_ - size5_;

	assert(size1.width == size1_.width && size1.height == size1_.height);
	assert(size2.width == size2_.width && size2.height == size2_.height);
	assert(size3.width == size3_.width && size3.height == size3_.height);
	assert(size5.width == size5_.width && size5.height == size5_.height);
	assert(size4.width == size4_.width && size4.height == size4_.height);

	return 0;
}

int test_Rect()
{
	fbc::Rect rect1;
	fbc::Rect rect2(2, 3, 19, 25);
	fbc::Rect rect3(rect2);
	fbc::Point point1(2, 4);
	fbc::Size size1(22, 56);
	fbc::Rect rect4(point1, size1);
	fbc::Rect rect5 = rect4;
	fbc::Point point2 = rect5.tl();
	fbc::Point point3 = rect5.br();
	fbc::Size size2 = rect5.size();
	int ret1 = rect5.area();
	assert(ret1 == 1232);
	fbc::Rect2f rect6(1.3, 4.5, 78.2, 44.6);
	rect1 = fbc::Rect(rect6);
	bool ret2 = rect5.contains(point1);
	assert(ret2 == true);
	rect1 += point1;
	rect1 -= point3;
	rect2 += size1;
	rect3 &= rect2;
	rect4 |= rect3;
	rect1 = rect2 + point3;
	rect5 = rect3 - point2;

	cv::Rect rect1_;
	cv::Rect rect2_(2, 3, 19, 25);
	cv::Rect rect3_(rect2_);
	cv::Point point1_(2, 4);
	cv::Size size1_(22, 56);
	cv::Rect rect4_(point1_, size1_);
	cv::Rect rect5_ = rect4_;
	cv::Point point2_ = rect5_.tl();
	cv::Point point3_ = rect5_.br();
	cv::Size size2_ = rect5_.size();
	int ret1_ = rect5_.area();
	assert(ret1_ == 1232);
	cv::Rect2f rect6_(1.3, 4.5, 78.2, 44.6);
	rect1_ = cv::Rect(rect6_);
	bool ret2_ = rect5_.contains(point1_);
	assert(ret2_ == true);
	rect1_ += point1_;
	rect1_ -= point3_;
	rect2_ += size1_;
	rect3_ &= rect2_;
	rect4_ |= rect3_;
	rect1_ = rect2_ + point3_;
	rect5_ = rect3_ - point2_;

	assert(rect1.x == rect1_.x && rect1.y == rect1_.y && rect1.width == rect1_.width && rect1.height == rect1_.height);
	assert(rect2.x == rect2_.x && rect2.y == rect2_.y && rect2.width == rect2_.width && rect2.height == rect2_.height);
	assert(rect3.x == rect3_.x && rect3.y == rect3_.y && rect3.width == rect3_.width && rect3.height == rect3_.height);
	assert(rect4.x == rect4_.x && rect4.y == rect4_.y && rect4.width == rect4_.width && rect4.height == rect4_.height);
	assert(rect5.x == rect5_.x && rect5.y == rect5_.y && rect5.width == rect5_.width && rect5.height == rect5_.height);

	return 0;
}

int test_Range()
{
	fbc::Range range1;
	fbc::Range range2(10, 20);
	int ret1 = range2.size();
	assert(ret1 == 10);
	bool ret2 = range1.empty();
	assert(ret2 == true);
	bool ret3 = (range1 == range2);
	assert(ret3 == false);
	fbc::Range range3(15, 40);
	fbc::Range range4 = range2 & range3;
	assert(range4.start == 15 && range4.end == 20);
	range3 = range3 + 5;
	range1 = range1 - 6;

	cv::Range range1_;
	cv::Range range2_(10, 20);
	int ret1_ = range2_.size();
	assert(ret1_ == 10);
	bool ret2_ = range1_.empty();
	assert(ret2_ == true);
	bool ret3_ = (range1_ == range2_);
	assert(ret3_ == false);
	cv::Range range3_(15, 40);
	cv::Range range4_ = range2_ & range3_;
	assert(range4_.start == 15 && range4_.end == 20);
	range3_ = range3_ + 5;
	range1_ = range1_ - 6;

	assert(range1.start == range1_.start && range1.end == range1_.end);
	assert(range2.start == range2_.start && range2.end == range2_.end);
	assert(range3.start == range3_.start && range3.end == range3_.end);
	assert(range4.start == range4_.start && range4.end == range4_.end);

	return 0;
}

int test_Scalar()
{
	fbc::Scalar scalar1;
	fbc::Scalar scalar2(1, 2, 3, 4);
	fbc::Vec6f vec1(-1.4, -2.5, -3.6, -4.8, -5, -6);
	fbc::Scalar scalar3(vec1);
	fbc::Scalar scalar4(9);
	fbc::Scalar scalar5 = fbc::Scalar::all(8);
	fbc::Scalar_<int> scalar6 = fbc::Scalar_<int>(scalar3);
	assert(scalar6.val[0] == -1 && scalar6.val[1] == -3 && scalar6.val[2] == -4 && scalar6[3] == -5);
	fbc::Scalar scalar7 = scalar2 * 1.5;
	scalar1 = scalar2 + scalar5;
	scalar2 *= -6;
	scalar3 = scalar1 * scalar7;
	fbc::Scalar_<float> scalar8(2.2f, 3.3f, 4.4f, 5.5f);
	fbc::Scalar_<float> scalar9;
	float f1 = 1.1f;
	scalar9 = fbc::Scalar_<float>(scalar8) / float(f1);
	fbc::Scalar_<float> scalar10;
	int i = 2;
	scalar10 = scalar8 / i;

	cv::Scalar scalar1_;
	cv::Scalar scalar2_(1, 2, 3, 4);
	cv::Vec6f vec1_(-1.4, -2.5, -3.6, -4.8, -5, -6);
	cv::Scalar scalar3_(vec1_);
	cv::Scalar scalar4_(9);
	cv::Scalar scalar5_ = cv::Scalar::all(8);
	cv::Scalar_<int> scalar6_ = cv::Scalar_<int>(scalar3_);
	assert(scalar6_.val[0] == -1 && scalar6_.val[1] == -3 && scalar6_.val[2] == -4 && scalar6_[3] == -5);
	cv::Scalar scalar7_ = scalar2_ * 1.5;
	scalar1_ = scalar2_ + scalar5_;
	scalar2_ *= -6;
	scalar3_ = scalar1_ * scalar7_;
	cv::Scalar_<float> scalar8_(2.2f, 3.3f, 4.4f, 5.5f);
	cv::Scalar_<float> scalar9_;
	float f1_ = 1.1f;
	scalar9_ = cv::Scalar_<float>(scalar8_) / float(f1_);
	cv::Scalar_<float> scalar10_;
	int i_ = 2;
	scalar10_ = scalar8_ / i_;

	assert(scalar1.val[0] == scalar1_.val[0] && scalar1.val[1] == scalar1_.val[1] && scalar1.val[2] == scalar1_.val[2] && scalar1.val[3] == scalar1_.val[3]);
	assert(scalar2.val[0] == scalar2_.val[0] && scalar2.val[1] == scalar2_.val[1] && scalar2.val[2] == scalar2_.val[2] && scalar2.val[3] == scalar2_.val[3]);
	assert(scalar3.val[0] == scalar3_.val[0] && scalar3.val[1] == scalar3_.val[1] && scalar3.val[2] == scalar3_.val[2] && scalar3.val[3] == scalar3_.val[3]);
	assert(scalar4.val[0] == scalar4_.val[0] && scalar4.val[1] == scalar4_.val[1] && scalar4.val[2] == scalar4_.val[2] && scalar4.val[3] == scalar4_.val[3]);
	assert(scalar5.val[0] == scalar5_.val[0] && scalar5.val[1] == scalar5_.val[1] && scalar5.val[2] == scalar5_.val[2] && scalar5.val[3] == scalar5_.val[3]);
	assert(scalar6.val[0] == scalar6_.val[0] && scalar6.val[1] == scalar6_.val[1] && scalar6.val[2] == scalar6_.val[2] && scalar6.val[3] == scalar6_.val[3]);
	assert(scalar7.val[0] == scalar7_.val[0] && scalar7.val[1] == scalar7_.val[1] && scalar7.val[2] == scalar7_.val[2] && scalar7.val[3] == scalar7_.val[3]);
	assert(scalar8.val[0] == scalar8_.val[0] && scalar8.val[1] == scalar8_.val[1] && scalar8.val[2] == scalar8_.val[2] && scalar8.val[3] == scalar8_.val[3]);
	assert(scalar9.val[0] == scalar9_.val[0] && scalar9.val[1] == scalar9_.val[1] && scalar9.val[2] == scalar9_.val[2] && scalar9.val[3] == scalar9_.val[3]);
	assert(scalar10.val[0] == scalar10_.val[0] && scalar10.val[1] == scalar10_.val[1] && scalar10.val[2] == scalar10_.val[2] && scalar10.val[3] == scalar10_.val[3]);

	return 0;
}

static void mat_dump(const cv::Mat& matSrc, cv::Mat& matDst)
{
	assert((matSrc.rows == matDst.rows) && (matSrc.cols == matDst.cols));

	const unsigned char* p1 = matSrc.data;
	unsigned char* p2 = matDst.data;
	int count = 0;

	for (int i = 0; i < matSrc.rows; i++) {
		for (int j = 0; j < matSrc.cols * 3; j++) {
			p2[count] = 255 - p1[count];
			count++;
		}
	}
}

static void mat_dump(const fbc::Mat3BGR& matSrc, fbc::Mat3BGR& matDst)
{
	assert((matSrc.rows == matDst.rows) && (matSrc.cols == matDst.cols));

	const unsigned char* p1 = matSrc.data;
	unsigned char* p2 = matDst.data;
	int count = 0;

	for (int i = 0; i < matSrc.rows; i++) {
		for (int j = 0; j < matSrc.cols * 3; j++) {
			p2[count] = 255 - p1[count];
			count++;
		}
	}
}

int test_Mat()
{
#ifdef _MSC_VER
	cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat mat = cv::imread("test_images/lena.png", 1);
#endif
	if (!mat.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::resize(mat, mat, cv::Size(100, 100));

	fbc::Mat4BGRA mat1;
	assert(mat1.total() == 0);
	assert(mat1.empty() == true);
	fbc::Mat1Gray mat2(111, 111);
	assert(mat2.total() == 111*111);
	assert(mat2.empty() == false);
	fbc::Mat3BGR mat3(5, 111, fbc::Scalar(128, 128, 255));
	fbc::Mat3BGR mat4(100, 100, mat.data);
	bool flag1 = mat4.isSubmatrix();
	assert(flag1 == false);
	bool flag_isContinuous1 = mat4.isContinuous();
	assert(flag_isContinuous1 == true);
	fbc::Mat3BGR mat5(mat4); // mat5分配了新的空间,注意与mat5_的不同
	fbc::Mat3BGR mat6;
	mat6 = mat4; // mat6分配了新的空间,注意与mat6_的不同
	fbc::Mat3BGR mat7(100, 100);
	mat_dump(mat4, mat7);
	fbc::Mat3BGR mat8;
	mat4.copyTo(mat8);
	fbc::Mat3BGR mat9; // mat9可能会分配空间，但mat10不会
	mat8.copyTo(mat9, fbc::Rect(20, 10, 40, 45));
	const fbc::uchar* p1 = mat8.ptr(50);
	fbc::Mat3BGR mat10;
	mat8.getROI(mat10, fbc::Rect(20, 10, 40, 45));
	bool flag2 = mat10.isSubmatrix();
	assert(flag2 == true);
	bool flag_isContinuous2 = mat10.isContinuous();
	assert(flag_isContinuous2 == false);
	mat10.setTo(fbc::Scalar::all(128));
	fbc::Size size_mat10;
	fbc::Point point_mat10;
	mat10.locateROI(size_mat10, point_mat10);
	fbc::Mat_<float, 3> mat11;
	mat7.convertTo(mat11, 0.5, fbc::Scalar(10, 20, 30, 40));
	fbc::Mat3BGR mat14;
	mat11.convertTo(mat14);
	fbc::Mat_<float, 3> mat12;
	mat12.zeros(50, 100);
	fbc::Mat3BGR mat13(200, 200);
	fbc::Size size1 = mat13.size();
	size_t elemSize1 = mat11.elemSize();
	int dtop = 9, dbottom = 7, dleft = 5, dright = 11;
	size_t addr_before = size_t(mat10.data);
	mat10.adjustROI(dtop, dbottom, dleft,dright);
	size_t addr_after = size_t(mat10.data);
	size_t length = addr_before - addr_after;

	cv::Mat mat1_;
	cv::Mat mat2_(111, 111, CV_8UC1);
	cv::Mat mat3_(5, 111, CV_8UC3, cv::Scalar(128, 128, 255));
	cv::Mat mat4_(100, 100, CV_8UC3, mat.data);
	bool flag1_ = mat4_.isSubmatrix();
	assert(flag1 == flag1_);
	bool flag_isContinuous1_ = mat4_.isContinuous();
	assert(flag_isContinuous1 == flag_isContinuous1_);
	cv::Mat mat5_(mat4_); // mat5并未分配新的空间
	cv::Mat mat6_;
	mat6_ = mat4_; // mat6并未分配新的空间
	cv::Mat mat7_(100, 100, CV_8UC3);
	mat_dump(mat4_, mat7_);
	cv::Mat mat8_;
	mat4_.copyTo(mat8_);
	uchar* p1_ = mat8_.ptr(50);
	cv::Mat mat10_;
	mat10_ = mat8_.rowRange(cv::Range(10, 55));
	cv::Mat mat10__ = mat10_.colRange(cv::Range(20, 60));
	bool flag2_ = mat10__.isSubmatrix();
	assert(flag2 == flag2_);
	bool flag_isContinuous2_ = mat10__.isContinuous();
	assert(flag_isContinuous2 == flag_isContinuous2_);
	cv::Size size_mat10__;
	cv::Point point_mat10__;
	mat10__.locateROI(size_mat10__, point_mat10__);
	assert(size_mat10.width == size_mat10__.width && size_mat10.height == size_mat10__.height);
	assert(point_mat10.x == point_mat10__.x && point_mat10.y == point_mat10__.y);
	cv::Mat mat11_;
	mat7_.convertTo(mat11_, CV_32FC3);
	cv::Mat mat12_ = cv::Mat::zeros(50, 100, CV_32FC3);
	size_t addr_before_ = size_t(mat10__.data);
	mat10__.adjustROI(dtop, dbottom, dleft, dright);
	size_t addr_after_ = size_t(mat10__.data);
	size_t length_ = addr_before_ - addr_after_;
	assert(length == length_);
	assert(mat10.rows == mat10__.rows && mat10.cols == mat10__.cols && mat10.step == mat10_.step);

	return 0;
}

int test_RotateRect()
{
	float angle = 99.9;

	fbc::Point2f point2f(111.5, 222.7);
	fbc::Size2f size2f(333.3, 555.5);
	fbc::Point2f vertices[4];
	fbc::Rect rect;

	fbc::RotatedRect rotate_rect(point2f, size2f, angle);
	rotate_rect.points(vertices);
	rect = rotate_rect.boundingRect();

	cv::Point2f point2f_(111.5, 222.7);
	cv::Size2f size2f_(333.3, 555.5);
	cv::Point2f vertices_[4];
	cv::Rect rect_;

	cv::RotatedRect rotate_rect_(point2f_, size2f_, angle);
	rotate_rect_.points(vertices_);
	rect_ = rotate_rect_.boundingRect();

	for (int i = 0; i < 4; i++) {
		assert(vertices[i].x == vertices_[i].x && vertices[i].y == vertices_[i].y);
	}
	assert(rect.x == rect_.x && rect.y == rect_.y && rect.width == rect_.width && rect.height == rect_.height);

	return 0;
}

