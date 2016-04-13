#include <iostream>
#include <string>
#include <fstream>
#include "base.hpp"

#include <opencv2/opencv.hpp>


int main()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");

	fbc::Point2i pt1, pt2, pt3;
	pt1 = fbc::Point2i(1, 2);
	pt2 = fbc::Point2i(3, 4);
	pt3 = pt1 + pt2;

	cv::namedWindow("show image");
	cv::imshow("show image", matSrc);
	cv::waitKey(0);
	cv::destroyWindow("show image");

	std::cout << "ok" << std::endl;
	return 0;
}

