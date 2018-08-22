#include <assert.h>
#include <vector>
#include <core/mat.hpp>
#include <merge.hpp>

#include <opencv2/opencv.hpp>

#include "fbc_cv_funset.hpp"

// Blog: http://blog.csdn.net/fengbingchun/article/details/51762027

int test_merge_uchar()
{
#ifdef _MSC_VER
	cv::Mat matSrc1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	cv::Mat matSrc2 = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
	cv::Mat matSrc3 = cv::imread("E:/GitCode/OpenCV_Test/test_images/2.jpg", 1);
#else
	cv::Mat matSrc1 = cv::imread("test_images/lena.png", 1);
	cv::Mat matSrc2 = cv::imread("test_images/1.jpg", 1);
	cv::Mat matSrc3 = cv::imread("test_images/2.jpg", 1);
#endif
	if (!matSrc1.data || !matSrc2.data || !matSrc3.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	int width = 500, height = 600;

	cv::cvtColor(matSrc1, matSrc1, CV_BGR2GRAY);
	cv::cvtColor(matSrc2, matSrc2, CV_BGR2GRAY);
	cv::cvtColor(matSrc3, matSrc3, CV_BGR2GRAY);
	cv::resize(matSrc1, matSrc1, cv::Size(width, height));
	cv::resize(matSrc2, matSrc2, cv::Size(width, height));
	cv::resize(matSrc3, matSrc3, cv::Size(width, height));

	fbc::Mat_<fbc::uchar, 1> mat1(height, width, matSrc1.data);
	fbc::Mat_<fbc::uchar, 1> mat2(height, width, matSrc2.data);
	fbc::Mat_<fbc::uchar, 1> mat3(height, width, matSrc3.data);
	std::vector<fbc::Mat_<fbc::uchar, 1>> mat;
	mat.push_back(mat1);
	mat.push_back(mat2);
	mat.push_back(mat3);
	fbc::Mat_<fbc::uchar, 3> matDst(height, width);
	fbc::merge(mat, matDst);

	std::vector<cv::Mat> mat_;
	mat_.push_back(matSrc1);
	mat_.push_back(matSrc2);
	mat_.push_back(matSrc3);
	cv::Mat matDst_;
	cv::merge(mat_, matDst_);

	assert(matDst.channels == matDst_.channels());
	assert((matDst.rows == matDst_.rows) && (matDst.cols == matDst_.cols));
	assert(matDst.step == matDst_.step);

	for (int i = 0; i < matDst.rows; i++) {
		const fbc::uchar* p1 = matDst.ptr(i);
		const uchar* p2 = matDst_.ptr(i);

		for (int j = 0; j < matDst.step; j++) {
			assert(p1[j] == p2[j]);
		}
	}

	return 0;
}

int test_merge_float()
{
#ifdef _MSC_VER
	cv::Mat matSrc1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	cv::Mat matSrc2 = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
	cv::Mat matSrc3 = cv::imread("E:/GitCode/OpenCV_Test/test_images/2.jpg", 1);
#else
	cv::Mat matSrc1 = cv::imread("test_images/lena.png", 1);
	cv::Mat matSrc2 = cv::imread("test_images/1.jpg", 1);
	cv::Mat matSrc3 = cv::imread("test_images/2.jpg", 1);
#endif
	if (!matSrc1.data || !matSrc2.data || !matSrc3.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	int width = 500, height = 600;

	cv::cvtColor(matSrc1, matSrc1, CV_BGR2GRAY);
	cv::cvtColor(matSrc2, matSrc2, CV_BGR2GRAY);
	cv::cvtColor(matSrc3, matSrc3, CV_BGR2GRAY);
	cv::resize(matSrc1, matSrc1, cv::Size(width, height));
	cv::resize(matSrc2, matSrc2, cv::Size(width, height));
	cv::resize(matSrc3, matSrc3, cv::Size(width, height));
	matSrc1.convertTo(matSrc1, CV_32FC1);
	matSrc2.convertTo(matSrc2, CV_32FC1);
	matSrc3.convertTo(matSrc3, CV_32FC1);

	fbc::Mat_<float, 1> mat1(height, width, matSrc1.data);
	fbc::Mat_<float, 1> mat2(height, width, matSrc2.data);
	fbc::Mat_<float, 1> mat3(height, width, matSrc3.data);
	std::vector<fbc::Mat_<float, 1>> mat;
	mat.push_back(mat1);
	mat.push_back(mat2);
	mat.push_back(mat3);
	fbc::Mat_<float, 3> matDst(height, width);
	fbc::merge(mat, matDst);

	std::vector<cv::Mat> mat_;
	mat_.push_back(matSrc1);
	mat_.push_back(matSrc2);
	mat_.push_back(matSrc3);
	cv::Mat matDst_;
	cv::merge(mat_, matDst_);

	assert(matDst.channels == matDst_.channels());
	assert((matDst.rows == matDst_.rows) && (matDst.cols == matDst_.cols));
	assert(matDst.step == matDst_.step);

	for (int i = 0; i < matDst.rows; i++) {
		const fbc::uchar* p1 = matDst.ptr(i);
		const uchar* p2 = matDst_.ptr(i);

		for (int j = 0; j < matDst.step; j++) {
			assert(p1[j] == p2[j]);
		}
	}

	return 0;
}
