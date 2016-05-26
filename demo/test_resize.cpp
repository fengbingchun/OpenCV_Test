#include <core/mat.hpp>
#include <resize.hpp>

#include <opencv2/opencv.hpp>

#include "test_resize.hpp"

int test_resize()
{
	cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!mat.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	fbc::Mat3BGR mat1(mat.rows, mat.cols, mat.data);
	fbc::Mat3BGR mat2(mat1);
	fbc::Mat3BGR mat3(100, 200);
	fbc::resize(mat2, mat3, fbc::INTER_NEAREST);


	cv::Mat mat1_(mat.rows, mat.cols, CV_8UC3, mat.data);
	cv::Mat mat2_(mat1_);
	cv::Mat mat3_(100, 200, CV_8SC3);
	cv::resize(mat2_, mat3_, cv::Size(100, 100), 0, 0, 0);

	

	return 0;
}
