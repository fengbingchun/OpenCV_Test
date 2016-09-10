#include "test_transpose.hpp"
#include <assert.h>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <transpose.hpp>

int test_transpose_uchar()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	int width = matSrc.cols;
	int height = matSrc.rows;

	fbc::Mat_<uchar, 3> mat1(height, width, matSrc.data);
	fbc::Mat_<uchar, 3> mat2(height, width);
	fbc::transpose(mat1, mat2);

	cv::Mat mat1_(height, width, CV_8UC3, matSrc.data);
	cv::Mat mat2_;
	cv::transpose(mat1_, mat2_);

	assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
	for (int y = 0; y < mat2.rows; y++) {
		const fbc::uchar* p1 = mat2.ptr(y);
		const uchar* p2 = mat2_.ptr(y);

		for (int x = 0; x < mat2.step; x++) {
			//assert(p1[x] == p2[x]);
		}
	}

	cv::Mat matSave(height, width, CV_8UC3, mat2.data);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/transpose.jpg", matSave);

	return 0;
}

int test_transpose_float()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);
	matSrc.convertTo(matSrc, CV_32FC1);

	int width = matSrc.cols;
	int height = matSrc.rows;

	fbc::Mat_<float, 1> mat1(height, width, matSrc.data);
	fbc::Mat_<float, 1> mat2(height, width);
	fbc::transpose(mat1, mat2);

	cv::Mat mat1_(height, width, CV_32FC1, matSrc.data);
	cv::Mat mat2_;
	cv::transpose(mat1_, mat2_);

	assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
	for (int y = 0; y < mat2.rows; y++) {
		const fbc::uchar* p1 = mat2.ptr(y);
		const uchar* p2 = mat2_.ptr(y);

		for (int x = 0; x < mat2.step; x++) {
			//assert(p1[x] == p2[x]);
		}
	}

	return 0;
}

