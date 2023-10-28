#ifndef FBC_FFMPEG_TEST_COMMON_HPP_
#define FBC_FFMPEG_TEST_COMMON_HPP_

#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>

#ifdef __cplusplus
}
#endif

constexpr int VIDEO_CODEC_MAX_STRING_SIZE = 64;

class Timer {
public:
	static long long getNowTime() { // milliseconds
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	}
};

inline void print_error_string(int value)
{
	static char errbuf[AV_ERROR_MAX_STRING_SIZE];
	memset(errbuf, 0, AV_ERROR_MAX_STRING_SIZE);
	fprintf(stderr, "Error occurred: code: %d, string: %s\n", value, av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, value));
}

typedef struct Buffer {
	unsigned char* data;
	unsigned int length;
} Buffer;

class BufferQueue {
public:
	BufferQueue() = default;
	~BufferQueue() {}

	void push(Buffer& buffer) {
		std::unique_lock<std::mutex> lck(mtx);
		queue.push(buffer);
		cv.notify_all();
	}

	void pop(Buffer& buffer) {
		std::unique_lock<std::mutex> lck(mtx);
		while (queue.empty()) {
			cv.wait(lck);
		}
		buffer = queue.front();
		queue.pop();
	}

	size_t size() const {
		return queue.size();
	}

private:
	std::queue<Buffer> queue;
	std::mutex mtx;
	std::condition_variable cv;
};

class PacketScaleQueue {
public:
	PacketScaleQueue() = default;

	~PacketScaleQueue() {
		Buffer buffer;

		while (getPacketSize() > 0) {
			popPacket(buffer);
			delete[] buffer.data;
		}

		while (getScaleSize() > 0) {
			popScale(buffer);
			delete[] buffer.data;
		}
	}

	void init(unsigned int buffer_num = 16, size_t buffer_size = 1024 * 1024 * 4) {
		for (auto i = 0; i < buffer_num; ++i) {
			Buffer buffer = { new unsigned char[buffer_size], buffer_num};
			pushPacket(buffer);
		}
	}

	void pushPacket(Buffer& buffer) { packet_queue.push(buffer); }
	void popPacket(Buffer& buffer) { packet_queue.pop(buffer); }
	size_t getPacketSize() const { return packet_queue.size(); }

	void pushScale(Buffer& buffer) { scale_queue.push(buffer); }
	void popScale(Buffer& buffer) { scale_queue.pop(buffer); }
	size_t getScaleSize() const { return scale_queue.size(); }

private:
	BufferQueue packet_queue, scale_queue;
};

class AVFrameQueue {
public:
	AVFrameQueue() = default;
	~AVFrameQueue() {}

	void push(AVFrame** frame) {
		std::unique_lock<std::mutex> lck(mtx);
		queue.push(*frame);
		cv.notify_all();
	}

	void pop(AVFrame** frame) {
		std::unique_lock<std::mutex> lck(mtx);
		while (queue.empty()) {
			//cv.wait(lck);
			if (cv.wait_for(lck, std::chrono::milliseconds(50)) == std::cv_status::timeout) {
				fprintf(stderr, "#### Warning: wait timeout\n");
				*frame = nullptr;
				return;
			}
		}
		*frame = queue.front();
		queue.pop();
	}

	size_t size() const {
		return queue.size();
	}

private:
	std::queue<AVFrame*> queue;
	std::mutex mtx;
	std::condition_variable cv;
};

class CodecQueue {
public:
	CodecQueue() = default;
	void init(unsigned int frame_num) {
		for (auto i = 0; i < frame_num; ++i) {
			AVFrame* frame = nullptr;
			pushDecode(&frame);
		}
	}

	~CodecQueue() { release(); }

	void release() {
		AVFrame* frame = nullptr;

		while (getDecodeSize() > 0) {
			popDecode(&frame);
			av_frame_free(&frame);
		}

		while (getEncodeSize() > 0) {
			popEncode(&frame);
			av_frame_free(&frame);
		}
	}

	void pushDecode(AVFrame** frame) { decode_queue.push(frame); }
	void popDecode(AVFrame** frame) { decode_queue.pop(frame); }
	size_t getDecodeSize() const { return decode_queue.size(); }

	void pushEncode(AVFrame** frame) { encode_queue.push(frame); }
	void popEncode(AVFrame** frame) { encode_queue.pop(frame); }
	size_t getEncodeSize() const { return encode_queue.size(); }

private:
	AVFrameQueue decode_queue, encode_queue;
};

typedef struct CodecCtx {
	char outfile_name[VIDEO_CODEC_MAX_STRING_SIZE];
	char video_size[VIDEO_CODEC_MAX_STRING_SIZE];
	char bitrate_str[VIDEO_CODEC_MAX_STRING_SIZE];
	char pixel_format[VIDEO_CODEC_MAX_STRING_SIZE];
	char filter_descr[VIDEO_CODEC_MAX_STRING_SIZE];
	AVFormatContext* ifmt_ctx;
	AVFormatContext* ofmt_ctx;
	AVCodecContext* dec_ctx;
	AVCodecContext* enc_ctx;
	AVFrame* dec_frame;
	AVFilterContext* buffersink_ctx;
	AVFilterContext* buffersrc_ctx;
	AVFilterGraph* filter_graph;
	AVPacket* enc_pkt;
	AVRational frame_rate;
	int term_status;
	int stream_index;
	int frame_count;
	bool encode_thread_end;
	bool exit_encode_thread;
} CodecCtx;

class VideoCodec {
public:
	VideoCodec() = default;
	~VideoCodec() {  }

	void setOutfileName(const std::string& name) { outfile_name_ = name; }
	void setVideoSize(const std::string& size) { video_size_ = size; }
	void setPixelFormat(const std::string& format) { pixel_format_ = format; }
	void setFilterDescr(const std::string& filter_descr) { filter_descr_ = filter_descr; }

	void stopEncode() {
		while (raw_packet_queue_.getScaleSize() > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		codec_ctx_->term_status = 1;

		Buffer buffer;
		raw_packet_queue_.popPacket(buffer);
		memset(buffer.data, 0, block_size_);
		raw_packet_queue_.pushScale(buffer); // for av_read_frame to exit normally
	}

	PacketScaleQueue& get_raw_packet_queue(unsigned int buffer_num, size_t buffer_size) {
		raw_packet_queue_.init(buffer_num, buffer_size);
		block_size_ = buffer_size;
		return raw_packet_queue_;
	}

	int openEncode();
	int processEncode();
	int closeEncode();

private:
	std::string outfile_name_ = "";
	std::string video_size_ = "";
	std::string pixel_format_ = "";
	std::string filter_descr_ = "";
	PacketScaleQueue raw_packet_queue_;
	int block_size_ = 0;
	CodecCtx* codec_ctx_ = nullptr;
	AVIOContext* avio_ctx_ = nullptr;
	CodecQueue codec_queue_;
	std::thread encode_thread_;

	int get_decode_context();
	int get_encode_context();
	int init_filters();
	int filter_encode_write_frame(AVFrame* frame);
	int get_output_format_context();

	int flush_encode_write_frame();
	int flush_decoder();
	int flush_encoder();
	void flush_codec();
};

#endif // FBC_FFMPEG_TEST_COMMON_HPP_

