// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_MAT_HPP_
#define FBC_CV_CORE_MAT_HPP_

/* reference: include/opencv2/core/mat.hpp
	      core/src/alloc.cpp
	      include/opencv2/core/utility.hpp
	      include/opencv2/core/cvstd.hpp
*/

#ifndef __cplusplus
	#error mat.hpp header must be compiled as C++
#endif

#include "core/fbcdef.hpp"
#include "core/types.hpp"
#include "core/base.hpp"
#include "core/interface.hpp"
#include "core/fbcstd.hpp"
#include "core/utility.hpp"

namespace fbc {

// The class Mat_ represents an n-dimensional dense numerical single-channel or multi-channel array
template<typename _Tp, int chs> class Mat_ {
public:
	typedef _Tp value_type;

	// default constructor
	Mat_() : rows(0), cols(0), channels(0), data(NULL), step(0), allocated(false) {}
	// constructs 2D matrix of the specified size
	Mat_(int _rows, int _cols);
	// constucts 2D matrix and fills it with the specified value _s
	Mat_(int _rows, int _cols, const Scalar& _s);
	// constructor for matrix headers pointing to user-allocated data, no data is copied
	Mat_(int _rows, int _cols, void* _data);
	// copy constructor, NOTE: deep copy
	Mat_(const Mat_<_Tp, chs>& _m);
	Mat_& operator = (const Mat_& _m);

	// copies the matrix content to "_m"
	void copyTo(Mat_<_Tp, chs>& _m, const Rect& rect = Rect(0, 0, 0, 0)) const;

	// return typed pointer to the specified matrix row
	const uchar* ptr(int i0 = 0) const;

	// no data is copied, no memory is allocated
	void getROI(Mat_<_Tp, chs>& _m, const Rect& rect = Rect(0, 0, 0, 0));

	// value converted to the actual array type
	void setTo(const Scalar& _value);

	// the method converts source pixel values to the target data type
	// if it does not have a proper size before the operation, it is reallocated
	template<typename _Tp2>
	void convertTo(Mat_<_Tp2, chs>& _m, double alpha = 1, const Scalar& scalar = Scalar(0, 0, 0, 0)) const;

	Mat_<_Tp, chs>& zeros(int _rows, int _cols);

	// returns the matrix cols and rows
	Size& size() const;
	// returns the matrix element size in bytes
	size_t elemSize() const;

	// release memory
	inline void release();
	// destructor - calls release()
	~Mat_() { release(); };

public:
	// the number of rows and columns
	int rows, cols;
	// channel num
	int channels;
	// pointer to the data
	uchar* data;
	// bytes per row
	int step; // stride
	// memory allocation flag
	bool allocated;
}; // Mat_

typedef Mat_<uchar, 1> Mat1Gray;
typedef Mat_<uchar, 3> Mat3BGR;
typedef Mat_<uchar, 4> Mat4BGRA;

template<typename _Tp, int chs> inline
void Mat_<_Tp, chs>::release()
{
	if (this->data && this->allocated) {
		fastFree(this->data);
	}

	this->data = NULL;
	this->allocated = false;
	this->rows = this->cols = this->step = this->channels = 0;
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(int _rows, int _cols)
{
	FBC_Assert(_rows > 0 && _cols > 0 && chs > 0);

	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = true;

	size_t size_ = this->rows * this->step;
	uchar* p = (uchar*)fastMalloc(size_);
	FBC_Assert(p != NULL);

	this->data = p;
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(int _rows, int _cols, const Scalar& _s)
{
	FBC_Assert(_rows > 0 && _cols > 0 && chs > 0);

	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = true;

	size_t size_ = this->rows * this->step;
	uchar* p = (uchar*)fastMalloc(size_);
	FBC_Assert(p != NULL);
	this->data = p;

	for (int i = 0; i < _rows; i++) {
		_Tp* pRow = (_Tp*)this->data + i * _cols * chs;

		for (int j = 0; j < _cols; j++) {
			_Tp* pPixel = pRow + j * chs;

			for (int m = 0, n = 0; m < chs && n < 4; m++, n++) {
				pPixel[n] = saturate_cast<_Tp>(_s.val[n]);
			}
		}
	}
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(int _rows, int _cols, void* _data)
{
	FBC_Assert(_rows > 0 && _cols > 0 && chs > 0);

	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = false;
	this->data = (uchar*)_data;
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(const Mat_<_Tp, chs>& _m)
{
	this->rows = _m.rows;
	this->cols = _m.cols;
	this->channels = _m.channels;
	this->step = sizeof(_Tp) * this->cols * this->channels;

	size_t size_ = this->rows * this->step;
	if (size_ > 0) {
		this->allocated = true;
		uchar* p = (uchar*)fastMalloc(size_);
		FBC_Assert(p != NULL);

		memcpy(p, _m.data, size_);
		this->data = p;
	} else {
		this->allocated = false;
		this->data = NULL;
	}
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>& Mat_<_Tp, chs>::operator = (const Mat_& _m)
{
	size_t size1 = this->rows * this->step;
	size_t size2 = _m.rows * _m.step;

	this->rows = _m.rows;
	this->cols = _m.cols;
	this->channels = _m.channels;
	this->step = sizeof(_Tp) * this->cols * this->channels;

	if ((size1 == size2) && (this->allocated == true) && (this->data != _m.data)) {
		memcpy(this->data, _m.data, size2);
	} else if (size2 > 0){
		if (this->allocated == true) {
			fastFree(this->data);
		}

		this->allocated = true;
		uchar* p = (uchar*)fastMalloc(size2);
		FBC_Assert(p != NULL);
		memcpy(p, _m.data, size2);
		this->data = p;
	} else {
		this->allocated = false;
		this->data = NULL;
	}

	return *this;
}

template<typename _Tp, int chs>
void Mat_<_Tp, chs>::copyTo(Mat_<_Tp, chs>& _m, const Rect& rect) const
{
	FBC_Assert((this->rows >= rect.y + rect.height) && (this->cols >= rect.x + rect.width));

	if (this->data != NULL) {
		if ((rect.width > 0) && (rect.height > 0)) {
			size_t size1 = sizeof(_Tp) * this->channels * rect.width * rect.height;
			int step_ = sizeof(_Tp) * this->channels * rect.width;
			size_t size2 = _m.rows * _m.step;

			if (size1 == size2) {
				uchar* p1 = _m.data;
				uchar* p2 = this->data;

				for (int i = 0; i < rect.height; i++) {
					uchar* p1_ = p1 + i * sizeof(_Tp) * this->channels * rect.width;
					uchar* p2_ = p2 + (rect.y + i) * this->step + rect.x * this->channels * sizeof(_Tp);

					memcpy(p1_, p2_, step_);
				}
			} else {
				if (_m.allocated == true)
					fastFree(_m.data);

				uchar* p1 = (uchar*)fastMalloc(size1);
				FBC_Assert(p1 != NULL);
				uchar* p2 = this->data;

				for (int i = 0; i < rect.height; i++) {
					uchar* p1_ = p1 + i * sizeof(_Tp) * this->channels * rect.width;
					uchar* p2_ = p2 + (rect.y + i) * this->step + rect.x * this->channels * sizeof(_Tp);

					memcpy(p1_, p2_, step_);
				}
				_m.data = p1;
				_m.allocated = true;
			}

			_m.rows = rect.height;
			_m.cols = rect.width;
			_m.step = step_;
		} else {
			size_t size1 = this->rows * this->step;
			size_t size2 = _m.step * _m.rows;

			if (size1 == size2) {
				memcpy(_m.data, this->data, size1);
			} else {
				if (_m.allocated == true)
					fastFree(_m.data);

				uchar* p = (uchar*)fastMalloc(size1);
				FBC_Assert(p != NULL);
				memcpy(p, this->data, size1);
				_m.data = p;
				_m.allocated = true;
			}

			_m.rows = this->rows;
			_m.cols = this->cols;
			_m.step = this->step;
		}
		_m.channels = this->channels;

	} else {
		if ((_m.data != NULL) && (_m.allocated == true)) {
			fastFree(_m.data);
		}

		_m.data = NULL;
		_m.allocated = false;
		_m.rows = 0;
		_m.cols = 0;
		_m.step = 0;
		_m.channels = 0;
	}
}

template<typename _Tp, int chs>
const uchar* Mat_<_Tp, chs>::ptr(int i0) const
{
	FBC_Assert(i0 < this->rows);

	return this->data + i0 * this->step;
}

template<typename _Tp, int chs>
void Mat_<_Tp, chs>::getROI(Mat_<_Tp, chs>& _m, const Rect& rect)
{
	FBC_Assert((rect.x >= 0) && (rect.y >= 0) && (rect.width > 0) && (rect.height > 0) &&
			(this->rows >= rect.y + rect.height) && (this->cols >= rect.x + rect.width));

	if (_m.allocated == true) {
		fastFree(_m.data);
	}

	_m.rows = rect.height;
	_m.cols = rect.width;
	_m.channels = this->channels;
	_m.allocated = false;
	_m.step = this->step;
	_m.data = this->data + rect.y * this->step + rect.x * sizeof(_Tp) * this->channels;
}

template<typename _Tp, int chs>
void Mat_<_Tp, chs>::setTo(const Scalar& _value)
{
	for (int i = 0; i < this->rows; i++) {
		uchar* pRow = this->data + i * this->step;

		for (int j = 0; j < this->cols; j++) {
			_Tp* pPixel = (_Tp*)pRow + j * chs;

			for (int m = 0, n = 0; m < chs && n < 4; m++, n++) {
				pPixel[n] = saturate_cast<_Tp>(_value.val[n]);
			}
		}
	}
}

template<typename _Tp, int chs> template<typename _Tp2>
void Mat_<_Tp, chs>::convertTo(Mat_<_Tp2, chs>& _m, double alpha = 1, const Scalar& scalar = Scalar(0, 0, 0, 0)) const
{
	FBC_Assert(this->channels <= 4);

	size_t size = this->rows * this->cols * this->channels * sizeof(_Tp2);

	if (_m.allocated == true) {
		if (this->rows * this->cols != _m.rows * _m.cols) {
			fastFree(_m.data);

			uchar* p = (uchar*)fastMalloc(size);
			FBC_Assert(p != NULL);
			_m.data = p;
		}
	} else {
		uchar* p = (uchar*)fastMalloc(size);
		FBC_Assert(p != NULL);
		_m.data = p;
	}

	_m.allocated = true;
	_m.channels = this->channels;
	_m.rows = this->rows;
	_m.cols = this->cols;
	_m.step = _m.cols * sizeof(_Tp2) * _m.channels;

	for (int i = 0; i < this->rows; i++) {
		uchar* p1 = this->data + i * this->step;
		uchar* p2 = _m.data + i * _m.step;

		for (int j = 0; j < this->cols; j++) {
			_Tp* p1_ = (_Tp*)p1 + j * chs;
			_Tp2* p2_ = (_Tp2*)p2 + j * chs;

			for (int ch = 0; ch < chs; ch++) {
				p2_[ch] = saturate_cast<_Tp2>(p1_[ch] * alpha + scalar.val[ch]);
			}
		}
	}
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>& Mat_<_Tp, chs>::zeros(int _rows, int _cols)
{
	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = true;

	size_t size_ = this->rows * this->step;
	uchar* p = (uchar*)fastMalloc(size_);
	FBC_Assert(p != NULL);
	this->data = p;

	memset(this->data, 0, size_);

	return *this;
}

template<typename _Tp, int chs>
Size& Mat_<_Tp, chs>::size() const
{
	return Size(this->cols, this->rows);
}

template<typename _Tp, int chs>
size_t Mat_<_Tp, chs>::elemSize() const
{
	return (this->channels * sizeof(_Tp));
}

} // fbc

#endif // FBC_CV_CORE_MAT_HPP_
