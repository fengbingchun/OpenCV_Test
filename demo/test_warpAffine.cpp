#include "test_warpAffine.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>
#include <core/mat.hpp>
#include <warpAffine.hpp>

int test_getAffineTransform()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	fbc::Point2f srcTri[3];
	fbc::Point2f dstTri[3];

	// Set your 3 points to calculate the  Affine Transform
	srcTri[0] = fbc::Point2f(0, 0);
	srcTri[1] = fbc::Point2f(matSrc.cols - 1, 0);
	srcTri[2] = fbc::Point2f(0, matSrc.rows - 1);

	dstTri[0] = fbc::Point2f(matSrc.cols*0.0, matSrc.rows*0.33);
	dstTri[1] = fbc::Point2f(matSrc.cols*0.85, matSrc.rows*0.25);
	dstTri[2] = fbc::Point2f(matSrc.cols*0.15, matSrc.rows*0.7);

	// Get the Affine Transform
	fbc::Mat_<double, 1> warp_mat(2, 3);
	int ret = fbc::getAffineTransform(srcTri, dstTri, warp_mat);
	assert(ret == 0);

	cv::Point2f srcTri_[3];
	cv::Point2f dstTri_[3];

	// Set your 3 points to calculate the  Affine Transform
	srcTri_[0] = cv::Point2f(0, 0);
	srcTri_[1] = cv::Point2f(matSrc.cols - 1, 0);
	srcTri_[2] = cv::Point2f(0, matSrc.rows - 1);

	dstTri_[0] = cv::Point2f(matSrc.cols*0.0, matSrc.rows*0.33);
	dstTri_[1] = cv::Point2f(matSrc.cols*0.85, matSrc.rows*0.25);
	dstTri_[2] = cv::Point2f(matSrc.cols*0.15, matSrc.rows*0.7);

	// Get the Affine Transform
	cv::Mat warp_mat_(2, 3, CV_64FC1);
	warp_mat_ = cv::getAffineTransform(srcTri_, dstTri_);

	assert(warp_mat.cols == warp_mat_.cols && warp_mat.rows == warp_mat_.rows);
	assert(warp_mat.step == warp_mat_.step);
	for (int y = 0; y < warp_mat.rows; y++) {
		const fbc::uchar* p = warp_mat.ptr(y);
		const uchar* p_ = warp_mat_.ptr(y);

		for (int x = 0; x < warp_mat.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}
