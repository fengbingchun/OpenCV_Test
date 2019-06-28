#include "funset.hpp"
#include <string.h>
#include <iostream>
#include <string>
#include <memory>
#include <fstream>

#include <opencv2/opencv.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <libswscale/swscale.h>
#include <libavutil/mem.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/90313518

int test_ffmpeg_libswscale_bgr_yuv()
{
#ifdef _MSC_VER
	const char* image_name = "E:/GitCode/OpenCV_Test/test_images/lena.png";
#else
	const char* image_name = "test_images/lena.png";
#endif
	cv::Mat mat = cv::imread(image_name, 1);
	if (!mat.data || mat.channels() != 3) {
		fprintf(stdout, "fail to read image: %s\n", image_name);
		return -1;
	}

	int width = mat.cols, height = mat.rows;
	std::unique_ptr<unsigned char[]> data(new unsigned char[width * height * 3 / 2]);
	std::unique_ptr<unsigned char[]> data2(new unsigned char[width * height * 3 / 2]);

{ // bgr --> yuv420p
	int align = 1;
	uint8_t *bgr_data[4], *yuv420p_data[4];
	int bgr_linesize[4], yuv420p_linesize[4];
	int bytes1 = av_image_alloc(bgr_data, bgr_linesize, width, height, AV_PIX_FMT_BGR24, align);
	memcpy(bgr_data[0], mat.data, width*height * 3);
	int bytes2 = av_image_alloc(yuv420p_data, yuv420p_linesize, width, height, AV_PIX_FMT_YUV420P, align);
	fprintf(stdout, "bgr size: %d, linesize: %d, %d, %d, yuv420p size: %d, linesize: %d, %d, %d\n",
		bytes1, bgr_linesize[0], bgr_linesize[1], bgr_linesize[2], bytes2, yuv420p_linesize[0], yuv420p_linesize[1], yuv420p_linesize[2]);
	if (bytes1 < 0 || bytes2 < 0) {
		fprintf(stderr, "bgr or yuv420p alloc buffer failed: %d, %d\n", bytes1, bytes2);
		return -1;
	}

	SwsContext* sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, AV_PIX_FMT_YUV420P, 0, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		fprintf(stderr, "fail to sws_getContext\n");
		return -1;
	}

	sws_scale(sws_ctx, bgr_data, bgr_linesize, 0, height, yuv420p_data, yuv420p_linesize);

#ifdef _MSC_VER
	const char* name = "E:/GitCode/OpenCV_Test/test_images/512w_512h.yuv420p";
#else
	const char* name = "test_images/512w_512h.yuv420p";
#endif
	std::ofstream fout(name, std::ios::out | std::ios::binary);
	if (!fout.is_open()) {
		fprintf(stderr, "fail to open file: %s\n", name);
		return -1;
	}

	memcpy(data.get(), yuv420p_data[0], width*height);
	memcpy(data.get() + width*height, yuv420p_data[1], width*height / 4);
	memcpy(data.get() + width*height * 5 / 4, yuv420p_data[2], width*height / 4);

	fout.write((char*)data.get(), width * height * 3 / 2);

	fout.close();
	av_freep(&bgr_data[0]);
	av_freep(&yuv420p_data[0]);
	sws_freeContext(sws_ctx);
}

{ // yuv420p --> bgr24
	int align = 1;
	uint8_t *bgr_data[4], *yuv420p_data[4];
	int bgr_linesize[4], yuv420p_linesize[4];
	int bytes1 = av_image_alloc(bgr_data, bgr_linesize, width, height, AV_PIX_FMT_BGR24, align);
	int bytes2 = av_image_alloc(yuv420p_data, yuv420p_linesize, width, height, AV_PIX_FMT_YUV420P, align);
	memcpy(yuv420p_data[0], data.get(), width*height);
	memcpy(yuv420p_data[1], data.get() + width*height, width*height / 4);
	memcpy(yuv420p_data[2], data.get() + width*height * 5 / 4, width*height / 4);
	fprintf(stdout, "bgr size: %d, linesize: %d, %d, %d, yuv420p size: %d, linesize: %d, %d, %d\n",
		bytes1, bgr_linesize[0], bgr_linesize[1], bgr_linesize[2], bytes2, yuv420p_linesize[0], yuv420p_linesize[1], yuv420p_linesize[2]);
	if (bytes1 < 0 || bytes2 < 0) {
		fprintf(stderr, "bgr or yuv420p alloc buffer failed: %d, %d\n", bytes1, bytes2);
		return -1;
	}

	SwsContext* sws_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		fprintf(stderr, "fail to sws_getContext\n");
		return -1;
	}

	sws_scale(sws_ctx, yuv420p_data, yuv420p_linesize, 0, height, bgr_data, bgr_linesize);

#ifdef _MSC_VER
	const char* name = "E:/GitCode/OpenCV_Test/test_images/yuv420ptobgr24.jpg";
#else
	const char* name = "test_images/yuv420ptobgr24.jpg";
#endif

	cv::Mat dst(height, width, CV_8UC3, bgr_data[0]);
	cv::imwrite(name, dst);

	av_freep(&bgr_data[0]);
	av_freep(&yuv420p_data[0]);
	sws_freeContext(sws_ctx);
}

{ // bgr --> nv12
	int align = 1;
	uint8_t *bgr_data[4], *nv12_data[4];
	int bgr_linesize[4], nv12_linesize[4];
	int bytes1 = av_image_alloc(bgr_data, bgr_linesize, width, height, AV_PIX_FMT_BGR24, align);
	memcpy(bgr_data[0], mat.data, width*height * 3);
	int bytes2 = av_image_alloc(nv12_data, nv12_linesize, width, height, AV_PIX_FMT_NV12, align);
	fprintf(stdout, "bgr size: %d, linesize: %d, %d, %d, nv12 size: %d, linesize: %d, %d, %d\n",
		bytes1, bgr_linesize[0], bgr_linesize[1], bgr_linesize[2], bytes2, nv12_linesize[0], nv12_linesize[1], nv12_linesize[2]);
	if (bytes1 < 0 || bytes2 < 0) {
		fprintf(stderr, "bgr or nv12 alloc buffer failed: %d, %d\n", bytes1, bytes2);
		return -1;
	}

	SwsContext* sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, AV_PIX_FMT_NV12, 0, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		fprintf(stderr, "fail to sws_getContext\n");
		return -1;
	}

	sws_scale(sws_ctx, bgr_data, bgr_linesize, 0, height, nv12_data, nv12_linesize);

#ifdef _MSC_VER
	const char* name = "E:/GitCode/OpenCV_Test/test_images/512w_512h.nv12";
#else
	const char* name = "test_images/512w_512h.nv12";
#endif
	std::ofstream fout(name, std::ios::out | std::ios::binary);
	if (!fout.is_open()) {
		fprintf(stderr, "fail to open file: %s\n", name);
		return -1;
	}

	memcpy(data2.get(), nv12_data[0], width*height);
	memcpy(data2.get() + width*height, nv12_data[1], width*height / 2);

	fout.write((char*)data2.get(), width * height * 3 / 2);

	fout.close();
	av_freep(&bgr_data[0]);
	av_freep(&nv12_data[0]);
	sws_freeContext(sws_ctx);
}

{ // nv12 --> bgr24
	int align = 1;
	uint8_t *bgr_data[4], *nv12_data[4];
	int bgr_linesize[4], nv12_linesize[4];
	int bytes1 = av_image_alloc(bgr_data, bgr_linesize, width, height, AV_PIX_FMT_BGR24, align);
	int bytes2 = av_image_alloc(nv12_data, nv12_linesize, width, height, AV_PIX_FMT_NV12, align);
	memcpy(nv12_data[0], data2.get(), width*height);
	memcpy(nv12_data[1], data2.get() + width*height, width*height / 2);
	fprintf(stdout, "bgr size: %d, linesize: %d, %d, %d, nv12 size: %d, linesize: %d, %d, %d\n",
		bytes1, bgr_linesize[0], bgr_linesize[1], bgr_linesize[2], bytes2, nv12_linesize[0], nv12_linesize[1], nv12_linesize[2]);
	if (bytes1 < 0 || bytes2 < 0) {
		fprintf(stderr, "bgr or nv12 alloc buffer failed: %d, %d\n", bytes1, bytes2);
		return -1;
	}

	SwsContext* sws_ctx = sws_getContext(width, height, AV_PIX_FMT_NV12, width, height, AV_PIX_FMT_BGR24, 0, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		fprintf(stderr, "fail to sws_getContext\n");
		return -1;
	}

	sws_scale(sws_ctx, nv12_data, nv12_linesize, 0, height, bgr_data, bgr_linesize);

#ifdef _MSC_VER
	const char* name = "E:/GitCode/OpenCV_Test/test_images/nv12tobgr24.jpg";
#else
	const char* name = "test_images/nv12tobgr24.jpg";
#endif

	cv::Mat dst(height, width, CV_8UC3, bgr_data[0]);
	cv::imwrite(name, dst);

	av_freep(&bgr_data[0]);
	av_freep(&nv12_data[0]);
	sws_freeContext(sws_ctx);
}

	return 0;
}

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

