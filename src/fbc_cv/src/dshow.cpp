// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#include "core/types.hpp"
#include "dshow.hpp"
#include "videocapture.hpp"

// reference: 2.4.13.6
//            highgui/src/cap_dshow.cpp

#ifdef _MSC_VER

namespace fbc {

interface ISampleGrabberCB : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SampleCB(
	double SampleTime,
	IMediaSample *pSample) = 0;

	virtual HRESULT STDMETHODCALLTYPE BufferCB(
		double SampleTime,
		BYTE *pBuffer,
		LONG BufferLen) = 0;

	virtual ~ISampleGrabberCB() {}
};

interface ISampleGrabber : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetOneShot(
	BOOL OneShot) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetMediaType(
		const AM_MEDIA_TYPE *pType) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
		AM_MEDIA_TYPE *pType) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(
		BOOL BufferThem) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(
		LONG *pBufferSize,
		LONG *pBuffer) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(
		IMediaSample **ppSample) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetCallback(
		ISampleGrabberCB *pCallback,
		LONG WhichMethodToCallback) = 0;

	virtual ~ISampleGrabber() {}
};

///////////////////////////  HANDY FUNCTIONS  /////////////////////////////
static void MyFreeMediaType(AM_MEDIA_TYPE& mt){
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// Unecessary because pUnk should not be used, but safest.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

static void MyDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt != NULL)
	{
		MyFreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}

//////////////////////////////  CALLBACK  ////////////////////////////////
//Callback class
class SampleGrabberCallback : public ISampleGrabberCB{
public:
	//------------------------------------------------
	SampleGrabberCallback(){
		InitializeCriticalSection(&critSection);
		freezeCheck = 0;


		bufferSetup = false;
		newFrame = false;
		latestBufferLength = 0;

		hEvent = CreateEvent(NULL, true, false, NULL);
	}

	//------------------------------------------------
	virtual ~SampleGrabberCallback(){
		ptrBuffer = NULL;
		DeleteCriticalSection(&critSection);
		CloseHandle(hEvent);
		if (bufferSetup){
			delete[] pixels;
		}
	}

	//------------------------------------------------
	bool setupBuffer(int numBytesIn){
		if (bufferSetup){
			return false;
		}
		else{
			numBytes = numBytesIn;
			pixels = new unsigned char[numBytes];
			bufferSetup = true;
			newFrame = false;
			latestBufferLength = 0;
		}
		return true;
	}

	//------------------------------------------------
	STDMETHODIMP_(ULONG) AddRef() { return 1; }
	STDMETHODIMP_(ULONG) Release() { return 2; }

	//------------------------------------------------
	STDMETHODIMP QueryInterface(REFIID, void **ppvObject){
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}

	//This method is meant to have less overhead
	//------------------------------------------------
	STDMETHODIMP SampleCB(double, IMediaSample *pSample){
		if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) return S_OK;

		HRESULT hr = pSample->GetPointer(&ptrBuffer);

		if (hr == S_OK){
			latestBufferLength = pSample->GetActualDataLength();
			if (latestBufferLength == numBytes){
				EnterCriticalSection(&critSection);
				memcpy(pixels, ptrBuffer, latestBufferLength);
				newFrame = true;
				freezeCheck = 1;
				LeaveCriticalSection(&critSection);
				SetEvent(hEvent);
			}
			else{
				printf("ERROR: SampleCB() - buffer sizes do not match\n");
			}
		}

		return S_OK;
	}

	//This method is meant to have more overhead
	STDMETHODIMP BufferCB(double, BYTE *, long){
		return E_NOTIMPL;
	}

	int freezeCheck;

	int latestBufferLength;
	int numBytes;
	bool newFrame;
	bool bufferSetup;
	unsigned char * pixels;
	unsigned char * ptrBuffer;
	CRITICAL_SECTION critSection;
	HANDLE hEvent;
};

//////////////////////////////  VIDEO DEVICE  ////////////////////////////////
// ----------------------------------------------------------------------
//    Should this class also be the callback?
//
// ----------------------------------------------------------------------
videoDevice::videoDevice(){
	pCaptureGraph = NULL;    // Capture graph builder object
	pGraph = NULL;    // Graph builder object
	pControl = NULL;    // Media control object
	pVideoInputFilter = NULL; // Video Capture filter
	pGrabber = NULL; // Grabs frame
	pDestFilter = NULL; // Null Renderer Filter
	pGrabberF = NULL; // Grabber Filter
	pMediaEvent = NULL;
	streamConf = NULL;
	pAmMediaType = NULL;

	//This is our callback class that processes the frame.
	sgCallback = new SampleGrabberCallback();
	sgCallback->newFrame = false;

	//Default values for capture type
	videoType = MEDIASUBTYPE_RGB24;
	connection = PhysConn_Video_Composite;
	storeConn = 0;

	videoSize = 0;
	width = 0;
	height = 0;

	tryWidth = 640;
	tryHeight = 480;
	tryVideoType = MEDIASUBTYPE_RGB24;
	nFramesForReconnect = 10000;
	nFramesRunning = 0;
	myID = -1;

	tryDiffSize = true;
	useCrossbar = false;
	readyToCapture = false;
	sizeSet = false;
	setupStarted = false;
	specificFormat = false;
	autoReconnect = false;
	requestedFrameTime = -1;

	memset(wDeviceName, 0, sizeof(WCHAR)* 255);
	memset(nDeviceName, 0, sizeof(char)* 255);
}

// ----------------------------------------------------------------------
//    The only place we are doing new
//
// ----------------------------------------------------------------------
void videoDevice::setSize(int w, int h){
	if (sizeSet){
		if (verbose)printf("SETUP: Error device size should not be set more than once \n");
	}
	else
	{
		width = w;
		height = h;
		videoSize = w*h * 3;
		sizeSet = true;
		pixels = new unsigned char[videoSize];
		pBuffer = new char[videoSize];

		memset(pixels, 0, videoSize);
		sgCallback->setupBuffer(videoSize);

	}
}

// ----------------------------------------------------------------------
//    Borrowed from the SDK, use it to take apart the graph from
//  the capture device downstream to the null renderer
// ----------------------------------------------------------------------
void videoDevice::NukeDownstream(IBaseFilter *pBF){
	IPin *pP, *pTo;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;
	HRESULT hr = pBF->EnumPins(&pins);
	pins->Reset();
	while (hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if (pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if (hr == NOERROR)
				{
					if (pininfo.dir == PINDIR_INPUT)
					{
						NukeDownstream(pininfo.pFilter);
						pGraph->Disconnect(pTo);
						pGraph->Disconnect(pP);
						pGraph->RemoveFilter(pininfo.pFilter);
					}
					pininfo.pFilter->Release();
					pininfo.pFilter = NULL;
				}
				pTo->Release();
			}
			pP->Release();
		}
	}
	if (pins) pins->Release();
}

// ----------------------------------------------------------------------
//    Also from SDK
// ----------------------------------------------------------------------
void videoDevice::destroyGraph(){
	HRESULT hr = 0;
	//int FuncRetval=0;
	//int NumFilters=0;

	int i = 0;
	while (hr == NOERROR)
	{
		IEnumFilters * pEnum = 0;
		ULONG cFetched;

		// We must get the enumerator again every time because removing a filter from the graph
		// invalidates the enumerator. We always get only the first filter from each enumerator.
		hr = pGraph->EnumFilters(&pEnum);
		if (FAILED(hr)) { if (verbose)printf("SETUP: pGraph->EnumFilters() failed. \n"); return; }

		IBaseFilter * pFilter = NULL;
		if (pEnum->Next(1, &pFilter, &cFetched) == S_OK)
		{
			FILTER_INFO FilterInfo;
			memset(&FilterInfo, 0, sizeof(FilterInfo));
			hr = pFilter->QueryFilterInfo(&FilterInfo);
			FilterInfo.pGraph->Release();

			int count = 0;
			char buffer[255];
			memset(buffer, 0, 255 * sizeof(char));

			while (FilterInfo.achName[count] != 0x00)
			{
				buffer[count] = (char)FilterInfo.achName[count];
				count++;
			}

			if (verbose)printf("SETUP: removing filter %s...\n", buffer);
			hr = pGraph->RemoveFilter(pFilter);
			if (FAILED(hr)) { if (verbose)printf("SETUP: pGraph->RemoveFilter() failed. \n"); return; }
			if (verbose)printf("SETUP: filter removed %s  \n", buffer);

			pFilter->Release();
			pFilter = NULL;
		}
		else break;
		pEnum->Release();
		pEnum = NULL;
		i++;
	}

	return;
}

// ----------------------------------------------------------------------
// Our deconstructor, attempts to tear down graph and release filters etc
// Does checking to make sure it only is freeing if it needs to
// Probably could be a lot cleaner! :)
// ----------------------------------------------------------------------
videoDevice::~videoDevice(){
	if (setupStarted){ if (verbose)printf("\nSETUP: Disconnecting device %i\n", myID); }
	else{
		if (sgCallback){
			sgCallback->Release();
			delete sgCallback;
		}
		return;
	}

	HRESULT HR = NOERROR;

	//Stop the callback and free it
	if ((sgCallback) && (pGrabber))
	{
		pGrabber->SetCallback(NULL, 1);
		if (verbose)printf("SETUP: freeing Grabber Callback\n");
		sgCallback->Release();

		//delete our pixels
		if (sizeSet){
			delete[] pixels;
			delete[] pBuffer;
		}

		delete sgCallback;
	}

	//Check to see if the graph is running, if so stop it.
	if ((pControl))
	{
		HR = pControl->Pause();
		if (FAILED(HR)) if (verbose)printf("ERROR - Could not pause pControl\n");

		HR = pControl->Stop();
		if (FAILED(HR)) if (verbose)printf("ERROR - Could not stop pControl\n");
	}

	//Disconnect filters from capture device
	if ((pVideoInputFilter))NukeDownstream(pVideoInputFilter);

	//Release and zero pointers to our filters etc
	if ((pDestFilter)){
		if (verbose)printf("SETUP: freeing Renderer \n");
		(pDestFilter)->Release();
		(pDestFilter) = 0;
	}
	if ((pVideoInputFilter)){
		if (verbose)printf("SETUP: freeing Capture Source \n");
		(pVideoInputFilter)->Release();
		(pVideoInputFilter) = 0;
	}
	if ((pGrabberF)){
		if (verbose)printf("SETUP: freeing Grabber Filter  \n");
		(pGrabberF)->Release();
		(pGrabberF) = 0;
	}
	if ((pGrabber)){
		if (verbose)printf("SETUP: freeing Grabber  \n");
		(pGrabber)->Release();
		(pGrabber) = 0;
	}
	if ((pControl)){
		if (verbose)printf("SETUP: freeing Control   \n");
		(pControl)->Release();
		(pControl) = 0;
	}
	if ((pMediaEvent)){
		if (verbose)printf("SETUP: freeing Media Event  \n");
		(pMediaEvent)->Release();
		(pMediaEvent) = 0;
	}
	if ((streamConf)){
		if (verbose)printf("SETUP: freeing Stream  \n");
		(streamConf)->Release();
		(streamConf) = 0;
	}

	if ((pAmMediaType)){
		if (verbose)printf("SETUP: freeing Media Type  \n");
		MyDeleteMediaType(pAmMediaType);
	}

	if ((pMediaEvent)){
		if (verbose)printf("SETUP: freeing Media Event  \n");
		(pMediaEvent)->Release();
		(pMediaEvent) = 0;
	}

	//Destroy the graph
	if ((pGraph))destroyGraph();

	//Release and zero our capture graph and our main graph
	if ((pCaptureGraph)){
		if (verbose)printf("SETUP: freeing Capture Graph \n");
		(pCaptureGraph)->Release();
		(pCaptureGraph) = 0;
	}
	if ((pGraph)){
		if (verbose)printf("SETUP: freeing Main Graph \n");
		(pGraph)->Release();
		(pGraph) = 0;
	}

	//delete our pointers
	delete pDestFilter;
	delete pVideoInputFilter;
	delete pGrabberF;
	delete pGrabber;
	delete pControl;
	delete streamConf;
	delete pMediaEvent;
	delete pCaptureGraph;
	delete pGraph;

	if (verbose)printf("SETUP: Device %i disconnected and freed\n\n", myID);
}

//////////////////////////////  VIDEO INPUT  ////////////////////////////////
////////////////////////////  PUBLIC METHODS  ///////////////////////////////
// ----------------------------------------------------------------------
// Constructor - creates instances of videoDevice and adds the various
// media subtypes to check.
// ----------------------------------------------------------------------
videoInput::videoInput(){
	//start com
	comInit();

	devicesFound = 0;
	callbackSetCount = 0;
	bCallback = true;

	//setup a max no of device objects
	for (int i = 0; i<VI_MAX_CAMERAS; i++)  VDList[i] = new videoDevice();

	if (verbose)printf("\n***** VIDEOINPUT LIBRARY - %2.04f - TFW07 *****\n\n", VI_VERSION);

	//added for the pixelink firewire camera
	//MEDIASUBTYPE_Y800 = (GUID)FOURCCMap(FCC('Y800'));
	//MEDIASUBTYPE_Y8   = (GUID)FOURCCMap(FCC('Y8'));
	//MEDIASUBTYPE_GREY = (GUID)FOURCCMap(FCC('GREY'));

	//The video types we support
	//in order of preference

	mediaSubtypes[0] = MEDIASUBTYPE_RGB24;
	mediaSubtypes[1] = MEDIASUBTYPE_RGB32;
	mediaSubtypes[2] = MEDIASUBTYPE_RGB555;
	mediaSubtypes[3] = MEDIASUBTYPE_RGB565;
	mediaSubtypes[4] = MEDIASUBTYPE_YUY2;
	mediaSubtypes[5] = MEDIASUBTYPE_YVYU;
	mediaSubtypes[6] = MEDIASUBTYPE_YUYV;
	mediaSubtypes[7] = MEDIASUBTYPE_IYUV;
	mediaSubtypes[8] = MEDIASUBTYPE_UYVY;
	mediaSubtypes[9] = MEDIASUBTYPE_YV12;
	mediaSubtypes[10] = MEDIASUBTYPE_YVU9;
	mediaSubtypes[11] = MEDIASUBTYPE_Y411;
	mediaSubtypes[12] = MEDIASUBTYPE_Y41P;
	mediaSubtypes[13] = MEDIASUBTYPE_Y211;
	mediaSubtypes[14] = MEDIASUBTYPE_AYUV;
	mediaSubtypes[15] = MEDIASUBTYPE_MJPG; // MGB

	//non standard
	mediaSubtypes[16] = MEDIASUBTYPE_Y800;
	mediaSubtypes[17] = MEDIASUBTYPE_Y8;
	mediaSubtypes[18] = MEDIASUBTYPE_GREY;
	mediaSubtypes[19] = MEDIASUBTYPE_I420;

	//The video formats we support
	formatTypes[VI_NTSC_M] = AnalogVideo_NTSC_M;
	formatTypes[VI_NTSC_M_J] = AnalogVideo_NTSC_M_J;
	formatTypes[VI_NTSC_433] = AnalogVideo_NTSC_433;

	formatTypes[VI_PAL_B] = AnalogVideo_PAL_B;
	formatTypes[VI_PAL_D] = AnalogVideo_PAL_D;
	formatTypes[VI_PAL_G] = AnalogVideo_PAL_G;
	formatTypes[VI_PAL_H] = AnalogVideo_PAL_H;
	formatTypes[VI_PAL_I] = AnalogVideo_PAL_I;
	formatTypes[VI_PAL_M] = AnalogVideo_PAL_M;
	formatTypes[VI_PAL_N] = AnalogVideo_PAL_N;
	formatTypes[VI_PAL_NC] = AnalogVideo_PAL_N_COMBO;

	formatTypes[VI_SECAM_B] = AnalogVideo_SECAM_B;
	formatTypes[VI_SECAM_D] = AnalogVideo_SECAM_D;
	formatTypes[VI_SECAM_G] = AnalogVideo_SECAM_G;
	formatTypes[VI_SECAM_H] = AnalogVideo_SECAM_H;
	formatTypes[VI_SECAM_K] = AnalogVideo_SECAM_K;
	formatTypes[VI_SECAM_K1] = AnalogVideo_SECAM_K1;
	formatTypes[VI_SECAM_L] = AnalogVideo_SECAM_L;
}

// ----------------------------------------------------------------------
// static - set whether messages get printed to console or not
//
// ----------------------------------------------------------------------
void videoInput::setVerbose(bool _verbose){
	verbose = _verbose;
}

// ----------------------------------------------------------------------
// change to use callback or regular capture
// callback tells you when a new frame has arrived
// but non-callback won't - but is single threaded
// ----------------------------------------------------------------------
void videoInput::setUseCallback(bool useCallback){
	if (callbackSetCount == 0){
		bCallback = useCallback;
		callbackSetCount = 1;
	}
	else{
		printf("ERROR: setUseCallback can only be called before setup\n");
	}
}

// ----------------------------------------------------------------------
// Set the requested framerate - no guarantee you will get this
//
// ----------------------------------------------------------------------
void videoInput::setIdealFramerate(int deviceNumber, int idealFramerate){
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return;

	if (idealFramerate > 0){
		VDList[deviceNumber]->requestedFrameTime = (unsigned long)(10000000 / idealFramerate);
	}
}

// ----------------------------------------------------------------------
// Set the requested framerate - no guarantee you will get this
//
// ----------------------------------------------------------------------
void videoInput::setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect){
	if (deviceNumber >= VI_MAX_CAMERAS) return;

	VDList[deviceNumber]->autoReconnect = doReconnect;
	VDList[deviceNumber]->nFramesForReconnect = numMissedFramesBeforeReconnect;

}

// ----------------------------------------------------------------------
// Setup a device with the default settings
//
// ----------------------------------------------------------------------
bool videoInput::setupDevice(int deviceNumber){
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return false;

	if (setup(deviceNumber))return true;
	return false;
}

// ----------------------------------------------------------------------
// Setup a device with the default size but specify input type
//
// ----------------------------------------------------------------------
bool videoInput::setupDevice(int deviceNumber, int _connection){
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return false;

	setPhyCon(deviceNumber, _connection);
	if (setup(deviceNumber))return true;
	return false;
}

// ----------------------------------------------------------------------
// Setup a device with the default connection but specify size
//
// ----------------------------------------------------------------------
bool videoInput::setupDevice(int deviceNumber, int w, int h){
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return false;

	setAttemptCaptureSize(deviceNumber, w, h);
	if (setup(deviceNumber))return true;
	return false;
}

// ----------------------------------------------------------------------
// Setup a device with the default connection but specify size and image format
//
// Note:
// Need a new name for this since signature clashes with ",int connection)"
// ----------------------------------------------------------------------
bool videoInput::setupDeviceFourcc(int deviceNumber, int w, int h, int fourcc){
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return false;

	if (fourcc != -1) {
		GUID *mediaType = getMediaSubtypeFromFourcc(fourcc);
		if (mediaType) {
			setAttemptCaptureSize(deviceNumber, w, h, *mediaType);
		}
	}
	else {
		setAttemptCaptureSize(deviceNumber, w, h);
	}
	if (setup(deviceNumber))return true;
	return false;
}

// ----------------------------------------------------------------------
// Setup a device with specific size and connection
//
// ----------------------------------------------------------------------
bool videoInput::setupDevice(int deviceNumber, int w, int h, int _connection){
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return false;

	setAttemptCaptureSize(deviceNumber, w, h);
	setPhyCon(deviceNumber, _connection);
	if (setup(deviceNumber))return true;
	return false;
}

// ----------------------------------------------------------------------
// Setup the default video format of the device
// Must be called after setup!
// See #define formats in header file (eg VI_NTSC_M )
//
// ----------------------------------------------------------------------
bool videoInput::setFormat(int deviceNumber, int format){
	if (deviceNumber >= VI_MAX_CAMERAS || !VDList[deviceNumber]->readyToCapture) return false;

	bool returnVal = false;

	if (format >= 0 && format < VI_NUM_FORMATS){
		VDList[deviceNumber]->formatType = formatTypes[format];
		VDList[deviceNumber]->specificFormat = true;

		if (VDList[deviceNumber]->specificFormat){

			HRESULT hr = getDevice(&VDList[deviceNumber]->pVideoInputFilter, deviceNumber, VDList[deviceNumber]->wDeviceName, VDList[deviceNumber]->nDeviceName);
			if (hr != S_OK){
				return false;
			}

			IAMAnalogVideoDecoder *pVideoDec = NULL;
			hr = VDList[deviceNumber]->pCaptureGraph->FindInterface(NULL, &MEDIATYPE_Video, VDList[deviceNumber]->pVideoInputFilter, IID_IAMAnalogVideoDecoder, (void **)&pVideoDec);


			//in case the settings window some how freed them first
			if (VDList[deviceNumber]->pVideoInputFilter)VDList[deviceNumber]->pVideoInputFilter->Release();
			if (VDList[deviceNumber]->pVideoInputFilter)VDList[deviceNumber]->pVideoInputFilter = NULL;

			if (FAILED(hr)){
				printf("SETUP: couldn't set requested format\n");
			}
			else{
				long lValue = 0;
				hr = pVideoDec->get_AvailableTVFormats(&lValue);
				if (SUCCEEDED(hr) && (lValue & VDList[deviceNumber]->formatType))
				{
					hr = pVideoDec->put_TVFormat(VDList[deviceNumber]->formatType);
					if (FAILED(hr)){
						printf("SETUP: couldn't set requested format\n");
					}
					else{
						returnVal = true;
					}
				}

				pVideoDec->Release();
				pVideoDec = NULL;
			}
		}
	}

	return returnVal;
}

// ----------------------------------------------------------------------
// Our static function for returning device names - thanks Peter!
// Must call listDevices first.
//
// ----------------------------------------------------------------------
char videoInput::deviceNames[VI_MAX_CAMERAS][255] = { { 0 } };

char * videoInput::getDeviceName(int deviceID){
	if (deviceID >= VI_MAX_CAMERAS){
		return NULL;
	}
	return deviceNames[deviceID];
}

// ----------------------------------------------------------------------
// Our static function for finding num devices available etc
//
// ----------------------------------------------------------------------
int videoInput::listDevices(bool silent){

	//COM Library Intialization
	comInit();

	if (!silent)printf("\nVIDEOINPUT SPY MODE!\n\n");

	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK){

			if (!silent)printf("SETUP: Looking For Capture Devices\n");
			IMoniker *pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK){
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)){
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}

				// Find the description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);

				if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

				if (SUCCEEDED(hr)){
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					int count = 0;
					int maxLen = sizeof(deviceNames[0]) / sizeof(deviceNames[0][0]) - 2;
					while (varName.bstrVal[count] != 0x00 && count < maxLen) {
						deviceNames[deviceCounter][count] = (char)varName.bstrVal[count];
						count++;
					}
					deviceNames[deviceCounter][count] = 0;

					if (!silent)printf("SETUP: %i) %s \n", deviceCounter, deviceNames[deviceCounter]);
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		if (!silent)printf("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	comUnInit();

	return deviceCounter;
}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
int videoInput::getWidth(int id){

	if (isDeviceSetup(id))
	{
		return VDList[id]->width;
	}

	return 0;

}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
int videoInput::getHeight(int id){
	if (isDeviceSetup(id))
	{
		return VDList[id]->height;
	}

	return 0;

}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
int videoInput::getFourcc(int id){
	if (isDeviceSetup(id))
	{
		return getFourccFromMediaSubtype(VDList[id]->videoType);
	}

	return 0;

}

double videoInput::getFPS(int id){
	if (isDeviceSetup(id))
	{
		double frameTime = VDList[id]->requestedFrameTime;
		if (frameTime>0) {
			return (10000000.0 / frameTime);
		}
	}

	return 0;

}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
int videoInput::getSize(int id){
	if (isDeviceSetup(id))
	{
		return VDList[id]->videoSize;
	}

	return 0;

}

// ----------------------------------------------------------------------
// Uses a supplied buffer
// ----------------------------------------------------------------------
bool videoInput::getPixels(int id, unsigned char * dstBuffer, bool flipRedAndBlue, bool flipImage){
	bool success = false;

	if (isDeviceSetup(id)){
		if (bCallback){
			//callback capture

			DWORD result = WaitForSingleObject(VDList[id]->sgCallback->hEvent, 1000);
			if (result != WAIT_OBJECT_0) return false;

			//double paranoia - mutexing with both event and critical section
			EnterCriticalSection(&VDList[id]->sgCallback->critSection);

			unsigned char * src = VDList[id]->sgCallback->pixels;
			unsigned char * dst = dstBuffer;
			int height = VDList[id]->height;
			int width = VDList[id]->width;

			processPixels(src, dst, width, height, flipRedAndBlue, flipImage);
			VDList[id]->sgCallback->newFrame = false;

			LeaveCriticalSection(&VDList[id]->sgCallback->critSection);

			ResetEvent(VDList[id]->sgCallback->hEvent);

			success = true;
		}
		else{
			//regular capture method
			long bufferSize = VDList[id]->videoSize;
			HRESULT hr = VDList[id]->pGrabber->GetCurrentBuffer(&bufferSize, (long *)VDList[id]->pBuffer);
			if (hr == S_OK){
				int numBytes = VDList[id]->videoSize;
				if (numBytes == bufferSize){

					unsigned char * src = (unsigned char *)VDList[id]->pBuffer;
					unsigned char * dst = dstBuffer;
					int height = VDList[id]->height;
					int width = VDList[id]->width;

					processPixels(src, dst, width, height, flipRedAndBlue, flipImage);
					success = true;
				}
				else{
					if (verbose)printf("ERROR: GetPixels() - bufferSizes do not match!\n");
				}
			}
			else{
				if (verbose)printf("ERROR: GetPixels() - Unable to grab frame for device %i\n", id);
			}
		}
	}

	return success;
}

// ----------------------------------------------------------------------
// Returns a buffer
// ----------------------------------------------------------------------
unsigned char * videoInput::getPixels(int id, bool flipRedAndBlue, bool flipImage){

	if (isDeviceSetup(id)){
		getPixels(id, VDList[id]->pixels, flipRedAndBlue, flipImage);
	}

	return VDList[id]->pixels;
}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
bool videoInput::isFrameNew(int id){
	if (!isDeviceSetup(id)) return false;
	if (!bCallback)return true;

	bool result = false;
	bool freeze = false;

	//again super paranoia!
	EnterCriticalSection(&VDList[id]->sgCallback->critSection);
	result = VDList[id]->sgCallback->newFrame;

	//we need to give it some time at the begining to start up so lets check after 400 frames
	if (VDList[id]->nFramesRunning > 400 && VDList[id]->sgCallback->freezeCheck > VDList[id]->nFramesForReconnect){
		freeze = true;
	}

	//we increment the freezeCheck var here - the callback resets it to 1
	//so as long as the callback is running this var should never get too high.
	//if the callback is not running then this number will get high and trigger the freeze action below
	VDList[id]->sgCallback->freezeCheck++;
	LeaveCriticalSection(&VDList[id]->sgCallback->critSection);

	VDList[id]->nFramesRunning++;

	if (freeze && VDList[id]->autoReconnect){
		if (verbose)printf("ERROR: Device seems frozen - attempting to reconnect\n");
		if (!restartDevice(VDList[id]->myID)){
			if (verbose)printf("ERROR: Unable to reconnect to device\n");
		}
		else{
			if (verbose)printf("SUCCESS: Able to reconnect to device\n");
		}
	}

	return result;
}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
bool videoInput::isDeviceSetup(int id){

	if (id<devicesFound && VDList[id]->readyToCapture)return true;
	else return false;

}

// ----------------------------------------------------------------------
// Gives us a little pop up window to adjust settings
// We do this in a seperate thread now!
// ----------------------------------------------------------------------
void __cdecl videoInput::basicThread(void * objPtr){

	//get a reference to the video device
	//not a copy as we need to free the filter
	videoDevice * vd = *((videoDevice **)(objPtr));
	ShowFilterPropertyPages(vd->pVideoInputFilter);

	//now we free the filter and make sure it set to NULL
	if (vd->pVideoInputFilter)vd->pVideoInputFilter->Release();
	if (vd->pVideoInputFilter)vd->pVideoInputFilter = NULL;

	return;
}

void videoInput::showSettingsWindow(int id){
	if (isDeviceSetup(id)){
		//HANDLE myTempThread;

		//we reconnect to the device as we have freed our reference to it
		//why have we freed our reference? because there seemed to be an issue
		//with some mpeg devices if we didn't
		HRESULT hr = getDevice(&VDList[id]->pVideoInputFilter, id, VDList[id]->wDeviceName, VDList[id]->nDeviceName);
		if (hr == S_OK){
			//myTempThread = (HANDLE)
			_beginthread(basicThread, 0, (void *)&VDList[id]);
		}
	}
}

// Set a video signal setting using IAMVideoProcAmp
bool videoInput::getVideoSettingFilter(int deviceID, long Property, long &min, long &max, long &SteppingDelta, long &currentValue, long &flags, long &defaultValue){
	if (!isDeviceSetup(deviceID))return false;

	HRESULT hr;
	//bool isSuccessful = false;

	videoDevice * VD = VDList[deviceID];

	hr = getDevice(&VD->pVideoInputFilter, deviceID, VD->wDeviceName, VD->nDeviceName);
	if (FAILED(hr)){
		printf("setVideoSetting - getDevice Error\n");
		return false;
	}

	IAMVideoProcAmp *pAMVideoProcAmp = NULL;

	hr = VD->pVideoInputFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pAMVideoProcAmp);
	if (FAILED(hr)){
		printf("setVideoSetting - QueryInterface Error\n");
		if (VD->pVideoInputFilter)VD->pVideoInputFilter->Release();
		if (VD->pVideoInputFilter)VD->pVideoInputFilter = NULL;
		return false;
	}

	char propStr[16];
	getVideoPropertyAsString(Property, propStr);

	if (verbose) printf("Setting video setting %s.\n", propStr);

	pAMVideoProcAmp->GetRange(Property, &min, &max, &SteppingDelta, &defaultValue, &flags);
	if (verbose) printf("Range for video setting %s: Min:%ld Max:%ld SteppingDelta:%ld Default:%ld Flags:%ld\n", propStr, min, max, SteppingDelta, defaultValue, flags);
	pAMVideoProcAmp->Get(Property, &currentValue, &flags);

	if (pAMVideoProcAmp)pAMVideoProcAmp->Release();
	if (VD->pVideoInputFilter)VD->pVideoInputFilter->Release();
	if (VD->pVideoInputFilter)VD->pVideoInputFilter = NULL;

	return true;

}

// Set a video signal setting using IAMVideoProcAmp
bool videoInput::setVideoSettingFilterPct(int deviceID, long Property, float pctValue, long Flags){
	if (!isDeviceSetup(deviceID))return false;

	long min, max, currentValue, flags, defaultValue, stepAmnt;

	if (!getVideoSettingFilter(deviceID, Property, min, max, stepAmnt, currentValue, flags, defaultValue))return false;

	if (pctValue > 1.0)pctValue = 1.0;
	else if (pctValue < 0)pctValue = 0.0;

	float range = (float)max - (float)min;
	if (range <= 0)return false;
	if (stepAmnt == 0) return false;

	long value = (long)((float)min + range * pctValue);
	long rasterValue = value;

	//if the range is the stepAmnt then it is just a switch
	//so we either set the value to low or high
	if (range == stepAmnt){
		if (pctValue < 0.5)rasterValue = min;
		else rasterValue = max;
	}
	else{
		//we need to rasterize the value to the stepping amnt
		long mod = value % stepAmnt;
		float halfStep = (float)stepAmnt * 0.5f;
		if (mod < halfStep) rasterValue -= mod;
		else rasterValue += stepAmnt - mod;
		printf("RASTER - pctValue is %f - value is %li - step is %li - mod is %li - rasterValue is %li\n", pctValue, value, stepAmnt, mod, rasterValue);
	}

	return setVideoSettingFilter(deviceID, Property, rasterValue, Flags, false);
}

// Set a video signal setting using IAMVideoProcAmp
bool videoInput::setVideoSettingFilter(int deviceID, long Property, long lValue, long Flags, bool useDefaultValue){
	if (!isDeviceSetup(deviceID))return false;

	HRESULT hr;
	//bool isSuccessful = false;

	char propStr[16];
	getVideoPropertyAsString(Property, propStr);

	videoDevice * VD = VDList[deviceID];

	hr = getDevice(&VD->pVideoInputFilter, deviceID, VD->wDeviceName, VD->nDeviceName);
	if (FAILED(hr)){
		printf("setVideoSetting - getDevice Error\n");
		return false;
	}

	IAMVideoProcAmp *pAMVideoProcAmp = NULL;

	hr = VD->pVideoInputFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pAMVideoProcAmp);
	if (FAILED(hr)){
		printf("setVideoSetting - QueryInterface Error\n");
		if (VD->pVideoInputFilter)VD->pVideoInputFilter->Release();
		if (VD->pVideoInputFilter)VD->pVideoInputFilter = NULL;
		return false;
	}

	if (verbose) printf("Setting video setting %s.\n", propStr);
	long CurrVal, Min, Max, SteppingDelta, Default, CapsFlags, AvailableCapsFlags = 0;


	pAMVideoProcAmp->GetRange(Property, &Min, &Max, &SteppingDelta, &Default, &AvailableCapsFlags);
	if (verbose) printf("Range for video setting %s: Min:%ld Max:%ld SteppingDelta:%ld Default:%ld Flags:%ld\n", propStr, Min, Max, SteppingDelta, Default, AvailableCapsFlags);
	pAMVideoProcAmp->Get(Property, &CurrVal, &CapsFlags);

	if (verbose) printf("Current value: %ld Flags %ld (%s)\n", CurrVal, CapsFlags, (CapsFlags == 1 ? "Auto" : (CapsFlags == 2 ? "Manual" : "Unknown")));

	if (useDefaultValue) {
		pAMVideoProcAmp->Set(Property, Default, VideoProcAmp_Flags_Auto);
	}
	else{
		// Perhaps add a check that lValue and Flags are within the range aquired from GetRange above
		pAMVideoProcAmp->Set(Property, lValue, Flags);
	}

	if (pAMVideoProcAmp)pAMVideoProcAmp->Release();
	if (VD->pVideoInputFilter)VD->pVideoInputFilter->Release();
	if (VD->pVideoInputFilter)VD->pVideoInputFilter = NULL;

	return true;

}

bool videoInput::setVideoSettingCameraPct(int deviceID, long Property, float pctValue, long Flags){
	if (!isDeviceSetup(deviceID))return false;

	long min, max, currentValue, flags, defaultValue, stepAmnt;

	if (!getVideoSettingCamera(deviceID, Property, min, max, stepAmnt, currentValue, flags, defaultValue))return false;

	if (pctValue > 1.0)pctValue = 1.0;
	else if (pctValue < 0)pctValue = 0.0;

	float range = (float)max - (float)min;
	if (range <= 0)return false;
	if (stepAmnt == 0) return false;

	long value = (long)((float)min + range * pctValue);
	long rasterValue = value;

	//if the range is the stepAmnt then it is just a switch
	//so we either set the value to low or high
	if (range == stepAmnt){
		if (pctValue < 0.5)rasterValue = min;
		else rasterValue = max;
	}
	else{
		//we need to rasterize the value to the stepping amnt
		long mod = value % stepAmnt;
		float halfStep = (float)stepAmnt * 0.5f;
		if (mod < halfStep) rasterValue -= mod;
		else rasterValue += stepAmnt - mod;
		printf("RASTER - pctValue is %f - value is %li - step is %li - mod is %li - rasterValue is %li\n", pctValue, value, stepAmnt, mod, rasterValue);
	}

	return setVideoSettingCamera(deviceID, Property, rasterValue, Flags, false);
}

bool videoInput::setVideoSettingCamera(int deviceID, long Property, long lValue, long Flags, bool useDefaultValue){
	IAMCameraControl *pIAMCameraControl;
	if (isDeviceSetup(deviceID))
	{
		HRESULT hr;
		hr = getDevice(&VDList[deviceID]->pVideoInputFilter, deviceID, VDList[deviceID]->wDeviceName, VDList[deviceID]->nDeviceName);

		char propStr[16];
		getCameraPropertyAsString(Property, propStr);

		if (verbose) printf("Setting video setting %s.\n", propStr);
		hr = VDList[deviceID]->pVideoInputFilter->QueryInterface(IID_IAMCameraControl, (void**)&pIAMCameraControl);
		if (FAILED(hr)) {
			printf("Error\n");
			if (VDList[deviceID]->pVideoInputFilter)VDList[deviceID]->pVideoInputFilter->Release();
			if (VDList[deviceID]->pVideoInputFilter)VDList[deviceID]->pVideoInputFilter = NULL;
			return false;
		}
		else
		{
			long CurrVal, Min, Max, SteppingDelta, Default, CapsFlags, AvailableCapsFlags;
			pIAMCameraControl->GetRange(Property, &Min, &Max, &SteppingDelta, &Default, &AvailableCapsFlags);
			if (verbose) printf("Range for video setting %s: Min:%ld Max:%ld SteppingDelta:%ld Default:%ld Flags:%ld\n", propStr, Min, Max, SteppingDelta, Default, AvailableCapsFlags);
			pIAMCameraControl->Get(Property, &CurrVal, &CapsFlags);
			if (verbose) printf("Current value: %ld Flags %ld (%s)\n", CurrVal, CapsFlags, (CapsFlags == 1 ? "Auto" : (CapsFlags == 2 ? "Manual" : "Unknown")));
			if (useDefaultValue) {
				pIAMCameraControl->Set(Property, Default, CameraControl_Flags_Auto);
			}
			else
			{
				// Perhaps add a check that lValue and Flags are within the range aquired from GetRange above
				pIAMCameraControl->Set(Property, lValue, Flags);
			}
			pIAMCameraControl->Release();
			if (VDList[deviceID]->pVideoInputFilter)VDList[deviceID]->pVideoInputFilter->Release();
			if (VDList[deviceID]->pVideoInputFilter)VDList[deviceID]->pVideoInputFilter = NULL;
			return true;
		}
	}
	return false;
}

bool videoInput::getVideoSettingCamera(int deviceID, long Property, long &min, long &max, long &SteppingDelta, long &currentValue, long &flags, long &defaultValue){
	if (!isDeviceSetup(deviceID))return false;

	HRESULT hr;
	//bool isSuccessful = false;

	videoDevice * VD = VDList[deviceID];

	hr = getDevice(&VD->pVideoInputFilter, deviceID, VD->wDeviceName, VD->nDeviceName);
	if (FAILED(hr)){
		printf("setVideoSetting - getDevice Error\n");
		return false;
	}

	IAMCameraControl *pIAMCameraControl = NULL;

	hr = VD->pVideoInputFilter->QueryInterface(IID_IAMCameraControl, (void**)&pIAMCameraControl);
	if (FAILED(hr)){
		printf("setVideoSetting - QueryInterface Error\n");
		if (VD->pVideoInputFilter)VD->pVideoInputFilter->Release();
		if (VD->pVideoInputFilter)VD->pVideoInputFilter = NULL;
		return false;
	}

	char propStr[16];
	getCameraPropertyAsString(Property, propStr);
	if (verbose) printf("Setting video setting %s.\n", propStr);

	pIAMCameraControl->GetRange(Property, &min, &max, &SteppingDelta, &defaultValue, &flags);
	if (verbose) printf("Range for video setting %s: Min:%ld Max:%ld SteppingDelta:%ld Default:%ld Flags:%ld\n", propStr, min, max, SteppingDelta, defaultValue, flags);
	pIAMCameraControl->Get(Property, &currentValue, &flags);

	if (pIAMCameraControl)pIAMCameraControl->Release();
	if (VD->pVideoInputFilter)VD->pVideoInputFilter->Release();
	if (VD->pVideoInputFilter)VD->pVideoInputFilter = NULL;

	return true;

}

// ----------------------------------------------------------------------
// Shutsdown the device, deletes the object and creates a new object
// so it is ready to be setup again
// ----------------------------------------------------------------------
void videoInput::stopDevice(int id){
	if (id < VI_MAX_CAMERAS)
	{
		delete VDList[id];
		VDList[id] = new videoDevice();
	}

}

// ----------------------------------------------------------------------
// Restarts the device with the same settings it was using
//
// ----------------------------------------------------------------------
bool videoInput::restartDevice(int id){
	if (isDeviceSetup(id))
	{
		int conn = VDList[id]->storeConn;
		int tmpW = VDList[id]->width;
		int tmpH = VDList[id]->height;

		bool bFormat = VDList[id]->specificFormat;
		long format = VDList[id]->formatType;

		int nReconnect = VDList[id]->nFramesForReconnect;
		bool bReconnect = VDList[id]->autoReconnect;

		unsigned long avgFrameTime = VDList[id]->requestedFrameTime;

		stopDevice(id);

		//set our fps if needed
		if (avgFrameTime != (unsigned long)-1){
			VDList[id]->requestedFrameTime = avgFrameTime;
		}

		if (setupDevice(id, tmpW, tmpH, conn)){
			//reapply the format - ntsc / pal etc
			if (bFormat){
				setFormat(id, format);
			}
			if (bReconnect){
				setAutoReconnectOnFreeze(id, true, nReconnect);
			}
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------
// Shuts down all devices, deletes objects and unitializes com if needed
//
// ----------------------------------------------------------------------
videoInput::~videoInput(){

	for (int i = 0; i < VI_MAX_CAMERAS; i++)
	{
		delete VDList[i];
	}
	//Unitialize com
	comUnInit();
}

//////////////////////////////  VIDEO INPUT  ////////////////////////////////
////////////////////////////  PRIVATE METHODS  //////////////////////////////
// ----------------------------------------------------------------------
// We only should init com if it hasn't been done so by our apps thread
// Use a static counter to keep track of other times it has been inited
// (do we need to worry about multithreaded apps?)
// ----------------------------------------------------------------------
bool videoInput::comInit(){
	/*HRESULT hr = NOERROR;

	//no need for us to start com more than once
	if(comInitCount == 0 ){

	// Initialize the COM library.
	//CoInitializeEx so videoInput can run in another thread
	#ifdef VI_COM_MULTI_THREADED
	hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
	#else
	hr = CoInitialize(NULL);
	#endif
	//this is the only case where there might be a problem
	//if another library has started com as single threaded
	//and we need it multi-threaded - send warning but don't fail
	if( hr == RPC_E_CHANGED_MODE){
	if(verbose)printf("SETUP - COM already setup - threaded VI might not be possible\n");
	}
	}

	comInitCount++;*/
	return true;
}

// ----------------------------------------------------------------------
// Same as above but to unitialize com, decreases counter and frees com
// if no one else is using it
// ----------------------------------------------------------------------
bool videoInput::comUnInit(){
	/*if(comInitCount > 0)comInitCount--;        //decrease the count of instances using com

	if(comInitCount == 0){
	CoUninitialize();    //if there are no instances left - uninitialize com
	return true;
	}

	return false;*/
	return true;
}

// ----------------------------------------------------------------------
// This is the size we ask for - we might not get it though :)
//
// ----------------------------------------------------------------------
void videoInput::setAttemptCaptureSize(int id, int w, int h, GUID mediaType){

	VDList[id]->tryWidth = w;
	VDList[id]->tryHeight = h;
	VDList[id]->tryDiffSize = true;
	VDList[id]->tryVideoType = mediaType;

}

// ----------------------------------------------------------------------
// Set the connection type
// (maybe move to private?)
// ----------------------------------------------------------------------
void videoInput::setPhyCon(int id, int conn){
	switch (conn){

	case 0:
		VDList[id]->connection = PhysConn_Video_Composite;
		break;
	case 1:
		VDList[id]->connection = PhysConn_Video_SVideo;
		break;
	case 2:
		VDList[id]->connection = PhysConn_Video_Tuner;
		break;
	case 3:
		VDList[id]->connection = PhysConn_Video_USB;
		break;
	case 4:
		VDList[id]->connection = PhysConn_Video_1394;
		break;
	default:
		return; //if it is not these types don't set crossbar
		break;
	}

	VDList[id]->storeConn = conn;
	VDList[id]->useCrossbar = true;
}

// ----------------------------------------------------------------------
// Check that we are not trying to setup a non-existant device
// Then start the graph building!
// ----------------------------------------------------------------------
bool videoInput::setup(int deviceNumber){
	devicesFound = getDeviceCount();

	if (deviceNumber>devicesFound - 1)
	{
		if (verbose)printf("SETUP: device[%i] not found - you have %i devices available\n", deviceNumber, devicesFound);
		if (devicesFound >= 0) if (verbose)printf("SETUP: this means that the last device you can use is device[%i] \n", devicesFound - 1);
		return false;
	}

	if (VDList[deviceNumber]->readyToCapture)
	{
		if (verbose)printf("SETUP: can't setup, device %i is currently being used\n", VDList[deviceNumber]->myID);
		return false;
	}

	HRESULT hr = start(deviceNumber, VDList[deviceNumber]);
	if (hr == S_OK)return true;
	else return false;
}

// ----------------------------------------------------------------------
// Does both vertical buffer flipping and bgr to rgb swapping
// You have any combination of those.
// ----------------------------------------------------------------------
void videoInput::processPixels(unsigned char * src, unsigned char * dst, int width, int height, bool bRGB, bool bFlip){

	int widthInBytes = width * 3;
	int numBytes = widthInBytes * height;

	if (!bRGB){

		//int x = 0;
		//int y = 0;

		if (bFlip){
			for (int y = 0; y < height; y++){
				memcpy(dst + (y * widthInBytes), src + ((height - y - 1) * widthInBytes), widthInBytes);
			}

		}
		else{
			memcpy(dst, src, numBytes);
		}
	}
	else{
		if (bFlip){

			int x = 0;
			int y = (height - 1) * widthInBytes;
			src += y;

			for (int i = 0; i < numBytes; i += 3){
				if (x >= width){
					x = 0;
					src -= widthInBytes * 2;
				}

				*dst = *(src + 2);
				dst++;

				*dst = *(src + 1);
				dst++;

				*dst = *src;
				dst++;

				src += 3;
				x++;
			}
		}
		else{
			for (int i = 0; i < numBytes; i += 3){
				*dst = *(src + 2);
				dst++;

				*dst = *(src + 1);
				dst++;

				*dst = *src;
				dst++;

				src += 3;
			}
		}
	}
}

//------------------------------------------------------------------------------------------
void videoInput::getMediaSubtypeAsString(GUID type, char * typeAsString){
	char tmpStr[8];
	if (type == MEDIASUBTYPE_RGB24)     sprintf(tmpStr, "RGB24");
	else if (type == MEDIASUBTYPE_RGB32) sprintf(tmpStr, "RGB32");
	else if (type == MEDIASUBTYPE_RGB555)sprintf(tmpStr, "RGB555");
	else if (type == MEDIASUBTYPE_RGB565)sprintf(tmpStr, "RGB565");
	else if (type == MEDIASUBTYPE_YUY2)  sprintf(tmpStr, "YUY2");
	else if (type == MEDIASUBTYPE_YVYU)  sprintf(tmpStr, "YVYU");
	else if (type == MEDIASUBTYPE_YUYV)  sprintf(tmpStr, "YUYV");
	else if (type == MEDIASUBTYPE_IYUV)  sprintf(tmpStr, "IYUV");
	else if (type == MEDIASUBTYPE_UYVY)  sprintf(tmpStr, "UYVY");
	else if (type == MEDIASUBTYPE_YV12)  sprintf(tmpStr, "YV12");
	else if (type == MEDIASUBTYPE_YVU9)  sprintf(tmpStr, "YVU9");
	else if (type == MEDIASUBTYPE_Y411)  sprintf(tmpStr, "Y411");
	else if (type == MEDIASUBTYPE_Y41P)  sprintf(tmpStr, "Y41P");
	else if (type == MEDIASUBTYPE_Y211)  sprintf(tmpStr, "Y211");
	else if (type == MEDIASUBTYPE_AYUV)  sprintf(tmpStr, "AYUV");
	else if (type == MEDIASUBTYPE_MJPG)  sprintf(tmpStr, "MJPG");
	else if (type == MEDIASUBTYPE_Y800)  sprintf(tmpStr, "Y800");
	else if (type == MEDIASUBTYPE_Y8)    sprintf(tmpStr, "Y8");
	else if (type == MEDIASUBTYPE_GREY)  sprintf(tmpStr, "GREY");
	else if (type == MEDIASUBTYPE_I420)  sprintf(tmpStr, "I420");
	else sprintf(tmpStr, "OTHER");

	memcpy(typeAsString, tmpStr, sizeof(char)* 8);
}

int videoInput::getFourccFromMediaSubtype(GUID type) {
	return type.Data1;
}

GUID *videoInput::getMediaSubtypeFromFourcc(int fourcc){

	for (int i = 0; i<VI_NUM_TYPES; i++) {
		if ((unsigned long)(unsigned)fourcc == mediaSubtypes[i].Data1) {
			return &mediaSubtypes[i];
		}
	}

	return NULL;
}

void videoInput::getVideoPropertyAsString(int prop, char * propertyAsString){
	char tmpStr[16];

	if (prop == VideoProcAmp_Brightness) sprintf(tmpStr, "Brightness");
	else if (prop == VideoProcAmp_Contrast) sprintf(tmpStr, "Contrast");
	else if (prop == VideoProcAmp_Saturation) sprintf(tmpStr, "Saturation");
	else if (prop == VideoProcAmp_Hue) sprintf(tmpStr, "Hue");
	else if (prop == VideoProcAmp_Gain) sprintf(tmpStr, "Gain");
	else if (prop == VideoProcAmp_Gamma) sprintf(tmpStr, "Gamma");
	else if (prop == VideoProcAmp_ColorEnable) sprintf(tmpStr, "ColorEnable");
	else if (prop == VideoProcAmp_Sharpness) sprintf(tmpStr, "Sharpness");
	else sprintf(tmpStr, "%u", prop);

	memcpy(propertyAsString, tmpStr, sizeof(char)* 16);
}

int videoInput::getVideoPropertyFromCV(int cv_property){
	// see VideoProcAmpProperty in strmif.h
	switch (cv_property) {
	case CV_CAP_PROP_BRIGHTNESS:
		return VideoProcAmp_Brightness;

	case CV_CAP_PROP_CONTRAST:
		return VideoProcAmp_Contrast;

	case CV_CAP_PROP_HUE:
		return VideoProcAmp_Hue;

	case CV_CAP_PROP_SATURATION:
		return VideoProcAmp_Saturation;

	case CV_CAP_PROP_SHARPNESS:
		return VideoProcAmp_Sharpness;

	case CV_CAP_PROP_GAMMA:
		return VideoProcAmp_Gamma;

	case CV_CAP_PROP_MONOCROME:
		return VideoProcAmp_ColorEnable;

	case CV_CAP_PROP_WHITE_BALANCE_U:
		return VideoProcAmp_WhiteBalance;

	case  CV_CAP_PROP_BACKLIGHT:
		return VideoProcAmp_BacklightCompensation;

	case CV_CAP_PROP_GAIN:
		return VideoProcAmp_Gain;
	}
	return -1;
}

int videoInput::getCameraPropertyFromCV(int cv_property){
	// see CameraControlProperty in strmif.h
	switch (cv_property) {
	case CV_CAP_PROP_PAN:
		return CameraControl_Pan;

	case CV_CAP_PROP_TILT:
		return CameraControl_Tilt;

	case CV_CAP_PROP_ROLL:
		return CameraControl_Roll;

	case CV_CAP_PROP_ZOOM:
		return CameraControl_Zoom;

	case CV_CAP_PROP_EXPOSURE:
		return CameraControl_Exposure;

	case CV_CAP_PROP_IRIS:
		return CameraControl_Iris;

	case CV_CAP_PROP_FOCUS:
		return CameraControl_Focus;
	}
	return -1;
}

void videoInput::getCameraPropertyAsString(int prop, char * propertyAsString){
	char tmpStr[16];

	if (prop == CameraControl_Pan) sprintf(tmpStr, "Pan");
	else if (prop == CameraControl_Tilt) sprintf(tmpStr, "Tilt");
	else if (prop == CameraControl_Roll) sprintf(tmpStr, "Roll");
	else if (prop == CameraControl_Zoom) sprintf(tmpStr, "Zoom");
	else if (prop == CameraControl_Exposure) sprintf(tmpStr, "Exposure");
	else if (prop == CameraControl_Iris) sprintf(tmpStr, "Iris");
	else if (prop == CameraControl_Focus) sprintf(tmpStr, "Focus");
	else sprintf(tmpStr, "%u", prop);

	memcpy(propertyAsString, tmpStr, sizeof(char)* 16);
}

//-------------------------------------------------------------------------------------------
static void findClosestSizeAndSubtype(videoDevice * VD, int widthIn, int heightIn, int &widthOut, int &heightOut, GUID & mediatypeOut){
	HRESULT hr;

	//find perfect match or closest size
	int nearW = 9999999;
	int nearH = 9999999;
	//bool foundClosestMatch     = true;

	int iCount = 0;
	int iSize = 0;
	hr = VD->streamConf->GetNumberOfCapabilities(&iCount, &iSize);

	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		//For each format type RGB24 YUV2 etc
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;
			hr = VD->streamConf->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

			if (SUCCEEDED(hr)){

				//his is how many diff sizes are available for the format
				int stepX = scc.OutputGranularityX;
				int stepY = scc.OutputGranularityY;

				int tempW = 999999;
				int tempH = 999999;

				//Don't want to get stuck in a loop
				if (stepX < 1 || stepY < 1) continue;

				//if(verbose)printf("min is %i %i max is %i %i - res is %i %i \n", scc.MinOutputSize.cx, scc.MinOutputSize.cy,  scc.MaxOutputSize.cx,  scc.MaxOutputSize.cy, stepX, stepY);
				//if(verbose)printf("min frame duration is %i  max duration is %i\n", scc.MinFrameInterval, scc.MaxFrameInterval);

				bool exactMatch = false;
				bool exactMatchX = false;
				bool exactMatchY = false;

				for (int x = scc.MinOutputSize.cx; x <= scc.MaxOutputSize.cx; x += stepX){
					//If we find an exact match
					if (widthIn == x){
						exactMatchX = true;
						tempW = x;
					}
					//Otherwise lets find the closest match based on width
					else if (abs(widthIn - x) < abs(widthIn - tempW)){
						tempW = x;
					}
				}

				for (int y = scc.MinOutputSize.cy; y <= scc.MaxOutputSize.cy; y += stepY){
					//If we find an exact match
					if (heightIn == y){
						exactMatchY = true;
						tempH = y;
					}
					//Otherwise lets find the closest match based on height
					else if (abs(heightIn - y) < abs(heightIn - tempH)){
						tempH = y;
					}
				}

				//see if we have an exact match!
				if (exactMatchX && exactMatchY){
					//foundClosestMatch = false;
					exactMatch = true;

					widthOut = widthIn;
					heightOut = heightIn;
					mediatypeOut = pmtConfig->subtype;
				}

				//otherwise lets see if this filters closest size is the closest
				//available. the closest size is determined by the sum difference
				//of the widths and heights
				else if (abs(widthIn - tempW) + abs(heightIn - tempH)  < abs(widthIn - nearW) + abs(heightIn - nearH))
				{
					nearW = tempW;
					nearH = tempH;

					widthOut = nearW;
					heightOut = nearH;
					mediatypeOut = pmtConfig->subtype;
				}

				MyDeleteMediaType(pmtConfig);

				//If we have found an exact match no need to search anymore
				if (exactMatch)break;
			}
		}
	}

}

//---------------------------------------------------------------------------------------------------
static bool setSizeAndSubtype(videoDevice * VD, int attemptWidth, int attemptHeight, GUID mediatype){
	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(VD->pAmMediaType->pbFormat);

	//store current size
	//int tmpWidth  = HEADER(pVih)->biWidth;
	//int tmpHeight = HEADER(pVih)->biHeight;
	AM_MEDIA_TYPE * tmpType = NULL;

	HRESULT    hr = VD->streamConf->GetFormat(&tmpType);
	if (hr != S_OK)return false;

	//set new size:
	//width and height
	HEADER(pVih)->biWidth = attemptWidth;
	HEADER(pVih)->biHeight = attemptHeight;

	VD->pAmMediaType->formattype = FORMAT_VideoInfo;
	VD->pAmMediaType->majortype = MEDIATYPE_Video;
	VD->pAmMediaType->subtype = mediatype;

	//buffer size
	if (mediatype == MEDIASUBTYPE_RGB24)
	{
		VD->pAmMediaType->lSampleSize = attemptWidth*attemptHeight * 3;
	}
	else
	{
		// For compressed data, the value can be zero.
		VD->pAmMediaType->lSampleSize = 0;
	}

	//set fps if requested
	if (VD->requestedFrameTime != -1){
		pVih->AvgTimePerFrame = VD->requestedFrameTime;
	}

	//okay lets try new size
	hr = VD->streamConf->SetFormat(VD->pAmMediaType);
	if (hr == S_OK){
		if (tmpType != NULL)MyDeleteMediaType(tmpType);
		return true;
	}
	else{
		VD->streamConf->SetFormat(tmpType);
		if (tmpType != NULL)MyDeleteMediaType(tmpType);
	}

	return false;
}

// ----------------------------------------------------------------------
// Where all the work happens!
// Attempts to build a graph for the specified device
// ----------------------------------------------------------------------
int videoInput::start(int deviceID, videoDevice *VD){
	HRESULT hr = NOERROR;
	VD->myID = deviceID;
	VD->setupStarted = true;
	CAPTURE_MODE = PIN_CATEGORY_CAPTURE; //Don't worry - it ends up being preview (which is faster)
	callbackSetCount = 1;  //make sure callback method is not changed after setup called

	if (verbose)printf("SETUP: Setting up device %i\n", deviceID);

	// CREATE THE GRAPH BUILDER //
	// Create the filter graph manager and query for interfaces.
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&VD->pCaptureGraph);
	if (FAILED(hr))    // FAILED is a macro that tests the return value
	{
		if (verbose)printf("ERROR - Could not create the Filter Graph Manager\n");
		return hr;
	}

	//FITLER GRAPH MANAGER//
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&VD->pGraph);
	if (FAILED(hr))
	{
		if (verbose)printf("ERROR - Could not add the graph builder!\n");
		stopDevice(deviceID);
		return hr;
	}

	//SET THE FILTERGRAPH//
	hr = VD->pCaptureGraph->SetFiltergraph(VD->pGraph);
	if (FAILED(hr))
	{
		if (verbose)printf("ERROR - Could not set filtergraph\n");
		stopDevice(deviceID);
		return hr;
	}

	//MEDIA CONTROL (START/STOPS STREAM)//
	// Using QueryInterface on the graph builder,
	// Get the Media Control object.
	hr = VD->pGraph->QueryInterface(IID_IMediaControl, (void **)&VD->pControl);
	if (FAILED(hr))
	{
		if (verbose)printf("ERROR - Could not create the Media Control object\n");
		stopDevice(deviceID);
		return hr;
	}

	//FIND VIDEO DEVICE AND ADD TO GRAPH//
	//gets the device specified by the second argument.
	hr = getDevice(&VD->pVideoInputFilter, deviceID, VD->wDeviceName, VD->nDeviceName);

	if (SUCCEEDED(hr)){
		if (verbose)printf("SETUP: %s\n", VD->nDeviceName);
		hr = VD->pGraph->AddFilter(VD->pVideoInputFilter, VD->wDeviceName);
	}
	else{
		if (verbose)printf("ERROR - Could not find specified video device\n");
		stopDevice(deviceID);
		return hr;
	}

	//LOOK FOR PREVIEW PIN IF THERE IS NONE THEN WE USE CAPTURE PIN AND THEN SMART TEE TO PREVIEW
	IAMStreamConfig *streamConfTest = NULL;
	hr = VD->pCaptureGraph->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, VD->pVideoInputFilter, IID_IAMStreamConfig, (void **)&streamConfTest);
	if (FAILED(hr)){
		if (verbose)printf("SETUP: Couldn't find preview pin using SmartTee\n");
	}
	else{
		CAPTURE_MODE = PIN_CATEGORY_PREVIEW;
		streamConfTest->Release();
		streamConfTest = NULL;
	}

	//CROSSBAR (SELECT PHYSICAL INPUT TYPE)//
	//my own function that checks to see if the device can support a crossbar and if so it routes it.
	//webcams tend not to have a crossbar so this function will also detect a webcams and not apply the crossbar
	if (VD->useCrossbar)
	{
		if (verbose)printf("SETUP: Checking crossbar\n");
		routeCrossbar(&VD->pCaptureGraph, &VD->pVideoInputFilter, VD->connection, CAPTURE_MODE);
	}

	//we do this because webcams don't have a preview mode
	hr = VD->pCaptureGraph->FindInterface(&CAPTURE_MODE, &MEDIATYPE_Video, VD->pVideoInputFilter, IID_IAMStreamConfig, (void **)&VD->streamConf);
	if (FAILED(hr)){
		if (verbose)printf("ERROR: Couldn't config the stream!\n");
		stopDevice(deviceID);
		return hr;
	}

	//NOW LETS DEAL WITH GETTING THE RIGHT SIZE
	hr = VD->streamConf->GetFormat(&VD->pAmMediaType);
	if (FAILED(hr)){
		if (verbose)printf("ERROR: Couldn't getFormat for pAmMediaType!\n");
		stopDevice(deviceID);
		return hr;
	}

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(VD->pAmMediaType->pbFormat);
	int currentWidth = HEADER(pVih)->biWidth;
	int currentHeight = HEADER(pVih)->biHeight;

	bool customSize = VD->tryDiffSize;

	bool foundSize = false;

	if (customSize){
		if (verbose)    printf("SETUP: Default Format is set to %i by %i \n", currentWidth, currentHeight);

		char guidStr[8];
		// try specified format and size
		getMediaSubtypeAsString(VD->tryVideoType, guidStr);
		if (verbose)printf("SETUP: trying specified format %s @ %i by %i\n", guidStr, VD->tryWidth, VD->tryHeight);

		if (setSizeAndSubtype(VD, VD->tryWidth, VD->tryHeight, VD->tryVideoType)){
			VD->setSize(VD->tryWidth, VD->tryHeight);
			VD->videoType = VD->tryVideoType;
			foundSize = true;
		}
		else {
			// try specified size with all formats
			for (int i = 0; i < VI_NUM_TYPES; i++){

				getMediaSubtypeAsString(mediaSubtypes[i], guidStr);

				if (verbose)printf("SETUP: trying format %s @ %i by %i\n", guidStr, VD->tryWidth, VD->tryHeight);
				if (setSizeAndSubtype(VD, VD->tryWidth, VD->tryHeight, mediaSubtypes[i])){
					VD->setSize(VD->tryWidth, VD->tryHeight);
					VD->videoType = mediaSubtypes[i];
					foundSize = true;
					break;
				}
			}
		}

		getCodecAndVideoSize(VD);

		//if we didn't find the requested size - lets try and find the closest matching size
		if (foundSize == false){
			if (verbose)printf("SETUP: couldn't find requested size - searching for closest matching size\n");

			int closestWidth = -1;
			int closestHeight = -1;
			GUID newMediaSubtype;

			findClosestSizeAndSubtype(VD, VD->tryWidth, VD->tryHeight, closestWidth, closestHeight, newMediaSubtype);

			if (closestWidth != -1 && closestHeight != -1){
				getMediaSubtypeAsString(newMediaSubtype, guidStr);

				if (verbose)printf("SETUP: closest supported size is %s @ %i %i\n", guidStr, closestWidth, closestHeight);
				if (setSizeAndSubtype(VD, closestWidth, closestHeight, newMediaSubtype)){
					VD->setSize(closestWidth, closestHeight);
					foundSize = true;
				}
			}
		}
	}

	//if we didn't specify a custom size or if we did but couldn't find it lets setup with the default settings
	if (customSize == false || foundSize == false){
		if (VD->requestedFrameTime != -1){
			pVih->AvgTimePerFrame = VD->requestedFrameTime;
			hr = VD->streamConf->SetFormat(VD->pAmMediaType);
		}
		VD->setSize(currentWidth, currentHeight);
	}

	//SAMPLE GRABBER (ALLOWS US TO GRAB THE BUFFER)//
	// Create the Sample Grabber.
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&VD->pGrabberF);
	if (FAILED(hr)){
		if (verbose)printf("Could not Create Sample Grabber - CoCreateInstance()\n");
		stopDevice(deviceID);
		return hr;
	}

	hr = VD->pGraph->AddFilter(VD->pGrabberF, L"Sample Grabber");
	if (FAILED(hr)){
		if (verbose)printf("Could not add Sample Grabber - AddFilter()\n");
		stopDevice(deviceID);
		return hr;
	}

	hr = VD->pGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&VD->pGrabber);
	if (FAILED(hr)){
		if (verbose)printf("ERROR: Could not query SampleGrabber\n");
		stopDevice(deviceID);
		return hr;
	}

	//Set Params - One Shot should be false unless you want to capture just one buffer
	hr = VD->pGrabber->SetOneShot(FALSE);
	if (bCallback){
		hr = VD->pGrabber->SetBufferSamples(FALSE);
	}
	else{
		hr = VD->pGrabber->SetBufferSamples(TRUE);
	}

	if (bCallback){
		//Tell the grabber to use our callback function - 0 is for SampleCB and 1 for BufferCB
		//We use SampleCB
		hr = VD->pGrabber->SetCallback(VD->sgCallback, 0);
		if (FAILED(hr)){
			if (verbose)printf("ERROR: problem setting callback\n");
			stopDevice(deviceID);
			return hr;
		}
		else{
			if (verbose)printf("SETUP: Capture callback set\n");
		}
	}

	//MEDIA CONVERSION
	//Get video properties from the stream's mediatype and apply to the grabber (otherwise we don't get an RGB image)
	//zero the media type - lets try this :) - maybe this works?
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	mt.formattype = FORMAT_VideoInfo;

	//VD->pAmMediaType->subtype = VD->videoType;
	hr = VD->pGrabber->SetMediaType(&mt);

	//lets try freeing our stream conf here too
	//this will fail if the device is already running
	if (VD->streamConf){
		VD->streamConf->Release();
		VD->streamConf = NULL;
	}
	else{
		if (verbose)printf("ERROR: connecting device - prehaps it is already being used?\n");
		stopDevice(deviceID);
		return S_FALSE;
	}

	//NULL RENDERER//
	//used to give the video stream somewhere to go to.
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&VD->pDestFilter));
	if (FAILED(hr)){
		if (verbose)printf("ERROR: Could not create filter - NullRenderer\n");
		stopDevice(deviceID);
		return hr;
	}

	hr = VD->pGraph->AddFilter(VD->pDestFilter, L"NullRenderer");
	if (FAILED(hr)){
		if (verbose)printf("ERROR: Could not add filter - NullRenderer\n");
		stopDevice(deviceID);
		return hr;
	}

	//RENDER STREAM//
	//This is where the stream gets put together.
	hr = VD->pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, VD->pVideoInputFilter, VD->pGrabberF, VD->pDestFilter);

	if (FAILED(hr)){
		if (verbose)printf("ERROR: Could not connect pins - RenderStream()\n");
		stopDevice(deviceID);
		return hr;
	}

	//EXP - lets try setting the sync source to null - and make it run as fast as possible
	{
		IMediaFilter *pMediaFilter = 0;
		hr = VD->pGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
		if (FAILED(hr)){
			if (verbose)printf("ERROR: Could not get IID_IMediaFilter interface\n");
		}
		else{
			pMediaFilter->SetSyncSource(NULL);
			pMediaFilter->Release();
		}
	}

	//LETS RUN THE STREAM!
	hr = VD->pControl->Run();

	if (FAILED(hr)){
		if (verbose)printf("ERROR: Could not start graph\n");
		stopDevice(deviceID);
		return hr;
	}

	//MAKE SURE THE DEVICE IS SENDING VIDEO BEFORE WE FINISH
	if (!bCallback){

		long bufferSize = VD->videoSize;

		while (hr != S_OK){
			hr = VD->pGrabber->GetCurrentBuffer(&bufferSize, (long *)VD->pBuffer);
			Sleep(10);
		}

	}

	if (verbose)printf("SETUP: Device is setup and ready to capture.\n\n");
	VD->readyToCapture = true;

	//Release filters - seen someone else do this
	//looks like it solved the freezes

	//if we release this then we don't have access to the settings
	//we release our video input filter but then reconnect with it
	//each time we need to use it
	VD->pVideoInputFilter->Release();
	VD->pVideoInputFilter = NULL;

	VD->pGrabberF->Release();
	VD->pGrabberF = NULL;

	VD->pDestFilter->Release();
	VD->pDestFilter = NULL;

	return S_OK;
}

const PixelFormatTag ff_raw_pix_fmt_tags[] = {
	{ AV_PIX_FMT_YUV420P, MKTAG('I', '4', '2', '0') }, /* Planar formats */
	{ AV_PIX_FMT_YUV420P, MKTAG('I', 'Y', 'U', 'V') },
	{ AV_PIX_FMT_YUV420P, MKTAG('y', 'v', '1', '2') },
	{ AV_PIX_FMT_YUV420P, MKTAG('Y', 'V', '1', '2') },
	{ AV_PIX_FMT_YUV410P, MKTAG('Y', 'U', 'V', '9') },
	{ AV_PIX_FMT_YUV410P, MKTAG('Y', 'V', 'U', '9') },
	{ AV_PIX_FMT_YUV411P, MKTAG('Y', '4', '1', 'B') },
	{ AV_PIX_FMT_YUV422P, MKTAG('Y', '4', '2', 'B') },
	{ AV_PIX_FMT_YUV422P, MKTAG('P', '4', '2', '2') },
	{ AV_PIX_FMT_YUV422P, MKTAG('Y', 'V', '1', '6') },
	/* yuvjXXX formats are deprecated hacks specific to libav*,
	they are identical to yuvXXX  */
	{ AV_PIX_FMT_YUVJ420P, MKTAG('I', '4', '2', '0') }, /* Planar formats */
	{ AV_PIX_FMT_YUVJ420P, MKTAG('I', 'Y', 'U', 'V') },
	{ AV_PIX_FMT_YUVJ420P, MKTAG('Y', 'V', '1', '2') },
	{ AV_PIX_FMT_YUVJ422P, MKTAG('Y', '4', '2', 'B') },
	{ AV_PIX_FMT_YUVJ422P, MKTAG('P', '4', '2', '2') },
	{ AV_PIX_FMT_GRAY8, MKTAG('Y', '8', '0', '0') },
	{ AV_PIX_FMT_GRAY8, MKTAG('Y', '8', ' ', ' ') },

	{ AV_PIX_FMT_YUYV422, MKTAG('Y', 'U', 'Y', '2') }, /* Packed formats */
	{ AV_PIX_FMT_YUYV422, MKTAG('Y', '4', '2', '2') },
	{ AV_PIX_FMT_YUYV422, MKTAG('V', '4', '2', '2') },
	{ AV_PIX_FMT_YUYV422, MKTAG('V', 'Y', 'U', 'Y') },
	{ AV_PIX_FMT_YUYV422, MKTAG('Y', 'U', 'N', 'V') },
	{ AV_PIX_FMT_YUYV422, MKTAG('Y', 'U', 'Y', 'V') },
	{ AV_PIX_FMT_YVYU422, MKTAG('Y', 'V', 'Y', 'U') }, /* Philips */
	{ AV_PIX_FMT_UYVY422, MKTAG('U', 'Y', 'V', 'Y') },
	{ AV_PIX_FMT_UYVY422, MKTAG('H', 'D', 'Y', 'C') },
	{ AV_PIX_FMT_UYVY422, MKTAG('U', 'Y', 'N', 'V') },
	{ AV_PIX_FMT_UYVY422, MKTAG('U', 'Y', 'N', 'Y') },
	{ AV_PIX_FMT_UYVY422, MKTAG('u', 'y', 'v', '1') },
	{ AV_PIX_FMT_UYVY422, MKTAG('2', 'V', 'u', '1') },
	{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', 'R', 'n') }, /* Avid AVI Codec 1:1 */
	{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', '1', 'x') }, /* Avid 1:1x */
	{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', 'u', 'p') },
	{ AV_PIX_FMT_UYVY422, MKTAG('V', 'D', 'T', 'Z') }, /* SoftLab-NSK VideoTizer */
	{ AV_PIX_FMT_UYVY422, MKTAG('a', 'u', 'v', '2') },
	{ AV_PIX_FMT_UYVY422, MKTAG('c', 'y', 'u', 'v') }, /* CYUV is also Creative YUV */
	{ AV_PIX_FMT_UYYVYY411, MKTAG('Y', '4', '1', '1') },
	{ AV_PIX_FMT_GRAY8, MKTAG('G', 'R', 'E', 'Y') },
	{ AV_PIX_FMT_NV12, MKTAG('N', 'V', '1', '2') },
	{ AV_PIX_FMT_NV21, MKTAG('N', 'V', '2', '1') },

	/* nut */
	{ AV_PIX_FMT_RGB555LE, MKTAG('R', 'G', 'B', 15) },
	{ AV_PIX_FMT_BGR555LE, MKTAG('B', 'G', 'R', 15) },
	{ AV_PIX_FMT_RGB565LE, MKTAG('R', 'G', 'B', 16) },
	{ AV_PIX_FMT_BGR565LE, MKTAG('B', 'G', 'R', 16) },
	{ AV_PIX_FMT_RGB555BE, MKTAG(15, 'B', 'G', 'R') },
	{ AV_PIX_FMT_BGR555BE, MKTAG(15, 'R', 'G', 'B') },
	{ AV_PIX_FMT_RGB565BE, MKTAG(16, 'B', 'G', 'R') },
	{ AV_PIX_FMT_BGR565BE, MKTAG(16, 'R', 'G', 'B') },
	{ AV_PIX_FMT_RGB444LE, MKTAG('R', 'G', 'B', 12) },
	{ AV_PIX_FMT_BGR444LE, MKTAG('B', 'G', 'R', 12) },
	{ AV_PIX_FMT_RGB444BE, MKTAG(12, 'B', 'G', 'R') },
	{ AV_PIX_FMT_BGR444BE, MKTAG(12, 'R', 'G', 'B') },
	{ AV_PIX_FMT_RGBA64LE, MKTAG('R', 'B', 'A', 64) },
	{ AV_PIX_FMT_BGRA64LE, MKTAG('B', 'R', 'A', 64) },
	{ AV_PIX_FMT_RGBA64BE, MKTAG(64, 'R', 'B', 'A') },
	{ AV_PIX_FMT_BGRA64BE, MKTAG(64, 'B', 'R', 'A') },
	{ AV_PIX_FMT_RGBA, MKTAG('R', 'G', 'B', 'A') },
	{ AV_PIX_FMT_RGB0, MKTAG('R', 'G', 'B', 0) },
	{ AV_PIX_FMT_BGRA, MKTAG('B', 'G', 'R', 'A') },
	{ AV_PIX_FMT_BGR0, MKTAG('B', 'G', 'R', 0) },
	{ AV_PIX_FMT_ABGR, MKTAG('A', 'B', 'G', 'R') },
	{ AV_PIX_FMT_0BGR, MKTAG(0, 'B', 'G', 'R') },
	{ AV_PIX_FMT_ARGB, MKTAG('A', 'R', 'G', 'B') },
	{ AV_PIX_FMT_0RGB, MKTAG(0, 'R', 'G', 'B') },
	{ AV_PIX_FMT_RGB24, MKTAG('R', 'G', 'B', 24) },
	{ AV_PIX_FMT_BGR24, MKTAG('B', 'G', 'R', 24) },
	{ AV_PIX_FMT_YUV411P, MKTAG('4', '1', '1', 'P') },
	{ AV_PIX_FMT_YUV422P, MKTAG('4', '2', '2', 'P') },
	{ AV_PIX_FMT_YUVJ422P, MKTAG('4', '2', '2', 'P') },
	{ AV_PIX_FMT_YUV440P, MKTAG('4', '4', '0', 'P') },
	{ AV_PIX_FMT_YUVJ440P, MKTAG('4', '4', '0', 'P') },
	{ AV_PIX_FMT_YUV444P, MKTAG('4', '4', '4', 'P') },
	{ AV_PIX_FMT_YUVJ444P, MKTAG('4', '4', '4', 'P') },
	{ AV_PIX_FMT_MONOWHITE, MKTAG('B', '1', 'W', '0') },
	{ AV_PIX_FMT_MONOBLACK, MKTAG('B', '0', 'W', '1') },
	{ AV_PIX_FMT_BGR8, MKTAG('B', 'G', 'R', 8) },
	{ AV_PIX_FMT_RGB8, MKTAG('R', 'G', 'B', 8) },
	{ AV_PIX_FMT_BGR4, MKTAG('B', 'G', 'R', 4) },
	{ AV_PIX_FMT_RGB4, MKTAG('R', 'G', 'B', 4) },
	{ AV_PIX_FMT_RGB4_BYTE, MKTAG('B', '4', 'B', 'Y') },
	{ AV_PIX_FMT_BGR4_BYTE, MKTAG('R', '4', 'B', 'Y') },
	{ AV_PIX_FMT_RGB48LE, MKTAG('R', 'G', 'B', 48) },
	{ AV_PIX_FMT_RGB48BE, MKTAG(48, 'R', 'G', 'B') },
	{ AV_PIX_FMT_BGR48LE, MKTAG('B', 'G', 'R', 48) },
	{ AV_PIX_FMT_BGR48BE, MKTAG(48, 'B', 'G', 'R') },
	{ AV_PIX_FMT_GRAY9LE, MKTAG('Y', '1', 0, 9) },
	{ AV_PIX_FMT_GRAY9BE, MKTAG(9, 0, '1', 'Y') },
	{ AV_PIX_FMT_GRAY10LE, MKTAG('Y', '1', 0, 10) },
	{ AV_PIX_FMT_GRAY10BE, MKTAG(10, 0, '1', 'Y') },
	{ AV_PIX_FMT_GRAY12LE, MKTAG('Y', '1', 0, 12) },
	{ AV_PIX_FMT_GRAY12BE, MKTAG(12, 0, '1', 'Y') },
	{ AV_PIX_FMT_GRAY14LE, MKTAG('Y', '1', 0, 14) },
	{ AV_PIX_FMT_GRAY14BE, MKTAG(14, 0, '1', 'Y') },
	{ AV_PIX_FMT_GRAY16LE, MKTAG('Y', '1', 0, 16) },
	{ AV_PIX_FMT_GRAY16BE, MKTAG(16, 0, '1', 'Y') },
	{ AV_PIX_FMT_YUV420P9LE, MKTAG('Y', '3', 11, 9) },
	{ AV_PIX_FMT_YUV420P9BE, MKTAG(9, 11, '3', 'Y') },
	{ AV_PIX_FMT_YUV422P9LE, MKTAG('Y', '3', 10, 9) },
	{ AV_PIX_FMT_YUV422P9BE, MKTAG(9, 10, '3', 'Y') },
	{ AV_PIX_FMT_YUV444P9LE, MKTAG('Y', '3', 0, 9) },
	{ AV_PIX_FMT_YUV444P9BE, MKTAG(9, 0, '3', 'Y') },
	{ AV_PIX_FMT_YUV420P10LE, MKTAG('Y', '3', 11, 10) },
	{ AV_PIX_FMT_YUV420P10BE, MKTAG(10, 11, '3', 'Y') },
	{ AV_PIX_FMT_YUV422P10LE, MKTAG('Y', '3', 10, 10) },
	{ AV_PIX_FMT_YUV422P10BE, MKTAG(10, 10, '3', 'Y') },
	{ AV_PIX_FMT_YUV444P10LE, MKTAG('Y', '3', 0, 10) },
	{ AV_PIX_FMT_YUV444P10BE, MKTAG(10, 0, '3', 'Y') },
	{ AV_PIX_FMT_YUV420P12LE, MKTAG('Y', '3', 11, 12) },
	{ AV_PIX_FMT_YUV420P12BE, MKTAG(12, 11, '3', 'Y') },
	{ AV_PIX_FMT_YUV422P12LE, MKTAG('Y', '3', 10, 12) },
	{ AV_PIX_FMT_YUV422P12BE, MKTAG(12, 10, '3', 'Y') },
	{ AV_PIX_FMT_YUV444P12LE, MKTAG('Y', '3', 0, 12) },
	{ AV_PIX_FMT_YUV444P12BE, MKTAG(12, 0, '3', 'Y') },
	{ AV_PIX_FMT_YUV420P14LE, MKTAG('Y', '3', 11, 14) },
	{ AV_PIX_FMT_YUV420P14BE, MKTAG(14, 11, '3', 'Y') },
	{ AV_PIX_FMT_YUV422P14LE, MKTAG('Y', '3', 10, 14) },
	{ AV_PIX_FMT_YUV422P14BE, MKTAG(14, 10, '3', 'Y') },
	{ AV_PIX_FMT_YUV444P14LE, MKTAG('Y', '3', 0, 14) },
	{ AV_PIX_FMT_YUV444P14BE, MKTAG(14, 0, '3', 'Y') },
	{ AV_PIX_FMT_YUV420P16LE, MKTAG('Y', '3', 11, 16) },
	{ AV_PIX_FMT_YUV420P16BE, MKTAG(16, 11, '3', 'Y') },
	{ AV_PIX_FMT_YUV422P16LE, MKTAG('Y', '3', 10, 16) },
	{ AV_PIX_FMT_YUV422P16BE, MKTAG(16, 10, '3', 'Y') },
	{ AV_PIX_FMT_YUV444P16LE, MKTAG('Y', '3', 0, 16) },
	{ AV_PIX_FMT_YUV444P16BE, MKTAG(16, 0, '3', 'Y') },
	{ AV_PIX_FMT_YUVA420P, MKTAG('Y', '4', 11, 8) },
	{ AV_PIX_FMT_YUVA422P, MKTAG('Y', '4', 10, 8) },
	{ AV_PIX_FMT_YUVA444P, MKTAG('Y', '4', 0, 8) },
	{ AV_PIX_FMT_YA8, MKTAG('Y', '2', 0, 8) },
	{ AV_PIX_FMT_PAL8, MKTAG('P', 'A', 'L', 8) },

	{ AV_PIX_FMT_YUVA420P9LE, MKTAG('Y', '4', 11, 9) },
	{ AV_PIX_FMT_YUVA420P9BE, MKTAG(9, 11, '4', 'Y') },
	{ AV_PIX_FMT_YUVA422P9LE, MKTAG('Y', '4', 10, 9) },
	{ AV_PIX_FMT_YUVA422P9BE, MKTAG(9, 10, '4', 'Y') },
	{ AV_PIX_FMT_YUVA444P9LE, MKTAG('Y', '4', 0, 9) },
	{ AV_PIX_FMT_YUVA444P9BE, MKTAG(9, 0, '4', 'Y') },
	{ AV_PIX_FMT_YUVA420P10LE, MKTAG('Y', '4', 11, 10) },
	{ AV_PIX_FMT_YUVA420P10BE, MKTAG(10, 11, '4', 'Y') },
	{ AV_PIX_FMT_YUVA422P10LE, MKTAG('Y', '4', 10, 10) },
	{ AV_PIX_FMT_YUVA422P10BE, MKTAG(10, 10, '4', 'Y') },
	{ AV_PIX_FMT_YUVA444P10LE, MKTAG('Y', '4', 0, 10) },
	{ AV_PIX_FMT_YUVA444P10BE, MKTAG(10, 0, '4', 'Y') },
	{ AV_PIX_FMT_YUVA420P16LE, MKTAG('Y', '4', 11, 16) },
	{ AV_PIX_FMT_YUVA420P16BE, MKTAG(16, 11, '4', 'Y') },
	{ AV_PIX_FMT_YUVA422P16LE, MKTAG('Y', '4', 10, 16) },
	{ AV_PIX_FMT_YUVA422P16BE, MKTAG(16, 10, '4', 'Y') },
	{ AV_PIX_FMT_YUVA444P16LE, MKTAG('Y', '4', 0, 16) },
	{ AV_PIX_FMT_YUVA444P16BE, MKTAG(16, 0, '4', 'Y') },

	{ AV_PIX_FMT_GBRP, MKTAG('G', '3', 00, 8) },
	{ AV_PIX_FMT_GBRP9LE, MKTAG('G', '3', 00, 9) },
	{ AV_PIX_FMT_GBRP9BE, MKTAG(9, 00, '3', 'G') },
	{ AV_PIX_FMT_GBRP10LE, MKTAG('G', '3', 00, 10) },
	{ AV_PIX_FMT_GBRP10BE, MKTAG(10, 00, '3', 'G') },
	{ AV_PIX_FMT_GBRP12LE, MKTAG('G', '3', 00, 12) },
	{ AV_PIX_FMT_GBRP12BE, MKTAG(12, 00, '3', 'G') },
	{ AV_PIX_FMT_GBRP14LE, MKTAG('G', '3', 00, 14) },
	{ AV_PIX_FMT_GBRP14BE, MKTAG(14, 00, '3', 'G') },
	{ AV_PIX_FMT_GBRP16LE, MKTAG('G', '3', 00, 16) },
	{ AV_PIX_FMT_GBRP16BE, MKTAG(16, 00, '3', 'G') },

	{ AV_PIX_FMT_GBRAP, MKTAG('G', '4', 00, 8) },
	{ AV_PIX_FMT_GBRAP10LE, MKTAG('G', '4', 00, 10) },
	{ AV_PIX_FMT_GBRAP10BE, MKTAG(10, 00, '4', 'G') },
	{ AV_PIX_FMT_GBRAP12LE, MKTAG('G', '4', 00, 12) },
	{ AV_PIX_FMT_GBRAP12BE, MKTAG(12, 00, '4', 'G') },
	{ AV_PIX_FMT_GBRAP16LE, MKTAG('G', '4', 00, 16) },
	{ AV_PIX_FMT_GBRAP16BE, MKTAG(16, 00, '4', 'G') },

	{ AV_PIX_FMT_XYZ12LE, MKTAG('X', 'Y', 'Z', 36) },
	{ AV_PIX_FMT_XYZ12BE, MKTAG(36, 'Z', 'Y', 'X') },

	{ AV_PIX_FMT_BAYER_BGGR8, MKTAG(0xBA, 'B', 'G', 8) },
	{ AV_PIX_FMT_BAYER_BGGR16LE, MKTAG(0xBA, 'B', 'G', 16) },
	{ AV_PIX_FMT_BAYER_BGGR16BE, MKTAG(16, 'G', 'B', 0xBA) },
	{ AV_PIX_FMT_BAYER_RGGB8, MKTAG(0xBA, 'R', 'G', 8) },
	{ AV_PIX_FMT_BAYER_RGGB16LE, MKTAG(0xBA, 'R', 'G', 16) },
	{ AV_PIX_FMT_BAYER_RGGB16BE, MKTAG(16, 'G', 'R', 0xBA) },
	{ AV_PIX_FMT_BAYER_GBRG8, MKTAG(0xBA, 'G', 'B', 8) },
	{ AV_PIX_FMT_BAYER_GBRG16LE, MKTAG(0xBA, 'G', 'B', 16) },
	{ AV_PIX_FMT_BAYER_GBRG16BE, MKTAG(16, 'B', 'G', 0xBA) },
	{ AV_PIX_FMT_BAYER_GRBG8, MKTAG(0xBA, 'G', 'R', 8) },
	{ AV_PIX_FMT_BAYER_GRBG16LE, MKTAG(0xBA, 'G', 'R', 16) },
	{ AV_PIX_FMT_BAYER_GRBG16BE, MKTAG(16, 'R', 'G', 0xBA) },

	/* quicktime */
	{ AV_PIX_FMT_YUV420P, MKTAG('R', '4', '2', '0') }, /* Radius DV YUV PAL */
	{ AV_PIX_FMT_YUV411P, MKTAG('R', '4', '1', '1') }, /* Radius DV YUV NTSC */
	{ AV_PIX_FMT_UYVY422, MKTAG('2', 'v', 'u', 'y') },
	{ AV_PIX_FMT_UYVY422, MKTAG('2', 'V', 'u', 'y') },
	{ AV_PIX_FMT_UYVY422, MKTAG('A', 'V', 'U', 'I') }, /* FIXME merge both fields */
	{ AV_PIX_FMT_UYVY422, MKTAG('b', 'x', 'y', 'v') },
	{ AV_PIX_FMT_YUYV422, MKTAG('y', 'u', 'v', '2') },
	{ AV_PIX_FMT_YUYV422, MKTAG('y', 'u', 'v', 's') },
	{ AV_PIX_FMT_YUYV422, MKTAG('D', 'V', 'O', 'O') }, /* Digital Voodoo SD 8 Bit */
	{ AV_PIX_FMT_RGB555LE, MKTAG('L', '5', '5', '5') },
	{ AV_PIX_FMT_RGB565LE, MKTAG('L', '5', '6', '5') },
	{ AV_PIX_FMT_RGB565BE, MKTAG('B', '5', '6', '5') },
	{ AV_PIX_FMT_BGR24, MKTAG('2', '4', 'B', 'G') },
	{ AV_PIX_FMT_BGR24, MKTAG('b', 'x', 'b', 'g') },
	{ AV_PIX_FMT_BGRA, MKTAG('B', 'G', 'R', 'A') },
	{ AV_PIX_FMT_RGBA, MKTAG('R', 'G', 'B', 'A') },
	{ AV_PIX_FMT_RGB24, MKTAG('b', 'x', 'r', 'g') },
	{ AV_PIX_FMT_ABGR, MKTAG('A', 'B', 'G', 'R') },
	{ AV_PIX_FMT_GRAY16BE, MKTAG('b', '1', '6', 'g') },
	{ AV_PIX_FMT_RGB48BE, MKTAG('b', '4', '8', 'r') },
	{ AV_PIX_FMT_RGBA64BE, MKTAG('b', '6', '4', 'a') },

	/* vlc */
	{ AV_PIX_FMT_YUV410P, MKTAG('I', '4', '1', '0') },
	{ AV_PIX_FMT_YUV411P, MKTAG('I', '4', '1', '1') },
	{ AV_PIX_FMT_YUV422P, MKTAG('I', '4', '2', '2') },
	{ AV_PIX_FMT_YUV440P, MKTAG('I', '4', '4', '0') },
	{ AV_PIX_FMT_YUV444P, MKTAG('I', '4', '4', '4') },
	{ AV_PIX_FMT_YUVJ420P, MKTAG('J', '4', '2', '0') },
	{ AV_PIX_FMT_YUVJ422P, MKTAG('J', '4', '2', '2') },
	{ AV_PIX_FMT_YUVJ440P, MKTAG('J', '4', '4', '0') },
	{ AV_PIX_FMT_YUVJ444P, MKTAG('J', '4', '4', '4') },
	{ AV_PIX_FMT_YUVA444P, MKTAG('Y', 'U', 'V', 'A') },
	{ AV_PIX_FMT_YUVA420P, MKTAG('I', '4', '0', 'A') },
	{ AV_PIX_FMT_YUVA422P, MKTAG('I', '4', '2', 'A') },
	{ AV_PIX_FMT_RGB8, MKTAG('R', 'G', 'B', '2') },
	{ AV_PIX_FMT_RGB555LE, MKTAG('R', 'V', '1', '5') },
	{ AV_PIX_FMT_RGB565LE, MKTAG('R', 'V', '1', '6') },
	{ AV_PIX_FMT_BGR24, MKTAG('R', 'V', '2', '4') },
	{ AV_PIX_FMT_BGR0, MKTAG('R', 'V', '3', '2') },
	{ AV_PIX_FMT_RGBA, MKTAG('A', 'V', '3', '2') },
	{ AV_PIX_FMT_YUV420P9LE, MKTAG('I', '0', '9', 'L') },
	{ AV_PIX_FMT_YUV420P9BE, MKTAG('I', '0', '9', 'B') },
	{ AV_PIX_FMT_YUV422P9LE, MKTAG('I', '2', '9', 'L') },
	{ AV_PIX_FMT_YUV422P9BE, MKTAG('I', '2', '9', 'B') },
	{ AV_PIX_FMT_YUV444P9LE, MKTAG('I', '4', '9', 'L') },
	{ AV_PIX_FMT_YUV444P9BE, MKTAG('I', '4', '9', 'B') },
	{ AV_PIX_FMT_YUV420P10LE, MKTAG('I', '0', 'A', 'L') },
	{ AV_PIX_FMT_YUV420P10BE, MKTAG('I', '0', 'A', 'B') },
	{ AV_PIX_FMT_YUV422P10LE, MKTAG('I', '2', 'A', 'L') },
	{ AV_PIX_FMT_YUV422P10BE, MKTAG('I', '2', 'A', 'B') },
	{ AV_PIX_FMT_YUV444P10LE, MKTAG('I', '4', 'A', 'L') },
	{ AV_PIX_FMT_YUV444P10BE, MKTAG('I', '4', 'A', 'B') },
	{ AV_PIX_FMT_YUV420P12LE, MKTAG('I', '0', 'C', 'L') },
	{ AV_PIX_FMT_YUV420P12BE, MKTAG('I', '0', 'C', 'B') },
	{ AV_PIX_FMT_YUV422P12LE, MKTAG('I', '2', 'C', 'L') },
	{ AV_PIX_FMT_YUV422P12BE, MKTAG('I', '2', 'C', 'B') },
	{ AV_PIX_FMT_YUV444P12LE, MKTAG('I', '4', 'C', 'L') },
	{ AV_PIX_FMT_YUV444P12BE, MKTAG('I', '4', 'C', 'B') },
	{ AV_PIX_FMT_YUV420P16LE, MKTAG('I', '0', 'F', 'L') },
	{ AV_PIX_FMT_YUV420P16BE, MKTAG('I', '0', 'F', 'B') },
	{ AV_PIX_FMT_YUV444P16LE, MKTAG('I', '4', 'F', 'L') },
	{ AV_PIX_FMT_YUV444P16BE, MKTAG('I', '4', 'F', 'B') },

	/* special */
	{ AV_PIX_FMT_RGB565LE, MKTAG(3, 0, 0, 0) }, /* flipped RGB565LE */
	{ AV_PIX_FMT_YUV444P, MKTAG('Y', 'V', '2', '4') }, /* YUV444P, swapped UV */

	{ AV_PIX_FMT_NONE, 0 },
};

const struct PixelFormatTag *avpriv_get_raw_pix_fmt_tags(void)
{
	return ff_raw_pix_fmt_tags;
}

enum AVPixelFormat avpriv_find_pix_fmt(const PixelFormatTag *tags, unsigned int fourcc)
{
	while (tags->pix_fmt >= 0) {
		if (tags->fourcc == fourcc)
			return tags->pix_fmt;
		tags++;
	}
	return AV_PIX_FMT_NONE;
}

static enum AVPixelFormat dshow_pixfmt(DWORD biCompression, WORD biBitCount)
{
	switch (biCompression) {
	case BI_BITFIELDS:
	case BI_RGB:
		switch (biBitCount) { /* 1-8 are untested */
		case 1:
			return AV_PIX_FMT_MONOWHITE;
		case 4:
			return AV_PIX_FMT_RGB4;
		case 8:
			return AV_PIX_FMT_RGB8;
		case 16:
			return AV_PIX_FMT_RGB555;
		case 24:
			return AV_PIX_FMT_BGR24;
		case 32:
			return AV_PIX_FMT_0RGB32;
		}
	}
	return avpriv_find_pix_fmt(avpriv_get_raw_pix_fmt_tags(), biCompression); // all others
}

const AVCodecTag ff_codec_bmp_tags[] = {
	{ AV_CODEC_ID_H264, MKTAG('H', '2', '6', '4') },
	{ AV_CODEC_ID_H264, MKTAG('h', '2', '6', '4') },
	{ AV_CODEC_ID_H264, MKTAG('X', '2', '6', '4') },
	{ AV_CODEC_ID_H264, MKTAG('x', '2', '6', '4') },
	{ AV_CODEC_ID_H264, MKTAG('a', 'v', 'c', '1') },
	{ AV_CODEC_ID_H264, MKTAG('D', 'A', 'V', 'C') },
	{ AV_CODEC_ID_H264, MKTAG('S', 'M', 'V', '2') },
	{ AV_CODEC_ID_H264, MKTAG('V', 'S', 'S', 'H') },
	{ AV_CODEC_ID_H264, MKTAG('Q', '2', '6', '4') }, /* QNAP surveillance system */
	{ AV_CODEC_ID_H264, MKTAG('V', '2', '6', '4') }, /* CCTV recordings */
	{ AV_CODEC_ID_H264, MKTAG('G', 'A', 'V', 'C') }, /* GeoVision camera */
	{ AV_CODEC_ID_H264, MKTAG('U', 'M', 'S', 'V') },
	{ AV_CODEC_ID_H264, MKTAG('t', 's', 'h', 'd') },
	{ AV_CODEC_ID_H264, MKTAG('I', 'N', 'M', 'C') },
	{ AV_CODEC_ID_H263, MKTAG('H', '2', '6', '3') },
	{ AV_CODEC_ID_H263, MKTAG('X', '2', '6', '3') },
	{ AV_CODEC_ID_H263, MKTAG('T', '2', '6', '3') },
	{ AV_CODEC_ID_H263, MKTAG('L', '2', '6', '3') },
	{ AV_CODEC_ID_H263, MKTAG('V', 'X', '1', 'K') },
	{ AV_CODEC_ID_H263, MKTAG('Z', 'y', 'G', 'o') },
	{ AV_CODEC_ID_H263, MKTAG('M', '2', '6', '3') },
	{ AV_CODEC_ID_H263, MKTAG('l', 's', 'v', 'm') },
	{ AV_CODEC_ID_H263P, MKTAG('H', '2', '6', '3') },
	{ AV_CODEC_ID_H263I, MKTAG('I', '2', '6', '3') }, /* Intel H.263 */
	{ AV_CODEC_ID_H261, MKTAG('H', '2', '6', '1') },
	{ AV_CODEC_ID_H263, MKTAG('U', '2', '6', '3') },
	{ AV_CODEC_ID_H263, MKTAG('V', 'S', 'M', '4') }, /* needs -vf il=l=i:c=i */
	{ AV_CODEC_ID_MPEG4, MKTAG('F', 'M', 'P', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', 'X') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'X', '5', '0') },
	{ AV_CODEC_ID_MPEG4, MKTAG('X', 'V', 'I', 'D') },
	{ AV_CODEC_ID_MPEG4, MKTAG('M', 'P', '4', 'S') },
	{ AV_CODEC_ID_MPEG4, MKTAG('M', '4', 'S', '2') },
	/* some broken AVIs use this */
	{ AV_CODEC_ID_MPEG4, MKTAG(4, 0, 0, 0) },
	/* some broken AVIs use this */
	{ AV_CODEC_ID_MPEG4, MKTAG('Z', 'M', 'P', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', '1') },
	{ AV_CODEC_ID_MPEG4, MKTAG('B', 'L', 'Z', '0') },
	{ AV_CODEC_ID_MPEG4, MKTAG('m', 'p', '4', 'v') },
	{ AV_CODEC_ID_MPEG4, MKTAG('U', 'M', 'P', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('W', 'V', '1', 'F') },
	{ AV_CODEC_ID_MPEG4, MKTAG('S', 'E', 'D', 'G') },
	{ AV_CODEC_ID_MPEG4, MKTAG('R', 'M', 'P', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('3', 'I', 'V', '2') },
	/* WaWv MPEG-4 Video Codec */
	{ AV_CODEC_ID_MPEG4, MKTAG('W', 'A', 'W', 'V') },
	{ AV_CODEC_ID_MPEG4, MKTAG('F', 'F', 'D', 'S') },
	{ AV_CODEC_ID_MPEG4, MKTAG('F', 'V', 'F', 'W') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'C', 'O', 'D') },
	{ AV_CODEC_ID_MPEG4, MKTAG('M', 'V', 'X', 'M') },
	{ AV_CODEC_ID_MPEG4, MKTAG('P', 'M', '4', 'V') },
	{ AV_CODEC_ID_MPEG4, MKTAG('S', 'M', 'P', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'X', 'G', 'M') },
	{ AV_CODEC_ID_MPEG4, MKTAG('V', 'I', 'D', 'M') },
	{ AV_CODEC_ID_MPEG4, MKTAG('M', '4', 'T', '3') },
	{ AV_CODEC_ID_MPEG4, MKTAG('G', 'E', 'O', 'X') },
	/* flipped video */
	{ AV_CODEC_ID_MPEG4, MKTAG('G', '2', '6', '4') },
	/* flipped video */
	{ AV_CODEC_ID_MPEG4, MKTAG('H', 'D', 'X', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'M', '4', 'V') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'M', 'K', '2') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'Y', 'M', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'I', 'G', 'I') },
	/* Ephv MPEG-4 */
	{ AV_CODEC_ID_MPEG4, MKTAG('E', 'P', 'H', 'V') },
	{ AV_CODEC_ID_MPEG4, MKTAG('E', 'M', '4', 'A') },
	/* Divio MPEG-4 */
	{ AV_CODEC_ID_MPEG4, MKTAG('M', '4', 'C', 'C') },
	{ AV_CODEC_ID_MPEG4, MKTAG('S', 'N', '4', '0') },
	{ AV_CODEC_ID_MPEG4, MKTAG('V', 'S', 'P', 'X') },
	{ AV_CODEC_ID_MPEG4, MKTAG('U', 'L', 'D', 'X') },
	{ AV_CODEC_ID_MPEG4, MKTAG('G', 'E', 'O', 'V') },
	/* Samsung SHR-6040 */
	{ AV_CODEC_ID_MPEG4, MKTAG('S', 'I', 'P', 'P') },
	{ AV_CODEC_ID_MPEG4, MKTAG('S', 'M', '4', 'V') },
	{ AV_CODEC_ID_MPEG4, MKTAG('X', 'V', 'I', 'X') },
	{ AV_CODEC_ID_MPEG4, MKTAG('D', 'r', 'e', 'X') },
	{ AV_CODEC_ID_MPEG4, MKTAG('Q', 'M', 'P', '4') }, /* QNAP Systems */
	{ AV_CODEC_ID_MPEG4, MKTAG('P', 'L', 'V', '1') }, /* Pelco DVR MPEG-4 */
	{ AV_CODEC_ID_MPEG4, MKTAG('G', 'L', 'V', '4') },
	{ AV_CODEC_ID_MPEG4, MKTAG('G', 'M', 'P', '4') }, /* GeoVision camera */
	{ AV_CODEC_ID_MPEG4, MKTAG('M', 'N', 'M', '4') }, /* March Networks DVR */
	{ AV_CODEC_ID_MPEG4, MKTAG('G', 'T', 'M', '4') }, /* Telefactor */
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('M', 'P', '4', '3') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '3') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('M', 'P', 'G', '3') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '5') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '6') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '4') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('D', 'V', 'X', '3') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('A', 'P', '4', '1') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('C', 'O', 'L', '1') },
	{ AV_CODEC_ID_MSMPEG4V3, MKTAG('C', 'O', 'L', '0') },
	{ AV_CODEC_ID_MSMPEG4V2, MKTAG('M', 'P', '4', '2') },
	{ AV_CODEC_ID_MSMPEG4V2, MKTAG('D', 'I', 'V', '2') },
	{ AV_CODEC_ID_MSMPEG4V1, MKTAG('M', 'P', 'G', '4') },
	{ AV_CODEC_ID_MSMPEG4V1, MKTAG('M', 'P', '4', '1') },
	{ AV_CODEC_ID_WMV1, MKTAG('W', 'M', 'V', '1') },
	{ AV_CODEC_ID_WMV2, MKTAG('W', 'M', 'V', '2') },
	{ AV_CODEC_ID_WMV2, MKTAG('G', 'X', 'V', 'E') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 's', 'd') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', 'd') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '1') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 's', 'l') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', '2', '5') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', '5', '0') },
	/* Canopus DV */
	{ AV_CODEC_ID_DVVIDEO, MKTAG('c', 'd', 'v', 'c') },
	/* Canopus DV */
	{ AV_CODEC_ID_DVVIDEO, MKTAG('C', 'D', 'V', 'H') },
	/* Canopus DV */
	{ AV_CODEC_ID_DVVIDEO, MKTAG('C', 'D', 'V', '5') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'c', ' ') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'c', 's') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '1') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'i', 's') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('p', 'd', 'v', 'c') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('S', 'L', '2', '5') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('S', 'L', 'D', 'V') },
	{ AV_CODEC_ID_DVVIDEO, MKTAG('A', 'V', 'd', '1') },
	{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('m', 'p', 'g', '1') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'p', 'g', '2') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', 'P', 'E', 'G') },
	{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('P', 'I', 'M', '1') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('P', 'I', 'M', '2') },
	{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('V', 'C', 'R', '2') },
	{ AV_CODEC_ID_MPEG1VIDEO, MKTAG(1, 0, 0, 16) },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG(2, 0, 0, 16) },
	{ AV_CODEC_ID_MPEG4, MKTAG(4, 0, 0, 16) },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('D', 'V', 'R', ' ') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', 'M', 'E', 'S') },
	/* Lead MPEG-2 in AVI */
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('L', 'M', 'P', '2') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('s', 'l', 'i', 'f') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('E', 'M', '2', 'V') },
	/* Matrox MPEG-2 intra-only */
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', '7', '0', '1') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', '7', '0', '2') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', '7', '0', '3') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', '7', '0', '4') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('M', '7', '0', '5') },
	{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'p', 'g', 'v') },
	{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('B', 'W', '1', '0') },
	{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('X', 'M', 'P', 'G') }, /* Xing MPEG intra only */
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'J', 'P', 'G') },
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'S', 'C', '2') }, /* Multiscope II */
	{ AV_CODEC_ID_MJPEG, MKTAG('L', 'J', 'P', 'G') },
	{ AV_CODEC_ID_MJPEG, MKTAG('d', 'm', 'b', '1') },
	{ AV_CODEC_ID_MJPEG, MKTAG('m', 'j', 'p', 'a') },
	{ AV_CODEC_ID_MJPEG, MKTAG('J', 'R', '2', '4') }, /* Quadrox Mjpeg */
	{ AV_CODEC_ID_LJPEG, MKTAG('L', 'J', 'P', 'G') },
	/* Pegasus lossless JPEG */
	{ AV_CODEC_ID_MJPEG, MKTAG('J', 'P', 'G', 'L') },
	/* JPEG-LS custom FOURCC for AVI - encoder */
	{ AV_CODEC_ID_JPEGLS, MKTAG('M', 'J', 'L', 'S') },
	{ AV_CODEC_ID_JPEGLS, MKTAG('M', 'J', 'P', 'G') },
	/* JPEG-LS custom FOURCC for AVI - decoder */
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'J', 'L', 'S') },
	{ AV_CODEC_ID_MJPEG, MKTAG('j', 'p', 'e', 'g') },
	{ AV_CODEC_ID_MJPEG, MKTAG('I', 'J', 'P', 'G') },
	{ AV_CODEC_ID_AVRN, MKTAG('A', 'V', 'R', 'n') },
	{ AV_CODEC_ID_MJPEG, MKTAG('A', 'C', 'D', 'V') },
	{ AV_CODEC_ID_MJPEG, MKTAG('Q', 'I', 'V', 'G') },
	/* SL M-JPEG */
	{ AV_CODEC_ID_MJPEG, MKTAG('S', 'L', 'M', 'J') },
	/* Creative Webcam JPEG */
	{ AV_CODEC_ID_MJPEG, MKTAG('C', 'J', 'P', 'G') },
	/* Intel JPEG Library Video Codec */
	{ AV_CODEC_ID_MJPEG, MKTAG('I', 'J', 'L', 'V') },
	/* Midvid JPEG Video Codec */
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'V', 'J', 'P') },
	{ AV_CODEC_ID_MJPEG, MKTAG('A', 'V', 'I', '1') },
	{ AV_CODEC_ID_MJPEG, MKTAG('A', 'V', 'I', '2') },
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'T', 'S', 'J') },
	/* Paradigm Matrix M-JPEG Codec */
	{ AV_CODEC_ID_MJPEG, MKTAG('Z', 'J', 'P', 'G') },
	{ AV_CODEC_ID_MJPEG, MKTAG('M', 'M', 'J', 'P') },
	{ AV_CODEC_ID_HUFFYUV, MKTAG('H', 'F', 'Y', 'U') },
	{ AV_CODEC_ID_FFVHUFF, MKTAG('F', 'F', 'V', 'H') },
	{ AV_CODEC_ID_CYUV, MKTAG('C', 'Y', 'U', 'V') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG(0, 0, 0, 0) },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG(3, 0, 0, 0) },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '2', '0') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'U', 'Y', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', '4', '2', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('V', '4', '2', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'U', 'N', 'V') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('U', 'Y', 'N', 'V') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('U', 'Y', 'N', 'Y') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('u', 'y', 'v', '1') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('2', 'V', 'u', '1') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('2', 'v', 'u', 'y') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('y', 'u', 'v', 's') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('y', 'u', 'v', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('P', '4', '2', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', '1', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', '1', '6') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', '2', '4') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('U', 'Y', 'V', 'Y') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('V', 'Y', 'U', 'Y') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', 'Y', 'U', 'V') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', '8', '0', '0') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', '8', ' ', ' ') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('H', 'D', 'Y', 'C') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', 'U', '9') },
	/* SoftLab-NSK VideoTizer */
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('V', 'D', 'T', 'Z') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', '4', '1', '1') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('N', 'V', '1', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('N', 'V', '2', '1') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', '4', '1', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', '4', '2', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'U', 'V', '9') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', 'U', '9') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('a', 'u', 'v', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', 'Y', 'U') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'U', 'Y', 'V') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '1', '0') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '1', '1') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '2', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '4', '0') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '4', '4') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('J', '4', '2', '0') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('J', '4', '2', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('J', '4', '4', '0') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('J', '4', '4', '4') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('Y', 'U', 'V', 'A') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '0', 'A') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '2', 'A') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'G', 'B', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'V', '1', '5') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'V', '1', '6') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'V', '2', '4') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'V', '3', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'G', 'B', 'A') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('A', 'V', '3', '2') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('G', 'R', 'E', 'Y') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', '9', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', '9', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '2', '9', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '2', '9', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '9', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', '9', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', 'A', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', 'A', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '2', 'A', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '2', 'A', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', 'A', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', 'A', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', 'F', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', 'F', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', 'C', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', 'C', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '2', 'C', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '2', 'C', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', 'C', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '4', 'C', 'B') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', 'F', 'L') },
	{ AV_CODEC_ID_RAWVIDEO, MKTAG('I', '0', 'F', 'B') },
	{ AV_CODEC_ID_FRWU, MKTAG('F', 'R', 'W', 'U') },
	{ AV_CODEC_ID_R10K, MKTAG('R', '1', '0', 'k') },
	{ AV_CODEC_ID_R210, MKTAG('r', '2', '1', '0') },
	{ AV_CODEC_ID_V210, MKTAG('v', '2', '1', '0') },
	{ AV_CODEC_ID_V210, MKTAG('C', '2', '1', '0') },
	{ AV_CODEC_ID_V308, MKTAG('v', '3', '0', '8') },
	{ AV_CODEC_ID_V408, MKTAG('v', '4', '0', '8') },
	{ AV_CODEC_ID_AYUV, MKTAG('A', 'Y', 'U', 'V') },
	{ AV_CODEC_ID_V410, MKTAG('v', '4', '1', '0') },
	{ AV_CODEC_ID_YUV4, MKTAG('y', 'u', 'v', '4') },
	{ AV_CODEC_ID_INDEO3, MKTAG('I', 'V', '3', '1') },
	{ AV_CODEC_ID_INDEO3, MKTAG('I', 'V', '3', '2') },
	{ AV_CODEC_ID_INDEO4, MKTAG('I', 'V', '4', '1') },
	{ AV_CODEC_ID_INDEO5, MKTAG('I', 'V', '5', '0') },
	{ AV_CODEC_ID_VP3, MKTAG('V', 'P', '3', '1') },
	{ AV_CODEC_ID_VP3, MKTAG('V', 'P', '3', '0') },
	{ AV_CODEC_ID_VP5, MKTAG('V', 'P', '5', '0') },
	{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', '0') },
	{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', '1') },
	{ AV_CODEC_ID_VP6, MKTAG('V', 'P', '6', '2') },
	{ AV_CODEC_ID_VP6A, MKTAG('V', 'P', '6', 'A') },
	{ AV_CODEC_ID_VP6F, MKTAG('V', 'P', '6', 'F') },
	{ AV_CODEC_ID_VP6F, MKTAG('F', 'L', 'V', '4') },
	{ AV_CODEC_ID_VP7, MKTAG('V', 'P', '7', '0') },
	{ AV_CODEC_ID_VP7, MKTAG('V', 'P', '7', '1') },
	{ AV_CODEC_ID_VP8, MKTAG('V', 'P', '8', '0') },
	{ AV_CODEC_ID_VP9, MKTAG('V', 'P', '9', '0') },
	{ AV_CODEC_ID_ASV1, MKTAG('A', 'S', 'V', '1') },
	{ AV_CODEC_ID_ASV2, MKTAG('A', 'S', 'V', '2') },
	{ AV_CODEC_ID_VCR1, MKTAG('V', 'C', 'R', '1') },
	{ AV_CODEC_ID_FFV1, MKTAG('F', 'F', 'V', '1') },
	{ AV_CODEC_ID_XAN_WC4, MKTAG('X', 'x', 'a', 'n') },
	{ AV_CODEC_ID_MIMIC, MKTAG('L', 'M', '2', '0') },
	{ AV_CODEC_ID_MSRLE, MKTAG('m', 'r', 'l', 'e') },
	{ AV_CODEC_ID_MSRLE, MKTAG(1, 0, 0, 0) },
	{ AV_CODEC_ID_MSRLE, MKTAG(2, 0, 0, 0) },
	{ AV_CODEC_ID_MSVIDEO1, MKTAG('M', 'S', 'V', 'C') },
	{ AV_CODEC_ID_MSVIDEO1, MKTAG('m', 's', 'v', 'c') },
	{ AV_CODEC_ID_MSVIDEO1, MKTAG('C', 'R', 'A', 'M') },
	{ AV_CODEC_ID_MSVIDEO1, MKTAG('c', 'r', 'a', 'm') },
	{ AV_CODEC_ID_MSVIDEO1, MKTAG('W', 'H', 'A', 'M') },
	{ AV_CODEC_ID_MSVIDEO1, MKTAG('w', 'h', 'a', 'm') },
	{ AV_CODEC_ID_CINEPAK, MKTAG('c', 'v', 'i', 'd') },
	{ AV_CODEC_ID_TRUEMOTION1, MKTAG('D', 'U', 'C', 'K') },
	{ AV_CODEC_ID_TRUEMOTION1, MKTAG('P', 'V', 'E', 'Z') },
	{ AV_CODEC_ID_MSZH, MKTAG('M', 'S', 'Z', 'H') },
	{ AV_CODEC_ID_ZLIB, MKTAG('Z', 'L', 'I', 'B') },
	{ AV_CODEC_ID_SNOW, MKTAG('S', 'N', 'O', 'W') },
	{ AV_CODEC_ID_4XM, MKTAG('4', 'X', 'M', 'V') },
	{ AV_CODEC_ID_FLV1, MKTAG('F', 'L', 'V', '1') },
	{ AV_CODEC_ID_FLV1, MKTAG('S', '2', '6', '3') },
	{ AV_CODEC_ID_FLASHSV, MKTAG('F', 'S', 'V', '1') },
	{ AV_CODEC_ID_SVQ1, MKTAG('s', 'v', 'q', '1') },
	{ AV_CODEC_ID_TSCC, MKTAG('t', 's', 'c', 'c') },
	{ AV_CODEC_ID_ULTI, MKTAG('U', 'L', 'T', 'I') },
	{ AV_CODEC_ID_VIXL, MKTAG('V', 'I', 'X', 'L') },
	{ AV_CODEC_ID_QPEG, MKTAG('Q', 'P', 'E', 'G') },
	{ AV_CODEC_ID_QPEG, MKTAG('Q', '1', '.', '0') },
	{ AV_CODEC_ID_QPEG, MKTAG('Q', '1', '.', '1') },
	{ AV_CODEC_ID_WMV3, MKTAG('W', 'M', 'V', '3') },
	{ AV_CODEC_ID_WMV3IMAGE, MKTAG('W', 'M', 'V', 'P') },
	{ AV_CODEC_ID_VC1, MKTAG('W', 'V', 'C', '1') },
	{ AV_CODEC_ID_VC1, MKTAG('W', 'M', 'V', 'A') },
	{ AV_CODEC_ID_VC1IMAGE, MKTAG('W', 'V', 'P', '2') },
	{ AV_CODEC_ID_LOCO, MKTAG('L', 'O', 'C', 'O') },
	{ AV_CODEC_ID_WNV1, MKTAG('W', 'N', 'V', '1') },
	{ AV_CODEC_ID_WNV1, MKTAG('Y', 'U', 'V', '8') },
	{ AV_CODEC_ID_AASC, MKTAG('A', 'A', 'S', '4') }, /* Autodesk 24 bit RLE compressor */
	{ AV_CODEC_ID_AASC, MKTAG('A', 'A', 'S', 'C') },
	{ AV_CODEC_ID_INDEO2, MKTAG('R', 'T', '2', '1') },
	{ AV_CODEC_ID_FRAPS, MKTAG('F', 'P', 'S', '1') },
	{ AV_CODEC_ID_THEORA, MKTAG('t', 'h', 'e', 'o') },
	{ AV_CODEC_ID_TRUEMOTION2, MKTAG('T', 'M', '2', '0') },
	{ AV_CODEC_ID_TRUEMOTION2RT, MKTAG('T', 'R', '2', '0') },
	{ AV_CODEC_ID_CSCD, MKTAG('C', 'S', 'C', 'D') },
	{ AV_CODEC_ID_ZMBV, MKTAG('Z', 'M', 'B', 'V') },
	{ AV_CODEC_ID_KMVC, MKTAG('K', 'M', 'V', 'C') },
	{ AV_CODEC_ID_CAVS, MKTAG('C', 'A', 'V', 'S') },
	{ AV_CODEC_ID_AVS2, MKTAG('A', 'V', 'S', '2') },
	{ AV_CODEC_ID_JPEG2000, MKTAG('m', 'j', 'p', '2') },
	{ AV_CODEC_ID_JPEG2000, MKTAG('M', 'J', '2', 'C') },
	{ AV_CODEC_ID_JPEG2000, MKTAG('L', 'J', '2', 'C') },
	{ AV_CODEC_ID_JPEG2000, MKTAG('L', 'J', '2', 'K') },
	{ AV_CODEC_ID_JPEG2000, MKTAG('I', 'P', 'J', '2') },
	{ AV_CODEC_ID_JPEG2000, MKTAG('A', 'V', 'j', '2') }, /* Avid jpeg2000 */
	{ AV_CODEC_ID_VMNC, MKTAG('V', 'M', 'n', 'c') },
	{ AV_CODEC_ID_TARGA, MKTAG('t', 'g', 'a', ' ') },
	{ AV_CODEC_ID_PNG, MKTAG('M', 'P', 'N', 'G') },
	{ AV_CODEC_ID_PNG, MKTAG('P', 'N', 'G', '1') },
	{ AV_CODEC_ID_PNG, MKTAG('p', 'n', 'g', ' ') }, /* ImageJ */
	{ AV_CODEC_ID_CLJR, MKTAG('C', 'L', 'J', 'R') },
	{ AV_CODEC_ID_DIRAC, MKTAG('d', 'r', 'a', 'c') },
	{ AV_CODEC_ID_RPZA, MKTAG('a', 'z', 'p', 'r') },
	{ AV_CODEC_ID_RPZA, MKTAG('R', 'P', 'Z', 'A') },
	{ AV_CODEC_ID_RPZA, MKTAG('r', 'p', 'z', 'a') },
	{ AV_CODEC_ID_SP5X, MKTAG('S', 'P', '5', '4') },
	{ AV_CODEC_ID_AURA, MKTAG('A', 'U', 'R', 'A') },
	{ AV_CODEC_ID_AURA2, MKTAG('A', 'U', 'R', '2') },
	{ AV_CODEC_ID_DPX, MKTAG('d', 'p', 'x', ' ') },
	{ AV_CODEC_ID_KGV1, MKTAG('K', 'G', 'V', '1') },
	{ AV_CODEC_ID_LAGARITH, MKTAG('L', 'A', 'G', 'S') },
	{ AV_CODEC_ID_AMV, MKTAG('A', 'M', 'V', 'F') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'R', 'A') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'R', 'G') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'Y', '0') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'Y', '2') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'Y', '4') },
	/* Ut Video version 13.0.1 BT.709 codecs */
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'H', '0') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'H', '2') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'L', 'H', '4') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'Q', 'Y', '2') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'Q', 'R', 'A') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'Q', 'R', 'G') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'M', 'Y', '2') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'M', 'H', '2') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'M', 'Y', '4') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'M', 'H', '4') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'M', 'R', 'A') },
	{ AV_CODEC_ID_UTVIDEO, MKTAG('U', 'M', 'R', 'G') },
	{ AV_CODEC_ID_VBLE, MKTAG('V', 'B', 'L', 'E') },
	{ AV_CODEC_ID_ESCAPE130, MKTAG('E', '1', '3', '0') },
	{ AV_CODEC_ID_DXTORY, MKTAG('x', 't', 'o', 'r') },
	{ AV_CODEC_ID_ZEROCODEC, MKTAG('Z', 'E', 'C', 'O') },
	{ AV_CODEC_ID_Y41P, MKTAG('Y', '4', '1', 'P') },
	{ AV_CODEC_ID_FLIC, MKTAG('A', 'F', 'L', 'C') },
	{ AV_CODEC_ID_MSS1, MKTAG('M', 'S', 'S', '1') },
	{ AV_CODEC_ID_MSA1, MKTAG('M', 'S', 'A', '1') },
	{ AV_CODEC_ID_TSCC2, MKTAG('T', 'S', 'C', '2') },
	{ AV_CODEC_ID_MTS2, MKTAG('M', 'T', 'S', '2') },
	{ AV_CODEC_ID_CLLC, MKTAG('C', 'L', 'L', 'C') },
	{ AV_CODEC_ID_MSS2, MKTAG('M', 'S', 'S', '2') },
	{ AV_CODEC_ID_SVQ3, MKTAG('S', 'V', 'Q', '3') },
	{ AV_CODEC_ID_012V, MKTAG('0', '1', '2', 'v') },
	{ AV_CODEC_ID_012V, MKTAG('a', '1', '2', 'v') },
	{ AV_CODEC_ID_G2M, MKTAG('G', '2', 'M', '2') },
	{ AV_CODEC_ID_G2M, MKTAG('G', '2', 'M', '3') },
	{ AV_CODEC_ID_G2M, MKTAG('G', '2', 'M', '4') },
	{ AV_CODEC_ID_G2M, MKTAG('G', '2', 'M', '5') },
	{ AV_CODEC_ID_FIC, MKTAG('F', 'I', 'C', 'V') },
	{ AV_CODEC_ID_HQX, MKTAG('C', 'H', 'Q', 'X') },
	{ AV_CODEC_ID_TDSC, MKTAG('T', 'D', 'S', 'C') },
	{ AV_CODEC_ID_HQ_HQA, MKTAG('C', 'U', 'V', 'C') },
	{ AV_CODEC_ID_RV40, MKTAG('R', 'V', '4', '0') },
	{ AV_CODEC_ID_SCREENPRESSO, MKTAG('S', 'P', 'V', '1') },
	{ AV_CODEC_ID_RSCC, MKTAG('R', 'S', 'C', 'C') },
	{ AV_CODEC_ID_RSCC, MKTAG('I', 'S', 'C', 'C') },
	{ AV_CODEC_ID_CFHD, MKTAG('C', 'F', 'H', 'D') },
	{ AV_CODEC_ID_M101, MKTAG('M', '1', '0', '1') },
	{ AV_CODEC_ID_M101, MKTAG('M', '1', '0', '2') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', 'A', 'G', 'Y') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'R', 'G') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'R', 'A') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'G', '0') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', '0') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', '2') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', '4') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', 'A') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'R', 'A') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'R', 'G') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'G', '0') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'Y', '2') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '2', 'R', 'A') },
	{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '2', 'R', 'G') },
	{ AV_CODEC_ID_YLC, MKTAG('Y', 'L', 'C', '0') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '0') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '1') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '2') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '3') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '4') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '5') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '7') },
	{ AV_CODEC_ID_SPEEDHQ, MKTAG('S', 'H', 'Q', '9') },
	{ AV_CODEC_ID_FMVC, MKTAG('F', 'M', 'V', 'C') },
	{ AV_CODEC_ID_SCPR, MKTAG('S', 'C', 'P', 'R') },
	{ AV_CODEC_ID_CLEARVIDEO, MKTAG('U', 'C', 'O', 'D') },
	{ AV_CODEC_ID_AV1, MKTAG('A', 'V', '0', '1') },
	{ AV_CODEC_ID_MSCC, MKTAG('M', 'S', 'C', 'C') },
	{ AV_CODEC_ID_SRGC, MKTAG('S', 'R', 'G', 'C') },
	{ AV_CODEC_ID_IMM4, MKTAG('I', 'M', 'M', '4') },
	{ AV_CODEC_ID_PROSUMER, MKTAG('B', 'T', '2', '0') },
	{ AV_CODEC_ID_MWSC, MKTAG('M', 'W', 'S', 'C') },
	{ AV_CODEC_ID_WCMV, MKTAG('W', 'C', 'M', 'V') },
	{ AV_CODEC_ID_RASC, MKTAG('R', 'A', 'S', 'C') },
	{ AV_CODEC_ID_NONE, 0 }
};

const struct AVCodecTag *avformat_get_riff_video_tags(void)
{
	return ff_codec_bmp_tags;
}

static inline av_const int av_toupper(int c)
{
	if (c >= 'a' && c <= 'z')
		c ^= 0x20;
	return c;
}

unsigned int avpriv_toupper4(unsigned int x)
{
	return av_toupper(x & 0xFF) +
		(av_toupper((x >> 8) & 0xFF) << 8) +
		(av_toupper((x >> 16) & 0xFF) << 16) +
		((unsigned)av_toupper((x >> 24) & 0xFF) << 24);
}

enum AVCodecID ff_codec_get_id(const AVCodecTag *tags, unsigned int tag)
{
	int i;
	for (i = 0; tags[i].id != AV_CODEC_ID_NONE; i++)
	if (tag == tags[i].tag)
		return tags[i].id;
	for (i = 0; tags[i].id != AV_CODEC_ID_NONE; i++)
	if (avpriv_toupper4(tag) == avpriv_toupper4(tags[i].tag))
		return tags[i].id;
	return AV_CODEC_ID_NONE;
}

enum AVCodecID av_codec_get_id(const AVCodecTag *const *tags, unsigned int tag)
{
	int i;
	for (i = 0; tags && tags[i]; i++) {
		enum AVCodecID id = ff_codec_get_id(tags[i], tag);
		if (id != AV_CODEC_ID_NONE)
			return id;
	}
	return AV_CODEC_ID_NONE;
}

void videoInput::getCodecAndVideoSize(videoDevice *VD)
{
	infolist[VD->myID].clear();

	int iCount = 0;
	int iSize = 0;
	HRESULT hr = VD->streamConf->GetNumberOfCapabilities(&iCount, &iSize);
	if (hr != S_OK) {
		fprintf(stderr, "fail to get number of capabilities\n");
		return;
	}

	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
		std::map<int, std::map<int, std::set<std::vector<int>>>> info;

		//For each format type RGB24 YUV2 etc
		for (int iFormat = 0; iFormat < iCount; iFormat++) {
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;

			hr = VD->streamConf->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr)){
				//his is how many diff sizes are available for the format
				int stepX = scc.OutputGranularityX;
				int stepY = scc.OutputGranularityY;

				//Don't want to get stuck in a loop
				if (stepX < 1 || stepY < 1) continue;

				std::vector<int> size_min{ scc.MinOutputSize.cx, scc.MinOutputSize.cy }, size_max{ scc.MaxOutputSize.cx, scc.MaxOutputSize.cy };

				VIDEOINFOHEADER* pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmtConfig->pbFormat);
				BITMAPINFOHEADER* bih = &pVih->bmiHeader;
				const AVCodecTag *const tags[] = { avformat_get_riff_video_tags(), NULL };

				enum AVPixelFormat pix_fmt = dshow_pixfmt(bih->biCompression, bih->biBitCount);
				if (pix_fmt == AV_PIX_FMT_NONE) {
					enum AVCodecID codec_id = av_codec_get_id(tags, bih->biCompression);
					if (codec_id != AV_CODEC_ID_NONE) {
						if (codec_id == AV_CODEC_ID_MJPEG) {
							info[VD->myID][VIDEO_CODEC_TYPE_MJPEG].insert(size_min);
							info[VD->myID][VIDEO_CODEC_TYPE_MJPEG].insert(size_max);
						}
						else if (codec_id == AV_CODEC_ID_H264) {
							info[VD->myID][VIDEO_CODEC_TYPE_H264].insert(size_min);
							info[VD->myID][VIDEO_CODEC_TYPE_H264].insert(size_max);
						}
						else if (codec_id == AV_CODEC_ID_H265) {
							info[VD->myID][VIDEO_CODEC_TYPE_H265].insert(size_min);
							info[VD->myID][VIDEO_CODEC_TYPE_H265].insert(size_max);
						}
					}
				}
				else {
					info[VD->myID][VIDEO_CODEC_TYPE_RAWVIDEO].insert(size_min);
					info[VD->myID][VIDEO_CODEC_TYPE_RAWVIDEO].insert(size_max);
				}

				MyDeleteMediaType(pmtConfig);
			}
		}

		for (auto codec_id = 0; codec_id < info[VD->myID].size(); ++codec_id) {
			for (auto it = info[VD->myID][codec_id].cbegin(); it != info[VD->myID][codec_id].cend(); ++it) {
				std::string size = std::to_string((*it)[0]);
				size += "x";
				size += std::to_string((*it)[1]);
				infolist[VD->myID][codec_id].push_back(size);
			}
		}
	}
	else {
		fprintf(stderr, "fail to get codec and video size\n");
		return;
	}
}

// ----------------------------------------------------------------------
// Returns number of good devices
//
// ----------------------------------------------------------------------
int videoInput::getDeviceCount(){
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK){
			IMoniker *pMoniker = NULL;
			while (pEnum->Next(1, &pMoniker, NULL) == S_OK){

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)){
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				deviceCounter++;
			}

			pEnum->Release();
			pEnum = NULL;
		}

		pDevEnum->Release();
		pDevEnum = NULL;
	}
	return deviceCounter;
}

// ----------------------------------------------------------------------
// Do we need this?
//
// Enumerate all of the video input devices
// Return the filter with a matching friendly name
// ----------------------------------------------------------------------
HRESULT videoInput::getDevice(IBaseFilter** gottaFilter, int deviceId, WCHAR * wDeviceName, char * nDeviceName){
	BOOL done = false;
	int deviceCounter = 0;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			if (deviceCounter == deviceId)
			{
				// Bind the first moniker to an object
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
				if (SUCCEEDED(hr))
				{
					// To retrieve the filter's friendly name, do the following:
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{

						//copy the name to nDeviceName & wDeviceName
						int count = 0;
						while (varName.bstrVal[count] != 0x00) {
							wDeviceName[count] = varName.bstrVal[count];
							nDeviceName[count] = (char)varName.bstrVal[count];
							count++;
						}

						// We found it, so send it back to the caller
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)gottaFilter);
						done = true;
					}
					VariantClear(&varName);
					pPropBag->Release();
					pPropBag = NULL;
					pMoniker->Release();
					pMoniker = NULL;
				}
			}
			deviceCounter++;
		}
		pEnumCat->Release();
		pEnumCat = NULL;
	}
	pSysDevEnum->Release();
	pSysDevEnum = NULL;

	if (done) {
		return hr;    // found it, return native error
	}
	else {
		return VFW_E_NOT_FOUND;    // didn't find it error
	}
}

// ----------------------------------------------------------------------
// Show the property pages for a filter
// This is stolen from the DX9 SDK
// ----------------------------------------------------------------------
HRESULT videoInput::ShowFilterPropertyPages(IBaseFilter *pFilter){
	ISpecifyPropertyPages *pProp;

	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr))
	{
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo);
		IUnknown *pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

		// Show the page.
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
			NULL,                   // Parent window
			0, 0,                   // Reserved
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			&pFilterUnk,            // Array of object pointers.
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL                 // Reserved
			);

		// Clean up.
		if (pFilterUnk)pFilterUnk->Release();
		if (FilterInfo.pGraph)FilterInfo.pGraph->Release();
		CoTaskMemFree(caGUID.pElems);
	}
	return hr;
}

HRESULT videoInput::ShowStreamPropertyPages(IAMStreamConfig  * /*pStream*/){

	HRESULT hr = NOERROR;
	return hr;
}

// ----------------------------------------------------------------------
// This code was also brazenly stolen from the DX9 SDK
// Pass it a file name in wszPath, and it will save the filter graph to that file.
// ----------------------------------------------------------------------
HRESULT videoInput::SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) {
	const WCHAR wszStreamName[] = L"ActiveMovieGraph";
	HRESULT hr;
	IStorage *pStorage = NULL;

	// First, create a document file which will hold the GRF file
	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if (FAILED(hr))
	{
		return hr;
	}

	// Next, create a stream to store.
	IStream *pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr))
	{
		pStorage->Release();
		return hr;
	}

	// The IPersistStream converts a stream into a persistent object.
	IPersistStream *pPersist = NULL;
	pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
	hr = pPersist->Save(pStream, TRUE);
	pStream->Release();
	pPersist->Release();
	if (SUCCEEDED(hr))
	{
		hr = pStorage->Commit(STGC_DEFAULT);
	}
	pStorage->Release();
	return hr;
}

// ----------------------------------------------------------------------
// For changing the input types
//
// ----------------------------------------------------------------------
HRESULT videoInput::routeCrossbar(ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode){

	//create local ICaptureGraphBuilder2
	ICaptureGraphBuilder2 *pBuild = NULL;
	pBuild = *ppBuild;

	//create local IBaseFilter
	IBaseFilter *pVidFilter = NULL;
	pVidFilter = *pVidInFilter;

	// Search upstream for a crossbar.
	IAMCrossbar *pXBar1 = NULL;
	HRESULT hr = pBuild->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pVidFilter,
		IID_IAMCrossbar, (void**)&pXBar1);
	if (SUCCEEDED(hr))
	{

		bool foundDevice = false;

		if (verbose)printf("SETUP: You are not a webcam! Setting Crossbar\n");
		pXBar1->Release();

		IAMCrossbar *Crossbar;
		hr = pBuild->FindInterface(&captureMode, &MEDIATYPE_Interleaved, pVidFilter, IID_IAMCrossbar, (void **)&Crossbar);

		if (hr != NOERROR){
			hr = pBuild->FindInterface(&captureMode, &MEDIATYPE_Video, pVidFilter, IID_IAMCrossbar, (void **)&Crossbar);
		}

		LONG lInpin, lOutpin;
		hr = Crossbar->get_PinCounts(&lOutpin, &lInpin);

		BOOL iPin = TRUE; LONG pIndex = 0, pRIndex = 0, pType = 0;

		while (pIndex < lInpin)
		{
			hr = Crossbar->get_CrossbarPinInfo(iPin, pIndex, &pRIndex, &pType);

			if (pType == conType){
				if (verbose)printf("SETUP: Found Physical Interface");

				switch (conType){

				case PhysConn_Video_Composite:
					if (verbose)printf(" - Composite\n");
					break;
				case PhysConn_Video_SVideo:
					if (verbose)printf(" - S-Video\n");
					break;
				case PhysConn_Video_Tuner:
					if (verbose)printf(" - Tuner\n");
					break;
				case PhysConn_Video_USB:
					if (verbose)printf(" - USB\n");
					break;
				case PhysConn_Video_1394:
					if (verbose)printf(" - Firewire\n");
					break;
				}

				foundDevice = true;
				break;
			}
			pIndex++;

		}

		if (foundDevice){
			BOOL OPin = FALSE; LONG pOIndex = 0, pORIndex = 0, pOType = 0;
			while (pOIndex < lOutpin)
			{
				hr = Crossbar->get_CrossbarPinInfo(OPin, pOIndex, &pORIndex, &pOType);
				if (pOType == PhysConn_Video_VideoDecoder)
					break;
			}
			Crossbar->Route(pOIndex, pIndex);
		}
		else{
			if (verbose) printf("SETUP: Didn't find specified Physical Connection type. Using Defualt. \n");
		}

		//we only free the crossbar when we close or restart the device
		//we were getting a crash otherwise
		//if(Crossbar)Crossbar->Release();
		//if(Crossbar)Crossbar = NULL;

		if (pXBar1)pXBar1->Release();
		if (pXBar1)pXBar1 = NULL;

	}
	else{
		if (verbose) printf("SETUP: You are a webcam or snazzy firewire cam! No Crossbar needed\n");
		return hr;
	}

	return hr;
}

bool videoInput::getCodecList(int device_id, std::vector<int>& codecids)
{
	codecids.clear();
	if (infolist[device_id].size() == 0) return false;

	for (auto it = infolist[device_id].cbegin(); it != infolist[device_id].cend(); ++it) {
		codecids.push_back(it->first);
	}

	return true;
}

bool videoInput::getVideoSizeList(int device_id, int codec_id, std::vector<std::string>& sizelist)
{
	if (device_id >= getDeviceCount()) return false;

	sizelist = this->infolist[device_id][codec_id];

	return true;
}

/********************* Capturing video from camera via DirectShow *********************/
struct SuppressVideoInputMessages
{
	SuppressVideoInputMessages() { videoInput::setVerbose(false); }
};

static SuppressVideoInputMessages do_it;
videoInput CvCaptureCAM_DShow::VI;

CvCaptureCAM_DShow::CvCaptureCAM_DShow()
{
	index = -1;
	frame = 0;
	width = height = fourcc = -1;
	widthSet = heightSet = -1;
	CoInitialize(0);
}

CvCaptureCAM_DShow::~CvCaptureCAM_DShow()
{
	close();
	CoUninitialize();
}

void CvCaptureCAM_DShow::close()
{
	if (index >= 0)
	{
		VI.stopDevice(index);
		index = -1;
		cvReleaseImage(&frame);
	}
	widthSet = heightSet = width = height = -1;
}

// Initialize camera input
bool CvCaptureCAM_DShow::open(int _index)
{
	int devices = 0;

	close();
	devices = VI.listDevices(true);
	if (devices == 0)
		return false;
	if (_index < 0 || _index > devices - 1)
		return false;
	VI.setupDevice(_index);
	if (!VI.isDeviceSetup(_index))
		return false;
	index = _index;
	return true;
}

bool CvCaptureCAM_DShow::grabFrame()
{
	return true;
}

IplImage* CvCaptureCAM_DShow::retrieveFrame(int)
{
	if (!frame || VI.getWidth(index) != frame->width || VI.getHeight(index) != frame->height)
	{
		if (frame)
			cvReleaseImage(&frame);
		int w = VI.getWidth(index), h = VI.getHeight(index);
		frame = cvCreateImage(cvSize(w, h), 8, 3);
	}

	if (VI.getPixels(index, (uchar*)frame->imageData, false, true))
		return frame;
	else
		return NULL;
}

double CvCaptureCAM_DShow::getProperty(int property_id)
{

	long min_value, max_value, stepping_delta, current_value, flags, defaultValue;

	// image format proprrties
	switch (property_id)
	{
	case CV_CAP_PROP_FRAME_WIDTH:
		return VI.getWidth(index);

	case CV_CAP_PROP_FRAME_HEIGHT:
		return VI.getHeight(index);

	case CV_CAP_PROP_FOURCC:
		return VI.getFourcc(index);

	case CV_CAP_PROP_FPS:
		return VI.getFPS(index);
	}

	// video filter properties
	switch (property_id)
	{
	case CV_CAP_PROP_BRIGHTNESS:
	case CV_CAP_PROP_CONTRAST:
	case CV_CAP_PROP_HUE:
	case CV_CAP_PROP_SATURATION:
	case CV_CAP_PROP_SHARPNESS:
	case CV_CAP_PROP_GAMMA:
	case CV_CAP_PROP_MONOCROME:
	case CV_CAP_PROP_WHITE_BALANCE_U:
	case CV_CAP_PROP_BACKLIGHT:
	case CV_CAP_PROP_GAIN:
		if (VI.getVideoSettingFilter(index, VI.getVideoPropertyFromCV(property_id), min_value, max_value, stepping_delta, current_value, flags, defaultValue)) return (double)current_value;
	}

	// camera properties
	switch (property_id)
	{
	case CV_CAP_PROP_PAN:
	case CV_CAP_PROP_TILT:
	case CV_CAP_PROP_ROLL:
	case CV_CAP_PROP_ZOOM:
	case CV_CAP_PROP_EXPOSURE:
	case CV_CAP_PROP_IRIS:
	case CV_CAP_PROP_FOCUS:
		if (VI.getVideoSettingCamera(index, VI.getCameraPropertyFromCV(property_id), min_value, max_value, stepping_delta, current_value, flags, defaultValue)) return (double)current_value;

	}

	// unknown parameter or value not available
	return -1;
}

bool CvCaptureCAM_DShow::setProperty(int property_id, double value)
{
	// image capture properties
	bool handled = false;
	switch (property_id)
	{
	case CV_CAP_PROP_FRAME_WIDTH:
		width = cvRound(value);
		handled = true;
		break;

	case CV_CAP_PROP_FRAME_HEIGHT:
		height = cvRound(value);
		handled = true;
		break;

	case CV_CAP_PROP_FOURCC:
		fourcc = (int)(unsigned long)(value);
		if (fourcc == -1) {
			// following cvCreateVideo usage will pop up caprturepindialog here if fourcc=-1
			// TODO - how to create a capture pin dialog
		}
		handled = true;
		break;

	case CV_CAP_PROP_FPS:
		int fps = cvRound(value);
		if (fps != VI.getFPS(index))
		{
			VI.stopDevice(index);
			VI.setIdealFramerate(index, fps);
			if (widthSet > 0 && heightSet > 0)
				VI.setupDevice(index, widthSet, heightSet);
			else
				VI.setupDevice(index);
		}
		return VI.isDeviceSetup(index);

	}

	if (handled) {
		// a stream setting
		if (width > 0 && height > 0)
		{
			if (width != VI.getWidth(index) || height != VI.getHeight(index))//|| fourcc != VI.getFourcc(index) )
			{
				int fps = static_cast<int>(VI.getFPS(index));
				VI.stopDevice(index);
				VI.setIdealFramerate(index, fps);
				VI.setupDeviceFourcc(index, width, height, fourcc);
			}

			bool success = VI.isDeviceSetup(index);
			if (success)
			{
				widthSet = width;
				heightSet = height;
				width = height = fourcc = -1;
			}
			return success;
		}
		return true;
	}

	// show video/camera filter dialog
	if (property_id == CV_CAP_PROP_SETTINGS) {
		VI.showSettingsWindow(index);
		return true;
	}

	//video Filter properties
	switch (property_id)
	{
	case CV_CAP_PROP_BRIGHTNESS:
	case CV_CAP_PROP_CONTRAST:
	case CV_CAP_PROP_HUE:
	case CV_CAP_PROP_SATURATION:
	case CV_CAP_PROP_SHARPNESS:
	case CV_CAP_PROP_GAMMA:
	case CV_CAP_PROP_MONOCROME:
	case CV_CAP_PROP_WHITE_BALANCE_U:
	case CV_CAP_PROP_BACKLIGHT:
	case CV_CAP_PROP_GAIN:
		return VI.setVideoSettingFilter(index, VI.getVideoPropertyFromCV(property_id), (long)value);
	}

	//camera properties
	switch (property_id)
	{
	case CV_CAP_PROP_PAN:
	case CV_CAP_PROP_TILT:
	case CV_CAP_PROP_ROLL:
	case CV_CAP_PROP_ZOOM:
	case CV_CAP_PROP_EXPOSURE:
	case CV_CAP_PROP_IRIS:
	case CV_CAP_PROP_FOCUS:
		return VI.setVideoSettingCamera(index, VI.getCameraPropertyFromCV(property_id), (long)value);
	}

	return false;
}

bool CvCaptureCAM_DShow::getDevicesList(std::map<int, std::string>& devicelist) const
{
	devicelist.clear();

	for (int i = 0; i < VI.getDeviceCount(); ++i) {
		devicelist[i] = VI.getDeviceName(i);
	}

	return true;
}

bool CvCaptureCAM_DShow::getCodecList(int device_id, std::vector<int>& codecids) const
{
	codecids.clear();
	return VI.getCodecList(device_id, codecids);
}

bool CvCaptureCAM_DShow::getVideoSizeList(int device_id, int codec_id, std::vector<std::string>& sizelist) const
{
	sizelist.clear();
	return VI.getVideoSizeList(device_id, codec_id, sizelist);
}

CvCapture* cvCreateCameraCapture_DShow(int index)
{
	CvCaptureCAM_DShow* capture = new CvCaptureCAM_DShow;

	try {
		if (capture->open(index))
			return capture;
	} catch (...) {
		delete capture;
		throw;
	}

	delete capture;
	return nullptr;
}

} // namespace fbc

#endif // _MSC_VER
