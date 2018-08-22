#include "fbc_cv_funset.hpp"
#include <assert.h>
#include <opencv2/opencv.hpp>
#include <warpPerspective.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/52004083

int test_getPerspectiveTransform()
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

	fbc::Point2f src_vertices[4], dst_vertices[4];
	src_vertices[0] = fbc::Point2f(0, 0);
	src_vertices[1] = fbc::Point2f(matSrc.cols - 5, 0);
	src_vertices[2] = fbc::Point2f(matSrc.cols - 10, matSrc.rows - 1);
	src_vertices[3] = fbc::Point2f(8, matSrc.rows - 13);
	dst_vertices[0] = fbc::Point2f(17, 21);
	dst_vertices[1] = fbc::Point2f(matSrc.cols - 23, 19);
	dst_vertices[2] = fbc::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
	dst_vertices[3] = fbc::Point2f(55, matSrc.rows / 5 + 33);

	fbc::Mat_<double, 1> warpMatrix(3, 3);
	fbc::getPerspectiveTransform(src_vertices, dst_vertices, warpMatrix);

	cv::Point2f src_vertices_[4], dst_vertices_[4];
	src_vertices_[0] = cv::Point2f(0, 0);
	src_vertices_[1] = cv::Point2f(matSrc.cols - 5, 0);
	src_vertices_[2] = cv::Point2f(matSrc.cols - 10, matSrc.rows - 1);
	src_vertices_[3] = cv::Point2f(8, matSrc.rows - 13);

	dst_vertices_[0] = cv::Point2f(17, 21);
	dst_vertices_[1] = cv::Point2f(matSrc.cols - 23, 19);
	dst_vertices_[2] = cv::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
	dst_vertices_[3] = cv::Point2f(55, matSrc.rows / 5 + 33);

	cv::Mat warpMatrix_ = cv::getPerspectiveTransform(src_vertices_, dst_vertices_);

	assert(warpMatrix.cols == warpMatrix_.cols && warpMatrix.rows == warpMatrix_.rows);
	assert(warpMatrix.step == warpMatrix_.step);
	for (int y = 0; y < warpMatrix.rows; y++) {
		const fbc::uchar* p = warpMatrix.ptr(y);
		const uchar* p_ = warpMatrix_.ptr(y);

		for (int x = 0; x < warpMatrix.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_warpPerspective_uchar()
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
		fbc::Point2f src_vertices[4], dst_vertices[4];
		src_vertices[0] = fbc::Point2f(0, 0);
		src_vertices[1] = fbc::Point2f(matSrc.cols - 5, 0);
		src_vertices[2] = fbc::Point2f(matSrc.cols - 10, matSrc.rows - 1);
		src_vertices[3] = fbc::Point2f(8, matSrc.rows - 13);
		dst_vertices[0] = fbc::Point2f(17, 21);
		dst_vertices[1] = fbc::Point2f(matSrc.cols - 23, 19);
		dst_vertices[2] = fbc::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
		dst_vertices[3] = fbc::Point2f(55, matSrc.rows / 5 + 33);

		fbc::Mat_<double, 1> warpMatrix(3, 3);
		fbc::getPerspectiveTransform(src_vertices, dst_vertices, warpMatrix);

		fbc::Mat_<uchar, 3> mat(matSrc.rows, matSrc.cols, matSrc.data);
		fbc::Mat_<uchar, 3> warp_dst;
		warp_dst.zeros(mat.rows, mat.cols);

		fbc::warpPerspective(mat, warp_dst, warpMatrix, interpolation);

		cv::Point2f src_vertices_[4], dst_vertices_[4];
		src_vertices_[0] = cv::Point2f(0, 0);
		src_vertices_[1] = cv::Point2f(matSrc.cols - 5, 0);
		src_vertices_[2] = cv::Point2f(matSrc.cols - 10, matSrc.rows - 1);
		src_vertices_[3] = cv::Point2f(8, matSrc.rows - 13);

		dst_vertices_[0] = cv::Point2f(17, 21);
		dst_vertices_[1] = cv::Point2f(matSrc.cols - 23, 19);
		dst_vertices_[2] = cv::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
		dst_vertices_[3] = cv::Point2f(55, matSrc.rows / 5 + 33);

		// Get the Perspective Transform
		cv::Mat warpMatrix_ = cv::getPerspectiveTransform(src_vertices_, dst_vertices_);

		// Set the dst image the same type and size as src
		cv::Mat warp_dst_ = cv::Mat::zeros(matSrc.rows, matSrc.cols, matSrc.type());
		cv::Mat mat_;
		matSrc.copyTo(mat_);

		// Apply the Affine Transform just found to the src image
		cv::warpPerspective(mat_, warp_dst_, warpMatrix_, warp_dst_.size(), interpolation);

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

int test_warpPerspective_float()
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
		fbc::Point2f src_vertices[4], dst_vertices[4];
		src_vertices[0] = fbc::Point2f(0, 0);
		src_vertices[1] = fbc::Point2f(matSrc.cols - 5, 0);
		src_vertices[2] = fbc::Point2f(matSrc.cols - 10, matSrc.rows - 1);
		src_vertices[3] = fbc::Point2f(8, matSrc.rows - 13);
		dst_vertices[0] = fbc::Point2f(17, 21);
		dst_vertices[1] = fbc::Point2f(matSrc.cols - 23, 19);
		dst_vertices[2] = fbc::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
		dst_vertices[3] = fbc::Point2f(55, matSrc.rows / 5 + 33);

		fbc::Mat_<double, 1> warpMatrix(3, 3);
		fbc::getPerspectiveTransform(src_vertices, dst_vertices, warpMatrix);

		fbc::Mat_<float, 1> mat(matSrc.rows, matSrc.cols, matSrc.data);
		fbc::Mat_<float, 1> warp_dst;
		warp_dst.zeros(mat.rows, mat.cols);

		fbc::warpPerspective(mat, warp_dst, warpMatrix, interpolation);

		cv::Point2f src_vertices_[4], dst_vertices_[4];
		src_vertices_[0] = cv::Point2f(0, 0);
		src_vertices_[1] = cv::Point2f(matSrc.cols - 5, 0);
		src_vertices_[2] = cv::Point2f(matSrc.cols - 10, matSrc.rows - 1);
		src_vertices_[3] = cv::Point2f(8, matSrc.rows - 13);

		dst_vertices_[0] = cv::Point2f(17, 21);
		dst_vertices_[1] = cv::Point2f(matSrc.cols - 23, 19);
		dst_vertices_[2] = cv::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
		dst_vertices_[3] = cv::Point2f(55, matSrc.rows / 5 + 33);

		// Get the Perspective Transform
		cv::Mat warpMatrix_ = cv::getPerspectiveTransform(src_vertices_, dst_vertices_);

		// Set the dst image the same type and size as src
		cv::Mat warp_dst_ = cv::Mat::zeros(matSrc.rows, matSrc.cols, matSrc.type());
		cv::Mat mat_;
		matSrc.copyTo(mat_);

		// Apply the Affine Transform just found to the src image
		cv::warpPerspective(mat_, warp_dst_, warpMatrix_, warp_dst_.size(), interpolation);

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
