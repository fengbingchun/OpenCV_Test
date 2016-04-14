#include <iostream>
#include <string>
#include <fstream>
#include "base.hpp"

#include <opencv2/opencv.hpp>


int main()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");

	fbc::ImageBGR image;
	image.data = matSrc.data;
	image.width = matSrc.cols;
	image.height = matSrc.rows;
	image.stride = matSrc.step;

	cv::namedWindow("show image");
	cv::imshow("show image", matSrc);
	cv::waitKey(0);
	cv::destroyWindow("show image");

	std::cout << "ok" << std::endl;
	return 0;
}

