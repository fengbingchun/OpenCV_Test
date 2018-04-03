#ifndef FBC_FBC_CV_FUNSET_HPP_
#define FBC_FBC_CV_FUNSET_HPP_

int test_fast_math();
int test_base();
int test_saturate();
int test_Matx();
int test_Vec();
int test_Point();
int test_Point3();
int test_Size();
int test_Rect();
int test_Range();
int test_Scalar();
int test_Mat();
int test_RotateRect();

int test_cvtColor_RGB2RGB();
int test_cvtColor_RGB2Gray();
int test_cvtColor_Gray2RGB();
int test_cvtColor_RGB2YCrCb();
int test_cvtColor_YCrCb2RGB();
int test_cvtColor_RGB2XYZ();
int test_cvtColor_XYZ2RGB();
int test_cvtColor_RGB2HSV();
int test_cvtColor_HSV2RGB();
int test_cvtColor_RGB2Lab();
int test_cvtColor_Lab2RGB();
int test_cvtColor_YUV2BGR();
int test_cvtColor_BGR2YUV();
int test_cvtColor_YUV2Gray();

int test_dft_float();

int test_getStructuringElement();
int test_dilate_uchar();
int test_dilate_float();

int test_directory_GetListFiles();
int test_directory_GetListFilesR();
int test_directory_GetListFolders();

int test_erode_uchar();
int test_erode_float();

int test_flip_uchar();
int test_flip_float();

int test_merge_uchar();
int test_merge_float();

int test_morphologyEx_uchar();
int test_morphologyEx_float();
int test_morphologyEx_hitmiss();

int test_remap_uchar();
int test_remap_float();

int test_resize_uchar();
int test_resize_float();
int test_resize_area();

int test_getRotationMatrix2D();
int test_rotate_uchar();
int test_rotate_float();
int test_rotate_without_crop();

int test_rotate90();

int test_split_uchar();
int test_split_float();

int test_threshold_uchar();
int test_threshold_float();

int test_transpose_uchar();
int test_transpose_float();

int test_getAffineTransform();
int test_warpAffine_uchar();
int test_warpAffine_float();

int test_getPerspectiveTransform();
int test_warpPerspective_uchar();
int test_warpPerspective_float();

int run_all_test();

#endif // FBC_FBC_CV_FUNSET_HPP_

