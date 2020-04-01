// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#include "videocapture.hpp"
#include "core/Ptr.hpp"
#include "dshow.hpp"

// referece: 2.4.13.6
//           highgui/src/cap.cpp

namespace fbc{

CvCapture* cvCreateCameraCapture(int index)
{
	// local variable to memorize the captured device
#ifdef _MSC_VER
	CvCapture* capture = nullptr;
	capture = cvCreateCameraCapture_DShow(index);
	return capture;
#else
	return nullptr;
#endif
}

int cvGrabFrame(CvCapture* capture)
{
	return capture ? capture->grabFrame() : 0;
}

IplImage* cvRetrieveFrame(CvCapture* capture, int idx)
{
	return capture ? capture->retrieveFrame(idx) : 0;
}

double cvGetCaptureProperty(CvCapture* capture, int id)
{
	return capture ? capture->getProperty(id) : 0;
}

int cvSetCaptureProperty(CvCapture* capture, int id, double value)
{
	return capture ? capture->setProperty(id, value) : 0;
}

VideoCapture::VideoCapture()
{}

VideoCapture::VideoCapture(const std::string& filename)
{
	open(filename);
}

VideoCapture::VideoCapture(int device)
{
	device_id = device;
	open(device);
}

VideoCapture::~VideoCapture()
{
	cap.release();
}

bool VideoCapture::open(const std::string& filename)
{
	if (isOpened()) release();
	open(0);
	if (!cap) return false;

	std::map<int, device_info> devices_name;
	cap->getDevicesList(devices_name);
	if (devices_name.size() == 0) return false;
	if (isOpened()) release();

	int device = -1;
	for (auto it = devices_name.cbegin(); it != devices_name.cend(); ++it) {
		if (filename.compare((*it).second.name) == 0)
			device = (*it).first;
	}
	if (device == -1) return false;

	return open(device);
}

bool VideoCapture::open(int device)
{
	if (isOpened()) release();
	cap.reset(cvCreateCameraCapture(device));
	return isOpened();
}

bool VideoCapture::isOpened() const { return !cap.empty(); }

void VideoCapture::release()
{
	cap.release();
}

bool VideoCapture::grab()
{
	return cvGrabFrame(cap) != 0;
}

bool VideoCapture::retrieve(Mat_<unsigned char, 3>& image, int channel)
{
	IplImage* _img = cvRetrieveFrame(cap, channel);
	if (!_img)
	{
		image.release();
		return false;
	}

	if (_img->width != image.cols || _img->height != image.rows || _img->nChannels != image.channels) {
		printf("image size mismatch\n");
		return false;
	}
	static int size = _img->width * _img->height * _img->nChannels;
	memcpy(image.data, _img->imageData, size);

	//if (_img->origin == IPL_ORIGIN_TL)
	//	Mat(_img).copyTo(image);
	//else
	//{
	//	Mat temp(_img);
	//	flip(temp, image, 0);
	//}
	return true;
}

bool VideoCapture::read(Mat_<unsigned char, 3>& image)
{
	if (grab())
		retrieve(image);
	else
		image.release();
	return !image.empty();
}

VideoCapture& VideoCapture::operator >> (Mat_<unsigned char, 3>& image)
{
	read(image);
	return *this;
}

bool VideoCapture::set(int propId, double value)
{
	return cvSetCaptureProperty(cap, propId, value) != 0;
}

double VideoCapture::get(int propId)
{
	return cvGetCaptureProperty(cap, propId);
}

bool VideoCapture::getDevicesList(std::map<int, device_info>& infos) const
{
	if (!cap) return false;
	return cap->getDevicesList(infos);
}

bool VideoCapture::getCodecList(std::vector<int>& codecids) const
{
	if (!cap) return false;
	return cap->getCodecList(device_id, codecids);
}

bool VideoCapture::getVideoSizeList(int codec_id, std::vector<std::string>& sizelist) const
{
	if (!cap) return false;
	return cap->getVideoSizeList(device_id, codec_id, sizelist);
}

bool FBC_EXPORTS get_camera_names(std::map<int, device_info>& infos)
{
	VideoCapture capture(0);
	if (!capture.isOpened()) {
		fprintf(stderr, "fail to open capture\n");
		return false;
	}

	return capture.getDevicesList(infos);
}

} // namespace fbc
