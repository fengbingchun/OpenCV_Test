// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

#ifndef FBC_CV_AVDICTIONARY_HPP_
#define FBC_CV_AVDICTIONARY_HPP_

// reference: ffmpeg 4.2
//            libavutil/dict.h

#ifdef _MSC_VER

#include "core/fbcdef.hpp"

namespace fbc {

#define AV_DICT_MATCH_CASE      1   /**< Only get an entry with exact-case key match. Only relevant in av_dict_get(). */
#define AV_DICT_IGNORE_SUFFIX   2   /**< Return first entry in a dictionary whose first part corresponds to the search key,
					ignoring the suffix of the found key string. Only relevant in av_dict_get(). */
#define AV_DICT_DONT_STRDUP_KEY 4   /**< Take ownership of a key that's been
					allocated with av_malloc() or another memory allocation function. */
#define AV_DICT_DONT_STRDUP_VAL 8   /**< Take ownership of a value that's been
					allocated with av_malloc() or another memory allocation function. */
#define AV_DICT_DONT_OVERWRITE 16   ///< Don't overwrite existing entries.
#define AV_DICT_APPEND         32   /**< If the entry already exists, append to it.  Note that no
					delimiter is added, the strings are simply concatenated. */
#define AV_DICT_MULTIKEY       64   /**< Allow to store several equal keys in the dictionary */

typedef struct AVDictionaryEntry {
	char *key;
	char *value;
} AVDictionaryEntry;

struct AVDictionary {
	int count;
	AVDictionaryEntry *elems;
};

FBC_EXPORTS void av_dict_free(AVDictionary **m);
FBC_EXPORTS int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags);
AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags);
int av_dict_copy(AVDictionary **dst, const AVDictionary *src, int flags);

} // namespace fbc

#endif // _MSC_VER
#endif // FBC_CV_AVDICTIONARY_HPP_
