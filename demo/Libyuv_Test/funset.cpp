#include "funset.hpp"
#include <iostream>
#include <assert.h>
#include <cmath>
#include <libyuv.h>
#include <opencv2/opencv.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/50323273

int test_BGRAToI420()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/cat.jpg");
	if (!matSrc.data) {
		std::cout << "read src image error" << std::endl;
		return -1;
	}

	//cv::resize(matSrc, matSrc, cv::Size(500, 111));

	int width = matSrc.cols;
	int height = matSrc.rows;
	int size_frame = width * height;

	// BGRA <--> I420(YUV420P)
	cv::Mat matBGRA, matI420, matARGB;
	cv::cvtColor(matSrc, matBGRA, cv::COLOR_BGR2BGRA);
	matARGB = cv::Mat(height, width, CV_8UC4, cv::Scalar::all(0));
	libyuv::BGRAToARGB(matBGRA.data, width * 4, matARGB.data, width * 4, width, height);

	uchar* pI420 = new uchar[width * height + (width + 1) / 2 * ((height + 1) / 2) * 2];
	memset(pI420, 0, sizeof(uchar)* (width * height + (width + 1) / 2 * ((height + 1) / 2) * 2));
	uchar* dst_y = pI420;
	int dst_y_stride = width;
	uchar* dst_u = pI420 + size_frame;
	int dst_u_stride = (width + 1) / 2;
	uchar* dst_v = pI420 + size_frame + dst_u_stride * ((height + 1) / 2);
	int dst_v_stride = (width + 1) / 2;

	libyuv::BGRAToI420(matARGB.data, width * 4, dst_y, dst_y_stride, dst_u, dst_u_stride, dst_v, dst_v_stride, width, height);
	matI420 = cv::Mat(height, width, CV_8UC4, cv::Scalar::all(0));
	libyuv::I420ToBGRA(dst_y, dst_y_stride, dst_u, dst_u_stride, dst_v, dst_v_stride, matI420.data, width * 4, width, height);
	cv::Mat matBGRA_ = cv::Mat(height, width, CV_8UC4, cv::Scalar::all(0));
	libyuv::ARGBToBGRA(matI420.data, width * 4, matBGRA_.data, width * 4, width, height);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/I420_bgra.jpg", matBGRA_);
	cv::Mat matDst;
	matBGRA_.copyTo(matDst);

	int count_diff = 0;
	int max_diff = 0;
	int threshold = 20;//
	for (int i = 0; i < height; i++) {
		uchar* pSrc = matBGRA.ptr(i);
		uchar* pDst = matBGRA_.ptr(i);
		for (int j = 0, m = 0; j < width; j++, m += 4) {
			int tmp = std::max(abs(pSrc[m] - pDst[m]), abs(pSrc[m + 1] - pDst[m + 1]));
			tmp = std::max(tmp, abs(pSrc[m + 2] - pDst[m + 2]));
			if (tmp > max_diff)
				max_diff = tmp;

			if (abs(pSrc[m] - pDst[m]) > threshold ||
				abs(pSrc[m + 1] - pDst[m + 1]) > threshold ||
				abs(pSrc[m + 2] - pDst[m + 2]) > threshold) {
				count_diff++;
				//std::cout << i << "    " << j << std::endl;
			}

		}
	}

	std::cout << "convert I420 to BGRA diff max: " << max_diff << std::endl;
	if (count_diff > width + height) {//
		std::cout << "convert I420 to BGRA error." << std::endl;
		std::cout << "diff num: " << count_diff << std::endl;
	}

	delete[] pI420;

	return 0;
}

int test_BGRAToNV21()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/cat.jpg");
	if (!matSrc.data) {
		std::cout << "read src image error" << std::endl;
		return -1;
	}

	//cv::resize(matSrc, matSrc, cv::Size(500, 111));

	int width = matSrc.cols;
	int height = matSrc.rows;
	int size_frame = width * height;

	// BGRA <--> NV12
	cv::Mat matBGRA, matNV12;
	cv::cvtColor(matSrc, matBGRA, cv::COLOR_BGR2BGRA);

	uchar* pNV12 = new uchar[width * height + ((width + 1) / 2) * ((height + 1) / 2) * 2];
	memset(pNV12, 0, sizeof(uchar)* (width * height + ((width + 1) / 2) * ((height + 1) / 2) * 2));
	uchar* dst_y = pNV12;
	int dst_y_stride = width;
	uchar* dst_vu = pNV12 + size_frame;
	int dst_vu_stride = (width + 1) / 2 * 2;

	libyuv::ARGBToNV12(matBGRA.data, width * 4, dst_y, dst_y_stride, dst_vu, dst_vu_stride, width, height);
	matNV12 = cv::Mat(height, width, CV_8UC4, cv::Scalar::all(0));
	libyuv::NV12ToARGB(dst_y, dst_y_stride, dst_vu, dst_vu_stride, matNV12.data, width * 4, width, height);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/NV12_bgra.jpg", matNV12);

	int count_diff = 0;
	int max_diff = 0;
	int threshold = 20;//
	for (int i = 0; i < height; i++) {
		uchar* pSrc = matBGRA.ptr(i);
		uchar* pDst = matNV12.ptr(i);
		for (int j = 0, m = 0; j < width; j++, m += 4) {
			int tmp = std::max(abs(pSrc[m] - pDst[m]), abs(pSrc[m + 1] - pDst[m + 1]));
			tmp = std::max(tmp, abs(pSrc[m + 2] - pDst[m + 2]));
			if (tmp > max_diff)
				max_diff = tmp;

			if (abs(pSrc[m] - pDst[m]) > threshold ||
				abs(pSrc[m + 1] - pDst[m + 1]) > threshold ||
				abs(pSrc[m + 2] - pDst[m + 2]) > threshold) {
				count_diff++;
				//std::cout << i << "    " << j << std::endl;
			}
		}
	}

	std::cout << "convert NV12 to BGRA diff max: " << max_diff << std::endl;
	if (count_diff > width + height) {//
		std::cout << "convert NV12 to BGRA error." << std::endl;
		std::cout << "diff num: " << count_diff << std::endl;
	}

	delete[] pNV12;

	return 0;
}

int test_BGRAToNV12()
{
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/cat.jpg");
	if (!matSrc.data) {
		std::cout << "read src image error" << std::endl;
		return -1;
	}

	//cv::resize(matSrc, matSrc, cv::Size(500, 111));

	int width = matSrc.cols;
	int height = matSrc.rows;
	int size_frame = width * height;

	// BGRA <--> NV21
	cv::Mat matBGRA, matNV21;
	cv::cvtColor(matSrc, matBGRA, cv::COLOR_BGR2BGRA);

	uchar* pNV21 = new uchar[width * height + ((width + 1) / 2) * ((height + 1) / 2) * 2];
	memset(pNV21, 0, sizeof(uchar)* (width * height + ((width + 1) / 2) * ((height + 1) / 2) * 2));
	uchar* dst_y = pNV21;
	int dst_y_stride = width;
	uchar* dst_vu = pNV21 + size_frame;
	int dst_vu_stride = (width + 1) / 2 * 2;

	libyuv::ARGBToNV21(matBGRA.data, width * 4, dst_y, dst_y_stride, dst_vu, dst_vu_stride, width, height);
	matNV21 = cv::Mat(height, width, CV_8UC4, cv::Scalar::all(0));
	libyuv::NV21ToARGB(dst_y, dst_y_stride, dst_vu, dst_vu_stride, matNV21.data, width * 4, width, height);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/NV21_bgra.jpg", matNV21);

	int count_diff = 0;
	int max_diff = 0;
	int threshold = 20;//
	for (int i = 0; i < height; i++) {
		uchar* pSrc = matBGRA.ptr(i);
		uchar* pDst = matNV21.ptr(i);
		for (int j = 0, m = 0; j < width; j++, m += 4) {
			int tmp = std::max(abs(pSrc[m] - pDst[m]), abs(pSrc[m + 1] - pDst[m + 1]));
			tmp = std::max(tmp, abs(pSrc[m + 2] - pDst[m + 2]));
			if (tmp > max_diff)
				max_diff = tmp;

			if (abs(pSrc[m] - pDst[m]) > threshold ||
				abs(pSrc[m + 1] - pDst[m + 1]) > threshold ||
				abs(pSrc[m + 2] - pDst[m + 2]) > threshold) {
				count_diff++;
				//std::cout << i << "    " << j << std::endl;
			}
		}
	}

	std::cout << "convert NV21 to BGRA diff max: " << max_diff << std::endl;
	if (count_diff > width + height) {//
		std::cout << "convert NV21 to BGRA error." << std::endl;
		std::cout << "diff num: " << count_diff << std::endl;
	}

	delete[] pNV21;

	return 0;
}

