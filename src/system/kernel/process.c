/*=========================================================================*//**
@file    progman.c

@author  Daniel Zorychta

@brief   This file support programs layer

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

/*==============================================================================
  Include files
==============================================================================*/
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "fs/vfs.h"
#include "kernel/process.h"
#include "kernel/kwrapper.h"
#include "kernel/kpanic.h"
#include "kernel/printk.h"
#include "lib/llist.h"
#include "lib/cast.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define USERSPACE
#define KERNELSPACE
#define foreach_process_resources(_v, _l)       for (res_header_t *_v = _l; _v; _v = _v->next)
#define foreach_process(_v)                     for (process_t *_v = process_list; _v; _v = reinterpret_cast(process_t*, _v->header.next))

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
typedef struct process {
        res_header_t     header;                /* resource header                    */
        task_t          *task;                  /* pointer to task handle             */
        FILE            *f_stdin;               /* stdin file                         */
        FILE            *f_stdout;              /* stdout file                        */
        FILE            *f_stderr;              /* stderr file                        */
        void            *globals;               /* address to global variables        */
        res_header_t    *res_list;              /* list of used resources             */
        const char      *cwd;                   /* current working path               */
        char            **argv;                 /* program arguments                  */
        int              argc;                  /* number of arguments                */
        int              status;                /* program status (return value)      */
        pid_t            pid;                   /* process ID                         */
        int              errnov;                /* program error number               */
        u32_t            timecnt;               /* counter used to calculate CPU load */
} process_t;

typedef struct thread {
        res_header_t     header;                /* resource header                    */
        task_t          *task;                  /* pointer to task handle             */
        process_t       *process;               /* reference to process               */
} thread_t;

/*==============================================================================
  Local function prototypes
==============================================================================*/
static void process_code(void *arg);
static process_t *task_get_process_container(task_t *taskhdl);
static void process_destroy_all_resources(process_t *proc);
static bool resource_destroy(res_header_t *resource);
static int  argtab_create(const char *str, int *argc, char **argv[]);
static void argtab_destroy(char **argv);
static int  find_program(const char *name, const struct _prog_data **prog);
static int  allocate_process_globals(process_t *proc, const struct _prog_data *usrprog);
static int  apply_process_attributes(process_t *proc, const process_attr_t *attr);

/*==============================================================================
  Local object definitions
==============================================================================*/
static pid_t      PID_cnt = 1;
static process_t *process_list;
static process_t *active_process;

/*==============================================================================
  Exported object definitions
==============================================================================*/
/* standard input */
FILE *stdin;

/* standard output */
FILE *stdout;

/* standard error */
FILE *stderr;

/* error number */
int _errno;

/* global variables */
struct _GVAR_STRUCT_NAME *global;

/*==============================================================================
  External object definitions
==============================================================================*/
extern const struct _prog_data _prog_table[];
extern const int               _prog_table_size;

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief  Create a new process
 *
 * @param[out] pid      PID of created process (can be NULL)
 * @param[in]  attr     process attributes (use NULL for default attributes)
 * @param[in]  cmd      command (name + arguments)
 *
 * @return One of errno value.
 */
//==============================================================================
KERNELSPACE int _process_create(pid_t *pid, const process_attr_t *attr, const char *cmd)
{
        int result = EINVAL;

        if (cmd && cmd[0] != '\0') {
                process_t *proc;
                result = _kzalloc(_MM_KRN, sizeof(process_t), static_cast(void**, &proc));
                if (result == ESUCC) {

                        // parse program's arguments
                        result = argtab_create(cmd, &proc->argc, &proc->argv);
                        if (result != ESUCC)
                                goto finish;

                        // find program
                        const struct _prog_data *usrprog;
                        result = find_program(proc->argv[0], &usrprog);
                        if (result != ESUCC)
                                goto finish;

                        // allocate program's global variables
                        result = allocate_process_globals(proc, usrprog);
                        if (result != ESUCC)
                                goto finish;

                        // set configured program settings
                        result = apply_process_attributes(proc, attr);
                        if (result != ESUCC)
                                goto finish;

                        // set default program settings
                        proc->pid = PID_cnt++;

                        // create program's task
                        result = _task_create(process_code,
                                              usrprog->name,
                                              *usrprog->stack_depth,
                                              usrprog->main,
                                              proc,
                                              &proc->task);

                        if (result == ESUCC) {
                                if (pid) {
                                        *pid = proc->pid;
                                }

                                _critical_section_begin();
                                {
                                        if (process_list == NULL) {
                                                process_list = proc;
                                        } else {
                                                res_header_t *next = process_list->header.next;
                                                process_list       = proc;
                                                proc->header.next  = next;
                                        }
                                }
                                _critical_section_end();

                                proc->header.type = RES_TYPE_PROCESS;
                        }

                        finish:
                        if (result != ESUCC) {
                                process_destroy_all_resources(proc);
                                _kfree(_MM_KRN, static_cast(void**, &proc));
                        }
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief  Free all process's resources and remove from process list
 *
 * @param  pid          process ID
 * @param  status       process exit status (can be NULL)
 *
 * @return One of errno value
 */
//==============================================================================
KERNELSPACE int _process_destroy(pid_t pid, int *status)
{
        int result = EINVAL;

        process_t *prev = NULL;
        foreach_process(curr) {
                if (curr->pid == pid) {
                        _critical_section_begin();
                        {
                                if (process_list == curr) {
                                        process_list = static_cast(process_t*, curr->header.next);
                                } else {
                                        prev->header.next = curr->header.next;
                                }
                        }
                        _critical_section_end();

                        if (status) {
                                *status = curr->status;
                        }

                        process_destroy_all_resources(curr);

                        _kfree(_MM_KRN, static_cast(void**, &curr));

                        result = ESUCC;
                }

                prev = curr;
        }

        return result;
}

//==============================================================================
/**
 * @brief  Clear all process resources, set exit code and close task. Process is
 *         not removed from process list.
 *
 * @param  taskhdl      task handle (contain PID)
 * @param  status       status (exit code)
 *
 * @return One of errno value
 */
//==============================================================================
KERNELSPACE int _process_exit(task_t *taskhdl, int status)
{
        int result = EINVAL;

        if (taskhdl) {
                process_t *proc = task_get_process_container(taskhdl);
                if (proc) {
                        proc->status = status;
                        process_destroy_all_resources(proc);
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief  Works almost the same as _process_exit() but set exit code to -1 and
 *         print on process's terminal suitable message.
 *
 * @param  pid          process ID
 *
 * @return One of errno value
 */
//==============================================================================
KERNELSPACE int _process_abort(task_t *taskhdl)
{
        process_t *proc = task_get_process_container(taskhdl);
        if (proc) {
                const char *aborted = "Aborted\n";
                size_t      wrcnt;
                _vfs_fwrite(aborted, strlen(aborted), &wrcnt, proc->f_stderr);
        }

        return _process_exit(taskhdl, -1);
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE const char *_process_get_CWD()
{
        const char *cwd = "";

        _critical_section_begin();
        {
                if (active_process && active_process->cwd) {
                        cwd = active_process->cwd;
                }
        }
        _critical_section_end();

        return cwd;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE int _process_register_resource(task_t *task, res_header_t *resource)
{
        int result = ESRCH;

        process_t *proc = task_get_process_container(task);
        if (proc) {
                _critical_section_begin();
                {
                        if (proc->res_list == NULL) {
                                proc->res_list = resource;
                        } else {
                                res_header_t *next   = proc->res_list;
                                proc->res_list       = resource;
                                proc->res_list->next = next;
                        }
                }
                _critical_section_end();
        }

        return result;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE int _process_release_resource(task_t *task, res_header_t *resource, res_type_t type)
{
        int result = ESRCH;

        process_t *proc = task_get_process_container(task);
        if (proc) {
                _critical_section_begin();

                result = ENOENT;

                res_header_t *prev     = NULL;
                int           max_deep = 1024;

                foreach_process_resources(curr, proc->res_list) {
                        if (curr == resource) {
                                if (curr->type == type) {
                                        if (proc->res_list == curr) {
                                                proc->res_list = proc->res_list->next;
                                        } else {
                                                prev->next = curr->next;
                                        }

                                        if (resource_destroy(curr) == false) {
                                                _kernel_panic_report(_KERNEL_PANIC_DESC_CAUSE_INTERNAL);
                                        }
                                } else {
                                        result = EFAULT;
                                        break;
                                }
                        }

                        prev = curr;

                        if (--max_deep == 0) {
                                _kernel_panic_report(_KERNEL_PANIC_DESC_CAUSE_INTERNAL);
                        }
                }

                _critical_section_end();
        }

        return result;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE int _process_get_statistics(task_t *task, process_stat_t *stat)
{
        int result = EINVAL;

        process_t *proc = task_get_process_container(task);

        if (proc && stat) {
                memset(stat, 0, sizeof(process_stat_t));

                stat->pid           = proc->pid;
                stat->cpu_load_cnt  = proc->timecnt;
                stat->threads_count = 1;

                foreach_process_resources(res, proc->res_list) {
                        switch (res->type) {
                        case RES_TYPE_FILE      : stat->files_count++; break;
                        case RES_TYPE_DIR       : stat->dir_count++; break;
                        case RES_TYPE_MUTEX     : stat->mutexes_count++; break;
                        case RES_TYPE_QUEUE     : stat->queue_count++; break;
                        case RES_TYPE_SEMAPHORE : stat->semaphores_count++; break;
                        case RES_TYPE_THREAD    : stat->threads_count++; break;
                        case RES_TYPE_MEMORY    : stat->memory_block_count++;
                                                  stat->memory_usage += _mm_get_block_size(res);
                                                  break;
                        default: break;
                        }
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE FILE *_process_get_stderr(task_t *task)
{
        FILE *f_stderr = NULL;

        process_t *proc = task_get_process_container(task);
        if (proc) {
                f_stderr = proc->f_stderr;
        }

        return f_stderr;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE int _thread_create()
{
        return ENOMEM;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
KERNELSPACE int _thread_destroy(thread_t *thread)
{
        return EINVAL;
}

//==============================================================================
/**
 * @brief Function copy task context to standard variables (stdin, stdout, stderr,
 *        global, errno)
 */
//==============================================================================
KERNELSPACE void _copy_task_context_to_standard_variables(void)
{
        active_process = _task_get_tag(_THIS_TASK);

        if (active_process->header.type == RES_TYPE_THREAD) {
                active_process = reinterpret_cast(thread_t*, active_process)->process;
        }

        if (active_process) {
                stdin  = active_process->f_stdin;
                stdout = active_process->f_stdout;
                stderr = active_process->f_stderr;
                global = active_process->globals;
                _errno = active_process->errnov;
        } else {
                stdin  = NULL;
                stdout = NULL;
                stderr = NULL;
                global = NULL;
                _errno = 0;
        }
}

//==============================================================================
/**
 * @brief Function copy standard variables (stdin, stdout, stderr, global, errno)
 *        to task context
 */
//==============================================================================
KERNELSPACE void _copy_standard_variables_to_task_context(void)
{
        if (active_process) {
                active_process->f_stdin  = stdin;
                active_process->f_stdout = stdout;
                active_process->f_stderr = stderr;
                active_process->globals  = global;
                active_process->errnov   = _errno;
        }
}

//==============================================================================
/**
 * @brief  This function start user code [USERLAND]
 *
 * @param  arg          process argument - process main function
 *
 * @return None
 */
//==============================================================================
USERSPACE static void process_code(void *arg)
{
        process_func_t funcmain = arg;
        process_t     *proc     = _task_get_tag(_THIS_TASK);

        proc->status = funcmain(proc->argc, proc->argv);

        syscall(SYSCALL_EXIT, &proc->status);

        _task_exit();
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
static process_t *task_get_process_container(task_t *taskhdl)
{
        process_t *process = _task_get_tag(taskhdl);

        if (process->header.type == RES_TYPE_THREAD) {
                process = reinterpret_cast(thread_t*, process)->process;
        }

        return process;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
static void process_destroy_all_resources(process_t *proc)
{
        if (proc->task) {
                _task_destroy(proc->task);
                proc->task = NULL;
        }

        if (proc->argv) {
                argtab_destroy(proc->argv);
                proc->argv = NULL;
                proc->argc = 0;
        }

        // suspend all threads
        foreach_process_resources(res, proc->res_list) {
                if (res->type == RES_TYPE_THREAD) {
                        _task_suspend(reinterpret_cast(thread_t*, res)->task);
                }
        }

        // free all resources
        foreach_process_resources(res, proc->res_list) {
                if (resource_destroy(res) == false) {
                        _printk("Unknown object: %p\n", res);
                }
        }

        proc->f_stdin  = NULL;
        proc->f_stdout = NULL;
        proc->f_stderr = NULL;
        proc->globals  = NULL;
        proc->cwd      = NULL;

}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
static bool resource_destroy(res_header_t *resource)
{
        res_header_t *res2free = resource;

        switch (resource->type) {
        case RES_TYPE_FILE:
                _vfs_fclose(static_cast(FILE*, res2free), true);
                break;

        case RES_TYPE_DIR:
                _vfs_closedir(static_cast(DIR*, res2free));
                break;

        case RES_TYPE_MEMORY:
                _kfree(_MM_PROG, static_cast(void*, &res2free));
                break;

        case RES_TYPE_MUTEX:
                _mutex_destroy(static_cast(mutex_t*, res2free));
                break;

        case RES_TYPE_QUEUE:
                _queue_destroy(static_cast(queue_t*, res2free));
                break;

        case RES_TYPE_SEMAPHORE:
                _semaphore_destroy(static_cast(sem_t*, res2free));
                break;

        case RES_TYPE_THREAD:
                _thread_destroy(static_cast(thread_t*, res2free));
                break;

        default:
                return false;
        }

        return true;
}

//==============================================================================
/**
 * @brief Function create new table with argument pointers
 *
 * @param[in]  str              argument string
 * @param[out] argc             number of argument
 * @param[out] argv             pointer to pointer of argument array
 *
 * @return One of errno value.
 */
//==============================================================================
static int argtab_create(const char *str, int *argc, char **argv[])
{
        int result = EINVAL;

        if (str && argc && argv) {

                llist_t *largs;
                result = _llist_create_krn(_MM_KRN, NULL, NULL, &largs);

                if (result == ESUCC) {
                        // parse arguments
                        while (*str != '\0') {
                                // skip spaces
                                str += strspn(str, " ");

                                // select character to find as end of argument
                                bool quo = false;
                                char find = ' ';
                                if (*str == '\'' || *str == '"') {
                                        quo = true;
                                        find = *str;
                                        str++;
                                }

                                // find selected character
                                const char *start = str;
                                const char *end   = strchr(str, find);

                                // check if string end is reached
                                if (!end) {
                                        end = strchr(str, '\0');
                                } else {
                                        end++;
                                }

                                // calculate argument length (without nul character)
                                int str_len = end - start;
                                if (str_len == 0)
                                        break;

                                if (quo || *(end - 1) == ' ') {
                                        str_len--;
                                }

                                // add argument to list
                                char *arg;
                                result = _kmalloc(_MM_KRN, str_len + 1, static_cast(void**, &arg));
                                if (result == ESUCC) {
                                        strncpy(arg, start, str_len);
                                        arg[str_len] = '\0';

                                        if (_llist_push_back(largs, arg) == NULL) {
                                                goto finish;
                                        }
                                } else {
                                        goto finish;
                                }

                                // next token
                                str = end;
                        }

                        // create table with arguments
                        int no_of_args = _llist_size(largs);
                        *argc = no_of_args;

                        char **arg = NULL;
                        result = _kmalloc(_MM_KRN, (no_of_args + 1) * sizeof(char*), static_cast(void*, &arg));
                        if (result == ESUCC) {
                                for (int i = 0; i < no_of_args; i++) {
                                        arg[i] = _llist_take_front(largs);
                                }

                                arg[no_of_args] = NULL;
                                *argv = arg;
                        }

                        finish:
                        _llist_destroy(largs);
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief Function remove argument table
 *
 * @param argv          pointer to argument table (must be ended with NULL)
 */
//==============================================================================
static void argtab_destroy(char **argv)
{
        if (argv) {
                int n = 0;
                while (argv[n]) {
                        _kfree(_MM_KRN, static_cast(void*, &argv[n]));
                        n++;
                }

                _kfree(_MM_KRN, static_cast(void*, &argv));
        }
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
static int find_program(const char *name, const struct _prog_data **prog)
{
        static const size_t kworker_stack_depth  = STACK_DEPTH_LOW;
        static const size_t kworker_globals_size = 0;
        static const struct _prog_data kworker   = {.globals_size = &kworker_globals_size,
                                                    .main         = _syscall_kworker_master,
                                                    .name         = "kworker",
                                                    .stack_depth  = &kworker_stack_depth};

        int result = ENOENT;

        if (strncmp(name, "kworker", 32) == 0) {
                *prog  = &kworker;
                result = ESUCC;

        } else {
                for (int i = 0; i < _prog_table_size; i++) {
                        if (strncmp(_prog_table[i].name, name, 128) == 0) {
                                *prog  = &_prog_table[i];
                                result = ESUCC;
                                break;
                        }
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
static int allocate_process_globals(process_t *proc, const struct _prog_data *usrprog)
{
        int result = ESUCC;

        if (*usrprog->globals_size > 0) {
                res_header_t *mem;
                result = _kzalloc(_MM_PROG, *usrprog->globals_size, static_cast(void*, &mem));

                if (result == ESUCC) {
                        proc->globals = &mem[1];
                        _process_register_resource(proc->task, mem);
                }
        }

        return result;
}

//==============================================================================
/**
 * @brief  ?
 * @param  ?
 * @return ?
 */
//==============================================================================
static int apply_process_attributes(process_t *proc, const process_attr_t *attr)
{
        int result = ESUCC;

        if (attr) {
                /*
                 * Apply stdin settings
                 * - if f_stdin is set then function use this resource as reference
                 * - if f_stdin is NULL and p_stdin is NULL then function set stdin as NULL
                 * - if f_stdin is NULL and p_stdin is valid then function open new file and use as stdin
                 */
                if (attr->f_stdin) {
                        proc->f_stdin = attr->f_stdin;

                } else if (attr->p_stdin) {
                        result = _vfs_fopen(attr->p_stdin, "a+", &proc->f_stdin);
                        if (result == ESUCC) {
                                _process_register_resource(proc->task, static_cast(res_header_t*, proc->f_stdin));
                        } else {
                                goto finish;
                        }
                }

                /*
                 * Apply stdout settings
                 * - if f_stdout is set then function use this resource as reference
                 * - if f_stdout is NULL and p_stdout is NULL then function set stdout as NULL
                 * - if f_stdout is NULL and p_stdout is valid then function open new file and use as stdout
                 * - if p_stdout is the same as p_stdin then function use stdin as reference of stdout
                 */
                if (attr->f_stdout) {
                        proc->f_stdout = attr->f_stdout;

                } else if (attr->p_stdout) {
                        if (strcmp(attr->p_stdout, attr->p_stdin) == 0) {
                                proc->f_stdout = proc->f_stdin;

                        } else {
                                result = _vfs_fopen(attr->p_stdout, "a", &proc->f_stdout);
                                if (result == ESUCC) {
                                        _process_register_resource(proc->task, static_cast(res_header_t*, proc->f_stdout));
                                } else {
                                        goto finish;
                                }
                        }
                }

                /*
                 * Apply stderr settings
                 * - if f_stderr is set then function use this resource as reference
                 * - if f_stderr is NULL and p_stderr is NULL then function set stderr as NULL
                 * - if f_stderr is NULL and p_stderr is valid then function open new file and use as stderr
                 * - if p_stderr is the same as p_stdin then function use stdin as reference of stderr
                 * - if p_stderr is the same as p_stdout then function use stdout as reference of stderr
                 */
                if (attr->f_stderr) {
                        proc->f_stderr = attr->f_stderr;

                } else if (attr->p_stderr) {
                        if (strcmp(attr->p_stderr, attr->p_stdin) == 0) {
                                proc->f_stderr = proc->f_stdin;

                        } else if (strcmp(attr->p_stderr, attr->p_stdout) == 0) {
                                proc->f_stderr = proc->f_stdout;

                        } else {
                                result = _vfs_fopen(attr->p_stderr, "a", &proc->f_stderr);
                                if (result == ESUCC) {
                                        _process_register_resource(proc->task, static_cast(res_header_t*, proc->f_stderr));
                                } else {
                                        goto finish;
                                }
                        }
                }

                /*
                 * Apply Current Working Directory path
                 */
                proc->cwd = attr->cwd;
        }

        finish:
        return result;
}

/*==============================================================================
  End of file
==============================================================================*/
