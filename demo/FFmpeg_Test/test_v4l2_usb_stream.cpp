#include "funset.hpp"
#include <string.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <set>

#ifndef _MSC_VER

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <dirent.h>

#include "v4l2_common.hpp"

// Blog: https://blog.csdn.net/fengbingchun/article/details/103101344
namespace {

//static const __u32 v4l2_pixel_format_map[4] = {875967048, 0, 1196444237, 1448695129};
static const __u32 v4l2_pixel_format_map[] = {V4L2_PIX_FMT_H264, 0, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV};

int v4l2_is_v4l_dev(const char *name)
{
	return !strncmp(name, "video", 5) ||
		!strncmp(name, "radio", 5) ||
		!strncmp(name, "vbi", 3) ||
		!strncmp(name, "v4l-subdev", 10);
}

int device_open(const char* device_path)
{
	int fd = open(device_path, O_RDWR, 0);
	if (fd < 0) {
		fprintf(stderr, "Error: cannot open video device %s\n", device_path);
		goto fail;
	}

	struct v4l2_capability cap;
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		fprintf(stderr, "Error: cam_info: can't open device: %s\n", device_path);
		goto fail;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "Error: Not a video capture device\n");
		goto fail;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        	fprintf(stderr, "Error: The device does not support the streaming I/O method.\n");
		goto fail;
	}

	return fd;

fail:
	close(fd);
	return -1;
}

const struct fmt_map ff_fmt_conversion_table[] = {
	//ff_fmt              codec_id              v4l2_fmt
	{ AV_PIX_FMT_YUV420P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV420  },
	{ AV_PIX_FMT_YUV420P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YVU420  },
	{ AV_PIX_FMT_YUV422P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV422P },
	{ AV_PIX_FMT_YUYV422, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUYV    },
	{ AV_PIX_FMT_UYVY422, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_UYVY    },
	{ AV_PIX_FMT_YUV411P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV411P },
	{ AV_PIX_FMT_YUV410P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YUV410  },
	{ AV_PIX_FMT_YUV410P, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_YVU410  },
	{ AV_PIX_FMT_RGB555LE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB555  },
	{ AV_PIX_FMT_RGB555BE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB555X },
	{ AV_PIX_FMT_RGB565LE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB565  },
	{ AV_PIX_FMT_RGB565BE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB565X },
	{ AV_PIX_FMT_BGR24,   AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_BGR24   },
	{ AV_PIX_FMT_RGB24,   AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB24   },
#ifdef V4L2_PIX_FMT_XBGR32
	{ AV_PIX_FMT_BGR0,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_XBGR32  },
	{ AV_PIX_FMT_0RGB,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_XRGB32  },
	{ AV_PIX_FMT_BGRA,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_ABGR32  },
	{ AV_PIX_FMT_ARGB,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_ARGB32  },
#endif
	{ AV_PIX_FMT_BGR0,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_BGR32   },
	{ AV_PIX_FMT_0RGB,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_RGB32   },
	{ AV_PIX_FMT_GRAY8,   AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_GREY    },
#ifdef V4L2_PIX_FMT_Y16
	{ AV_PIX_FMT_GRAY16LE,AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_Y16     },
#endif
	{ AV_PIX_FMT_NV12,    AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_NV12    },
	{ AV_PIX_FMT_NONE,    AV_CODEC_ID_MJPEG,    V4L2_PIX_FMT_MJPEG   },
	{ AV_PIX_FMT_NONE,    AV_CODEC_ID_MJPEG,    V4L2_PIX_FMT_JPEG    },
#ifdef V4L2_PIX_FMT_H264
	{ AV_PIX_FMT_NONE,    AV_CODEC_ID_H264,     V4L2_PIX_FMT_H264    },
#endif
#ifdef V4L2_PIX_FMT_MPEG4
	{ AV_PIX_FMT_NONE,    AV_CODEC_ID_MPEG4,    V4L2_PIX_FMT_MPEG4   },
#endif
#ifdef V4L2_PIX_FMT_CPIA1
	{ AV_PIX_FMT_NONE,    AV_CODEC_ID_CPIA,     V4L2_PIX_FMT_CPIA1   },
#endif
#ifdef V4L2_PIX_FMT_SRGGB8
	{ AV_PIX_FMT_BAYER_BGGR8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SBGGR8 },
	{ AV_PIX_FMT_BAYER_GBRG8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SGBRG8 },
	{ AV_PIX_FMT_BAYER_GRBG8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SGRBG8 },
	{ AV_PIX_FMT_BAYER_RGGB8, AV_CODEC_ID_RAWVIDEO, V4L2_PIX_FMT_SRGGB8 },
#endif
	{ AV_PIX_FMT_NONE,    AV_CODEC_ID_NONE,     0                    },
};

enum AVCodecID ff_fmt_v4l2codec(uint32_t v4l2_fmt)
{
	for (int i = 0; ff_fmt_conversion_table[i].codec_id != AV_CODEC_ID_NONE; i++) {
		if (ff_fmt_conversion_table[i].v4l2_fmt == v4l2_fmt) {
			return ff_fmt_conversion_table[i].codec_id;
		}
	}

	return AV_CODEC_ID_NONE;
}

enum AVPixelFormat ff_fmt_v4l2ff(uint32_t v4l2_fmt, enum AVCodecID codec_id)
{
	for (int i = 0; ff_fmt_conversion_table[i].codec_id != AV_CODEC_ID_NONE; i++) {
		if (ff_fmt_conversion_table[i].v4l2_fmt == v4l2_fmt &&
			ff_fmt_conversion_table[i].codec_id == codec_id) {
			return ff_fmt_conversion_table[i].ff_fmt;
		}
	}

	return AV_PIX_FMT_NONE;
}

}; // namespace

int test_v4l2_get_device_list(std::map<std::string, std::string>& device_list)
{
	device_list.clear();

	const char* dir_name = "/dev";
	DIR* dir = opendir(dir_name);
	if (!dir) {
		fprintf(stderr, "Error: couldn't open the directory: %s\n", dir_name);
		return -1;
	}

	struct dirent* entry = nullptr;
	int fd;

	while ((entry = readdir(dir))) {
		char device_name[256];
		if (!v4l2_is_v4l_dev(entry->d_name))
			continue;
		
		snprintf(device_name, sizeof(device_name), "/dev/%s", entry->d_name);
		if ((fd = device_open(device_name)) < 0)
			continue;

		struct v4l2_capability cap;
		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
			fprintf(stderr, "Error: cam_info: can't open device: %s\n", device_name);
			goto fail;
		}

		device_list[device_name] = reinterpret_cast<char*>(cap.card);

		close(fd);
		continue;

		fail:
			if (fd >= 0) close(fd);
			break;
	}

	closedir(dir);
	return 0;
}

int test_v4l2_get_codec_type_list(const std::string& device_name, std::vector<int>& codec_list)
{
	codec_list.clear();

	int fd = device_open(device_name.c_str());
	if (fd < 0) {
		fprintf(stderr, "Error: fail to open device: %s\n", device_name.c_str());
		return -1;
	}

	struct v4l2_capability cap;
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		fprintf(stderr, "Error: cam_info: can't open device: %s\n", device_name.c_str());
		return -1;
	}

	struct v4l2_fmtdesc vfd;
	vfd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vfd.index = 0;
	
	while(!ioctl(fd, VIDIOC_ENUM_FMT, &vfd)) {
		enum AVCodecID codec_id = ff_fmt_v4l2codec(vfd.pixelformat);
		enum AVPixelFormat pix_fmt = ff_fmt_v4l2ff(vfd.pixelformat, codec_id);

		vfd.index++;

		if (!(vfd.flags & V4L2_FMT_FLAG_COMPRESSED)) {
			if (pix_fmt != AV_PIX_FMT_NONE)
				codec_list.emplace_back(VIDEO_CODEC_TYPE_RAWVIDEO);
		} else if (vfd.flags & V4L2_FMT_FLAG_COMPRESSED) {
			if (codec_id == AV_CODEC_ID_MJPEG)
				codec_list.emplace_back(VIDEO_CODEC_TYPE_MJPEG);
			else if (codec_id == AV_CODEC_ID_H264)
				codec_list.emplace_back(VIDEO_CODEC_TYPE_H264);
			else if (codec_id == AV_CODEC_ID_H265)
				codec_list.emplace_back(VIDEO_CODEC_TYPE_H265);
			else
				fprintf(stdout, "WARNING: support codec type: %d\n", codec_id);
		}
	}

	std::sort(codec_list.begin(), codec_list.end());
	close(fd);
	return 0;
}

int test_v4l2_get_video_size_list(const std::string& device_name, int codec_type, std::vector<std::string>& size_list)
{
	size_list.clear();
	if (codec_type < 0 || codec_type > 3) return -1;

	int fd = device_open(device_name.c_str());
	if (fd < 0) {
		fprintf(stderr, "Error: fail to open device: %s\n", device_name.c_str());
		return -1;
	}

	struct v4l2_capability cap;
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		fprintf(stderr, "Error: cam_info: can't open device: %s\n", device_name.c_str());
		return -1;
	}

	struct v4l2_frmsizeenum vfse;
	vfse.pixel_format = v4l2_pixel_format_map[codec_type];
	vfse.index = 0;

	std::set<std::vector<unsigned int>> list;
	while(!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &vfse)) {
		switch (vfse.type) {
			case V4L2_FRMSIZE_TYPE_DISCRETE:
			list.insert({vfse.discrete.width, vfse.discrete.height});
			break;
		}
		vfse.index++;
   	}

	for (auto it = list.cbegin(); it != list.cend(); ++it) {
		std::string str = std::to_string((*it)[0]);
		str +="x";
		str += std::to_string((*it)[1]);
		size_list.emplace_back(str);
	}

	close(fd);
	return 0;
}

int test_v4l2_get_video_device_info()
{
	std::map<std::string, std::string> device_list;
	test_v4l2_get_device_list(device_list);
	fprintf(stdout, "device count: %d\n", device_list.size());
	for (auto it = device_list.cbegin(); it != device_list.cend(); ++it) {
		fprintf(stdout, "device name: %s, description: %s\n", (*it).first.c_str(), (*it).second.c_str());

		std::vector<int> codec_list;
		test_v4l2_get_codec_type_list((*it).first, codec_list);
		for (auto it2 = codec_list.cbegin(); it2 != codec_list.cend(); ++it2) {
			fprintf(stdout, " support codec type(0: h264; 1: h265; 2: mjpeg; 3: rawvideo):%d\n", (*it2));

			std::vector<std::string> size_list;
			test_v4l2_get_video_size_list((*it).first, (*it2), size_list);
			fprintf(stdout, "  support video size(width*height):\n");
			for (auto it3 = size_list.cbegin(); it3 != size_list.cend(); ++it3) {
				fprintf(stdout, "   %s\n", (*it3).c_str());
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/95984569
namespace {

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

struct buffer {
	void *start;
	size_t length;
};

char* dev_name;
enum io_method io = IO_METHOD_MMAP;
int fd = -1;
struct buffer* buffers;
unsigned int n_buffers;
int out_buf;
int force_format = 1;
int frame_count = 10;
int width = 640;
int height = 480;
FILE* f;

void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

void process_image(const void *p, int size)
{
	/*if (out_buf)
		fwrite(p, size, 1, stdout);

	fflush(stderr);
	fprintf(stderr, ".");
	fflush(stdout);*/
	fwrite(p, size, 1, f);
}

int read_frame(void)
{
	struct v4l2_buffer buf;
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				// Could ignore EIO, see spec. fall through

			default:
				errno_exit("read");
			}
		}

		process_image(buffers[0].start, buffers[0].length);
		break;

	case IO_METHOD_MMAP:
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				// Could ignore EIO, see spec. fall through

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		assert(buf.index < n_buffers);
		process_image(buffers[buf.index].start, buf.bytesused);
		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		break;

	case IO_METHOD_USERPTR:
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				// Could ignore EIO, see spec. fall through

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < n_buffers; ++i)
		if (buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
			break;

		assert(i < n_buffers);
		process_image((void *)buf.m.userptr, buf.bytesused);
		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
		break;
	}

	return 1;
}

void mainloop(void)
{
	unsigned int count = frame_count;
	while (count-- > 0) {
		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			// Timeout
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(fd + 1, &fds, NULL, NULL, &tv);
			if (-1 == r) {
				if (EINTR == errno)
					continue;
				errno_exit("select");
			}

			if (0 == r) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}

			if (read_frame())
				break;
			// EAGAIN - continue select loop
		}
	}
}

void stop_capturing(void)
{
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		// Nothing to do
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");
		break;
	}
}

void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		// Nothing to do
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long)buffers[i].start;
			buf.length = buffers[i].length;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");
		break;
	}

	f = fopen("usb.yuv", "w");
	if (!f) {
		errno_exit("fail to open file");
	}
}

void uninit_device(void)
{
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		free(buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
		if (-1 == munmap(buffers[i].start, buffers[i].length))
			errno_exit("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			free(buffers[i].start);
		break;
	}

	free(buffers);
	fclose(f);
}

void init_read(unsigned int buffer_size)
{
	buffers = (struct buffer*)calloc(1, sizeof(*buffers));
	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

void init_mmap(void)
{
	struct v4l2_requestbuffers req;
	CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mappingn\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));
	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
			mmap(NULL, // start anywhere
			buf.length,
			PROT_READ | PROT_WRITE, // required
			MAP_SHARED, // recommended
			fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}

void init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;
	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support user pointer i/on", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	buffers = (struct buffer*)calloc(4, sizeof(*buffers));
	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = malloc(buffer_size);

		if (!buffers[n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}

void init_device(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}
		break;
	}

	// Select video input, video standard and tune here
	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; // reset to default
		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
				// Cropping not supported
				break;
			default:
				// Errors ignored
				break;
			}
		}
	} else {
		// Errors ignored
	}

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (force_format) {
		fmt.fmt.pix.width = width;
		fmt.fmt.pix.height = height;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

		// Note VIDIOC_S_FMT may change width and height
		if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
			errno_exit("VIDIOC_S_FMT");
	} else {
		// Preserve original settings as set by v4l2-ctl for example
		if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
			errno_exit("VIDIOC_G_FMT");
	}

	// Buggy driver paranoia
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
		init_read(fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap();
		break;

	case IO_METHOD_USERPTR:
		init_userp(fmt.fmt.pix.sizeimage);
		break;
	}
}

void close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");

	fd = -1;
}

void open_device(void)
{
	struct stat st;
	if (-1 == stat(dev_name, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no devicen\n", dev_name);
		exit(EXIT_FAILURE);
	}

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0); // O_RDWR: required
	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

} // namespace

int test_v4l2_usb_stream()
{
	// reference: https://linuxtv.org/downloads/v4l-dvb-apis-new/uapi/v4l/capture.c.html
	dev_name = "/dev/video0";

	open_device();
	init_device();
	start_capturing();
	mainloop();
	stop_capturing();
	uninit_device();
	close_device();

	fprintf(stdout, "test finish\n");
	return 0;
}

#else

int test_v4l2_usb_stream()
{
	fprintf(stderr, "Error: this test code only support linux platform\n");
	return -1;
}

int test_v4l2_get_video_device_info()
{
	fprintf(stderr, "Error: this test code only support linux platform\n");
	return -1;
}

#endif
