#ifndef __win32_sys_mman_h_included__
#define __win32_sys_mman_h_included__

#include <io.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <windows.h>

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FAILED ((void*)-1)

/* Internal structure to track mapping handles */
struct _win32_mmap_entry {
  void* addr;
  size_t length;
  HANDLE hFile;
  HANDLE hMapping;
  struct _win32_mmap_entry* next;
};

static struct _win32_mmap_entry* _win32_mmap_list = NULL;

static inline void*
mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  HANDLE hFile;
  HANDLE hMapping;
  void* ptr;
  DWORD flProtect;
  DWORD dwDesiredAccess;
  struct _win32_mmap_entry* entry;

  (void)addr;
  (void)flags;

  if (prot & PROT_WRITE) {
    flProtect = PAGE_READWRITE;
    dwDesiredAccess = FILE_MAP_WRITE;
  } else {
    flProtect = PAGE_READONLY;
    dwDesiredAccess = FILE_MAP_READ;
  }

  hFile = (HANDLE)_get_osfhandle(fd);
  if (hFile == INVALID_HANDLE_VALUE)
    return MAP_FAILED;

  hMapping = CreateFileMappingA(hFile, NULL, flProtect, 0, 0, NULL);
  if (!hMapping)
    return MAP_FAILED;

  ptr = MapViewOfFile(hMapping, dwDesiredAccess,
                      (DWORD)(((ULONGLONG)offset) >> 32),
                      (DWORD)(offset & 0xFFFFFFFF),
                      length);
  if (!ptr) {
    CloseHandle(hMapping);
    return MAP_FAILED;
  }

  entry = (struct _win32_mmap_entry*)malloc(sizeof(*entry));
  if (entry) {
    entry->addr = ptr;
    entry->length = length;
    entry->hFile = hFile;
    entry->hMapping = hMapping;
    entry->next = _win32_mmap_list;
    _win32_mmap_list = entry;
  }

  return ptr;
}

static inline int
munmap(void* addr, size_t length)
{
  struct _win32_mmap_entry** pp;
  (void)length;

  for (pp = &_win32_mmap_list; *pp; pp = &(*pp)->next) {
    if ((*pp)->addr == addr) {
      struct _win32_mmap_entry* e = *pp;
      *pp = e->next;
      UnmapViewOfFile(addr);
      CloseHandle(e->hMapping);
      free(e);
      return 0;
    }
  }
  UnmapViewOfFile(addr);
  return 0;
}

#endif
