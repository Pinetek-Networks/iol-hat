
#ifndef IOLMASTER_EXPORT_H
#define IOLMASTER_EXPORT_H

#ifdef IOLMASTER_STATIC_DEFINE
#  define IOLMASTER_EXPORT
#  define IOLMASTER_NO_EXPORT
#else
#  ifndef IOLMASTER_EXPORT
#    ifdef iolmaster_EXPORTS
        /* We are building this library */
#      define IOLMASTER_EXPORT 
#    else
        /* We are using this library */
#      define IOLMASTER_EXPORT 
#    endif
#  endif

#  ifndef IOLMASTER_NO_EXPORT
#    define IOLMASTER_NO_EXPORT 
#  endif
#endif

#ifndef IOLMASTER_DEPRECATED
#  define IOLMASTER_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef IOLMASTER_DEPRECATED_EXPORT
#  define IOLMASTER_DEPRECATED_EXPORT IOLMASTER_EXPORT IOLMASTER_DEPRECATED
#endif

#ifndef IOLMASTER_DEPRECATED_NO_EXPORT
#  define IOLMASTER_DEPRECATED_NO_EXPORT IOLMASTER_NO_EXPORT IOLMASTER_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef IOLMASTER_NO_DEPRECATED
#    define IOLMASTER_NO_DEPRECATED
#  endif
#endif

#endif /* IOLMASTER_EXPORT_H */
