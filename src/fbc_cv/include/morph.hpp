// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_MORPH_HPP_
#define FBC_CV_MORPH_HPP_

/* reference:  modules/imgproc/src/filterengine.hpp
              modules/imgproc/src/morph.cpp
*/

#include <typeinfo>
#include "core/mat.hpp"
#include "core/Ptr.hpp"
#include "imgproc.hpp"
#include "filterengine.hpp"

namespace fbc {

template<typename T> struct MinOp
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator ()(const T a, const T b) const { return std::min(a, b); }
};

template<typename T> struct MaxOp
{
	typedef T type1;
	typedef T type2;
	typedef T rtype;
	T operator ()(const T a, const T b) const { return std::max(a, b); }
};

#undef CV_MIN_8U
#undef CV_MAX_8U
#define CV_FAST_CAST_8U(t)	(assert(-256 <= (t) && (t) <= 512), icvSaturate8u_cv[(t)+256])
#define CV_MIN_8U(a,b)		((a) - CV_FAST_CAST_8U((a) - (b)))
#define CV_MAX_8U(a,b)		((a) + CV_FAST_CAST_8U((b) - (a)))

template<> inline uchar MinOp<uchar>::operator ()(const uchar a, const uchar b) const { return CV_MIN_8U(a, b); }
template<> inline uchar MaxOp<uchar>::operator ()(const uchar a, const uchar b) const { return CV_MAX_8U(a, b); }

struct MorphRowNoVec
{
	MorphRowNoVec(int, int) {}
	int operator()(const uchar*, uchar*, int, int) const { return 0; }
};

struct MorphColumnNoVec
{
	MorphColumnNoVec(int, int) {}
	int operator()(const uchar**, uchar*, int, int, int) const { return 0; }
};

struct MorphNoVec
{
	int operator()(uchar**, int, uchar*, int) const { return 0; }
};

template<class Op, class VecOp> struct MorphRowFilter : public BaseRowFilter
{
	typedef typename Op::rtype T;

	MorphRowFilter(int _ksize, int _anchor) : vecOp(_ksize, _anchor)
	{
		ksize = _ksize;
		anchor = _anchor;
	}

	void operator()(const uchar* src, uchar* dst, int width, int cn)
	{
		int i, j, k, _ksize = ksize*cn;
		const T* S = (const T*)src;
		Op op;
		T* D = (T*)dst;

		if (_ksize == cn) {
			for (i = 0; i < width*cn; i++)
				D[i] = S[i];
			return;
		}

		int i0 = vecOp(src, dst, width, cn);
		width *= cn;

		for (k = 0; k < cn; k++, S++, D++) {
			for (i = i0; i <= width - cn * 2; i += cn * 2) {
				const T* s = S + i;
				T m = s[cn];
				for (j = cn * 2; j < _ksize; j += cn)
					m = op(m, s[j]);
				D[i] = op(m, s[0]);
				D[i + cn] = op(m, s[j]);
			}

			for (; i < width; i += cn) {
				const T* s = S + i;
				T m = s[0];
				for (j = cn; j < _ksize; j += cn)
					m = op(m, s[j]);
				D[i] = m;
			}
		}
	}

	VecOp vecOp;
};

template<class Op, class VecOp> struct MorphColumnFilter : public BaseColumnFilter
{
	typedef typename Op::rtype T;

	MorphColumnFilter(int _ksize, int _anchor) : vecOp(_ksize, _anchor)
	{
		ksize = _ksize;
		anchor = _anchor;
	}

	void operator()(const uchar** _src, uchar* dst, int dststep, int count, int width)
	{
		int i, k, _ksize = ksize;
		const T** src = (const T**)_src;
		T* D = (T*)dst;
		Op op;

		int i0 = vecOp(_src, dst, dststep, count, width);
		dststep /= sizeof(D[0]);

		for (; _ksize > 1 && count > 1; count -= 2, D += dststep * 2, src += 2) {
			i = i0;
			for (; i <= width - 4; i += 4) {
				const T* sptr = src[1] + i;
				T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

				for (k = 2; k < _ksize; k++) {
					sptr = src[k] + i;
					s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
					s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
				}

				sptr = src[0] + i;
				D[i] = op(s0, sptr[0]);
				D[i + 1] = op(s1, sptr[1]);
				D[i + 2] = op(s2, sptr[2]);
				D[i + 3] = op(s3, sptr[3]);

				sptr = src[k] + i;
				D[i + dststep] = op(s0, sptr[0]);
				D[i + dststep + 1] = op(s1, sptr[1]);
				D[i + dststep + 2] = op(s2, sptr[2]);
				D[i + dststep + 3] = op(s3, sptr[3]);
			}

			for (; i < width; i++) {
				T s0 = src[1][i];

				for (k = 2; k < _ksize; k++)
					s0 = op(s0, src[k][i]);

				D[i] = op(s0, src[0][i]);
				D[i + dststep] = op(s0, src[k][i]);
			}
		}

		for (; count > 0; count--, D += dststep, src++) {
			i = i0;
			for (; i <= width - 4; i += 4) {
				const T* sptr = src[0] + i;
				T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

				for (k = 1; k < _ksize; k++) {
					sptr = src[k] + i;
					s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
					s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
				}

				D[i] = s0; D[i + 1] = s1;
				D[i + 2] = s2; D[i + 3] = s3;
			}

			for (; i < width; i++) {
				T s0 = src[0][i];
				for (k = 1; k < _ksize; k++)
					s0 = op(s0, src[k][i]);
				D[i] = s0;
			}
		}
	}

	VecOp vecOp;
};

template<class Op, class VecOp> struct MorphFilter : BaseFilter
{
	typedef typename Op::rtype T;

	MorphFilter(const Mat_<uchar, 1>& _kernel, Point _anchor)
	{
		anchor = _anchor;
		ksize = _kernel.size();

		std::vector<uchar> coeffs; // we do not really the values of non-zero
		// kernel elements, just their locations
		preprocess2DKernel<uchar, 1>(_kernel, coords, coeffs);
		ptrs.resize(coords.size());
	}

	void operator()(const uchar** src, uchar* dst, int dststep, int count, int width, int cn)
	{
		const Point* pt = &coords[0];
		const T** kp = (const T**)&ptrs[0];
		int i, k, nz = (int)coords.size();
		Op op;

		width *= cn;
		for (; count > 0; count--, dst += dststep, src++) {
			T* D = (T*)dst;

			for (k = 0; k < nz; k++)
				kp[k] = (const T*)src[pt[k].y] + pt[k].x*cn;

			i = vecOp(&ptrs[0], nz, dst, width);

			for (; i <= width - 4; i += 4) {
				const T* sptr = kp[0] + i;
				T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

				for (k = 1; k < nz; k++) {
					sptr = kp[k] + i;
					s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
					s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
				}

				D[i] = s0; D[i + 1] = s1;
				D[i + 2] = s2; D[i + 3] = s3;
			}

			for (; i < width; i++) {
				T s0 = kp[0][i];
				for (k = 1; k < nz; k++)
					s0 = op(s0, kp[k][i]);
				D[i] = s0;
			}
		}
	}

	std::vector<Point> coords;
	std::vector<uchar*> ptrs;
	VecOp vecOp;
};

// returns horizontal 1D morphological filter
template<typename _Tp, int chs>
Ptr<BaseRowFilter> getMorphologyRowFilter(int op, int ksize, int anchor = -1)
{
	if (anchor < 0)
		anchor = ksize / 2;
	FBC_Assert(op == MORPH_ERODE || op == MORPH_DILATE);

	if (op == MORPH_ERODE) {
		if (typeid(uchar).name() == typeid(_Tp).name()) {
			return makePtr<MorphRowFilter<MinOp<uchar>, MorphRowNoVec> >(ksize, anchor);
		}
		if (typeid(float).name() == typeid(_Tp).name()) {
			return makePtr<MorphRowFilter<MinOp<float>, MorphRowNoVec> >(ksize, anchor);
		}
	}
	else {
		if (typeid(uchar).name() == typeid(_Tp).name()) {
			return makePtr<MorphRowFilter<MaxOp<uchar>, MorphRowNoVec> >(ksize, anchor);
		}
		if (typeid(float).name() == typeid(_Tp).name()) {
			return makePtr<MorphRowFilter<MaxOp<float>, MorphRowNoVec> >(ksize, anchor);
		}
	}

	FBC_Error("Unsupported data type");
	return Ptr<BaseRowFilter>();
}

// returns vertical 1D morphological filter
template<typename _Tp, int chs>
Ptr<BaseColumnFilter> getMorphologyColumnFilter(int op, int ksize, int anchor = -1)
{
	if (anchor < 0)
		anchor = ksize / 2;
	FBC_Assert(op == MORPH_ERODE || op == MORPH_DILATE);

	if (op == MORPH_ERODE) {
		if (typeid(uchar).name() == typeid(_Tp).name()) {
			return makePtr<MorphColumnFilter<MinOp<uchar>, MorphColumnNoVec> >(ksize, anchor);
		}
		if (typeid(float).name() == typeid(_Tp).name()) {
			return makePtr<MorphColumnFilter<MinOp<float>, MorphColumnNoVec> >(ksize, anchor);
		}
	} else {
		if (typeid(uchar).name() == typeid(_Tp).name()) {
			return makePtr<MorphColumnFilter<MaxOp<uchar>, MorphColumnNoVec> >(ksize, anchor);
		}
		if (typeid(float).name() == typeid(_Tp).name()) {
			return makePtr<MorphColumnFilter<MaxOp<float>, MorphColumnNoVec> >(ksize, anchor);
		}
	}

	FBC_Error("Unsupported data type");
	return Ptr<BaseColumnFilter>();
}

// returns 2D morphological filter
template<typename _Tp, int chs>
Ptr<BaseFilter> getMorphologyFilter(int op, Mat_<uchar, 1> kernel, Point anchor = Point(-1, -1))
{
	anchor = normalizeAnchor(anchor, kernel.size());
	FBC_Assert(op == MORPH_ERODE || op == MORPH_DILATE);

	if (op == MORPH_ERODE) {
		if (typeid(uchar).name() == typeid(_Tp).name()) {
			return makePtr<MorphFilter<MinOp<uchar>, MorphNoVec> >(kernel, anchor);
		}
		if (typeid(float).name() == typeid(_Tp).name()) {
			return makePtr<MorphFilter<MinOp<float>, MorphNoVec> >(kernel, anchor);
		}
	} else {
		if (typeid(uchar).name() == typeid(_Tp).name()) {
			return makePtr<MorphFilter<MaxOp<uchar>, MorphNoVec> >(kernel, anchor);
		}
		if (typeid(float).name() == typeid(_Tp).name()) {
			return makePtr<MorphFilter<MaxOp<float>, MorphNoVec> >(kernel, anchor);
		}
	}

	FBC_Error("Unsupported data type");
	return Ptr<BaseFilter>();
}

} // namespace fbc

#endif // FBC_CV_MORPH_HPP_
