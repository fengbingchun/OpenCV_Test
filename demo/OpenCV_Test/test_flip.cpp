#include "fbc_cv_funset.hpp"
#include <assert.h>
#include <iostream>
#include <string>

#include <flip.hpp>
#include <opencv2/opencv.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/52554409

int test_flip_uchar()
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

	int width = matSrc.cols;
	int height = matSrc.rows;

	int flipCode[3] {-1, 0, 1}; // both axes, x axis, y axis

	for (int i = 0; i < 3; i++) {
		fbc::Mat_<uchar, 3> mat1(height, width, matSrc.data);
		fbc::Mat_<uchar, 3> mat2(height, width);
		fbc::flip(mat1, mat2, flipCode[i]);

		cv::Mat mat1_(height, width, CV_8UC3, matSrc.data);
		cv::Mat mat2_;
		cv::flip(mat1_, mat2_, flipCode[i]);

		assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
		for (int y = 0; y < mat2.rows; y++) {
			const fbc::uchar* p1 = mat2.ptr(y);
			const uchar* p2 = mat2_.ptr(y);

			for (int x = 0; x < mat2.step; x++) {
				assert(p1[x] == p2[x]);
			}
		}

		std::string name = std::to_string(i);
#ifdef _MSC_VER
		std::string file_path = "E:/GitCode/OpenCV_Test/test_images/";
#else
		std::string file_path = "test_images/";
#endif
		std::string name_fbc = file_path + "flip_fbc_" + name + ".jpg";
		std::string name_cv = file_path + "flip_cv_" + name + ".jpg";
		cv::Mat matSave(height, width, CV_8UC3, mat2.data);
		cv::imwrite(name_fbc, matSave);
		cv::imwrite(name_cv, mat2_);
	}

	return 0;
}

int test_flip_float()
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
	cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);
	matSrc.convertTo(matSrc, CV_32FC1);

	int width = matSrc.cols;
	int height = matSrc.rows;

	int flipCode[3] {-1, 0, 1}; // both axes, x axis, y axis

	for (int i = 0; i < 3; i++) {
		fbc::Mat_<float, 1> mat1(height, width, matSrc.data);
		fbc::Mat_<float, 1> mat2(height, width);
		fbc::flip(mat1, mat2, flipCode[i]);

		cv::Mat mat1_(height, width, CV_32FC1, matSrc.data);
		cv::Mat mat2_;
		cv::flip(mat1_, mat2_, flipCode[i]);

		assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
		for (int y = 0; y < mat2.rows; y++) {
			const fbc::uchar* p1 = mat2.ptr(y);
			const uchar* p2 = mat2_.ptr(y);

			for (int x = 0; x < mat2.step; x++) {
				assert(p1[x] == p2[x]);
			}
		}
	}

	return 0;
}
