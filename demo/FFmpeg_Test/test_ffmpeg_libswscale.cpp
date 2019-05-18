#include "funset.hpp"
#include <iostream>
#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <libswscale/swscale.h>
#include <libavutil/mem.h>

#ifdef __cplusplus
}
#endif

// Blog: https://blog.csdn.net/fengbingchun/article/details/90313518

int test_ffmpeg_libswscale_scale()
{
	// bgr to rgb and resize
#ifdef _MSC_VER
	const char* image_name = "E:/GitCode/OpenCV_Test/test_images/lena.png";
#else
	const char* image_name = "test_images/lena.png";	
#endif
	cv::Mat src = cv::imread(image_name, 1); 
	if (!src.data || src.channels() != 3) {
		fprintf(stderr, "fail to read image: %s\n", image_name);
		return -1;
	}
	
	int width_src = src.cols, height_src = src.rows;
	int width_dst = width_src / 1.5, height_dst = height_src / 1.2;
	std::unique_ptr<uint8_t[]> data(new uint8_t[width_dst * height_dst * 3]);

	SwsContext* ctx = sws_getContext(width_src, height_src, AV_PIX_FMT_BGR24, width_dst, height_dst, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
	if (!ctx) {
		fprintf(stderr, "fail to sws_getContext\n");
		return -1;
	}
	
	const uint8_t* p1[1] = {(const uint8_t*)src.data};
	uint8_t* p2[1] = {data.get()};
	int src_stride[1] = {width_src * 3};
	int dst_stride[1] = {width_dst * 3};
	sws_scale(ctx, p1, src_stride, 0, height_src, p2, dst_stride);
#ifdef _MSC_VER
	const char* result_image_name = "E:/GitCode/OpenCV_Test/test_images/lena_resize_rgb_libswscale.png";
#else
	const char* result_image_name = "test_images/lena_resize_rgb_libswscale.png";
#endif
	cv::Mat dst(height_dst, width_dst, CV_8UC3, (unsigned char*)data.get());
	cv::imwrite(result_image_name, dst);

	sws_freeContext(ctx);
	
	return 0;
}

int test_ffmpeg_libswscale_colorspace()
{
	fprintf(stdout, "swscale configuration: %s\n", swscale_configuration());
	fprintf(stdout, "swscale license: %s\n", swscale_license());

	AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
	fprintf(stdout, "is supported input:: %d\n", sws_isSupportedInput(pix_fmt));

	pix_fmt = AV_PIX_FMT_BGR24;
	fprintf(stdout, "is supported output: %d\n", sws_isSupportedOutput(pix_fmt));

	pix_fmt = AV_PIX_FMT_GRAY8;
	fprintf(stdout, "is supported endianness conversion: %d\n", sws_isSupportedEndiannessConversion(pix_fmt));

	// bgr to gray
#ifdef _MSC_VER
	const char* image_name = "E:/GitCode/OpenCV_Test/test_images/lena.png";
#else
	const char* image_name = "test_images/lena.png";	
#endif
	cv::Mat src = cv::imread(image_name, 1); 
	if (!src.data || src.channels() != 3) {
		fprintf(stderr, "fail to read image: %s\n", image_name);
		return -1;
	}
	
	int width = src.cols, height = src.rows;
	std::unique_ptr<uint8_t[]> data(new uint8_t[width * height]);

	SwsContext* ctx = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, AV_PIX_FMT_GRAY8, 0, nullptr, nullptr, nullptr);
	if (!ctx) {
		fprintf(stderr, "fail to sws_getContext\n");
		return -1;
	}
	
	const uint8_t* p1[1] = {(const uint8_t*)src.data};
	uint8_t* p2[1] = {data.get()};
	int src_stride[1] = {width*3};
	int dst_stride[1] = {width};
	sws_scale(ctx, p1, src_stride, 0, height, p2, dst_stride);
#ifdef _MSC_VER
	const char* result_image_name = "E:/GitCode/OpenCV_Test/test_images/lena_gray_libswscale.png";
#else
	const char* result_image_name = "test_images/lena_gray_libswscale.png";
#endif
	cv::Mat dst(height, width, CV_8UC1, (unsigned char*)data.get());
	cv::imwrite(result_image_name, dst);

	sws_freeContext(ctx);

	return 0;
}

