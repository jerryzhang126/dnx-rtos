/*=========================================================================*//**
@file    mknod.c

@author  Daniel Zorychta

@brief   Create device node file

@note    Copyright (C) 2015 Daniel Zorychta <daniel.zorychta@gmail.com>

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

/*==============================================================================
  Include files
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local object definitions
==============================================================================*/
GLOBAL_VARIABLES_SECTION {
};

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/
//==============================================================================
/**
 * @brief Program main function
 */
//==============================================================================
int_main(mknod, STACK_DEPTH_LOW, int argc, char *argv[])
{
        if (argc < 5) {
                printf("%s <file> <module name> <major> <minor>\n", argv[0]);
                return EXIT_SUCCESS;
        }

        int major = atoi(argv[3]);
        int minor = atoi(argv[4]);

        if (mknod(argv[1], argv[2], major, minor) != 0) {
                perror(argv[1]);
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}

/*==============================================================================
  End of file
==============================================================================*/
