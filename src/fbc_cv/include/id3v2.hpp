// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_ID3V2_HPP_
#define FBC_CV_ID3V2_HPP_

// reference: ffmpeg 4.2
//            libavformat/id3v2.h

#ifdef _MSC_VER

#include <stdint.h>

namespace fbc {

#define ID3v2_DEFAULT_MAGIC "ID3"
#define ID3v2_HEADER_SIZE 10

typedef struct ID3v2ExtraMeta {
	const char *tag;
	void *data;
	struct ID3v2ExtraMeta *next;
} ID3v2ExtraMeta;

void ff_id3v2_free_extra_meta(ID3v2ExtraMeta **extra_meta);
int ff_id3v2_match(const uint8_t *buf, const char *magic);
int ff_id3v2_tag_len(const uint8_t *buf);

} // namespae codec

#endif // _MSC_VER
#endif // FBC_CV_ID3V2_HPP_
