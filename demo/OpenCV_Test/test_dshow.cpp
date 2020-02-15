#include <fstream>
#include <iostream>
#include "fbc_cv_funset.hpp"
#include <videocapture.hpp>
#include <opencv2/opencv.hpp>
#include <avdevice.hpp>
#include <avformat.hpp>
#include <avutil.hpp>
#include <avmem.hpp>

// Blog: https://blog.csdn.net/fengbingchun/article/details/104333073

int test_ffmpeg_dshow_mjpeg()
{
#ifdef _MSC_VER
	fbc::avdevice_register_all();

	fbc::AVFormatContext* format_context = fbc::avformat_alloc_context();
	fbc::AVCodecID id = fbc::AV_CODEC_ID_MJPEG;
	format_context->video_codec_id = id;

	fbc::AVInputFormat* input_format = fbc::av_find_input_format("dshow");
	if (!input_format) {
		fprintf(stderr, "Error: input format is not supported\n");
		return -1;
	}

	fbc::AVDictionary* dict = nullptr;
	int ret = fbc::av_dict_set(&dict, "video_size", "1280x720", 0);
	if (ret < 0) {
		fprintf(stderr, "Error: fail to av_dict_set: %d\n", ret);
		return -1;
	}

	ret = fbc::avformat_open_input(&format_context, "video=Integrated Webcam", input_format, &dict);
	if (ret != 0) {
		fprintf(stderr, "Error: fail to avformat_open_input: %d\n", ret);
		return -1;
	}

	int video_stream_index = -1;
	for (unsigned int i = 0; i < format_context->nb_streams; ++i) {
		const fbc::AVStream* stream = format_context->streams[i];
		if (stream->codecpar->codec_type == fbc::AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			fprintf(stdout, "type of the encoded data: %d, dimensions of the video frame in pixels: width: %d, height: %d, pixel format: %d\n",
				stream->codecpar->codec_id, stream->codecpar->width, stream->codecpar->height, stream->codecpar->format);
			//break;
		}
	}

	if (video_stream_index == -1) {
		fprintf(stderr, "Error: no video stream\n");
		return -1;
	}

	fbc::AVCodecParameters* codecpar = format_context->streams[video_stream_index]->codecpar;
	if (codecpar->codec_id != id) {
		fprintf(stderr, "Error: this test code only support mjpeg encode: %d\n", codecpar->codec_id);
		return -1;
	}

	fbc::AVPacket* packet = (fbc::AVPacket*)fbc::av_malloc(sizeof(fbc::AVPacket));
	if (!packet) {
		fprintf(stderr, "Error: fail to alloc\n");
		return -1;
	}

	std::ofstream out("E:/GitCode/OpenCV_Test/test_images/test.mjpeg", std::ios::binary | std::ios::out);
	if (!out.is_open()) {
		fprintf(stderr, "Error, fail to open file\n");
		return -1;
	}

	int count = 0;
	while (count++ < 100) {
		ret = fbc::av_read_frame(format_context, packet);
		if (ret >= 0 && packet->stream_index == video_stream_index && packet->size > 0) {
			fprintf(stdout, "packet size: %d\n", packet->size);
			out.write((char*)packet->data, packet->size);
		}
		else if (ret < 0 || packet->size <= 0) {
			fprintf(stderr, "Warnint: fail to av_read_frame: %d, packet size: %d\n", ret, packet->size);
			continue;
		}

		fbc::av_packet_unref(packet);
	}

	fbc::av_freep(packet);
	fbc::avformat_close_input(&format_context);
	fbc::av_dict_free(&dict);

	out.close();
	fprintf(stdout, "test finish\n");
	return 0;
#else
	fprintf(stderr, "Error: only support windows platform\n");
	return -1;
#endif
}

/////////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/102806822
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
int test_opencv_dshow()
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
