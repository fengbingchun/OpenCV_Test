#ifdef _MSC_VER
#define CINTERFACE
#define COBJMACROS
#include <strmif.h>
#include <Setupapi.h>
#include <uuids.h>
#include <devguid.h>
#include <memory>
#endif // _MSC_VER

#include "funset.hpp"
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

#ifndef _MSC_VER
// Blog: https://blog.csdn.net/fengbingchun/article/details/103507754
namespace {

int parse_input_devices(const std::string& name, unsigned int& vid, unsigned int& pid)
{
	const std::string device_list_file = "/proc/bus/input/devices";
	std::ifstream file_input(device_list_file.c_str());
	if (!file_input.is_open()) {
		fprintf(stderr, "fail to open file: %s\n", device_list_file.c_str());
		return -1;
	}

	std::string current_line, bus_line, search_name_line = name, search_bus_line = "Bus=";
	while (getline(file_input, current_line)) {
		auto pos = current_line.find(search_bus_line);
		if (pos != std::string::npos)
		      bus_line = current_line;

		pos = current_line.find(search_name_line);
		if (pos != std::string::npos)
		      break;
	}
	file_input.close();

	auto pos = bus_line.find("Vendor");
	if (pos != std::string::npos) {
		std::string str = bus_line.substr(pos+7, 4);
		std::istringstream(str) >> std::hex >> vid;
	} else {
		fprintf(stderr, "not found vid\n");
		return -1;
	}

	pos = bus_line.find("Product");
	if (pos != std::string::npos) {
		std::string str = bus_line.substr(pos+8, 4);
		std::istringstream(str) >> std::hex >> pid;
	} else {
		fprintf(stderr, "not found pid\n");
		return -1;
	}

	return 0;
}

int parse_input_devices2(const std::string& name, unsigned int& vid, unsigned int& pid)
{
	const std::string device_list_file = "/proc/bus/input/devices";
	std::ifstream file_input(device_list_file.c_str());
	if (!file_input.is_open()) {
		fprintf(stderr, "fail to open file: %s\n", device_list_file.c_str());
		return -1;
	}

	std::string current_line, event_line, search_name_line = name, search_event_line = "event";
	bool flag = false;
	while (getline(file_input, current_line)) {
		auto pos = current_line.find(search_name_line);
		if (pos != std::string::npos)
		      flag = true;
		else if (!flag)
			continue;

		if (flag) {
			pos = current_line.find(search_event_line);
			if (pos != std::string::npos) {
				event_line = current_line;
				break;
			}
		}
	}
	file_input.close();

	if (!flag) {
		fprintf(stderr, "not found event\n");
		return -1;
	}

	auto pos = event_line.find(search_event_line);
	if (pos != std::string::npos) {
		std::string str = event_line.substr(pos, std::string::npos);
		str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
		std::string vid_str, pid_str;
		std::string prefix = "/sys/class/input/" + str + "/device/id/";
		if (!(std::ifstream(prefix +"vendor") >> vid_str)) {
		      fprintf(stderr, "not found /device/id/vendor\n");
		      return -1;
		}
		if (!(std::ifstream(prefix + "product") >> pid_str)) {
		      fprintf(stderr, "not found /device/id/product\n");
		      return -1;
		}
			
		std::istringstream(vid_str) >> std::hex >> vid;
		std::istringstream(pid_str) >> std::hex >> pid;
	}

	return 0;
}

} // namespace

int test_get_usb_camera_vid_pid()
{
	// get usb video device list
	std::map<std::string, std::string> device_list;
	if (test_v4l2_get_device_list(device_list) !=0) {
		fprintf(stderr, "fail to get usb video device list\n");
		return -1;
	}

	int count = 1;
	fprintf(stdout, "device count: %d\n", device_list.size());
	for (auto it = device_list.cbegin(); it != device_list.cend(); ++it) {
		fprintf(stdout, "%d. device address: %s, description(name): %s\n", count++,  (*it).first.c_str(), (*it).second.c_str());

		unsigned int vid_value, pid_value;
		fprintf(stdout, " method1. get vid and pid through v4l2:\n");
		std::string str = (*it).second;
		auto pos = str.find(":");
		if (pos != std::string::npos) {
			std::string vid_str = str.substr(pos-4, 4);
			std::string pid_str = str.substr(pos+1, 4);
			std::istringstream(vid_str) >> std::hex >> vid_value;
			std::istringstream(pid_str) >> std::hex >> pid_value;
			fprintf(stdout, "  vid: str: %s, value: %d; pid: str: %s, value: %d\n", vid_str.c_str(), vid_value, pid_str.c_str(), pid_value);
		} else {
			fprintf(stderr, "  fail to get vid and pid\n");	
		}

		fprintf(stdout, " method2. get vid and pid through device/modalias:\n");
		str = (*it).first;
		pos = str.find_last_of("/");
		if (pos == std::string::npos) {
			fprintf(stderr, "  fail to get vid and pid\n");
		}
		std::string name = str.substr(pos+1);
		std::string modalias;
		vid_value = 0; pid_value = 0;
		if (!(std::ifstream("/sys/class/video4linux/" + name + "/device/modalias") >> modalias))
			fprintf(stderr, "  fail to read modalias\n");
		if (modalias.size() < 14 || modalias.substr(0,5) != "usb:v" || modalias[9] != 'p')
			fprintf(stderr, "  not a usb format modalias\n");
		if (!(std::istringstream(modalias.substr(5,4)) >> std::hex >> vid_value))
			fprintf(stderr, "  fail to read vid\n");
		if (!(std::istringstream(modalias.substr(10,4)) >> std::hex >> pid_value))
			fprintf(stderr, "  fail to read pid\n");
		fprintf(stdout, "  vid value: %d, pid value: %d\n", vid_value, pid_value);

		fprintf(stdout, " method3. get vid and pid through /proc/bus/input/devices:\n");
		vid_value = 0; pid_value = 0;
		parse_input_devices((*it).second, vid_value, pid_value);
		fprintf(stdout, "  vid value: %d, pid value: %d\n", vid_value, pid_value);

		fprintf(stderr, " method4. get vid and pid through /sys/class/input/eventXXX:\n");
		vid_value = 0; pid_value = 0;
		parse_input_devices2((*it).second, vid_value, pid_value);
		fprintf(stdout, "  vid value: %d, pid value: %d\n", vid_value, pid_value);
	}	

	return  0;
}

int test_get_windows_camera_list()
{
	fprintf(stderr, "Error: it only supports windows platform\n");
	return -1;
}
#else
// Blog: https://blog.csdn.net/fengbingchun/article/details/105248231
namespace {

int get_all_usb_devices_vid_pid()
{
	GUID *guidDev = (GUID*)&GUID_DEVCLASS_USB;
	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(guidDev, NULL, NULL, DIGCF_PRESENT | DIGCF_PROFILE);

	CHAR buffer[4000];
	DWORD memberIndex = 0;

	while (true) {
		SP_DEVINFO_DATA deviceInfoData;
		ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (SetupDiEnumDeviceInfo(deviceInfoSet, memberIndex, &deviceInfoData) == FALSE) {
			if (GetLastError() == ERROR_NO_MORE_ITEMS) {
				break;
			}
		}

		DWORD nSize = 0;
		SetupDiGetDeviceInstanceId(deviceInfoSet, &deviceInfoData, buffer, sizeof(buffer), &nSize);
		buffer[nSize] = '\0';
		fprintf(stdout, "%s\n", buffer);

		memberIndex++;
	}

	if (deviceInfoSet) {
		SetupDiDestroyDeviceInfoList(deviceInfoSet);
	}

	return 0;
}

std::unique_ptr<char[]> dup_wchar_to_utf8(wchar_t *w, int& len)
{
	len = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	std::unique_ptr<char[]> s(new char[len]);
	WideCharToMultiByte(CP_UTF8, 0, w, -1, s.get(), len, 0, 0);
	return s;
}

int parse_string(const std::string& src, std::string& vid, std::string& pid)
{
	int pos = src.find("vid_");
	if (pos == std::string::npos) return -1;
	vid.assign(src, pos + 4, 4);

	pos = src.find("pid_");
	if (pos == std::string::npos) return -1;
	pid.assign(src, pos + 4, 4);

	return 0;
}

int get_usb_video_devices_vid_pid()
{
	CoInitialize(0);

	ICreateDevEnum *devenum = NULL;
	int r = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&devenum));
	if (r != S_OK) {
		fprintf(stdout, "fail to CoCreateInstance: %d\n", r);
		return -1;
	}

	IEnumMoniker *classenum = NULL;
	r = ICreateDevEnum_CreateClassEnumerator(devenum, CLSID_VideoInputDeviceCategory, (IEnumMoniker **)&classenum, 0);
	if (r != S_OK) {
		fprintf(stdout, "fail to ICreateDevEnum_CreateClassEnumerator: %d\n", r);
		return -1;
	}

	IBaseFilter *device_filter = NULL;
	IMoniker *m = NULL;

	while (!device_filter && IEnumMoniker_Next(classenum, 1, &m, NULL) == S_OK) {
		IPropertyBag *bag = NULL;
		VARIANT var;
		r = IMoniker_BindToStorage(m, 0, 0, IID_IPropertyBag, (void **)&bag);
		if (r != S_OK) {
			fprintf(stdout, "fail to IMoniker_BindToStorage: %d\n", r);
			return -1;
		}

		var.vt = VT_BSTR;
		r = IPropertyBag_Read(bag, L"FriendlyName", &var, NULL);
		if (r != S_OK) {
			fprintf(stdout, "fail to IPropertyBag_Read: %d\n", r);
			return -1;
		}

		int length;
		auto friendly_name = dup_wchar_to_utf8(var.bstrVal, length);
		fprintf(stdout, "friendly_name: %s\n", friendly_name.get());

		LPMALLOC co_malloc = NULL;
		r = CoGetMalloc(1, &co_malloc);
		if (r != S_OK) {
			fprintf(stdout, "fail to CoGetMalloc: %d\n", r);
			return -1;
		}

		IBindCtx *bind_ctx = NULL;
		r = CreateBindCtx(0, &bind_ctx);
		if (r != S_OK) {
			fprintf(stdout, "fail to CreateBindCtx: %d\n", r);
			return -1;
		}

		LPOLESTR olestr = NULL;
		r = IMoniker_GetDisplayName(m, bind_ctx, NULL, &olestr);
		if (r != S_OK) {
			fprintf(stdout, "fail to IMoniker_GetDisplayName: %d\n", r);
			return -1;
		}

		auto unique_name = dup_wchar_to_utf8(olestr, length);
		/* replace ':' with '_' since we use : to delineate between sources */
		std::for_each(unique_name.get(), unique_name.get() + length, [](char& ch) { if (ch == ':') ch = '_'; });
		fprintf(stdout, "unique_name: %s\n", unique_name.get());
		std::string src(unique_name.get()), vid, pid;
		parse_string(src, vid, pid);
		unsigned int value_vid, value_pid;
		std::istringstream(vid) >> std::hex >> value_vid;
		std::istringstream(pid) >> std::hex >> value_pid;
		fprintf(stdout, "usb device name: %s, vid: %s, value: %d; pid: %s, value: %d\n",
			friendly_name.get(), vid.c_str(), value_vid, pid.c_str(), value_pid);

		if (olestr && co_malloc)
			IMalloc_Free(co_malloc, olestr);
		if (bind_ctx)
			IBindCtx_Release(bind_ctx);
		if (bag)
			IPropertyBag_Release(bag);

		IMoniker_Release(m);
	}

	IEnumMoniker_Release(classenum);

	return 0;
}

} // namespace

int test_get_usb_camera_vid_pid()
{
	//return get_all_usb_devices_vid_pid();
	return get_usb_video_devices_vid_pid();
}

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/105674068
int test_get_windows_camera_list()
{
	CoInitialize(nullptr);

	ICreateDevEnum *devenum = nullptr;
	int r = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&devenum));
	if (r != S_OK) {
		fprintf(stdout, "fail to CoCreateInstance: %d\n", r);
		return -1;
	}

	IEnumMoniker *classenum = nullptr;
	r = ICreateDevEnum_CreateClassEnumerator(devenum, CLSID_VideoInputDeviceCategory, (IEnumMoniker **)&classenum, 0);
	if (r != S_OK) {
		fprintf(stdout, "fail to ICreateDevEnum_CreateClassEnumerator: %d\n", r);
		return -1;
	}

	IMoniker *m = nullptr;
	typedef struct devices_info {
		int index;
		std::string name;
	} devices_info;
	std::vector<devices_info> lists;
	int device_counter = 0;

	while (IEnumMoniker_Next(classenum, 1, &m, nullptr) == S_OK) {
		IPropertyBag *bag = nullptr;
		VARIANT var;
		r = IMoniker_BindToStorage(m, 0, 0, IID_IPropertyBag, (void **)&bag);
		if (r != S_OK) {
			fprintf(stdout, "fail to IMoniker_BindToStorage: %d\n", r);
			return -1;
		}

		var.vt = VT_BSTR;
		r = IPropertyBag_Read(bag, L"FriendlyName", &var, nullptr);
		if (r != S_OK) {
			fprintf(stdout, "fail to IPropertyBag_Read: %d\n", r);
			return -1;
		}

		int length;
		auto friendly_name = dup_wchar_to_utf8(var.bstrVal, length);
		lists.push_back({ device_counter++, friendly_name.get() });

		if (bag)
			IPropertyBag_Release(bag);

		IMoniker_Release(m);
	}

	IEnumMoniker_Release(classenum);
	CoUninitialize();

	fprintf(stdout, "device lists:\n");
	std::for_each(lists.cbegin(), lists.cend(), [](const devices_info& info) {
		fprintf(stdout, "  index: %d, name: %s\n", info.index, info.name.c_str());
	});

	return 0;
}

#endif

