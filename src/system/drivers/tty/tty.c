/*=========================================================================*//**
@file    tty.c

@author  Daniel Zorychta

@brief   This file support virtual terminal.

@note    Copyright (C) 2012, 2013 Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include "system/dnxmodule.h"
#include "system/thread.h"
#include "system/dnx.h"
#include "tty_cfg.h"
#include "tty_def.h"
#include "tty.h"
#include <unistd.h>

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define SERVICE_IN_NAME                         "tty-in"
#define SERVICE_OUT_NAME                        "tty-out"
#define SERVICE_IN_STACK_DEPTH                  STACK_DEPTH_VERY_LOW /* - 20 */
#define SERVICE_OUT_STACK_DEPTH                 STACK_DEPTH_VERY_LOW /* - 23 */
#define SERVICE_IN_PRIORITY                     0
#define SERVICE_IN_PRIORITY                     0

#define QUEUE_CMD_LEN                           16





#define BLOCK_TIME                              10000

#define VT100_RESET_ATTRIBUTES                  "\e[0m"
#define VT100_CLEAR_SCREEN                      "\e[2J"
#define VT100_DISABLE_LINE_WRAP                 "\e[7l"
#define VT100_CURSOR_HOME                       "\e[H"
#define VT100_CURSOR_OFF                        "\e[?25l"
#define VT100_CURSOR_ON                         "\e[?25h"
#define VT100_CARRIAGE_RETURN                   "\r"
#define VT100_ERASE_LINE_FROM_CUR               "\e[K"
#define VT100_SAVE_CURSOR_POSITION              "\e7"
#define VT100_SET_CURSOR_POSITION(r, c)         "\e["#r";"#c"H"
#define VT100_QUERY_CURSOR_POSITION             "\e[6n"
#define VT100_RESTORE_CURSOR_POSITION           "\e8"
#define VT100_CURSOR_ON                         "\e[?25h"
#define VT100_SHIFT_CURSOR_RIGHT(t)             "\e["#t"C"

#ifndef EOF
#define EOF                                     (-1)
#endif

#ifndef ETX
#define ETX                                     0x03
#endif

#ifndef EOT
#define EOT                                     0x04
#endif

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
enum cmd {
        CMD_INPUT,
        CMD_SWITCH_TTY,
        CMD_CLEAR_TTY
};

typedef struct {
        enum cmd        cmd : 8;
        u8_t            arg;
} tty_cmd_t;

typedef struct {
       queue_t         *queue_out;
       mutex_t         *secure_mtx;
       tty_buffer_t    *screen;
       tty_editline_t  *editline;
       u8_t             major;
} tty_t;

struct module {
        FILE           *iofile;
        task_t         *service_in;
        task_t         *service_out;
        queue_t        *queue_cmd;
        tty_t          *tty[_TTY_NUMBER];
        int             current_tty;
        u16_t           vt100_width;
        u16_t           vt100_height;
};

/*==============================================================================
  Local function prototypes
==============================================================================*/
static void service_out (void *arg);
static void service_in  (void *arg);

/*==============================================================================
  Local object definitions
==============================================================================*/
struct module *tty_module;

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Initialize device
 *
 * @param[out]          **device_handle        device allocated memory
 * @param[in ]            major                major device number
 * @param[in ]            minor                minor device number
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_INIT(TTY, void **device_handle, u8_t major, u8_t minor)
{
        UNUSED_ARG(minor);
        STOP_IF(device_handle == NULL);

        if (major >= _TTY_NUMBER || minor != 0) {
                errno = EINVAL;
                return STD_RET_ERROR;
        }

        /* initialize module base */
        if (!tty_module) {
                tty_module = calloc(1 ,sizeof(struct module));
                if (!tty_module)
                        return STD_RET_ERROR;

                tty_module->iofile      = vfs_fopen(_TTY_IO_FILE, "r+");
                tty_module->service_in  = task_new(service_in, SERVICE_IN_NAME, SERVICE_IN_STACK_DEPTH, NULL);
                tty_module->service_out = task_new(service_out, SERVICE_OUT_NAME, SERVICE_OUT_STACK_DEPTH, NULL);
                tty_module->queue_cmd   = queue_new(QUEUE_CMD_LEN, sizeof(tty_cmd_t));

                if (  !tty_module->iofile     || !tty_module->queue_cmd
                   || !tty_module->service_in || !tty_module->service_out) {

                        if (tty_module->iofile)
                                vfs_fclose(tty_module->iofile);

                        if (tty_module->queue_cmd)
                                queue_delete(tty_module->queue_cmd);

                        if (tty_module->service_in)
                                task_delete(tty_module->service_in);

                        if (tty_module->service_out)
                                task_delete(tty_module->service_out);

                        free(tty_module);
                        tty_module = NULL;

                        return STD_RET_ERROR;
                } else {
                        tty_module->vt100_width  = _TTY_DEFAULT_TERMINAL_WIDTH;
                        tty_module->vt100_height = _TTY_DEFAULT_TERMINAL_HEIGHT;
                }
        }

        /* initialize selected TTY */
        tty_t *tty = calloc(1, sizeof(tty_t));
        if (tty) {
                tty->queue_out  = queue_new(_TTY_STREAM_SIZE, sizeof(char));
                tty->secure_mtx = mutex_new(MUTEX_NORMAL);
                tty->screen     = ttybfr_new();
                tty->editline   = ttyedit_new(tty_module->iofile);

                if (tty->queue_out && tty->secure_mtx && tty->screen && tty->editline) {

                        tty->major             = major;
                        tty_module->tty[major] = tty;
                        *device_handle         = tty;

                        return STD_RET_OK;
                } else {
                        if (tty->secure_mtx)
                                mutex_delete(tty->secure_mtx);

                        if (tty->queue_out)
                                queue_delete(tty->queue_out);

                        if (tty->screen)
                                ttybfr_delete(tty->screen);

                        if (tty->editline)
                                ttyedit_delete(tty->editline);

                        free(tty);
                }
        }

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Release device
 *
 * @param[in ]          *device_handle          device allocated memory
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_RELEASE(TTY, void *device_handle)
{
        STOP_IF(!device_handle);

        tty_t *tty = device_handle;

        if (mutex_lock(tty->secure_mtx, 0)) {
                critical_section_begin();
                mutex_delete(tty->secure_mtx);
                queue_delete(tty->queue_out);
                ttybfr_delete(tty->screen);
                ttyedit_delete(tty->editline);
                tty_module->tty[tty->major] = NULL;
                free(tty);
                critical_section_end();

                return STD_RET_OK;
        } else {
                return STD_RET_ERROR;
        }
}

//==============================================================================
/**
 * @brief Open device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           flags                  file operation flags (O_RDONLY, O_WRONLY, O_RDWR)
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_OPEN(TTY, void *device_handle, int flags)
{
        UNUSED_ARG(flags);

        STOP_IF(device_handle == NULL);

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Close device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           force                  device force close (true)
 * @param[in ]          *opened_by_task         task with opened this device (valid only if force is true)
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_CLOSE(TTY, void *device_handle, bool force, const task_t *opened_by_task)
{
        UNUSED_ARG(force);
        UNUSED_ARG(opened_by_task);

        STOP_IF(device_handle == NULL);

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Write data to device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]          *src                    data source
 * @param[in ]           count                  number of bytes to write
 * @param[in ][out]     *fpos                   file position
 *
 * @return number of written bytes, -1 if error
 */
//==============================================================================
API_MOD_WRITE(TTY, void *device_handle, const u8_t *src, size_t count, u64_t *fpos)
{
        UNUSED_ARG(fpos);

        STOP_IF(device_handle == NULL);
        STOP_IF(src == NULL);
        STOP_IF(count == 0);

        return 0;
}

//==============================================================================
/**
 * @brief Read data from device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[out]          *dst                    data destination
 * @param[in ]           count                  number of bytes to read
 * @param[in ][out]     *fpos                   file position
 *
 * @return number of read bytes, -1 if error
 */
//==============================================================================
API_MOD_READ(TTY, void *device_handle, u8_t *dst, size_t count, u64_t *fpos)
{
        UNUSED_ARG(fpos);

        STOP_IF(device_handle == NULL);
        STOP_IF(dst == NULL);
        STOP_IF(count == 0);

        return 0;
}

//==============================================================================
/**
 * @brief IO control
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           request                request
 * @param[in ][out]     *arg                    request's argument
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_IOCTL(TTY, void *device_handle, int request, void *arg)
{
        STOP_IF(device_handle == NULL);

        tty_t *tty = device_handle;

        switch (request) {
        case TTY_IORQ_GET_CURRENT_TTY:
                if (arg) {
                        *(int *)arg = tty_module->current_tty;
                } else {
                        errno = EINVAL;
                        return STD_RET_ERROR;
                }
                break;

        case TTY_IORQ_SWITCH_TTY_TO:
                break;

        case TTY_IORQ_GET_COL:
                if (arg) {
                        *(int *)arg = tty_module->vt100_width;
                } else {
                        errno = EINVAL;
                        return STD_RET_ERROR;
                }
                break;

        case TTY_IORQ_GET_ROW:
                if (arg) {
                        *(int *)arg = tty_module->vt100_height;
                } else {
                        errno = EINVAL;
                        return STD_RET_ERROR;
                }
                break;

        case TTY_IORQ_CLEAR_SCR:
                break;

        case TTY_IORQ_ECHO_ON:
                ttyedit_echo_enable(tty->editline);
                break;

        case TTY_IORQ_ECHO_OFF:
                ttyedit_echo_disable(tty->editline);
                break;

        default:
                errno = EBADRQC;
                return STD_RET_ERROR;
        }

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Flush device
 *
 * @param[in ]          *device_handle          device allocated memory
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_FLUSH(TTY, void *device_handle)
{
        STOP_IF(device_handle == NULL);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Device information
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[out]          *device_stat            device status
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
API_MOD_STAT(TTY, void *device_handle, struct vfs_dev_stat *device_stat)
{
        STOP_IF(device_handle == NULL);
        STOP_IF(device_stat == NULL);

        tty_t *tty = device_handle;

        device_stat->st_size  = 0;
        device_stat->st_major = tty->major;
        device_stat->st_minor = _TTY_MINOR_NUMBER;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief TTY input service (helper task)
 */
//==============================================================================
static void service_in(void *arg)
{
        UNUSED_ARG(arg);

        tty_cmd_t cmd;
        cmd.cmd = CMD_INPUT;

        for (;;) {
                if (vfs_fread(&cmd.arg, 1, 1, tty_module->iofile) > 0) {
                        queue_send(tty_module->queue_cmd, &cmd, MAX_DELAY);
                }
        }
}

//==============================================================================
/**
 * @brief TTY output service (main task)
 */
//==============================================================================
static void service_out(void *arg)
{
        UNUSED_ARG(arg);

        for (;;) {

        }
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
