// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

// reference: modules/core/src/alloc.cpp

#include <malloc.h>
#include <stdio.h>
#include "core/fbcstd.hpp"
#include "core/interface.hpp"
#include "core/utility.hpp"
#include "core/base.hpp"

namespace fbc {

// Allocates an aligned memory buffer
void* fastMalloc(size_t size)
{
	uchar* udata = (uchar*)malloc(size + sizeof(void*) + FBC_MALLOC_ALIGN);
	if (!udata) {
		fprintf(stderr, "failed to allocate %lu bytes\n", (unsigned long)size);
		return NULL;
	}
	uchar** adata = alignPtr((uchar**)udata + 1, FBC_MALLOC_ALIGN);
	adata[-1] = udata;
	return adata;
}

// Deallocates a memory buffer
void fastFree(void* ptr)
{
	if (ptr) {
		uchar* udata = ((uchar**)ptr)[-1];
		FBC_Assert(udata < (uchar*)ptr && ((uchar*)ptr - udata) <= (ptrdiff_t)(sizeof(void*) + FBC_MALLOC_ALIGN));
		free(udata);
	}
}

} // namespace fbc
