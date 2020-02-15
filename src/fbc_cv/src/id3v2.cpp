// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "id3v2.hpp"
#include <string.h>
#include <string>
#include "avmem.hpp"
#include "avformat.hpp"

// reference: ffmpeg 4.2
//            libavformat/id3v2.c

#ifdef _MSC_VER

namespace fbc {

typedef struct ID3v2EMFunc {
	const char *tag3;
	const char *tag4;
	void(*read)(AVFormatContext *s, AVIOContext *pb, int taglen,
		const char *tag, ID3v2ExtraMeta **extra_meta,
		int isv34);
	void(*free)(void *obj);
} ID3v2EMFunc;

void ff_id3v2_free_extra_meta(ID3v2ExtraMeta **extra_meta)
{
	ID3v2ExtraMeta *current = *extra_meta, *next;
	const ID3v2EMFunc *extra_func;

	if (current != NULL)
		fprintf(stderr, "Error, current's value should be NULL\n");
	//while (current) {
	//	if ((extra_func = get_extra_meta_func(current->tag, 1)))
	//		extra_func->free(current->data);
	//	next = current->next;
	//	av_freep(&current);
	//	current = next;
	//}

	*extra_meta = NULL;
}

int ff_id3v2_match(const uint8_t *buf, const char *magic)
{
	return  buf[0] == magic[0] &&
		buf[1] == magic[1] &&
		buf[2] == magic[2] &&
		buf[3] != 0xff &&
		buf[4] != 0xff &&
		(buf[6] & 0x80) == 0 &&
		(buf[7] & 0x80) == 0 &&
		(buf[8] & 0x80) == 0 &&
		(buf[9] & 0x80) == 0;
}

int ff_id3v2_tag_len(const uint8_t *buf)
{
	int len = ((buf[6] & 0x7f) << 21) +
		((buf[7] & 0x7f) << 14) +
		((buf[8] & 0x7f) << 7) +
		(buf[9] & 0x7f) +
		ID3v2_HEADER_SIZE;
	if (buf[5] & 0x10)
		len += ID3v2_HEADER_SIZE;
	return len;
}

} // namespace fbc

#endif // _MSC_VER
