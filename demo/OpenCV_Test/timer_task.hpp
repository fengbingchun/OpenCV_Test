#pragma once

#include <filesystem>
#include <string>
#include <iostream>
#include <tuple>
#include <atomic>
#include <thread>

class TimerTask {
public:
	TimerTask(const std::string& path):path_(path) {}
	~TimerTask() { release(); }
	void release()
	{
		running_ = false;
		if (monitor_thread_.joinable())
			monitor_thread_.join();
	}

	std::tuple<bool, float> set_minimum_available_space(unsigned int gb);
	bool set_save_directory_name(const std::string& dir_name);

	std::string get_local_time();
	std::tuple<bool, std::string> get_current_directory_name();

	void save_video(unsigned int seconds) { save_video_ = true; seconds_ = seconds; }
	void save_image() { save_video_ = false; }

	void monitor_disk_space(unsigned int gb);

private:
	float get_available_space();

	std::string path_;
	unsigned int gb_{0};
	std::string dir_name_{}; // relative path,used to store videos or images
	bool save_video_{false}; // video or image
	unsigned int seconds_{ 0 };
	std::atomic<bool> running_{ true };
	std::thread monitor_thread_;
}; // class TimerTask
