#include "test_core.hpp"
#include <core/mat.hpp>
#include <opencv2/opencv.hpp>

int test_mat()
{
	cv::Mat mat1_1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!mat1_1.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	//fbc::Mat_<fbc::uchar, 3> mat2_1;//(mat1_1.rows, mat1_1.cols);

	return 0;
}

int test_point()
{
	fbc::Point2i point;
	point.x = point.y = 10;

	return 0;
}

