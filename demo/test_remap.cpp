#include "test_remap.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>
#include <remap.hpp>

static void update_map(const cv::Mat& src, cv::Mat& map_x, cv::Mat& map_y, int ind_)
{
	int ind = ind_ % 4;

	for (int j = 0; j < src.rows; j++) {
		for (int i = 0; i < src.cols; i++) {
			switch (ind) {
				case 0:
					if (i > src.cols*0.25 && i < src.cols*0.75 && j > src.rows*0.25 && j < src.rows*0.75) {
						map_x.at<float>(j, i) = 2 * (i - src.cols*0.25) + 0.5;
						map_y.at<float>(j, i) = 2 * (j - src.rows*0.25) + 0.5;
					} else {
						map_x.at<float>(j, i) = 0;
						map_y.at<float>(j, i) = 0;
					}
					break;
				case 1:
					map_x.at<float>(j, i) = i;
					map_y.at<float>(j, i) = src.rows - j;
					break;
				case 2:
					map_x.at<float>(j, i) = src.cols - i;
					map_y.at<float>(j, i) = j;
					break;
				case 3:
					map_x.at<float>(j, i) = src.cols - i;
					map_y.at<float>(j, i) = src.rows - j;
					break;
			} // end of switch
		}
	}
}

int test_remap_uchar()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	int ind = 0;
	cv::Mat map_x, map_y;
	map_x.create(matSrc.size(), CV_32FC1);
	map_y.create(matSrc.size(), CV_32FC1);

	update_map(matSrc, map_x, map_y, ind);

	int width = matSrc.cols;
	int height = matSrc.rows;

	fbc::Mat_<fbc::uchar, 3> mat1(height, width, matSrc.data);
	fbc::Mat_<float, 1> mapX(height, width, map_x.data);
	fbc::Mat_<float, 1> mapY(height, width, map_y.data);
	fbc::Mat_<fbc::uchar, 3> mat2(height, width);

	fbc::remap(mat1, mat2, mapX, mapY, 0, 0, fbc::Scalar::all(0));

	cv::Mat mat2_ = cv::Mat(height, width, CV_8UC3);
	cv::remap(matSrc, mat2_, map_x, map_y, 0, 0, cv::Scalar::all(0));

	assert(mat2.step == mat2_.step);
	for (int y = 0; y < mat2.rows; y++) {
		const fbc::uchar* p = mat2.ptr(y);
		const uchar* p_ = mat2_.ptr(y);

		for (int x = 0; x < mat2.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_remap_float()
{

	return 0;
}
