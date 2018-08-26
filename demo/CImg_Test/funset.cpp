#include "funset.hpp"
#include <iostream>
#include "CImg.h"

int test_CImg_1()
{
	// 定义一个每个颜色 8 位(bit)的 640x400 的彩色图像
	cimg_library::CImg<unsigned char> img(640, 400, 1, 3);
	//将像素值设为 0（黑色）
	img.fill(0);
	// 定义一个紫色
	unsigned char purple[] = { 255, 0, 255 };

	// 在坐标(100, 100)处画一个紫色的“Hello world”
	img.draw_text(100, 100, "Hello World", purple);
	// 在一个标题为“My first CImg code”的窗口中显示这幅图像
	img.display("My first CImg code");

	std::cout << "ok!" << std::endl;
	return 0;
}

