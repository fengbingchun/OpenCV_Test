// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_HAL_HPP_
#define FBC_CV_CORE_HAL_HPP_

// reference: include/opencv2/core/hal/hal.hpp

#include <stddef.h>
#include "fbcdef.hpp"
#include "interface.hpp"

namespace fbc { namespace hal {

FBC_EXPORTS int normHamming(const uchar* a, int n);
FBC_EXPORTS int normHamming(const uchar* a, const uchar* b, int n);

FBC_EXPORTS int normHamming(const uchar* a, int n, int cellSize);
FBC_EXPORTS int normHamming(const uchar* a, const uchar* b, int n, int cellSize);

FBC_EXPORTS int LU32f(float* A, size_t astep, int m, float* b, size_t bstep, int n);
FBC_EXPORTS int LU64f(double* A, size_t astep, int m, double* b, size_t bstep, int n);
FBC_EXPORTS bool Cholesky32f(float* A, size_t astep, int m, float* b, size_t bstep, int n);
FBC_EXPORTS bool Cholesky64f(double* A, size_t astep, int m, double* b, size_t bstep, int n);

FBC_EXPORTS int normL1_(const uchar* a, const uchar* b, int n);
FBC_EXPORTS float normL1_(const float* a, const float* b, int n);
FBC_EXPORTS float normL2Sqr_(const float* a, const float* b, int n);

FBC_EXPORTS void exp32f(const float* src, float* dst, int n);
FBC_EXPORTS void exp64f(const double* src, double* dst, int n);
FBC_EXPORTS void log32f(const float* src, float* dst, int n);
FBC_EXPORTS void log64f(const double* src, double* dst, int n);

FBC_EXPORTS void fastAtan2(const float* y, const float* x, float* dst, int n, bool angleInDegrees);
FBC_EXPORTS void magnitude32f(const float* x, const float* y, float* dst, int n);
FBC_EXPORTS void magnitude64f(const double* x, const double* y, double* dst, int n);
FBC_EXPORTS void sqrt32f(const float* src, float* dst, int len);
FBC_EXPORTS void sqrt64f(const double* src, double* dst, int len);
FBC_EXPORTS void invSqrt32f(const float* src, float* dst, int len);
FBC_EXPORTS void invSqrt64f(const double* src, double* dst, int len);

FBC_EXPORTS void split8u(const uchar* src, uchar** dst, int len, int cn);
FBC_EXPORTS void split16u(const ushort* src, ushort** dst, int len, int cn);
FBC_EXPORTS void split32s(const int* src, int** dst, int len, int cn);
FBC_EXPORTS void split64s(const int64* src, int64** dst, int len, int cn);

FBC_EXPORTS void merge8u(const uchar** src, uchar* dst, int len, int cn);
FBC_EXPORTS void merge16u(const ushort** src, ushort* dst, int len, int cn);
FBC_EXPORTS void merge32s(const int** src, int* dst, int len, int cn);
FBC_EXPORTS void merge64s(const int64** src, int64* dst, int len, int cn);

FBC_EXPORTS void add8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void add8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void add16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void add16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void add32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void add32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void add64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void*);

FBC_EXPORTS void sub8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void sub8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void sub16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void sub16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void sub32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void sub32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void sub64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void*);

FBC_EXPORTS void max8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void max8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void max16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void max16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void max32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void max32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void max64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void*);

FBC_EXPORTS void min8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void min8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void min16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void min16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void min32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void min32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void min64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void*);

FBC_EXPORTS void absdiff8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void absdiff8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void absdiff16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void absdiff16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void absdiff32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void absdiff32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void absdiff64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void*);

FBC_EXPORTS void and8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void or8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void xor8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);
FBC_EXPORTS void not8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void*);

FBC_EXPORTS void cmp8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);
FBC_EXPORTS void cmp8s(const schar* src1, size_t step1, const schar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);
FBC_EXPORTS void cmp16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);
FBC_EXPORTS void cmp16s(const short* src1, size_t step1, const short* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);
FBC_EXPORTS void cmp32s(const int* src1, size_t step1, const int* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);
FBC_EXPORTS void cmp32f(const float* src1, size_t step1, const float* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);
FBC_EXPORTS void cmp64f(const double* src1, size_t step1, const double* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _cmpop);

FBC_EXPORTS void mul8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void mul8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void mul16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void mul16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void mul32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void mul32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void mul64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void* scale);

FBC_EXPORTS void div8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void div8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void div16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void div16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void div32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void div32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void div64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void* scale);

FBC_EXPORTS void recip8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void recip8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void recip16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void recip16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void recip32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void recip32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void* scale);
FBC_EXPORTS void recip64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void* scale);

FBC_EXPORTS void addWeighted8u(const uchar* src1, size_t step1, const uchar* src2, size_t step2, uchar* dst, size_t step, int width, int height, void* _scalars);
FBC_EXPORTS void addWeighted8s(const schar* src1, size_t step1, const schar* src2, size_t step2, schar* dst, size_t step, int width, int height, void* scalars);
FBC_EXPORTS void addWeighted16u(const ushort* src1, size_t step1, const ushort* src2, size_t step2, ushort* dst, size_t step, int width, int height, void* scalars);
FBC_EXPORTS void addWeighted16s(const short* src1, size_t step1, const short* src2, size_t step2, short* dst, size_t step, int width, int height, void* scalars);
FBC_EXPORTS void addWeighted32s(const int* src1, size_t step1, const int* src2, size_t step2, int* dst, size_t step, int width, int height, void* scalars);
FBC_EXPORTS void addWeighted32f(const float* src1, size_t step1, const float* src2, size_t step2, float* dst, size_t step, int width, int height, void* scalars);
FBC_EXPORTS void addWeighted64f(const double* src1, size_t step1, const double* src2, size_t step2, double* dst, size_t step, int width, int height, void* scalars);

FBC_EXPORTS int LU(float* A, size_t astep, int m, float* b, size_t bstep, int n);
FBC_EXPORTS int LU(double* A, size_t astep, int m, double* b, size_t bstep, int n);
FBC_EXPORTS bool Cholesky(float* A, size_t astep, int m, float* b, size_t bstep, int n);
FBC_EXPORTS bool Cholesky(double* A, size_t astep, int m, double* b, size_t bstep, int n);

FBC_EXPORTS void exp(const float* src, float* dst, int n);
FBC_EXPORTS void exp(const double* src, double* dst, int n);
FBC_EXPORTS void log(const float* src, float* dst, int n);
FBC_EXPORTS void log(const double* src, double* dst, int n);

FBC_EXPORTS void magnitude(const float* x, const float* y, float* dst, int n);
FBC_EXPORTS void magnitude(const double* x, const double* y, double* dst, int n);
FBC_EXPORTS void sqrt(const float* src, float* dst, int len);
FBC_EXPORTS void sqrt(const double* src, double* dst, int len);
FBC_EXPORTS void invSqrt(const float* src, float* dst, int len);
FBC_EXPORTS void invSqrt(const double* src, double* dst, int len);

} // namespace hal
} // namespace fbc

#endif // FBC_CV_CORE_HAL_HPP_
