#ifndef __win32_fnctl_h_included__
#define __win32_fnctl_h_included__

/* Include the real MinGW fcntl.h first */
#include_next <fcntl.h>

#include <errno.h>
#include <io.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <windows.h>

#ifndef F_RDLCK
#define F_RDLCK 0
#endif
#ifndef F_WRLCK
#define F_WRLCK 1
#endif
#ifndef F_UNLCK
#define F_UNLCK 2
#endif

#ifndef F_GETLK
#define F_GETLK 5
#endif
#ifndef F_SETLK
#define F_SETLK 6
#endif
#ifndef F_SETLKW
#define F_SETLKW 7
#endif

struct flock {
  short l_type;
  short l_whence;
  off_t l_start;
  off_t l_len;
  int l_pid;
};

static inline int
fcntl(int fd, int cmd, ...)
{
  HANDLE hFile;
  OVERLAPPED ov;
  struct flock *lck;
  va_list ap;

  if (cmd != F_SETLK && cmd != F_SETLKW && cmd != F_GETLK)
    return 0;

  va_start(ap, cmd);
  lck = va_arg(ap, struct flock *);
  va_end(ap);

  if (!lck)
    return -1;

  hFile = (HANDLE)_get_osfhandle(fd);
  if (hFile == INVALID_HANDLE_VALUE) {
    errno = EBADF;
    return -1;
  }

  memset(&ov, 0, sizeof(ov));

  if (lck->l_type == F_UNLCK) {
    if (!UnlockFileEx(hFile, 0, 1, 0, &ov)) {
      errno = EACCES;
      return -1;
    }
    return 0;
  }

  {
    DWORD flags = LOCKFILE_EXCLUSIVE_LOCK;
    if (cmd == F_SETLK)
      flags |= LOCKFILE_FAIL_IMMEDIATELY;
    if (!LockFileEx(hFile, flags, 0, 1, 0, &ov)) {
      errno = (cmd == F_SETLK) ? EACCES : EINTR;
      return -1;
    }
  }
  return 0;
}

#endif
