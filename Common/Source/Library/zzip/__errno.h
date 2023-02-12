#ifndef __ZZIP_INTERNAL_ERRNO_H
#define __ZZIP_INTERNAL_ERRNO_H

#include <errno.h>

/* Mingw cross compile fix */
#ifndef EBADMSG
#define EBADMSG 74
#endif

#ifndef ENAMETOOLONG
#define ENAMETOOLONG 38
#endif

#ifndef EFBIG
#define EFBIG 27
#endif

#ifndef EFAULT
#define EFAULT 14
#endif

#ifndef EPIPE
#define EPIPE 32
#endif

#ifndef ENOTDIR
#define ENOTDIR 20
#endif

#ifndef ESPIPE
#define ESPIPE 29
#endif

#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EEXIST
#define EEXIST 17
#endif

#ifndef EMFILE
#define EMFILE 24
#endif

#ifndef ENOEXEC
#define ENOEXEC 8
#endif

#ifndef EIO
#define EIO 5
#endif

#ifndef EACCES
#define EACCES 13
#endif

#endif
