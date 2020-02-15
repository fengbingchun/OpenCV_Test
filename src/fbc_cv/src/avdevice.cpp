// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <cstddef>
#include "avdevice.hpp"
#include "avformat.hpp"
#include "averror.hpp"

// reference: ffmpeg 4.2
//            libavdevice/alldevices.c

#ifdef _MSC_VER

namespace fbc {

extern AVInputFormat ff_dshow_demuxer;

static const AVOutputFormat *outdev_list[] = { NULL };
static const AVInputFormat *indev_list[] = { &ff_dshow_demuxer, NULL };

void avdevice_register_all(void)
{
	avpriv_register_devices(outdev_list, indev_list);
}

int avdevice_dev_to_app_control_message(struct AVFormatContext *s, enum AVDevToAppMessageType type,
	void *data, size_t data_size)
{
	if (!s->control_message_cb)
		return AVERROR(ENOSYS);
	return s->control_message_cb(s, type, data, data_size);
}

} // namespace fbc

#endif // _MSC_VER
