#ifndef DNXFS_H_
#define DNXFS_H_
/*=========================================================================*//**
@file    dnxfs.h

@author  Daniel Zorychta

@brief   This function provide all required function needed to write file systems.

@note    Copyright (C) 2012 Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the  Free Software  Foundation;  either version 2 of the License, or
         any later version.

         This  program  is  distributed  in the hope that  it will be useful,
         but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         You  should  have received a copy  of the GNU General Public License
         along  with  this  program;  if not,  write  to  the  Free  Software
         Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


*//*==========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "core/systypes.h"
#include "core/sysmoni.h"
#include "core/vfs.h"
#include "kernel/kwrapper.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
#ifdef SYSTEM_H_
#error "dnx.h and dnxfs.h cannot never included together!"
#endif

#ifndef calloc
#define calloc(nmemb, msize)              sysm_syscalloc(nmemb, msize)
#endif

#ifndef malloc
#define malloc(size)                      sysm_sysmalloc(size)
#endif

#ifndef free
#define free(mem)                         sysm_sysfree(mem)
#endif

#define mknod(path, drv_cfgPtr)           vfs_mknod(path, drv_cfgPtr)
#define mkdir(path)                       vfs_mkdir(path)
#define opendir(path)                     vfs_opendir(path)
#define closedir(dir)                     vfs_closedir(dir)
#define readdir(dir)                      vfs_readdir(dir)
#define remove(path)                      vfs_remove(path)
#define rename(oldName, newName)          vfs_rename(oldName, newName)
#define chmod(path, mode)                 vfs_chmod(path, mode)
#define chown(path, owner, group)         vfs_chown(path, owner, group)
#define stat(path, statPtr)               vfs_stat(path, statPtr)
#define statfs(path, statfsPtr)           vfs_statfs(path, statfsPtr)
#define fopen(path, mode)                 vfs_fopen(path, mode)
#define fclose(file)                      vfs_fclose(file)
#define fwrite(ptr, isize, nitems, file)  vfs_fwrite(ptr, isize, nitems, file)
#define fread(ptr, isize, nitems, file)   vfs_fread(ptr, isize, nitems, file)
#define fseek(file, offset, mode)         vfs_fseek(file, offset, mode)
#define ftell(file)                       vfs_ftell(file)
#define ioctl(file, rq, data)             vfs_ioctl(file, rq, data)
#define fstat(file, statPtr)              vfs_fstat(file, stat)
#define fflush(file)                      vfs_fflush(file)

#define FILE_SYSTEM_INTERFACE(fsname)                                                \
extern stdret_t fsname##_init   (void**, const char*);                               \
extern stdret_t fsname##_release(void*);                                             \
extern stdret_t fsname##_open   (void*, fd_t*, size_t*, const char*, const char*);   \
extern stdret_t fsname##_close  (void*, fd_t);                                       \
extern size_t   fsname##_write  (void*, fd_t, void*, size_t, size_t, size_t);        \
extern size_t   fsname##_read   (void*, fd_t, void*, size_t, size_t, size_t);        \
extern stdret_t fsname##_ioctl  (void*, fd_t, iorq_t, va_list);                      \
extern stdret_t fsname##_mkdir  (void*, const char*);                                \
extern stdret_t fsname##_mknod  (void*, const char*, struct vfs_drv_interface*);     \
extern stdret_t fsname##_opendir(void*, const char*, dir_t*);                        \
extern stdret_t fsname##_remove (void*, const char*);                                \
extern stdret_t fsname##_rename (void*, const char*, const char*);                   \
extern stdret_t fsname##_chmod  (void*, const char*, u32_t);                         \
extern stdret_t fsname##_chown  (void*, const char*, u16_t, u16_t);                  \
extern stdret_t fsname##_stat   (void*, const char*, struct vfs_statf*);             \
extern stdret_t fsname##_fstat  (void*, fd_t, struct vfs_statf*);                    \
extern stdret_t fsname##_flush  (void*, fd_t);                                       \
extern stdret_t fsname##_statfs (void*, struct vfs_statfs*)

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
  Exported function prototypes
==============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* DNXFS_H_ */
/*==============================================================================
  End of file
==============================================================================*/