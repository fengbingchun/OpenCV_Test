#include "fbc_cv_funset.hpp"
#include <assert.h>

#include <dilate.hpp>
#include <opencv2/opencv.hpp>

// Blog: http://blog.csdn.net/fengbingchun/article/details/52333643

int test_getStructuringElement()
{
	for (int elem = 0; elem < 3; elem++) {
		for (int size = 0; size < 10; size++) {
			int type;
			if (elem == 0){ type = fbc::MORPH_RECT; }
			else if (elem == 1){ type = fbc::MORPH_CROSS; }
			else if (elem == 2) { type = fbc::MORPH_ELLIPSE; }

			fbc::Mat_<uchar, 1> element(2*size+1, 2*size+1);
			fbc::getStructuringElement(element, type, fbc::Size(2 * size + 1, 2 * size + 1), fbc::Point(size, size));

			int type_;
			if (elem == 0){ type_ = cv::MORPH_RECT; }
			else if (elem == 1){ type_ = cv::MORPH_CROSS; }
			else if (elem == 2) { type_ = cv::MORPH_ELLIPSE; }

			cv::Mat element_ = cv::getStructuringElement(type_, cv::Size(2 * size + 1, 2 * size + 1), cv::Point(size, size));

			assert(element.rows == element_.rows && element.cols == element.cols && element.step == element_.step);
			for (int y = 0; y < element.rows; y++) {
				const fbc::uchar* p1 = element.ptr(y);
				const uchar* p2 = element_.ptr(y);

				for (int x = 0; x < element.step; x++) {
					assert(p1[x] == p2[x]);
					if (size == 5)
						fprintf(stderr, "%d  ", p1[x]);
				}
				if (size == 5)
					fprintf(stderr, "\n");
			}

			if (size == 5)
				fprintf(stderr, "\n");
		}
	}

	return 0;
}

int test_dilate_uchar()
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

	int width = matSrc.cols;
	int height = matSrc.rows;

	for (int elem = 0; elem < 3; elem++) {
		for (int size = 0; size < 5; size++) {
			for (int iterations = 1; iterations < 3; iterations++) {
				int type;
				if (elem == 0){ type = fbc::MORPH_RECT; }
				else if (elem == 1){ type = fbc::MORPH_CROSS; }
				else if (elem == 2) { type = fbc::MORPH_ELLIPSE; }

				fbc::Mat_<uchar, 1> element(2 * size + 1, 2 * size + 1);
				fbc::getStructuringElement(element, type, fbc::Size(2 * size + 1, 2 * size + 1), fbc::Point(size, size));

				int type_;
				if (elem == 0){ type_ = cv::MORPH_RECT; }
				else if (elem == 1){ type_ = cv::MORPH_CROSS; }
				else if (elem == 2) { type_ = cv::MORPH_ELLIPSE; }

				cv::Mat element_ = cv::getStructuringElement(type_, cv::Size(2 * size + 1, 2 * size + 1), cv::Point(size, size));

				assert(element.rows == element_.rows && element.cols == element.cols && element.step == element_.step);
				for (int y = 0; y < element.rows; y++) {
					const fbc::uchar* p1 = element.ptr(y);
					const uchar* p2 = element_.ptr(y);

					for (int x = 0; x < element.step; x++) {
						assert(p1[x] == p2[x]);
					}
				}

				fbc::Mat3BGR mat1(height, width, matSrc.data);
				fbc::Mat3BGR mat2(height, width);
				fbc::dilate(mat1, mat2, element, fbc::Point(-1, -1), iterations, 0, fbc::Scalar::all(128));

				cv::Mat mat1_(height, width, CV_8UC3, matSrc.data);
				cv::Mat mat2_;
				cv::dilate(mat1_, mat2_, element_, cv::Point(-1, -1), iterations, 0, cv::Scalar::all(128));

				assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
				for (int y = 0; y < mat2.rows; y++) {
					const fbc::uchar* p1 = mat2.ptr(y);
					const uchar* p2 = mat2_.ptr(y);

					for (int x = 0; x < mat2.step; x++) {
						assert(p1[x] == p2[x]);
					}
				}
			}
		}
	}

	return 0;
}

int test_dilate_float()
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

	for (int elem = 0; elem < 3; elem++) {
		for (int size = 0; size < 5; size++) {
			for (int iterations = 1; iterations < 3; iterations++) {
				int type;
				if (elem == 0){ type = fbc::MORPH_RECT; }
				else if (elem == 1){ type = fbc::MORPH_CROSS; }
				else if (elem == 2) { type = fbc::MORPH_ELLIPSE; }

				fbc::Mat_<uchar, 1> element(2 * size + 1, 2 * size + 1);
				fbc::getStructuringElement(element, type, fbc::Size(2 * size + 1, 2 * size + 1), fbc::Point(size, size));

				int type_;
				if (elem == 0){ type_ = cv::MORPH_RECT; }
				else if (elem == 1){ type_ = cv::MORPH_CROSS; }
				else if (elem == 2) { type_ = cv::MORPH_ELLIPSE; }

				cv::Mat element_ = cv::getStructuringElement(type_, cv::Size(2 * size + 1, 2 * size + 1), cv::Point(size, size));

				assert(element.rows == element_.rows && element.cols == element.cols && element.step == element_.step);
				for (int y = 0; y < element.rows; y++) {
					const fbc::uchar* p1 = element.ptr(y);
					const uchar* p2 = element_.ptr(y);

					for (int x = 0; x < element.step; x++) {
						assert(p1[x] == p2[x]);
					}
				}

				fbc::Mat_<float, 1> mat1(height, width, matSrc.data);
				fbc::Mat_<float, 1> mat2(height, width);
				fbc::dilate(mat1, mat2, element, fbc::Point(-1, -1), iterations, 0, fbc::Scalar::all(128));

				cv::Mat mat1_(height, width, CV_32FC1, matSrc.data);
				cv::Mat mat2_;
				cv::dilate(mat1_, mat2_, element_, cv::Point(-1, -1), iterations, 0, cv::Scalar::all(128));

				assert(mat2.rows == mat2_.rows && mat2.cols == mat2_.cols && mat2.step == mat2_.step);
				for (int y = 0; y < mat2.rows; y++) {
					const fbc::uchar* p1 = mat2.ptr(y);
					const uchar* p2 = mat2_.ptr(y);

					for (int x = 0; x < mat2.step; x++) {
						assert(p1[x] == p2[x]);
					}
				}
			}
		}
	}

	return 0;
}
