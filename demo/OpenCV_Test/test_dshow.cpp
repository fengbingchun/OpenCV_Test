#include "fbc_cv_funset.hpp"
#include <videocapture.hpp>
#include <opencv2/opencv.hpp>

// Blog: https://blog.csdn.net/fengbingchun/article/details/102641967

int test_dshow()
{
#ifdef _MSC_VER
	int width = 640;
	int height = 480;
	int length = width * height * 3;

	fbc::VideoCapture capture(0);
	if (!capture.isOpened()) {
		fprintf(stderr, "fail to open capture\n");
		return -1;
	}

	capture.set(fbc::CV_CAP_PROP_FOURCC, fbc::CV_FOURCC('M', 'J', 'P', 'G'));
	capture.set(fbc::CV_CAP_PROP_FRAME_WIDTH, width);
	capture.set(fbc::CV_CAP_PROP_FRAME_HEIGHT, height);

	fbc::Mat_<unsigned char, 3> image(height, width);
	cv::Mat mat(height, width, CV_8UC3);
	const char* winname = "dshow video";
	cv::namedWindow(winname, 1);

	while (1) {
		capture >> image;
		mat.data = image.data;

		cv::imshow(winname, mat);
		if (cv::waitKey(30) >= 0)
			break;
	}

	return 0;
#else
	fprintf(stderr, "Error: only support windows platform\n");
	return -1;
#endif
}
