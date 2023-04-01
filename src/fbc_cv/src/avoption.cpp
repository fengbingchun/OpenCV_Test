// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "avdictionary.hpp"
#include "avoption.hpp"
#include "avclass.hpp"
#include "averror.hpp"
#include "avutil.hpp"
#include "avmem.hpp"
#include "ffmpeg_pixel_format.hpp"

// reference: ffmpeg 4.2
//            libavutil/opt.c

#ifdef _MSC_VER

namespace fbc {

const AVOption *av_opt_next(const void *obj, const AVOption *last)
{
	const AVClass *class_;
	if (!obj)
		return NULL;
	class_ = *(const AVClass**)obj;
	if (!last && class_ && class_->option && class_->option[0].name)
		return class_->option;
	if (last && last[1].name)
		return ++last;
	return NULL;
}

static int write_number(void *obj, const AVOption *o, void *dst, double num, int den, int64_t intnum)
{
	if (o->type != AV_OPT_TYPE_FLAGS &&
		(!den || o->max * den < num * intnum || o->min * den > num * intnum)) {
		num = den ? num * intnum / den : (num && intnum ? INFINITY : NAN);
		fprintf(stderr, "avoption: Value %f for parameter '%s' out of range [%g - %g]\n",
			num, o->name, o->min, o->max);
		return AVERROR(ERANGE);
	}
	if (o->type == AV_OPT_TYPE_FLAGS) {
		double d = num*intnum / den;
		if (d < -1.5 || d > 0xFFFFFFFF + 0.5 || (llrint(d * 256) & 255)) {
			fprintf(stderr, "avoption: Value %f for parameter '%s' is not a valid set of 32bit integer flags\n",
				num*intnum / den, o->name);
			return AVERROR(ERANGE);
		}
	}

	switch (o->type) {
	case AV_OPT_TYPE_PIXEL_FMT:
		*(enum AVPixelFormat *)dst = static_cast<AVPixelFormat>(llrint(num / den) * intnum);
		break;
	case AV_OPT_TYPE_SAMPLE_FMT:
		*(enum AVSampleFormat *)dst = static_cast<AVSampleFormat>(llrint(num / den) * intnum);
		break;
	case AV_OPT_TYPE_BOOL:
	case AV_OPT_TYPE_FLAGS:
	case AV_OPT_TYPE_INT:
		*(int *)dst = llrint(num / den) * intnum;
		break;
	case AV_OPT_TYPE_DURATION:
	case AV_OPT_TYPE_CHANNEL_LAYOUT:
	case AV_OPT_TYPE_INT64:{
		double d = num / den;
		if (intnum == 1 && d == (double)INT64_MAX) {
			*(int64_t *)dst = INT64_MAX;
		}
		else
			*(int64_t *)dst = llrint(d) * intnum;
		break; }
	case AV_OPT_TYPE_UINT64:{
		double d = num / den;
		// We must special case uint64_t here as llrint() does not support values
		// outside the int64_t range and there is no portable function which does
		// "INT64_MAX + 1ULL" is used as it is representable exactly as IEEE double
		// while INT64_MAX is not
		if (intnum == 1 && d == (double)UINT64_MAX) {
			*(uint64_t *)dst = UINT64_MAX;
		}
		else if (d > INT64_MAX + 1ULL) {
			*(uint64_t *)dst = (llrint(d - (INT64_MAX + 1ULL)) + (INT64_MAX + 1ULL))*intnum;
		}
		else {
			*(uint64_t *)dst = llrint(d) * intnum;
		}
		break; }
	case AV_OPT_TYPE_FLOAT:
		*(float *)dst = num * intnum / den;
		break;
	case AV_OPT_TYPE_DOUBLE:
		*(double    *)dst = num * intnum / den;
		break;
	case AV_OPT_TYPE_RATIONAL:
	case AV_OPT_TYPE_VIDEO_RATE:
		if ((int)num == num)
			*(AVRational *)dst = AVRational{ static_cast<int>(num *intnum), den };
		else
			*(AVRational *)dst = av_d2q(num * intnum / den, 1 << 24);
		break;
	default:
		return AVERROR(EINVAL);
	}
	return 0;
}

static int set_string_color(void *obj, const AVOption *o, const char *val, uint8_t *dst)
{
	int ret;

	if (!val) {
		return 0;
	}
	else {
		ret = av_parse_color(dst, val, -1, obj);
		if (ret < 0)
			fprintf(stderr, "avoption: Unable to parse option value \"%s\" as color\n", val);
		return ret;
	}
	return 0;
}

int set_string_image_size(void *obj, const AVOption *o, const char *val, int *dst);
int set_string(void *obj, const AVOption *o, const char *val, uint8_t **dst);
int set_string_binary(void *obj, const AVOption *o, const char *val, uint8_t **dst);

void av_opt_set_defaults2(void *s, int mask, int flags)
{
	const AVOption *opt = NULL;
	while ((opt = av_opt_next(s, opt))) {
		void *dst = ((uint8_t*)s) + opt->offset;

		if ((opt->flags & mask) != flags)
			continue;

		if (opt->flags & AV_OPT_FLAG_READONLY)
			continue;

		switch (opt->type) {
		case AV_OPT_TYPE_CONST:
			/* Nothing to be done here */
			break;
		case AV_OPT_TYPE_BOOL:
		case AV_OPT_TYPE_FLAGS:
		case AV_OPT_TYPE_INT:
		case AV_OPT_TYPE_INT64:
		case AV_OPT_TYPE_UINT64:
		case AV_OPT_TYPE_DURATION:
		case AV_OPT_TYPE_CHANNEL_LAYOUT:
		case AV_OPT_TYPE_PIXEL_FMT:
		case AV_OPT_TYPE_SAMPLE_FMT:
			write_number(s, opt, dst, 1, 1, opt->default_val.i64);
			break;
		//case AV_OPT_TYPE_DOUBLE:
		//case AV_OPT_TYPE_FLOAT: {
		//	double val;
		//	val = opt->default_val.dbl;
		//	write_number(s, opt, dst, val, 1, 1);
		//}
		//	break;
		//case AV_OPT_TYPE_RATIONAL: {
		//	AVRational val;
		//	val = av_d2q(opt->default_val.dbl, INT_MAX);
		//	write_number(s, opt, dst, 1, val.den, val.num);
		//}
		//	break;
		//case AV_OPT_TYPE_COLOR:
		//	set_string_color(s, opt, opt->default_val.str, dst);
		//	break;
		case AV_OPT_TYPE_STRING:
			set_string(s, opt, opt->default_val.str, (uint8_t**)dst);
			break;
		case AV_OPT_TYPE_IMAGE_SIZE:
			set_string_image_size(s, opt, opt->default_val.str, static_cast<int*>(dst));
			break;
		//case AV_OPT_TYPE_VIDEO_RATE:
		//	set_string_video_rate(s, opt, opt->default_val.str, dst);
		//	break;
		case AV_OPT_TYPE_BINARY:
			set_string_binary(s, opt, opt->default_val.str, (uint8_t**)dst);
			break;
		//case AV_OPT_TYPE_DICT:
		//	/* Cannot set defaults for these types */
		//	break;
		default:
			fprintf(stderr, "avoption: AVOption type %d of option %s not implemented yet\n",
				opt->type, opt->name);
		}
	}
}

void av_opt_set_defaults(void *s)
{
	av_opt_set_defaults2(s, 0, 0);
}

static int set_string_bool(void *obj, const AVOption *o, const char *val, int *dst)
{
	int n;

	if (!val)
		return 0;

	if (!strcmp(val, "auto")) {
		n = -1;
	}
	else if (av_match_name(val, "true,y,yes,enable,enabled,on")) {
		n = 1;
	}
	else if (av_match_name(val, "false,n,no,disable,disabled,off")) {
		n = 0;
	}
	else {
		char *end = NULL;
		n = strtol(val, &end, 10);
		if (val + strlen(val) != end)
			goto fail;
	}

	if (n < o->min || n > o->max)
		goto fail;

	*dst = n;
	return 0;

fail:
	fprintf(stderr, "avoption: Unable to parse option value \"%s\" as boolean\n", val);
	return AVERROR(EINVAL);
}

static int set_string(void *obj, const AVOption *o, const char *val, uint8_t **dst)
{
	av_freep(dst);
	*dst = reinterpret_cast<uint8_t*>(av_strdup(val));
	return *dst ? 0 : AVERROR(ENOMEM);
}

static int hexchar2int(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int set_string_binary(void *obj, const AVOption *o, const char *val, uint8_t **dst)
{
	int *lendst = (int *)(dst + 1);
	uint8_t *bin, *ptr;
	int len;

	av_freep(dst);
	*lendst = 0;

	if (!val || !(len = strlen(val)))
		return 0;

	if (len & 1)
		return AVERROR(EINVAL);
	len /= 2;

	ptr = bin = static_cast<uint8_t*>(av_malloc(len));
	if (!ptr)
		return AVERROR(ENOMEM);
	while (*val) {
		int a = hexchar2int(*val++);
		int b = hexchar2int(*val++);
		if (a < 0 || b < 0) {
			av_free(bin);
			return AVERROR(EINVAL);
		}
		*ptr++ = (a << 4) | b;
	}
	*dst = bin;
	*lendst = len;

	return 0;
}

static int read_number(const AVOption *o, const void *dst, double *num, int *den, int64_t *intnum)
{
	switch (o->type) {
	case AV_OPT_TYPE_FLAGS:
		*intnum = *(unsigned int*)dst;
		return 0;
	case AV_OPT_TYPE_PIXEL_FMT:
		*intnum = *(enum AVPixelFormat *)dst;
		return 0;
	case AV_OPT_TYPE_SAMPLE_FMT:
		*intnum = *(enum AVSampleFormat *)dst;
		return 0;
	case AV_OPT_TYPE_BOOL:
	case AV_OPT_TYPE_INT:
		*intnum = *(int *)dst;
		return 0;
	case AV_OPT_TYPE_CHANNEL_LAYOUT:
	case AV_OPT_TYPE_DURATION:
	case AV_OPT_TYPE_INT64:
	case AV_OPT_TYPE_UINT64:
		*intnum = *(int64_t *)dst;
		return 0;
	case AV_OPT_TYPE_FLOAT:
		*num = *(float *)dst;
		return 0;
	case AV_OPT_TYPE_DOUBLE:
		*num = *(double *)dst;
		return 0;
	case AV_OPT_TYPE_RATIONAL:
		*intnum = ((AVRational *)dst)->num;
		*den = ((AVRational *)dst)->den;
		return 0;
	case AV_OPT_TYPE_CONST:
		*num = o->default_val.dbl;
		return 0;
	}
	return AVERROR(EINVAL);
}

#define DEFAULT_NUMVAL(opt) ((opt->type == AV_OPT_TYPE_INT64 || \
			opt->type == AV_OPT_TYPE_UINT64 || \
			opt->type == AV_OPT_TYPE_CONST || \
			opt->type == AV_OPT_TYPE_FLAGS || \
			opt->type == AV_OPT_TYPE_INT)     \
			? opt->default_val.i64             \
			: opt->default_val.dbl)

static int set_string_number(void *obj, void *target_obj, const AVOption *o, const char *val, void *dst)
{
	int ret = 0;
	int num, den;
	char c;

	if (sscanf(val, "%d%*1[:/]%d%c", &num, &den, &c) == 2) {
		if ((ret = write_number(obj, o, dst, 1, den, num)) >= 0)
			return ret;
		ret = 0;
	}

	for (;;) {
		int i = 0;
		char buf[256];
		int cmd = 0;
		double d;
		int64_t intnum = 1;

		if (o->type == AV_OPT_TYPE_FLAGS) {
			if (*val == '+' || *val == '-')
				cmd = *(val++);
			for (; i < sizeof(buf)-1 && val[i] && val[i] != '+' && val[i] != '-'; i++)
				buf[i] = val[i];
			buf[i] = 0;
		}

		{
			const AVOption *o_named = av_opt_find(target_obj, i ? buf : val, o->unit, 0, 0);
			int res;
			int ci = 0;
			double const_values[64];
			const char * const_names[64];
			if (o_named && o_named->type == AV_OPT_TYPE_CONST)
				d = DEFAULT_NUMVAL(o_named);
			else {
				if (o->unit) {
					for (o_named = NULL; o_named = av_opt_next(target_obj, o_named);) {
						if (o_named->type == AV_OPT_TYPE_CONST &&
							o_named->unit &&
							!strcmp(o_named->unit, o->unit)) {
							if (ci + 6 >= FF_ARRAY_ELEMS(const_values)) {
								fprintf(stderr, "avoption: const_values array too small for %s\n", o->unit);
								return AVERROR_PATCHWELCOME;
							}
							const_names[ci] = o_named->name;
							const_values[ci++] = DEFAULT_NUMVAL(o_named);
						}
					}
				}
				const_names[ci] = "default";
				const_values[ci++] = DEFAULT_NUMVAL(o);
				const_names[ci] = "max";
				const_values[ci++] = o->max;
				const_names[ci] = "min";
				const_values[ci++] = o->min;
				const_names[ci] = "none";
				const_values[ci++] = 0;
				const_names[ci] = "all";
				const_values[ci++] = ~0;
				const_names[ci] = NULL;
				const_values[ci] = 0;

				res = av_expr_parse_and_eval(&d, i ? buf : val, const_names,
					const_values, NULL, NULL, NULL, NULL, NULL, 0, obj);
				if (res < 0) {
					fprintf(stderr, "avoption: Unable to parse option value \"%s\"\n", val);
					return res;
				}
			}
		}
		if (o->type == AV_OPT_TYPE_FLAGS) {
			read_number(o, dst, NULL, NULL, &intnum);
			if (cmd == '+')
				d = intnum | (int64_t)d;
			else if (cmd == '-')
				d = intnum &~(int64_t)d;
		}

		if ((ret = write_number(obj, o, dst, d, 1, 1)) < 0)
			return ret;
		val += i;
		if (!i || !*val)
			return 0;
	}
}

static int set_string_image_size(void *obj, const AVOption *o, const char *val, int *dst)
{
	int ret;

	if (!val || !strcmp(val, "none")) {
		dst[0] =
			dst[1] = 0;
		return 0;
	}
	ret = av_parse_video_size(dst, dst + 1, val);
	if (ret < 0)
		fprintf(stderr, "avoption: Unable to parse option value \"%s\" as image size\n", val);
	return ret;
}

static int set_string_video_rate(void *obj, const AVOption *o, const char *val, AVRational *dst)
{
	int ret;
	if (!val) {
		ret = AVERROR(EINVAL);
	}
	else {
		ret = av_parse_video_rate(dst, val);
	}
	if (ret < 0)
		fprintf(stderr, "avoption: Unable to parse option value \"%s\" as video rate\n", val);
	return ret;
}

static int set_string_fmt(void *obj, const AVOption *o, const char *val, uint8_t *dst,
	int fmt_nb, int((*get_fmt)(const char *)), const char *desc)
{
	int fmt, min, max;

	if (!val || !strcmp(val, "none")) {
		fmt = -1;
	}
	else {
		fmt = get_fmt(val);
		if (fmt == -1) {
			char *tail;
			fmt = strtol(val, &tail, 0);
			if (*tail || (unsigned)fmt >= fmt_nb) {
				fprintf(stderr, "avoption: Unable to parse option value \"%s\" as %s\n", val, desc);
				return AVERROR(EINVAL);
			}
		}
	}

	min = FFMAX(o->min, -1);
	max = FFMIN(o->max, fmt_nb - 1);

	// hack for compatibility with old ffmpeg
	if (min == 0 && max == 0) {
		min = -1;
		max = fmt_nb - 1;
	}

	if (fmt < min || fmt > max) {
		fprintf(stderr, "avoption: Value %d for parameter '%s' out of %s format range [%d - %d]\n",
			fmt, o->name, desc, min, max);
		return AVERROR(ERANGE);
	}

	*(int *)dst = fmt;
	return 0;
}

int av_opt_set(void *obj, const char *name, const char *val, int search_flags)
{
	int ret = 0;
	void *dst, *target_obj;
	const AVOption *o = av_opt_find2(obj, name, NULL, 0, search_flags, &target_obj);
	if (!o || !target_obj)
		return AVERROR_OPTION_NOT_FOUND;
	if (!val && (o->type != AV_OPT_TYPE_STRING &&
		o->type != AV_OPT_TYPE_PIXEL_FMT && o->type != AV_OPT_TYPE_SAMPLE_FMT &&
		o->type != AV_OPT_TYPE_IMAGE_SIZE && o->type != AV_OPT_TYPE_VIDEO_RATE &&
		o->type != AV_OPT_TYPE_DURATION && o->type != AV_OPT_TYPE_COLOR &&
		o->type != AV_OPT_TYPE_CHANNEL_LAYOUT && o->type != AV_OPT_TYPE_BOOL))
		return AVERROR(EINVAL);

	if (o->flags & AV_OPT_FLAG_READONLY)
		return AVERROR(EINVAL);

	if (o->flags & AV_OPT_FLAG_DEPRECATED)
		fprintf(stderr, "avoption: The \"%s\" option is deprecated: %s\n", name, o->help);

	dst = ((uint8_t *)target_obj) + o->offset;
	switch (o->type) {
	case AV_OPT_TYPE_BOOL:
		return set_string_bool(obj, o, val, static_cast<int*>(dst));
	case AV_OPT_TYPE_STRING:
		return set_string(obj, o, val, (uint8_t**)dst);
	case AV_OPT_TYPE_BINARY:
		return set_string_binary(obj, o, val, (uint8_t**)dst);
	case AV_OPT_TYPE_FLAGS:
	case AV_OPT_TYPE_INT:
	case AV_OPT_TYPE_INT64:
	case AV_OPT_TYPE_UINT64:
	case AV_OPT_TYPE_FLOAT:
	case AV_OPT_TYPE_DOUBLE:
	case AV_OPT_TYPE_RATIONAL:
		return set_string_number(obj, target_obj, o, val, dst);
	case AV_OPT_TYPE_IMAGE_SIZE:
		return set_string_image_size(obj, o, val, static_cast<int*>(dst));
	case AV_OPT_TYPE_VIDEO_RATE: {
		AVRational tmp;
		ret = set_string_video_rate(obj, o, val, &tmp);
		if (ret < 0)
			return ret;
		return write_number(obj, o, dst, 1, tmp.den, tmp.num);
	}
	//case AV_OPT_TYPE_PIXEL_FMT:
	//	return set_string_pixel_fmt(obj, o, val, dst);
	//case AV_OPT_TYPE_SAMPLE_FMT:
	//	return set_string_sample_fmt(obj, o, val, dst);
	//case AV_OPT_TYPE_DURATION:
	//{
	//	int64_t usecs = 0;
	//	if (val) {
	//		if ((ret = av_parse_time(&usecs, val, 1)) < 0) {
	//			fprintf(stderr, "avoption: Unable to parse option value \"%s\" as duration\n", val);
	//			return ret;
	//		}
	//	}
	//	if (usecs < o->min || usecs > o->max) {
	//		fprintf(stderr, "avoption: Value %f for parameter '%s' out of range [%g - %g]\n",
	//			usecs / 1000000.0, o->name, o->min / 1000000.0, o->max / 1000000.0);
	//		return AVERROR(ERANGE);
	//	}
	//	*(int64_t *)dst = usecs;
	//	return 0;
	//}
	case AV_OPT_TYPE_COLOR:
		return set_string_color(obj, o, val, static_cast<uint8_t*>(dst));
	//case AV_OPT_TYPE_CHANNEL_LAYOUT:
	//	if (!val || !strcmp(val, "none")) {
	//		*(int64_t *)dst = 0;
	//	}
	//	else {
	//		int64_t cl = av_get_channel_layout(val);
	//		if (!cl) {
	//			fprintf(stderr, "avoption: Unable to parse option value \"%s\" as channel layout\n", val);
	//			ret = AVERROR(EINVAL);
	//		}
	//		*(int64_t *)dst = cl;
	//		return ret;
	//	}
	//	break;
	}

	fprintf(stderr, "avoption: Invalid option type.\n");
	return AVERROR(EINVAL);
}

const AVOption *av_opt_find2(void *obj, const char *name, const char *unit,
	int opt_flags, int search_flags, void **target_obj)
{
	const AVClass  *c;
	const AVOption *o = NULL;

	if (!obj)
		return NULL;

	c = *(AVClass**)obj;

	if (!c)
		return NULL;

	if (search_flags & AV_OPT_SEARCH_CHILDREN) {
		if (search_flags & AV_OPT_SEARCH_FAKE_OBJ) {
			//const AVClass *child = NULL;
			//while (child = av_opt_child_class_next(c, child))
			//	if (o = av_opt_find2(&child, name, unit, opt_flags, search_flags, NULL))
			//		return o;
		}
		else {
			void *child = NULL;
			while (child = av_opt_child_next(obj, child))
				if (o = av_opt_find2(child, name, unit, opt_flags, search_flags, target_obj))
					return o;
		}
	}

	while (o = av_opt_next(obj, o)) {
		if (!strcmp(o->name, name) && (o->flags & opt_flags) == opt_flags &&
			((!unit && o->type != AV_OPT_TYPE_CONST) ||
			(unit  && o->type == AV_OPT_TYPE_CONST && o->unit && !strcmp(o->unit, unit)))) {
			if (target_obj) {
				if (!(search_flags & AV_OPT_SEARCH_FAKE_OBJ))
					*target_obj = obj;
				else
					*target_obj = NULL;
			}
			return o;
		}
	}
	return NULL;
}

const AVOption *av_opt_find(void *obj, const char *name, const char *unit,
	int opt_flags, int search_flags)
{
	return av_opt_find2(obj, name, unit, opt_flags, search_flags, NULL);
}

void *av_opt_child_next(void *obj, void *prev)
{
	const AVClass *c = *(AVClass **)obj;
	if (c->child_next)
		return c->child_next(obj, prev);
	return NULL;
}

void av_opt_free(void *obj)
{
	const AVOption *o = NULL;
	while ((o = av_opt_next(obj, o))) {
		switch (o->type) {
		case AV_OPT_TYPE_STRING:
		case AV_OPT_TYPE_BINARY:
			av_freep((uint8_t *)obj + o->offset);
			break;

		case AV_OPT_TYPE_DICT:
			av_dict_free((AVDictionary **)(((uint8_t *)obj) + o->offset));
			break;

		default:
			break;
		}
	}
}

int av_opt_set_dict(void *obj, struct AVDictionary **options)
{
	return av_opt_set_dict2(obj, options, 0);
}

int av_opt_set_dict2(void *obj, struct AVDictionary **options, int search_flags)
{
	AVDictionaryEntry *t = NULL;
	AVDictionary    *tmp = NULL;
	int ret = 0;

	if (!options)
		return 0;

	while ((t = av_dict_get(*options, "", t, AV_DICT_IGNORE_SUFFIX))) {
		ret = av_opt_set(obj, t->key, t->value, search_flags);
		if (ret == AVERROR_OPTION_NOT_FOUND)
			ret = av_dict_set(&tmp, t->key, t->value, 0);
		if (ret < 0) {
			fprintf(stderr, "obj, Error setting option %s to value %s.\n", t->key, t->value);
			av_dict_free(&tmp);
			return ret;
		}
		ret = 0;
	}
	av_dict_free(options);
	*options = tmp;
	return ret;
}

int av_opt_get_dict_val(void *obj, const char *name, int search_flags, AVDictionary **out_val)
{
	void *target_obj;
	AVDictionary *src;
	const AVOption *o = av_opt_find2(obj, name, NULL, 0, search_flags, &target_obj);

	if (!o || !target_obj)
		return AVERROR_OPTION_NOT_FOUND;
	if (o->type != AV_OPT_TYPE_DICT)
		return AVERROR(EINVAL);

	src = *(AVDictionary **)(((uint8_t *)target_obj) + o->offset);
	av_dict_copy(out_val, src, 0);

	return 0;
}

int av_opt_set_dict_val(void *obj, const char *name, const AVDictionary *val,
	int search_flags)
{
	void *target_obj;
	AVDictionary **dst;
	const AVOption *o = av_opt_find2(obj, name, NULL, 0, search_flags, &target_obj);

	if (!o || !target_obj)
		return AVERROR_OPTION_NOT_FOUND;
	if (o->flags & AV_OPT_FLAG_READONLY)
		return AVERROR(EINVAL);

	dst = (AVDictionary **)(((uint8_t *)target_obj) + o->offset);
	av_dict_free(dst);
	av_dict_copy(dst, val, 0);

	return 0;
}

} // namespace fbc

#endif // _MSC_VER
