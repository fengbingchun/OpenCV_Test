// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CAPTURE_HPP_
#define FBC_CV_CAPTURE_HPP_

// reference: 2.4.13.6
//            highgui/src/precomp.hpp
//            highgui/include/opencv2/highgui/highgui_c.h

#include <string>
#include <map>
#include <vector>

#include "iplimage.hpp"
#include "core/mat.hpp"

namespace fbc {

enum {
	CV_CAP_ANY = 0,     // autodetect

	CV_CAP_MIL = 100,   // MIL proprietary drivers

	CV_CAP_VFW = 200,   // platform native
	CV_CAP_V4L = 200,
	CV_CAP_V4L2 = 200,

	CV_CAP_FIREWARE = 300,   // IEEE 1394 drivers
	CV_CAP_FIREWIRE = 300,
	CV_CAP_IEEE1394 = 300,
	CV_CAP_DC1394 = 300,
	CV_CAP_CMU1394 = 300,

	CV_CAP_STEREO = 400,   // TYZX proprietary drivers
	CV_CAP_TYZX = 400,
	CV_TYZX_LEFT = 400,
	CV_TYZX_RIGHT = 401,
	CV_TYZX_COLOR = 402,
	CV_TYZX_Z = 403,

	CV_CAP_QT = 500,   // QuickTime

	CV_CAP_UNICAP = 600,   // Unicap drivers

	CV_CAP_DSHOW = 700,   // DirectShow (via videoInput)
	CV_CAP_MSMF = 1400,  // Microsoft Media Foundation (via videoInput)

	CV_CAP_PVAPI = 800,   // PvAPI, Prosilica GigE SDK

	CV_CAP_OPENNI = 900,   // OpenNI (for Kinect)
	CV_CAP_OPENNI_ASUS = 910,   // OpenNI (for Asus Xtion)

	CV_CAP_ANDROID = 1000,  // Android
	CV_CAP_ANDROID_BACK = CV_CAP_ANDROID + 99, // Android back camera
	CV_CAP_ANDROID_FRONT = CV_CAP_ANDROID + 98, // Android front camera

	CV_CAP_XIAPI = 1100,   // XIMEA Camera API

	CV_CAP_AVFOUNDATION = 1200,  // AVFoundation framework for iOS (OS X Lion will have the same API)

	CV_CAP_GIGANETIX = 1300,  // Smartek Giganetix GigEVisionSDK

	CV_CAP_INTELPERC = 1500 // Intel Perceptual Computing SDK
};

enum {
	// modes of the controlling registers (can be: auto, manual, auto single push, absolute Latter allowed with any other mode)
	// every feature can have only one mode turned on at a time
	CV_CAP_PROP_DC1394_OFF = -4,  //turn the feature off (not controlled manually nor automatically)
	CV_CAP_PROP_DC1394_MODE_MANUAL = -3, //set automatically when a value of the feature is set by the user
	CV_CAP_PROP_DC1394_MODE_AUTO = -2,
	CV_CAP_PROP_DC1394_MODE_ONE_PUSH_AUTO = -1,
	CV_CAP_PROP_POS_MSEC = 0,
	CV_CAP_PROP_POS_FRAMES = 1,
	CV_CAP_PROP_POS_AVI_RATIO = 2,
	CV_CAP_PROP_FRAME_WIDTH = 3,
	CV_CAP_PROP_FRAME_HEIGHT = 4,
	CV_CAP_PROP_FPS = 5,
	CV_CAP_PROP_FOURCC = 6,
	CV_CAP_PROP_FRAME_COUNT = 7,
	CV_CAP_PROP_FORMAT = 8,
	CV_CAP_PROP_MODE = 9,
	CV_CAP_PROP_BRIGHTNESS = 10,
	CV_CAP_PROP_CONTRAST = 11,
	CV_CAP_PROP_SATURATION = 12,
	CV_CAP_PROP_HUE = 13,
	CV_CAP_PROP_GAIN = 14,
	CV_CAP_PROP_EXPOSURE = 15,
	CV_CAP_PROP_CONVERT_RGB = 16,
	CV_CAP_PROP_WHITE_BALANCE_U = 17,
	CV_CAP_PROP_RECTIFICATION = 18,
	CV_CAP_PROP_MONOCROME = 19,
	CV_CAP_PROP_SHARPNESS = 20,
	CV_CAP_PROP_AUTO_EXPOSURE = 21, // exposure control done by camera,
	// user can adjust refernce level
	// using this feature
	CV_CAP_PROP_GAMMA = 22,
	CV_CAP_PROP_TEMPERATURE = 23,
	CV_CAP_PROP_TRIGGER = 24,
	CV_CAP_PROP_TRIGGER_DELAY = 25,
	CV_CAP_PROP_WHITE_BALANCE_V = 26,
	CV_CAP_PROP_ZOOM = 27,
	CV_CAP_PROP_FOCUS = 28,
	CV_CAP_PROP_GUID = 29,
	CV_CAP_PROP_ISO_SPEED = 30,
	CV_CAP_PROP_MAX_DC1394 = 31,
	CV_CAP_PROP_BACKLIGHT = 32,
	CV_CAP_PROP_PAN = 33,
	CV_CAP_PROP_TILT = 34,
	CV_CAP_PROP_ROLL = 35,
	CV_CAP_PROP_IRIS = 36,
	CV_CAP_PROP_SETTINGS = 37,
	CV_CAP_PROP_BUFFERSIZE = 38,

	CV_CAP_PROP_AUTOGRAB = 1024, // property for highgui class CvCapture_Android only
	CV_CAP_PROP_SUPPORTED_PREVIEW_SIZES_STRING = 1025, // readonly, tricky property, returns cpnst char* indeed
	CV_CAP_PROP_PREVIEW_FORMAT = 1026, // readonly, tricky property, returns cpnst char* indeed

	// OpenNI map generators
	CV_CAP_OPENNI_DEPTH_GENERATOR = 1 << 31,
	CV_CAP_OPENNI_IMAGE_GENERATOR = 1 << 30,
	CV_CAP_OPENNI_GENERATORS_MASK = CV_CAP_OPENNI_DEPTH_GENERATOR + CV_CAP_OPENNI_IMAGE_GENERATOR,

	// Properties of cameras available through OpenNI interfaces
	CV_CAP_PROP_OPENNI_OUTPUT_MODE = 100,
	CV_CAP_PROP_OPENNI_FRAME_MAX_DEPTH = 101, // in mm
	CV_CAP_PROP_OPENNI_BASELINE = 102, // in mm
	CV_CAP_PROP_OPENNI_FOCAL_LENGTH = 103, // in pixels
	CV_CAP_PROP_OPENNI_REGISTRATION = 104, // flag
	CV_CAP_PROP_OPENNI_REGISTRATION_ON = CV_CAP_PROP_OPENNI_REGISTRATION, // flag that synchronizes the remapping depth map to image map
	// by changing depth generator's view point (if the flag is "on") or
	// sets this view point to its normal one (if the flag is "off").
	CV_CAP_PROP_OPENNI_APPROX_FRAME_SYNC = 105,
	CV_CAP_PROP_OPENNI_MAX_BUFFER_SIZE = 106,
	CV_CAP_PROP_OPENNI_CIRCLE_BUFFER = 107,
	CV_CAP_PROP_OPENNI_MAX_TIME_DURATION = 108,

	CV_CAP_PROP_OPENNI_GENERATOR_PRESENT = 109,

	CV_CAP_OPENNI_IMAGE_GENERATOR_PRESENT = CV_CAP_OPENNI_IMAGE_GENERATOR + CV_CAP_PROP_OPENNI_GENERATOR_PRESENT,
	CV_CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE = CV_CAP_OPENNI_IMAGE_GENERATOR + CV_CAP_PROP_OPENNI_OUTPUT_MODE,
	CV_CAP_OPENNI_DEPTH_GENERATOR_BASELINE = CV_CAP_OPENNI_DEPTH_GENERATOR + CV_CAP_PROP_OPENNI_BASELINE,
	CV_CAP_OPENNI_DEPTH_GENERATOR_FOCAL_LENGTH = CV_CAP_OPENNI_DEPTH_GENERATOR + CV_CAP_PROP_OPENNI_FOCAL_LENGTH,
	CV_CAP_OPENNI_DEPTH_GENERATOR_REGISTRATION = CV_CAP_OPENNI_DEPTH_GENERATOR + CV_CAP_PROP_OPENNI_REGISTRATION,
	CV_CAP_OPENNI_DEPTH_GENERATOR_REGISTRATION_ON = CV_CAP_OPENNI_DEPTH_GENERATOR_REGISTRATION,

	// Properties of cameras available through GStreamer interface
	CV_CAP_GSTREAMER_QUEUE_LENGTH = 200, // default is 1

	// PVAPI
	CV_CAP_PROP_PVAPI_MULTICASTIP = 300, // ip for anable multicast master mode. 0 for disable multicast
	CV_CAP_PROP_PVAPI_FRAMESTARTTRIGGERMODE = 301, // FrameStartTriggerMode: Determines how a frame is initiated
	CV_CAP_PROP_PVAPI_DECIMATIONHORIZONTAL = 302, // Horizontal sub-sampling of the image
	CV_CAP_PROP_PVAPI_DECIMATIONVERTICAL = 303, // Vertical sub-sampling of the image
	CV_CAP_PROP_PVAPI_BINNINGX = 304, // Horizontal binning factor
	CV_CAP_PROP_PVAPI_BINNINGY = 305, // Vertical binning factor
	CV_CAP_PROP_PVAPI_PIXELFORMAT = 306, // Pixel format

	// Properties of cameras available through XIMEA SDK interface
	CV_CAP_PROP_XI_DOWNSAMPLING = 400,      // Change image resolution by binning or skipping.
	CV_CAP_PROP_XI_DATA_FORMAT = 401,       // Output data format.
	CV_CAP_PROP_XI_OFFSET_X = 402,      // Horizontal offset from the origin to the area of interest (in pixels).
	CV_CAP_PROP_XI_OFFSET_Y = 403,      // Vertical offset from the origin to the area of interest (in pixels).
	CV_CAP_PROP_XI_TRG_SOURCE = 404,      // Defines source of trigger.
	CV_CAP_PROP_XI_TRG_SOFTWARE = 405,      // Generates an internal trigger. PRM_TRG_SOURCE must be set to TRG_SOFTWARE.
	CV_CAP_PROP_XI_GPI_SELECTOR = 406,      // Selects general purpose input
	CV_CAP_PROP_XI_GPI_MODE = 407,      // Set general purpose input mode
	CV_CAP_PROP_XI_GPI_LEVEL = 408,      // Get general purpose level
	CV_CAP_PROP_XI_GPO_SELECTOR = 409,      // Selects general purpose output
	CV_CAP_PROP_XI_GPO_MODE = 410,      // Set general purpose output mode
	CV_CAP_PROP_XI_LED_SELECTOR = 411,      // Selects camera signalling LED
	CV_CAP_PROP_XI_LED_MODE = 412,      // Define camera signalling LED functionality
	CV_CAP_PROP_XI_MANUAL_WB = 413,      // Calculates White Balance(must be called during acquisition)
	CV_CAP_PROP_XI_AUTO_WB = 414,      // Automatic white balance
	CV_CAP_PROP_XI_AEAG = 415,      // Automatic exposure/gain
	CV_CAP_PROP_XI_EXP_PRIORITY = 416,      // Exposure priority (0.5 - exposure 50%, gain 50%).
	CV_CAP_PROP_XI_AE_MAX_LIMIT = 417,      // Maximum limit of exposure in AEAG procedure
	CV_CAP_PROP_XI_AG_MAX_LIMIT = 418,      // Maximum limit of gain in AEAG procedure
	CV_CAP_PROP_XI_AEAG_LEVEL = 419,       // Average intensity of output signal AEAG should achieve(in %)
	CV_CAP_PROP_XI_TIMEOUT = 420,       // Image capture timeout in milliseconds

	// Properties for Android cameras
	CV_CAP_PROP_ANDROID_FLASH_MODE = 8001,
	CV_CAP_PROP_ANDROID_FOCUS_MODE = 8002,
	CV_CAP_PROP_ANDROID_WHITE_BALANCE = 8003,
	CV_CAP_PROP_ANDROID_ANTIBANDING = 8004,
	CV_CAP_PROP_ANDROID_FOCAL_LENGTH = 8005,
	CV_CAP_PROP_ANDROID_FOCUS_DISTANCE_NEAR = 8006,
	CV_CAP_PROP_ANDROID_FOCUS_DISTANCE_OPTIMAL = 8007,
	CV_CAP_PROP_ANDROID_FOCUS_DISTANCE_FAR = 8008,
	CV_CAP_PROP_ANDROID_EXPOSE_LOCK = 8009,
	CV_CAP_PROP_ANDROID_WHITEBALANCE_LOCK = 8010,

	// Properties of cameras available through AVFOUNDATION interface
	CV_CAP_PROP_IOS_DEVICE_FOCUS = 9001,
	CV_CAP_PROP_IOS_DEVICE_EXPOSURE = 9002,
	CV_CAP_PROP_IOS_DEVICE_FLASH = 9003,
	CV_CAP_PROP_IOS_DEVICE_WHITEBALANCE = 9004,
	CV_CAP_PROP_IOS_DEVICE_TORCH = 9005,

	// Properties of cameras available through Smartek Giganetix Ethernet Vision interface
	/* --- Vladimir Litvinenko (litvinenko.vladimir@gmail.com) --- */
	CV_CAP_PROP_GIGA_FRAME_OFFSET_X = 10001,
	CV_CAP_PROP_GIGA_FRAME_OFFSET_Y = 10002,
	CV_CAP_PROP_GIGA_FRAME_WIDTH_MAX = 10003,
	CV_CAP_PROP_GIGA_FRAME_HEIGH_MAX = 10004,
	CV_CAP_PROP_GIGA_FRAME_SENS_WIDTH = 10005,
	CV_CAP_PROP_GIGA_FRAME_SENS_HEIGH = 10006,

	CV_CAP_PROP_INTELPERC_PROFILE_COUNT = 11001,
	CV_CAP_PROP_INTELPERC_PROFILE_IDX = 11002,
	CV_CAP_PROP_INTELPERC_DEPTH_LOW_CONFIDENCE_VALUE = 11003,
	CV_CAP_PROP_INTELPERC_DEPTH_SATURATION_VALUE = 11004,
	CV_CAP_PROP_INTELPERC_DEPTH_CONFIDENCE_THRESHOLD = 11005,
	CV_CAP_PROP_INTELPERC_DEPTH_FOCAL_LENGTH_HORZ = 11006,
	CV_CAP_PROP_INTELPERC_DEPTH_FOCAL_LENGTH_VERT = 11007,

	// Intel PerC streams
	CV_CAP_INTELPERC_DEPTH_GENERATOR = 1 << 29,
	CV_CAP_INTELPERC_IMAGE_GENERATOR = 1 << 28,
	CV_CAP_INTELPERC_GENERATORS_MASK = CV_CAP_INTELPERC_DEPTH_GENERATOR + CV_CAP_INTELPERC_IMAGE_GENERATOR
};

#define CV_FOURCC_MACRO(c1, c2, c3, c4) (((c1) & 255) + (((c2) & 255) << 8) + (((c3) & 255) << 16) + (((c4) & 255) << 24))

inline int CV_FOURCC(char c1, char c2, char c3, char c4)
{
	return CV_FOURCC_MACRO(c1, c2, c3, c4);
}

typedef struct device_info {
	std::string name;
	int vendor_id;
	int product_id;
} device_info;

struct CvCapture {
	virtual ~CvCapture() {}
	virtual double getProperty(int) { return 0; }
	virtual bool setProperty(int, double) { return 0; }
	virtual bool grabFrame() { return true; }
	virtual IplImage* retrieveFrame(int) { return 0; }
	virtual int getCaptureDomain() { return CV_CAP_ANY; } // Return the type of the capture object: CV_CAP_VFW, etc...
	virtual bool getDevicesList(std::map<int, device_info>& devicelist) const { return false; };
	virtual bool getCodecList(int device_id, std::vector<int>& codecids) const { return false; }
	virtual bool getVideoSizeList(int device_id, int codec_id, std::vector<std::string>& sizelist) const { return false; }
};

} // namespace fbc

#endif // FBC_CV_CAPTURE_HPP_
