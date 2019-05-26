# OpenCV_Test
**The main role of the project:**
- OpenCV's usage [OpenCV GitHub](https://github.com/opencv/opencv)
- fbc_cv library: an open source image process library
- libyuv's usage [libyuv GitHub](https://github.com/lemenkov/libyuv)
- VLFeat's usage [vlfeat.org](http://www.vlfeat.org/)
- Vigra's usage [vigra GitHub](https://github.com/ukoethe/vigra)
- CImg's usage [cimg.eu](http://www.cimg.eu/)
- FFmpeg'usage [ffmpeg.org](https://ffmpeg.org/)
- LIVE555'usage [LIVE555.COM](http://live555.com/)

**The project support platform:**
- windows7/10 64 bits: It can be directly build with VS2013 in windows7/10 64bits.
- Linux:
	- OpenCV_Test support cmake build(file position: prj/linux_cmake_OpenCV_Test)
	- FFmpeg_Test support cmake build(test code include: FFmpeg, LIVE555; file position: prj/linux_cmake_FFmpeg_Test)

**OpenCV's version: 3.1**
- close support for OpenCL/CUDA/SIMD/TBB/OpenMP when build with CMake
- modify modules/core/include/opencv2/core/cvdef.h,close SIMD support: adjust line 167 to: #if 0
- in order to keep the linux and windows results consistent, modify modules/core/include/opencv2/core/fast_math.hpp: make cvFound/cvFloor/cvCeil/cvRound to execute the last branch
- insure that all algorithms are implemented with c++

# fbc_cv
- it is an open source image process library
- most of the algorithms come from OpenCV3.1
- it has a template class Mat_ replace of OpenCV's Mat class: src/fbc_cv/include/core/mat.hpp
- interface names are consistent with OpenCV3.1
- each algorithm's result is same with OpenCV3.1
- the codes are written in C++ without dependence on any 3rd-party libraries

**fbc_cv have been implemented include:**
- resize
- cvtColor
- merge/split
- remap
- warpAffine
- rotate
- warpPerspective
- dilate
- erode
- morphologyEx
- threshold
- transpose
- flip
- dft/idft

**Screenshot:**  
![](https://github.com/fengbingchun/OpenCV_Test/blob/master/prj/x86_x64_vc12/Screenshot.png)

**Blog:**
- [opencv/fbc_cv](https://blog.csdn.net/fengbingchun/article/category/721609)
- [ffmpeg/live555](https://blog.csdn.net/fengbingchun/article/category/3069675)

**fbc_cv library licence: uses the same licence as OpenCV3.1**
