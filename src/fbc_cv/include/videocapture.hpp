// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_VIDEOIO_HPP_
#define FBC_CV_VIDEOIO_HPP_

// reference: 2.4.13.6
//            highgui/include/opencv2/highgui/highgui.hpp

#include <string>
#include <map>
#include "core/mat.hpp"
#include "core/Ptr.hpp"
#include "capture.hpp"

namespace fbc {

typedef enum video_codec_type_t {
	VIDEO_CODEC_TYPE_H264,
	VIDEO_CODEC_TYPE_H265,
	VIDEO_CODEC_TYPE_MJPEG,
	VIDEO_CODEC_TYPE_RAWVIDEO
} video_codec_type_t;

class FBC_EXPORTS VideoCapture {
public:
	VideoCapture();
	VideoCapture(const std::string& filename);
	VideoCapture(int device);

	virtual ~VideoCapture();
	virtual bool open(const std::string& filename);
	virtual bool open(int device);
	virtual bool isOpened() const;
	virtual void release();

	virtual bool grab();
	virtual bool retrieve(Mat_<unsigned char, 3>& image, int channel = 0);
	virtual VideoCapture& operator >> (Mat_<unsigned char, 3>& image);
	virtual bool read(Mat_<unsigned char, 3>& image);

	virtual bool set(int propId, double value);
	virtual double get(int propId);

	virtual bool getDevicesList(std::map<int, device_info>& infos) const;
	virtual bool getCodecList(std::vector<int>& codecids) const;
	virtual bool getVideoSizeList(int codec_id, std::vector<std::string>& sizelist) const;

protected:
	Ptr<CvCapture> cap;

private:
	int device_id;
};

bool FBC_EXPORTS get_camera_names(std::map<int, device_info>& infos); // index value, device info

} // namespace fbc

#endif // FBC_CV_VIDEOIO_HPP_
