// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

// reference: 2.4.13.6
//            core/src/array.cpp

#include "iplimage.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "core/fbcstd.hpp"

namespace fbc {

static struct
{
	Cv_iplCreateImageHeader  createHeader;
	Cv_iplAllocateImageData  allocateData;
	Cv_iplDeallocate  deallocate;
	Cv_iplCreateROI  createROI;
	Cv_iplCloneImage  cloneImage;
} CvIPL;

static  void icvGetColorModel(int nchannels, const char** colorModel, const char** channelSeq)
{
	static const char* tab[][2] =
	{
		{ "GRAY", "GRAY" },
		{ "", "" },
		{ "RGB", "BGR" },
		{ "RGB", "BGRA" }
	};

	nchannels--;
	*colorModel = *channelSeq = "";

	if ((unsigned)nchannels <= 3)
	{
		*colorModel = tab[nchannels][0];
		*channelSeq = tab[nchannels][1];
	}
}

// create IplImage header
IplImage*  cvCreateImageHeader(CvSize size, int depth, int channels)
{
	IplImage *img = 0;

	if (!CvIPL.createHeader)
	{
		img = (IplImage *)cvAlloc(sizeof(*img));
		cvInitImageHeader(img, size, depth, channels, IPL_ORIGIN_TL,
			CV_DEFAULT_IMAGE_ROW_ALIGN);
	}
	else
	{
		const char *colorModel, *channelSeq;

		icvGetColorModel(channels, &colorModel, &channelSeq);

		img = CvIPL.createHeader(channels, 0, depth, (char*)colorModel, (char*)channelSeq,
			IPL_DATA_ORDER_PIXEL, IPL_ORIGIN_TL,
			CV_DEFAULT_IMAGE_ROW_ALIGN,
			size.width, size.height, 0, 0, 0, 0);
	}

	return img;
}

// create IplImage header and allocate underlying data
IplImage* cvCreateImage(CvSize size, int depth, int channels)
{
	IplImage *img = cvCreateImageHeader(size, depth, channels);
	assert(img);
	cvCreateData(img);

	return img;
}

void cvReleaseImage(IplImage** image)
{
	if (!image)
		printf("pointer null\n");

	if (*image)
	{
		IplImage* img = *image;
		*image = 0;

		cvReleaseData(img);
		cvReleaseImageHeader(&img);
	}
}

// initialize IplImage header, allocated by the user
IplImage* cvInitImageHeader(IplImage * image, CvSize size, int depth,
	int channels, int origin, int align)
{
	const char *colorModel, *channelSeq;

	if (!image)
		printf("null pointer to header\n");

	memset(image, 0, sizeof(*image));
	image->nSize = sizeof(*image);

	icvGetColorModel(channels, &colorModel, &channelSeq);
	strncpy(image->colorModel, colorModel, 4);
	strncpy(image->channelSeq, channelSeq, 4);

	if (size.width < 0 || size.height < 0)
		printf("Bad input roi\n");

	if ((depth != (int)IPL_DEPTH_1U && depth != (int)IPL_DEPTH_8U &&
		depth != (int)IPL_DEPTH_8S && depth != (int)IPL_DEPTH_16U &&
		depth != (int)IPL_DEPTH_16S && depth != (int)IPL_DEPTH_32S &&
		depth != (int)IPL_DEPTH_32F && depth != (int)IPL_DEPTH_64F) ||
		channels < 0)
		printf("Unsupported format\n");
	if (origin != CV_ORIGIN_BL && origin != CV_ORIGIN_TL)
		printf("Bad input origin\n");

	if (align != 4 && align != 8)
		printf("Bad input align\n");

	image->width = size.width;
	image->height = size.height;

	if (image->roi)
	{
		image->roi->coi = 0;
		image->roi->xOffset = image->roi->yOffset = 0;
		image->roi->width = size.width;
		image->roi->height = size.height;
	}

	image->nChannels = MAX(channels, 1);
	image->depth = depth;
	image->align = align;
	image->widthStep = (((image->width * image->nChannels *
		(image->depth & ~IPL_DEPTH_SIGN) + 7) / 8) + align - 1) & (~(align - 1));
	image->origin = origin;
	image->imageSize = image->widthStep * image->height;

	return image;
}

void cvReleaseImageHeader(IplImage** image)
{
	if (!image)
		printf("pointer null\n");

	if (*image)
	{
		IplImage* img = *image;
		*image = 0;

		if (!CvIPL.deallocate)
		{
			cvFree(&img->roi);
			cvFree(&img);
		}
		else
		{
			CvIPL.deallocate(img, IPL_IMAGE_HEADER | IPL_IMAGE_ROI);
		}
	}
}

// Allocates underlying array data
void cvCreateData(CvArr* arr)
{
	if (CV_IS_MAT_HDR_Z(arr))
	{
		printf("don't support mat header");
		//size_t step, total_size;
		//CvMat* mat = (CvMat*)arr;
		//step = mat->step;

		//if (mat->rows == 0 || mat->cols == 0)
		//	return;

		//if (mat->data.ptr != 0)
		//	printf("Data is already allocated\n");

		//if (step == 0)
		//	step = CV_ELEM_SIZE(mat->type)*mat->cols;

		//int64 _total_size = (int64)step*mat->rows + sizeof(int)+CODEC_MALLOC_ALIGN;
		//total_size = (size_t)_total_size;
		//if (_total_size != (int64)total_size)
		//	printf("Too big buffer is allocated\n");
		//mat->refcount = (int*)cvAlloc((size_t)total_size);
		//mat->data.ptr = (uchar*)cvAlignPtr(mat->refcount + 1, CODEC_MALLOC_ALIGN);
		//*mat->refcount = 1;
	}
	else if (CV_IS_IMAGE_HDR(arr))
	{
		IplImage* img = (IplImage*)arr;

		if (img->imageData != 0)
			printf("Data is already allocated\n");

		if (!CvIPL.allocateData)
		{
			img->imageData = img->imageDataOrigin =
				(char*)cvAlloc((size_t)img->imageSize);
		}
		else
		{
			int depth = img->depth;
			int width = img->width;

			if (img->depth == IPL_DEPTH_32F || img->depth == IPL_DEPTH_64F)
			{
				img->width *= img->depth == IPL_DEPTH_32F ? sizeof(float) : sizeof(double);
				img->depth = IPL_DEPTH_8U;
			}

			CvIPL.allocateData(img, 0, 0);

			img->width = width;
			img->depth = depth;
		}
	}
	else if (CV_IS_MATND_HDR(arr))
	{
		printf("don't support matnd header\n");
		//CvMatND* mat = (CvMatND*)arr;
		//int i;
		//size_t total_size = CV_ELEM_SIZE(mat->type);

		//if (mat->dim[0].size == 0)
		//	return;

		//if (mat->data.ptr != 0)
		//	printf("Data is already allocated\n");

		//if (CV_IS_MAT_CONT(mat->type))
		//{
		//	total_size = (size_t)mat->dim[0].size*(mat->dim[0].step != 0 ?
		//		(size_t)mat->dim[0].step : total_size);
		//}
		//else
		//{
		//	for (i = mat->dims - 1; i >= 0; i--)
		//	{
		//		size_t size = (size_t)mat->dim[i].step*mat->dim[i].size;

		//		if (total_size < size)
		//			total_size = size;
		//	}
		//}

		//mat->refcount = (int*)cvAlloc(total_size +
		//	sizeof(int)+CODEC_MALLOC_ALIGN);
		//mat->data.ptr = (uchar*)cvAlignPtr(mat->refcount + 1, CODEC_MALLOC_ALIGN);
		//*mat->refcount = 1;
	}
	else
		printf("unrecognized or unsupported array type\n");
}

// Deallocates array's data
void cvReleaseData(CvArr* arr)
{
	if (CV_IS_MAT_HDR(arr) || CV_IS_MATND_HDR(arr))
	{
		printf("don't support mat header and matnd header release\n");
		//CvMat* mat = (CvMat*)arr;
		//cvDecRefData(mat);
	}
	else if (CV_IS_IMAGE_HDR(arr))
	{
		IplImage* img = (IplImage*)arr;

		if (!CvIPL.deallocate)
		{
			char* ptr = img->imageDataOrigin;
			img->imageData = img->imageDataOrigin = 0;
			cvFree(&ptr);
		}
		else
		{
			CvIPL.deallocate(img, IPL_IMAGE_DATA);
		}
	}
	else
		printf("unrecognized or unsupported array type\n");
}

} // namespace fbc
