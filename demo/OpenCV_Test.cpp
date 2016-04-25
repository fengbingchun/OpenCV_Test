#include <iostream>
#include <string>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/base.hpp>

int main()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");
	cvRound
	//fbc::ImageBGR image;
	//image.data = matSrc.data;
	//image.width = matSrc.cols;
	//image.height = matSrc.rows;
	//image.stride = matSrc.step;
	cv::Vec2i vec1(2, 3), vec2(4, 5), vec3;
	vec3 = vec1.mul(vec2);
	cv::Point3d pt;
	cv::namedWindow("show image");
	cv::imshow("show image", matSrc);
	cv::waitKey(0);
	cv::destroyWindow("show image");

	std::cout << "ok" << std::endl;
	return 0;
}

