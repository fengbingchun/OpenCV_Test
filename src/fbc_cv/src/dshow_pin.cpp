// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "dshow_capture.hpp"
#include <stddef.h>
#include <inttypes.h>
#include "avmem.hpp"

// reference: ffmpeg 4.2
//            libavdevice/dshow_pin.c

#ifdef _MSC_VER

namespace fbc {

#define imemoffset offsetof(libAVPin, imemvtbl)

DECLARE_QUERYINTERFACE(libAVPin,
	{ { &IID_IUnknown, 0 }, { &IID_IPin, 0 }, { &IID_IMemInputPin, imemoffset } })
DECLARE_ADDREF(libAVPin)
DECLARE_RELEASE(libAVPin)

long WINAPI libAVPin_Connect(libAVPin *thisx, IPin *pin, const AM_MEDIA_TYPE *type)
{
	dshowdebug("libAVPin_Connect(%p, %p, %p)\n", thisx, pin, type);
	/* Input pins receive connections. */
	return S_FALSE;
}

long WINAPI libAVPin_ReceiveConnection(libAVPin *thisx, IPin *pin,
	const AM_MEDIA_TYPE *type)
{
	enum dshowDeviceType devtype = thisx->filter->type;
	dshowdebug("libAVPin_ReceiveConnection(%p)\n", this);

	if (!pin)
		return E_POINTER;
	if (thisx->connectedto)
		return VFW_E_ALREADY_CONNECTED;

	ff_print_AM_MEDIA_TYPE(type);
	if (devtype == VideoDevice) {
		if (!IsEqualGUID(type->majortype, MEDIATYPE_Video))
			return VFW_E_TYPE_NOT_ACCEPTED;
	}
	else {
		if (!IsEqualGUID(type->majortype, MEDIATYPE_Audio))
			return VFW_E_TYPE_NOT_ACCEPTED;
	}

	IPin_AddRef(pin);
	thisx->connectedto = pin;

	ff_copy_dshow_media_type(&thisx->type, type);

	return S_OK;
}

long WINAPI libAVPin_Disconnect(libAVPin *thisx)
{
	dshowdebug("libAVPin_Disconnect(%p)\n", this);

	if (thisx->filter->state != State_Stopped)
		return VFW_E_NOT_STOPPED;
	if (!thisx->connectedto)
		return S_FALSE;
	IPin_Release(thisx->connectedto);
	thisx->connectedto = NULL;

	return S_OK;
}

long WINAPI libAVPin_ConnectedTo(libAVPin *thisx, IPin **pin)
{
	dshowdebug("libAVPin_ConnectedTo(%p)\n", thisx);

	if (!pin)
		return E_POINTER;
	if (!thisx->connectedto)
		return VFW_E_NOT_CONNECTED;
	IPin_AddRef(thisx->connectedto);
	*pin = thisx->connectedto;

	return S_OK;
}

long WINAPI libAVPin_ConnectionMediaType(libAVPin *thisx, AM_MEDIA_TYPE *type)
{
	dshowdebug("libAVPin_ConnectionMediaType(%p)\n", thisx);

	if (!type)
		return E_POINTER;
	if (!thisx->connectedto)
		return VFW_E_NOT_CONNECTED;

	return ff_copy_dshow_media_type(type, &thisx->type);
}

long WINAPI libAVPin_QueryPinInfo(libAVPin *thisx, PIN_INFO *info)
{
	dshowdebug("libAVPin_QueryPinInfo(%p)\n", thisx);

	if (!info)
		return E_POINTER;

	if (thisx->filter)
		libAVFilter_AddRef(thisx->filter);

	info->pFilter = (IBaseFilter *) thisx->filter;
	info->dir = PINDIR_INPUT;
	wcscpy(info->achName, L"Capture");

	return S_OK;
}

long WINAPI libAVPin_QueryDirection(libAVPin *thisx, PIN_DIRECTION *dir)
{
	dshowdebug("libAVPin_QueryDirection(%p)\n", this);
	if (!dir)
		return E_POINTER;
	*dir = PINDIR_INPUT;
	return S_OK;
}

long WINAPI libAVPin_QueryId(libAVPin *thisx, wchar_t **id)
{
	dshowdebug("libAVPin_QueryId(%p)\n", this);

	if (!id)
		return E_POINTER;

	*id = _wcsdup(L"libAV Pin");

	return S_OK;
}

long WINAPI libAVPin_QueryAccept(libAVPin *thisx, const AM_MEDIA_TYPE *type)
{
	dshowdebug("libAVPin_QueryAccept(%p)\n", this);
	return S_FALSE;
}

long WINAPI libAVPin_EnumMediaTypes(libAVPin *thisx, IEnumMediaTypes **enumtypes)
{
	const AM_MEDIA_TYPE *type = NULL;
	libAVEnumMediaTypes *newx;
	dshowdebug("libAVPin_EnumMediaTypes(%p)\n", thisx);

	if (!enumtypes)
		return E_POINTER;
	newx = libAVEnumMediaTypes_Create(type);
	if (!newx)
		return E_OUTOFMEMORY;

	*enumtypes = (IEnumMediaTypes *) newx;
	return S_OK;
}

long WINAPI libAVPin_QueryInternalConnections(libAVPin *thisx, IPin **pin,
	unsigned long *npin)
{
	dshowdebug("libAVPin_QueryInternalConnections(%p)\n", thisx);
	return E_NOTIMPL;
}

long WINAPI libAVPin_EndOfStream(libAVPin *thisx)
{
	dshowdebug("libAVPin_EndOfStream(%p)\n", thisx);
	/* I don't care. */
	return S_OK;
}

long WINAPI libAVPin_BeginFlush(libAVPin *thisx)
{
	dshowdebug("libAVPin_BeginFlush(%p)\n", thisx);
	/* I don't care. */
	return S_OK;
}

long WINAPI libAVPin_EndFlush(libAVPin *thisx)
{
	dshowdebug("libAVPin_EndFlush(%p)\n", thisx);
	/* I don't care. */
	return S_OK;
}

long WINAPI libAVPin_NewSegment(libAVPin *thisx, REFERENCE_TIME start, REFERENCE_TIME stop,
	double rate)
{
	dshowdebug("libAVPin_NewSegment(%p)\n", thisx);
	/* I don't care. */
	return S_OK;
}

typedef HRESULT(STDMETHODCALLTYPE *QueryInterfacexx)(IMemInputPin * This, REFIID riid, void **ppvObject);
typedef ULONG(STDMETHODCALLTYPE *AddRefxx)(IMemInputPin * This);
typedef ULONG(STDMETHODCALLTYPE *Releasexx)(IMemInputPin * This);
typedef HRESULT(STDMETHODCALLTYPE *GetAllocatorxx)(IMemInputPin * This, IMemAllocator **ppAllocator);
typedef HRESULT(STDMETHODCALLTYPE *NotifyAllocatorxx)(IMemInputPin * This, IMemAllocator *pAllocator, BOOL bReadOnly);
typedef HRESULT(STDMETHODCALLTYPE *GetAllocatorRequirementsxx)(IMemInputPin * This, ALLOCATOR_PROPERTIES *pProps);
typedef HRESULT(STDMETHODCALLTYPE *Receivexx)(IMemInputPin * This, IMediaSample *pSample);
typedef HRESULT(STDMETHODCALLTYPE *ReceiveMultiplexx)(IMemInputPin * This, IMediaSample **pSamples, long nSamples, long *nSamplesProcessed);
typedef HRESULT(STDMETHODCALLTYPE *ReceiveCanBlockxx)(IMemInputPin * This);

typedef HRESULT(STDMETHODCALLTYPE *QueryInterfacexxx)(IPin * This, REFIID riid, void **ppvObject);
typedef ULONG(STDMETHODCALLTYPE *AddRefxxx)(IPin * This);
typedef ULONG(STDMETHODCALLTYPE *Releasexxx)(IPin * This);
typedef HRESULT(STDMETHODCALLTYPE *Connectxxx)(IPin * This, IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
typedef HRESULT(STDMETHODCALLTYPE *ReceiveConnectionxxx)(IPin * This, IPin *pConnector, const AM_MEDIA_TYPE *pmt);
typedef HRESULT(STDMETHODCALLTYPE *Disconnectxxx)(IPin * This);
typedef HRESULT(STDMETHODCALLTYPE *ConnectedToxxx)(IPin * This, IPin **pPin);
typedef HRESULT(STDMETHODCALLTYPE *ConnectionMediaTypexxx)(IPin * This, AM_MEDIA_TYPE *pmt);
typedef HRESULT(STDMETHODCALLTYPE *QueryPinInfoxxx)(IPin * This, PIN_INFO *pInfo);
typedef HRESULT(STDMETHODCALLTYPE *QueryDirectionxxx)(IPin * This, PIN_DIRECTION *pPinDir);
typedef HRESULT(STDMETHODCALLTYPE *QueryIdxxx)(IPin * This, LPWSTR *Id);
typedef HRESULT(STDMETHODCALLTYPE *QueryAcceptxxx)(IPin * This, const AM_MEDIA_TYPE *pmt);
typedef HRESULT(STDMETHODCALLTYPE *EnumMediaTypesxxx)(IPin * This, IEnumMediaTypes **ppEnum);
typedef HRESULT(STDMETHODCALLTYPE *QueryInternalConnectionsxxx)(IPin * This, IPin **apPin, ULONG *nPin);
typedef HRESULT(STDMETHODCALLTYPE *EndOfStreamxxx)(IPin * This);
typedef HRESULT(STDMETHODCALLTYPE *BeginFlushxxx)(IPin * This);
typedef HRESULT(STDMETHODCALLTYPE *EndFlushxxx)(IPin * This);
typedef HRESULT(STDMETHODCALLTYPE *NewSegmentxxx)(IPin * This, REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

static int libAVPin_Setup(libAVPin *thisx, libAVFilter *filter)
{
	IPinVtbl *vtbl = thisx->vtbl;
	IMemInputPinVtbl *imemvtbl;

	if (!filter)
		return 0;

	imemvtbl = static_cast<IMemInputPinVtbl *>(av_malloc(sizeof(IMemInputPinVtbl)));
	if (!imemvtbl)
		return 0;

	imemvtbl->QueryInterface = reinterpret_cast<QueryInterfacexx>(libAVMemInputPin_QueryInterface);
	imemvtbl->AddRef = reinterpret_cast<AddRefxx>(libAVMemInputPin_AddRef);
	imemvtbl->Release = reinterpret_cast<Releasexx>(libAVMemInputPin_Release);
	imemvtbl->GetAllocator = reinterpret_cast<GetAllocatorxx>(libAVMemInputPin_GetAllocator);
	imemvtbl->NotifyAllocator = reinterpret_cast<NotifyAllocatorxx>(libAVMemInputPin_NotifyAllocator);
	imemvtbl->GetAllocatorRequirements = reinterpret_cast<GetAllocatorRequirementsxx>(libAVMemInputPin_GetAllocatorRequirements);
	imemvtbl->Receive = reinterpret_cast<Receivexx>(libAVMemInputPin_Receive);
	imemvtbl->ReceiveMultiple = reinterpret_cast<ReceiveMultiplexx>(libAVMemInputPin_ReceiveMultiple);
	imemvtbl->ReceiveCanBlock = reinterpret_cast<ReceiveCanBlockxx>(libAVMemInputPin_ReceiveCanBlock);

	thisx->imemvtbl = imemvtbl;

	vtbl->QueryInterface = reinterpret_cast<QueryInterfacexxx>(libAVPin_QueryInterface);
	vtbl->AddRef = reinterpret_cast<AddRefxxx>(libAVPin_AddRef);
	vtbl->Release = reinterpret_cast<Releasexxx>(libAVPin_Release);
	vtbl->Connect = reinterpret_cast<Connectxxx>(libAVPin_Connect);
	vtbl->ReceiveConnection = reinterpret_cast<ReceiveConnectionxxx>(libAVPin_ReceiveConnection);
	vtbl->Disconnect = reinterpret_cast<Disconnectxxx>(libAVPin_Disconnect);
	vtbl->ConnectedTo = reinterpret_cast<ConnectedToxxx>(libAVPin_ConnectedTo);
	vtbl->ConnectionMediaType = reinterpret_cast<ConnectionMediaTypexxx>(libAVPin_ConnectionMediaType);
	vtbl->QueryPinInfo = reinterpret_cast<QueryPinInfoxxx>(libAVPin_QueryPinInfo);
	vtbl->QueryId = reinterpret_cast<QueryIdxxx>(libAVPin_QueryId);
	vtbl->QueryAccept = reinterpret_cast<QueryAcceptxxx>(libAVPin_QueryAccept);
	vtbl->EnumMediaTypes = reinterpret_cast<EnumMediaTypesxxx>(libAVPin_EnumMediaTypes);
	vtbl->QueryInternalConnections = reinterpret_cast<QueryInternalConnectionsxxx>(libAVPin_QueryInternalConnections);
	vtbl->EndOfStream = reinterpret_cast<EndOfStreamxxx>(libAVPin_EndOfStream);
	vtbl->BeginFlush = reinterpret_cast<BeginFlushxxx>(libAVPin_BeginFlush);
	vtbl->EndFlush = reinterpret_cast<EndFlushxxx>(libAVPin_EndFlush);
	vtbl->NewSegment = reinterpret_cast<NewSegmentxxx>(libAVPin_NewSegment);
	vtbl->QueryDirection = reinterpret_cast<QueryDirectionxxx>(libAVPin_QueryDirection);

	thisx->filter = filter;

	return 1;
}

static void libAVPin_Free(libAVPin *thisx)
{
	if (!thisx)
		return;
	av_freep(&thisx->imemvtbl);
	if (thisx->type.pbFormat) {
		CoTaskMemFree(thisx->type.pbFormat);
		thisx->type.pbFormat = NULL;
	}
}
DECLARE_AVPIN_CREATE(libAVPin, libAVPin_Setup(thisx, filter), libAVFilter *filter)
DECLARE_DESTROY(libAVPin, libAVPin_Free)

/*****************************************************************************
* libAVMemInputPin
****************************************************************************/
long WINAPI libAVMemInputPin_QueryInterface(libAVMemInputPin *thisx, const GUID *riid,
	void **ppvObject)
{
	libAVPin *pin = (libAVPin *)((uint8_t *) thisx - imemoffset);
	dshowdebug("libAVMemInputPin_QueryInterface(%p)\n", thisx);
	return libAVPin_QueryInterface(pin, riid, ppvObject);
}

unsigned long WINAPI libAVMemInputPin_AddRef(libAVMemInputPin *thisx)
{
	libAVPin *pin = (libAVPin *)((uint8_t *) thisx - imemoffset);
	dshowdebug("libAVMemInputPin_AddRef(%p)\n", thisx);
	return libAVPin_AddRef(pin);
}

unsigned long WINAPI libAVMemInputPin_Release(libAVMemInputPin *thisx)
{
	libAVPin *pin = (libAVPin *)((uint8_t *) thisx - imemoffset);
	dshowdebug("libAVMemInputPin_Release(%p)\n", thisx);
	return libAVPin_Release(pin);
}

long WINAPI libAVMemInputPin_GetAllocator(libAVMemInputPin *thisx, IMemAllocator **alloc)
{
	dshowdebug("libAVMemInputPin_GetAllocator(%p)\n", thisx);
	return VFW_E_NO_ALLOCATOR;
}

long WINAPI libAVMemInputPin_NotifyAllocator(libAVMemInputPin *thisx, IMemAllocator *alloc,
	BOOL rdwr)
{
	dshowdebug("libAVMemInputPin_NotifyAllocator(%p)\n", thisx);
	return S_OK;
}

long WINAPI libAVMemInputPin_GetAllocatorRequirements(libAVMemInputPin *thisx,
	ALLOCATOR_PROPERTIES *props)
{
	dshowdebug("libAVMemInputPin_GetAllocatorRequirements(%p)\n", thisx);
	return E_NOTIMPL;
}

long WINAPI libAVMemInputPin_Receive(libAVMemInputPin *thisx, IMediaSample *sample)
{
	libAVPin *pin = (libAVPin *)((uint8_t *) thisx - imemoffset);
	enum dshowDeviceType devtype = pin->filter->type;
	void *priv_data;
	AVFormatContext *s;
	uint8_t *buf;
	int buf_size; /* todo should be a long? */
	int index;
	int64_t curtime;
	int64_t orig_curtime;
	int64_t graphtime;
	const char *devtypename = (devtype == VideoDevice) ? "video" : "audio";
	IReferenceClock *clock = pin->filter->clock;
	int64_t dummy;
	struct dshow_ctx *ctx;


	dshowdebug("libAVMemInputPin_Receive(%p)\n", this);

	if (!sample)
		return E_POINTER;

	IMediaSample_GetTime(sample, &orig_curtime, &dummy);
	orig_curtime += pin->filter->start_time;
	IReferenceClock_GetTime(clock, &graphtime);
	if (devtype == VideoDevice) {
		/* PTS from video devices is unreliable. */
		IReferenceClock_GetTime(clock, &curtime);
	}
	else {
		IMediaSample_GetTime(sample, &curtime, &dummy);
		if (curtime > 400000000000000000LL) {
			/* initial frames sometimes start < 0 (shown as a very large number here,
			like 437650244077016960 which FFmpeg doesn't like.
			TODO figure out math. For now just drop them. */
			fprintf(stderr, "dshow_pin: dshow dropping initial (or ending) audio frame with odd PTS too high %"PRId64"\n", curtime);
			return S_OK;
		}
		curtime += pin->filter->start_time;
	}

	buf_size = IMediaSample_GetActualDataLength(sample);
	IMediaSample_GetPointer(sample, &buf);
	priv_data = pin->filter->priv_data;
	s = static_cast<AVFormatContext *>(priv_data);
	ctx = static_cast<struct dshow_ctx *>(s->priv_data);
	index = pin->filter->stream_index;

	//fprintf(stderr, "AV_LOG_VERBOSE, dshow_pin: dshow passing through packet of type %s size %8d "
	//	"timestamp %"PRId64" orig timestamp %"PRId64" graph timestamp %"PRId64" diff %"PRId64" %s\n",
	//	devtypename, buf_size, curtime, orig_curtime, graphtime, graphtime - orig_curtime, ctx->device_name[devtype]);
	pin->filter->callback(priv_data, index, buf, buf_size, curtime, devtype);

	return S_OK;
}

long WINAPI libAVMemInputPin_ReceiveMultiple(libAVMemInputPin *thisx,
	IMediaSample **samples, long n, long *nproc)
{
	int i;
	dshowdebug("libAVMemInputPin_ReceiveMultiple(%p)\n", thisx);

	for (i = 0; i < n; i++)
		libAVMemInputPin_Receive(thisx, samples[i]);

	*nproc = n;
	return S_OK;
}

long WINAPI libAVMemInputPin_ReceiveCanBlock(libAVMemInputPin *thisx)
{
	dshowdebug("libAVMemInputPin_ReceiveCanBlock(%p)\n", thisx);
	/* I swear I will not block. */
	return S_FALSE;
}

void libAVMemInputPin_Destroy(libAVMemInputPin *thisx)
{
	libAVPin *pin = (libAVPin *)((uint8_t *) thisx - imemoffset);
	dshowdebug("libAVMemInputPin_Destroy(%p)\n", thisx);
	libAVPin_Destroy(pin);
}

} // namespace fbc

#endif // _MSC_VER
