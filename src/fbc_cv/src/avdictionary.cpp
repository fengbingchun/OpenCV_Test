// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#include "avdictionary.hpp"
#include <string.h>
#include "avmem.hpp"
#include "avutil.hpp"

// reference: ffmpeg 4.2
//            libavutil/dict.c

#ifdef _MSC_VER

namespace fbc {

void av_dict_free(AVDictionary **pm)
{
	AVDictionary *m = *pm;

	if (m) {
		while (m->count--) {
			av_freep(&m->elems[m->count].key);
			av_freep(&m->elems[m->count].value);
		}
		av_freep(&m->elems);
	}
	av_freep(pm);
}

AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key,
	const AVDictionaryEntry *prev, int flags)
{
	unsigned int i, j;

	if (!m)
		return NULL;

	if (prev)
		i = prev - m->elems + 1;
	else
		i = 0;

	for (; i < m->count; i++) {
		const char *s = m->elems[i].key;
		if (flags & AV_DICT_MATCH_CASE)
		for (j = 0; s[j] == key[j] && key[j]; j++)
			;
		else
		for (j = 0; av_toupper(s[j]) == av_toupper(key[j]) && key[j]; j++)
			;
		if (key[j])
			continue;
		if (s[j] && !(flags & AV_DICT_IGNORE_SUFFIX))
			continue;
		return &m->elems[i];
	}
	return NULL;
}

int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags)
{
	AVDictionary *m = *pm;
	AVDictionaryEntry *tag = NULL;
	char *oldval = NULL, *copy_key = NULL, *copy_value = NULL;

	if (!(flags & AV_DICT_MULTIKEY)) {
		tag = av_dict_get(m, key, NULL, flags);
	}
	if (flags & AV_DICT_DONT_STRDUP_KEY)
		copy_key = const_cast<char*>(key);
	else
		copy_key = av_strdup(key);
	if (flags & AV_DICT_DONT_STRDUP_VAL)
		copy_value = const_cast<char*>(value);
	else if (copy_key)
		copy_value = av_strdup(value);
	if (!m)
		m = *pm = static_cast<AVDictionary*>(av_mallocz(sizeof(*m)));
	if (!m || (key && !copy_key) || (value && !copy_value))
		goto err_out;

	if (tag) {
		if (flags & AV_DICT_DONT_OVERWRITE) {
			av_free(copy_key);
			av_free(copy_value);
			return 0;
		}
		if (flags & AV_DICT_APPEND)
			oldval = tag->value;
		else
			av_free(tag->value);
		av_free(tag->key);
		*tag = m->elems[--m->count];
	} else if (copy_value) {
		AVDictionaryEntry *tmp = static_cast<AVDictionaryEntry *>(av_realloc(m->elems, (m->count + 1) * sizeof(*m->elems)));
		if (!tmp)
			goto err_out;
		m->elems = tmp;
	}
	if (copy_value) {
		m->elems[m->count].key = copy_key;
		m->elems[m->count].value = copy_value;
		if (oldval && flags & AV_DICT_APPEND) {
			size_t len = strlen(oldval) + strlen(copy_value) + 1;
			char *newval = static_cast<char*>(av_mallocz(len));
			if (!newval)
				goto err_out;
			av_strlcat(newval, oldval, len);
			av_freep(&oldval);
			av_strlcat(newval, copy_value, len);
			m->elems[m->count].value = newval;
			av_freep(&copy_value);
		}
		m->count++;
	} else {
		av_freep(&copy_key);
	}
	if (!m->count) {
		av_freep(&m->elems);
		av_freep(pm);
	}

	return 0;

err_out:
	if (m && !m->count) {
		av_freep(&m->elems);
		av_freep(pm);
	}
	av_free(copy_key);
	av_free(copy_value);
	return AVERROR(ENOMEM);
}

int av_dict_copy(AVDictionary **dst, const AVDictionary *src, int flags)
{
	AVDictionaryEntry *t = NULL;

	while ((t = av_dict_get(src, "", t, AV_DICT_IGNORE_SUFFIX))) {
		int ret = av_dict_set(dst, t->key, t->value, flags);
		if (ret < 0)
			return ret;
	}

	return 0;
}

} // namespace fbc

#endif // _MSC_VER
