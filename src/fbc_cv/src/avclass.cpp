// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "avclass.hpp"

// reference: ffmpeg 4.2
//            libavutil/log.c

#ifdef _MSC_VER

namespace fbc {

const char *av_default_item_name(void *ptr)
{
	return (*(AVClass **)ptr)->class_name;
}

} // namesapce codec

#endif // _MSC_VER
