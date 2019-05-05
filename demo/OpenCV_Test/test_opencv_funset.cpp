#include "opencv_funset.hpp"

#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include <opencv2/opencv.hpp>

int test_opencv_resize_cplusplus()
{
	// Blog: https://blog.csdn.net/fengbingchun/article/details/17335477

	cv::Mat matSrc, matDst1, matDst2;
#ifdef _MSC_VER
	matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png");
#else
	matSrc = cv::imread("test_images/lena.png");
#endif
	if (matSrc.empty()) {
		fprintf(stderr, "read image fail\n");
		return -1;
	}
	fprintf(stdout, "image size: width: %d, height: %d\n", matSrc.cols, matSrc.rows);
	matDst1 = cv::Mat(cv::Size(800, 1000), matSrc.type(), cv::Scalar::all(0));
	matDst2 = cv::Mat(matDst1.size(), matSrc.type(), cv::Scalar::all(0));

	const double scale_x = (double)matSrc.cols / matDst1.cols;
	const double scale_y = (double)matSrc.rows / matDst1.rows;

{ // nearest
	fprintf(stdout, "==== start nearest ====\n");
	for (int i = 0; i < matDst1.cols; ++i) {
		int sx = cvFloor(i * scale_x);
		sx = std::min(sx, matSrc.cols - 1);
		for (int j = 0; j < matDst1.rows; ++j) {
			int sy = cvFloor(j * scale_y);
			sy = std::min(sy, matSrc.rows - 1);
			matDst1.at<cv::Vec3b>(j, i) = matSrc.at<cv::Vec3b>(sy, sx);
		}
	}
	fprintf(stdout, "==== end nearest ====\n");

#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/nearest_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 0);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/nearest_2.jpg", matDst2);
#else
	cv::imwrite("test_images/nearest_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 0);
	cv::imwrite("test_images/nearest_2.jpg", matDst2);
#endif
}

{ // linear
	fprintf(stdout, "==== start linear ====\n");
	uchar* dataDst = matDst1.data;
	int stepDst = matDst1.step;
	const uchar* dataSrc = matSrc.data;
	int stepSrc = matSrc.step;
	int iWidthSrc = matSrc.cols;
	int iHiehgtSrc = matSrc.rows;

	for (int j = 0; j < matDst1.rows; ++j) {
		float fy = (float)((j + 0.5) * scale_y - 0.5);
		int sy = cvFloor(fy);
		fy -= sy;
		sy = std::min(sy, iHiehgtSrc - 2);
		sy = std::max(0, sy);

		short cbufy[2];
		cbufy[0] = cv::saturate_cast<short>((1.f - fy) * 2048);
		cbufy[1] = 2048 - cbufy[0];

		for (int i = 0; i < matDst1.cols; ++i) {
			float fx = (float)((i + 0.5) * scale_x - 0.5);
			int sx = cvFloor(fx);
			fx -= sx;

			if (sx < 0) {
				fx = 0, sx = 0;
			}
			if (sx >= iWidthSrc - 1) {
				fx = 0, sx = iWidthSrc - 2;
			}

			short cbufx[2];
			cbufx[0] = cv::saturate_cast<short>((1.f - fx) * 2048);
			cbufx[1] = 2048 - cbufx[0];

			for (int k = 0; k < matSrc.channels(); ++k) {
				*(dataDst + j*stepDst + 3 * i + k) = (*(dataSrc + sy*stepSrc + 3 * sx + k) * cbufx[0] * cbufy[0] +
					*(dataSrc + (sy + 1)*stepSrc + 3 * sx + k) * cbufx[0] * cbufy[1] +
					*(dataSrc + sy*stepSrc + 3 * (sx + 1) + k) * cbufx[1] * cbufy[0] +
					*(dataSrc + (sy + 1)*stepSrc + 3 * (sx + 1) + k) * cbufx[1] * cbufy[1]) >> 22;
			}
		}
	}
	fprintf(stdout, "==== end linear ====\n");

#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/linear_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 1);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/linear_2.jpg", matDst2);
#else
	cv::imwrite("test_images/linear_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 1);
	cv::imwrite("test_images/linear_2.jpg", matDst2);
#endif
}

{ // cubic
	fprintf(stdout, "==== start cublic ====\n");
	int iscale_x = cv::saturate_cast<int>(scale_x);
	int iscale_y = cv::saturate_cast<int>(scale_y);

	for (int j = 0; j < matDst1.rows; ++j) {
		float fy = (float)((j + 0.5) * scale_y - 0.5);
		int sy = cvFloor(fy);
		fy -= sy;
		sy = std::min(sy, matSrc.rows - 3);
		sy = std::max(1, sy);

		const float A = -0.75f;

		float coeffsY[4];
		coeffsY[0] = ((A*(fy + 1) - 5 * A)*(fy + 1) + 8 * A)*(fy + 1) - 4 * A;
		coeffsY[1] = ((A + 2)*fy - (A + 3))*fy*fy + 1;
		coeffsY[2] = ((A + 2)*(1 - fy) - (A + 3))*(1 - fy)*(1 - fy) + 1;
		coeffsY[3] = 1.f - coeffsY[0] - coeffsY[1] - coeffsY[2];

		short cbufY[4];
		cbufY[0] = cv::saturate_cast<short>(coeffsY[0] * 2048);
		cbufY[1] = cv::saturate_cast<short>(coeffsY[1] * 2048);
		cbufY[2] = cv::saturate_cast<short>(coeffsY[2] * 2048);
		cbufY[3] = cv::saturate_cast<short>(coeffsY[3] * 2048);

		for (int i = 0; i < matDst1.cols; ++i) {
			float fx = (float)((i + 0.5) * scale_x - 0.5);
			int sx = cvFloor(fx);
			fx -= sx;

			if (sx < 1) {
				fx = 0, sx = 1;
			}
			if (sx >= matSrc.cols - 3) {
				fx = 0, sx = matSrc.cols - 3;
			}

			float coeffsX[4];
			coeffsX[0] = ((A*(fx + 1) - 5 * A)*(fx + 1) + 8 * A)*(fx + 1) - 4 * A;
			coeffsX[1] = ((A + 2)*fx - (A + 3))*fx*fx + 1;
			coeffsX[2] = ((A + 2)*(1 - fx) - (A + 3))*(1 - fx)*(1 - fx) + 1;
			coeffsX[3] = 1.f - coeffsX[0] - coeffsX[1] - coeffsX[2];

			short cbufX[4];
			cbufX[0] = cv::saturate_cast<short>(coeffsX[0] * 2048);
			cbufX[1] = cv::saturate_cast<short>(coeffsX[1] * 2048);
			cbufX[2] = cv::saturate_cast<short>(coeffsX[2] * 2048);
			cbufX[3] = cv::saturate_cast<short>(coeffsX[3] * 2048);

			for (int k = 0; k < matSrc.channels(); ++k) {
				matDst1.at<cv::Vec3b>(j, i)[k] = abs((matSrc.at<cv::Vec3b>(sy - 1, sx - 1)[k] * cbufX[0] * cbufY[0] + matSrc.at<cv::Vec3b>(sy, sx - 1)[k] * cbufX[0] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy + 1, sx - 1)[k] * cbufX[0] * cbufY[2] + matSrc.at<cv::Vec3b>(sy + 2, sx - 1)[k] * cbufX[0] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy - 1, sx)[k] * cbufX[1] * cbufY[0] + matSrc.at<cv::Vec3b>(sy, sx)[k] * cbufX[1] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy + 1, sx)[k] * cbufX[1] * cbufY[2] + matSrc.at<cv::Vec3b>(sy + 2, sx)[k] * cbufX[1] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy - 1, sx + 1)[k] * cbufX[2] * cbufY[0] + matSrc.at<cv::Vec3b>(sy, sx + 1)[k] * cbufX[2] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 1)[k] * cbufX[2] * cbufY[2] + matSrc.at<cv::Vec3b>(sy + 2, sx + 1)[k] * cbufX[2] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy - 1, sx + 2)[k] * cbufX[3] * cbufY[0] + matSrc.at<cv::Vec3b>(sy, sx + 2)[k] * cbufX[3] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 2)[k] * cbufX[3] * cbufY[2] + matSrc.at<cv::Vec3b>(sy + 2, sx + 2)[k] * cbufX[3] * cbufY[3]) >> 22);
			}
		}
	}
	fprintf(stdout, "==== end cublic ====\n");

#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/cubic_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 2);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/cubic_2.jpg", matDst2);
#else
	cv::imwrite("test_images/cubic_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 2);
	cv::imwrite("test_images/cubic_2.jpg", matDst2);
#endif
}

{ // Lanczos
	fprintf(stdout, "==== start Lanczos ====\n");
	int iscale_x = cv::saturate_cast<int>(scale_x);
	int iscale_y = cv::saturate_cast<int>(scale_y);

	for (int j = 0; j < matDst1.rows; ++j) {
		float fy = (float)((j + 0.5) * scale_y - 0.5);
		int sy = cvFloor(fy);
		fy -= sy;
		sy = std::min(sy, matSrc.rows - 5);
		sy = std::max(3, sy);

		const double s45 = 0.70710678118654752440084436210485;
		const double cs[][2] = { { 1, 0 }, { -s45, -s45 }, { 0, 1 }, { s45, -s45 }, { -1, 0 }, { s45, s45 }, { 0, -1 }, { -s45, s45 } };
		float coeffsY[8];

		if (fy < FLT_EPSILON) {
			for (int t = 0; t < 8; t++)
				coeffsY[t] = 0;
			coeffsY[3] = 1;
		}
		else {
			float sum = 0;
			double y0 = -(fy + 3) * CV_PI * 0.25, s0 = sin(y0), c0 = cos(y0);

			for (int t = 0; t < 8; ++t) {
				double dy = -(fy + 3 - t) * CV_PI * 0.25;
				coeffsY[t] = (float)((cs[t][0] * s0 + cs[t][1] * c0) / (dy * dy));
				sum += coeffsY[t];
			}

			sum = 1.f / sum;
			for (int t = 0; t < 8; ++t)
				coeffsY[t] *= sum;
		}

		short cbufY[8];
		cbufY[0] = cv::saturate_cast<short>(coeffsY[0] * 2048);
		cbufY[1] = cv::saturate_cast<short>(coeffsY[1] * 2048);
		cbufY[2] = cv::saturate_cast<short>(coeffsY[2] * 2048);
		cbufY[3] = cv::saturate_cast<short>(coeffsY[3] * 2048);
		cbufY[4] = cv::saturate_cast<short>(coeffsY[4] * 2048);
		cbufY[5] = cv::saturate_cast<short>(coeffsY[5] * 2048);
		cbufY[6] = cv::saturate_cast<short>(coeffsY[6] * 2048);
		cbufY[7] = cv::saturate_cast<short>(coeffsY[7] * 2048);

		for (int i = 0; i < matDst1.cols; ++i) {
			float fx = (float)((i + 0.5) * scale_x - 0.5);
			int sx = cvFloor(fx);
			fx -= sx;

			if (sx < 3) {
				fx = 0, sx = 3;
			}
			if (sx >= matSrc.cols - 5) {
				fx = 0, sx = matSrc.cols - 5;
			}

			float coeffsX[8];

			if (fx < FLT_EPSILON) {
				for (int t = 0; t < 8; t++)
					coeffsX[t] = 0;
				coeffsX[3] = 1;
			} else {
				float sum = 0;
				double x0 = -(fx + 3) * CV_PI * 0.25, s0 = sin(x0), c0 = cos(x0);

				for (int t = 0; t < 8; ++t) {
					double dx = -(fx + 3 - t) * CV_PI * 0.25;
					coeffsX[t] = (float)((cs[t][0] * s0 + cs[t][1] * c0) / (dx * dx));
					sum += coeffsX[t];
				}

				sum = 1.f / sum;
				for (int t = 0; t < 8; ++t)
					coeffsX[t] *= sum;
			}

			short cbufX[8];
			cbufX[0] = cv::saturate_cast<short>(coeffsX[0] * 2048);
			cbufX[1] = cv::saturate_cast<short>(coeffsX[1] * 2048);
			cbufX[2] = cv::saturate_cast<short>(coeffsX[2] * 2048);
			cbufX[3] = cv::saturate_cast<short>(coeffsX[3] * 2048);
			cbufX[4] = cv::saturate_cast<short>(coeffsX[4] * 2048);
			cbufX[5] = cv::saturate_cast<short>(coeffsX[5] * 2048);
			cbufX[6] = cv::saturate_cast<short>(coeffsX[6] * 2048);
			cbufX[7] = cv::saturate_cast<short>(coeffsX[7] * 2048);

			for (int k = 0; k < matSrc.channels(); ++k) {
				matDst1.at<cv::Vec3b>(j, i)[k] = abs((matSrc.at<cv::Vec3b>(sy - 3, sx - 3)[k] * cbufX[0] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx - 3)[k] * cbufX[0] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx - 3)[k] * cbufX[0] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx - 3)[k] * cbufX[0] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx - 3)[k] * cbufX[0] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx - 3)[k] * cbufX[0] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx - 3)[k] * cbufX[0] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx - 3)[k] * cbufX[0] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx - 2)[k] * cbufX[1] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx - 2)[k] * cbufX[1] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx - 2)[k] * cbufX[1] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx - 2)[k] * cbufX[1] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx - 2)[k] * cbufX[1] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx - 2)[k] * cbufX[1] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx - 2)[k] * cbufX[1] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx - 2)[k] * cbufX[1] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx - 1)[k] * cbufX[2] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx - 1)[k] * cbufX[2] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx - 1)[k] * cbufX[2] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx - 1)[k] * cbufX[2] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx - 1)[k] * cbufX[2] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx - 1)[k] * cbufX[2] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx - 1)[k] * cbufX[2] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx - 1)[k] * cbufX[2] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx)[k] * cbufX[3] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx)[k] * cbufX[3] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx)[k] * cbufX[3] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx)[k] * cbufX[3] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx)[k] * cbufX[3] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx)[k] * cbufX[3] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx)[k] * cbufX[3] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx)[k] * cbufX[3] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx + 1)[k] * cbufX[4] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx + 1)[k] * cbufX[4] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx + 1)[k] * cbufX[4] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx + 1)[k] * cbufX[4] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 1)[k] * cbufX[4] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx + 1)[k] * cbufX[4] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx + 1)[k] * cbufX[4] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx + 1)[k] * cbufX[4] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx + 2)[k] * cbufX[5] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx + 2)[k] * cbufX[5] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx + 2)[k] * cbufX[5] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx + 2)[k] * cbufX[5] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 2)[k] * cbufX[5] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx + 2)[k] * cbufX[5] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx + 2)[k] * cbufX[5] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx + 2)[k] * cbufX[5] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx + 3)[k] * cbufX[6] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx + 3)[k] * cbufX[6] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx + 3)[k] * cbufX[6] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx + 3)[k] * cbufX[6] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 3)[k] * cbufX[6] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx + 3)[k] * cbufX[6] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx + 3)[k] * cbufX[6] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx + 3)[k] * cbufX[6] * cbufY[7] +

					matSrc.at<cv::Vec3b>(sy - 3, sx + 4)[k] * cbufX[7] * cbufY[0] + matSrc.at<cv::Vec3b>(sy - 2, sx + 4)[k] * cbufX[7] * cbufY[1] +
					matSrc.at<cv::Vec3b>(sy - 1, sx + 4)[k] * cbufX[7] * cbufY[2] + matSrc.at<cv::Vec3b>(sy, sx + 4)[k] * cbufX[7] * cbufY[3] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 4)[k] * cbufX[7] * cbufY[4] + matSrc.at<cv::Vec3b>(sy + 2, sx + 4)[k] * cbufX[7] * cbufY[5] +
					matSrc.at<cv::Vec3b>(sy + 3, sx + 4)[k] * cbufX[7] * cbufY[6] + matSrc.at<cv::Vec3b>(sy + 4, sx + 4)[k] * cbufX[7] * cbufY[7]) >> 22);// 4194304
			}
		}
	}
	fprintf(stdout, "==== end Lanczos ====\n");

#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/Lanczos_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 4);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/Lanczos_2.jpg", matDst2);
#else
	cv::imwrite("test_images/Lanczos_1.jpg", matDst1);
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 4);
	cv::imwrite("test_images/Lanczos_2.jpg", matDst2);
#endif
}

{ // area
#ifdef _MSC_VER
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 3);
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/area_2.jpg", matDst2);
#else
	cv::resize(matSrc, matDst2, matDst1.size(), 0, 0, 3);
	cv::imwrite("test_images/area_2.jpg", matDst2);
#endif

	fprintf(stdout, "==== start area ====\n");
	double inv_scale_x = 1. / scale_x;
	double inv_scale_y = 1. / scale_y;
	int iscale_x = cv::saturate_cast<int>(scale_x);
	int iscale_y = cv::saturate_cast<int>(scale_y);
	bool is_area_fast = std::abs(scale_x - iscale_x) < DBL_EPSILON && std::abs(scale_y - iscale_y) < DBL_EPSILON;

	if (scale_x >= 1 && scale_y >= 1)  { // zoom out
		if (is_area_fast)  { // integer multiples
			for (int j = 0; j < matDst1.rows; ++j) {
				int sy = std::min(cvFloor(j * scale_y), matSrc.rows - 1);

				for (int i = 0; i < matDst1.cols; ++i) {
					int sx = std::min(cvFloor(i * scale_x), matSrc.cols -1);

					matDst1.at<cv::Vec3b>(j, i) = matSrc.at<cv::Vec3b>(sy, sx);
				}
			}
#ifdef _MSC_VER
			cv::imwrite("E:/GitCode/OpenCV_Test/test_images/area_1.jpg", matDst1);
#else
			cv::imwrite("test_images/area_1.jpg", matDst1);
#endif
			return 0;
		}

		for (int j = 0; j < matDst1.rows; ++j) {
			double fsy1 = j * scale_y;
			double fsy2 = fsy1 + scale_y;
			double cellHeight = cv::min(scale_y, matSrc.rows - fsy1);

			int sy1 = cvCeil(fsy1), sy2 = cvFloor(fsy2);

			sy2 = std::min(sy2, matSrc.rows - 2);
			sy1 = std::min(sy1, sy2);

			float cbufy[2];
			cbufy[0] = (float)((sy1 - fsy1) / cellHeight);
			cbufy[1] = (float)(std::min(std::min(fsy2 - sy2, 1.), cellHeight) / cellHeight);

			for (int i = 0; i < matDst1.cols; ++i) {
				double fsx1 = i * scale_x;
				double fsx2 = fsx1 + scale_x;
				double cellWidth = std::min(scale_x, matSrc.cols - fsx1);

				int sx1 = cvCeil(fsx1), sx2 = cvFloor(fsx2);

				sx2 = std::min(sx2, matSrc.cols - 2);
				sx1 = std::min(sx1, sx2);

				float cbufx[2];
				cbufx[0] = (float)((sx1 - fsx1) / cellWidth);
				cbufx[1] = (float)(std::min(std::min(fsx2 - sx2, 1.), cellWidth) / cellWidth);

				for (int k = 0; k < matSrc.channels(); ++k) {
					matDst1.at<cv::Vec3b>(j, i)[k] = (uchar)(matSrc.at<cv::Vec3b>(sy1, sx1)[k] * cbufx[0] * cbufy[0] +
						matSrc.at<cv::Vec3b>(sy1 + 1, sx1)[k] * cbufx[0] * cbufy[1] +
						matSrc.at<cv::Vec3b>(sy1, sx1 + 1)[k] * cbufx[1] * cbufy[0] +
						matSrc.at<cv::Vec3b>(sy1 + 1, sx1 + 1)[k] * cbufx[1] * cbufy[1]);
				}
			}
		}
#ifdef _MSC_VER
		cv::imwrite("E:/GitCode/OpenCV_Test/test_images/area_1.jpg", matDst1);
#else
		cv::imwrite("test_images/area_1.jpg", matDst1);
#endif

		return 0;
	}

	//zoom in,it is emulated using some variant of bilinear interpolation
	for (int j = 0; j < matDst1.rows; ++j) {
		int  sy = cvFloor(j * scale_y);
		float fy = (float)((j + 1) - (sy + 1) * inv_scale_y);
		fy = fy <= 0 ? 0.f : fy - cvFloor(fy);
		sy = std::min(sy, matSrc.rows - 2);

		short cbufy[2];
		cbufy[0] = cv::saturate_cast<short>((1.f - fy) * 2048);
		cbufy[1] = 2048 - cbufy[0];

		for (int i = 0; i < matDst1.cols; ++i) {
			int sx = cvFloor(i * scale_x);
			float fx = (float)((i + 1) - (sx + 1) * inv_scale_x);
			fx = fx < 0 ? 0.f : fx - cvFloor(fx);

			if (sx < 0) {
				fx = 0, sx = 0;
			}

			if (sx >= matSrc.cols - 1) {
				fx = 0, sx = matSrc.cols - 2;
			}

			short cbufx[2];
			cbufx[0] = cv::saturate_cast<short>((1.f - fx) * 2048);
			cbufx[1] = 2048 - cbufx[0];

			for (int k = 0; k < matSrc.channels(); ++k) {
				matDst1.at<cv::Vec3b>(j, i)[k] = (matSrc.at<cv::Vec3b>(sy, sx)[k] * cbufx[0] * cbufy[0] +
					matSrc.at<cv::Vec3b>(sy + 1, sx)[k] * cbufx[0] * cbufy[1] +
					matSrc.at<cv::Vec3b>(sy, sx + 1)[k] * cbufx[1] * cbufy[0] +
					matSrc.at<cv::Vec3b>(sy + 1, sx + 1)[k] * cbufx[1] * cbufy[1]) >> 22;
			}
		}
	}
	fprintf(stdout, "==== end area ====\n");

#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/area_1.jpg", matDst1);
#else
	cv::imwrite("test_images/area_1.jpg", matDst1);
#endif
}

	return 0;
}

int test_opencv_kmeans()
{
	// reference: https://docs.opencv.org/3.1.0/de/d63/kmeans_8cpp-example.html
	const int MAX_CLUSTERS = 5;
	cv::Scalar colorTab[] = {cv::Scalar(0, 0, 255), cv::Scalar(0, 255, 0), cv::Scalar(255, 100, 100), cv::Scalar(255, 0, 255), cv::Scalar(0, 255, 255)};
	cv::Mat img(500, 500, CV_8UC3);
	cv::RNG rng(12345);

	for (;;) {
		int k, clusterCount = rng.uniform(2, MAX_CLUSTERS + 1);
		int i, sampleCount = rng.uniform(1, 1001);
		cv::Mat points(sampleCount, 1, CV_32FC2), labels;
		clusterCount = MIN(clusterCount, sampleCount);
		cv::Mat centers;
		/* generate random sample from multigaussian distribution */
		for (k = 0; k < clusterCount; k++) {
			cv::Point center;
			center.x = rng.uniform(0, img.cols);
			center.y = rng.uniform(0, img.rows);
			cv::Mat pointChunk = points.rowRange(k*sampleCount / clusterCount,
				k == clusterCount - 1 ? sampleCount : (k + 1)*sampleCount / clusterCount);
			rng.fill(pointChunk, cv::RNG::NORMAL, cv::Scalar(center.x, center.y), cv::Scalar(img.cols*0.05, img.rows*0.05));
		}
		cv::randShuffle(points, 1, &rng);
		cv::kmeans(points, clusterCount, labels,
			cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, centers);
		img = cv::Scalar::all(0);
		for (i = 0; i < sampleCount; i++) {
			int clusterIdx = labels.at<int>(i);
			cv::Point ipt = points.at<cv::Point2f>(i);
			cv::circle(img, ipt, 2, colorTab[clusterIdx], cv::FILLED, cv::LINE_AA);
		}
		cv::imshow("clusters", img);
		char key = (char)cv::waitKey();
		if (key == 27 || key == 'q' || key == 'Q') // 'ESC'
			break;
	}

	return 0;
}

int test_opencv_Laplacian()
{
	// reference: https://docs.opencv.org/3.1.0/d5/db5/tutorial_laplace_operator.html
#ifdef _MSC_VER
	cv::Mat src = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 0);
#else	
	cv::Mat src = cv::imread("test_images/lena.png", 0);
#endif
	if (!src.data || src.channels() != 1) {
		fprintf(stderr, "read image fail\n");
		return -1;
	}
	cv::resize(src, src, cv::Size(100, 100));

	cv::Mat dst;
	cv::Laplacian(src, dst, src.depth(), 1);
#ifdef _MSC_VER
	cv::imwrite("E:/GitCode/OpenCV_Test/test_images/laplacian_lena.png", dst);
#else
	cv::imwrite("test_images/laplacian_lena.png", dst);
#endif
	return 0;
}

namespace {

void drawAxis(cv::Mat& img, cv::Point p, cv::Point q, cv::Scalar colour, const float scale = 0.2)
{
	double angle = std::atan2((double)p.y - q.y, (double)p.x - q.x); // angle in radians
	double hypotenuse = std::sqrt((double)(p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));

	//    double degrees = angle * 180 / CV_PI; // convert radians to degrees (0-180 range)
	//    cout << "Degrees: " << abs(degrees - 180) << endl; // angle in 0-360 degrees range
	// Here we lengthen the arrow by a factor of scale
	q.x = (int)(p.x - scale * hypotenuse * std::cos(angle));
	q.y = (int)(p.y - scale * hypotenuse * std::sin(angle));
	cv::line(img, p, q, colour, 1, CV_AA);
	// create the arrow hooks
	p.x = (int)(q.x + 9 * std::cos(angle + CV_PI / 4));
	p.y = (int)(q.y + 9 * std::sin(angle + CV_PI / 4));
	cv::line(img, p, q, colour, 1, CV_AA);
	p.x = (int)(q.x + 9 * std::cos(angle - CV_PI / 4));
	p.y = (int)(q.y + 9 * std::sin(angle - CV_PI / 4));
	cv::line(img, p, q, colour, 1, CV_AA);
}

double getOrientation(const std::vector<cv::Point> &pts, cv::Mat &img)
{
	//Construct a buffer used by the pca analysis
	int sz = static_cast<int>(pts.size());
	cv::Mat data_pts = cv::Mat(sz, 2, CV_64FC1);
	for (int i = 0; i < data_pts.rows; ++i) {
		data_pts.at<double>(i, 0) = pts[i].x;
		data_pts.at<double>(i, 1) = pts[i].y;
	}

	//Perform PCA analysis
	cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
	//Store the center of the object
	cv::Point cntr = cv::Point(static_cast<int>(pca_analysis.mean.at<double>(0, 0)), static_cast<int>(pca_analysis.mean.at<double>(0, 1)));
	//Store the eigenvalues and eigenvectors
	std::vector<cv::Point2d> eigen_vecs(2);
	std::vector<double> eigen_val(2);

	double* eigenvalues = (double*)pca_analysis.eigenvalues.data;
	for (int i = 0; i < 2; ++i) {
		eigen_vecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0), pca_analysis.eigenvectors.at<double>(i, 1));
		//eigen_val[i] = pca_analysis.eigenvalues.at<double>(0, i);
		eigen_val[i] = eigenvalues[i];
	}

	// Draw the principal components
	cv::circle(img, cntr, 3, cv::Scalar(255, 0, 255), 2);
	cv::Point p1 = cntr + 0.02 * cv::Point(static_cast<int>(eigen_vecs[0].x * eigen_val[0]), static_cast<int>(eigen_vecs[0].y * eigen_val[0]));
	cv::Point p2 = cntr - 0.02 * cv::Point(static_cast<int>(eigen_vecs[1].x * eigen_val[1]), static_cast<int>(eigen_vecs[1].y * eigen_val[1]));
	drawAxis(img, cntr, p1, cv::Scalar(0, 255, 0), 1);
	drawAxis(img, cntr, p2, cv::Scalar(255, 255, 0), 5);
	double angle = std::atan2(eigen_vecs[0].y, eigen_vecs[0].x); // orientation in radians
	return angle;
}

} // namespace

int test_opencv_PCA()
{
	// reference: https://docs.opencv.org/3.1.0/d1/dee/tutorial_introduction_to_pca.html
	// Load image
#ifdef _MSC_VER
	cv::Mat src = cv::imread("E:/GitCode/OpenCV_Test/test_images/pac_test1.jpg");
#else	
	cv::Mat src = cv::imread("test_images/pca_test1.jpg");
#endif
	// Check if image is loaded successfully
	if (!src.data || src.empty()) {
		std::cout << "Problem loading image!!!" << std::endl;
		return -1;
	}
	cv::imshow("src", src);
	// Convert image to grayscale
	cv::Mat gray;
	cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	// Convert image to binary
	cv::Mat bw;
	cv::threshold(gray, bw, 50, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	// Find all the contours in the thresholded image
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (size_t i = 0; i < contours.size(); ++i) {
		// Calculate the area of each contour
		double area = cv::contourArea(contours[i]);
		// Ignore contours that are too small or too large
		if (area < 1e2 || 1e5 < area) continue;
		// Draw each contour only for visualisation purposes
		cv::drawContours(src, contours, static_cast<int>(i), cv::Scalar(0, 0, 255), 2, 8, hierarchy, 0);
		// Find the orientation of each shape
		getOrientation(contours[i], src);
	}

	cv::imshow("output", src);
	cv::waitKey(0);

	return 0;
}

int test_opencv_calcCovarMatrix()
{
	std::vector<std::vector<float>> vec{ { 1.2f, 2.5f, 5.6f, -2.5f },
					{ -3.6f, 9.2f, 0.5f, 7.2f },
					{ 4.3f, 1.3f, 9.4f, -3.4f } };
	const int rows{ 3 }, cols{ 4 };

	cv::Mat mat(rows, cols, CV_32FC1);
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			mat.at<float>(y, x) = vec.at(y).at(x);
		}
	}
	std::cout << mat << std::endl;

	cv::Mat covar, mean;
	cv::calcCovarMatrix(mat, covar, mean, CV_COVAR_NORMAL | CV_COVAR_ROWS /*| CV_COVAR_SCALE*/, CV_32FC1);
	std::cout << "covariance matrix:" << std::endl << covar << std::endl;
	std::cout << "mean values: " << std::endl << mean << std::endl;

	return 0;
}

int test_opencv_meanStdDev()
{
	std::vector<std::vector<float>> vec{ { 1.2f, 2.5f, 5.6f, -2.5f },
					{ -3.6f, 9.2f, 0.5f, 7.2f },
					{ 4.3f, 1.3f, 9.4f, -3.4f } };
	const int rows{ 3 }, cols{ 4 };

	cv::Mat mat(rows, cols, CV_32FC1);
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			mat.at<float>(y, x) = vec.at(y).at(x);
		}
	}

	cv::Scalar mean, stddev;
	cv::meanStdDev(mat, mean, stddev);

	return 0;
}

int test_opencv_trace()
{
	std::vector<std::vector<float>> vec{ { 1.2f, 2.5f, 5.6f, -2.5f },
					{ -3.6f, 9.2f, 0.5f, 7.2f },
					{ 4.3f, 1.3f, 9.4f, -3.4f } };
	const int rows{ 3 }, cols{ 4 };

	cv::Mat mat(rows, cols, CV_32FC1);
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			mat.at<float>(y, x) = vec.at(y).at(x);
		}
	}

	cv::Scalar scalar = cv::trace(mat);

	return 0;
}

int test_opencv_pseudoinverse()
{
	std::vector<std::vector<float>> vec{ { 0.68f, 0.597f },
					{ -0.211f, 0.823f },
					{ 0.566f, -0.605f } };
	const int rows{ 3 }, cols{ 2 };

	cv::Mat mat(rows, cols, CV_32FC1);
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			mat.at<float>(y, x) = vec.at(y).at(x);
		}
	}

	cv::Mat pinv;
	cv::invert(mat, pinv, cv::DECOMP_SVD);

	return 0;
}

int test_opencv_SVD()
{
	//std::vector<std::vector<float>> vec{ { 1.2f, 2.5f, 5.6f, -2.5f },
	//				{ -3.6f, 9.2f, 0.5f, 7.2f },
	//				{ 4.3f, 1.3f, 9.4f, -3.4f },
	//				{ 6.4f, 0.1f, -3.7f, 0.9f } };
	//const int rows{ 4 }, cols{ 4 };

	//std::vector<std::vector<float>> vec{ { 1.2f, 2.5f, 5.6f, -2.5f },
	//				{ -3.6f, 9.2f, 0.5f, 7.2f },
	//				{ 4.3f, 1.3f, 9.4f, -3.4f } };
	//const int rows{ 3 }, cols{ 4 };

	std::vector<std::vector<float>> vec{ { 0.68f, 0.597f },
					{ -0.211f, 0.823f },
					{ 0.566f, -0.605f } };
	const int rows{ 3 }, cols{ 2 };

	cv::Mat mat(rows, cols, CV_32FC1);
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			mat.at<float>(y, x) = vec.at(y).at(x);
		}
	}

	cv::Mat w, u, vt;
	cv::SVD::compute(mat, w, u, vt, 4);

	return 0;
}

int test_opencv_eigen()
{
	std::vector<float> vec{1.23f, 2.12f, -4.2f,
			       2.12f, -5.6f, 1.79f,
			       -4.2f, 1.79f, -7.3f };
	const int N{ 3 };
	cv::Mat mat(N, N, CV_32FC1, vec.data());

	cv::Mat eigen_values, eigen_vectors;
	bool ret = cv::eigen(mat, eigen_values, eigen_vectors);
	if (!ret) {
		fprintf(stderr, "fail to run cv::eigen\n");
		return -1;
	}

	return 0;
}

int test_opencv_norm()
{
	std::vector<int> norm_types{ 1, 2, 4 }; // 正无穷、L1、L2
	std::vector<std::string> str {"Inf", "L1", "L2"};
	// 1. vector:
	std::vector<float> vec1{ -2, 3, 1 };
	cv::Mat mat1(1, vec1.size(), CV_32FC1, vec1.data());

	for (int i = 0; i < norm_types.size(); ++i) {
		double value = cv::norm(mat1, norm_types[i]);
		fprintf(stderr, "vector: %s: %f\n", str[i].c_str(), value);
	}

	// 2. matrix:
	std::vector<float> vec2{ -3, 2, 0, 5, 6, 2, 7, 4, 8 };
	cv::Mat mat2((int)(sqrt(vec2.size())), (int)(sqrt(vec2.size())), CV_32FC1, vec2.data());

	for (int i = 0; i < norm_types.size(); ++i) {
		double value = cv::norm(mat2, norm_types[i]);
		fprintf(stderr, "matrix: %s: %f\n", str[i].c_str(), value);
	}

	return 0;
}

int test_opencv_inverse()
{
	std::vector<float> vec{ 5, -2, 2, 7, 1, 0, 0, 3, -3, 1, 5, 0, 3, -1, -9, 4 };
	const int N{ 4 };
	if (vec.size() != (int)pow(N, 2)) {
		fprintf(stderr, "vec must be N^2\n");
		return -1;
	}

	cv::Mat mat(N, N, CV_32FC1, vec.data());
	cv::Mat inverse = mat.inv();

	return 0;
}

int test_opencv_determinant()
{
	const int size{ 16 };
	std::vector<float> vec;
	vec.resize(size);
	float tmp{ 1.f }, factor{ 3.f };

	for (auto& value : vec) {
		value = factor * tmp;
		factor += 5.f;
	}

	int length = std::sqrt(size);
	cv::Mat mat(length, length, CV_32FC1, vec.data());

	double det = cv::determinant(mat);
	fprintf(stderr, "matrix's determinant: %f\n", det);

	return 0;
}

int test_read_write_video()
{
	// reference: http://docs.opencv.org/trunk/dd/d9e/classcv_1_1VideoWriter.html
	if (1) { // read image and write video
#ifdef _MSC_VER
		cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg");
#else	
		cv::Mat mat = cv::imread("test_images/1.jpg");
#endif
		if (mat.empty()) {
			fprintf(stderr, "read image fail\n");
			return -1;
		}

		int width{ 640 }, height{ 480 };
		int codec = CV_FOURCC('M', 'J', 'P', 'G');
		double fps = 25.0;
		bool isColor = (mat.type() == CV_8UC3);
		cv::VideoWriter write_video;
#ifdef _MSC_VER
		write_video.open("E:/GitCode/OpenCV_Test/test_images/video_1.avi", codec, fps, cv::Size(width, height), isColor);
#else
		write_video.open("test_images/video_1.avi", codec, fps, cv::Size(width, height), isColor);
#endif
		if (!write_video.isOpened()) {
			fprintf(stderr, "open video file fail\n");
			return -1;
		}

		int count{ 0 };
		cv::Mat tmp;
		while (mat.data) {
			cv::resize(mat, tmp, cv::Size(width, height));
			write_video.write(tmp);

			if (++count > 50)
				break;
		}
	}

	if (1) { // read video and write video
#ifdef _MSC_VER
		cv::VideoCapture read_video("E:/GitCode/OpenCV_Test/test_images/video_1.avi");
#else	
		cv::VideoCapture read_video("test_images/video_1.avi");
#endif
		if (!read_video.isOpened()) {
			fprintf(stderr, "open video file fail\n");
			return -1;
		}

		cv::Mat frame, tmp;
		if (!read_video.read(frame)) {
			fprintf(stderr, "read video frame fail\n");
			return -1;
		}

		fprintf(stderr, "src frame size: (%d, %d)\n", frame.cols, frame.rows);

		int width{ 640 }, height{ 480 };
		int codec = CV_FOURCC('M', 'J', 'P', 'G');
		double fps = 25.0;
		bool isColor = (frame.type() == CV_8UC3);
		cv::VideoWriter write_video;
#ifdef _MSC_VER
		write_video.open("E:/GitCode/OpenCV_Test/test_images/video_2.avi", codec, fps, cv::Size(width, height), isColor);
#else
		write_video.open("test_images/video_2.avi", codec, fps, cv::Size(width, height), isColor);
#endif
		if (!write_video.isOpened()) {
			fprintf(stderr, "open video file fail\n");
			return -1;
		}

		int count{ 0 };
		while (read_video.read(frame)) {
			// fprintf(stderr, "frame width: %d, frame height: %d\n", frame.cols, frame.rows);
			cv::resize(frame, tmp, cv::Size(width, height));
			write_video.write(tmp);

			if (++count > 50)
				break;
		}

		fprintf(stderr, "dst frame size: (%d, %d)\n", tmp.cols, tmp.rows);
	}

	return 0;
}

int test_encode_decode()
{
	// Blog: http://blog.csdn.net/fengbingchun/article/details/60780232
	// cv::imread/cv::imwrite
#ifdef _MSC_VER
	std::string image_name = "E:/GitCode/OpenCV_Test/test_images/1.jpg";
#else
	std::string image_name = "test_images/1.jpg";
#endif
	cv::Mat mat1 = cv::imread(image_name, 1);
	if (mat1.empty()) {
		fprintf(stderr, "read image fail: %s\n", image_name.c_str());
		return -1;
	}
#ifdef _MSC_VER
	std::string save_image = "E:/GitCode/OpenCV_Test/test_images/1_1.jpg";
#else
	std::string save_image = "test_images/1_1.jpg";
#endif
	cv::imwrite(save_image, mat1);

	// cv::imdecode/cv::imencode
	std::ifstream file(image_name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		fprintf(stderr, "open file fail: %s\n", image_name.c_str());
		return -1;
	}

	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string buffer(size, ' ');
	file.read(&buffer[0], size);
	file.close();

	std::vector<char> vec_data(&buffer[0], &buffer[0] + size);
	cv::Mat mat2 = cv::imdecode(vec_data, 1);
#ifdef _MSC_VER
	std::string save_image2 = "E:/GitCode/OpenCV_Test/test_images/2_1.jpg";
#else
	std::string save_image2 = "test_images/2_1.jpg";
#endif
	cv::imwrite(save_image2, mat2);

	std::vector<uchar> buf;
	cv::imencode(".jpg", mat1, buf);
#ifdef _MSC_VER
	std::string save_image3 = "E:/GitCode/OpenCV_Test/test_images/2_2.jpg";
#else
	std::string save_image3 = "test_images/2_2.jpg";
#endif
	std::ofstream file2(save_image3.c_str(), std::ios::out | std::ios::binary);
	if (!file2) {
		fprintf(stderr, "open file fail: %s\n", save_image3.c_str());
		return -1;
	}
	file2.write((char*)&buf[0], buf.size()*sizeof(uchar));
	file2.close();

	cv::Mat image1 = cv::imread(save_image, 1);
	cv::Mat image2 = cv::imread(save_image2, 1);
	cv::Mat image3 = cv::imread(save_image3, 1);
	if (!image1.data || !image2.data || !image3.data) {
		fprintf(stderr, "read image fail\n");
		return -1;
	}

	if (image1.rows != image2.rows || image1.cols != image2.cols ||
		image1.rows != image3.rows || image1.cols != image3.cols ||
		image1.step != image2.step || image1.step != image3.step) {
		fprintf(stderr, "their size are different\n");
		return -1;
	}

	for (int h = 0; h < image1.rows; ++h) {
		const uchar* p1 = image1.ptr(h);
		const uchar* p2 = image2.ptr(h);
		const uchar* p3 = image3.ptr(h);

		for (int w = 0; w < image1.cols; ++w) {
			if (p1[w] != p2[w] || p1[w] != p3[w]) {
				fprintf(stderr, "their value are different\n");
				return -1;
			}
		}
	}

	fprintf(stdout, "test image encode/decode, imread/imwrite finish\n");

	return 0;
}

int test_opencv_resize()
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
	mat.convertTo(mat, CV_8UC3);

	cv::Mat mat1_(mat.rows, mat.cols, CV_8UC3, mat.data);
	cv::Mat mat2_(mat1_);
	mat1_.convertTo(mat2_, CV_8UC3);
	cv::Mat mat3_(11, 23, CV_8UC3);
	cv::resize(mat2_, mat3_, cv::Size(23, 11), 0, 0, 4); // (23, 11) (256, 256) (888, 999)

	return 0;
}

int test_opencv_cvtColor()
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
	//cv::cvtColor(mat, mat, CV_BGR2YCrCb);
	cv::resize(mat, mat, cv::Size(20, 60));
	//mat.convertTo(mat, CV_32FC3);

	cv::Mat matDst;
	cv::cvtColor(mat, matDst, CV_BGR2YUV_I420);

	return 0;
}

int test_opencv_split()
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
	//cv::cvtColor(matSrc, matSrc, CV_BGR2GRAY);
	matSrc.convertTo(matSrc, CV_32FC3);

	std::vector<cv::Mat> matDst;
	cv::split(matSrc, matDst);

	return 0;
}

int test_opencv_merge()
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

	std::vector<cv::Mat> matSrc;
	matSrc.push_back(matSrc1);
	matSrc.push_back(matSrc2);
	matSrc.push_back(matSrc3);

	cv::Mat matDst;
	cv::merge(matSrc, matDst);

	return 0;
}

int test_opencv_warpAffine()
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
	//matSrc.convertTo(matSrc, CV_32FC3);

	cv::Point2f srcTri[3];
	cv::Point2f dstTri[3];

	cv::Mat rot_mat(2, 3, CV_32FC1);
	cv::Mat warp_mat(2, 3, CV_32FC1);
	cv::Mat warp_dst, warp_rotate_dst;

	// Set the dst image the same type and size as src
	warp_dst = cv::Mat::zeros(matSrc.rows / 2, matSrc.cols / 2, matSrc.type());

	// Set your 3 points to calculate the  Affine Transform
	srcTri[0] = cv::Point2f(0, 0);
	srcTri[1] = cv::Point2f(matSrc.cols - 1, 0);
	srcTri[2] = cv::Point2f(0, matSrc.rows - 1);

	dstTri[0] = cv::Point2f(matSrc.cols*0.0, matSrc.rows*0.33);
	dstTri[1] = cv::Point2f(matSrc.cols*0.85, matSrc.rows*0.25);
	dstTri[2] = cv::Point2f(matSrc.cols*0.15, matSrc.rows*0.7);

	// Get the Affine Transform
	warp_mat = cv::getAffineTransform(srcTri, dstTri);

	// Apply the Affine Transform just found to the src image
	cv::warpAffine(matSrc, warp_dst, warp_mat, warp_dst.size(), 1);

	/** Rotating the image after Warp */
	// Compute a rotation matrix with respect to the center of the image
	cv::Point center = cv::Point(warp_dst.cols / 2, warp_dst.rows / 2);
	double angle = -50.0;
	double scale = 0.6;

	// Get the rotation matrix with the specifications above
	rot_mat = getRotationMatrix2D(center, angle, scale);

	// Rotate the warped image
	cv::warpAffine(warp_dst, warp_rotate_dst, rot_mat, warp_dst.size());

	return 0;
}

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
				}
				else {
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

int test_opencv_remap()
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

	cv::Mat matDst;
	cv::Mat map_x, map_y;
	//matSrc.convertTo(matSrc, CV_32FC3);

	// Create dst, map_x and map_y with the same size as src:
	matDst.create(matSrc.size(), matSrc.type());
	map_x.create(matSrc.size(), CV_32FC1);
	map_y.create(matSrc.size(), CV_32FC1);

	char* remap_window = "Remap demo";
	cv::namedWindow(remap_window, CV_WINDOW_AUTOSIZE);

	int ind = 0;

	while (true) {
		// Each 1 sec. Press ESC to exit the program
		int c = cv::waitKey(1000);

		if ((char)c == 27) {
			break;
		}

		// Update map_x & map_y. Then apply remap
		update_map(matSrc, map_x, map_y, ind);
		cv::remap(matSrc, matDst, map_x, map_y, 1, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

		/// Display results
		cv::imshow(remap_window, matDst);

		ind++;
	}

	return 0;
}

int test_opencv_rotate()
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


	// Compute a rotation matrix with respect to the center of the image
	cv::Point center = cv::Point(matSrc.cols / 2, matSrc.rows / 2);
	double angle = -50.0;
	double scale = 0.6;

	// Get the rotation matrix with the specifications above
	cv::Mat mat_rot = getRotationMatrix2D(center, angle, scale);
	cv::Mat rotate_dst;

	//cv::Rect bbox = cv::RotatedRect(center, matSrc.size(), angle).boundingRect();

	// Rotate the warped image
	cv::warpAffine(matSrc, rotate_dst, mat_rot, matSrc.size());

	return 0;
}

int test_opencv_warpPerspective()
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

	cv::Point2f src_vertices[4], dst_vertices[4];
	src_vertices[0] = cv::Point2f(0, 0);
	src_vertices[1] = cv::Point2f(matSrc.cols - 5, 0);
	src_vertices[2] = cv::Point2f(matSrc.cols - 10, matSrc.rows - 1);
	src_vertices[3] = cv::Point2f(8, matSrc.rows - 13);

	dst_vertices[0] = cv::Point2f(17, 21);
	dst_vertices[1] = cv::Point2f(matSrc.cols - 23, 19);
	dst_vertices[2] = cv::Point2f(matSrc.cols / 2 + 5, matSrc.rows / 3 + 7);
	dst_vertices[3] = cv::Point2f(55, matSrc.rows / 5 + 33);

	cv::Mat warpMatrix = cv::getPerspectiveTransform(src_vertices, dst_vertices);

	cv::Mat matDst;
	cv::warpPerspective(matSrc, matDst, warpMatrix, matSrc.size(), 0);

	return 0;
}

int test_opencv_dilate()
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

	int dilation_elem = 0;
	int dilation_size = 5;
	int dilation_type;

	if (dilation_elem == 0){ dilation_type = cv::MORPH_RECT; }
	else if (dilation_elem == 1){ dilation_type = cv::MORPH_CROSS; }
	else if (dilation_elem == 2) { dilation_type = cv::MORPH_ELLIPSE; }

	cv::Mat element = cv::getStructuringElement(dilation_type,
		cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
		cv::Point(dilation_size, dilation_size));
	/// Apply the dilation operation
	cv::Mat matDst;
	cv::dilate(matSrc, matDst, element, cv::Point(-1, -1), 2, 0, cv::Scalar::all(128));

	return 0;
}

int test_opencv_erode()
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

	int erosion_elem = 0;
	int erosion_size = 1;
	int erosion_type;

	if (erosion_elem == 0){ erosion_type = cv::MORPH_RECT; }
	else if (erosion_elem == 1){ erosion_type = cv::MORPH_CROSS; }
	else if (erosion_elem == 2) { erosion_type = cv::MORPH_ELLIPSE; }

	//cv::Mat element = cv::getStructuringElement(erosion_type,
	//	cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
	//	cv::Point(erosion_size, erosion_size));
	cv::Mat element;

	/// Apply the erosion operation
	cv::Mat matDst;
	cv::erode(matSrc, matDst, element);

	return 0;
}

int test_opencv_morphologyEx()
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

	//cv::Mat src_complement;
	//cv::bitwise_not(matSrc, src_complement);

	int morph_elem = 0;
	int morph_size = 1;
	int morph_operator = 0;

	// Since MORPH_X : 2,3,4,5 and 6
	int operation = morph_operator + 2;

	cv::Mat element = cv::getStructuringElement(morph_elem, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));

	/// Apply the specified morphology operation
	cv::Mat matDst;
	morphologyEx(matSrc, matDst, operation, element, cv::Point(-1, -1), 2, 0, cv::Scalar::all(128));

	return 0;
}

int test_opencv_threshold()
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

	double thresh = 128;
	double maxval = 255;
	int type = 8;
	cv::Mat matDst;
	cv::threshold(matSrc, matDst, thresh, maxval, type);

	return 0;
}

int test_opencv_transpose()
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

	cv::Mat matDst;
	cv::transpose(matSrc, matDst);

	return 0;
}

int test_opencv_flip()
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
	matSrc.convertTo(matSrc, CV_32FC3);

	cv::Mat matDst;
	cv::flip(matSrc, matDst, -1);

	return 0;
}

int test_opencv_dft()
{
#ifdef _MSC_VER
	cv::Mat I = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
#else	
	cv::Mat I = cv::imread("test_images/1.jpg", 1);
#endif
	if (I.empty()) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}
	cv::cvtColor(I, I, CV_BGR2GRAY);

	cv::Mat padded;                            //expand input image to optimal size
	int m = cv::getOptimalDFTSize(I.rows);
	int n = cv::getOptimalDFTSize(I.cols); // on the border add zero values
	cv::copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);         // Add to the expanded another plane with zeros
	cv::Mat complexI_;
	cv::dft(complexI, complexI_);            // this way the result may fit in the source matrix

	// compute the magnitude and switch to logarithmic scale
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	cv::split(complexI_, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	cv::Mat magI = planes[0];

	magI += cv::Scalar::all(1);                    // switch to logarithmic scale
	cv::log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
	cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
	cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

	cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	cv::normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
	// viewable image form (float between values 0 and 1).

	cv::imshow("Input Image", I);    // Show the result
	cv::imshow("spectrum magnitude", magI);
	cv::waitKey();

	return 0;
}

int test_opencv_filter2D()
{
#ifdef _MSC_VER
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
#else	
	cv::Mat matSrc = cv::imread("test_images/1.jpg", 1);
#endif
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	cv::Mat kernal = (cv::Mat_<uchar>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);

	cv::Mat matDst;
	cv::filter2D(matSrc, matDst, matSrc.depth(), kernal);

	return 0;
}
