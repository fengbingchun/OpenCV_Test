// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVCLASS_HPP_
#define FBC_CV_AVCLASS_HPP_

// reference: ffmpeg 4.2
//            libavutil/log.h

#ifdef _MSC_VER

namespace fbc {

typedef enum {
	AV_CLASS_CATEGORY_NA = 0,
	AV_CLASS_CATEGORY_INPUT,
	AV_CLASS_CATEGORY_OUTPUT,
	AV_CLASS_CATEGORY_MUXER,
	AV_CLASS_CATEGORY_DEMUXER,
	AV_CLASS_CATEGORY_ENCODER,
	AV_CLASS_CATEGORY_DECODER,
	AV_CLASS_CATEGORY_FILTER,
	AV_CLASS_CATEGORY_BITSTREAM_FILTER,
	AV_CLASS_CATEGORY_SWSCALER,
	AV_CLASS_CATEGORY_SWRESAMPLER,
	AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT = 40,
	AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
	AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
	AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT,
	AV_CLASS_CATEGORY_DEVICE_OUTPUT,
	AV_CLASS_CATEGORY_DEVICE_INPUT,
	AV_CLASS_CATEGORY_NB  ///< not part of ABI/API
} AVClassCategory;

typedef struct AVClass {
	/**
	* The name of the class; usually it is the same name as the
	* context structure type to which the AVClass is associated.
	*/
	const char* class_name;

	/**
	* A pointer to a function which returns the name of a context
	* instance ctx associated with the class.
	*/
	const char* (*item_name)(void* ctx);

	/**
	* a pointer to the first option specified in the class if any or NULL
	*
	* @see av_set_default_options()
	*/
	const struct AVOption *option;

	/**
	* LIBAVUTIL_VERSION with which this structure was created.
	* This is used to allow fields to be added without requiring major
	* version bumps everywhere.
	*/

	int version;

	/**
	* Offset in the structure where log_level_offset is stored.
	* 0 means there is no such variable
	*/
	int log_level_offset_offset;

	/**
	* Offset in the structure where a pointer to the parent context for
	* logging is stored. For example a decoder could pass its AVCodecContext
	* to eval as such a parent context, which an av_log() implementation
	* could then leverage to display the parent context.
	* The offset can be NULL.
	*/
	int parent_log_context_offset;

	/**
	* Return next AVOptions-enabled child or NULL
	*/
	void* (*child_next)(void *obj, void *prev);

	/**
	* Return an AVClass corresponding to the next potential
	* AVOptions-enabled child.
	*
	* The difference between child_next and this is that
	* child_next iterates over _already existing_ objects, while
	* child_class_next iterates over _all possible_ children.
	*/
	const struct AVClass* (*child_class_next)(const struct AVClass *prev);

	/**
	* Category used for visualization (like color)
	* This is only set if the category is equal for all objects using this class.
	* available since version (51 << 16 | 56 << 8 | 100)
	*/
	AVClassCategory category;

	/**
	* Callback to return the category.
	* available since version (51 << 16 | 59 << 8 | 100)
	*/
	AVClassCategory(*get_category)(void* ctx);

	/**
	* Callback to return the supported/allowed ranges.
	* available since version (52.12)
	*/
	int(*query_ranges)(struct AVOptionRanges **, void *obj, const char *key, int flags);
} AVClass;

const char* av_default_item_name(void* ctx);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVCLASS_HPP_
