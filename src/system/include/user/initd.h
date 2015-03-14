/*=========================================================================*//**
@file    initd.h

@author  Daniel Zorychta

@brief   This file contain initialize and runtime daemon

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

#ifndef _INITD_H_
#define _INITD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
/** initd stack size */
#define INITD_STACK_DEPTH               __OS_INITD_STACK_DEPTH__

/** initd name */
#define INITD_NAME                      "initd"

/** priority */
#define INITD_PRIORITY                  0

/** arguments */
#define INITD_ARGS                      NULL

#define printk                          _printk
#define printk_enable                   _printk_enable
#define printk_disable                  _printk_disable
#define detect_kernel_panic             _sysm_kernel_panic_detect

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
  Exported function prototypes
==============================================================================*/
extern void initd(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* _INITD_H_ */
/*==============================================================================
  End of file
==============================================================================*/
