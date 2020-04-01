#include <iostream>
#include "funset.hpp"

int main()
{
	int ret = test_get_usb_camera_vid_pid();

	if (ret == 0) fprintf(stdout, "========== test success ==========\n");
	else fprintf(stderr, "########## test fail ##########\n");

	return 0;
}

