// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVMEM_HPP_
#define FBC_CV_AVMEM_HPP_

// reference: ffmpeg 4.2
//            libavutil/mem.h

#ifdef _MSC_VER

#include "ffmpeg_common.hpp"
#include "core/fbcdef.hpp"

namespace fbc {

#define av_malloc_attrib
#define av_alloc_size(...)

char *av_strdup(const char *s) av_malloc_attrib;
void *av_realloc(void *ptr, size_t size) av_alloc_size(2);
FBC_EXPORTS void *av_malloc(size_t size) av_malloc_attrib av_alloc_size(1);
void av_free(void *ptr);
FBC_EXPORTS void av_freep(void *ptr);
void *av_realloc_array(void *ptr, size_t nmemb, size_t size);
void *av_mallocz(size_t size) av_malloc_attrib av_alloc_size(1);
void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVMEM_HPP_
