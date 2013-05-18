#ifndef DNXMODULE_H_
#define DNXMODULE_H_
/*=========================================================================*//**
@file    dnxmodule.h

@author  Daniel Zorychta

@brief   This function provide all required function needed to write modules.

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
#include "core/vfs.h"
#include "core/sysmoni.h"
#include "user/regdrv.h"
#include "kernel/kwrapper.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
#ifdef DNX_H_
#error "dnx.h and dnxmodule.h shall never included together!"
#endif

#define MODULE_NAME(module_name)                static const char *__module_name__ = #module_name

#undef  calloc
#define calloc(nmemb, msize)                    sysm_modcalloc(nmemb, msize, regdrv_get_module_number(__module_name__))

#undef  malloc
#define malloc(size)                            sysm_modmalloc(size, regdrv_get_module_number(__module_name__))

#undef  free
#define free(mem)                               sysm_modfree(mem, regdrv_get_module_number(__module_name__))

#undef  mknod
#define mknod(path, drv_cfgPtr)                 vfs_mknod(path, drv_cfgPtr)

#undef  mkdir
#define mkdir(path)                             vfs_mkdir(path)

#undef  opendir
#define opendir(path)                           vfs_opendir(path)

#undef  closedir
#define closedir(dir)                           vfs_closedir(dir)

#undef  readdir
#define readdir(dir)                            vfs_readdir(dir)

#undef  remove
#define remove(path)                            vfs_remove(path)

#undef  rename
#define rename(oldName, newName)                vfs_rename(oldName, newName)

#undef  chmod
#define chmod(path, mode)                       vfs_chmod(path, mode)

#undef  chown
#define chown(path, owner, group)               vfs_chown(path, owner, group)

#undef  stat
#define stat(path, statPtr)                     vfs_stat(path, statPtr)

#undef  statfs
#define statfs(path, statfsPtr)                 vfs_statfs(path, statfsPtr)

#undef  fopen
#define fopen(path, mode)                       vfs_fopen(path, mode)

#undef  fclose
#define fclose(file)                            vfs_fclose(file)

#undef  fwrite
#define fwrite(ptr, isize, nitems, file)        vfs_fwrite(ptr, isize, nitems, file)

#undef  fread
#define fread(ptr, isize, nitems, file)         vfs_fread(ptr, isize, nitems, file)

#undef  fseek
#define fseek(file, offset, mode)               vfs_fseek(file, offset, mode)

#undef  ftell
#define ftell(file)                             vfs_ftell(file)

#undef  ioctl
#define ioctl(file, rq, data)                   vfs_ioctl(file, rq, data)

#undef  fstat
#define fstat(file, statPtr)                    vfs_fstat(file, stat)

#undef  fflush
#define fflush(file)                            vfs_fflush(file)

#undef  fstat
#define fstat(file, statPtr)                    vfs_fstat(file, stat)

#undef  fflush
#define fflush(file)                            vfs_fflush(file)

#undef  feof
#define feof(file)                              vfs_feof(file)

#undef  rewind
#define rewind(file)                            vfs_rewind(file)

#define DRIVER_INTERFACE(modname)                                               \
extern stdret_t modname##_init   (void**, uint, uint);                          \
extern stdret_t modname##_release(void*);                                       \
extern stdret_t modname##_open   (void*);                                       \
extern stdret_t modname##_close  (void*);                                       \
extern size_t   modname##_write  (void*, const void*, size_t, size_t, size_t);  \
extern size_t   modname##_read   (void*, void*, size_t, size_t, size_t);        \
extern stdret_t modname##_ioctl  (void*, int, va_list);                         \
extern stdret_t modname##_flush  (void*)

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

#endif /* DNXMODULE_H_ */
/*==============================================================================
  End of file
==============================================================================*/