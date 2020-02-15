// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "dshow_capture.hpp"

// reference: ffmpeg 4.2
//            libavdevice/dshow_enumpins.c

#ifdef _MSC_VER

namespace fbc {

DECLARE_QUERYINTERFACE(libAVEnumPins,
	{ { &IID_IUnknown, 0 }, { &IID_IEnumPins, 0 } })
DECLARE_ADDREF(libAVEnumPins)
DECLARE_RELEASE(libAVEnumPins)

long WINAPI libAVEnumPins_Next(libAVEnumPins *thisx, unsigned long n, IPin **pins,
	unsigned long *fetched)
{
	int count = 0;
	dshowdebug("libAVEnumPins_Next(%p)\n", thisx);
	if (!pins)
		return E_POINTER;
	if (!thisx->pos && n == 1) {
		libAVPin_AddRef(thisx->pin);
		*pins = (IPin *) thisx->pin;
		count = 1;
		thisx->pos = 1;
	}
	if (fetched)
		*fetched = count;
	if (!count)
		return S_FALSE;
	return S_OK;
}

long WINAPI libAVEnumPins_Skip(libAVEnumPins *thisx, unsigned long n)
{
	dshowdebug("libAVEnumPins_Skip(%p)\n", thisx);
	if (n) /* Any skip will always fall outside of the only valid pin. */
		return S_FALSE;
	return S_OK;
}

long WINAPI libAVEnumPins_Reset(libAVEnumPins *thisx)
{
	dshowdebug("libAVEnumPins_Reset(%p)\n", thisx);
	thisx->pos = 0;
	return S_OK;
}

long WINAPI libAVEnumPins_Clone(libAVEnumPins *thisx, libAVEnumPins **pins)
{
	libAVEnumPins *newx;
	dshowdebug("libAVEnumPins_Clone(%p)\n", thisx);
	if (!pins)
		return E_POINTER;
	newx = libAVEnumPins_Create(thisx->pin, thisx->filter);
	if (!newx)
		return E_OUTOFMEMORY;
	newx->pos = thisx->pos;
	*pins = newx;
	return S_OK;
}

typedef HRESULT(STDMETHODCALLTYPE *QueryInterfacexx)(IEnumPins * This, REFIID riid, void **ppvObject);
typedef ULONG(STDMETHODCALLTYPE *AddRefxx)(IEnumPins * This);
typedef ULONG(STDMETHODCALLTYPE *Releasexx)(IEnumPins * This);
typedef HRESULT(STDMETHODCALLTYPE *Nextxx)(IEnumPins * This, ULONG cPins, IPin **ppPins, ULONG *pcFetched);
typedef HRESULT(STDMETHODCALLTYPE *Skipxx)(IEnumPins * This, ULONG cPins);
typedef HRESULT(STDMETHODCALLTYPE *Resetxx)(IEnumPins * This);
typedef HRESULT(STDMETHODCALLTYPE *Clonexx)(IEnumPins * This, IEnumPins **ppEnum);

static int libAVEnumPins_Setup(libAVEnumPins *thisx, libAVPin *pin, libAVFilter *filter)
{
	IEnumPinsVtbl *vtbl = thisx->vtbl;
	vtbl->QueryInterface = reinterpret_cast<QueryInterfacexx>(libAVEnumPins_QueryInterface);
	vtbl->AddRef = reinterpret_cast<AddRefxx>(libAVEnumPins_AddRef);
	vtbl->Release = reinterpret_cast<Releasexx>(libAVEnumPins_Release);
	vtbl->Next = reinterpret_cast<Nextxx>(libAVEnumPins_Next);
	vtbl->Skip = reinterpret_cast<Skipxx>(libAVEnumPins_Skip);
	vtbl->Reset = reinterpret_cast<Resetxx>(libAVEnumPins_Reset);
	vtbl->Clone = reinterpret_cast<Clonexx>(libAVEnumPins_Clone);

	thisx->pin = pin;
	thisx->filter = filter;
	libAVFilter_AddRef(thisx->filter);

	return 1;
}

static int libAVEnumPins_Cleanup(libAVEnumPins *thisx)
{
	libAVFilter_Release(thisx->filter);
	return 1;
}

DECLARE_AVENUMPINS_CREATE(libAVEnumPins, libAVEnumPins_Setup(thisx, pin, filter),
	libAVPin *pin, libAVFilter *filter)
DECLARE_DESTROY(libAVEnumPins, libAVEnumPins_Cleanup)

} // namespace fbc

#endif // _MSC_VER
