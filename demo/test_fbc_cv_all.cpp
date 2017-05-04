#include "fbc_cv_funset.hpp"
#include <assert.h>
#include <iostream>

int run_all_test()
{
	// test core
	std::cout << "test core: " << std::endl;
	test_fast_math();
	test_base();
	test_saturate();
	test_Matx();
	test_Vec();
	test_Point();
	test_Point3();
	test_Size();
	test_Rect();
	test_Range();
	test_Scalar();
	test_Mat();
	test_RotateRect();

	// test directory
	std::cout << "test directory: " << std::endl;
	test_directory_GetListFiles();
	test_directory_GetListFilesR();
	test_directory_GetListFolders();

	// test cvtColor
	std::cout << "test cvtColor: " << std::endl;
	int ret = test_cvtColor_RGB2RGB();
	assert(ret == 0);
	ret = test_cvtColor_RGB2Gray();
	assert(ret == 0);
	ret = test_cvtColor_Gray2RGB();
	assert(ret == 0);
	ret = test_cvtColor_RGB2YCrCb();
	assert(ret == 0);
	ret = test_cvtColor_YCrCb2RGB();
	assert(ret == 0);
	ret = test_cvtColor_RGB2XYZ();
	assert(ret == 0);
	ret = test_cvtColor_XYZ2RGB();
	assert(ret == 0);
	ret = test_cvtColor_RGB2HSV();
	assert(ret == 0);
	ret = test_cvtColor_HSV2RGB();
	assert(ret == 0);
	ret = test_cvtColor_RGB2Lab();
	assert(ret == 0);
	ret = test_cvtColor_Lab2RGB();
	assert(ret == 0);
	ret = test_cvtColor_YUV2BGR();
	assert(ret == 0);
	ret = test_cvtColor_BGR2YUV();
	assert(ret == 0);
	ret = test_cvtColor_YUV2Gray();
	assert(ret == 0);

	// test merge
	std::cout << "test merge: " << std::endl;
	ret = test_merge_uchar();
	assert(ret == 0);
	ret = test_merge_float();
	assert(ret == 0);

	// test split
	std::cout << "test split: " << std::endl;
	ret = test_split_uchar();
	assert(ret == 0);
	ret = test_split_float();
	assert(ret == 0);

	// test resize
	std::cout << "test resize: " << std::endl;
	ret = test_resize_uchar();
	assert(ret == 0);
	ret = test_resize_float();
	assert(ret == 0);
	ret = test_resize_area();
	assert(ret == 0);

	// test remap
	std::cout << "test remap: " << std::endl;
	ret = test_remap_uchar();
	assert(ret == 0);
	ret = test_remap_float();
	assert(ret == 0);

	// test warpAffine
	std::cout << "test warpAffine: " << std::endl;
	ret = test_getAffineTransform();
	assert(ret == 0);
	ret = test_warpAffine_uchar();
	assert(ret == 0);
	ret = test_warpAffine_float();
	assert(ret == 0);

	// test rotate
	std::cout << "test rotate: " << std::endl;
	ret = test_getRotationMatrix2D();
	assert(ret == 0);
	ret = test_rotate_uchar();
	assert(ret == 0);
	ret = test_rotate_float();
	assert(ret == 0);
	ret = test_rotate_without_crop();
	assert(ret == 0);

	// test warpPerspective
	std::cout << "test warpPerspective: " << std::endl;
	ret = test_getPerspectiveTransform();
	assert(ret == 0);
	ret = test_warpPerspective_uchar();
	assert(ret == 0);
	ret = test_warpPerspective_float();
	assert(ret == 0);

	// test dilate
	std::cout << "test dilate: " << std::endl;
	ret = test_getStructuringElement();
	assert(ret == 0);
	ret = test_dilate_uchar();
	assert(ret == 0);
	ret = test_dilate_float();
	assert(ret == 0);

	// test erode
	std::cout << "test erode: " << std::endl;
	ret = test_erode_uchar();
	assert(ret == 0);
	ret = test_erode_float();
	assert(ret == 0);

	// test morphologyEx
	std::cout << "test morphologyEx: " << std::endl;
	ret = test_morphologyEx_uchar();
	assert(ret == 0);
	ret = test_morphologyEx_float();
	assert(ret == 0);
	ret = test_morphologyEx_hitmiss();
	assert(ret == 0);

	// test threshold
	std::cout << "test threshold: " << std::endl;
	ret = test_threshold_uchar();
	assert(ret == 0);
	ret = test_threshold_float();
	assert(ret == 0);

	// test transpose
	std::cout << "test transpose: " << std::endl;
	ret = test_transpose_uchar();
	assert(ret == 0);
	ret = test_transpose_float();
	assert(ret == 0);

	// test flip
	std::cout << "test flip: " << std::endl;
	ret = test_flip_uchar();
	assert(ret == 0);
	ret = test_flip_float();
	assert(ret == 0);

	// test rotate 90
	std::cout << "test rotate 90: " << std::endl;
	ret = test_rotate90();
	assert(ret == 0);

	// test dft
	std::cout << "test dft: " << std::endl;
	ret = test_dft_float();
	assert(ret == 0);

	return 0;
}
