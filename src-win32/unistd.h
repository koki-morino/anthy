#ifndef __win32_unistd_h_included__
#define __win32_unistd_h_included__

/* Include the real MinGW unistd.h first */
#include_next <unistd.h>

#include <direct.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32_GETUID_DEFINED
#define _WIN32_GETUID_DEFINED
typedef unsigned int uid_t;
static inline uid_t
getuid(void)
{
  return 0;
}
#endif

/* Windows has no symlinks in the POSIX sense; lstat == stat.           */
#ifndef S_ISLNK
#define S_ISLNK(m) (0)
#endif

static inline int
lstat(const char *path, struct stat *buf)
{
  return stat(path, buf);
}

static inline int
dprintf(int fd, const char *fmt, ...)
{
  va_list ap;
  char buf[4096];
  int n;

  va_start(ap, fmt);
  n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if (n > 0)
    _write(fd, buf, (unsigned int)n);
  return n;
}

/* Windows does not support per-fd permission changes; stub it out.     */
static inline int
fchmod(int fd, mode_t mode)
{
  (void)fd;
  (void)mode;
  return 0;
}

/* MinGW's mkdir only takes 1 argument; wrap it to accept mode too.     */
/* Use a macro so we don't conflict with direct.h's 1-arg declaration.  */
#ifdef mkdir
#undef mkdir
#endif
#define mkdir(path, mode) _mkdir(path)

#endif
