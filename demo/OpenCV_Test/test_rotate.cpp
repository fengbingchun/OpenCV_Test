#include "fbc_cv_funset.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>
#include <rotate.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/51923769

int test_getRotationMatrix2D()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/lena.png", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	double angle = -50.0;
	double scale = 0.6;

	fbc::Point2f center = fbc::Point2f(matSrc.cols / 2, matSrc.rows / 2);
	fbc::Mat_<double, 1> mat_rot(2, 3);
	fbc::getRotationMatrix2D(center, angle, scale, mat_rot);

	// Compute a rotation matrix with respect to the center of the image
	cv::Point center_ = cv::Point(matSrc.cols / 2, matSrc.rows / 2);
	// Get the rotation matrix with the specifications above
	cv::Mat mat_rot_ = cv::getRotationMatrix2D(center_, angle, scale);

	assert(mat_rot.cols == mat_rot_.cols && mat_rot.rows == mat_rot_.rows);
	assert(mat_rot.step == mat_rot_.step);
	for (int y = 0; y < mat_rot.rows; y++) {
		const fbc::uchar* p = mat_rot.ptr(y);
		const uchar* p_ = mat_rot_.ptr(y);

		for (int x = 0; x < mat_rot.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_rotate_uchar()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/lena.png", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	double angle = -50.0;

	for (int interpolation = 0; interpolation < 5; interpolation++) {
		fbc::Point2f center = fbc::Point2f(matSrc.cols / 2.0, matSrc.rows / 2.0);
		fbc::Mat_<uchar, 3> mat(matSrc.rows, matSrc.cols, matSrc.data);
		fbc::Mat_<uchar, 3> rotate_dst;
		fbc::rotate(mat, rotate_dst, center, angle, true, interpolation);

		// Compute a rotation matrix with respect to the center of the image
		cv::Point2f center_ = cv::Point2f(matSrc.cols / 2.0, matSrc.rows / 2.0);

		// Get the rotation matrix with the specifications above
		cv::Mat mat_rot_ = getRotationMatrix2D(center_, angle, 1.0);
		cv::Mat rotate_dst_;

		cv::warpAffine(matSrc, rotate_dst_, mat_rot_, matSrc.size(), interpolation);

		assert(rotate_dst.step == rotate_dst_.step && rotate_dst.rows == rotate_dst_.rows);
		for (int y = 0; y < rotate_dst.rows; y++) {
			const fbc::uchar* p = rotate_dst.ptr(y);
			const uchar* p_ = rotate_dst_.ptr(y);

			for (int x = 0; x < rotate_dst.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_rotate_float()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/lena.png", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);
	matSrc.convertTo(matSrc, CV_32FC1);

	double angle = -50.0;

	for (int interpolation = 0; interpolation < 5; interpolation++) {
		fbc::Point2f center = fbc::Point2f(matSrc.cols / 2.0, matSrc.rows / 2.0);
		fbc::Mat_<float, 1> mat(matSrc.rows, matSrc.cols, matSrc.data);
		fbc::Mat_<float, 1> rotate_dst;
		fbc::rotate(mat, rotate_dst, center, angle, true, interpolation);

		// Compute a rotation matrix with respect to the center of the image
		cv::Point2f center_ = cv::Point2f(matSrc.cols / 2.0, matSrc.rows / 2.0);

		// Get the rotation matrix with the specifications above
		cv::Mat mat_rot_ = getRotationMatrix2D(center_, angle, 1.0);
		cv::Mat rotate_dst_;

		cv::warpAffine(matSrc, rotate_dst_, mat_rot_, matSrc.size(), interpolation);

		assert(rotate_dst.step == rotate_dst_.step && rotate_dst.rows == rotate_dst_.rows);
		for (int y = 0; y < rotate_dst.rows; y++) {
			const fbc::uchar* p = rotate_dst.ptr(y);
			const uchar* p_ = rotate_dst_.ptr(y);

			for (int x = 0; x < rotate_dst.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_rotate_without_crop()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/1.jpg", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	double angle = -50.0;
	double scale = 0.6;

	fbc::Point2f center = fbc::Point2f(matSrc.cols / 2.0, matSrc.rows / 2.0);
	fbc::Mat_<uchar, 3> mat(matSrc.rows, matSrc.cols, matSrc.data);
	fbc::Mat_<uchar, 3> rotate_dst;
	fbc::rotate(mat, rotate_dst, center, angle, true/*false*/, 2, 0, fbc::Scalar(128, 255, 0));

	cv::Mat mat_save(rotate_dst.rows, rotate_dst.cols, CV_8UC3, rotate_dst.data);
#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/1_rotate2.jpg", mat_save);
#else
	cv::imwrite("test_images/1_rotate2.jpg", mat_save);
#endif

	return 0;
}
