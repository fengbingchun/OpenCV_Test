#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_v4l2_get_video_device_info();

	if (ret == 0) fprintf(stdout, "========== test success ==========\n");
	else fprintf(stderr, "########## test fail ##########\n");

	return 0;
}

