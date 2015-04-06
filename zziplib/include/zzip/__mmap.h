#ifndef __ZZIP_INTERNAL_MMAP_H
#define __ZZIP_INTERNAL_MMAP_H
#include <zzip/types.h>

/*
 * DO NOT USE THIS CODE.
 *
 * It is an internal header file for zziplib that carries some inline
 * functions (or just static members) and a few defines, simply to be
 * able to reuse these across - and have everything in a specific place.
 *
 * Copyright (c) 2002,2003 Guido Draheim
 *          All rights reserved,
 *          use under the restrictions of the 
 *          Lesser GNU General Public License
 *          or alternatively the restrictions 
 *          of the Mozilla Public License 1.1
 */

#ifdef _USE_MMAP
#if    defined ZZIP_HAVE_SYS_MMAN_H
#include <sys/mman.h>
#define USE_POSIX_MMAP 1
#elif defined ZZIP_HAVE_WINBASE_H || defined WIN32
#include <windows.h>
#define USE_WIN32_MMAP 1
#else
#undef _USE_MMAP
#endif
#endif

/* -------------- specify MMAP function imports --------------------------- */

#if     defined  USE_POSIX_MMAP && ZZIP_TEST2
#define USE_MMAP 1

#define ZZIP_MAPHANDLE_NULL 0
#define zzip_maphandle_t int

static void* posix_mmap(zzip_maphandle_t* maphandle, int fd, zzip_off_t offs, size_t len)
{
    return mmap (0, len, PROT_READ, MAP_SHARED, fd, offs);
}

static void posix_munmap (zzip_maphandle_t* maphandle, char* fd_map, size_t len)
{
    munmap(fd_map, len);
}

#define _zzip_mmap(maphandle, fd, offs, len) \
        posix_mmap((zzip_maphandle_t*)&maphandle, fd, offs, len)
#define _zzip_munmap(maphandle, ptr, len) \
        posix_munmap ((zzip_maphandle_t*)&maphandle, ptr, len)
#define _zzip_getpagesize() getpagesize()

#ifndef MAP_FAILED /* hpux10.20 does not have it */
#define MAP_FAILED ((void*)(-1))
#endif

#elif   defined  USE_POSIX_MMAP
#define USE_MMAP 1

#define ZZIP_MAPHANDLE_NULL 0
#define zzip_maphandle_t int
#define _zzip_mmap(maphandle, fd, offs, len) \
              mmap (0, len, PROT_READ, MAP_SHARED, fd, offs)
#define _zzip_munmap(maphandle, ptr, len) \
              munmap (ptr, len)
#define _zzip_getpagesize() getpagesize()

#ifndef MAP_FAILED /* hpux10.20 does not have it */
#define MAP_FAILED ((void*)(-1))
#endif

#elif   defined USE_WIN32_MMAP
#define USE_MMAP 1
#ifndef MAP_FAILED
#define MAP_FAILED 0
#endif
/* we (ab)use the "*user" variable to store the FileMapping handle */
                 /* which assumes (sizeof(long) == sizeof(HANDLE)) */

#define ZZIP_MAPHANDLE_NULL NULL
#define zzip_maphandle_t HANDLE
static size_t win32_getpagesize (void)
{ 
    SYSTEM_INFO si; GetSystemInfo (&si); 
    return si.dwAllocationGranularity; 
}
static void*  win32_mmap (zzip_maphandle_t* maphandle, int fd, zzip_off_t offs, size_t len)
{
    if (! maphandle || *maphandle != ZZIP_MAPHANDLE_NULL) /* || offs % getpagesize() */
        return MAP_FAILED;
  {
    HANDLE hFile = (HANDLE)_get_osfhandle(fd);
    if (hFile)
        *maphandle = CreateFileMapping (hFile, 0, PAGE_READONLY, 0, 0, NULL);
    if (*maphandle)
    {
        char* p = 0;
        p = MapViewOfFile(*maphandle, FILE_MAP_READ, 0, offs, len);
        if (p) return p + offs;
        CloseHandle (*maphandle); *maphandle = ZZIP_MAPHANDLE_NULL;
    } 
    return MAP_FAILED;
  }
}
static void win32_munmap (zzip_maphandle_t* maphandle, char* fd_map, size_t len)
{
    UnmapViewOfFile (fd_map);
    CloseHandle (*maphandle); *maphandle = ZZIP_MAPHANDLE_NULL;
}

#define _zzip_mmap(maphandle, fd, offs, len) \
        win32_mmap ((zzip_maphandle_t*)&maphandle, fd, offs, len)
#define _zzip_munmap(maphandle, ptr, len) \
        win32_munmap ((zzip_maphandle_t*)&maphandle, ptr, len)
#define _zzip_getpagesize() win32_getpagesize()

#else   /* disable */
#define USE_MMAP 0
/* USE_MMAP is intentional: we expect the compiler to do some "code removal"
 * on any source code enclosed in if (USE_MMAP) {...}   i.e. the unreachable
 * branch of an if (0) {....} is not emitted to the final object binary. */

#ifndef MAP_FAILED
#define MAP_FAILED  0
#endif

#define ZZIP_MAPHANDLE_NULL 0
#define zzip_maphandle_t int
#define _zzip_mmap(maphandle, fd, offs, len) (MAP_FAILED)
#define _zzip_munmap(maphandle, ptr, len) {}
#define _zzip_getpagesize() 1024

#endif /* USE_MMAP defines */


#endif
