#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

int main()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");

	std::cout << "ok" << std::endl;
	return 0;
}

