// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#include "imgproc.hpp"
#include "filterengine.hpp"


/* reference: include/opencv2/imgproc.hpp
              modules/imgproc/src/morph.cpp
*/

namespace fbc {

int getStructuringElement(Mat_<uchar, 1>& dst, int shape, Size ksize, Point anchor)
{
	int i, j;
	int r = 0, c = 0;
	double inv_r2 = 0;

	FBC_Assert(shape == MORPH_RECT || shape == MORPH_CROSS || shape == MORPH_ELLIPSE);

	anchor = normalizeAnchor(anchor, ksize);

	if (ksize == Size(1, 1))
		shape = MORPH_RECT;

	if (shape == MORPH_ELLIPSE) {
		r = ksize.height / 2;
		c = ksize.width / 2;
		inv_r2 = r ? 1. / ((double)r*r) : 0;
	}

	for (i = 0; i < ksize.height; i++) {
		uchar* ptr = const_cast<uchar*>(dst.ptr(i));
		int j1 = 0, j2 = 0;

		if (shape == MORPH_RECT || (shape == MORPH_CROSS && i == anchor.y)) {
			j2 = ksize.width;
		}
		else if (shape == MORPH_CROSS) {
			j1 = anchor.x, j2 = j1 + 1;
		} else {
			int dy = i - r;
			if (std::abs(dy) <= r) {
				int dx = saturate_cast<int>(c*std::sqrt((r*r - dy*dy)*inv_r2));
				j1 = std::max(c - dx, 0);
				j2 = std::min(c + dx + 1, ksize.width);
			}
		}

		for (j = 0; j < j1; j++)
			ptr[j] = 0;
		for (; j < j2; j++)
			ptr[j] = 1;
		for (; j < ksize.width; j++)
			ptr[j] = 0;
	}

	return 0;
}

} // namespace fbc
