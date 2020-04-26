#include "funset.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <libusb.h>

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/105767565
namespace {

bool running = true;

int LIBUSB_CALL hotplug_callback(libusb_context* ctx, libusb_device* dev, libusb_hotplug_event event, void* user_data)
{
	struct libusb_device_descriptor desc;
	int ret = libusb_get_device_descriptor(dev, &desc);
	if (LIBUSB_SUCCESS != ret) {
		fprintf(stderr, "fail to get device descriptor: %d, %s\n", ret, libusb_error_name(ret));
		//return -1;
	}

	if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
		fprintf(stdout, "device attached: %04x:%04x\n", desc.idVendor, desc.idProduct);
	else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT)
		fprintf(stdout, "device detached: %04x:%04x\n", desc.idVendor, desc.idProduct);
	else
		fprintf(stderr, "Error: unsupported hotplug events\n");

	return 0;
}

void run()
{
	for (int i = 0; i < 60; ++i)
		std::this_thread::sleep_for(std::chrono::seconds(1));
	running = false;
}

} // namespace

int test_libusb_hotplug()
{
	// reference: examples/hotplugtest.c
#ifdef _MSC_VER
	const char* platform = "Windows";
#else
	const char* platform = "Linux";
#endif

	int ret = libusb_init(nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to init: %d, %s\n", ret, libusb_error_name(ret));
		return -1;
	}

	if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
		fprintf(stderr, "hotplug capabilites are not supported on this platform: %s\n", platform);
		libusb_exit(nullptr);
		return -1;
	}

	int vendor_id = 0x046d, product_id = 0x081b;
	libusb_hotplug_callback_handle hp[2];
	ret = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_NO_FLAGS, vendor_id,
		product_id, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, nullptr, &hp[0]);
	if (LIBUSB_SUCCESS != ret) {
		fprintf(stderr, "fail to register callback arrived: %d, %s\n", ret, libusb_error_name(ret));
		libusb_exit(nullptr);
		return -1;
	}

	ret = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_NO_FLAGS, vendor_id,
		product_id, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, nullptr, &hp[1]);
	if (LIBUSB_SUCCESS != ret) {
		fprintf(stderr, "fail to register callback left: %d, %s\n", ret, libusb_error_name(ret));
		libusb_exit(nullptr);
		return -1;
	}

	std::thread th(run);

	while (running) {
		//ret = libusb_handle_events(nullptr);
		timeval tv = {1, 0};
		ret = libusb_handle_events_timeout(nullptr, &tv);
		if (ret < 0)
			fprintf(stderr, "fail to libusb_handle_events: %d, %s\n", ret, libusb_error_name(ret));
	}

	libusb_exit(nullptr);
	th.join();

	return 0;
}

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/105712776
int test_libusb_get_devices_list()
{
	// reference: examples/listdevs.c
	int ret = libusb_init(nullptr);
	if (ret != 0) {
		fprintf(stderr, "fail to init: %d\n", ret);
		return -1;
	}

	libusb_device** devs = nullptr;
	ssize_t count = libusb_get_device_list(nullptr, &devs);
	if (count < 0) {
		fprintf(stderr, "fail to get device list: %d\n", count);
		libusb_exit(nullptr);
		return -1;
	}

	libusb_device* dev = nullptr;
	int i = 0;

	while ((dev = devs[i++]) != nullptr) {
		struct libusb_device_descriptor desc;
		ret = libusb_get_device_descriptor(dev, &desc);
		if (ret < 0) {
			fprintf(stderr, "fail to get device descriptor: %d\n", ret);
			return -1;
		}

		fprintf(stdout, "%04x:%04x (bus: %d, device: %d) ",
			desc.idVendor, desc.idProduct, libusb_get_bus_number(dev), libusb_get_device_address(dev));

		uint8_t path[8];
		ret = libusb_get_port_numbers(dev, path, sizeof(path));
		if (ret > 0) {
			fprintf(stdout, "path: %d", path[0]);
			for (int j = 1; j < ret; ++j)
				fprintf(stdout, ".%d", path[j]);
		}
		fprintf(stdout, "\n");
	}

	libusb_free_device_list(devs, 1);
	libusb_exit(nullptr);

	return 0;
}

