#include <assert.h>
#include "test_core.hpp"
#include <core/mat.hpp>
#include <core/fast_math.hpp>
#include <core/base.hpp>
#include <core/saturate.hpp>
#include <opencv2/opencv.hpp>

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

int test_mat()
{
	cv::Mat mat1_1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!mat1_1.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	fbc::Mat_<int, 3> mat2_1;//(mat1_1.rows, mat1_1.cols);

	return 0;
}

int test_point()
{
	fbc::Point2i point;
	point.x = point.y = 10;

	return 0;
}

