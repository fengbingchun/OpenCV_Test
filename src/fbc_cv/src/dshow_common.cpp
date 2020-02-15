// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "dshow_capture.hpp"

// reference: ffmpeg 4.2
//            libavdevice/dshow_common.c

#ifdef _MSC_VER

namespace fbc {

long ff_copy_dshow_media_type(AM_MEDIA_TYPE *dst, const AM_MEDIA_TYPE *src)
{
	uint8_t *pbFormat = NULL;

	if (src->cbFormat) {
		pbFormat = static_cast<uint8_t*>(CoTaskMemAlloc(src->cbFormat));
		if (!pbFormat)
			return E_OUTOFMEMORY;
		memcpy(pbFormat, src->pbFormat, src->cbFormat);
	}

	*dst = *src;
	dst->pUnk = NULL;
	dst->pbFormat = pbFormat;

	return S_OK;
}

void ff_printGUID(const GUID *g)
{
}

void ff_print_VIDEO_STREAM_CONFIG_CAPS(const VIDEO_STREAM_CONFIG_CAPS *caps)
{
}

void ff_print_AUDIO_STREAM_CONFIG_CAPS(const AUDIO_STREAM_CONFIG_CAPS *caps)
{
}

void ff_print_AM_MEDIA_TYPE(const AM_MEDIA_TYPE *type)
{
}

} // namespace fbc

#endif // _MSC_VER
