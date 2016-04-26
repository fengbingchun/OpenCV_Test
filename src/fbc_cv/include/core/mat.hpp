#ifndef FBC_CV_CORE_MAT_HPP_
#define FBC_CV_CORE_MAT_HPP_

// reference: include/opencv2/core/mat.hpp

#ifndef __cplusplus
	#error mat.hpp header must be compiled as C++
#endif

#include "core/fbcdef.hpp"
#include "core/types.hpp"
#include "core/base.hpp"
#include "core/interface.hpp"

namespace fbc {

template<typename _Tp, int chs> class Mat {
public:
	typedef _Tp value_type;

	Mat() : rows(0), cols(0), channels(0), data(NULL), step(0), allocated(false) {}
	Mat(int _rows, int _cols);
	Mat(int _rows, int _cols, const Scalar& _s);
	Mat(Size _size, const Scalar& _s);
	Mat(int _rows, int _cols, _Tp* _data, bool _alloc = false);
	Mat(const Mat<_Tp, chs>& _m);
	Mat(const Mat<_Tp, chs>& _m, const Range& _rowRange, const Range& _colRange = Range::all());
	Mat(const Mat<_Tp, chs>& _m, const Rect& _roi);

	Mat& operator = (const Mat& _m);

	Mat<_Tp, chs> row(int _y) const;
	Mat<_Tp, chs> col(int _x) const;
	Mat<_Tp, chs> rowRange(int _startrow, int _endrow) const;
	Mat<_Tp, chs> rowRange(const Range& _r) const;
	Mat<_Tp, chs> colRange(int _startcol, int _endcol) const;
	Mat<_Tp, chs> colRange(const Range& _r) const;

	Mat<_Tp, chs> clone();
	void copyTo(Mat<_Tp, chs>& _m) const;

	Mat<_Tp, chs>& setTo(const Scalar& _value);

	static Mat<_Tp, chs> zeros(int _rows, int _cols);
	static Mat<_Tp, chs> ones(int _rows, int _cols);
	static Mat<_Tp, chs> eye(int _rows, int _cols);

	Mat<_Tp, chs> operator()(Range _rowRange, Range _colRange) const;
	Mat<_Tp, chs> operator()(const Rect& _roi) const;
	Mat<_Tp, chs> operator()(const Range* _ranges) const;

	_Tp* ptr(int i0 = 0);

	void deallocate();
	void release();

	~Mat() { release(); };

	int rows;
	int cols;
	int channels;
	_Tp* data;
	int step; // stride
	bool allocated;

protected:
	bool create(int rows, int cols);
	bool allocate();

}; // Mat

template<typename _Tp, int chs> inline
void Mat<_Tp, chs>::release()
{
	if (this->data && this->allocated) {
		delete[] this->data;
	}

	this->data = NULL;
	this->allocated = false;
}

template<typename _Tp, int chs> inline
Mat<_Tp, chs>::Mat(int _rows, int _cols)
{
	FBC_Assert(_rows > 0 && _cols > 0 && chs > 0);

	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = true;

	int length = _rows * _cols * chs;
	_Tp* _data = new _Tp[length];
	FBC_Assert(_data != NULL);
	memset(_data, 0, length * sizeof(_Tp));

	this->data = _data;
}

template<typename _Tp, int chs> inline
Mat<_Tp, chs>::Mat(int _rows, int _cols, const Scalar& _s)
{
	FBC_Assert(_rows > 0 && _cols > 0 && chs > 0);

	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = true;

	int length = _rows * _cols * chs;
	_Tp* _data = new _Tp[length];
	FBC_Assert(_data != NULL);

	memset(_data, 0, length * sizeof(_Tp));

	for (int i = 0; i < _rows; i++) {
		_Tp* pRow = _data + i * _cols * chs;

		for (int j = 0; j < _cols; j++) {
			_Tp* pPixel = pRow + j * chs;

			for (int m = 0, n = 0; m < chs && n < 4; m++, n++) {
				pPixel[n] = static_cast<_Tp>(_s.val[n]);
			}
		}
	}

	this->data = _data;
}

//template<typename _Tp> inline
//Mat<_Tp> Mat<_Tp>::clone()
//{
//	Mat<_Tp> mat_;
//
//	mat_.rows = this->rows;
//	mat_.cols = this->cols;
//	mat_.channels = this->channels;
//	mat_.step = this->step;
//
//	int data_length = this->step * this->rows;
//	_Tp* data_ = new _Tp[data_length];
//	memcpy(data_, this->data, data_length);
//
//	mat_.data = data_;
//
//	return mat_;
//}
//
//template<typename _Tp> inline
//Mat<_Tp>::~Mat()
//{
//	destroy();
//}
//
//template<typename _Tp> inline
//void Mat<_Tp>::destroy()
//{
//	this->rows = this->cols = this->channels = this->step = 0;
//	delete[] data;
//	data = NULL;
//}

} // fbc

#endif // FBC_CV_CORE_MAT_HPP_
