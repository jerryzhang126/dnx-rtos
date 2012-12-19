#ifndef TTY_DEF_H_
#define TTY_DEF_H_
/*=============================================================================================*//**
@file    tty_def.h

@author  Daniel Zorychta

@brief   This file support global definitions of TTY

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


*//*==============================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                            Include files
==================================================================================================*/


/*==================================================================================================
                                 Exported symbolic constants/macros
==================================================================================================*/
/** define TTY max supported lines */
#define TTY_MAX_LINES               40

/** define number of virtual terminals */
#define TTY_NUMBER_OF_VT            4

/** define part count */
#define TTY_PART_NONE               0

/*==================================================================================================
                                  Exported types, enums definitions
==================================================================================================*/
/** devices number */
enum tty_list
{
      #if TTY_NUMBER_OF_VT > 0
      TTY_DEV_0,
      #endif

      #if TTY_NUMBER_OF_VT > 1
      TTY_DEV_1,
      #endif

      #if TTY_NUMBER_OF_VT > 2
      TTY_DEV_2,
      #endif

      #if TTY_NUMBER_OF_VT > 3
      TTY_DEV_3,
      #endif

      #if TTY_NUMBER_OF_VT > 4
      #error "TTY support 4 virtual terminals!"
      #endif
      TTY_LAST
};


/** TTY requests */
enum TTY_IORQ_enum
{
      TTY_IORQ_GETCURRENTTTY,                   /* [out] u8_t */
      TTY_IORQ_SETACTIVETTY,                    /* [in ] u8_t */
      TTY_IORQ_CLEARTTY,                        /* none */
      TTY_IORQ_GETCOL,                          /* [out] u8_t */
      TTY_IORQ_GETROW,                          /* [out] u8_t */
      TTY_IORQ_CLEARSCR,                        /* none */
      TTY_IORQ_CLEARLASTLINE,                   /* none */
};


/*==================================================================================================
                                     Exported object declarations
==================================================================================================*/


/*==================================================================================================
                                     Exported function prototypes
==================================================================================================*/


#ifdef __cplusplus
}
#endif

#endif /* TTY_DEF_H_ */
/*==================================================================================================
                                            End of file
==================================================================================================*/