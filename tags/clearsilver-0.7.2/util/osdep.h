
#ifndef __OSDEP_H__
#define __OSDEP_H__ 1


#ifdef __WINDOWS_GCC__

#include <stdlib.h>
#include <stdarg.h>

#define __BEGIN_DECLS
#define __END_DECLS
#define _POSIX_PATH_MAX 255

#define	S_IXGRP         S_IXUSR
#define	S_IWGRP         S_IWUSR
#define	S_IRGRP         S_IRUSR

#define HAVE_STDARG_H 1
#define HAVE_STRING_H 1

int snprintf (char *str, size_t count, const char *fmt, ...);
int vsnprintf (char *str, size_t count, const char *fmt, va_list arg);

int mkstemp(char *path);

#define os_random rand

#else // __WINDOWS_GCC__

#define os_random random

#endif 



#endif // __OSDEP_H__
