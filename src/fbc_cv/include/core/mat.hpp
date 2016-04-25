#ifndef FBC_CV_CORE_MAT_HPP_
#define FBC_CV_CORE_MAT_HPP_

// reference: include/opencv2/core/mat.hpp

#ifndef __cplusplus
	#error mat.hpp header must be compiled as C++
#endif

#include "core/fbcdef.hpp"
#include "core/types.hpp"

namespace fbc {
template<typename _Tp, int chs> class Mat {
public:
	typedef _Tp value_type;

	Mat() : rows(0), cols(0), data(NULL), step(0), allocated(false) {}
	Mat(int rows, int cols);
	Mat(int rows, int cols, const Scalar& s);
	Mat(Size size, const Scalar& s);
	Mat(int rows, int cols, _Tp* data, bool alloc = false);
	Mat(const Mat& m);
	//Mat(const Mat& m, const Range& rowRange, const Range& colRange = Range::all());
	Mat(const Mat& m, const Rect& roi);

	Mat& operator = (const Mat& m);

	Mat row(int y) const;
	Mat col(int x) const;
	Mat rowRange(int startrow, int endrow) const;
	//Mat rowRange(const Range& r) const;
	Mat colRange(int startcol, int endcol) const;
	//Mat colRange(const Range& r) const;

	Mat clone();
	void copyTo(Mat& m) const;

	Mat& setTo(const Scalar& value);

	static Mat zeros(int rows, int cols);
	static Mat ones(int rows, int cols);
	static Mat eye(int rows, int cols);

	//Mat operator()(Range rowRange, Range colRange) const;
	Mat operator()(const Rect& roi) const;
	//Mat operator()(const Range* ranges) const;

	_Tp* ptr(int i0 = 0);

	void deallocate();
	void release();

	~Mat();

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

//template<typename _Tp> inline
//Mat<_Tp>::Mat(): rows(0), cols(0), channels(0), data(NULL), step(0){}
//
//template<typename _Tp> inline
//Mat<_Tp>::Mat(int rows, int cols, int channels)
//{
//	this->rows = rows;
//	this->cols = cols;
//	this->channels = channels;
//	this->step = sizeof(_Tp) * channels;
//
//	int data_length = this->step * this->rows;
//	_Tp* data_ = new _Tp[data_length];
//	memset(data_, 0, data_length);
//
//	this->data = data_;
//}
//
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
