#include "fbc_cv_funset.hpp"
#include <assert.h>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <transpose.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/52550460

int test_transpose_uchar()
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

	int width = matSrc.cols;
	int height = matSrc.rows;
	cv::Mat matSrc_;
	cv::resize(matSrc, matSrc_, cv::Size(width, width));

	fbc::Mat_<uchar, 3> mat1(width, width);
	memcpy(mat1.data, matSrc_.data, width * width * 3);
	fbc::transpose(mat1, mat1);

	cv::Mat mat1_(width, width, CV_8UC3);
	memcpy(mat1_.data, matSrc_.data, width * width * 3);
	cv::transpose(mat1_, mat1_);

	assert(mat1.rows == mat1_.rows && mat1.cols == mat1_.cols && mat1.step == mat1_.step);
	for (int y = 0; y < mat1.rows; y++) {
		const fbc::uchar* p1 = mat1.ptr(y);
		const uchar* p2 = mat1_.ptr(y);

		for (int x = 0; x < mat1.step; x++) {
			assert(p1[x] == p2[x]);
		}
	}

	cv::Mat matSave(width, width, CV_8UC3, mat1.data);
#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/transpose_fbc.jpg", matSave);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/transpose_cv.jpg", mat1_);

	cv::Mat matSrc1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
#else
	cv::imwrite("test_images/transpose_fbc.jpg", matSave);
	cv::imwrite("test_images/transpose_cv.jpg", mat1_);

	cv::Mat matSrc1 = cv::imread("test_images/1.jpg", 1);
#endif
	if (!matSrc1.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	width = matSrc1.cols;
	height = matSrc1.rows;

	fbc::Mat_<uchar, 3> mat2(height, width, matSrc1.data);
	fbc::Mat_<uchar, 3> mat3(width, height);
	fbc::transpose(mat2, mat3);

	cv::Mat mat2_(height, width, CV_8UC3, matSrc1.data);
	cv::Mat mat3_;
	cv::transpose(mat2_, mat3_);

	assert(mat3.rows == mat3_.rows && mat3.cols == mat3_.cols && mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p1 = mat3.ptr(y);
		const uchar* p2 = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p1[x] == p2[x]);
		}
	}

	cv::Mat matSave1(width, height, CV_8UC3, mat3.data);
#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/transpose1_fbc.jpg", matSave1);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/transpose1_cv.jpg", mat3_);
#else
	cv::imwrite("test_images/transpose1_fbc.jpg", matSave1);
	cv::imwrite("test_images/transpose1_cv.jpg", mat3_);
#endif
	return 0;
}

int test_transpose_float()
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

	int width = matSrc.cols;
	int height = matSrc.rows;
	cv::Mat matSrc_;
	cv::resize(matSrc, matSrc_, cv::Size(width, width));

	fbc::Mat_<float, 1> mat1(width, width);
	memcpy(mat1.data, matSrc_.data, width * width * sizeof(float));
	fbc::transpose(mat1, mat1);

	cv::Mat mat1_(width, width, CV_32FC1);
	memcpy(mat1_.data, matSrc_.data, width * width * sizeof(float));
	cv::transpose(mat1_, mat1_);

	assert(mat1.rows == mat1_.rows && mat1.cols == mat1_.cols && mat1.step == mat1_.step);
	for (int y = 0; y < mat1.rows; y++) {
		const fbc::uchar* p1 = mat1.ptr(y);
		const uchar* p2 = mat1_.ptr(y);

		for (int x = 0; x < mat1.step; x++) {
			assert(p1[x] == p2[x]);
		}
	}

#ifdef _MSC_VER
	cv::Mat matSrc1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
#else	
	cv::Mat matSrc1 = cv::imread("test_images/1.jpg", 1);
#endif
	if (!matSrc1.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(matSrc1, matSrc1, CV_BGR2GRAY);
	matSrc1.convertTo(matSrc1, CV_32FC1);

	width = matSrc1.cols;
	height = matSrc1.rows;

	fbc::Mat_<float, 1> mat2(height, width, matSrc1.data);
	fbc::Mat_<float, 1> mat3(width, height);
	fbc::transpose(mat2, mat3);

	cv::Mat mat2_(height, width, CV_32FC1, matSrc1.data);
	cv::Mat mat3_;
	cv::transpose(mat2_, mat3_);

	assert(mat3.rows == mat3_.rows && mat3.cols == mat3_.cols && mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p1 = mat3.ptr(y);
		const uchar* p2 = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p1[x] == p2[x]);
		}
	}

	return 0;
}
