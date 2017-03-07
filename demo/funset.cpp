#include "funset.hpp"

#include <string>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>

int test_encode_decode()
{
	// cv::imread/cv::imwrite
	std::string image_name = "E:/GitCode/OpenCV_Test/test_images/1.jpg";
	cv::Mat mat1 = cv::imread(image_name, 1);
	if (mat1.empty()) {
		fprintf(stderr, "read image fail: %s\n", image_name.c_str());
		return -1;
	}

	std::string save_image = "E:/GitCode/OpenCV_Test/test_images/1_1.jpg";
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
	std::string save_image2 = "E:/GitCode/OpenCV_Test/test_images/2_1.jpg";
	cv::imwrite(save_image2, mat2);

	std::vector<uchar> buf;
	cv::imencode(".jpg", mat1, buf);
	std::string save_image3 = "E:/GitCode/OpenCV_Test/test_images/2_2.jpg";
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
