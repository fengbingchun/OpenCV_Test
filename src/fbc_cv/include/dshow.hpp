// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_DSHOW_HPP_
#define FBC_CV_DSHOW_HPP_

// reference: 2.4.13.6
//            highgui/src/cap_dshow.cpp

#ifdef _MSC_VER
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <map>
#include <set>
#include <windows.h>

#include <DShow.h>
#include <strmif.h>
#include <Aviriff.h>
#include <dvdmedia.h>
#include <bdaiface.h>

//for threading
#include <process.h>

//this is for TryEnterCriticalSection
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif

#include <initguid.h>

#include "capture.hpp"

//allows us to directShow classes here with the includes in the cpp
struct ICaptureGraphBuilder2;
struct IGraphBuilder;
struct IBaseFilter;
struct IAMCrossbar;
struct IMediaControl;
struct ISampleGrabber;
struct IMediaEventEx;
struct IAMStreamConfig;
struct _AMMediaType;
class SampleGrabberCallback;
typedef _AMMediaType AM_MEDIA_TYPE;

namespace fbc {

DEFINE_GUID(MEDIASUBTYPE_GREY, 0x59455247, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y8, 0x20203859, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y800, 0x30303859, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(CLSID_CaptureGraphBuilder2, 0xbf87b6e1, 0x8c27, 0x11d0, 0xb3, 0xf0, 0x00, 0xaa, 0x00, 0x37, 0x61, 0xc5);
DEFINE_GUID(CLSID_FilterGraph, 0xe436ebb3, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(CLSID_NullRenderer, 0xc1f400a4, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(CLSID_SampleGrabber, 0xc1f400a0, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(CLSID_SystemDeviceEnum, 0x62be5d10, 0x60eb, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(CLSID_VideoInputDeviceCategory, 0x860bb310, 0x5d01, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(FORMAT_VideoInfo, 0x05589f80, 0xc356, 0x11ce, 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a);
DEFINE_GUID(IID_IAMAnalogVideoDecoder, 0xc6e13350, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMCameraControl, 0xc6e13370, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMCrossbar, 0xc6e13380, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMStreamConfig, 0xc6e13340, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMVideoProcAmp, 0xc6e13360, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IBaseFilter, 0x56a86895, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_ICaptureGraphBuilder2, 0x93e5a4e0, 0x2d50, 0x11d2, 0xab, 0xfa, 0x00, 0xa0, 0xc9, 0xc6, 0xe3, 0x8d);
DEFINE_GUID(IID_ICreateDevEnum, 0x29840822, 0x5b84, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(IID_IGraphBuilder, 0x56a868a9, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_IMPEG2PIDMap, 0xafb6c2a1, 0x2c41, 0x11d3, 0x8a, 0x60, 0x00, 0x00, 0xf8, 0x1e, 0x0e, 0x4a);
DEFINE_GUID(IID_IMediaControl, 0x56a868b1, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_IMediaFilter, 0x56a86899, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_ISampleGrabber, 0x6b652fff, 0x11fe, 0x4fce, 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f);
DEFINE_GUID(LOOK_UPSTREAM_ONLY, 0xac798be0, 0x98e3, 0x11d1, 0xb3, 0xf1, 0x00, 0xaa, 0x00, 0x37, 0x61, 0xc5);
DEFINE_GUID(MEDIASUBTYPE_AYUV, 0x56555941, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_IYUV, 0x56555949, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_RGB24, 0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB32, 0xe436eb7e, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB555, 0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB565, 0xe436eb7b, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_UYVY, 0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y211, 0x31313259, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y411, 0x31313459, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y41P, 0x50313459, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YUY2, 0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YUYV, 0x56595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YV12, 0x32315659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YVU9, 0x39555659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YVYU, 0x55595659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MJPG, 0x47504A4D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); // MGB
DEFINE_GUID(MEDIATYPE_Interleaved, 0x73766169, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIATYPE_Video, 0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(PIN_CATEGORY_CAPTURE, 0xfb6c4281, 0x0353, 0x11d1, 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);
DEFINE_GUID(PIN_CATEGORY_PREVIEW, 0xfb6c4282, 0x0353, 0x11d1, 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);

// Pixel format
enum AVPixelFormat {
	AV_PIX_FMT_NONE = -1,
	AV_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
	AV_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
	AV_PIX_FMT_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
	AV_PIX_FMT_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
	AV_PIX_FMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
	AV_PIX_FMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
	AV_PIX_FMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
	AV_PIX_FMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
	AV_PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
	AV_PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
	AV_PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
	AV_PIX_FMT_PAL8,      ///< 8 bits with AV_PIX_FMT_RGB32 palette
	AV_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
	AV_PIX_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
	AV_PIX_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
	AV_PIX_FMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
	AV_PIX_FMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
	AV_PIX_FMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
	AV_PIX_FMT_BGR4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
	AV_PIX_FMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
	AV_PIX_FMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
	AV_PIX_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
	AV_PIX_FMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
	AV_PIX_FMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
	AV_PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped

	AV_PIX_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
	AV_PIX_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
	AV_PIX_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
	AV_PIX_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

	AV_PIX_FMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
	AV_PIX_FMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
	AV_PIX_FMT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
	AV_PIX_FMT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV440P and setting color_range
	AV_PIX_FMT_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
	AV_PIX_FMT_RGB48BE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
	AV_PIX_FMT_RGB48LE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

	AV_PIX_FMT_RGB565BE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
	AV_PIX_FMT_RGB565LE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
	AV_PIX_FMT_RGB555BE,  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
	AV_PIX_FMT_RGB555LE,  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined

	AV_PIX_FMT_BGR565BE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
	AV_PIX_FMT_BGR565LE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
	AV_PIX_FMT_BGR555BE,  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
	AV_PIX_FMT_BGR555LE,  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined

#if FF_API_VAAPI
	/** @name Deprecated pixel formats */
	/**@{*/
	AV_PIX_FMT_VAAPI_MOCO, ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
	AV_PIX_FMT_VAAPI_IDCT, ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
	AV_PIX_FMT_VAAPI_VLD,  ///< HW decoding through VA API, Picture.data[3] contains a VASurfaceID
	/**@}*/
	AV_PIX_FMT_VAAPI = AV_PIX_FMT_VAAPI_VLD,
#else
	/**
	*  Hardware acceleration through VA-API, data[3] contains a
	*  VASurfaceID.
	*/
	AV_PIX_FMT_VAAPI,
#endif

	AV_PIX_FMT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	AV_PIX_FMT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	AV_PIX_FMT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	AV_PIX_FMT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	AV_PIX_FMT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	AV_PIX_FMT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	AV_PIX_FMT_DXVA2_VLD,    ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

	AV_PIX_FMT_RGB444LE,  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
	AV_PIX_FMT_RGB444BE,  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
	AV_PIX_FMT_BGR444LE,  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
	AV_PIX_FMT_BGR444BE,  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
	AV_PIX_FMT_YA8,       ///< 8 bits gray, 8 bits alpha

	AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8, ///< alias for AV_PIX_FMT_YA8
	AV_PIX_FMT_GRAY8A = AV_PIX_FMT_YA8, ///< alias for AV_PIX_FMT_YA8

	AV_PIX_FMT_BGR48BE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
	AV_PIX_FMT_BGR48LE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

	/**
	* The following 12 formats have the disadvantage of needing 1 format for each bit depth.
	* Notice that each 9/10 bits sample is stored in 16 bits with extra padding.
	* If you want to support multiple bit depths, then using AV_PIX_FMT_YUV420P16* with the bpp stored separately is better.
	*/
	AV_PIX_FMT_YUV420P9BE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	AV_PIX_FMT_YUV420P9LE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	AV_PIX_FMT_YUV420P10BE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	AV_PIX_FMT_YUV420P10LE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	AV_PIX_FMT_YUV422P10BE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	AV_PIX_FMT_YUV422P10LE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	AV_PIX_FMT_YUV444P9BE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	AV_PIX_FMT_YUV444P9LE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	AV_PIX_FMT_YUV444P10BE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	AV_PIX_FMT_YUV444P10LE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	AV_PIX_FMT_YUV422P9BE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	AV_PIX_FMT_YUV422P9LE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	AV_PIX_FMT_GBRP,      ///< planar GBR 4:4:4 24bpp
	AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP, // alias for #AV_PIX_FMT_GBRP
	AV_PIX_FMT_GBRP9BE,   ///< planar GBR 4:4:4 27bpp, big-endian
	AV_PIX_FMT_GBRP9LE,   ///< planar GBR 4:4:4 27bpp, little-endian
	AV_PIX_FMT_GBRP10BE,  ///< planar GBR 4:4:4 30bpp, big-endian
	AV_PIX_FMT_GBRP10LE,  ///< planar GBR 4:4:4 30bpp, little-endian
	AV_PIX_FMT_GBRP16BE,  ///< planar GBR 4:4:4 48bpp, big-endian
	AV_PIX_FMT_GBRP16LE,  ///< planar GBR 4:4:4 48bpp, little-endian
	AV_PIX_FMT_YUVA422P,  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
	AV_PIX_FMT_YUVA444P,  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
	AV_PIX_FMT_YUVA420P9BE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
	AV_PIX_FMT_YUVA420P9LE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
	AV_PIX_FMT_YUVA422P9BE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
	AV_PIX_FMT_YUVA422P9LE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
	AV_PIX_FMT_YUVA444P9BE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
	AV_PIX_FMT_YUVA444P9LE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
	AV_PIX_FMT_YUVA420P10BE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
	AV_PIX_FMT_YUVA420P10LE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
	AV_PIX_FMT_YUVA422P10BE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
	AV_PIX_FMT_YUVA422P10LE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
	AV_PIX_FMT_YUVA444P10BE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
	AV_PIX_FMT_YUVA444P10LE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
	AV_PIX_FMT_YUVA420P16BE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
	AV_PIX_FMT_YUVA420P16LE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
	AV_PIX_FMT_YUVA422P16BE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
	AV_PIX_FMT_YUVA422P16LE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
	AV_PIX_FMT_YUVA444P16BE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
	AV_PIX_FMT_YUVA444P16LE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)

	AV_PIX_FMT_VDPAU,     ///< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface

	AV_PIX_FMT_XYZ12LE,      ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0
	AV_PIX_FMT_XYZ12BE,      ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0
	AV_PIX_FMT_NV16,         ///< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
	AV_PIX_FMT_NV20LE,       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	AV_PIX_FMT_NV20BE,       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian

	AV_PIX_FMT_RGBA64BE,     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
	AV_PIX_FMT_RGBA64LE,     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
	AV_PIX_FMT_BGRA64BE,     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
	AV_PIX_FMT_BGRA64LE,     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian

	AV_PIX_FMT_YVYU422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb

	AV_PIX_FMT_YA16BE,       ///< 16 bits gray, 16 bits alpha (big-endian)
	AV_PIX_FMT_YA16LE,       ///< 16 bits gray, 16 bits alpha (little-endian)

	AV_PIX_FMT_GBRAP,        ///< planar GBRA 4:4:4:4 32bpp
	AV_PIX_FMT_GBRAP16BE,    ///< planar GBRA 4:4:4:4 64bpp, big-endian
	AV_PIX_FMT_GBRAP16LE,    ///< planar GBRA 4:4:4:4 64bpp, little-endian
	/**
	*  HW acceleration through QSV, data[3] contains a pointer to the
	*  mfxFrameSurface1 structure.
	*/
	AV_PIX_FMT_QSV,
	/**
	* HW acceleration though MMAL, data[3] contains a pointer to the
	* MMAL_BUFFER_HEADER_T structure.
	*/
	AV_PIX_FMT_MMAL,

	AV_PIX_FMT_D3D11VA_VLD,  ///< HW decoding through Direct3D11 via old API, Picture.data[3] contains a ID3D11VideoDecoderOutputView pointer

	/**
	* HW acceleration through CUDA. data[i] contain CUdeviceptr pointers
	* exactly as for system memory frames.
	*/
	AV_PIX_FMT_CUDA,

	AV_PIX_FMT_0RGB,        ///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
	AV_PIX_FMT_RGB0,        ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
	AV_PIX_FMT_0BGR,        ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
	AV_PIX_FMT_BGR0,        ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined

	AV_PIX_FMT_YUV420P12BE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	AV_PIX_FMT_YUV420P12LE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	AV_PIX_FMT_YUV420P14BE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	AV_PIX_FMT_YUV420P14LE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	AV_PIX_FMT_YUV422P12BE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	AV_PIX_FMT_YUV422P12LE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	AV_PIX_FMT_YUV422P14BE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	AV_PIX_FMT_YUV422P14LE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	AV_PIX_FMT_YUV444P12BE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	AV_PIX_FMT_YUV444P12LE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	AV_PIX_FMT_YUV444P14BE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	AV_PIX_FMT_YUV444P14LE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	AV_PIX_FMT_GBRP12BE,    ///< planar GBR 4:4:4 36bpp, big-endian
	AV_PIX_FMT_GBRP12LE,    ///< planar GBR 4:4:4 36bpp, little-endian
	AV_PIX_FMT_GBRP14BE,    ///< planar GBR 4:4:4 42bpp, big-endian
	AV_PIX_FMT_GBRP14LE,    ///< planar GBR 4:4:4 42bpp, little-endian
	AV_PIX_FMT_YUVJ411P,    ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV411P and setting color_range

	AV_PIX_FMT_BAYER_BGGR8,    ///< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
	AV_PIX_FMT_BAYER_RGGB8,    ///< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
	AV_PIX_FMT_BAYER_GBRG8,    ///< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
	AV_PIX_FMT_BAYER_GRBG8,    ///< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */
	AV_PIX_FMT_BAYER_BGGR16LE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian */
	AV_PIX_FMT_BAYER_BGGR16BE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian */
	AV_PIX_FMT_BAYER_RGGB16LE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian */
	AV_PIX_FMT_BAYER_RGGB16BE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian */
	AV_PIX_FMT_BAYER_GBRG16LE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian */
	AV_PIX_FMT_BAYER_GBRG16BE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian */
	AV_PIX_FMT_BAYER_GRBG16LE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian */
	AV_PIX_FMT_BAYER_GRBG16BE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian */

	AV_PIX_FMT_XVMC,///< XVideo Motion Acceleration via common packet passing

	AV_PIX_FMT_YUV440P10LE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
	AV_PIX_FMT_YUV440P10BE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
	AV_PIX_FMT_YUV440P12LE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
	AV_PIX_FMT_YUV440P12BE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
	AV_PIX_FMT_AYUV64LE,    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
	AV_PIX_FMT_AYUV64BE,    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian

	AV_PIX_FMT_VIDEOTOOLBOX, ///< hardware decoding through Videotoolbox

	AV_PIX_FMT_P010LE, ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
	AV_PIX_FMT_P010BE, ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian

	AV_PIX_FMT_GBRAP12BE,  ///< planar GBR 4:4:4:4 48bpp, big-endian
	AV_PIX_FMT_GBRAP12LE,  ///< planar GBR 4:4:4:4 48bpp, little-endian

	AV_PIX_FMT_GBRAP10BE,  ///< planar GBR 4:4:4:4 40bpp, big-endian
	AV_PIX_FMT_GBRAP10LE,  ///< planar GBR 4:4:4:4 40bpp, little-endian

	AV_PIX_FMT_MEDIACODEC, ///< hardware decoding through MediaCodec

	AV_PIX_FMT_GRAY12BE,   ///<        Y        , 12bpp, big-endian
	AV_PIX_FMT_GRAY12LE,   ///<        Y        , 12bpp, little-endian
	AV_PIX_FMT_GRAY10BE,   ///<        Y        , 10bpp, big-endian
	AV_PIX_FMT_GRAY10LE,   ///<        Y        , 10bpp, little-endian

	AV_PIX_FMT_P016LE, ///< like NV12, with 16bpp per component, little-endian
	AV_PIX_FMT_P016BE, ///< like NV12, with 16bpp per component, big-endian

	/**
	* Hardware surfaces for Direct3D11.
	*
	* This is preferred over the legacy AV_PIX_FMT_D3D11VA_VLD. The new D3D11
	* hwaccel API and filtering support AV_PIX_FMT_D3D11 only.
	*
	* data[0] contains a ID3D11Texture2D pointer, and data[1] contains the
	* texture array index of the frame as intptr_t if the ID3D11Texture2D is
	* an array texture (or always 0 if it's a normal texture).
	*/
	AV_PIX_FMT_D3D11,

	AV_PIX_FMT_GRAY9BE,   ///<        Y        , 9bpp, big-endian
	AV_PIX_FMT_GRAY9LE,   ///<        Y        , 9bpp, little-endian

	AV_PIX_FMT_GBRPF32BE,  ///< IEEE-754 single precision planar GBR 4:4:4,     96bpp, big-endian
	AV_PIX_FMT_GBRPF32LE,  ///< IEEE-754 single precision planar GBR 4:4:4,     96bpp, little-endian
	AV_PIX_FMT_GBRAPF32BE, ///< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, big-endian
	AV_PIX_FMT_GBRAPF32LE, ///< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, little-endian

	/**
	* DRM-managed buffers exposed through PRIME buffer sharing.
	*
	* data[0] points to an AVDRMFrameDescriptor.
	*/
	AV_PIX_FMT_DRM_PRIME,
	/**
	* Hardware surfaces for OpenCL.
	*
	* data[i] contain 2D image objects (typed in C as cl_mem, used
	* in OpenCL as image2d_t) for each plane of the surface.
	*/
	AV_PIX_FMT_OPENCL,

	AV_PIX_FMT_GRAY14BE,   ///<        Y        , 14bpp, big-endian
	AV_PIX_FMT_GRAY14LE,   ///<        Y        , 14bpp, little-endian

	AV_PIX_FMT_GRAYF32BE,  ///< IEEE-754 single precision Y, 32bpp, big-endian
	AV_PIX_FMT_GRAYF32LE,  ///< IEEE-754 single precision Y, 32bpp, little-endian

	AV_PIX_FMT_NB         ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};

#if AV_HAVE_BIGENDIAN
#   define AV_PIX_FMT_NE(be, le) AV_PIX_FMT_##be
#else
#   define AV_PIX_FMT_NE(be, le) AV_PIX_FMT_##le
#endif

#define AV_PIX_FMT_RGB32   AV_PIX_FMT_NE(ARGB, BGRA)
#define AV_PIX_FMT_RGB32_1 AV_PIX_FMT_NE(RGBA, ABGR)
#define AV_PIX_FMT_BGR32   AV_PIX_FMT_NE(ABGR, RGBA)
#define AV_PIX_FMT_BGR32_1 AV_PIX_FMT_NE(BGRA, ARGB)
#define AV_PIX_FMT_0RGB32  AV_PIX_FMT_NE(0RGB, BGR0)
#define AV_PIX_FMT_0BGR32  AV_PIX_FMT_NE(0BGR, RGB0)

#define AV_PIX_FMT_GRAY9  AV_PIX_FMT_NE(GRAY9BE,  GRAY9LE)
#define AV_PIX_FMT_GRAY10 AV_PIX_FMT_NE(GRAY10BE, GRAY10LE)
#define AV_PIX_FMT_GRAY12 AV_PIX_FMT_NE(GRAY12BE, GRAY12LE)
#define AV_PIX_FMT_GRAY14 AV_PIX_FMT_NE(GRAY14BE, GRAY14LE)
#define AV_PIX_FMT_GRAY16 AV_PIX_FMT_NE(GRAY16BE, GRAY16LE)
#define AV_PIX_FMT_YA16   AV_PIX_FMT_NE(YA16BE,   YA16LE)
#define AV_PIX_FMT_RGB48  AV_PIX_FMT_NE(RGB48BE,  RGB48LE)
#define AV_PIX_FMT_RGB565 AV_PIX_FMT_NE(RGB565BE, RGB565LE)
#define AV_PIX_FMT_RGB555 AV_PIX_FMT_NE(RGB555BE, RGB555LE)
#define AV_PIX_FMT_RGB444 AV_PIX_FMT_NE(RGB444BE, RGB444LE)
#define AV_PIX_FMT_RGBA64 AV_PIX_FMT_NE(RGBA64BE, RGBA64LE)
#define AV_PIX_FMT_BGR48  AV_PIX_FMT_NE(BGR48BE,  BGR48LE)
#define AV_PIX_FMT_BGR565 AV_PIX_FMT_NE(BGR565BE, BGR565LE)
#define AV_PIX_FMT_BGR555 AV_PIX_FMT_NE(BGR555BE, BGR555LE)
#define AV_PIX_FMT_BGR444 AV_PIX_FMT_NE(BGR444BE, BGR444LE)
#define AV_PIX_FMT_BGRA64 AV_PIX_FMT_NE(BGRA64BE, BGRA64LE)

typedef struct PixelFormatTag {
	enum AVPixelFormat pix_fmt;
	unsigned int fourcc;
} PixelFormatTag;

enum AVCodecID {
	AV_CODEC_ID_NONE,

	/* video codecs */
	AV_CODEC_ID_MPEG1VIDEO,
	AV_CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
	AV_CODEC_ID_H261,
	AV_CODEC_ID_H263,
	AV_CODEC_ID_RV10,
	AV_CODEC_ID_RV20,
	AV_CODEC_ID_MJPEG,
	AV_CODEC_ID_MJPEGB,
	AV_CODEC_ID_LJPEG,
	AV_CODEC_ID_SP5X,
	AV_CODEC_ID_JPEGLS,
	AV_CODEC_ID_MPEG4,
	AV_CODEC_ID_RAWVIDEO,
	AV_CODEC_ID_MSMPEG4V1,
	AV_CODEC_ID_MSMPEG4V2,
	AV_CODEC_ID_MSMPEG4V3,
	AV_CODEC_ID_WMV1,
	AV_CODEC_ID_WMV2,
	AV_CODEC_ID_H263P,
	AV_CODEC_ID_H263I,
	AV_CODEC_ID_FLV1,
	AV_CODEC_ID_SVQ1,
	AV_CODEC_ID_SVQ3,
	AV_CODEC_ID_DVVIDEO,
	AV_CODEC_ID_HUFFYUV,
	AV_CODEC_ID_CYUV,
	AV_CODEC_ID_H264,
	AV_CODEC_ID_INDEO3,
	AV_CODEC_ID_VP3,
	AV_CODEC_ID_THEORA,
	AV_CODEC_ID_ASV1,
	AV_CODEC_ID_ASV2,
	AV_CODEC_ID_FFV1,
	AV_CODEC_ID_4XM,
	AV_CODEC_ID_VCR1,
	AV_CODEC_ID_CLJR,
	AV_CODEC_ID_MDEC,
	AV_CODEC_ID_ROQ,
	AV_CODEC_ID_INTERPLAY_VIDEO,
	AV_CODEC_ID_XAN_WC3,
	AV_CODEC_ID_XAN_WC4,
	AV_CODEC_ID_RPZA,
	AV_CODEC_ID_CINEPAK,
	AV_CODEC_ID_WS_VQA,
	AV_CODEC_ID_MSRLE,
	AV_CODEC_ID_MSVIDEO1,
	AV_CODEC_ID_IDCIN,
	AV_CODEC_ID_8BPS,
	AV_CODEC_ID_SMC,
	AV_CODEC_ID_FLIC,
	AV_CODEC_ID_TRUEMOTION1,
	AV_CODEC_ID_VMDVIDEO,
	AV_CODEC_ID_MSZH,
	AV_CODEC_ID_ZLIB,
	AV_CODEC_ID_QTRLE,
	AV_CODEC_ID_TSCC,
	AV_CODEC_ID_ULTI,
	AV_CODEC_ID_QDRAW,
	AV_CODEC_ID_VIXL,
	AV_CODEC_ID_QPEG,
	AV_CODEC_ID_PNG,
	AV_CODEC_ID_PPM,
	AV_CODEC_ID_PBM,
	AV_CODEC_ID_PGM,
	AV_CODEC_ID_PGMYUV,
	AV_CODEC_ID_PAM,
	AV_CODEC_ID_FFVHUFF,
	AV_CODEC_ID_RV30,
	AV_CODEC_ID_RV40,
	AV_CODEC_ID_VC1,
	AV_CODEC_ID_WMV3,
	AV_CODEC_ID_LOCO,
	AV_CODEC_ID_WNV1,
	AV_CODEC_ID_AASC,
	AV_CODEC_ID_INDEO2,
	AV_CODEC_ID_FRAPS,
	AV_CODEC_ID_TRUEMOTION2,
	AV_CODEC_ID_BMP,
	AV_CODEC_ID_CSCD,
	AV_CODEC_ID_MMVIDEO,
	AV_CODEC_ID_ZMBV,
	AV_CODEC_ID_AVS,
	AV_CODEC_ID_SMACKVIDEO,
	AV_CODEC_ID_NUV,
	AV_CODEC_ID_KMVC,
	AV_CODEC_ID_FLASHSV,
	AV_CODEC_ID_CAVS,
	AV_CODEC_ID_JPEG2000,
	AV_CODEC_ID_VMNC,
	AV_CODEC_ID_VP5,
	AV_CODEC_ID_VP6,
	AV_CODEC_ID_VP6F,
	AV_CODEC_ID_TARGA,
	AV_CODEC_ID_DSICINVIDEO,
	AV_CODEC_ID_TIERTEXSEQVIDEO,
	AV_CODEC_ID_TIFF,
	AV_CODEC_ID_GIF,
	AV_CODEC_ID_DXA,
	AV_CODEC_ID_DNXHD,
	AV_CODEC_ID_THP,
	AV_CODEC_ID_SGI,
	AV_CODEC_ID_C93,
	AV_CODEC_ID_BETHSOFTVID,
	AV_CODEC_ID_PTX,
	AV_CODEC_ID_TXD,
	AV_CODEC_ID_VP6A,
	AV_CODEC_ID_AMV,
	AV_CODEC_ID_VB,
	AV_CODEC_ID_PCX,
	AV_CODEC_ID_SUNRAST,
	AV_CODEC_ID_INDEO4,
	AV_CODEC_ID_INDEO5,
	AV_CODEC_ID_MIMIC,
	AV_CODEC_ID_RL2,
	AV_CODEC_ID_ESCAPE124,
	AV_CODEC_ID_DIRAC,
	AV_CODEC_ID_BFI,
	AV_CODEC_ID_CMV,
	AV_CODEC_ID_MOTIONPIXELS,
	AV_CODEC_ID_TGV,
	AV_CODEC_ID_TGQ,
	AV_CODEC_ID_TQI,
	AV_CODEC_ID_AURA,
	AV_CODEC_ID_AURA2,
	AV_CODEC_ID_V210X,
	AV_CODEC_ID_TMV,
	AV_CODEC_ID_V210,
	AV_CODEC_ID_DPX,
	AV_CODEC_ID_MAD,
	AV_CODEC_ID_FRWU,
	AV_CODEC_ID_FLASHSV2,
	AV_CODEC_ID_CDGRAPHICS,
	AV_CODEC_ID_R210,
	AV_CODEC_ID_ANM,
	AV_CODEC_ID_BINKVIDEO,
	AV_CODEC_ID_IFF_ILBM,
#define AV_CODEC_ID_IFF_BYTERUN1 AV_CODEC_ID_IFF_ILBM
	AV_CODEC_ID_KGV1,
	AV_CODEC_ID_YOP,
	AV_CODEC_ID_VP8,
	AV_CODEC_ID_PICTOR,
	AV_CODEC_ID_ANSI,
	AV_CODEC_ID_A64_MULTI,
	AV_CODEC_ID_A64_MULTI5,
	AV_CODEC_ID_R10K,
	AV_CODEC_ID_MXPEG,
	AV_CODEC_ID_LAGARITH,
	AV_CODEC_ID_PRORES,
	AV_CODEC_ID_JV,
	AV_CODEC_ID_DFA,
	AV_CODEC_ID_WMV3IMAGE,
	AV_CODEC_ID_VC1IMAGE,
	AV_CODEC_ID_UTVIDEO,
	AV_CODEC_ID_BMV_VIDEO,
	AV_CODEC_ID_VBLE,
	AV_CODEC_ID_DXTORY,
	AV_CODEC_ID_V410,
	AV_CODEC_ID_XWD,
	AV_CODEC_ID_CDXL,
	AV_CODEC_ID_XBM,
	AV_CODEC_ID_ZEROCODEC,
	AV_CODEC_ID_MSS1,
	AV_CODEC_ID_MSA1,
	AV_CODEC_ID_TSCC2,
	AV_CODEC_ID_MTS2,
	AV_CODEC_ID_CLLC,
	AV_CODEC_ID_MSS2,
	AV_CODEC_ID_VP9,
	AV_CODEC_ID_AIC,
	AV_CODEC_ID_ESCAPE130,
	AV_CODEC_ID_G2M,
	AV_CODEC_ID_WEBP,
	AV_CODEC_ID_HNM4_VIDEO,
	AV_CODEC_ID_HEVC,
#define AV_CODEC_ID_H265 AV_CODEC_ID_HEVC
	AV_CODEC_ID_FIC,
	AV_CODEC_ID_ALIAS_PIX,
	AV_CODEC_ID_BRENDER_PIX,
	AV_CODEC_ID_PAF_VIDEO,
	AV_CODEC_ID_EXR,
	AV_CODEC_ID_VP7,
	AV_CODEC_ID_SANM,
	AV_CODEC_ID_SGIRLE,
	AV_CODEC_ID_MVC1,
	AV_CODEC_ID_MVC2,
	AV_CODEC_ID_HQX,
	AV_CODEC_ID_TDSC,
	AV_CODEC_ID_HQ_HQA,
	AV_CODEC_ID_HAP,
	AV_CODEC_ID_DDS,
	AV_CODEC_ID_DXV,
	AV_CODEC_ID_SCREENPRESSO,
	AV_CODEC_ID_RSCC,
	AV_CODEC_ID_AVS2,

	AV_CODEC_ID_Y41P = 0x8000,
	AV_CODEC_ID_AVRP,
	AV_CODEC_ID_012V,
	AV_CODEC_ID_AVUI,
	AV_CODEC_ID_AYUV,
	AV_CODEC_ID_TARGA_Y216,
	AV_CODEC_ID_V308,
	AV_CODEC_ID_V408,
	AV_CODEC_ID_YUV4,
	AV_CODEC_ID_AVRN,
	AV_CODEC_ID_CPIA,
	AV_CODEC_ID_XFACE,
	AV_CODEC_ID_SNOW,
	AV_CODEC_ID_SMVJPEG,
	AV_CODEC_ID_APNG,
	AV_CODEC_ID_DAALA,
	AV_CODEC_ID_CFHD,
	AV_CODEC_ID_TRUEMOTION2RT,
	AV_CODEC_ID_M101,
	AV_CODEC_ID_MAGICYUV,
	AV_CODEC_ID_SHEERVIDEO,
	AV_CODEC_ID_YLC,
	AV_CODEC_ID_PSD,
	AV_CODEC_ID_PIXLET,
	AV_CODEC_ID_SPEEDHQ,
	AV_CODEC_ID_FMVC,
	AV_CODEC_ID_SCPR,
	AV_CODEC_ID_CLEARVIDEO,
	AV_CODEC_ID_XPM,
	AV_CODEC_ID_AV1,
	AV_CODEC_ID_BITPACKED,
	AV_CODEC_ID_MSCC,
	AV_CODEC_ID_SRGC,
	AV_CODEC_ID_SVG,
	AV_CODEC_ID_GDV,
	AV_CODEC_ID_FITS,
	AV_CODEC_ID_IMM4,
	AV_CODEC_ID_PROSUMER,
	AV_CODEC_ID_MWSC,
	AV_CODEC_ID_WCMV,
	AV_CODEC_ID_RASC,

	/* various PCM "codecs" */
	AV_CODEC_ID_FIRST_AUDIO = 0x10000,     ///< A dummy id pointing at the start of audio codecs
	AV_CODEC_ID_PCM_S16LE = 0x10000,
	AV_CODEC_ID_PCM_S16BE,
	AV_CODEC_ID_PCM_U16LE,
	AV_CODEC_ID_PCM_U16BE,
	AV_CODEC_ID_PCM_S8,
	AV_CODEC_ID_PCM_U8,
	AV_CODEC_ID_PCM_MULAW,
	AV_CODEC_ID_PCM_ALAW,
	AV_CODEC_ID_PCM_S32LE,
	AV_CODEC_ID_PCM_S32BE,
	AV_CODEC_ID_PCM_U32LE,
	AV_CODEC_ID_PCM_U32BE,
	AV_CODEC_ID_PCM_S24LE,
	AV_CODEC_ID_PCM_S24BE,
	AV_CODEC_ID_PCM_U24LE,
	AV_CODEC_ID_PCM_U24BE,
	AV_CODEC_ID_PCM_S24DAUD,
	AV_CODEC_ID_PCM_ZORK,
	AV_CODEC_ID_PCM_S16LE_PLANAR,
	AV_CODEC_ID_PCM_DVD,
	AV_CODEC_ID_PCM_F32BE,
	AV_CODEC_ID_PCM_F32LE,
	AV_CODEC_ID_PCM_F64BE,
	AV_CODEC_ID_PCM_F64LE,
	AV_CODEC_ID_PCM_BLURAY,
	AV_CODEC_ID_PCM_LXF,
	AV_CODEC_ID_S302M,
	AV_CODEC_ID_PCM_S8_PLANAR,
	AV_CODEC_ID_PCM_S24LE_PLANAR,
	AV_CODEC_ID_PCM_S32LE_PLANAR,
	AV_CODEC_ID_PCM_S16BE_PLANAR,

	AV_CODEC_ID_PCM_S64LE = 0x10800,
	AV_CODEC_ID_PCM_S64BE,
	AV_CODEC_ID_PCM_F16LE,
	AV_CODEC_ID_PCM_F24LE,
	AV_CODEC_ID_PCM_VIDC,

	/* various ADPCM codecs */
	AV_CODEC_ID_ADPCM_IMA_QT = 0x11000,
	AV_CODEC_ID_ADPCM_IMA_WAV,
	AV_CODEC_ID_ADPCM_IMA_DK3,
	AV_CODEC_ID_ADPCM_IMA_DK4,
	AV_CODEC_ID_ADPCM_IMA_WS,
	AV_CODEC_ID_ADPCM_IMA_SMJPEG,
	AV_CODEC_ID_ADPCM_MS,
	AV_CODEC_ID_ADPCM_4XM,
	AV_CODEC_ID_ADPCM_XA,
	AV_CODEC_ID_ADPCM_ADX,
	AV_CODEC_ID_ADPCM_EA,
	AV_CODEC_ID_ADPCM_G726,
	AV_CODEC_ID_ADPCM_CT,
	AV_CODEC_ID_ADPCM_SWF,
	AV_CODEC_ID_ADPCM_YAMAHA,
	AV_CODEC_ID_ADPCM_SBPRO_4,
	AV_CODEC_ID_ADPCM_SBPRO_3,
	AV_CODEC_ID_ADPCM_SBPRO_2,
	AV_CODEC_ID_ADPCM_THP,
	AV_CODEC_ID_ADPCM_IMA_AMV,
	AV_CODEC_ID_ADPCM_EA_R1,
	AV_CODEC_ID_ADPCM_EA_R3,
	AV_CODEC_ID_ADPCM_EA_R2,
	AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
	AV_CODEC_ID_ADPCM_IMA_EA_EACS,
	AV_CODEC_ID_ADPCM_EA_XAS,
	AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
	AV_CODEC_ID_ADPCM_IMA_ISS,
	AV_CODEC_ID_ADPCM_G722,
	AV_CODEC_ID_ADPCM_IMA_APC,
	AV_CODEC_ID_ADPCM_VIMA,

	AV_CODEC_ID_ADPCM_AFC = 0x11800,
	AV_CODEC_ID_ADPCM_IMA_OKI,
	AV_CODEC_ID_ADPCM_DTK,
	AV_CODEC_ID_ADPCM_IMA_RAD,
	AV_CODEC_ID_ADPCM_G726LE,
	AV_CODEC_ID_ADPCM_THP_LE,
	AV_CODEC_ID_ADPCM_PSX,
	AV_CODEC_ID_ADPCM_AICA,
	AV_CODEC_ID_ADPCM_IMA_DAT4,
	AV_CODEC_ID_ADPCM_MTAF,

	/* AMR */
	AV_CODEC_ID_AMR_NB = 0x12000,
	AV_CODEC_ID_AMR_WB,

	/* RealAudio codecs*/
	AV_CODEC_ID_RA_144 = 0x13000,
	AV_CODEC_ID_RA_288,

	/* various DPCM codecs */
	AV_CODEC_ID_ROQ_DPCM = 0x14000,
	AV_CODEC_ID_INTERPLAY_DPCM,
	AV_CODEC_ID_XAN_DPCM,
	AV_CODEC_ID_SOL_DPCM,

	AV_CODEC_ID_SDX2_DPCM = 0x14800,
	AV_CODEC_ID_GREMLIN_DPCM,

	/* audio codecs */
	AV_CODEC_ID_MP2 = 0x15000,
	AV_CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
	AV_CODEC_ID_AAC,
	AV_CODEC_ID_AC3,
	AV_CODEC_ID_DTS,
	AV_CODEC_ID_VORBIS,
	AV_CODEC_ID_DVAUDIO,
	AV_CODEC_ID_WMAV1,
	AV_CODEC_ID_WMAV2,
	AV_CODEC_ID_MACE3,
	AV_CODEC_ID_MACE6,
	AV_CODEC_ID_VMDAUDIO,
	AV_CODEC_ID_FLAC,
	AV_CODEC_ID_MP3ADU,
	AV_CODEC_ID_MP3ON4,
	AV_CODEC_ID_SHORTEN,
	AV_CODEC_ID_ALAC,
	AV_CODEC_ID_WESTWOOD_SND1,
	AV_CODEC_ID_GSM, ///< as in Berlin toast format
	AV_CODEC_ID_QDM2,
	AV_CODEC_ID_COOK,
	AV_CODEC_ID_TRUESPEECH,
	AV_CODEC_ID_TTA,
	AV_CODEC_ID_SMACKAUDIO,
	AV_CODEC_ID_QCELP,
	AV_CODEC_ID_WAVPACK,
	AV_CODEC_ID_DSICINAUDIO,
	AV_CODEC_ID_IMC,
	AV_CODEC_ID_MUSEPACK7,
	AV_CODEC_ID_MLP,
	AV_CODEC_ID_GSM_MS, /* as found in WAV */
	AV_CODEC_ID_ATRAC3,
	AV_CODEC_ID_APE,
	AV_CODEC_ID_NELLYMOSER,
	AV_CODEC_ID_MUSEPACK8,
	AV_CODEC_ID_SPEEX,
	AV_CODEC_ID_WMAVOICE,
	AV_CODEC_ID_WMAPRO,
	AV_CODEC_ID_WMALOSSLESS,
	AV_CODEC_ID_ATRAC3P,
	AV_CODEC_ID_EAC3,
	AV_CODEC_ID_SIPR,
	AV_CODEC_ID_MP1,
	AV_CODEC_ID_TWINVQ,
	AV_CODEC_ID_TRUEHD,
	AV_CODEC_ID_MP4ALS,
	AV_CODEC_ID_ATRAC1,
	AV_CODEC_ID_BINKAUDIO_RDFT,
	AV_CODEC_ID_BINKAUDIO_DCT,
	AV_CODEC_ID_AAC_LATM,
	AV_CODEC_ID_QDMC,
	AV_CODEC_ID_CELT,
	AV_CODEC_ID_G723_1,
	AV_CODEC_ID_G729,
	AV_CODEC_ID_8SVX_EXP,
	AV_CODEC_ID_8SVX_FIB,
	AV_CODEC_ID_BMV_AUDIO,
	AV_CODEC_ID_RALF,
	AV_CODEC_ID_IAC,
	AV_CODEC_ID_ILBC,
	AV_CODEC_ID_OPUS,
	AV_CODEC_ID_COMFORT_NOISE,
	AV_CODEC_ID_TAK,
	AV_CODEC_ID_METASOUND,
	AV_CODEC_ID_PAF_AUDIO,
	AV_CODEC_ID_ON2AVC,
	AV_CODEC_ID_DSS_SP,
	AV_CODEC_ID_CODEC2,

	AV_CODEC_ID_FFWAVESYNTH = 0x15800,
	AV_CODEC_ID_SONIC,
	AV_CODEC_ID_SONIC_LS,
	AV_CODEC_ID_EVRC,
	AV_CODEC_ID_SMV,
	AV_CODEC_ID_DSD_LSBF,
	AV_CODEC_ID_DSD_MSBF,
	AV_CODEC_ID_DSD_LSBF_PLANAR,
	AV_CODEC_ID_DSD_MSBF_PLANAR,
	AV_CODEC_ID_4GV,
	AV_CODEC_ID_INTERPLAY_ACM,
	AV_CODEC_ID_XMA1,
	AV_CODEC_ID_XMA2,
	AV_CODEC_ID_DST,
	AV_CODEC_ID_ATRAC3AL,
	AV_CODEC_ID_ATRAC3PAL,
	AV_CODEC_ID_DOLBY_E,
	AV_CODEC_ID_APTX,
	AV_CODEC_ID_APTX_HD,
	AV_CODEC_ID_SBC,
	AV_CODEC_ID_ATRAC9,

	/* subtitle codecs */
	AV_CODEC_ID_FIRST_SUBTITLE = 0x17000,          ///< A dummy ID pointing at the start of subtitle codecs.
	AV_CODEC_ID_DVD_SUBTITLE = 0x17000,
	AV_CODEC_ID_DVB_SUBTITLE,
	AV_CODEC_ID_TEXT,  ///< raw UTF-8 text
	AV_CODEC_ID_XSUB,
	AV_CODEC_ID_SSA,
	AV_CODEC_ID_MOV_TEXT,
	AV_CODEC_ID_HDMV_PGS_SUBTITLE,
	AV_CODEC_ID_DVB_TELETEXT,
	AV_CODEC_ID_SRT,

	AV_CODEC_ID_MICRODVD = 0x17800,
	AV_CODEC_ID_EIA_608,
	AV_CODEC_ID_JACOSUB,
	AV_CODEC_ID_SAMI,
	AV_CODEC_ID_REALTEXT,
	AV_CODEC_ID_STL,
	AV_CODEC_ID_SUBVIEWER1,
	AV_CODEC_ID_SUBVIEWER,
	AV_CODEC_ID_SUBRIP,
	AV_CODEC_ID_WEBVTT,
	AV_CODEC_ID_MPL2,
	AV_CODEC_ID_VPLAYER,
	AV_CODEC_ID_PJS,
	AV_CODEC_ID_ASS,
	AV_CODEC_ID_HDMV_TEXT_SUBTITLE,
	AV_CODEC_ID_TTML,

	/* other specific kind of codecs (generally used for attachments) */
	AV_CODEC_ID_FIRST_UNKNOWN = 0x18000,           ///< A dummy ID pointing at the start of various fake codecs.
	AV_CODEC_ID_TTF = 0x18000,

	AV_CODEC_ID_SCTE_35, ///< Contain timestamp estimated through PCR of program stream.
	AV_CODEC_ID_BINTEXT = 0x18800,
	AV_CODEC_ID_XBIN,
	AV_CODEC_ID_IDF,
	AV_CODEC_ID_OTF,
	AV_CODEC_ID_SMPTE_KLV,
	AV_CODEC_ID_DVD_NAV,
	AV_CODEC_ID_TIMED_ID3,
	AV_CODEC_ID_BIN_DATA,

	AV_CODEC_ID_PROBE = 0x19000, ///< codec_id is not known (like AV_CODEC_ID_NONE) but lavf should attempt to identify it

	AV_CODEC_ID_MPEG2TS = 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
	* stream (only used by libavformat) */
	AV_CODEC_ID_MPEG4SYSTEMS = 0x20001, /**< _FAKE_ codec to indicate a MPEG-4 Systems
	* stream (only used by libavformat) */
	AV_CODEC_ID_FFMETADATA = 0x21000,   ///< Dummy codec for streams containing only metadata information.
	AV_CODEC_ID_WRAPPED_AVFRAME = 0x21001, ///< Passthrough codec, AVFrames wrapped in AVPacket
};

typedef struct AVCodecTag {
	enum AVCodecID id;
	unsigned int tag;
} AVCodecTag;

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

#define av_const

//STUFF YOU CAN CHANGE

//change for verbose debug info
static bool verbose = true;

//if you need VI to use multi threaded com
//#define VI_COM_MULTI_THREADED

//STUFF YOU DON'T CHANGE

//videoInput defines
#define VI_VERSION      0.1995
#define VI_MAX_CAMERAS  20
#define VI_NUM_TYPES    20 //MGB
#define VI_NUM_FORMATS  18 //DON'T TOUCH

//defines for setPhyCon - tuner is not as well supported as composite and s-video
#define VI_COMPOSITE 0
#define VI_S_VIDEO   1
#define VI_TUNER     2
#define VI_USB       3
#define VI_1394      4

//defines for formats
#define VI_NTSC_M   0
#define VI_PAL_B    1
#define VI_PAL_D    2
#define VI_PAL_G    3
#define VI_PAL_H    4
#define VI_PAL_I    5
#define VI_PAL_M    6
#define VI_PAL_N    7
#define VI_PAL_NC   8
#define VI_SECAM_B  9
#define VI_SECAM_D  10
#define VI_SECAM_G  11
#define VI_SECAM_H  12
#define VI_SECAM_K  13
#define VI_SECAM_K1 14
#define VI_SECAM_L  15
#define VI_NTSC_M_J 16
#define VI_NTSC_433 17

interface ISampleGrabberCB;
interface ISampleGrabber;
class SampleGrabberCallback;

class videoDevice {
public:
	videoDevice();
	void setSize(int w, int h);
	void NukeDownstream(IBaseFilter *pBF);
	void destroyGraph();
	~videoDevice();

	int videoSize;
	int width;
	int height;

	int tryWidth;
	int tryHeight;
	GUID tryVideoType;

	ICaptureGraphBuilder2 *pCaptureGraph;    // Capture graph builder object
	IGraphBuilder *pGraph;                    // Graph builder object
	IMediaControl *pControl;                // Media control object
	IBaseFilter *pVideoInputFilter;          // Video Capture filter
	IBaseFilter *pGrabberF;
	IBaseFilter * pDestFilter;
	IAMStreamConfig *streamConf;
	ISampleGrabber * pGrabber;                // Grabs frame
	AM_MEDIA_TYPE * pAmMediaType;

	IMediaEventEx * pMediaEvent;

	GUID videoType;
	long formatType;

	SampleGrabberCallback * sgCallback;

	bool tryDiffSize;
	bool useCrossbar;
	bool readyToCapture;
	bool sizeSet;
	bool setupStarted;
	bool specificFormat;
	bool autoReconnect;
	int  nFramesForReconnect;
	unsigned long nFramesRunning;
	int  connection;
	int  storeConn;
	int  myID;
	long requestedFrameTime; //ie fps

	char  nDeviceName[255];
	WCHAR wDeviceName[255];

	unsigned char * pixels;
	char * pBuffer;
};

class videoInput {
public:
	videoInput();
	~videoInput();

	//turns off console messages - default is to print messages
	static void setVerbose(bool _verbose);

	//Functions in rough order they should be used.
	static int listDevices(bool silent = false);

	//needs to be called after listDevices - otherwise returns NULL
	static char * getDeviceName(int deviceID);

	//choose to use callback based capture - or single threaded
	void setUseCallback(bool useCallback);

	//call before setupDevice
	//directshow will try and get the closest possible framerate to what is requested
	void setIdealFramerate(int deviceID, int idealFramerate);

	//some devices will stop delivering frames after a while - this method gives you the option to try and reconnect
	//to a device if videoInput detects that a device has stopped delivering frames.
	//you MUST CALL isFrameNew every app loop for this to have any effect
	void setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect);

	//Choose one of these five to setup your device
	bool setupDevice(int deviceID);
	bool setupDevice(int deviceID, int w, int h);
	bool setupDeviceFourcc(int deviceID, int w, int h, int fourcc);

	//These two are only for capture cards
	//USB and Firewire cameras souldn't specify connection
	bool setupDevice(int deviceID, int connection);
	bool setupDevice(int deviceID, int w, int h, int connection);

	bool setFourcc(int deviceNumber, int fourcc);

	//If you need to you can set your NTSC/PAL/SECAM
	//preference here. if it is available it will be used.
	//see #defines above for available formats - eg VI_NTSC_M or VI_PAL_B
	//should be called after setupDevice
	//can be called multiple times
	bool setFormat(int deviceNumber, int format);

	//Tells you when a new frame has arrived - you should call this if you have specified setAutoReconnectOnFreeze to true
	bool isFrameNew(int deviceID);

	bool isDeviceSetup(int deviceID);

	//Returns the pixels - flipRedAndBlue toggles RGB/BGR flipping - and you can flip the image too
	unsigned char * getPixels(int deviceID, bool flipRedAndBlue = true, bool flipImage = false);

	//Or pass in a buffer for getPixels to fill returns true if successful.
	bool getPixels(int id, unsigned char * pixels, bool flipRedAndBlue = true, bool flipImage = false);

	//Launches a pop up settings window
	//For some reason in GLUT you have to call it twice each time.
	void showSettingsWindow(int deviceID);

	//Manual control over settings thanks.....
	//These are experimental for now.
	bool setVideoSettingFilter(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
	bool setVideoSettingFilterPct(int deviceID, long Property, float pctValue, long Flags = 0);
	bool getVideoSettingFilter(int deviceID, long Property, long &min, long &max, long &SteppingDelta, long &currentValue, long &flags, long &defaultValue);

	bool setVideoSettingCamera(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
	bool setVideoSettingCameraPct(int deviceID, long Property, float pctValue, long Flags = 0);
	bool getVideoSettingCamera(int deviceID, long Property, long &min, long &max, long &SteppingDelta, long &currentValue, long &flags, long &defaultValue);

	//bool setVideoSettingCam(int deviceID, long Property, long lValue, long Flags = NULL, bool useDefaultValue = false);

	//get width, height and number of pixels
	int  getWidth(int deviceID);
	int  getHeight(int deviceID);
	int  getSize(int deviceID);
	int  getFourcc(int deviceID);
	double getFPS(int deviceID);

	//completely stops and frees a device
	void stopDevice(int deviceID);

	//as above but then sets it up with same settings
	bool restartDevice(int deviceID);

	//number of devices available
	int  devicesFound;

	// mapping from OpenCV CV_CAP_PROP to videoinput/dshow properties
	int getVideoPropertyFromCV(int cv_property);
	int getCameraPropertyFromCV(int cv_property);

	int getDeviceCount();
	bool getCodecList(int device_id, std::vector<int>& codecids);
	bool getVideoSizeList(int device_id, int codec_id, std::vector<std::string>& sizelist);

private:
	void setPhyCon(int deviceID, int conn);
	void setAttemptCaptureSize(int deviceID, int w, int h, GUID mediaType = MEDIASUBTYPE_RGB24);
	bool setup(int deviceID);
	void processPixels(unsigned char * src, unsigned char * dst, int width, int height, bool bRGB, bool bFlip);
	int  start(int deviceID, videoDevice * VD);
	//int  getDeviceCount();
	void getMediaSubtypeAsString(GUID type, char * typeAsString);
	GUID *getMediaSubtypeFromFourcc(int fourcc);
	int    getFourccFromMediaSubtype(GUID type);

	void getVideoPropertyAsString(int prop, char * propertyAsString);
	void getCameraPropertyAsString(int prop, char * propertyAsString);

	HRESULT getDevice(IBaseFilter **pSrcFilter, int deviceID, WCHAR * wDeviceName, char * nDeviceName);
	static HRESULT ShowFilterPropertyPages(IBaseFilter *pFilter);
	static HRESULT ShowStreamPropertyPages(IAMStreamConfig  *pStream);

	HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);
	HRESULT routeCrossbar(ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode);

	//don't touch
	static bool comInit();
	static bool comUnInit();

	int  connection;
	int  callbackSetCount;
	bool bCallback;

	GUID CAPTURE_MODE;

	//Extra video subtypes
	GUID MEDIASUBTYPE_Y800;
	GUID MEDIASUBTYPE_Y8;
	GUID MEDIASUBTYPE_GREY;

	videoDevice * VDList[VI_MAX_CAMERAS];
	GUID mediaSubtypes[VI_NUM_TYPES];
	long formatTypes[VI_NUM_FORMATS];

	static void __cdecl basicThread(void * objPtr);

	static char deviceNames[VI_MAX_CAMERAS][255];

	void getCodecAndVideoSize(videoDevice *VD);
	std::map<int, std::map<int, std::vector<std::string>>> infolist; // device id, codec id, video size
};

class CvCaptureCAM_DShow : public CvCapture {
public:
	CvCaptureCAM_DShow();
	virtual ~CvCaptureCAM_DShow();

	virtual bool open(int index);
	virtual void close();
	virtual double getProperty(int);
	virtual bool setProperty(int, double);
	virtual bool grabFrame();
	virtual IplImage* retrieveFrame(int);
	virtual int getCaptureDomain() { return CV_CAP_DSHOW; } // Return the type of the capture object: CV_CAP_VFW, etc...

	virtual bool getDevicesList(std::map<int, std::string>& devicelist) const;
	virtual bool getCodecList(int device_id, std::vector<int>& codecids) const;
	virtual bool getVideoSizeList(int device_id, int codec_id, std::vector<std::string>& sizelist) const;

protected:
	void init();

	int index, width, height, fourcc;
	int widthSet, heightSet;
	IplImage* frame;
	static videoInput VI;
};


CvCapture* cvCreateCameraCapture_DShow(int index);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_DSHOW_HPP_
