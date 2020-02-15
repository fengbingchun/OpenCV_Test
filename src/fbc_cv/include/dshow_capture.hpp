// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

// reference: ffmpeg 4.2
//            libavdevice/dshow_capture.h

#ifdef _MSC_VER

#ifndef FBC_CV_FFMPEG_DSHOW_CAPTURE_HPP_
#define FBC_CV_FFMPEG_DSHOW_CAPTURE_HPP_

#include <objidl.h>
#define CINTERFACE
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <dshow.h>
#include <dvdmedia.h>
#include <stdint.h>
#include <strmif.h>

#include "ffmpeg_pixel_format.hpp"
#include "ffmpeg_codec_id.hpp"
#include "ffmpeg_common.hpp"
#include "avclass.hpp"
#include "avpacket.hpp"
#include "avrational.hpp"
#include "avformat.hpp"

namespace fbc {

long ff_copy_dshow_media_type(AM_MEDIA_TYPE *dst, const AM_MEDIA_TYPE *src);
void ff_print_VIDEO_STREAM_CONFIG_CAPS(const VIDEO_STREAM_CONFIG_CAPS *caps);
void ff_print_AUDIO_STREAM_CONFIG_CAPS(const AUDIO_STREAM_CONFIG_CAPS *caps);
void ff_print_AM_MEDIA_TYPE(const AM_MEDIA_TYPE *type);
void ff_printGUID(const GUID *g);

#define dshowdebug(...)

struct GUIDoffset {
	const GUID *iid;
	int offset;
};

enum dshowDeviceType {
	VideoDevice = 0,
	AudioDevice = 1,
};

enum dshowSourceFilterType {
	VideoSourceDevice = 0,
	AudioSourceDevice = 1,
};

static inline void nothing(void *foo)
{
}

#define DECLARE_QUERYINTERFACE(classx, ...)                                   \
long WINAPI                                                                  \
classx##_QueryInterface(classx *thisx, const GUID *riid, void **ppvObject)      \
{                                                                            \
	struct GUIDoffset ifaces[] = __VA_ARGS__;                                \
	int i;                                                                   \
	dshowdebug(AV_STRINGIFY(classx)"_QueryInterface(%p, %p, %p)\n", thisx, riid, ppvObject); \
	ff_printGUID(riid);                                                      \
	if (!ppvObject)                                                          \
		return E_POINTER;                                                    \
	for (i = 0; i < sizeof(ifaces) / sizeof(ifaces[0]); i++) {                   \
		if (IsEqualGUID(*riid, *(ifaces[i].iid))) {                               \
			void *obj = (void *)((uint8_t *) thisx + ifaces[i].offset);      \
			classx##_AddRef(thisx);                                            \
			dshowdebug("\tfound %d with offset %d\n", i, ifaces[i].offset);  \
			*ppvObject = (void *)obj;                                       \
			return S_OK;                                                     \
		}                                                                    \
	}                                                                        \
	dshowdebug("\tE_NOINTERFACE\n");                                         \
	*ppvObject = NULL;                                                       \
	return E_NOINTERFACE;                                                    \
}

#define DECLARE_ADDREF(classx)                                                \
unsigned long WINAPI                                                         \
classx##_AddRef(classx *thisx)                                                  \
{                                                                            \
	dshowdebug(AV_STRINGIFY(classx)"_AddRef(%p)\t%ld\n", thisx, thisx->ref + 1);  \
	return InterlockedIncrement(&thisx->ref);                                 \
}

#define DECLARE_RELEASE(classx)                                               \
unsigned long WINAPI                                                         \
classx##_Release(classx *thisx)                                                 \
{                                                                            \
	long ref = InterlockedDecrement(&thisx->ref);                             \
	dshowdebug(AV_STRINGIFY(classx)"_Release(%p)\t%ld\n", thisx, ref);         \
	if (!ref)                                                                \
	classx##_Destroy(thisx);                                               \
	return ref;                                                              \
}

#define DECLARE_DESTROY(classx, func)                                         \
void classx##_Destroy(classx *thisx)                                            \
{                                                                            \
	dshowdebug(AV_STRINGIFY(classx)"_Destroy(%p)\n", thisx);                   \
	func(thisx);                                                              \
	if (thisx) {                                                               \
		if (thisx->vtbl)                                                      \
			CoTaskMemFree(thisx->vtbl);                                       \
		CoTaskMemFree(thisx);                                                 \
	}                                                                        \
}

#define DECLARE_CREATE(classx, setup, ...)                                    \
classx *classx##_Create(__VA_ARGS__)                                           \
{                                                                            \
	classx *thisx = CoTaskMemAlloc(sizeof(classx));                             \
	void  *vtbl = CoTaskMemAlloc(sizeof(*thisx->vtbl));                       \
	dshowdebug(AV_STRINGIFY(classx)"_Create(%p)\n", thisx);                    \
	if (!thisx || !vtbl)                                                      \
		goto fail;                                                           \
	ZeroMemory(thisx, sizeof(classx));                                         \
	ZeroMemory(vtbl, sizeof(*thisx->vtbl));                                   \
	thisx->ref = 1;                                                          \
	thisx->vtbl = vtbl;                                                       \
	if (!setup)                                                              \
		goto fail;                                                           \
	dshowdebug("created "AV_STRINGIFY(classx)" %p\n", thisx);                  \
	return thisx;                                                             \
fail:                                                                        \
	classx##_Destroy(thisx);                                                   \
	dshowdebug("could not create "AV_STRINGIFY(classx)"\n");                  \
	return NULL;                                                             \
}

#define DECLARE_AVFILTER_CREATE(classx, setup, ...)                                    \
classx *classx##_Create(__VA_ARGS__)                                           \
{                                                                            \
	classx *thisx = static_cast<classx*>(CoTaskMemAlloc(sizeof(classx)));                             \
	void  *vtbl = CoTaskMemAlloc(sizeof(*thisx->vtbl));                       \
	dshowdebug(AV_STRINGIFY(classx)"_Create(%p)\n", thisx);                    \
	if (!thisx || !vtbl)                                                      \
		goto fail;                                                           \
	ZeroMemory(thisx, sizeof(classx));                                         \
	ZeroMemory(vtbl, sizeof(*thisx->vtbl));                                   \
	thisx->ref = 1;                                                          \
	thisx->vtbl = static_cast<IBaseFilterVtbl *>(vtbl);                       \
	if (!setup)                                                              \
		goto fail;                                                           \
	dshowdebug("created "AV_STRINGIFY(classx)" %p\n", thisx);                  \
	return thisx;                                                             \
fail:                                                                        \
	classx##_Destroy(thisx);                                                   \
	dshowdebug("could not create "AV_STRINGIFY(classx)"\n");                  \
	return NULL;                                                             \
}

#define DECLARE_AVPIN_CREATE(classx, setup, ...)                                    \
	classx *classx##_Create(__VA_ARGS__)                                           \
{                                                                            \
	classx *thisx = static_cast<classx*>(CoTaskMemAlloc(sizeof(classx)));                             \
	void  *vtbl = CoTaskMemAlloc(sizeof(*thisx->vtbl));                       \
	dshowdebug(AV_STRINGIFY(classx)"_Create(%p)\n", thisx);                    \
	if (!thisx || !vtbl)                                                      \
		goto fail;                                                           \
	ZeroMemory(thisx, sizeof(classx));                                         \
	ZeroMemory(vtbl, sizeof(*thisx->vtbl));                                   \
	thisx->ref = 1;                                                          \
	thisx->vtbl = static_cast<IPinVtbl *>(vtbl);                                                       \
	if (!setup)                                                              \
		goto fail;                                                           \
	dshowdebug("created "AV_STRINGIFY(classx)" %p\n", thisx);                  \
	return thisx;                                                             \
fail:                                                                        \
	classx##_Destroy(thisx);                                                   \
	dshowdebug("could not create "AV_STRINGIFY(classx)"\n");                  \
	return NULL;                                                             \
}

#define DECLARE_AVENUMPINS_CREATE(classx, setup, ...)                                    \
	classx *classx##_Create(__VA_ARGS__)                                           \
{                                                                            \
	classx *thisx = static_cast<classx*>(CoTaskMemAlloc(sizeof(classx)));                             \
	void  *vtbl = CoTaskMemAlloc(sizeof(*thisx->vtbl));                       \
	dshowdebug(AV_STRINGIFY(classx)"_Create(%p)\n", thisx);                    \
	if (!thisx || !vtbl)                                                      \
		goto fail;                                                           \
	ZeroMemory(thisx, sizeof(classx));                                         \
	ZeroMemory(vtbl, sizeof(*thisx->vtbl));                                   \
	thisx->ref = 1;                                                          \
	thisx->vtbl = static_cast<IEnumPinsVtbl *>(vtbl);                                                       \
	if (!setup)                                                              \
		goto fail;                                                           \
	dshowdebug("created "AV_STRINGIFY(classx)" %p\n", thisx);                  \
	return thisx;                                                             \
fail:                                                                        \
	classx##_Destroy(thisx);                                                   \
	dshowdebug("could not create "AV_STRINGIFY(classx)"\n");                  \
	return NULL;                                                             \
}

#define DECLARE_AVENUMMEDIATYPES_CREATE(classx, setup, ...)                                    \
	classx *classx##_Create(__VA_ARGS__)                                           \
{                                                                            \
	classx *thisx = static_cast<classx*>(CoTaskMemAlloc(sizeof(classx)));                             \
	void  *vtbl = CoTaskMemAlloc(sizeof(*thisx->vtbl));                       \
	dshowdebug(AV_STRINGIFY(classx)"_Create(%p)\n", thisx);                    \
	if (!thisx || !vtbl)                                                      \
		goto fail;                                                           \
	ZeroMemory(thisx, sizeof(classx));                                         \
	ZeroMemory(vtbl, sizeof(*thisx->vtbl));                                   \
	thisx->ref = 1;                                                          \
	thisx->vtbl = static_cast<IEnumMediaTypesVtbl *>(vtbl);                                                       \
	if (!setup)                                                              \
		goto fail;                                                           \
	dshowdebug("created "AV_STRINGIFY(classx)" %p\n", thisx);                  \
	return thisx;                                                             \
fail:                                                                        \
	classx##_Destroy(thisx);                                                   \
	dshowdebug("could not create "AV_STRINGIFY(classx)"\n");                  \
	return NULL;                                                             \
}

#define SETVTBL(vtbl, classx, fn) \
	do { (vtbl)->fn = classx##_##fn; } while (0)

/*****************************************************************************
* Forward Declarations
****************************************************************************/
typedef struct libAVPin libAVPin;
typedef struct libAVMemInputPin libAVMemInputPin;
typedef struct libAVEnumPins libAVEnumPins;
typedef struct libAVEnumMediaTypes libAVEnumMediaTypes;
typedef struct libAVFilter libAVFilter;

/*****************************************************************************
* libAVPin
****************************************************************************/
struct libAVPin {
	IPinVtbl *vtbl;
	long ref;
	libAVFilter *filter;
	IPin *connectedto;
	AM_MEDIA_TYPE type;
	IMemInputPinVtbl *imemvtbl;
};

long          WINAPI libAVPin_QueryInterface(libAVPin *, const GUID *, void **);
unsigned long WINAPI libAVPin_AddRef(libAVPin *);
unsigned long WINAPI libAVPin_Release(libAVPin *);
long          WINAPI libAVPin_Connect(libAVPin *, IPin *, const AM_MEDIA_TYPE *);
long          WINAPI libAVPin_ReceiveConnection(libAVPin *, IPin *, const AM_MEDIA_TYPE *);
long          WINAPI libAVPin_Disconnect(libAVPin *);
long          WINAPI libAVPin_ConnectedTo(libAVPin *, IPin **);
long          WINAPI libAVPin_ConnectionMediaType(libAVPin *, AM_MEDIA_TYPE *);
long          WINAPI libAVPin_QueryPinInfo(libAVPin *, PIN_INFO *);
long          WINAPI libAVPin_QueryDirection(libAVPin *, PIN_DIRECTION *);
long          WINAPI libAVPin_QueryId(libAVPin *, wchar_t **);
long          WINAPI libAVPin_QueryAccept(libAVPin *, const AM_MEDIA_TYPE *);
long          WINAPI libAVPin_EnumMediaTypes(libAVPin *, IEnumMediaTypes **);
long          WINAPI libAVPin_QueryInternalConnections(libAVPin *, IPin **, unsigned long *);
long          WINAPI libAVPin_EndOfStream(libAVPin *);
long          WINAPI libAVPin_BeginFlush(libAVPin *);
long          WINAPI libAVPin_EndFlush(libAVPin *);
long          WINAPI libAVPin_NewSegment(libAVPin *, REFERENCE_TIME, REFERENCE_TIME, double);

long          WINAPI libAVMemInputPin_QueryInterface(libAVMemInputPin *, const GUID *, void **);
unsigned long WINAPI libAVMemInputPin_AddRef(libAVMemInputPin *);
unsigned long WINAPI libAVMemInputPin_Release(libAVMemInputPin *);
long          WINAPI libAVMemInputPin_GetAllocator(libAVMemInputPin *, IMemAllocator **);
long          WINAPI libAVMemInputPin_NotifyAllocator(libAVMemInputPin *, IMemAllocator *, BOOL);
long          WINAPI libAVMemInputPin_GetAllocatorRequirements(libAVMemInputPin *, ALLOCATOR_PROPERTIES *);
long          WINAPI libAVMemInputPin_Receive(libAVMemInputPin *, IMediaSample *);
long          WINAPI libAVMemInputPin_ReceiveMultiple(libAVMemInputPin *, IMediaSample **, long, long *);
long          WINAPI libAVMemInputPin_ReceiveCanBlock(libAVMemInputPin *);

void                 libAVPin_Destroy(libAVPin *);
libAVPin            *libAVPin_Create(libAVFilter *filter);

void                 libAVMemInputPin_Destroy(libAVMemInputPin *);

/*****************************************************************************
* libAVEnumPins
****************************************************************************/
struct libAVEnumPins {
	IEnumPinsVtbl *vtbl;
	long ref;
	int pos;
	libAVPin *pin;
	libAVFilter *filter;
};

long          WINAPI libAVEnumPins_QueryInterface(libAVEnumPins *, const GUID *, void **);
unsigned long WINAPI libAVEnumPins_AddRef(libAVEnumPins *);
unsigned long WINAPI libAVEnumPins_Release(libAVEnumPins *);
long          WINAPI libAVEnumPins_Next(libAVEnumPins *, unsigned long, IPin **, unsigned long *);
long          WINAPI libAVEnumPins_Skip(libAVEnumPins *, unsigned long);
long          WINAPI libAVEnumPins_Reset(libAVEnumPins *);
long          WINAPI libAVEnumPins_Clone(libAVEnumPins *, libAVEnumPins **);

void                 libAVEnumPins_Destroy(libAVEnumPins *);
libAVEnumPins       *libAVEnumPins_Create(libAVPin *pin, libAVFilter *filter);

/*****************************************************************************
* libAVEnumMediaTypes
****************************************************************************/
struct libAVEnumMediaTypes {
	IEnumMediaTypesVtbl *vtbl;
	long ref;
	int pos;
	AM_MEDIA_TYPE type;
};

long          WINAPI libAVEnumMediaTypes_QueryInterface(libAVEnumMediaTypes *, const GUID *, void **);
unsigned long WINAPI libAVEnumMediaTypes_AddRef(libAVEnumMediaTypes *);
unsigned long WINAPI libAVEnumMediaTypes_Release(libAVEnumMediaTypes *);
long          WINAPI libAVEnumMediaTypes_Next(libAVEnumMediaTypes *, unsigned long, AM_MEDIA_TYPE **, unsigned long *);
long          WINAPI libAVEnumMediaTypes_Skip(libAVEnumMediaTypes *, unsigned long);
long          WINAPI libAVEnumMediaTypes_Reset(libAVEnumMediaTypes *);
long          WINAPI libAVEnumMediaTypes_Clone(libAVEnumMediaTypes *, libAVEnumMediaTypes **);

void                 libAVEnumMediaTypes_Destroy(libAVEnumMediaTypes *);
libAVEnumMediaTypes *libAVEnumMediaTypes_Create(const AM_MEDIA_TYPE *type);

/*****************************************************************************
* libAVFilter
****************************************************************************/
struct libAVFilter {
	IBaseFilterVtbl *vtbl;
	long ref;
	const wchar_t *name;
	libAVPin *pin;
	FILTER_INFO info;
	FILTER_STATE state;
	IReferenceClock *clock;
	enum dshowDeviceType type;
	void *priv_data;
	int stream_index;
	int64_t start_time;
	void(*callback)(void *priv_data, int index, uint8_t *buf, int buf_size, int64_t time, enum dshowDeviceType type);
};

long          WINAPI libAVFilter_QueryInterface(libAVFilter *, const GUID *, void **);
unsigned long WINAPI libAVFilter_AddRef(libAVFilter *);
unsigned long WINAPI libAVFilter_Release(libAVFilter *);
long          WINAPI libAVFilter_GetClassID(libAVFilter *, CLSID *);
long          WINAPI libAVFilter_Stop(libAVFilter *);
long          WINAPI libAVFilter_Pause(libAVFilter *);
long          WINAPI libAVFilter_Run(libAVFilter *, REFERENCE_TIME);
long          WINAPI libAVFilter_GetState(libAVFilter *, DWORD, FILTER_STATE *);
long          WINAPI libAVFilter_SetSyncSource(libAVFilter *, IReferenceClock *);
long          WINAPI libAVFilter_GetSyncSource(libAVFilter *, IReferenceClock **);
long          WINAPI libAVFilter_EnumPins(libAVFilter *, IEnumPins **);
long          WINAPI libAVFilter_FindPin(libAVFilter *, const wchar_t *, IPin **);
long          WINAPI libAVFilter_QueryFilterInfo(libAVFilter *, FILTER_INFO *);
long          WINAPI libAVFilter_JoinFilterGraph(libAVFilter *, IFilterGraph *, const wchar_t *);
long          WINAPI libAVFilter_QueryVendorInfo(libAVFilter *, wchar_t **);

void                 libAVFilter_Destroy(libAVFilter *);
libAVFilter         *libAVFilter_Create(void *, void *, enum dshowDeviceType);

/*****************************************************************************
* dshow_ctx
****************************************************************************/
struct dshow_ctx {
	const AVClass *class_;

	IGraphBuilder *graph;

	char *device_name[2];
	char *device_unique_name[2];

	int video_device_number;
	int audio_device_number;

	int   list_options;
	int   list_devices;
	int   audio_buffer_size;
	int   crossbar_video_input_pin_number;
	int   crossbar_audio_input_pin_number;
	char *video_pin_name;
	char *audio_pin_name;
	int   show_video_device_dialog;
	int   show_audio_device_dialog;
	int   show_video_crossbar_connection_dialog;
	int   show_audio_crossbar_connection_dialog;
	int   show_analog_tv_tuner_dialog;
	int   show_analog_tv_tuner_audio_dialog;
	char *audio_filter_load_file;
	char *audio_filter_save_file;
	char *video_filter_load_file;
	char *video_filter_save_file;

	IBaseFilter *device_filter[2];
	IPin        *device_pin[2];
	libAVFilter *capture_filter[2];
	libAVPin    *capture_pin[2];

	HANDLE mutex;
	HANDLE event[2]; /* event[0] is set by DirectShow
				* event[1] is set by callback() */
	AVPacketList *pktl;

	int eof;

	int64_t curbufsize[2];
	unsigned int video_frame_num;

	IMediaControl *control;
	IMediaEvent *media_event;

	enum AVPixelFormat pixel_format;
	enum AVCodecID video_codec_id;
	char *framerate;

	int requested_width;
	int requested_height;
	AVRational requested_framerate;

	int sample_rate;
	int sample_size;
	int channels;
};

int dshow_open_device(AVFormatContext *avctx, ICreateDevEnum *devenum,
		enum dshowDeviceType devtype, enum dshowSourceFilterType sourcetype);
int dshow_cycle_devices(AVFormatContext *avctx, ICreateDevEnum *devenum,
		enum dshowDeviceType devtype, enum dshowSourceFilterType sourcetype,
		IBaseFilter **pfilter, char **device_unique_name);
int dshow_cycle_pins(AVFormatContext *avctx, enum dshowDeviceType devtype,
		enum dshowSourceFilterType sourcetype, IBaseFilter *device_filter, IPin **ppin);
HRESULT dshow_try_setup_crossbar_options(ICaptureGraphBuilder2 *graph_builder2,
		IBaseFilter *device_filter, enum dshowDeviceType devtype, AVFormatContext *avctx);
void callback(void *priv_data, int index, uint8_t *buf, int buf_size, int64_t time, enum dshowDeviceType devtype);
HRESULT SHCreateStreamOnFile_xxx(LPCSTR pszFile, DWORD grfMode, IStream **ppstm);
void dshow_show_filter_properties(IBaseFilter *pFilter, AVFormatContext *avctx);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_FFMPEG_DSHOW_CAPTURE_HPP_