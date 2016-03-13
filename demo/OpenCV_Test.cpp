#include <iostream>
#include <string>
#include <fstream>

#include <opencv2/opencv.hpp>


int main()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");

	cv::namedWindow("show image");
	cv::imshow("show image", matSrc);
	cv::waitKey(0);
	cv::destroyWindow("show image");

	std::cout << "ok" << std::endl;
	return 0;
}

