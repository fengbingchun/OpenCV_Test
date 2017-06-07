#include "opencv_funset.hpp"

#include <math.h>
#include <string>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>

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
	std::vector<int> norm_types{ 1, 2, 4 }; // ÕýÎÞÇî¡¢L1¡¢L2
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
		cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg");
		if (mat.empty()) {
			fprintf(stderr, "read image fail\n");
			return -1;
		}

		int width{ 640 }, height{ 480 };
		int codec = CV_FOURCC('M', 'J', 'P', 'G');
		double fps = 25.0;
		bool isColor = (mat.type() == CV_8UC3);
		cv::VideoWriter write_video;
		write_video.open("E:/GitCode/OpenCV_Test/test_images/video_1.avi", codec, fps, cv::Size(width, height), isColor);
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
		cv::VideoCapture read_video("E:/GitCode/OpenCV_Test/test_images/video_1.avi");
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
		write_video.open("E:/GitCode/OpenCV_Test/test_images/video_2.avi", codec, fps, cv::Size(width, height), isColor);
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

int test_opencv_resize()
{
	cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat mat = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc1 = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
	cv::Mat matSrc2 = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
	cv::Mat matSrc3 = cv::imread("E:/GitCode/OpenCV_Test/test_images/2.jpg", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/lena.png", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
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
	cv::Mat I = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
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
	cv::Mat matSrc = cv::imread("E:/GitCode/OpenCV_Test/test_images/1.jpg", 1);
	if (!matSrc.data) {
		std::cout << "read image fail" << std::endl;
		return -1;
	}

	cv::Mat kernal = (cv::Mat_<uchar>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);

	cv::Mat matDst;
	cv::filter2D(matSrc, matDst, matSrc.depth(), kernal);

	return 0;
}
