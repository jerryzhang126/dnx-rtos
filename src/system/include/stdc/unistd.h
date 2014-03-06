/*=========================================================================*//**
@file    unistd.h

@author  Daniel Zorychta

@brief   Unix standard library.

@note    Copyright (C) 2013 Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifndef _UNISTD_H_
#define _UNISTD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include <string.h>
#include "kernel/kwrapper.h"

/*==============================================================================
  Exported macros
==============================================================================*/

/*==============================================================================
  Exported object types
==============================================================================*/

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  Exported functions
==============================================================================*/

/*==============================================================================
  Exported inline functions
==============================================================================*/
//==============================================================================
/**
 * @brief void sleep(const uint seconds)
 * The sleep() makes the calling thread sleep until seconds <i>seconds</i>
 * have elapsed.
 *
 * @param seconds   number of seconds to sleep
 *
 * @errors None
 *
 * @return None
 *
 * @example
 * // ...
 * sleep(2);
 * // code here will be executed after 1s sleep
 * // ...
 */
//==============================================================================
static inline void sleep(const uint seconds)
{
        _sleep(seconds);
}

//==============================================================================
/**
 * @brief void sleep_ms(const uint milliseconds)
 * The sleep_ms() makes the calling thread sleep until milliseconds
 * <i>milliseconds</i> have elapsed.
 *
 * @param milliseconds      number of milliseconds to sleep
 *
 * @errors None
 *
 * @return None
 *
 * @example
 * // ...
 * sleep_ms(10);
 * // code here will be executed after 10ms sleep
 * // ...
 */
//==============================================================================
static inline void sleep_ms(const uint milliseconds)
{
        _sleep_ms(milliseconds);
}

//==============================================================================
/**
 * @brief void usleep(const uint microseconds)
 * The usleep() makes the calling thread sleep until microseconds
 * <i>microseconds</i> have elapsed.<p>
 *
 * Function is not supported by dnx RTOS. Function sleep thread at least 1ms.
 *
 * @param microseconds      number of microseconds to sleep
 *
 * @errors None
 *
 * @return None
 *
 * @example
 * // ...
 * usleep(10);
 * // code here will be executed after 1ms sleep (shall be 10us)
 * // ...
 */
//==============================================================================
static inline void usleep(const uint microseconds)
{
        (void) microseconds;

        vTaskDelay(1);
}

//==============================================================================
/**
 * @brief void sleep_until_ms(const uint milliseconds, int *ref_time_ticks)
 * The sleep_until_ms() makes the calling thread sleep until milliseconds
 * <i>milliseconds</i> have elapsed. Function produces more precise delay.
 *
 * @param microseconds      number of microseconds to sleep
 *
 * @errors None
 *
 * @return None
 *
 * @example
 * #include <dnx/os.h>
 * #include <unistd.h>
 *
 * // ...
 * int ref_time = get_tick_counter();
 *
 * for (;;) {
 *         // ...
 *
 *         sleep_until_ms(10, &ref_time);
 * }
 * // ...
 */
//==============================================================================
static inline void sleep_until_ms(const uint milliseconds, int *ref_time_ticks)
{
        _sleep_until_ms(milliseconds, ref_time_ticks);
}

//==============================================================================
/**
 * @brief void sleep_until(const uint seconds, int *ref_time_ticks)
 * The sleep_until() makes the calling thread sleep until seconds
 * <i>seconds</i> have elapsed. Function produces more precise delay.
 *
 * @param seconds       number of seconds to sleep
 *
 * @errors None
 *
 * @return None
 *
 * @example
 * #include <dnx/os.h>
 * #include <unistd.h>
 *
 * // ...
 * int ref_time = get_tick_counter();
 *
 * for (;;) {
 *         // ...
 *
 *         sleep_until(1, &ref_time);
 * }
 * // ...
 */
//==============================================================================
static inline void sleep_until(const uint seconds, int *ref_time_ticks)
{
        _sleep_until(seconds, ref_time_ticks);
}

//==============================================================================
/**
 * @brief char *getcwd(char *buf, size_t size)
 * The getcwd() function copies an absolute pathname of the current
 * working directory to the array pointed to by <i>buf</i>, which is of length
 * <i>size</i>.
 *
 * @param buf       buffer to store path
 * @param size      buffer length
 *
 * @errors None
 *
 * @return On success, these functions return a pointer to a string containing
 * the pathname of the current working directory. In the case getcwd() is the
 * same value as <i>buf</i>.
 *
 * @example
 * #include <unistd.h>
 *
 * // ...
 * char *buf[100];
 * getcwd(buf, 100);
 * // ...
 */
//==============================================================================
static inline char *getcwd(char *buf, size_t size)
{
        return strncpy(buf, _task_get_data()->f_cwd, size);
}

//==============================================================================
/**
 * @brief Changes file owner and group
 *
 * @param[in]   path    file path
 * @param[in]   owner   owner
 * @param[in]   group   group
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
//==============================================================================
/**
 * @brief int chown(const char *pathname, int owner, int group)
 * The chown() changes the ownership of the file specified by <i>pathname</i>.<p>
 *
 * This function is not supported by dnx RTOS, because users and groups are
 * not implemented yet.
 *
 * @param pathname      path to file
 * @param owner         owner ID
 * @param group         group ID
 *
 * @errors EINVAL, ENOENT, ...
 *
 * @return On success, zero is returned. On error, -1 is returned, and
 * <b>errno</b> is set appropriately.
 *
 * @example
 * #include <unistd.h>
 *
 * // ...
 * chown("/foo/bar", 1000, 1000);
 * // ...
 */
//==============================================================================
static inline int chown(const char *pathname, int owner, int group)
{
        return vfs_chown(pathname, owner, group);
}

#ifdef __cplusplus
}
#endif

#endif /* _UNISTD_H_ */
/*==============================================================================
  End of file
==============================================================================*/
