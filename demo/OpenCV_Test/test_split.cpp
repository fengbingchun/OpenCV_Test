#include <assert.h>
#include <vector>
#include <core/mat.hpp>
#include <split.hpp>

#include <opencv2/opencv.hpp>

#include "fbc_cv_funset.hpp"

// Blog: http://blog.csdn.net/fengbingchun/article/details/51762027

int test_split_uchar()
{
#ifdef _MSC_VER
	cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat mat = cv::imread("test_images/lena.png", 1);
#endif
	if (!mat.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	//cv::cvtColor(mat, mat, CV_BGR2GRAY);

	int chs = mat.channels();
	int width = mat.cols;
	int height = mat.rows;

	fbc::Mat_<fbc::uchar, 3> mat1(height, width, mat.data);
	std::vector<fbc::Mat_<fbc::uchar, 1>> vecMat2;
	fbc::Mat_<fbc::uchar, 1>* mat2 = new fbc::Mat_<fbc::uchar, 1>[chs];
	for (int i = 0; i < chs; i++) {
		mat2[i] = fbc::Mat_<fbc::uchar, 1>(height, width);
		vecMat2.push_back(mat2[i]);
	}

	fbc::split(mat1, vecMat2);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	std::vector<cv::Mat> vecMat2_;
	cv::split(mat1_, vecMat2_);

	assert(vecMat2.size() == vecMat2_.size());
	for (int i = 0; i < vecMat2.size(); i++) {
		assert(vecMat2[i].rows == vecMat2_[i].rows && vecMat2[i].cols == vecMat2_[i].cols);
		assert(vecMat2[i].step == vecMat2_[i].step);
		assert(vecMat2[i].channels == vecMat2_[i].channels());

		for (int y = 0; y < vecMat2[i].rows; y++) {
			const fbc::uchar* p = vecMat2[i].ptr(y);
			const uchar* p_ = vecMat2_[i].ptr(y);

			for (int x = 0; x < vecMat2[i].step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	delete[] mat2;

	return 0;
}

int test_split_float()
{
#ifdef _MSC_VER
	cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
#else	
	cv::Mat mat = cv::imread("test_images/lena.png", 1);
#endif
	if (!mat.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	mat.convertTo(mat, CV_32FC3);
	//cv::cvtColor(mat, mat, CV_BGR2GRAY);

	int chs = mat.channels();
	int width = mat.cols;
	int height = mat.rows;

	fbc::Mat_<float, 3> mat1(height, width, mat.data);
	std::vector<fbc::Mat_<float, 1>> vecMat2;
	fbc::Mat_<float, 1>* mat2 = new fbc::Mat_<float, 1>[chs];
	for (int i = 0; i < chs; i++) {
		mat2[i] = fbc::Mat_<float, 1>(height, width);
		vecMat2.push_back(mat2[i]);
	}

	fbc::split(mat1, vecMat2);

	cv::Mat mat1_(height, width, CV_32FC3, mat.data);
	std::vector<cv::Mat> vecMat2_;
	cv::split(mat1_, vecMat2_);

	assert(vecMat2.size() == vecMat2_.size());
	for (int i = 0; i < vecMat2.size(); i++) {
		assert(vecMat2[i].rows == vecMat2_[i].rows && vecMat2[i].cols == vecMat2_[i].cols);
		assert(vecMat2[i].step == vecMat2_[i].step);
		assert(vecMat2[i].channels == vecMat2_[i].channels());

		for (int y = 0; y < vecMat2[i].rows; y++) {
			const fbc::uchar* p = vecMat2[i].ptr(y);
			const uchar* p_ = vecMat2_[i].ptr(y);

			for (int x = 0; x < vecMat2[i].step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	delete[] mat2;

	return 0;
}
