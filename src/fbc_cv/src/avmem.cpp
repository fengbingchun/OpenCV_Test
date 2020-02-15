// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <cstddef>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include "avmem.hpp"

// reference: ffmpeg 4.2
//            libavutil/mem.c

#ifdef _MSC_VER

namespace fbc {

static size_t max_alloc_size = INT_MAX;
#define ALIGN (HAVE_AVX512 ? 64 : (HAVE_AVX ? 32 : 16))

char *av_strdup(const char *s)
{
	char *ptr = NULL;
	if (s) {
		size_t len = strlen(s) + 1;
		ptr = static_cast<char*>(av_realloc(NULL, len));
		if (ptr)
			memcpy(ptr, s, len);
	}
	return ptr;
}

void *av_realloc(void *ptr, size_t size)
{
	/* let's disallow possibly ambiguous cases */
	if (size > (max_alloc_size - 32))
		return NULL;

#if HAVE_ALIGNED_MALLOC
	return _aligned_realloc(ptr, size + !size, ALIGN);
#else
	return realloc(ptr, size + !size);
#endif
}

void *av_malloc(size_t size)
{
	void *ptr = NULL;

	/* let's disallow possibly ambiguous cases */
	if (size > (max_alloc_size - 32))
		return NULL;

#if HAVE_POSIX_MEMALIGN
	if (size) //OS X on SDK 10.6 has a broken posix_memalign implementation
	if (posix_memalign(&ptr, ALIGN, size))
		ptr = NULL;
#elif HAVE_ALIGNED_MALLOC
	ptr = _aligned_malloc(size, ALIGN);
#elif HAVE_MEMALIGN
#ifndef __DJGPP__
	ptr = memalign(ALIGN, size);
#else
	ptr = memalign(size, ALIGN);
#endif
#else
	ptr = malloc(size);
#endif
	if (!ptr && !size) {
		size = 1;
		ptr = av_malloc(1);
	}
#if CONFIG_MEMORY_POISONING
	if (ptr)
		memset(ptr, FF_MEMORY_POISON, size);
#endif
	return ptr;
}

void av_free(void *ptr)
{
#if HAVE_ALIGNED_MALLOC
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

void av_freep(void *arg)
{
	void *val;

	memcpy(&val, arg, sizeof(val));
	arg = NULL;
	av_free(val);
}

void *av_realloc_array(void *ptr, size_t nmemb, size_t size)
{
	if (!size || nmemb >= INT_MAX / size)
		return NULL;
	return av_realloc(ptr, nmemb * size);
}

void *av_mallocz(size_t size)
{
	void *ptr = av_malloc(size);
	if (ptr)
		memset(ptr, 0, size);
	return ptr;
}

void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size)
{
	if (min_size <= *size)
		return ptr;

	if (min_size > max_alloc_size - 32) {
		*size = 0;
		return NULL;
	}

	min_size = FFMIN(max_alloc_size - 32, FFMAX(min_size + min_size / 16 + 32, min_size));

	ptr = av_realloc(ptr, min_size);
	/* we could set this to the unmodified min_size but this is safer
	* if the user lost the ptr and uses NULL now
	*/
	if (!ptr)
		min_size = 0;

	*size = min_size;

	return ptr;
}

} // namespace fbc

#endif // _MSC_VER
