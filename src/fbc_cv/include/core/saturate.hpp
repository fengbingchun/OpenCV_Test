//#ifndef FBC_OPENCV_TEST_SATURATE_HPP_
//#define FBC_OPENCV_TEST_SATURATE_HPP_
//
//// reference: opencv/include/opencv2/core/matx.hpp
//
//namespace fbc
//{
//template<typename _Tp> static inline _Tp saturate_cast(uchar v)    { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(schar v)    { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(ushort v)   { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(short v)    { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(unsigned v) { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(int v)      { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(float v)    { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(double v)   { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(int64 v)    { return _Tp(v); }
//template<typename _Tp> static inline _Tp saturate_cast(uint64 v)   { return _Tp(v); }
//
//template<> inline uchar saturate_cast<uchar>(schar v)        { return (uchar)std::max((int)v, 0); }
//template<> inline uchar saturate_cast<uchar>(ushort v)       { return (uchar)std::min((unsigned)v, (unsigned)UCHAR_MAX); }
//template<> inline uchar saturate_cast<uchar>(int v)          { return (uchar)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
//template<> inline uchar saturate_cast<uchar>(short v)        { return saturate_cast<uchar>((int)v); }
//template<> inline uchar saturate_cast<uchar>(unsigned v)     { return (uchar)std::min(v, (unsigned)UCHAR_MAX); }
//template<> inline uchar saturate_cast<uchar>(float v)        { int iv = cvRound(v); return saturate_cast<uchar>(iv); }
//template<> inline uchar saturate_cast<uchar>(double v)       { int iv = cvRound(v); return saturate_cast<uchar>(iv); }
//template<> inline uchar saturate_cast<uchar>(int64 v)        { return (uchar)((uint64)v <= (uint64)UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
//template<> inline uchar saturate_cast<uchar>(uint64 v)       { return (uchar)std::min(v, (uint64)UCHAR_MAX); }
//
//template<> inline schar saturate_cast<schar>(uchar v)        { return (schar)std::min((int)v, SCHAR_MAX); }
//template<> inline schar saturate_cast<schar>(ushort v)       { return (schar)std::min((unsigned)v, (unsigned)SCHAR_MAX); }
//template<> inline schar saturate_cast<schar>(int v)          { return (schar)((unsigned)(v-SCHAR_MIN) <= (unsigned)UCHAR_MAX ? v : v > 0 ? SCHAR_MAX : SCHAR_MIN); }
//template<> inline schar saturate_cast<schar>(short v)        { return saturate_cast<schar>((int)v); }
//template<> inline schar saturate_cast<schar>(unsigned v)     { return (schar)std::min(v, (unsigned)SCHAR_MAX); }
//template<> inline schar saturate_cast<schar>(float v)        { int iv = cvRound(v); return saturate_cast<schar>(iv); }
//template<> inline schar saturate_cast<schar>(double v)       { int iv = cvRound(v); return saturate_cast<schar>(iv); }
//template<> inline schar saturate_cast<schar>(int64 v)        { return (schar)((uint64)((int64)v-SCHAR_MIN) <= (uint64)UCHAR_MAX ? v : v > 0 ? SCHAR_MAX : SCHAR_MIN); }
//template<> inline schar saturate_cast<schar>(uint64 v)       { return (schar)std::min(v, (uint64)SCHAR_MAX); }
//
//template<> inline ushort saturate_cast<ushort>(schar v)      { return (ushort)std::max((int)v, 0); }
//template<> inline ushort saturate_cast<ushort>(short v)      { return (ushort)std::max((int)v, 0); }
//template<> inline ushort saturate_cast<ushort>(int v)        { return (ushort)((unsigned)v <= (unsigned)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0); }
//template<> inline ushort saturate_cast<ushort>(unsigned v)   { return (ushort)std::min(v, (unsigned)USHRT_MAX); }
//template<> inline ushort saturate_cast<ushort>(float v)      { int iv = cvRound(v); return saturate_cast<ushort>(iv); }
//template<> inline ushort saturate_cast<ushort>(double v)     { int iv = cvRound(v); return saturate_cast<ushort>(iv); }
//template<> inline ushort saturate_cast<ushort>(int64 v)      { return (ushort)((uint64)v <= (uint64)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0); }
//template<> inline ushort saturate_cast<ushort>(uint64 v)     { return (ushort)std::min(v, (uint64)USHRT_MAX); }
//
//template<> inline short saturate_cast<short>(ushort v)       { return (short)std::min((int)v, SHRT_MAX); }
//template<> inline short saturate_cast<short>(int v)          { return (short)((unsigned)(v - SHRT_MIN) <= (unsigned)USHRT_MAX ? v : v > 0 ? SHRT_MAX : SHRT_MIN); }
//template<> inline short saturate_cast<short>(unsigned v)     { return (short)std::min(v, (unsigned)SHRT_MAX); }
//template<> inline short saturate_cast<short>(float v)        { int iv = cvRound(v); return saturate_cast<short>(iv); }
//template<> inline short saturate_cast<short>(double v)       { int iv = cvRound(v); return saturate_cast<short>(iv); }
//template<> inline short saturate_cast<short>(int64 v)        { return (short)((uint64)((int64)v - SHRT_MIN) <= (uint64)USHRT_MAX ? v : v > 0 ? SHRT_MAX : SHRT_MIN); }
//template<> inline short saturate_cast<short>(uint64 v)       { return (short)std::min(v, (uint64)SHRT_MAX); }
//
//template<> inline int saturate_cast<int>(float v)            { return cvRound(v); }
//template<> inline int saturate_cast<int>(double v)           { return cvRound(v); }
//
//// we intentionally do not clip negative numbers, to make -1 become 0xffffffff etc.
//template<> inline unsigned saturate_cast<unsigned>(float v)  { return cvRound(v); }
//template<> inline unsigned saturate_cast<unsigned>(double v) { return cvRound(v); }
//
//} // fbc
//
//#endif // FBC_OPENCV_TEST_SATURATE_HPP_
