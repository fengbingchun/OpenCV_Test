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

protected:
	Ptr<CvCapture> cap;
};

} // namespace fbc

#endif // FBC_CV_VIDEOIO_HPP_
