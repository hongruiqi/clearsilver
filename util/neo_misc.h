/*
 * Neotonic ClearSilver Templating System
 *
 * This code is made available under the terms of the 
 * Neotonic ClearSilver License.
 * http://www.neotonic.com/clearsilver/license.hdf
 *
 * Copyright (C) 2001 by Brandon Long
 */

#ifndef __NEO_MISC_H_
#define __NEO_MISC_H_ 1

#include <stdlib.h>

/* Fix Up for systems that don't define these standard things... */
#ifndef __BEGIN_DECLS
#define extern "C" {
#endif

#ifndef __END_DECLS
}
#endif

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 255
#endif

#ifndef S_IXGRP
#define S_IXGRP S_IXUSR
#endif
#ifndef S_IWGRP
#define S_IWGRP S_IWUSR
#endif
#ifndef S_IRGRP
#define S_IRGRP S_IRUSR
#endif
#ifndef S_IXOTH
#define S_IXOTH S_IXUSR
#endif
#ifndef S_IWOTH
#define S_IWOTH S_IWUSR
#endif
#ifndef S_IROTH
#define S_IROTH S_IRUSR
#endif

__BEGIN_DECLS

#include <stdarg.h>
#include <sys/types.h>

typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned char UINT8;
typedef char INT8;
typedef char BOOL;

#ifndef MIN
#define MIN(x,y)        (((x) < (y)) ? (x) : (y))
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

void ne_vwarn (char *fmt, va_list ap);
void ne_warn (char *fmt, ...);
UINT32 python_string_hash (const char *s);
UINT8 *ne_stream4 (UINT8  *dest, UINT32 num);
UINT8 *ne_unstream4 (UINT32 *pnum, UINT8 *src);
UINT8 *ne_stream2 (UINT8  *dest, UINT16 num);
UINT8 *ne_unstream2 (UINT16 *pnum, UINT8 *src);
UINT8 *ne_stream_str (UINT8 *dest, char *s, int l);
UINT8 *ne_unstream_str (char *s, int l, UINT8 *src);
double ne_timef (void);
UINT32 ne_crc (UINT8 *data, UINT32 bytes);

__END_DECLS

#endif /* __NEO_MISC_H_ */
