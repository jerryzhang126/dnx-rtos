/*=========================================================================*//**
@file    time.c

@author  Daniel Zorychta

@brief   Check execution time of selected program

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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <dnx/os.h>
#include <dnx/thread.h>
#include <sys/types.h>

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define CWD_LEN         80
#define CMD_LEN         100

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
        char cwd[CWD_LEN];
        char cmd[CMD_LEN];
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
int_main(time, STACK_DEPTH_VERY_LOW, int argc, char *argv[])
{
        if (argc == 1) {
                printf("Usage: %s [program]\n", argv[0]);
                return EXIT_SUCCESS;
        }

        getcwd(global->cwd, CWD_LEN);

        for (int i = 1; i < argc; i++) {
                if (strlen(argv[i]) + strlen(global->cmd) < CMD_LEN) {
                        strcat(global->cmd, argv[i]);
                        strcat(global->cmd, " ");
                } else {
                        break;
                }
        }

        errno = 0;

        u32_t start_time = get_time_ms();

        process_attr_t attr;
        memset(&attr, 0, sizeof(attr));
        attr.cwd        = global->cwd;
        attr.f_stdin    = stdin;
        attr.f_stdout   = stdout;
        attr.f_stderr   = stderr;
        attr.has_parent = true;

        pid_t pid = process_create(global->cmd, &attr);
        if (pid) {
                process_wait(pid, NULL, MAX_DELAY_MS);
        } else {
                perror(argv[1]);
        }

        u32_t total_time = get_time_ms() - start_time;
        printf("\nreal\t%dm%d.%03ds\n", total_time / 60000, (total_time / 1000) % 60, total_time % 1000);

        return EXIT_SUCCESS;
}

/*==============================================================================
  End of file
==============================================================================*/
