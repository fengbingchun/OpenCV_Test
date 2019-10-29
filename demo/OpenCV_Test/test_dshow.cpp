#include "fbc_cv_funset.hpp"
#include <videocapture.hpp>
#include <opencv2/opencv.hpp>

int test_get_camera_info()
{
#ifdef _MSC_VER
	std::map<int, std::string> camera_names;
	fbc::get_camera_names(camera_names);
	fprintf(stdout, "camera count: %d\n", camera_names.size());
	for (auto it = camera_names.cbegin(); it != camera_names.cend(); ++it) {
		fprintf(stdout, "camera index: %d, name: %s\n", (*it).first, (*it).second.c_str());
	}

	fbc::VideoCapture capture(0);
	if (!capture.isOpened()) {
		fprintf(stderr, "fail to open capture\n");
		return -1;
	}

	std::vector<int> codecids;
	capture.getCodecList(codecids);
	fprintf(stdout, "support codec type(video_codec_type_t: 0: h264; 1: h265; 2: mjpeg; 3: rawvideo): ");
	for (auto i = 0; i < codecids.size(); ++i) {
		fprintf(stdout, "%d  ", codecids[i]);
	}
	fprintf(stdout, "\n");

	std::vector<std::string> sizelist;
	int codec_type = fbc::VIDEO_CODEC_TYPE_MJPEG;
	capture.getVideoSizeList(codec_type, sizelist);
	fprintf(stdout, "codec type: %d, support video size list:(width*height)\n", codec_type);
	for (auto it = sizelist.cbegin(); it != sizelist.cend(); ++it) {
		fprintf(stdout, "\t%s\n", (*it).c_str());
	}

	return 0;
#else
	fprintf(stderr, "Error: only support windows platform\n");
	return -1;
#endif
}

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/102641967
int test_dshow()
{
#ifdef _MSC_VER
	int width = 640;
	int height = 480;
	int length = width * height * 3;

	fbc::VideoCapture capture("Integrated Webcam"); // 0
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
