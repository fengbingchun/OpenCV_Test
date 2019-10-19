// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_IPLIMAGE_HPP_
#define FBC_CV_IPLIMAGE_HPP_

// reference: 2.4.13.6
//            core/include/opencv2/core/types_c.h
//            core/include/opencv2/core/core_c.h

#include "core/types.hpp"
#include "core/interface.hpp"

namespace fbc {

typedef struct _IplROI {
	int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
	int  xOffset;
	int  yOffset;
	int  width;
	int  height;
} IplROI;

typedef struct _IplTileInfo IplTileInfo;

typedef struct _IplImage {
	int  nSize;             /* sizeof(IplImage) */
	int  ID;                /* version (=0)*/
	int  nChannels;         /* Most of OpenCV functions support 1,2,3 or 4 channels */
	int  alphaChannel;      /* Ignored by OpenCV */
	int  depth;             /* Pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
				IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported.  */
	char colorModel[4];     /* Ignored by OpenCV */
	char channelSeq[4];     /* ditto */
	int  dataOrder;         /* 0 - interleaved color channels, 1 - separate color channels.
				cvCreateImage can only create interleaved images */
	int  origin;            /* 0 - top-left origin,
				1 - bottom-left origin (Windows bitmaps style).  */
	int  align;             /* Alignment of image rows (4 or 8).
				OpenCV ignores it and uses widthStep instead.    */
	int  width;             /* Image width in pixels.                           */
	int  height;            /* Image height in pixels.                          */
	struct _IplROI *roi;    /* Image ROI. If NULL, the whole image is selected. */
	struct _IplImage *maskROI;      /* Must be NULL. */
	void  *imageId;                 /* "           " */
	struct _IplTileInfo *tileInfo;  /* "           " */
	int  imageSize;         /* Image data size in bytes
				(==image->height*image->widthStep
				in case of interleaved data)*/
	char *imageData;        /* Pointer to aligned image data.         */
	int  widthStep;         /* Size of aligned image row in bytes.    */
	int  BorderMode[4];     /* Ignored by OpenCV.                     */
	int  BorderConst[4];    /* Ditto.                                 */
	char *imageDataOrigin;  /* Pointer to very origin of image data
				(not necessarily aligned) -
				needed for correct deallocation */
} IplImage;

typedef IplImage* (CV_STDCALL* Cv_iplCreateImageHeader)
	(int, int, int, char*, char*, int, int, int, int, int,
	IplROI*, IplImage*, void*, IplTileInfo*);
typedef void (CV_STDCALL* Cv_iplAllocateImageData)(IplImage*, int, int);
typedef void (CV_STDCALL* Cv_iplDeallocate)(IplImage*, int);
typedef IplROI* (CV_STDCALL* Cv_iplCreateROI)(int, int, int, int, int);
typedef IplImage* (CV_STDCALL* Cv_iplCloneImage)(const IplImage*);

/* Creates IPL image (header and data) */
IplImage* cvCreateImage(CvSize size, int depth, int channels);
/* Allocates and initializes IplImage header */
IplImage*  cvCreateImageHeader(CvSize size, int depth, int channels);
/* Inializes IplImage header */
IplImage* cvInitImageHeader(IplImage* image, CvSize size, int depth,
	int channels, int origin = 0, int align = 4);
/* Releases IPL image header and data */
void cvReleaseImage(IplImage** image);
/* Releases (i.e. deallocates) IPL image header */
void cvReleaseImageHeader(IplImage** image);
/* Allocates array data */
void cvCreateData(CvArr* arr);
/* Releases array data */
void cvReleaseData(CvArr* arr);

} // namespace fbc

#endif // FBC_CV_IPLIMAGE_HPP_
