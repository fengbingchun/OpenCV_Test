# OpenCV_Test
*The project has two main effect:*
- OpenCV's usage
- fbc_cv library
***
*The project support platform: windows7 64 bits. It can be directly build with VS2013 in windows7 64bits.*
***
*OpenCV's version: 3.1*
- close support for OpenCL/CUDA/SIMD/TBB/OpenMP when build with CMake
- modify sources/modules/core/include/opencv2/core/cvdef.h, for example: #define CV_SSE2 0
- insure that all algorithms are implemented with c++
***
# fbc_cv
- it is an open source image process library
- most of the algorithms come from OpenCV3.1
- it has a template class Mat_ replace of OpenCV's Mat class: src/fbc_cv/include/core/mat.hpp
- interface names are consistent with OpenCV3.1
- each algorithm's result is same with OpenCV3.1
***
*The algorithms have been implemented include:*
- resize
- cvtColor
- merge
- split
- remap
- warpAffine
- rotate
- warpPerspective
- dilate
- erode
- morphologyEx
*Licence: uses the same licence as OpenCV3.1*
