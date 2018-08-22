#include <assert.h>
#include <vector>
#include <dft.hpp>
#include <imgproc.hpp>
#include <merge.hpp>
#include <split.hpp>
#include <core/mathfuncs.hpp>
#include <opencv2/opencv.hpp>
#include "fbc_cv_funset.hpp"

// Blog: http://blog.csdn.net/fengbingchun/article/details/53000396

int test_dft_float()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/1.jpg", 1);
#endif
	if (matSrc.empty()) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);
	int width = matSrc.cols;
	int height = matSrc.rows;

	fbc::Mat_<uchar, 1> mat1(height, width, matSrc.data);
	int m = fbc::getOptimalDFTSize(mat1.rows);
	int n = fbc::getOptimalDFTSize(mat1.cols);
	fbc::Mat_<uchar, 1> padded(m, n); //expand input image to optimal size
	// on the border add zero values
	fbc::copyMakeBorder(mat1, padded, 0, m - mat1.rows, 0, n - mat1.cols, fbc::BORDER_CONSTANT, fbc::Scalar::all(0));
	fbc::Mat_<float, 1> padded1(m, n);
	padded.convertTo(padded1);
	fbc::Mat_<float, 1> tmp(m, n, fbc::Scalar::all(0));
	std::vector<fbc::Mat_<float, 1>> planes{ padded1, tmp}; // Add to the expanded another plane with zeros
	fbc::Mat_<float, 2> complexI(m, n);
	fbc::merge(planes, complexI);
	fbc::Mat_<float, 2> dft(m, n);
	fbc::dft(complexI, dft);

	// compute the magnitude and switch to logarithmic scale
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	fbc::split(dft, planes);
	fbc::Mat_<float, 1> mag;
	fbc::magnitude(planes[0], planes[1], mag);
	fbc::Mat_<float, 1> ones(mag.rows, mag.cols, fbc::Scalar::all(1.0));
	mag += ones;
	fbc::log(mag, mag);

	// crop the spectrum, if it has an odd number of rows or columns
	fbc::Mat_<float, 1> crop;
	mag.copyTo(crop, fbc::Rect(0, 0, mag.cols & -2, mag.rows & -2));

	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = crop.cols / 2;
	int cy = crop.rows / 2;

	fbc::Mat_<float, 1> q0, q1, q2, q3;
	crop.getROI(q0, fbc::Rect(0, 0, cx, cy)); // Top-Left - Create a ROI per quadrant
	crop.getROI(q1, fbc::Rect(cx, 0, cx, cy)); // Top-Right
	crop.getROI(q2, fbc::Rect(0, cy, cx, cy)); // Bottom-Left
	crop.getROI(q3, fbc::Rect(cx, cy, cx, cy)); // Bottom-Right

	fbc::Mat_<float, 1> tmp1; // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp1);
	for (int y = 0; y < q3.rows; y++) { // q3.copyTo(q0);
		float* p1 = (float*)q0.ptr(y);
		float* p2 = (float*)q3.ptr(y);
		for (int x = 0; x < q3.cols; x++) {
			p1[x] = p2[x];
		}
	}
	for (int y = 0; y < q3.rows; y++) { // tmp1.copyTo(q3);
		float* p1 = (float*)tmp1.ptr(y);
		float* p2 = (float*)q3.ptr(y);
		for (int x = 0; x < q3.cols; x++) {
			p2[x] = p1[x];
		}
	}

	q1.copyTo(tmp1); // swap quadrant (Top-Right with Bottom-Left)
	for (int y = 0; y < q2.rows; y++) { // q2.copyTo(q1);
		float* p1 = (float*)q1.ptr(y);
		float* p2 = (float*)q2.ptr(y);
		for (int x = 0; x < q2.cols; x++) {
			p1[x] = p2[x];
		}
	}
	for (int y = 0; y < q2.rows; y++) { // tmp1.copyTo(q2);
		float* p1 = (float*)q2.ptr(y);
		float* p2 = (float*)tmp1.ptr(y);
		for (int x = 0; x < q2.cols; x++) {
			p1[x] = p2[x];
		}
	}

	fbc::normalize(crop, crop, 0, 1, FBC_MINMAX); // Transform the matrix with float values into a
	// viewable image form (float between values 0 and 1).

	cv::Mat mat1_(height, width, CV_8UC1, matSrc.data);
	int m_ = cv::getOptimalDFTSize(mat1_.rows);
	int n_ = cv::getOptimalDFTSize(mat1_.cols);
	cv::Mat padded_;
	cv::copyMakeBorder(mat1_, padded_, 0, m_ - mat1_.rows, 0, n_ - mat1_.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	cv::Mat planes_[2] { cv::Mat_<float>(padded_), cv::Mat::zeros(padded_.size(), CV_32F) };
	cv::Mat complexI_;
	cv::merge(planes_, 2, complexI_); // Add to the expanded another plane with zeros
	cv::Mat dft_;
	cv::dft(complexI_, dft_);

	cv::split(dft_, planes_);
	cv::Mat mag_;
	cv::magnitude(planes_[0], planes_[1], mag_);
	cv::Mat ones_(mag_.rows, mag_.cols, CV_32FC1, cv::Scalar::all(1.0));
	mag_ += ones_;
	cv::log(mag_, mag_);

	cv::Mat crop_ = mag_(cv::Rect(0, 0, mag.cols & -2, mag.rows & -2));

	int cx_ = crop_.cols / 2;
	int cy_ = crop_.rows / 2;

	cv::Mat q0_(crop_, cv::Rect(0, 0, cx, cy));
	cv::Mat q1_(crop_, cv::Rect(cx, 0, cx, cy));
	cv::Mat q2_(crop_, cv::Rect(0, cy, cx, cy));
	cv::Mat q3_(crop_, cv::Rect(cx, cy, cx, cy));

	cv::Mat tmp1_;
	q0_.copyTo(tmp1_);
	q3_.copyTo(q0_);
	tmp1_.copyTo(q3_);

	q1_.copyTo(tmp1_);
	q2_.copyTo(q1_);
	tmp1_.copyTo(q2_);

	cv::normalize(crop_, crop_, 0, 1, CV_MINMAX);

	assert(m == m_ && n == n_ && padded.step == padded_.step);
	for (int y = 0; y < m; y++) {
		const fbc::uchar* p = padded.ptr(y);
		const uchar* p_ = padded_.ptr(y);

		for (int x = 0; x < padded.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	assert(complexI.rows == complexI_.rows && complexI.cols == complexI_.cols && complexI.channels == complexI_.channels() && complexI.step == complexI_.step);
	for (int y = 0; y < m; y++) {
		const fbc::uchar* p = complexI.ptr(y);
		const uchar* p_ = complexI_.ptr(y);

		for (int x = 0; x < complexI.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	assert(dft.rows == dft_.rows && dft.cols == dft_.cols && dft.channels == dft_.channels() && dft.step == dft_.step);
	for (int y = 0; y < m; y++) {
		const fbc::uchar* p = dft.ptr(y);
		const uchar* p_ = dft_.ptr(y);

		for (int x = 0; x < dft.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	assert(crop.rows == crop_.rows && crop.cols == crop_.cols && crop.channels == crop_.channels() && crop.step == crop_.step);
	for (int y = 0; y < crop.rows; y++) {
		const fbc::uchar* p = crop.ptr(y);
		const uchar* p_ = crop_.ptr(y);

		for (int x = 0; x < crop.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	cv::Mat dst(crop.rows, crop.cols, CV_32FC1, crop_.data);
	dst = dst * 255;
#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/dft.jpg", dst);
#else
	cv::imwrite("test_images/dft.jpg", dst);
#endif

	return 0;
}
