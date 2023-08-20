#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_ffmpeg_libavfilter_movie("E:/yulong.mp4");

	if (ret == 0) fprintf(stdout, "========== test success ==========\n");
	else fprintf(stderr, "########## test fail: %d ##########\n", ret);

	return 0;
}

