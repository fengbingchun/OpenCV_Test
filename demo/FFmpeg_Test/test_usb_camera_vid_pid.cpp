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
#else
int test_get_usb_camera_vid_pid()
{
	fprintf(stderr, "only support linux platform\n");
	return -1;
}
#endif

