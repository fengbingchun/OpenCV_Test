// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_FFMPEG_COMMON_HPP_
#define FBC_CV_FFMPEG_COMMON_HPP_

// reference: ffmpeg 4.2
//            libavutil/version.h

#ifdef _MSC_VER

#include <stdint.h>
#include <crtdefs.h>
#include <cstddef>
#include <errno.h>
#include <stdlib.h>
#include <process.h>
#include <direct.h>
#include <fcntl.h>

namespace fbc {

#define AV_VERSION_INT(a, b, c) ((a)<<16 | (b)<<8 | (c))
#define AV_VERSION_DOT(a, b, c) a ##.## b ##.## c
#define AV_VERSION(a, b, c) AV_VERSION_DOT(a, b, c)

#define LIBAVUTIL_VERSION_MAJOR  56
#define LIBAVUTIL_VERSION_MINOR  36
#define LIBAVUTIL_VERSION_MICRO 101

#define LIBAVUTIL_VERSION_INT		AV_VERSION_INT(LIBAVUTIL_VERSION_MAJOR, LIBAVUTIL_VERSION_MINOR, LIBAVUTIL_VERSION_MICRO)
#define LIBAVUTIL_VERSION		AV_VERSION(LIBAVUTIL_VERSION_MAJOR, LIBAVUTIL_VERSION_MINOR, LIBAVUTIL_VERSION_MICRO)
#define LIBAVUTIL_BUILD			LIBAVUTIL_VERSION_INT

#ifndef FF_API_VAAPI
#define FF_API_VAAPI			(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_FRAME_QP
#define FF_API_FRAME_QP			(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_PLUS1_MINUS1
#define FF_API_PLUS1_MINUS1		(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_ERROR_FRAME
#define FF_API_ERROR_FRAME		(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_PKT_PTS
#define FF_API_PKT_PTS			(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_CRYPTO_SIZE_T
#define FF_API_CRYPTO_SIZE_T		(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_FRAME_GET_SET
#define FF_API_FRAME_GET_SET		(LIBAVUTIL_VERSION_MAJOR < 57)
#endif
#ifndef FF_API_PSEUDOPAL
#define FF_API_PSEUDOPAL		(LIBAVUTIL_VERSION_MAJOR < 57)
#endif

#define LIBAVCODEC_VERSION_MAJOR  58
#define LIBAVCODEC_VERSION_MINOR  65
#define LIBAVCODEC_VERSION_MICRO 100

#ifndef FF_API_LOWRES
#define FF_API_LOWRES			(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_DEBUG_MV
#define FF_API_DEBUG_MV			(LIBAVCODEC_VERSION_MAJOR < 58)
#endif
#ifndef FF_API_AVCTX_TIMEBASE
#define FF_API_AVCTX_TIMEBASE		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_CODED_FRAME
#define FF_API_CODED_FRAME		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_SIDEDATA_ONLY_PKT
#define FF_API_SIDEDATA_ONLY_PKT	(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_VDPAU_PROFILE
#define FF_API_VDPAU_PROFILE		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_CONVERGENCE_DURATION
#define FF_API_CONVERGENCE_DURATION	(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_AVPICTURE
#define FF_API_AVPICTURE		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_AVPACKET_OLD_API
#define FF_API_AVPACKET_OLD_API		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_RTP_CALLBACK
#define FF_API_RTP_CALLBACK		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_VBV_DELAY
#define FF_API_VBV_DELAY		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_CODER_TYPE
#define FF_API_CODER_TYPE		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_STAT_BITS
#define FF_API_STAT_BITS		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_PRIVATE_OPT
#define FF_API_PRIVATE_OPT		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_ASS_TIMING
#define FF_API_ASS_TIMING		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_OLD_BSF
#define FF_API_OLD_BSF			(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_COPY_CONTEXT
#define FF_API_COPY_CONTEXT		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_GET_CONTEXT_DEFAULTS
#define FF_API_GET_CONTEXT_DEFAULTS	(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_NVENC_OLD_NAME
#define FF_API_NVENC_OLD_NAME		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_STRUCT_VAAPI_CONTEXT
#define FF_API_STRUCT_VAAPI_CONTEXT	(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_MERGE_SD_API
#define FF_API_MERGE_SD_API		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_TAG_STRING
#define FF_API_TAG_STRING		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_GETCHROMA
#define FF_API_GETCHROMA		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_CODEC_GET_SET
#define FF_API_CODEC_GET_SET		(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_USER_VISIBLE_AVHWACCEL
#define FF_API_USER_VISIBLE_AVHWACCEL	(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_LOCKMGR
#define FF_API_LOCKMGR			(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_NEXT
#define FF_API_NEXT			(LIBAVCODEC_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_UNSANITIZED_BITRATES
#define FF_API_UNSANITIZED_BITRATES	(LIBAVCODEC_VERSION_MAJOR < 59)
#endif

#define LIBAVFORMAT_VERSION_MAJOR  58
#define LIBAVFORMAT_VERSION_MINOR  35
#define LIBAVFORMAT_VERSION_MICRO 101

#ifndef FF_API_COMPUTE_PKT_FIELDS2
#define FF_API_COMPUTE_PKT_FIELDS2	(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_OLD_OPEN_CALLBACKS
#define FF_API_OLD_OPEN_CALLBACKS	(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_LAVF_AVCTX
#define FF_API_LAVF_AVCTX		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_HTTP_USER_AGENT
#define FF_API_HTTP_USER_AGENT		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_HLS_WRAP
#define FF_API_HLS_WRAP			(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_HLS_USE_LOCALTIME
#define FF_API_HLS_USE_LOCALTIME	(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_LAVF_KEEPSIDE_FLAG
#define FF_API_LAVF_KEEPSIDE_FLAG	(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_OLD_ROTATE_API
#define FF_API_OLD_ROTATE_API		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_FORMAT_GET_SET
#define FF_API_FORMAT_GET_SET		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_OLD_AVIO_EOF_0
#define FF_API_OLD_AVIO_EOF_0		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_LAVF_FFSERVER
#define FF_API_LAVF_FFSERVER		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_FORMAT_FILENAME
#define FF_API_FORMAT_FILENAME		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_OLD_RTSP_OPTIONS
#define FF_API_OLD_RTSP_OPTIONS		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_DASH_MIN_SEG_DURATION
#define FF_API_DASH_MIN_SEG_DURATION	(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_LAVF_MP4A_LATM
#define FF_API_LAVF_MP4A_LATM		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif
#ifndef FF_API_AVIOFORMAT
#define FF_API_AVIOFORMAT		(LIBAVFORMAT_VERSION_MAJOR < 59)
#endif

#ifndef FF_API_R_FRAME_RATE
#define FF_API_R_FRAME_RATE		1
#endif

#define CONFIG_SMALL 0
#define CONFIG_SDL2 1

#if CONFIG_SMALL
#   define NULL_IF_CONFIG_SMALL(x) NULL
#else
#   define NULL_IF_CONFIG_SMALL(x) x
#endif

#define av_always_inline __forceinline

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define attribute_deprecated __declspec(deprecated)

typedef intptr_t atomic_uint;

#define HAVE_ALIGNED_MALLOC 1
#define HAVE_AVX 1
#define HAVE_AVX2 1
#define HAVE_AVX512 1
#define HAVE_WGLGETPROCADDRESS 1
#define HAVE_PRAGMA_DEPRECATED 1

#if HAVE_PRAGMA_DEPRECATED
#    if defined(__ICL) || defined (__INTEL_COMPILER)
#        define FF_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:1478))
#        define FF_ENABLE_DEPRECATION_WARNINGS  __pragma(warning(pop))
#    elif defined(_MSC_VER)
#        define FF_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:4996))
#        define FF_ENABLE_DEPRECATION_WARNINGS  __pragma(warning(pop))
#    else
#        define FF_DISABLE_DEPRECATION_WARNINGS _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#        define FF_ENABLE_DEPRECATION_WARNINGS  _Pragma("GCC diagnostic pop")
#    endif
#else
#    define FF_DISABLE_DEPRECATION_WARNINGS
#    define FF_ENABLE_DEPRECATION_WARNINGS
#endif

#if FF_API_PSEUDOPAL
#define FF_PSEUDOPAL AV_PIX_FMT_FLAG_PSEUDOPAL
#else
#define FF_PSEUDOPAL 0
#endif

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define av_builtin_constant_p(x) 0
#define av_printf_format(fmtpos, attrpos)
#define AV_CEIL_RSHIFT(a,b) (!av_builtin_constant_p(b) ? -((-(a)) >> (b)) \
						       : ((a)+(1 << (b)) - 1) >> (b))

#define FFDIFFSIGN(x,y) (((x)>(y)) - ((x)<(y)))

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)

#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)

#define AV_STRINGIFY(s)         AV_TOSTRING(s)
#define AV_TOSTRING(s) #s

#define av_assert0(cond) do {                                           \
	if (!(cond)) { \
		fprintf(stderr, "Assertion %s failed at %s:%d\n", \
		AV_STRINGIFY(cond), __FILE__, __LINE__);                 \
		abort();                                                        \
	}                                                                   \
} while (0)

#define av_assert1(cond) ((void)0)
#define av_assert2(cond) ((void)0)
#define av_assert2_fpu() ((void)0)

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#if EDOM > 0
#define AVERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define AVUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define AVERROR(e) (e)
#define AVUNERROR(e) (e)
#endif

#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

static inline int av_toupper(int c)
{
	if (c >= 'a' && c <= 'z')
		c ^= 0x20;
	return c;
}

#define AV_LOG_QUIET			-8
#define AV_LOG_TRACE			56
#define AV_LOG_MAX_OFFSET		(AV_LOG_TRACE - AV_LOG_QUIET)
#define AV_LOG_INFO			32
#define AV_LOG_DEBUG			48

#define CONFIG_FTRAPV 0

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define HAVE_GETSYSTEMTIMEASFILETIME 1

#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
//#define   X_OK    1       /* execute permission - unsupported in windows*/
#define F_OK    0       /* Test for existence.  */

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#define srandom srand
#define random rand

#define inline __inline
typedef int mode_t;
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#define HAVE_UNISTD_H 1

#include <intrin.h>
#define AV_READ_TIME __rdtsc

#define AV_INPUT_BUFFER_PADDING_SIZE 64

#define ERROR_POS \
	fprintf(stderr, "Error, It should not execute to this position: file: %s, func: %s, line: %d\n", __FILE__, __FUNCTION__, __LINE__); \
	abort();

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_FFMPEG_COMMON_HPP_
