#include <assert.h>
#include <core/mat.hpp>
#include <resize.hpp>

#include <opencv2/opencv.hpp>

#include "fbc_cv_funset.hpp"

// Blog: http://blog.csdn.net/fengbingchun/article/details/51622450

int test_resize_uchar()
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

	int width = 23, height = 11;

	for (int inter = 0; inter < 5; inter++) {
		fbc::Mat3BGR mat1(mat.rows, mat.cols, mat.data);
		fbc::Mat3BGR mat2(mat1);
		fbc::Mat3BGR mat3(height, width);
		fbc::resize(mat2, mat3, inter);

		cv::Mat mat1_(mat.rows, mat.cols, CV_8UC3, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(height, width, CV_8UC3);
		cv::resize(mat2_, mat3_, cv::Size(width, height), 0, 0, inter);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_resize_float()
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
	int width = 623, height = 711;
	cv::cvtColor(mat, mat, CV_BGR2GRAY);
	mat.convertTo(mat, CV_32FC1);

	for (int inter = 0; inter < 5; inter++) {
		fbc::Mat_<float, 1> mat1(mat.rows, mat.cols, mat.data);
		fbc::Mat_<float, 1> mat2(mat1);
		fbc::Mat_<float, 1> mat3(height, width);
		fbc::resize(mat2, mat3, inter);

		cv::Mat mat1_(mat.rows, mat.cols, CV_32FC1, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(height, width, CV_32FC1);
		cv::resize(mat2_, mat3_, cv::Size(width, height), 0, 0, inter);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_resize_area()
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
	fbc::Size size[3] = {fbc::Size(123, 111), fbc::Size(256, 256), fbc::Size(723, 817)};

	for (int i = 0; i < 3; i++) {
		fbc::Mat3BGR mat1(mat.rows, mat.cols, mat.data);
		fbc::Mat3BGR mat2(mat1);
		fbc::Mat3BGR mat3(size[i].height, size[i].width);
		fbc::resize(mat2, mat3, 3);

		cv::Mat mat1_(mat.rows, mat.cols, CV_8UC3, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(size[i].height, size[i].width, CV_8UC3);
		cv::resize(mat2_, mat3_, cv::Size(size[i].width, size[i].height), 0, 0, 3);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	cv::Mat matf;
	cv::cvtColor(mat, matf, CV_BGR2GRAY);
	matf.convertTo(matf, CV_32FC1);

	for (int i = 0; i < 3; i++) {
		fbc::Mat_<float, 1> mat1(mat.rows, mat.cols, matf.data);
		fbc::Mat_<float, 1> mat2(mat1);
		fbc::Mat_<float, 1> mat3(size[i].height, size[i].width);
		fbc::resize(mat2, mat3, 3);

		cv::Mat mat1_(mat.rows, mat.cols, CV_32FC1, matf.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(size[i].height, size[i].width, CV_32FC1);
		cv::resize(mat2_, mat3_, cv::Size(size[i].width, size[i].height), 0, 0, 3);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}
