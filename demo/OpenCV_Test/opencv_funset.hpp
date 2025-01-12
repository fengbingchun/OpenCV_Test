#ifndef FBC_OPENCV_FUNSET_HPP_
#define FBC_OPENCV_FUNSET_HPP_

int test_libexif_thumbnail();
int test_read_write_video();
int test_write_video();

int test_opencv_color_correction_Macbeth();
int test_opencv_camera_calibration();
int test_opencv_two_merge_one_image(int flag = 0); // 将两幅图像合并成一幅图像: flag: 0:垂直; 1:水平
int test_opencv_grab_video_frame();
int test_opencv_videocapture();
int test_opencv_resize_cplusplus();
int test_opencv_kmeans(); // k-均值聚类
int test_opencv_Laplacian(); // 边缘检测：拉普拉斯
int test_opencv_PCA(); // 主成分分析(Principal Components Analysis, PCA)
int test_opencv_calcCovarMatrix(); // 计算协方差矩阵
int test_opencv_meanStdDev(); // 计算均值和标准差
int test_opencv_trace(); // 矩阵的迹运算
int test_opencv_pseudoinverse(); // 求伪逆矩阵
int test_opencv_SVD();
int test_opencv_eigen(); // 矩阵的特征值和特征向量
int test_opencv_norm();
int test_opencv_inverse();
int test_opencv_determinant();
int test_opencv_resize();
int test_opencv_cvtColor();
int test_opencv_split();
int test_opencv_merge();
int test_opencv_warpAffine();
int test_opencv_remap();
int test_opencv_rotate();
int test_opencv_warpPerspective();
int test_opencv_dilate();
int test_opencv_erode();
int test_opencv_morphologyEx();
int test_opencv_threshold();
int test_opencv_transpose();
int test_opencv_flip();
int test_opencv_dft();
int test_opencv_filter2D();

#endif // FBC_OPENCV_FUNSET_HPP_
