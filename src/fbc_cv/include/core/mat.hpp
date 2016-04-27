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

template<typename _Tp, int chs> class Mat_ {
public:
	typedef _Tp value_type;

	// default constructor
	Mat_() : rows(0), cols(0), channels(0), data(NULL), step(0), allocated(false) {}
	// constructs 2D matrix of the specified size
	Mat_(int _rows, int _cols);
	// constucts 2D matrix and fills it with the specified value _s
	Mat_(int _rows, int _cols, const Scalar& _s);
	Mat_(Size _size, const Scalar& _s);
	// constructor for matrix headers pointing to user-allocated data
	Mat_(int _rows, int _cols, _Tp* _data);
	// copy constructor
	Mat_(const Mat_<_Tp, chs>& _m);
	//Mat_(const Mat_<_Tp, chs>& _m, const Range& _rowRange, const Range& _colRange = Range::all());
	//Mat_(const Mat_<_Tp, chs>& _m, const Rect& _roi);
	//Mat_& operator = (const Mat_& _m);

	//Mat_<_Tp, chs> row(int _y) const;
	//Mat_<_Tp, chs> col(int _x) const;
	//Mat_<_Tp, chs> rowRange(int _startrow, int _endrow) const;
	//Mat_<_Tp, chs> rowRange(const Range& _r) const;
	//Mat_<_Tp, chs> colRange(int _startcol, int _endcol) const;
	//Mat_<_Tp, chs> colRange(const Range& _r) const;

	// returns deep copy of the matrix, i.e. the data is copied
	Mat_<_Tp, chs> clone();
	// copies the matrix content to "_m"
	void copyTo(Mat_<_Tp, chs>& _m) const;

	//Mat_<_Tp, chs>& setTo(const Scalar& _value);

	//static Mat_<_Tp, chs> zeros(int _rows, int _cols);
	//static Mat_<_Tp, chs> ones(int _rows, int _cols);
	//static Mat_<_Tp, chs> eye(int _rows, int _cols);

	//Mat_<_Tp, chs> operator()(Range _rowRange, Range _colRange) const;
	//Mat_<_Tp, chs> operator()(const Rect& _roi) const;
	//Mat_<_Tp, chs> operator()(const Range* _ranges) const;
	// returns pointer to i0-th submatrix along the dimension #0
	inline _Tp* ptr(int i0 = 0);

	//void deallocate();
	// release memory
	inline void release();
	// destructor - calls release()
	~Mat_() { release(); };

	// the number of rows and columns
	int rows£¬cols;
	// channel num
	int channels;
	// pointer to the data
	_Tp* data;
	// bytes per row
	int step; // stride
	// memory allocation flag
	bool allocated;

protected:
	//bool create(int rows, int cols);
	//bool allocate();

}; // Mat

typedef Mat_<uchar, 1> Mat1Gray;
typedef Mat_<uchar, 3> Mat3BGR;
typedef Mat_<uchar, 4> Mat4BGRA;

template<typename _Tp, int chs> inline
void Mat_<_Tp, chs>::release()
{
	if (this->data && this->allocated) {
		delete[] this->data;
	}

	this->data = NULL;
	this->allocated = false;
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

	int length = _rows * _cols * chs;
	_Tp* _data = new _Tp[length];
	FBC_Assert(_data != NULL);
	memset(_data, 0, length * sizeof(_Tp));

	this->data = _data;
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

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(Size _size, const Scalar& _s)
{
	Mat_<_Tp, chs>(_size.height, _size.width, _s);
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(int _rows, int _cols, _Tp* _data)
{
	FBC_Assert(_rows > 0 && _cols > 0 && chs > 0);

	this->rows = _rows;
	this->cols = _cols;
	this->channels = chs;
	this->step = sizeof(_Tp) * _cols * chs;
	this->allocated = false;
	this->data = _data;
}

template<typename _Tp, int chs>
Mat_<_Tp, chs>::Mat_(const Mat_<_Tp, chs>& _m)
{
	FBC_Assert(_m.rows > 0 && _m.cols > 0 && _m.channels > 0);

	this->rows = _m.rows;
	this->cols = _m.cols;
	this->channels = _m.channels;
	this->step = _m.step;
	this->allocated = true;

	int length = _m.cols * _m.rows * _m.channels;
	_Tp* _data = new _Tp[length];
	memcpy(_data, _m.data, length * sizeof(_Tp));
	this->data = _data;
}

template<typename _Tp, int chs>
Mat_<_Tp, chs> Mat_<_Tp, chs>::clone()
{
	FBC_Assert(this->rows > 0 && this->cols > 0 && this->channels > 0);

	Mat_<_Tp, chs> mat_ = Mat_(this->rows, this->cols);
	memcpy(mat_.data, this->data, this->rows * this->step);

	return mat_;
}

template<typename _Tp, int chs>
void Mat_<_Tp, chs>::copyTo(Mat_<_Tp, chs>& _m) const
{
	FBC_Assert(this->rows > 0 && this->cols > 0 && this->channels > 0);

	_m = Mat_(this->rows, this->cols);
	memcpy(_m.data, this->data, this->rows * this->step);
}

template<typename _Tp, int chs>
_Tp* Mat_<_Tp, chs>::ptr(int i0)
{
	FBC_Assert(i0 < this->rows);

	return this->data + i0 * this->step;
}

} // fbc

#endif // FBC_CV_CORE_MAT_HPP_
