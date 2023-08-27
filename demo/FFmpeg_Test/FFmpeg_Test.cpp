#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_ffmpeg_libavfilter_movie_multi_thread();

	if (ret == 0) fprintf(stdout, "========== test success ==========\n");
	else fprintf(stderr, "########## test fail: %d ##########\n", ret);

	return 0;
}

