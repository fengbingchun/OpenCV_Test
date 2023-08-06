#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_ffmpeg_save_video_slice();

	if (ret == 0) fprintf(stdout, "========== test success ==========\n");
	else fprintf(stderr, "########## test fail: %d ##########\n", ret);

	return 0;
}

