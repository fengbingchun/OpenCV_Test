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

	int  getDeviceCount();

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

	virtual bool getDevicesList(std::map<int, std::string>& filenames) const;

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
