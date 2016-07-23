#include "test_all.hpp"
#include <assert.h>

int run_all_test()
{
	// test core
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
	test_directory_GetListFiles();
	test_directory_GetListFilesR();
	test_directory_GetListFolders();

	// test cvtColor
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
	ret = test_merge_uchar();
	assert(ret == 0);
	ret = test_merge_float();
	assert(ret == 0);

	// test resize
	ret = test_resize_uchar();
	assert(ret == 0);
	ret = test_resize_float();
	assert(ret == 0);
	ret = test_resize_area();
	assert(ret == 0);

	// test split
	ret = test_split_uchar();
	assert(ret == 0);
	ret = test_split_float();
	assert(ret == 0);

	// test remap
	ret = test_remap_uchar();
	assert(ret == 0);
	ret = test_remap_float();
	assert(ret == 0);

	// test warpAffine
	ret = test_getAffineTransform();
	assert(ret == 0);
	ret = test_warpAffine_uchar();
	assert(ret == 0);
	ret = test_warpAffine_float();
	assert(ret == 0);

	// test rotate
	ret = test_getRotationMatrix2D();
	assert(ret == 0);
	ret = test_rotate_uchar();
	assert(ret == 0);
	ret = test_rotate_float();
	assert(ret == 0);
	ret = test_rotate_without_crop();
	assert(ret == 0);

	// test warpPerspective
	ret = test_getPerspectiveTransform();
	assert(ret == 0);
	ret = test_warpPerspective_uchar();
	assert(ret == 0);
	ret = test_warpPerspective_float();
	assert(ret == 0);


	return 0;
}
