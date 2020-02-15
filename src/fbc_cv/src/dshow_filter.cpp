// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "dshow_capture.hpp"

// reference: ffmpeg 4.2
//            libavdevice/dshow_filter.c

#ifdef _MSC_VER

namespace fbc {

DECLARE_QUERYINTERFACE(libAVFilter,
	{ { &IID_IUnknown, 0 },
	{ &IID_IBaseFilter, 0 } })
DECLARE_ADDREF(libAVFilter)
DECLARE_RELEASE(libAVFilter)

long WINAPI libAVFilter_GetClassID(libAVFilter *thisx, CLSID *id)
{
	dshowdebug("libAVFilter_GetClassID(%p)\n", thisx);
	/* I'm not creating a ClassID just for this. */
	return E_FAIL;
}

long WINAPI libAVFilter_Stop(libAVFilter *thisx)
{
	dshowdebug("libAVFilter_Stop(%p)\n", thisx);
	thisx->state = State_Stopped;
	return S_OK;
}

long WINAPI libAVFilter_Pause(libAVFilter *thisx)
{
	dshowdebug("libAVFilter_Pause(%p)\n", thisx);
	thisx->state = State_Paused;
	return S_OK;
}

long WINAPI libAVFilter_Run(libAVFilter *thisx, REFERENCE_TIME start)
{
	dshowdebug("libAVFilter_Run(%p) %"PRId64"\n", thisx, start);
	thisx->state = State_Running;
	thisx->start_time = start;
	return S_OK;
}

long WINAPI libAVFilter_GetState(libAVFilter *thisx, DWORD ms, FILTER_STATE *state)
{
	dshowdebug("libAVFilter_GetState(%p)\n", thisx);
	if (!state)
		return E_POINTER;
	*state = thisx->state;
	return S_OK;
}

long WINAPI libAVFilter_SetSyncSource(libAVFilter *thisx, IReferenceClock *clock)
{
	dshowdebug("libAVFilter_SetSyncSource(%p)\n", thisx);

	if (thisx->clock != clock) {
		if (thisx->clock)
			IReferenceClock_Release(thisx->clock);
		thisx->clock = clock;
		if (clock)
			IReferenceClock_AddRef(clock);
	}

	return S_OK;
}

long WINAPI libAVFilter_GetSyncSource(libAVFilter *thisx, IReferenceClock **clock)
{
	dshowdebug("libAVFilter_GetSyncSource(%p)\n", thisx);

	if (!clock)
		return E_POINTER;
	if (thisx->clock)
		IReferenceClock_AddRef(thisx->clock);
	*clock = thisx->clock;

	return S_OK;
}

long WINAPI libAVFilter_EnumPins(libAVFilter *thisx, IEnumPins **enumpin)
{
	libAVEnumPins *newx;
	dshowdebug("libAVFilter_EnumPins(%p)\n", thisx);

	if (!enumpin)
		return E_POINTER;
	newx = libAVEnumPins_Create(thisx->pin, thisx);
	if (!newx)
		return E_OUTOFMEMORY;

	*enumpin = (IEnumPins *) newx;
	return S_OK;
}

long WINAPI libAVFilter_FindPin(libAVFilter *thisx, const wchar_t *id, IPin **pin)
{
	libAVPin *found = NULL;
	dshowdebug("libAVFilter_FindPin(%p)\n", this);

	if (!id || !pin)
		return E_POINTER;
	if (!wcscmp(id, L"In")) {
		found = thisx->pin;
		libAVPin_AddRef(found);
	}
	*pin = (IPin *)found;
	if (!found)
		return VFW_E_NOT_FOUND;

	return S_OK;
}

long WINAPI libAVFilter_QueryFilterInfo(libAVFilter *thisx, FILTER_INFO *info)
{
	dshowdebug("libAVFilter_QueryFilterInfo(%p)\n", thisx);

	if (!info)
		return E_POINTER;
	if (thisx->info.pGraph)
		IFilterGraph_AddRef(thisx->info.pGraph);
	*info = thisx->info;

	return S_OK;
}

long WINAPI libAVFilter_JoinFilterGraph(libAVFilter *thisx, IFilterGraph *graph, const wchar_t *name)
{
	dshowdebug("libAVFilter_JoinFilterGraph(%p)\n", thisx);

	thisx->info.pGraph = graph;
	if (name)
		wcscpy(thisx->info.achName, name);

	return S_OK;
}

long WINAPI libAVFilter_QueryVendorInfo(libAVFilter *thisx, wchar_t **info)
{
	dshowdebug("libAVFilter_QueryVendorInfo(%p)\n", this);

	if (!info)
		return E_POINTER;
	return E_NOTIMPL; /* don't have to do anything here */
}

typedef void(*callbackxx)(void *priv_data, int index, uint8_t *buf, int buf_size, int64_t time, enum dshowDeviceType type);
typedef HRESULT(STDMETHODCALLTYPE *QueryInterfacexx)(IBaseFilter * This, REFIID riid, void **ppvObject);
typedef ULONG(STDMETHODCALLTYPE *AddRefxx)(IBaseFilter * This);
typedef ULONG(STDMETHODCALLTYPE *Releasexx)(IBaseFilter * This);
typedef HRESULT(STDMETHODCALLTYPE *GetClassIDxx)(IBaseFilter * This,CLSID *pClassID);
typedef HRESULT(STDMETHODCALLTYPE *Stopxx)(IBaseFilter * This);
typedef HRESULT(STDMETHODCALLTYPE *Pausexx)(IBaseFilter * This);
typedef HRESULT(STDMETHODCALLTYPE *Runxx)(IBaseFilter * This, REFERENCE_TIME tStart);
typedef HRESULT(STDMETHODCALLTYPE *GetStatexx)(IBaseFilter * This, DWORD dwMilliSecsTimeout, FILTER_STATE *State);
typedef HRESULT(STDMETHODCALLTYPE *SetSyncSourcexx)(IBaseFilter * This, IReferenceClock *pClock);
typedef HRESULT(STDMETHODCALLTYPE *GetSyncSourcexx)(IBaseFilter * This, IReferenceClock **pClock);
typedef HRESULT(STDMETHODCALLTYPE *EnumPinsxx)(IBaseFilter * This, IEnumPins **ppEnum);
typedef HRESULT(STDMETHODCALLTYPE *FindPinxx)(IBaseFilter * This, LPCWSTR Id, IPin **ppPin);
typedef HRESULT(STDMETHODCALLTYPE *QueryFilterInfoxx)(IBaseFilter * This, FILTER_INFO *pInfo);
typedef HRESULT(STDMETHODCALLTYPE *JoinFilterGraphxx)(IBaseFilter * This, IFilterGraph *pGraph, LPCWSTR pName);
typedef HRESULT(STDMETHODCALLTYPE *QueryVendorInfoxx)(IBaseFilter * This, LPWSTR *pVendorInfo);

static int libAVFilter_Setup(libAVFilter *thisx, void *priv_data, void *callback, enum dshowDeviceType type)
{
	IBaseFilterVtbl *vtbl = thisx->vtbl;
	vtbl->QueryInterface = reinterpret_cast<QueryInterfacexx>(libAVFilter_QueryInterface);
	vtbl->AddRef = reinterpret_cast<AddRefxx>(libAVFilter_AddRef);
	vtbl->Release = reinterpret_cast<Releasexx>(libAVFilter_Release);
	vtbl->GetClassID = reinterpret_cast<GetClassIDxx>(libAVFilter_GetClassID);
	vtbl->Stop = reinterpret_cast<Stopxx>(libAVFilter_Stop);
	vtbl->Pause = reinterpret_cast<Pausexx>(libAVFilter_Pause);
	vtbl->Run = reinterpret_cast<Runxx>(libAVFilter_Run);
	vtbl->GetState = reinterpret_cast<GetStatexx>(libAVFilter_GetState);
	vtbl->SetSyncSource = reinterpret_cast<SetSyncSourcexx>(libAVFilter_SetSyncSource);
	vtbl->GetSyncSource = reinterpret_cast<GetSyncSourcexx>(libAVFilter_GetSyncSource);
	vtbl->EnumPins = reinterpret_cast<EnumPinsxx>(libAVFilter_EnumPins);
	vtbl->FindPin = reinterpret_cast<FindPinxx>(libAVFilter_FindPin);
	vtbl->QueryFilterInfo = reinterpret_cast<QueryFilterInfoxx>(libAVFilter_QueryFilterInfo);
	vtbl->JoinFilterGraph = reinterpret_cast<JoinFilterGraphxx>(libAVFilter_JoinFilterGraph);
	vtbl->QueryVendorInfo = reinterpret_cast<QueryVendorInfoxx>(libAVFilter_QueryVendorInfo);

	thisx->pin = libAVPin_Create(thisx);

	thisx->priv_data = priv_data;
	thisx->callback = reinterpret_cast<callbackxx>(callback);
	thisx->type = type;

	return 1;
}

static int libAVFilter_Cleanup(libAVFilter *thisx)
{
	libAVPin_Release(thisx->pin);
	return 1;
}

DECLARE_AVFILTER_CREATE(libAVFilter, libAVFilter_Setup(thisx, priv_data, callback, type),
	void *priv_data, void *callback, enum dshowDeviceType type)
DECLARE_DESTROY(libAVFilter, libAVFilter_Cleanup)

} // namespace fbc

#endif // _MSC_VER
