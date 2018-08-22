#include "fbc_cv_funset.hpp"
#include <assert.h>

#include <threshold.hpp>
#include <opencv2/opencv.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/52494129

int test_threshold_uchar()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/lena.png", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);

	int width = matSrc.cols;
	int height = matSrc.rows;
	int types[8] = {0, 1, 2, 3, 4, 7, 8, 16};

	for (int i = 0; i < 8; i++) {
		if (types[i] == 7) continue;
		double thresh = 135.0;
		double maxval = 255.0;

		fbc::Mat_<uchar, 1> mat1(height, width, matSrc.data);
		fbc::Mat_<uchar, 1> mat2(height, width);
		fbc::threshold(mat1, mat2, thresh, maxval, types[i]);

		cv::Mat mat1_(height, width, CV_8UC1, matSrc.data);
		cv::Mat mat2_;
		cv::threshold(mat1_, mat2_, thresh, maxval, types[i]);

		assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
		for (int y = 0; y < mat2.rows; y++) {
			const fbc::uchar* p1 = mat2.ptr(y);
			const uchar* p2 = mat2_.ptr(y);

			for (int x = 0; x < mat2.step; x++) {
				assert(p1[x] == p2[x]);
			}
		}
	}

	return 0;
}

int test_threshold_float()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/lena.png", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);
	matSrc.convertTo(matSrc, CV_32FC1);

	int width = matSrc.cols;
	int height = matSrc.rows;
	int types[6] = { 0, 1, 2, 3, 4, 7 };

	for (int i = 0; i < 6; i++) {
		if (types[i] == 7) continue;
		double thresh = 135.0;
		double maxval = 255.0;

		fbc::Mat_<float, 1> mat1(height, width, matSrc.data);
		fbc::Mat_<float, 1> mat2(height, width);
		fbc::threshold(mat1, mat2, thresh, maxval, types[i]);

		cv::Mat mat1_(height, width, CV_32FC1, matSrc.data);
		cv::Mat mat2_;
		cv::threshold(mat1_, mat2_, thresh, maxval, types[i]);

		assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
		for (int y = 0; y < mat2.rows; y++) {
			const fbc::uchar* p1 = mat2.ptr(y);
			const uchar* p2 = mat2_.ptr(y);

			for (int x = 0; x < mat2.step; x++) {
				assert(p1[x] == p2[x]);
			}
		}
	}

	return 0;
}
