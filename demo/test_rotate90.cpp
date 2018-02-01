#include "fbc_cv_funset.hpp"
#include <opencv2/opencv.hpp>
#include <transpose.hpp>
#include <flip.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/52554711

int test_rotate90()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	int width = matSrc.cols;
	int height = matSrc.rows;

	fbc::Mat_<uchar, 3> mat1(height, width, matSrc.data);
	fbc::Mat_<uchar, 3> matTranspose(width, height);
	fbc::transpose(mat1, matTranspose);

	// clockwise rotation  90
	fbc::Mat_<uchar, 3> matRotate90(width, height);
	fbc::flip(matTranspose, matRotate90, 1);
	cv::Mat tmp2(width, height, CV_8UC3, matRotate90.data);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/rotate_90.jpg", tmp2);

	// clockwise rotation 180
	fbc::Mat_<uchar, 3> matRotate180(height, width);
	fbc::flip(mat1, matRotate180, -1);
	cv::Mat tmp3(height, width, CV_8UC3, matRotate180.data);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/rotate_180.jpg", tmp3);

	// clockwise rotation 270
	fbc::Mat_<uchar, 3> matRotate270(width, height);
	fbc::flip(matTranspose, matRotate270, 0);
	cv::Mat tmp4(matTranspose.rows, matTranspose.cols, CV_8UC3, matRotate270.data);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/rotate_270.jpg", tmp4);

	return 0;
}

