// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_SOLVE_HPP_
#define FBC_CV_SOLVE_HPP_

/* reference: core/include/opencv2/core.hpp
              modules/core/src/lapack.cpp
*/

#ifndef __cplusplus
	#error solve.hpp header must be compiled as C++
#endif

#include "base.hpp"
#include "mat.hpp"
#include "hal.hpp"
#include "rng.hpp"

namespace fbc {

template<typename _Tp>
bool JacobiImpl_(_Tp* A, size_t astep, _Tp* W, _Tp* V, size_t vstep, int n, uchar* buf)
{
	const _Tp eps = std::numeric_limits<_Tp>::epsilon();
	int i, j, k, m;

	astep /= sizeof(A[0]);
	if (V) {
		vstep /= sizeof(V[0]);
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++)
				V[i*vstep + j] = (_Tp)0;
			V[i*vstep + i] = (_Tp)1;
		}
	}

	int iters, maxIters = n*n * 30;

	int* indR = (int*)alignPtr(buf, sizeof(int));
	int* indC = indR + n;
	_Tp mv = (_Tp)0;

	for (k = 0; k < n; k++) {
		W[k] = A[(astep + 1)*k];
		if (k < n - 1) {
			for (m = k + 1, mv = std::abs(A[astep*k + m]), i = k + 2; i < n; i++) {
				_Tp val = std::abs(A[astep*k + i]);
				if (mv < val)
					mv = val, m = i;
			}
			indR[k] = m;
		}
		if (k > 0) {
			for (m = 0, mv = std::abs(A[k]), i = 1; i < k; i++) {
				_Tp val = std::abs(A[astep*i + k]);
				if (mv < val)
					mv = val, m = i;
			}
			indC[k] = m;
		}
	}

	if (n > 1) for (iters = 0; iters < maxIters; iters++) {
		// find index (k,l) of pivot p
		for (k = 0, mv = std::abs(A[indR[0]]), i = 1; i < n - 1; i++) {
			_Tp val = std::abs(A[astep*i + indR[i]]);
			if (mv < val)
				mv = val, k = i;
		}
		int l = indR[k];
		for (i = 1; i < n; i++) {
			_Tp val = std::abs(A[astep*indC[i] + i]);
			if (mv < val)
				mv = val, k = indC[i], l = i;
		}

		_Tp p = A[astep*k + l];
		if (std::abs(p) <= eps)
			break;
		_Tp y = (_Tp)((W[l] - W[k])*0.5);
		_Tp t = std::abs(y) + hypot(p, y);
		_Tp s = hypot(p, t);
		_Tp c = t / s;
		s = p / s; t = (p / t)*p;
		if (y < 0)
			s = -s, t = -t;
		A[astep*k + l] = 0;

		W[k] -= t;
		W[l] += t;

		_Tp a0, b0;

#undef rotate
#define rotate(v0, v1) a0 = v0, b0 = v1, v0 = a0*c - b0*s, v1 = a0*s + b0*c

		// rotate rows and columns k and l
		for (i = 0; i < k; i++)
			rotate(A[astep*i + k], A[astep*i + l]);
		for (i = k + 1; i < l; i++)
			rotate(A[astep*k + i], A[astep*i + l]);
		for (i = l + 1; i < n; i++)
			rotate(A[astep*k + i], A[astep*l + i]);

		// rotate eigenvectors
		if (V) {
			for (i = 0; i < n; i++)
				rotate(V[vstep*k + i], V[vstep*l + i]);
		}

#undef rotate

		for (j = 0; j < 2; j++) {
			int idx = j == 0 ? k : l;
			if (idx < n - 1) {
				for (m = idx + 1, mv = std::abs(A[astep*idx + m]), i = idx + 2; i < n; i++) {
					_Tp val = std::abs(A[astep*idx + i]);
					if (mv < val)
						mv = val, m = i;
				}
				indR[idx] = m;
			}
			if (idx > 0) {
				for (m = 0, mv = std::abs(A[idx]), i = 1; i < idx; i++) {
					_Tp val = std::abs(A[astep*i + idx]);
					if (mv < val)
						mv = val, m = i;
				}
				indC[idx] = m;
			}
		}
	}

	// sort eigenvalues & eigenvectors
	for (k = 0; k < n - 1; k++) {
		m = k;
		for (i = k + 1; i < n; i++) {
			if (W[m] < W[i])
				m = i;
		}
		if (k != m) {
			std::swap(W[m], W[k]);
			if (V) {
				for (i = 0; i < n; i++)
					std::swap(V[vstep*m + i], V[vstep*k + i]);
			}
		}
	}

	return true;
}

static bool Jacobi(float* S, size_t sstep, float* e, float* E, size_t estep, int n, uchar* buf)
{
	return JacobiImpl_(S, sstep, e, E, estep, n, buf);
}

static bool Jacobi(double* S, size_t sstep, double* e, double* E, size_t estep, int n, uchar* buf)
{
	return JacobiImpl_(S, sstep, e, E, estep, n, buf);
}

template<typename T> struct VBLAS
{
	int dot(const T*, const T*, int, T*) const { return 0; }
	int givens(T*, T*, int, T, T) const { return 0; }
	int givensx(T*, T*, int, T, T, T*, T*) const { return 0; }
};

template<typename _Tp>
static inline _Tp hypot(_Tp a, _Tp b)
{
	a = std::abs(a);
	b = std::abs(b);
	if (a > b) {
		b /= a;
		return a*std::sqrt(1 + b*b);
	}
	if (b > 0) {
		a /= b;
		return b*std::sqrt(1 + a*a);
	}
	return 0;
}

template<typename _Tp>
void JacobiSVDImpl_(_Tp* At, size_t astep, _Tp* _W, _Tp* Vt, size_t vstep, int m, int n, int n1, double minval, _Tp eps)
{
	VBLAS<_Tp> vblas;
	AutoBuffer<double> Wbuf(n);
	double* W = Wbuf;
	int i, j, k, iter, max_iter = std::max(m, 30);
	_Tp c, s;
	double sd;
	astep /= sizeof(At[0]);
	vstep /= sizeof(Vt[0]);

	for (i = 0; i < n; i++) {
		for (k = 0, sd = 0; k < m; k++) {
			_Tp t = At[i*astep + k];
			sd += (double)t*t;
		}
		W[i] = sd;

		if (Vt) {
			for (k = 0; k < n; k++)
				Vt[i*vstep + k] = 0;
			Vt[i*vstep + i] = 1;
		}
	}

	for (iter = 0; iter < max_iter; iter++) {
		bool changed = false;

		for (i = 0; i < n - 1; i++) {
			for (j = i + 1; j < n; j++) {
				_Tp *Ai = At + i*astep, *Aj = At + j*astep;
				double a = W[i], p = 0, b = W[j];

				for (k = 0; k < m; k++)
					p += (double)Ai[k] * Aj[k];

				if (std::abs(p) <= eps*std::sqrt((double)a*b))
					continue;

				p *= 2;
				double beta = a - b, gamma = hypot((double)p, beta);
				if (beta < 0) {
					double delta = (gamma - beta)*0.5;
					s = (_Tp)std::sqrt(delta / gamma);
					c = (_Tp)(p / (gamma*s * 2));
				} else {
					c = (_Tp)std::sqrt((gamma + beta) / (gamma * 2));
					s = (_Tp)(p / (gamma*c * 2));
				}

				a = b = 0;
				for (k = 0; k < m; k++) {
					_Tp t0 = c*Ai[k] + s*Aj[k];
					_Tp t1 = -s*Ai[k] + c*Aj[k];
					Ai[k] = t0; Aj[k] = t1;

					a += (double)t0*t0; b += (double)t1*t1;
				}
				W[i] = a; W[j] = b;

				changed = true;

				if (Vt) {
					_Tp *Vi = Vt + i*vstep, *Vj = Vt + j*vstep;
					k = vblas.givens(Vi, Vj, n, c, s);

					for (; k < n; k++) {
						_Tp t0 = c*Vi[k] + s*Vj[k];
						_Tp t1 = -s*Vi[k] + c*Vj[k];
						Vi[k] = t0; Vj[k] = t1;
					}
				}
			}
		}
		if (!changed)
			break;
	}

	for (i = 0; i < n; i++) {
		for (k = 0, sd = 0; k < m; k++) {
			_Tp t = At[i*astep + k];
			sd += (double)t*t;
		}
		W[i] = std::sqrt(sd);
	}

	for (i = 0; i < n - 1; i++) {
		j = i;
		for (k = i + 1; k < n; k++) {
			if (W[j] < W[k])
				j = k;
		}
		if (i != j) {
			std::swap(W[i], W[j]);
			if (Vt) {
				for (k = 0; k < m; k++)
					std::swap(At[i*astep + k], At[j*astep + k]);

				for (k = 0; k < n; k++)
					std::swap(Vt[i*vstep + k], Vt[j*vstep + k]);
			}
		}
	}

	for (i = 0; i < n; i++)
		_W[i] = (_Tp)W[i];

	if (!Vt)
		return;

	RNG<uchar, uchar, uchar, 1, 1, 1> rng(0x12345678);
	for (i = 0; i < n1; i++) {
		sd = i < n ? W[i] : 0;

		for (int ii = 0; ii < 100 && sd <= minval; ii++) {
			// if we got a zero singular value, then in order to get the corresponding left singular vector
			// we generate a random vector, project it to the previously computed left singular vectors,
			// subtract the projection and normalize the difference.
			const _Tp val0 = (_Tp)(1. / m);
			for (k = 0; k < m; k++) {
				_Tp val = (rng.next() & 256) != 0 ? val0 : -val0;
				At[i*astep + k] = val;
			}
			for (iter = 0; iter < 2; iter++) {
				for (j = 0; j < i; j++) {
					sd = 0;
					for (k = 0; k < m; k++)
						sd += At[i*astep + k] * At[j*astep + k];
					_Tp asum = 0;
					for (k = 0; k < m; k++) {
						_Tp t = (_Tp)(At[i*astep + k] - sd*At[j*astep + k]);
						At[i*astep + k] = t;
						asum += std::abs(t);
					}
					asum = asum > eps * 100 ? 1 / asum : 0;
					for (k = 0; k < m; k++)
						At[i*astep + k] *= asum;
				}
			}
			sd = 0;
			for (k = 0; k < m; k++) {
				_Tp t = At[i*astep + k];
				sd += (double)t*t;
			}
			sd = std::sqrt(sd);
		}

		s = (_Tp)(sd > minval ? 1 / sd : 0.);
		for (k = 0; k < m; k++)
			At[i*astep + k] *= s;
	}
}

static void JacobiSVD(float* At, size_t astep, float* W, float* Vt, size_t vstep, int m, int n, int n1 = -1)
{
	JacobiSVDImpl_(At, astep, W, Vt, vstep, m, n, !Vt ? 0 : n1 < 0 ? n : n1, FLT_MIN, FLT_EPSILON * 2);
}

static void JacobiSVD(double* At, size_t astep, double* W, double* Vt, size_t vstep, int m, int n, int n1 = -1)
{
	JacobiSVDImpl_(At, astep, W, Vt, vstep, m, n, !Vt ? 0 : n1 < 0 ? n : n1, DBL_MIN, DBL_EPSILON * 10);
}

/* y[0:m,0:n] += diag(a[0:1,0:m]) * x[0:m,0:n] */
template<typename T1, typename T2, typename T3>
static void MatrAXPY(int m, int n, const T1* x, int dx,
const T2* a, int inca, T3* y, int dy)
{
	int i;
	for (i = 0; i < m; i++, x += dx, y += dy) {
		T2 s = a[i*inca];
		int j = 0;
		for (; j <= n - 4; j += 4) {
			T3 t0 = (T3)(y[j] + s*x[j]);
			T3 t1 = (T3)(y[j + 1] + s*x[j + 1]);
			y[j] = t0;
			y[j + 1] = t1;
			t0 = (T3)(y[j + 2] + s*x[j + 2]);
			t1 = (T3)(y[j + 3] + s*x[j + 3]);
			y[j + 2] = t0;
			y[j + 3] = t1;
		}

		for (; j < n; j++)
			y[j] = (T3)(y[j] + s*x[j]);
	}
}

template<typename T>
static void SVBkSbImpl_(int m, int n, const T* w, int incw, const T* u, int ldu, bool uT,
			const T* v, int ldv, bool vT, const T* b, int ldb, int nb, T* x, int ldx, double* buffer, T eps)
{
	double threshold = 0;
	int udelta0 = uT ? ldu : 1, udelta1 = uT ? 1 : ldu;
	int vdelta0 = vT ? ldv : 1, vdelta1 = vT ? 1 : ldv;
	int i, j, nm = std::min(m, n);

	if (!b)
		nb = m;

	for (i = 0; i < n; i++)
		for (j = 0; j < nb; j++)
			x[i*ldx + j] = 0;

	for (i = 0; i < nm; i++)
		threshold += w[i*incw];
	threshold *= eps;

	// v * inv(w) * uT * b
	for (i = 0; i < nm; i++, u += udelta0, v += vdelta0) {
		double wi = w[i*incw];
		if ((double)std::abs(wi) <= threshold)
			continue;
		wi = 1 / wi;

		if (nb == 1) {
			double s = 0;
			if (b)
				for (j = 0; j < m; j++)
					s += u[j*udelta1] * b[j*ldb];
			else
				s = u[0];
			s *= wi;

			for (j = 0; j < n; j++)
				x[j*ldx] = (T)(x[j*ldx] + s*v[j*vdelta1]);
		} else {
			if (b) {
				for (j = 0; j < nb; j++)
					buffer[j] = 0;
				MatrAXPY(m, nb, b, ldb, u, udelta1, buffer, 0);
				for (j = 0; j < nb; j++)
					buffer[j] *= wi;
			} else {
				for (j = 0; j < nb; j++)
					buffer[j] = u[j*udelta1] * wi;
			}
			MatrAXPY(n, nb, buffer, 0, v, vdelta1, x, ldx);
		}
	}
}

static void SVBkSb(int m, int n, const float* w, size_t wstep, const float* u, size_t ustep, bool uT,
			const float* v, size_t vstep, bool vT, const float* b, size_t bstep, int nb, float* x, size_t xstep, uchar* buffer)
{
	SVBkSbImpl_(m, n, w, wstep ? (int)(wstep / sizeof(w[0])) : 1,
		u, (int)(ustep / sizeof(u[0])), uT,
		v, (int)(vstep / sizeof(v[0])), vT,
		b, (int)(bstep / sizeof(b[0])), nb,
		x, (int)(xstep / sizeof(x[0])),
		(double*)alignPtr(buffer, sizeof(double)), (float)(DBL_EPSILON * 2));
}

static void SVBkSb(int m, int n, const double* w, size_t wstep, const double* u, size_t ustep, bool uT,
			const double* v, size_t vstep, bool vT, const double* b, size_t bstep, int nb, double* x, size_t xstep, uchar* buffer)
{
	SVBkSbImpl_(m, n, w, wstep ? (int)(wstep / sizeof(w[0])) : 1,
		u, (int)(ustep / sizeof(u[0])), uT,
		v, (int)(vstep / sizeof(v[0])), vT,
		b, (int)(bstep / sizeof(b[0])), nb,
		x, (int)(xstep / sizeof(x[0])),
		(double*)alignPtr(buffer, sizeof(double)), DBL_EPSILON * 2);
}

#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

#define det2(m)   ((double)m(0,0)*m(1,1) - (double)m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*((double)m(1,1)*m(2,2) - (double)m(1,2)*m(2,1)) -  \
                   m(0,1)*((double)m(1,0)*m(2,2) - (double)m(1,2)*m(2,0)) +  \
                   m(0,2)*((double)m(1,0)*m(2,1) - (double)m(1,1)*m(2,0)))

// Solves one or more linear systems or least-squares problems
// The function solve solves a linear system or least-squares problem (the
// latter is possible with SVD or QR methods, or by specifying the flag DECOMP_NORMAL)
template<typename _Tp, int chs>
bool solve(const Mat_<_Tp, chs>& src1, const Mat_<_Tp, chs>& src2, Mat_<_Tp, chs>& dst, int method = DECOMP_LU)
{
	FBC_Assert(src1.data != NULL && src2.data != NULL && dst.data != NULL);
	FBC_Assert(chs == 1);

	bool result = true;
	bool is_normal = (method & DECOMP_NORMAL) != 0;

	FBC_Assert(sizeof(_Tp) == sizeof(float) || sizeof(_Tp) == sizeof(double)); // float(4) || double(8)
	method &= ~DECOMP_NORMAL;
	FBC_Assert((method != DECOMP_LU && method != DECOMP_CHOLESKY) || is_normal || src1.rows == src1.cols);

	// check case of a single equation and small matrix
	if ((method == DECOMP_LU || method == DECOMP_CHOLESKY) && !is_normal &&
		src1.rows <= 3 && src1.rows == src1.cols && src2.cols == 1) {
		FBC_Assert(dst.rows == src1.cols && dst.cols == src2.cols);

		#define bf(y) ((float*)(bdata + y*src2step))[0]
		#define bd(y) ((double*)(bdata + y*src2step))[0]

		const uchar* srcdata = src1.ptr();
		const uchar* bdata = src2.ptr();
		uchar* dstdata = (uchar*)dst.ptr();
		size_t srcstep = src1.step;
		size_t src2step = src2.step;
		size_t dststep = dst.step;

		if (src1.rows == 2) {
			if (sizeof(_Tp) == sizeof(float)) {
				double d = det2(Sf);
				if (d != 0.) {
					double t;
					d = 1. / d;
					t = (float)(((double)bf(0)*Sf(1, 1) - (double)bf(1)*Sf(0, 1))*d);
					Df(1, 0) = (float)(((double)bf(1)*Sf(0, 0) - (double)bf(0)*Sf(1, 0))*d);
					Df(0, 0) = (float)t;
				} else {
					result = false;
				}
			} else {
				double d = det2(Sd);
				if (d != 0.) {
					double t;
					d = 1. / d;
					t = (bd(0)*Sd(1, 1) - bd(1)*Sd(0, 1))*d;
					Dd(1, 0) = (bd(1)*Sd(0, 0) - bd(0)*Sd(1, 0))*d;
					Dd(0, 0) = t;
				} else {
					result = false;
				}
			}
		} else if (src1.rows == 3) {
			if (sizeof(_Tp) == sizeof(float)) {
				double d = det3(Sf);
				if (d != 0.) {
					float t[3];
					d = 1. / d;

					t[0] = (float)(d*
						(bf(0)*((double)Sf(1, 1)*Sf(2, 2) - (double)Sf(1, 2)*Sf(2, 1)) -
						Sf(0, 1)*((double)bf(1)*Sf(2, 2) - (double)Sf(1, 2)*bf(2)) +
						Sf(0, 2)*((double)bf(1)*Sf(2, 1) - (double)Sf(1, 1)*bf(2))));

					t[1] = (float)(d*
						(Sf(0, 0)*(double)(bf(1)*Sf(2, 2) - (double)Sf(1, 2)*bf(2)) -
						bf(0)*((double)Sf(1, 0)*Sf(2, 2) - (double)Sf(1, 2)*Sf(2, 0)) +
						Sf(0, 2)*((double)Sf(1, 0)*bf(2) - (double)bf(1)*Sf(2, 0))));

					t[2] = (float)(d*
						(Sf(0, 0)*((double)Sf(1, 1)*bf(2) - (double)bf(1)*Sf(2, 1)) -
						Sf(0, 1)*((double)Sf(1, 0)*bf(2) - (double)bf(1)*Sf(2, 0)) +
						bf(0)*((double)Sf(1, 0)*Sf(2, 1) - (double)Sf(1, 1)*Sf(2, 0))));

					Df(0, 0) = t[0];
					Df(1, 0) = t[1];
					Df(2, 0) = t[2];
				} else {
					result = false;
				}
			} else {
				double d = det3(Sd);
				if (d != 0.) {
					double t[9];

					d = 1. / d;

					t[0] = ((Sd(1, 1) * Sd(2, 2) - Sd(1, 2) * Sd(2, 1))*bd(0) +
						(Sd(0, 2) * Sd(2, 1) - Sd(0, 1) * Sd(2, 2))*bd(1) +
						(Sd(0, 1) * Sd(1, 2) - Sd(0, 2) * Sd(1, 1))*bd(2))*d;

					t[1] = ((Sd(1, 2) * Sd(2, 0) - Sd(1, 0) * Sd(2, 2))*bd(0) +
						(Sd(0, 0) * Sd(2, 2) - Sd(0, 2) * Sd(2, 0))*bd(1) +
						(Sd(0, 2) * Sd(1, 0) - Sd(0, 0) * Sd(1, 2))*bd(2))*d;

					t[2] = ((Sd(1, 0) * Sd(2, 1) - Sd(1, 1) * Sd(2, 0))*bd(0) +
						(Sd(0, 1) * Sd(2, 0) - Sd(0, 0) * Sd(2, 1))*bd(1) +
						(Sd(0, 0) * Sd(1, 1) - Sd(0, 1) * Sd(1, 0))*bd(2))*d;

					Dd(0, 0) = t[0];
					Dd(1, 0) = t[1];
					Dd(2, 0) = t[2];
				} else {
					result = false;
				}
			}
		} else {
			FBC_Assert(src1.rows == 1);

			if (sizeof(_Tp) == sizeof(float)) {
				double d = Sf(0, 0);
				if (d != 0.)
					Df(0, 0) = (float)(bf(0) / d);
				else
					result = false;
			} else {
				double d = Sd(0, 0);
				if (d != 0.)
					Dd(0, 0) = (bd(0) / d);
				else
					result = false;
			}
		}
		return result;
	}

	if (method == DECOMP_QR)
		method = DECOMP_SVD;

	int m = src1.rows, m_ = m, n = src1.cols, nb = src2.cols;
	size_t esz = sizeof(_Tp) * chs, bufsize = 0;
	size_t vstep = alignSize(n*esz, 16);
	size_t astep = method == DECOMP_SVD && !is_normal ? alignSize(m*esz, 16) : vstep;
	AutoBuffer<uchar> buffer;

	FBC_Assert(dst.rows == src1.cols && dst.cols == src2.cols);

	if (m < n) {
		FBC_Error("The function can not solve under-determined linear systems");
	}

	if (m == n) {
		is_normal = false;
	} else if (is_normal) {
		m_ = n;
		if (method == DECOMP_SVD)
			method = DECOMP_EIG;
	}

	size_t asize = astep*(method == DECOMP_SVD || is_normal ? n : m);
	bufsize += asize + 32;

	if (is_normal)
		bufsize += n*nb*esz;

	if (method == DECOMP_SVD || method == DECOMP_EIG)
		bufsize += n * 5 * esz + n*vstep + nb*sizeof(double) + 32;

	buffer.allocate(bufsize);
	uchar* ptr = alignPtr((uchar*)buffer, 16);

	Mat_<_Tp, chs> a(m_, n, ptr);

	if (is_normal) {
		//mulTransposed(src, a, true); // TODO
		FBC_Assert(0);
	} else if (method != DECOMP_SVD) {
		src1.copyTo(a);
	} else {
		a = Mat_<_Tp, chs>(n, m_, ptr);
		transpose(src1, a);
	}
	ptr += asize;

	if (!is_normal) {
		if (method == DECOMP_LU || method == DECOMP_CHOLESKY)
			src2.copyTo(dst);
	} else {
		FBC_Assert(0);
		// a'*b
		if (method == DECOMP_LU || method == DECOMP_CHOLESKY) {
			//gemm(src, src2, 1, Mat(), 0, dst, GEMM_1_T); // TODO
		} else {
			//Mat tmp(n, nb, type, ptr);
			//ptr += n*nb*esz;
			//gemm(src, src2, 1, Mat(), 0, tmp, GEMM_1_T); // TODO
			//src2 = tmp;
		}
	}

	if (method == DECOMP_LU) {
		if (sizeof(_Tp) == sizeof(float))
			result = hal::LU32f((float*)a.ptr(), a.step, n, (float*)dst.ptr(), dst.step, nb) != 0;
		else
			result = hal::LU64f((double*)a.ptr(), a.step, n, (double*)dst.ptr(), dst.step, nb) != 0;
	} else if (method == DECOMP_CHOLESKY) {
		if (sizeof(_Tp) == sizeof(float))
			result = hal::Cholesky32f((float*)a.ptr(), a.step, n, (float*)dst.ptr(), dst.step, nb);
		else
			result = hal::Cholesky64f((double*)a.ptr(), a.step, n, (double*)dst.ptr(), dst.step, nb);
	} else {
		ptr = alignPtr(ptr, 16);
		FBC_Assert(n * sizeof(_Tp) == vstep);
		Mat_<_Tp, 1> v(n, n, ptr), w(n, 1, ptr + vstep*n), u;
		ptr += n*(vstep + esz);

		if (method == DECOMP_EIG) {
			if (sizeof(_Tp) == sizeof(float))
				Jacobi((float*)a.ptr(), a.step, (float*)w.ptr(), (float*)v.ptr(), v.step, n, ptr);
			else
				Jacobi((double*)a.ptr(), a.step, (double*)w.ptr(), (double*)v.ptr(), v.step, n, ptr);
			u = v;
		} else {
			if (sizeof(_Tp) == sizeof(float))
				JacobiSVD((float*)a.ptr(), a.step, (float*)w.ptr(), (float*)v.ptr(), v.step, m_, n);
			else
				JacobiSVD((double*)a.ptr(), a.step, (double*)w.ptr(), (double*)v.ptr(), v.step, m_, n);
			u = a;
		}

		if (sizeof(_Tp) == sizeof(float)) {
			SVBkSb(m_, n, (float*)w.ptr(), 0, (float*)u.ptr(), u.step, true,
				(float*)v.ptr(), v.step, true, (float*)src2.ptr(),
				src2.step, nb, (float*)dst.ptr(), dst.step, ptr);
		} else {
			SVBkSb(m_, n, (double*)w.ptr(), 0, (double*)u.ptr(), u.step, true,
				(double*)v.ptr(), v.step, true, (double*)src2.ptr(),
				src2.step, nb, (double*)dst.ptr(), dst.step, ptr);
		}
		result = true;
	}

	return result;
}

} // namespace fbc

#endif // FBC_CV_SOLVE_HPP_
