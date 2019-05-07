#include "fbc_cv_funset.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>
#include <core/mat.hpp>
#include <warpAffine.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/51923392

int test_getAffineTransform()
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

int test_warpAffine_uchar()
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

	for (int interpolation = 0; interpolation < 5; interpolation++) {
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

		fbc::Mat_<uchar, 3> mat(matSrc.rows, matSrc.cols, matSrc.data);
		fbc::Mat_<uchar, 3> warp_dst;
		warp_dst.zeros(mat.rows, mat.cols);
		fbc::warpAffine(mat, warp_dst, warp_mat, interpolation);

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

		// Set the dst image the same type and size as src
		cv::Mat warp_dst_ = cv::Mat::zeros(matSrc.rows, matSrc.cols, matSrc.type());
		cv::Mat mat_;
		matSrc.copyTo(mat_);

		// Apply the Affine Transform just found to the src image
		cv::warpAffine(mat_, warp_dst_, warp_mat_, warp_dst_.size(), interpolation);

		assert(warp_dst.cols == warp_dst_.cols && warp_dst.rows == warp_dst_.rows);
		assert(warp_dst.step == warp_dst_.step);
		for (int y = 0; y < warp_dst.rows; y++) {
			const fbc::uchar* p = warp_dst.ptr(y);
			const uchar* p_ = warp_dst_.ptr(y);

			for (int x = 0; x < warp_dst.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_warpAffine_float()
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

	for (int interpolation = 0; interpolation < 5; interpolation++) {
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

		fbc::Mat_<float, 1> mat(matSrc.rows, matSrc.cols, matSrc.data);
		fbc::Mat_<float, 1> warp_dst;
		warp_dst.zeros(mat.rows, mat.cols);

		fbc::warpAffine(mat, warp_dst, warp_mat, interpolation);

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

		// Set the dst image the same type and size as src
		cv::Mat warp_dst_ = cv::Mat::zeros(matSrc.rows, matSrc.cols, matSrc.type());
		cv::Mat mat_;
		matSrc.copyTo(mat_);

		// Apply the Affine Transform just found to the src image
		cv::warpAffine(mat_, warp_dst_, warp_mat_, warp_dst_.size(), interpolation);

		assert(warp_dst.cols == warp_dst_.cols && warp_dst.rows == warp_dst_.rows);
		assert(warp_dst.step == warp_dst_.step);
		for (int y = 0; y < warp_dst.rows; y++) {
			const fbc::uchar* p = warp_dst.ptr(y);
			const uchar* p_ = warp_dst_.ptr(y);

			for (int x = 0; x < warp_dst.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}
