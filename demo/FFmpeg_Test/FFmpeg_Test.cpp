#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_ffmpeg_stream_show_two_thread();

	if (ret == 0) fprintf(stdout, "========== test success ==========\n");
	else fprintf(stderr, "########## test fail ##########\n");

	return 0;
}

