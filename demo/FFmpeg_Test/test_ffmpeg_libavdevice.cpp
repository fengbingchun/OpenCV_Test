#include "funset.hpp"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavdevice/avdevice.h>

#ifdef __cplusplus
}
#endif

int test_ffmpeg_libavdevice_device_list()
{
	/*AVFormatContext* ctx = avformat_alloc_context();
	AVDeviceInfoList* list = nullptr;

	// crash both windows and linux
	int count = avdevice_list_devices(ctx, &list);
	if (count < 0) {
		fprintf(stderr, "fail to avdevice_list_devices: %d\n", count);
		return -1;
	}
	fprintf(stdout, "available devcie count: %d\n", count);

	avdevice_free_list_devices(&list);
	avformat_close_input(&ctx);*/


	fprintf(stdout, "test finish\n");
	return 0;
}

