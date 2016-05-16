#include <iostream>
#include <string>
#include <fstream>

#include "test_core.hpp"

int main()
{
	//cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");
	////cvRound
	////fbc::ImageBGR image;
	////image.data = matSrc.data;
	////image.width = matSrc.cols;
	////image.height = matSrc.rows;
	////image.stride = matSrc.step;
	//cv::Vec2i vec1(2, 3), vec2(4, 5), vec3;
	////vec3 = vec1.mul(vec2);
	////cv::Point3d pt;
	//cv::Mat mat = cv::Mat(10, 10, CV_8UC1);
	//cv::namedWindow("show image");
	//cv::imshow("show image", matSrc);
	//cv::waitKey(0);
	//cv::destroyWindow("show image");
	test_Mat();

	std::cout << "ok" << std::endl;
	return 0;
}

