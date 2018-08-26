#include "funset.hpp"
#include <iostream>
#include <string>
#include <vigra/stdimage.hxx>
#include <vigra/impex.hxx> // Image import and export functions
#include <vigra/edgedetection.hxx>

int test_vigra_1()
{
	try {
		std::cout << "supported formats: " << vigra::impexListFormats() << std::endl;

		std::string strImageName = "E:/GitCode/OpenCV_Test/test_images/lenna.bmp";
		std::string strOutImage = "E:/GitCode/OpenCV_Test/test_images/lenna_vigra.bmp";

		vigra::ImageImportInfo info(strImageName.c_str(), 0);//read image
		//vigra_precondition(info.isGrayscale(), "Sorry, cannot operate on color images");

		double threshold = 200, scale = 0.5;

		if (info.isGrayscale()) {
			vigra::BImage out(info.width(), info.height()); // create a gray scale image of appropriate size
			vigra::BImage in(info.width(), info.height());

			vigra::importImage(info, destImage(in));

			out = 255;// paint output image white

			vigra::importImage(info, destImage(out));// import the image just read

			//differenceOfExponentialEdgeImage(srcImageRange(in), destImage(out), scale, threshold, 0);
			//cannyEdgeImage(srcImageRange(in), destImage(out), scale, threshold, 0);// call edge detection algorithm
			vigra::transformImage(srcImageRange(in), destImage(out), vigra::linearIntensityTransform(-1, -255));//invert image
			vigra::exportImage(srcImageRange(out), vigra::ImageExportInfo(strOutImage.c_str()));// write the image to the file
		} else {
			vigra::BRGBImage out(info.width(), info.height());// create a RGB image of appropriate size
			vigra::BRGBImage in(info.width(), info.height());

			vigra::importImage(info, destImage(out));
			vigra::importImage(info, destImage(in));

			//vigra::RGBValue<int> offset(-255, -255, -255);
			//vigra::transformImage(srcImageRange(in), destImage(out), vigra::linearIntensityTransform(-1, offset));

			double sizefactor = 1.2;

			int nw = (int)(sizefactor*(info.width() - 1) + 1.5); // calculate new image size
			int nh = (int)(sizefactor*(info.height() - 1) + 1.5);

			vigra::BRGBImage out1(nw, nh);
			vigra::resizeImageSplineInterpolation(srcImageRange(in), vigra::destImageRange(out1));// resize the image, using a bi-cubic spline algorithms
			vigra::exportImage(srcImageRange(out1), vigra::ImageExportInfo(strOutImage.c_str()));
		}

	} catch (vigra::StdException &e) {
		std::cout << e.what() << std::endl; // catch any errors that might have occurred and print their reason
		return -1;
	}

	return 0;
}

