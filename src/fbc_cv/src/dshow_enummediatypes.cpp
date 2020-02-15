// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "dshow_capture.hpp"
#include "avmem.hpp"

// reference: ffmpeg 4.2
//            libavdevice/dshow_enummediatypes.c

#ifdef _MSC_VER

namespace fbc {

DECLARE_QUERYINTERFACE(libAVEnumMediaTypes,
	{ { &IID_IUnknown, 0 }, { &IID_IEnumMediaTypes, 0 } })
DECLARE_ADDREF(libAVEnumMediaTypes)
DECLARE_RELEASE(libAVEnumMediaTypes)

long WINAPI libAVEnumMediaTypes_Next(libAVEnumMediaTypes *thisx, unsigned long n,
	AM_MEDIA_TYPE **types, unsigned long *fetched)
{
	int count = 0;
	dshowdebug("libAVEnumMediaTypes_Next(%p)\n", thisx);
	if (!types)
		return E_POINTER;
	if (!thisx->pos && n == 1) {
		if (!IsEqualGUID(thisx->type.majortype, GUID_NULL)) {
			AM_MEDIA_TYPE *type = static_cast<AM_MEDIA_TYPE *>(av_malloc(sizeof(AM_MEDIA_TYPE)));
			if (!type)
				return E_OUTOFMEMORY;
			ff_copy_dshow_media_type(type, &thisx->type);
			*types = type;
			count = 1;
		}
		thisx->pos = 1;
	}
	if (fetched)
		*fetched = count;
	if (!count)
		return S_FALSE;
	return S_OK;
}

long WINAPI libAVEnumMediaTypes_Skip(libAVEnumMediaTypes *thisx, unsigned long n)
{
	dshowdebug("libAVEnumMediaTypes_Skip(%p)\n", thisx);
	if (n) /* Any skip will always fall outside of the only valid type. */
		return S_FALSE;
	return S_OK;
}

long WINAPI libAVEnumMediaTypes_Reset(libAVEnumMediaTypes *thisx)
{
	dshowdebug("libAVEnumMediaTypes_Reset(%p)\n", thisx);
	thisx->pos = 0;
	return S_OK;
}

long WINAPI libAVEnumMediaTypes_Clone(libAVEnumMediaTypes *thisx, libAVEnumMediaTypes **enums)
{
	libAVEnumMediaTypes *newx;
	dshowdebug("libAVEnumMediaTypes_Clone(%p)\n", thisx);
	if (!enums)
		return E_POINTER;
	newx = libAVEnumMediaTypes_Create(&thisx->type);
	if (!newx)
		return E_OUTOFMEMORY;
	newx->pos = thisx->pos;
	*enums = newx;
	return S_OK;
}

typedef HRESULT(STDMETHODCALLTYPE *QueryInterfacexx)(IEnumMediaTypes * This, REFIID riid, void **ppvObject);
typedef ULONG(STDMETHODCALLTYPE *AddRefxx)(IEnumMediaTypes * This);
typedef ULONG(STDMETHODCALLTYPE *Releasexx)(IEnumMediaTypes * This);
typedef HRESULT(STDMETHODCALLTYPE *Nextxx)(IEnumMediaTypes * This, ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched);
typedef HRESULT(STDMETHODCALLTYPE *Skipxx)(IEnumMediaTypes * This, ULONG cMediaTypes);
typedef HRESULT(STDMETHODCALLTYPE *Resetxx)(IEnumMediaTypes * This);
typedef HRESULT(STDMETHODCALLTYPE *Clonexx)(IEnumMediaTypes * This, IEnumMediaTypes **ppEnum);

static int libAVEnumMediaTypes_Setup(libAVEnumMediaTypes *thisx, const AM_MEDIA_TYPE *type)
{
	IEnumMediaTypesVtbl *vtbl = thisx->vtbl;
	vtbl->QueryInterface = reinterpret_cast<QueryInterfacexx>(libAVEnumMediaTypes_QueryInterface);
	vtbl->AddRef = reinterpret_cast<AddRefxx>(libAVEnumMediaTypes_AddRef);
	vtbl->Release = reinterpret_cast<Releasexx>(libAVEnumMediaTypes_Release);
	vtbl->Next = reinterpret_cast<Nextxx>(libAVEnumMediaTypes_Next);
	vtbl->Skip = reinterpret_cast<Skipxx>(libAVEnumMediaTypes_Skip);
	vtbl->Reset = reinterpret_cast<Resetxx>(libAVEnumMediaTypes_Reset);
	vtbl->Clone = reinterpret_cast<Clonexx>(libAVEnumMediaTypes_Clone);

	if (!type) {
		thisx->type.majortype = GUID_NULL;
	}
	else {
		ff_copy_dshow_media_type(&thisx->type, type);
	}

	return 1;
}

DECLARE_AVENUMMEDIATYPES_CREATE(libAVEnumMediaTypes, libAVEnumMediaTypes_Setup(thisx, type), const AM_MEDIA_TYPE *type)
DECLARE_DESTROY(libAVEnumMediaTypes, nothing)

} // namespace fbc

#endif // _MSC_VER
