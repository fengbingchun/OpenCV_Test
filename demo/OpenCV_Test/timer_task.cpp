#include "timer_task.hpp"
#include <vector>
#include <mutex>
#include <algorithm>
#include <string_view>

namespace {
float get_disk_space(std::string_view path)
{
	namespace fs = std::filesystem;
	constexpr float GB{ 1024.0 * 1024 * 1024 };

	try {
		auto space_info = fs::space(path);
		return (space_info.available / GB);
	} catch (const fs::filesystem_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 0.f;
	}
}

void monitor_space(unsigned int gb, std::string_view path, std::atomic<bool>& running)
{
	namespace fs = std::filesystem;
	std::mutex mtx;

	if (!fs::exists(path) || !fs::is_directory(path)) {
		std::lock_guard<std::mutex> lock(mtx);
		std::cerr << "Error: " << path << "is not a directory" << std::endl;
	}

	while (running) {
		try {
			float space = get_disk_space(path);
			//std::cout << "space: " << space << ", path: " << path << std::endl;
			if (space < gb) {
				std::vector<fs::path> names;
				for (const auto& entry : fs::directory_iterator(path)) {
					if (fs::is_directory(entry)) {
						names.push_back(entry.path());
					}
				}

				if (names.size() <= 1) {
					//{
					//	std::lock_guard<std::mutex> lock(mtx);
					//	std::cerr << "Error: requires at least 2 directories to exist: " << names.size() << std::endl;
					//}
					continue;
				}

				std::sort(names.begin(), names.end());

				fs::remove_all(names[0]);
				{
					std::lock_guard<std::mutex> lock(mtx);
					std::cout << "delete dir: " << names[0] << std::endl;
				}
			}
		} catch (const fs::filesystem_error& e) {
			std::lock_guard<std::mutex> lock(mtx);
			std::cerr << "Error: " << e.what() << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

} // namespace

float TimerTask::get_available_space()
{
	return get_disk_space(path_);
}

std::tuple<bool, float> TimerTask::set_minimum_available_space(unsigned int gb)
{
	gb_ = gb;

	auto space = get_available_space();
	if (gb_ > space)
		return std::make_tuple(false, space);
	else
		return std::make_tuple(true, space);
}

std::string TimerTask::get_local_time()
{
	using std::chrono::system_clock;
	auto time = system_clock::to_time_t(system_clock::now());
	std::tm* tm = std::localtime(&time);

	std::stringstream buffer;
	buffer << std::put_time(tm, "%Y%m%d%H%M%S");
	return buffer.str();
}

bool TimerTask::set_save_directory_name(const std::string& dir_name)
{
	namespace fs = std::filesystem;
	dir_name_ = dir_name;

	fs::path path(path_ + "/" + dir_name_);
	if (fs::exists(path))
		return true;
	else {
		try {
			return fs::create_directories(path);
		} catch (const fs::filesystem_error& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			return false;
		}
	}
}

std::tuple<bool, std::string> TimerTask::get_current_directory_name()
{
	namespace fs = std::filesystem;
	auto local_time = get_local_time();
	std::string month(local_time.cbegin(), local_time.cbegin() + 6);
	std::string day(local_time.cbegin(), local_time.cbegin() + 8);
	auto curr_dir_name = path_ + "/" + dir_name_ + "/" + month + "/" + day;

	fs::path path(curr_dir_name);
	if (fs::exists(path))
		return std::make_tuple(true, curr_dir_name);
	else {
		try {
			return std::make_tuple(fs::create_directories(path), curr_dir_name);
		} catch (const fs::filesystem_error& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			return std::make_tuple(false, curr_dir_name);
		}
	}
}

void TimerTask::monitor_disk_space(unsigned int gb)
{
	monitor_thread_ = std::thread(monitor_space, gb, path_ + "/" + dir_name_, std::ref(running_));
}
