// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVOPTION_HPP_
#define FBC_CV_AVOPTION_HPP_

// reference: ffmpeg 4.2
//            libavutil/opt.h

#ifdef _MSC_VER

#include "avrational.hpp"

namespace fbc {

enum AVOptionType{
	AV_OPT_TYPE_FLAGS,
	AV_OPT_TYPE_INT,
	AV_OPT_TYPE_INT64,
	AV_OPT_TYPE_DOUBLE,
	AV_OPT_TYPE_FLOAT,
	AV_OPT_TYPE_STRING,
	AV_OPT_TYPE_RATIONAL,
	AV_OPT_TYPE_BINARY,  ///< offset must point to a pointer immediately followed by an int for the length
	AV_OPT_TYPE_DICT,
	AV_OPT_TYPE_UINT64,
	AV_OPT_TYPE_CONST,
	AV_OPT_TYPE_IMAGE_SIZE, ///< offset must point to two consecutive integers
	AV_OPT_TYPE_PIXEL_FMT,
	AV_OPT_TYPE_SAMPLE_FMT,
	AV_OPT_TYPE_VIDEO_RATE, ///< offset must point to AVRational
	AV_OPT_TYPE_DURATION,
	AV_OPT_TYPE_COLOR,
	AV_OPT_TYPE_CHANNEL_LAYOUT,
	AV_OPT_TYPE_BOOL,
};

typedef struct AVOption {
	const char *name;

	/**
	* short English help text
	* @todo What about other languages?
	*/
	const char *help;

	/**
	* The offset relative to the context structure where the option
	* value is stored. It should be 0 for named constants.
	*/
	int offset;
	enum AVOptionType type;

	/**
	* the default value for scalar options
	*/
	typedef struct default_val_ {
		int64_t i64;
		double dbl;
		const char *str;
		/* TODO those are unused now */
		AVRational q;
	} default_val_;
	default_val_ default_val;
	double min;                 ///< minimum valid value for the option
	double max;                 ///< maximum valid value for the option

	int flags;
#define AV_OPT_FLAG_ENCODING_PARAM  1   ///< a generic parameter which can be set by the user for muxing or encoding
#define AV_OPT_FLAG_DECODING_PARAM  2   ///< a generic parameter which can be set by the user for demuxing or decoding
#define AV_OPT_FLAG_AUDIO_PARAM     8
#define AV_OPT_FLAG_VIDEO_PARAM     16
#define AV_OPT_FLAG_SUBTITLE_PARAM  32
	/**
	* The option is intended for exporting values to the caller.
	*/
#define AV_OPT_FLAG_EXPORT          64
	/**
	* The option may not be set through the AVOptions API, only read.
	* This flag only makes sense when AV_OPT_FLAG_EXPORT is also set.
	*/
#define AV_OPT_FLAG_READONLY        128
#define AV_OPT_FLAG_BSF_PARAM       (1<<8) ///< a generic parameter which can be set by the user for bit stream filtering
#define AV_OPT_FLAG_RUNTIME_PARAM   (1<<15) ///< a generic parameter which can be set by the user at runtime
#define AV_OPT_FLAG_FILTERING_PARAM (1<<16) ///< a generic parameter which can be set by the user for filtering
#define AV_OPT_FLAG_DEPRECATED      (1<<17) ///< set if option is deprecated, users should refer to AVOption.help text for more information
	//FIXME think about enc-audio, ... style flags

	/**
	* The logical unit to which the option belongs. Non-constant
	* options and corresponding named constants share the same
	* unit. May be NULL.
	*/
	const char *unit;
} AVOption;

#define AV_OPT_SEARCH_CHILDREN			(1 << 0)
#define AV_OPT_SEARCH_FAKE_OBJ			(1 << 1)

void av_opt_set_defaults2(void *s, int mask, int flags);
const AVOption *av_opt_next(const void *obj, const AVOption *prev);
void av_opt_set_defaults(void *s);
int av_opt_set(void *obj, const char *name, const char *val, int search_flags);
const AVOption *av_opt_find2(void *obj, const char *name, const char *unit, int opt_flags, int search_flags, void **target_obj);
const AVOption *av_opt_find(void *obj, const char *name, const char *unit, int opt_flags, int search_flags);
//const AVClass *av_opt_child_class_next(const AVClass *parent, const AVClass *prev);
void *av_opt_child_next(void *obj, void *prev);
void av_opt_free(void *obj);
int av_opt_set_dict(void *obj, struct AVDictionary **options);
int av_opt_set_dict2(void *obj, struct AVDictionary **options, int search_flags);
int av_opt_get_dict_val(void *obj, const char *name, int search_flags, AVDictionary **out_val);
int av_opt_set_dict_val(void *obj, const char *name, const AVDictionary *val, int search_flags);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVOPTION_HPP_
