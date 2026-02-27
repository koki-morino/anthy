#ifndef __win32_pwd_h_included__
#define __win32_pwd_h_included__

#include <string.h>
#include <windows.h>

#ifndef _WIN32_GETUID_DEFINED
#define _WIN32_GETUID_DEFINED
typedef unsigned int uid_t;
static inline uid_t
getuid(void)
{
  return 0;
}
#endif

struct passwd {
  char *pw_name;
  char *pw_passwd;
  uid_t pw_uid;
  uid_t pw_gid;
  char *pw_gecos;
  char *pw_dir;
  char *pw_shell;
};

static inline struct passwd *
getpwuid(uid_t uid)
{
  static struct passwd pw;
  static char name[256];
  static char dir[MAX_PATH];
  DWORD sz;

  (void)uid;

  sz = sizeof(name);
  if (!GetUserNameA(name, &sz))
    name[0] = '\0';

  sz = sizeof(dir);
  if (!GetEnvironmentVariableA("USERPROFILE", dir, sz)) {
    if (!GetEnvironmentVariableA("HOMEDRIVE", dir, sz))
      dir[0] = '\0';
    else {
      char drive[MAX_PATH];
      strncpy(drive, dir, sizeof(drive) - 1);
      drive[sizeof(drive) - 1] = '\0';
      GetEnvironmentVariableA("HOMEPATH", dir, sz);
      /* prepend drive */
      memmove(dir + strlen(drive), dir, strlen(dir) + 1);
      memcpy(dir, drive, strlen(drive));
    }
  }

  pw.pw_name = name;
  pw.pw_passwd = (char *)"";
  pw.pw_uid = 0;
  pw.pw_gid = 0;
  pw.pw_gecos = name;
  pw.pw_dir = dir;
  pw.pw_shell = (char *)"";

  return &pw;
}

#endif
