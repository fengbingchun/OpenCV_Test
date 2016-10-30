// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_FILTER_ENGINE_HPP_
#define FBC_CV_FILTER_ENGINE_HPP_

/* reference: modules/imgproc/src/filterengine.hpp
              modules/imgproc/src/filter.cpp
*/

#include <vector>
#include "core/mat.hpp"
#include "core/Ptr.hpp"
#include "core/fbcdef.hpp"

namespace fbc {

// type of the kernel
enum {
	KERNEL_GENERAL = 0, // the kernel is generic. No any type of symmetry or other properties.
	KERNEL_SYMMETRICAL = 1, // kernel[i] == kernel[ksize-i-1] , and the anchor is at the center
	KERNEL_ASYMMETRICAL = 2, // kernel[i] == -kernel[ksize-i-1] , and the anchor is at the center
	KERNEL_SMOOTH = 4, // all the kernel elements are non-negative and summed to 1
	KERNEL_INTEGER = 8  // all the kernel coefficients are integer numbers
};

// The Base Class for 1D or Row-wise Filters
class BaseRowFilter {
public:
	// the default constructor
	BaseRowFilter() { ksize = anchor = -1; }
	// the destructor
	virtual ~BaseRowFilter() {}
	// the filtering operator. Must be overridden in the derived classes. The horizontal border interpolation is done outside of the class.
	virtual void operator()(const uchar* src, uchar* dst, int width, int cn) = 0;

	int ksize;
	int anchor;
};

// The Base Class for Column-wise Filters
class BaseColumnFilter {
public:
	// the default constructor
	BaseColumnFilter() { ksize = anchor = -1; }
	// the destructor
	virtual ~BaseColumnFilter() {}
	// the filtering operator. Must be overridden in the derived classes. The vertical border interpolation is done outside of the class.
	virtual void operator()(const uchar** src, uchar* dst, int dststep, int dstcount, int width) = 0;
	// resets the internal buffers, if any
	virtual void reset() {}

	int ksize;
	int anchor;
};

// The Base Class for Non-Separable 2D Filters.
class BaseFilter {
public:
	// the default constructor
	BaseFilter() { ksize = Size(-1, -1); anchor = Point(-1, -1); }
	// the destructor
	virtual ~BaseFilter() {}
	// the filtering operator. The horizontal and the vertical border interpolation is done outside of the class.
	virtual void operator()(const uchar** src, uchar* dst, int dststep, int dstcount, int width, int cn) = 0;
	// resets the internal buffers, if any
	virtual void reset() {}

	Size ksize;
	Point anchor;
};

// The Main Class for Image Filtering.
// The class can be used to apply an arbitrary filtering operation to an image.
template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3> // srcType, dstType, bufferType, srcChs, dstChs, bufferChs
class FilterEngine {
public:
	// the default constructor
	FilterEngine();
	// the full constructor. Either _filter2D or both _rowFilter and _columnFilter must be non-empty.
	FilterEngine(const Ptr<BaseFilter>& _filter2D, const Ptr<BaseRowFilter>& _rowFilter, const Ptr<BaseColumnFilter>& _columnFilter,
		int _rowBorderType = BORDER_REPLICATE, int _columnBorderType = -1, const Scalar& _borderValue = Scalar());
	// the destructor
	virtual ~FilterEngine();
	// reinitializes the engine. The previously assigned filters are released.
	void init(const Ptr<BaseFilter>& _filter2D, const Ptr<BaseRowFilter>& _rowFilter, const Ptr<BaseColumnFilter>& _columnFilter,
		int _rowBorderType = BORDER_REPLICATE, int _columnBorderType = -1, const Scalar& _borderValue = Scalar());
	// starts filtering of the specified ROI of an image of size wholeSize.
	virtual int start(Size wholeSize, Rect roi, int maxBufRows = -1);
	// starts filtering of the specified ROI of the specified image.
	virtual int start(const Mat_<_Tp1, chs1>& src, const Rect& srcRoi = Rect(0, 0, -1, -1), bool isolated = false, int maxBufRows = -1);
	// processes the next srcCount rows of the image.
	virtual int proceed(const uchar* src, int srcStep, int srcCount, uchar* dst, int dstStep);
	// applies filter to the specified ROI of the image. if srcRoi=(0,0,-1,-1), the whole image is filtered.
	virtual void apply(const Mat_<_Tp1, chs1>& src, Mat_<_Tp2, chs2>& dst,
		const Rect& srcRoi = Rect(0, 0, -1, -1), Point dstOfs = Point(0, 0), bool isolated = false);
	// returns true if the filter is separable
	bool isSeparable() const { return !filter2D; }
	// returns the number
	int remainingInputRows() const;
	int remainingOutputRows() const;

	Size ksize;
	Point anchor;
	int maxWidth;
	Size wholeSize;
	Rect roi;
	int dx1;
	int dx2;
	int rowBorderType;
	int columnBorderType;
	std::vector<int> borderTab;
	int borderElemSize;
	std::vector<uchar> ringBuf;
	std::vector<uchar> srcRow;
	std::vector<uchar> constBorderValue;
	std::vector<uchar> constBorderRow;
	int bufStep;
	int startY;
	int startY0;
	int endY;
	int rowCount;
	int dstY;
	std::vector<uchar*> rows;

	Ptr<BaseFilter> filter2D;
	Ptr<BaseRowFilter> rowFilter;
	Ptr<BaseColumnFilter> columnFilter;
};

static inline Point normalizeAnchor(Point anchor, Size ksize)
{
	if (anchor.x == -1)
		anchor.x = ksize.width / 2;
	if (anchor.y == -1)
		anchor.y = ksize.height / 2;
	FBC_Assert(anchor.inside(Rect(0, 0, ksize.width, ksize.height)));

	return anchor;
}

template<typename _Tp, int chs>
void preprocess2DKernel(const Mat_<_Tp, chs>& kernel, std::vector<Point>& coords, std::vector<uchar>& coeffs)
{
	int i, j, k, nz = countNonZero<_Tp, chs>(kernel);
	if (nz == 0)
		nz = 1;
	FBC_Assert(typeid(uchar).name() == typeid(_Tp).name() || typeid(float).name() == typeid(_Tp).name()); // uchar || float
	coords.resize(nz);
	coeffs.resize(nz * sizeof(_Tp) * chs);
	uchar* _coeffs = &coeffs[0];

	for (i = k = 0; i < kernel.rows; i++) {
		const uchar* krow = kernel.ptr(i);
		for (j = 0; j < kernel.cols; j++) {
			if (typeid(uchar).name() == typeid(_Tp).name())
			{
				uchar val = krow[j];
				if (val == 0)
					continue;
				coords[k] = Point(j, i);
				_coeffs[k++] = val;
			} else {
				float val = ((const float*)krow)[j];
				if (val == 0)
					continue;
				coords[k] = Point(j, i);
				((float*)_coeffs)[k++] = val;
			}
		}
	}
}

//////////////////////////// FilterEngine impl ///////////////////////////////
template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::FilterEngine()
{
	rowBorderType = columnBorderType = BORDER_REPLICATE;
	bufStep = startY = startY0 = endY = rowCount = dstY = 0;
	maxWidth = 0;

	wholeSize = Size(-1, -1);
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::FilterEngine(const Ptr<BaseFilter>& _filter2D, const Ptr<BaseRowFilter>& _rowFilter, const Ptr<BaseColumnFilter>& _columnFilter,
	int _rowBorderType, int _columnBorderType, const Scalar& _borderValue)
{
	init(_filter2D, _rowFilter, _columnFilter, _rowBorderType, _columnBorderType, _borderValue);
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::~FilterEngine()
{
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
void FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::init(const Ptr<BaseFilter>& _filter2D, const Ptr<BaseRowFilter>& _rowFilter, const Ptr<BaseColumnFilter>& _columnFilter,
	int _rowBorderType, int _columnBorderType, const Scalar& _borderValue)
{
	int srcElemSize = sizeof(_Tp1) * chs1;

	filter2D = _filter2D;
	rowFilter = _rowFilter;
	columnFilter = _columnFilter;

	if (_columnBorderType < 0)
		_columnBorderType = _rowBorderType;

	rowBorderType = _rowBorderType;
	columnBorderType = _columnBorderType;

	FBC_Assert(columnBorderType != BORDER_WRAP);

	if (isSeparable()) {
		FBC_Assert(rowFilter && columnFilter);
		ksize = Size(rowFilter->ksize, columnFilter->ksize);
		anchor = Point(rowFilter->anchor, columnFilter->anchor);
	} else {
		FBC_Assert(typeid(_Tp3).name() == typeid(_Tp1).name() && chs3 == chs1);
		ksize = filter2D->ksize;
		anchor = filter2D->anchor;
	}

	FBC_Assert(0 <= anchor.x && anchor.x < ksize.width && 0 <= anchor.y && anchor.y < ksize.height);

	borderElemSize = srcElemSize / (sizeof(_Tp1) >= 4 ? sizeof(int) : 1);
	int borderLength = std::max(ksize.width - 1, 1);
	borderTab.resize(borderLength*borderElemSize);

	maxWidth = bufStep = 0;
	constBorderRow.clear();

	if (rowBorderType == BORDER_CONSTANT || columnBorderType == BORDER_CONSTANT) {
		constBorderValue.resize(srcElemSize*borderLength);
		scalarToRawData<_Tp1, chs1>(_borderValue, &constBorderValue[0], borderLength*chs1);
	}

	wholeSize = Size(-1, -1);
}

// the alignment of all the allocated buffers
#define FBC_MALLOC_ALIGN	16
#define VEC_ALIGN		FBC_MALLOC_ALIGN

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
int FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::start(Size _wholeSize, Rect _roi, int _maxBufRows)
{
	int i, j;

	wholeSize = _wholeSize;
	roi = _roi;
	FBC_Assert(roi.x >= 0 && roi.y >= 0 && roi.width >= 0 && roi.height >= 0 &&
		roi.x + roi.width <= wholeSize.width &&
		roi.y + roi.height <= wholeSize.height);

	int esz = sizeof(_Tp1) * chs1;
	int bufElemSize = sizeof(_Tp3) * chs3;
	const uchar* constVal = !constBorderValue.empty() ? &constBorderValue[0] : 0;

	if (_maxBufRows < 0)
		_maxBufRows = ksize.height + 3;
	_maxBufRows = std::max(_maxBufRows, std::max(anchor.y, ksize.height - anchor.y - 1) * 2 + 1);

	if (maxWidth < roi.width || _maxBufRows != (int)rows.size()) {
		rows.resize(_maxBufRows);
		maxWidth = std::max(maxWidth, roi.width);
		int cn = chs1;
		srcRow.resize(esz*(maxWidth + ksize.width - 1));
		if (columnBorderType == BORDER_CONSTANT) {
			constBorderRow.resize((sizeof(_Tp3) * chs3)*(maxWidth + ksize.width - 1 + VEC_ALIGN));
			uchar *dst = alignPtr(&constBorderRow[0], VEC_ALIGN), *tdst;
			int n = (int)constBorderValue.size(), N;
			N = (maxWidth + ksize.width - 1)*esz;
			tdst = isSeparable() ? &srcRow[0] : dst;

			for (i = 0; i < N; i += n) {
				n = std::min(n, N - i);
				for (j = 0; j < n; j++)
					tdst[i + j] = constVal[j];
			}

			if (isSeparable())
				(*rowFilter)(&srcRow[0], dst, maxWidth, cn);
		}

		int maxBufStep = bufElemSize*(int)alignSize(maxWidth +
			(!isSeparable() ? ksize.width - 1 : 0), VEC_ALIGN);
		ringBuf.resize(maxBufStep*rows.size() + VEC_ALIGN);
	}

	// adjust bufstep so that the used part of the ring buffer stays compact in memory
	bufStep = bufElemSize*(int)alignSize(roi.width + (!isSeparable() ? ksize.width - 1 : 0), 16);

	dx1 = std::max(anchor.x - roi.x, 0);
	dx2 = std::max(ksize.width - anchor.x - 1 + roi.x + roi.width - wholeSize.width, 0);

	// recompute border tables
	if (dx1 > 0 || dx2 > 0) {
		if (rowBorderType == BORDER_CONSTANT) {
			int nr = isSeparable() ? 1 : (int)rows.size();
			for (i = 0; i < nr; i++) {
				uchar* dst = isSeparable() ? &srcRow[0] : alignPtr(&ringBuf[0], VEC_ALIGN) + bufStep*i;
				memcpy(dst, constVal, dx1*esz);
				memcpy(dst + (roi.width + ksize.width - 1 - dx2)*esz, constVal, dx2*esz);
			}
		} else {
			int xofs1 = std::min(roi.x, anchor.x) - roi.x;

			int btab_esz = borderElemSize, wholeWidth = wholeSize.width;
			int* btab = (int*)&borderTab[0];

			for (i = 0; i < dx1; i++) {
				int p0 = (borderInterpolate<int>(i - dx1, wholeWidth, rowBorderType) + xofs1)*btab_esz;
				for (j = 0; j < btab_esz; j++)
					btab[i*btab_esz + j] = p0 + j;
			}

			for (i = 0; i < dx2; i++) {
				int p0 = (borderInterpolate<int>(wholeWidth + i, wholeWidth, rowBorderType) + xofs1)*btab_esz;
				for (j = 0; j < btab_esz; j++)
					btab[(i + dx1)*btab_esz + j] = p0 + j;
			}
		}
	}

	rowCount = dstY = 0;
	startY = startY0 = std::max(roi.y - anchor.y, 0);
	endY = std::min(roi.y + roi.height + ksize.height - anchor.y - 1, wholeSize.height);
	if (columnFilter)
		columnFilter->reset();
	if (filter2D)
		filter2D->reset();

	return startY;
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
int FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::start(const Mat_<_Tp1, chs1>& src, const Rect& _srcRoi, bool isolated, int maxBufRows)
{
	Rect srcRoi = _srcRoi;

	if (srcRoi == Rect(0, 0, -1, -1))
		srcRoi = Rect(0, 0, src.cols, src.rows);

	FBC_Assert(srcRoi.x >= 0 && srcRoi.y >= 0 &&
		srcRoi.width >= 0 && srcRoi.height >= 0 &&
		srcRoi.x + srcRoi.width <= src.cols &&
		srcRoi.y + srcRoi.height <= src.rows);

	Point ofs;
	Size wsz(src.cols, src.rows);
	if (!isolated)
		src.locateROI(wsz, ofs);
	start(wsz, srcRoi + ofs, maxBufRows);

	return startY - ofs.y;
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
int FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::remainingInputRows() const
{
	return endY - startY - rowCount;
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
int FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::remainingOutputRows() const
{
	return roi.height - dstY;
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
int FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::proceed(const uchar* src, int srcstep, int count, uchar* dst, int dststep)
{
	FBC_Assert(wholeSize.width > 0 && wholeSize.height > 0);

	const int *btab = &borderTab[0];
	int esz = sizeof(_Tp1) * chs1, btab_esz = borderElemSize;
	uchar** brows = &rows[0];
	int bufRows = (int)rows.size();
	int cn = chs3;
	int width = roi.width, kwidth = ksize.width;
	int kheight = ksize.height, ay = anchor.y;
	int _dx1 = dx1, _dx2 = dx2;
	int width1 = roi.width + kwidth - 1;
	int xofs1 = std::min(roi.x, anchor.x);
	bool isSep = isSeparable();
	bool makeBorder = (_dx1 > 0 || _dx2 > 0) && rowBorderType != BORDER_CONSTANT;
	int dy = 0, i = 0;

	src -= xofs1*esz;
	count = std::min(count, remainingInputRows());

	FBC_Assert(src && dst && count > 0);

	for (;; dst += dststep*i, dy += i) {
		int dcount = bufRows - ay - startY - rowCount + roi.y;
		dcount = dcount > 0 ? dcount : bufRows - kheight + 1;
		dcount = std::min(dcount, count);
		count -= dcount;
		for (; dcount-- > 0; src += srcstep) {
			int bi = (startY - startY0 + rowCount) % bufRows;
			uchar* brow = alignPtr(&ringBuf[0], VEC_ALIGN) + bi*bufStep;
			uchar* row = isSep ? &srcRow[0] : brow;

			if (++rowCount > bufRows) {
				--rowCount;
				++startY;
			}

			memcpy(row + _dx1*esz, src, (width1 - _dx2 - _dx1)*esz);

			if (makeBorder) {
				if (btab_esz*(int)sizeof(int) == esz) {
					const int* isrc = (const int*)src;
					int* irow = (int*)row;

					for (i = 0; i < _dx1*btab_esz; i++)
						irow[i] = isrc[btab[i]];
					for (i = 0; i < _dx2*btab_esz; i++)
						irow[i + (width1 - _dx2)*btab_esz] = isrc[btab[i + _dx1*btab_esz]];
				} else {
					for (i = 0; i < _dx1*esz; i++)
						row[i] = src[btab[i]];
					for (i = 0; i < _dx2*esz; i++)
						row[i + (width1 - _dx2)*esz] = src[btab[i + _dx1*esz]];
				}
			}

			if (isSep)
				(*rowFilter)(row, brow, width, chs1);
		}

		int max_i = std::min(bufRows, roi.height - (dstY + dy) + (kheight - 1));
		for (i = 0; i < max_i; i++) {
			int srcY = borderInterpolate<int>(dstY + dy + i + roi.y - ay,
				wholeSize.height, columnBorderType);
			if (srcY < 0) { // can happen only with constant border type
				brows[i] = alignPtr(&constBorderRow[0], VEC_ALIGN);
			} else {
				FBC_Assert(srcY >= startY);
				if (srcY >= startY + rowCount)
					break;
				int bi = (srcY - startY0) % bufRows;
				brows[i] = alignPtr(&ringBuf[0], VEC_ALIGN) + bi*bufStep;
			}
		}
		if (i < kheight)
			break;
		i -= kheight - 1;
		if (isSeparable())
			(*columnFilter)((const uchar**)brows, dst, dststep, i, roi.width*cn);
		else
			(*filter2D)((const uchar**)brows, dst, dststep, i, roi.width, cn);
	}

	dstY += dy;
	FBC_Assert(dstY <= roi.height);
	return dy;
}

template <typename _Tp1, typename _Tp2, typename _Tp3, int chs1, int chs2, int chs3>
void FilterEngine<_Tp1, _Tp2, _Tp3, chs1, chs2, chs3>::apply(const Mat_<_Tp1, chs1>& src, Mat_<_Tp2, chs2>& dst, const Rect& _srcRoi, Point dstOfs, bool isolated)
{
	Rect srcRoi = _srcRoi;
	if (srcRoi == Rect(0, 0, -1, -1))
		srcRoi = Rect(0, 0, src.cols, src.rows);

	if (srcRoi.area() == 0)
		return;

	FBC_Assert(dstOfs.x >= 0 && dstOfs.y >= 0 &&
		dstOfs.x + srcRoi.width <= dst.cols &&
		dstOfs.y + srcRoi.height <= dst.rows);

	int y = start(src, srcRoi, isolated);
	proceed(src.ptr() + y*src.step + srcRoi.x*src.elemSize(),
		(int)src.step, endY - startY,
		dst.ptr(dstOfs.y) +
		dstOfs.x*dst.elemSize(), (int)dst.step);
}

} // namespace fbc

#endif // FBC_CV_FILTER_ENGINE_HPP_
