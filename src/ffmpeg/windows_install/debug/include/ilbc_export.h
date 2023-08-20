
#ifndef ILBC_EXPORT_H
#define ILBC_EXPORT_H

#ifdef ILBC_STATIC_DEFINE
#define ILBC_EXPORT
#define ILBC_NO_EXPORT
#else
#ifndef ILBC_EXPORT
#ifdef ilbc_EXPORTS
/* We are building this library */
#define ILBC_EXPORT __declspec(dllexport)
#else
/* We are using this library */
#define ILBC_EXPORT __declspec(dllimport)
#endif
#endif

#ifndef ILBC_NO_EXPORT
#define ILBC_NO_EXPORT
#endif
#endif

#ifndef ILBC_DEPRECATED
#define ILBC_DEPRECATED __declspec(deprecated)
#endif

#ifndef ILBC_DEPRECATED_EXPORT
#define ILBC_DEPRECATED_EXPORT ILBC_EXPORT ILBC_DEPRECATED
#endif

#ifndef ILBC_DEPRECATED_NO_EXPORT
#define ILBC_DEPRECATED_NO_EXPORT ILBC_NO_EXPORT ILBC_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef ILBC_NO_DEPRECATED
#define ILBC_NO_DEPRECATED
#endif
#endif

#endif /* ILBC_EXPORT_H */
