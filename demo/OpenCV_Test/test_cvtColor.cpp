#include <assert.h>
#include <core/mat.hpp>
#include <cvtColor.hpp>

#include <opencv2/opencv.hpp>

#include "fbc_cv_funset.hpp"

// Blog: http://blog.csdn.net/fengbingchun/article/details/51712532

int test_cvtColor_RGB2RGB()
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

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat3BGR mat1(height, width, mat.data);
	fbc::Mat3BGR mat2(mat1);
	fbc::Mat_<uchar, 4> mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_BGR2BGRA);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC4);
	cv::cvtColor(mat2_, mat3_, CV_BGR2BGRA);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC3);

	fbc::Mat_<float, 3> mat4(height, width, matf.data);
	fbc::Mat_<float, 3> mat5(mat4);
	fbc::Mat_<float, 4> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_BGR2BGRA);

	cv::Mat mat4_(height, width, CV_32FC3, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC4);
	cv::cvtColor(mat5_, mat6_, CV_BGR2BGRA);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_RGB2Gray()
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

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat3BGR mat1(height, width, mat.data);
	fbc::Mat3BGR mat2(mat1);
	fbc::Mat1Gray mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_BGR2GRAY);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC1);
	cv::cvtColor(mat2_, mat3_, CV_BGR2GRAY);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC3);

	fbc::Mat_<float, 3> mat4(height, width, matf.data);
	fbc::Mat_<float, 3> mat5(mat4);
	fbc::Mat_<float, 1> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_BGR2GRAY);

	cv::Mat mat4_(height, width, CV_32FC3, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC1);
	cv::cvtColor(mat5_, mat6_, CV_BGR2GRAY);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_Gray2RGB()
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
	cv::cvtColor(mat, mat, CV_BGR2GRAY);

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat1Gray mat1(height, width, mat.data);
	fbc::Mat1Gray mat2(mat1);
	fbc::Mat3BGR mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_GRAY2BGR);

	cv::Mat mat1_(height, width, CV_8UC1, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC3);
	cv::cvtColor(mat2_, mat3_, CV_GRAY2BGR);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC1);

	fbc::Mat_<float, 1> mat4(height, width, matf.data);
	fbc::Mat_<float, 1> mat5(mat4);
	fbc::Mat_<float, 3> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_GRAY2BGR);

	cv::Mat mat4_(height, width, CV_32FC1, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC3);
	cv::cvtColor(mat5_, mat6_, CV_GRAY2BGR);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_RGB2YCrCb()
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

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat3BGR mat1(height, width, mat.data);
	fbc::Mat3BGR mat2(mat1);
	fbc::Mat_<uchar, 3> mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_BGR2YCrCb);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC3);
	cv::cvtColor(mat2_, mat3_, CV_BGR2YCrCb);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC3);

	fbc::Mat_<float, 3> mat4(height, width, matf.data);
	fbc::Mat_<float, 3> mat5(mat4);
	fbc::Mat_<float, 3> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_BGR2YCrCb);

	cv::Mat mat4_(height, width, CV_32FC3, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC3);
	cv::cvtColor(mat5_, mat6_, CV_BGR2YCrCb);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_YCrCb2RGB()
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
	cv::cvtColor(mat, mat, CV_BGR2YCrCb);

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
	fbc::Mat_<uchar, 3> mat2(mat1);
	fbc::Mat3BGR mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_YCrCb2BGR);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC3);
	cv::cvtColor(mat2_, mat3_, CV_YCrCb2BGR);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC3);

	fbc::Mat_<float, 3> mat4(height, width, matf.data);
	fbc::Mat_<float, 3> mat5(mat4);
	fbc::Mat_<float, 3> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_YCrCb2BGR);

	cv::Mat mat4_(height, width, CV_32FC3, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC3);
	cv::cvtColor(mat5_, mat6_, CV_YCrCb2BGR);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_RGB2XYZ()
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

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
	fbc::Mat_<uchar, 3> mat2(mat1);
	fbc::Mat_<uchar, 3> mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_BGR2XYZ);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC3);
	cv::cvtColor(mat2_, mat3_, CV_BGR2XYZ);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC3);

	fbc::Mat_<float, 3> mat4(height, width, matf.data);
	fbc::Mat_<float, 3> mat5(mat4);
	fbc::Mat_<float, 3> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_BGR2XYZ);

	cv::Mat mat4_(height, width, CV_32FC3, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC3);
	cv::cvtColor(mat5_, mat6_, CV_BGR2XYZ);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_XYZ2RGB()
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
	cv::cvtColor(mat, mat, CV_BGR2XYZ);

	int width = mat.cols;
	int height = mat.rows;

	// uchar
	fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
	fbc::Mat_<uchar, 3> mat2(mat1);
	fbc::Mat_<uchar, 3> mat3(height, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_XYZ2BGR);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(height, width, CV_8UC3);
	cv::cvtColor(mat2_, mat3_, CV_XYZ2BGR);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	// float
	cv::Mat matf;
	mat.convertTo(matf, CV_32FC3);

	fbc::Mat_<float, 3> mat4(height, width, matf.data);
	fbc::Mat_<float, 3> mat5(mat4);
	fbc::Mat_<float, 3> mat6(height, width);
	fbc::cvtColor(mat5, mat6, fbc::CV_XYZ2BGR);

	cv::Mat mat4_(height, width, CV_32FC3, matf.data);
	cv::Mat mat5_;
	mat4_.copyTo(mat5_);
	cv::Mat mat6_(height, width, CV_32FC3);
	cv::cvtColor(mat5_, mat6_, CV_XYZ2BGR);

	assert(mat6.step == mat6_.step);
	for (int y = 0; y < mat6.rows; y++) {
		const fbc::uchar* p = mat6.ptr(y);
		const uchar* p_ = mat6_.ptr(y);

		for (int x = 0; x < mat6.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_RGB2HSV()
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

	int code[] = { fbc::CV_BGR2HSV, fbc::CV_BGR2HLS };

	for (int i = 0; i < 2; i++) {
		int width = mat.cols;
		int height = mat.rows;

		// uchar
		fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
		fbc::Mat_<uchar, 3> mat2(mat1);
		fbc::Mat_<uchar, 3> mat3(height, width);
		fbc::cvtColor(mat2, mat3, code[i]);

		cv::Mat mat1_(height, width, CV_8UC3, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(height, width, CV_8UC3);
		cv::cvtColor(mat2_, mat3_, code[i]);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}

		// float
		cv::Mat matf;
		mat.convertTo(matf, CV_32FC3);

		fbc::Mat_<float, 3> mat4(height, width, matf.data);
		fbc::Mat_<float, 3> mat5(mat4);
		fbc::Mat_<float, 3> mat6(height, width);
		fbc::cvtColor(mat5, mat6, code[i]);

		cv::Mat mat4_(height, width, CV_32FC3, matf.data);
		cv::Mat mat5_;
		mat4_.copyTo(mat5_);
		cv::Mat mat6_(height, width, CV_32FC3);
		cv::cvtColor(mat5_, mat6_, code[i]);

		assert(mat6.step == mat6_.step);
		for (int y = 0; y < mat6.rows; y++) {
			const fbc::uchar* p = mat6.ptr(y);
			const uchar* p_ = mat6_.ptr(y);

			for (int x = 0; x < mat6.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_cvtColor_HSV2RGB()
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

	int code[] = { fbc::CV_HSV2BGR, fbc::CV_HLS2BGR };
	int code1[] = { fbc::CV_BGR2HSV, fbc::CV_BGR2HLS };

	for (int i = 0; i < 2; i++) {
		cv::cvtColor(mat, mat, code1[i]);

		int width = mat.cols;
		int height = mat.rows;

		// uchar
		fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
		fbc::Mat_<uchar, 3> mat2(mat1);
		fbc::Mat_<uchar, 3> mat3(height, width);
		fbc::cvtColor(mat2, mat3, code[i]);

		cv::Mat mat1_(height, width, CV_8UC3, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(height, width, CV_8UC3);
		cv::cvtColor(mat2_, mat3_, code[i]);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}

		// float
		cv::Mat matf;
		mat.convertTo(matf, CV_32FC3);

		fbc::Mat_<float, 3> mat4(height, width, matf.data);
		fbc::Mat_<float, 3> mat5(mat4);
		fbc::Mat_<float, 3> mat6(height, width);
		fbc::cvtColor(mat5, mat6, code[i]);

		cv::Mat mat4_(height, width, CV_32FC3, matf.data);
		cv::Mat mat5_;
		mat4_.copyTo(mat5_);
		cv::Mat mat6_(height, width, CV_32FC3);
		cv::cvtColor(mat5_, mat6_, code[i]);

		assert(mat6.step == mat6_.step);
		for (int y = 0; y < mat6.rows; y++) {
			const fbc::uchar* p = mat6.ptr(y);
			const uchar* p_ = mat6_.ptr(y);

			for (int x = 0; x < mat6.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_cvtColor_RGB2Lab()
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

	int code[] = { fbc::CV_BGR2Lab, fbc::CV_BGR2Luv };

	for (int i = 0; i < 2; i++) {
		int width = mat.cols;
		int height = mat.rows;

		// uchar
		fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
		fbc::Mat_<uchar, 3> mat2(mat1);
		fbc::Mat_<uchar, 3> mat3(height, width);
		fbc::cvtColor(mat2, mat3, code[i]);

		cv::Mat mat1_(height, width, CV_8UC3, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(height, width, CV_8UC3);
		cv::cvtColor(mat2_, mat3_, code[i]);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}

		// float
		cv::Mat matf;
		mat.convertTo(matf, CV_32FC3);

		fbc::Mat_<float, 3> mat4(height, width, matf.data);
		fbc::Mat_<float, 3> mat5(mat4);
		fbc::Mat_<float, 3> mat6(height, width);
		fbc::cvtColor(mat5, mat6, code[i]);

		cv::Mat mat4_(height, width, CV_32FC3, matf.data);
		cv::Mat mat5_;
		mat4_.copyTo(mat5_);
		cv::Mat mat6_(height, width, CV_32FC3);
		cv::cvtColor(mat5_, mat6_, code[i]);

		assert(mat6.step == mat6_.step);
		for (int y = 0; y < mat6.rows; y++) {
			const fbc::uchar* p = mat6.ptr(y);
			const uchar* p_ = mat6_.ptr(y);

			for (int x = 0; x < mat6.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_cvtColor_Lab2RGB()
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

	int code[] = { fbc::CV_Lab2BGR, fbc::CV_Luv2BGR };
	int code1[] = { fbc::CV_BGR2Lab, fbc::CV_BGR2Luv };

	for (int i = 0; i < 2; i++) {
		cv::cvtColor(mat, mat, code1[i]);

		int width = mat.cols;
		int height = mat.rows;

		// uchar
		fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
		fbc::Mat_<uchar, 3> mat2(mat1);
		fbc::Mat_<uchar, 3> mat3(height, width);
		fbc::cvtColor(mat2, mat3, code[i]);

		cv::Mat mat1_(height, width, CV_8UC3, mat.data);
		cv::Mat mat2_;
		mat1_.copyTo(mat2_);
		cv::Mat mat3_(height, width, CV_8UC3);
		cv::cvtColor(mat2_, mat3_, code[i]);

		assert(mat3.step == mat3_.step);
		for (int y = 0; y < mat3.rows; y++) {
			const fbc::uchar* p = mat3.ptr(y);
			const uchar* p_ = mat3_.ptr(y);

			for (int x = 0; x < mat3.step; x++) {
				assert(p[x] == p_[x]);
			}
		}

		// float
		cv::Mat matf;
		mat.convertTo(matf, CV_32FC3);

		fbc::Mat_<float, 3> mat4(height, width, matf.data);
		fbc::Mat_<float, 3> mat5(mat4);
		fbc::Mat_<float, 3> mat6(height, width);
		fbc::cvtColor(mat5, mat6, code[i]);

		cv::Mat mat4_(height, width, CV_32FC3, matf.data);
		cv::Mat mat5_;
		mat4_.copyTo(mat5_);
		cv::Mat mat6_(height, width, CV_32FC3);
		cv::cvtColor(mat5_, mat6_, code[i]);

		assert(mat6.step == mat6_.step);
		for (int y = 0; y < mat6.rows; y++) {
			const fbc::uchar* p = mat6.ptr(y);
			const uchar* p_ = mat6_.ptr(y);

			for (int x = 0; x < mat6.step; x++) {
				assert(p[x] == p_[x]);
			}
		}
	}

	return 0;
}

int test_cvtColor_YUV2BGR()
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

	cv::cvtColor(mat, mat, CV_BGR2YUV_I420);

	int width = mat.cols;
	int height = mat.rows;
	int newHeight = height * 2 / 3;

	// uchar
	fbc::Mat_<uchar, 1> mat1(height, width, mat.data);
	fbc::Mat_<uchar, 1> mat2(mat1);
	fbc::Mat_<uchar, 3> mat3(newHeight, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_YUV2BGR_I420);

	cv::Mat mat1_(height, width, CV_8UC1, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(newHeight, width, CV_8UC3);
	cv::cvtColor(mat2_, mat3_, CV_YUV2BGR_I420);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_BGR2YUV()
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

	int width = mat.cols;
	int height = mat.rows;
	int newHeight = height / 2 * 3;

	// uchar
	fbc::Mat_<uchar, 3> mat1(height, width, mat.data);
	fbc::Mat_<uchar, 3> mat2(mat1);
	fbc::Mat_<uchar, 1> mat3(newHeight, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_BGR2YUV_YV12);

	cv::Mat mat1_(height, width, CV_8UC3, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(newHeight, width, CV_8UC1);
	cv::cvtColor(mat2_, mat3_, CV_BGR2YUV_YV12);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}

int test_cvtColor_YUV2Gray()
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
	cv::cvtColor(mat, mat, CV_BGRA2YUV_I420);

	int width = mat.cols;
	int height = mat.rows;
	int newHeight = height * 2 / 3;

	// uchar
	fbc::Mat_<uchar, 1> mat1(height, width, mat.data);
	fbc::Mat_<uchar, 1> mat2(mat1);
	fbc::Mat_<uchar, 1> mat3(newHeight, width);
	fbc::cvtColor(mat2, mat3, fbc::CV_YUV2GRAY_420);

	cv::Mat mat1_(height, width, CV_8UC1, mat.data);
	cv::Mat mat2_;
	mat1_.copyTo(mat2_);
	cv::Mat mat3_(newHeight, width, CV_8UC1);
	cv::cvtColor(mat2_, mat3_, CV_YUV2GRAY_420);

	assert(mat3.step == mat3_.step);
	for (int y = 0; y < mat3.rows; y++) {
		const fbc::uchar* p = mat3.ptr(y);
		const uchar* p_ = mat3_.ptr(y);

		for (int x = 0; x < mat3.step; x++) {
			assert(p[x] == p_[x]);
		}
	}

	return 0;
}
