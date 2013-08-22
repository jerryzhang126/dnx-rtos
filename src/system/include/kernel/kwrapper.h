#ifndef KWRAPPER_H_
#define KWRAPPER_H_
/*=========================================================================*//**
@file    kwrapper.h

@author  Daniel Zorychta

@brief   Kernel wrapper

@note    Copyright (C) 2012  Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include "kernel/ktypes.h"
#include "core/systypes.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "config.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
/** error handling */
#if (pdTRUE != true)
#error "pdTRUE != true"
#endif

#if (pdFALSE != false)
#error "pdFALSE != false"
#endif


/** UNDEFINE MEMORY MANAGEMENT DEFINITIONS LOCALIZED IN FreeRTOS.h file (IMPORTANT!) */
#undef free
#undef malloc

/** STANDART STACK SIZES */
#define STACK_DEPTH_MINIMAL                                             (1  * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_VERY_LOW                                            (2  * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_LOW                                                 (4  * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_MEDIUM                                              (6  * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_LARGE                                               (8  * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_VERY_LARGE                                          (10 * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_HUGE                                                (12 * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_VERY_HUGE                                           (14 * (CONFIG_RTOS_TASK_MIN_STACK_DEPTH))
#define STACK_DEPTH_USER(depth)                                         (depth)

/** OS BASIC DEFINITIONS */
#define THIS_TASK                                                       NULL
#define MAX_DELAY                                                       (portMAX_DELAY / 1000)

/** OS kernel control functions */
#define start_task_scheduler()                                          vTaskStartScheduler()

/** CALCULATIONS */
#define PRIORITY(prio)                                                  (prio + (configMAX_PRIORITIES / 2))
#define LOWEST_PRIORITY                                                 (-(int)(configMAX_PRIORITIES / 2))
#define HIGHEST_PRIORITY                                                (configMAX_PRIORITIES / 2)
#define _CEILING(x,y)                                                   (((x) + (y) - 1) / (y))
#define MS2TICK(ms)                                                     (ms <= (1000/(configTICK_RATE_HZ)) ? 1 : _CEILING(ms,(1000/(configTICK_RATE_HZ))))

/** TASK LEVEL DEFINITIONS */
#define new_task(void__pfunc, const_char__pname, uint__stack_depth, void__pargv) kwrap_new_task(void__pfunc, const_char__pname, uint__stack_depth, void__pargv)
#define delete_task(task_t__ptaskhdl)                                   kwrap_delete_task(task_t__ptaskhdl)
#define task_exit()                                                     kwrap_task_exit()
#define sleep_ms(uint__msdelay)                                         vTaskDelay(MS2TICK(uint__msdelay))
#define sleep(uint__seconds)                                            vTaskDelay(MS2TICK((uint__seconds) * 1000UL))
#define prepare_sleep_until()                                           unsigned long int __last_wake_time__ = get_tick_counter();
#define sleep_until(uint__seconds)                                      vTaskDelayUntil(&__last_wake_time__, MS2TICK((uint__seconds) * 1000UL))
#define sleep_ms_until(uint__msdelay)                                   vTaskDelayUntil(&__last_wake_time__, MS2TICK(uint__msdelay))
#define suspend_task(task_t__ptaskhdl)                                  vTaskSuspend(task_t__ptaskhdl)
#define suspend_this_task()                                             vTaskSuspend(THIS_TASK)
#define resume_task(task_t__ptaskhdl)                                   vTaskResume(task_t__ptaskhdl)
#define resume_task_from_ISR(task_t__ptaskhdl)                          xTaskResumeFromISR(task_t__ptaskhdl)
#define yield_task()                                                    taskYIELD()
#define enter_critical_section()                                        taskENTER_CRITICAL()
#define exit_critical_section()                                         taskEXIT_CRITICAL()
#define disable_ISR()                                                   taskDISABLE_INTERRUPTS()
#define enable_ISR()                                                    taskENABLE_INTERRUPTS()
#define get_tick_counter()                                              xTaskGetTickCount()
#define get_OS_time_ms()                                                (xTaskGetTickCount() * ((1000/(configTICK_RATE_HZ))))
#define get_task_name(task_t__ptaskhdl)                                 (char*)pcTaskGetTaskName(task_t__ptaskhdl)
#define get_this_task_name()                                            (char*)pcTaskGetTaskName(THIS_TASK)
#define get_task_handle()                                               xTaskGetCurrentTaskHandle()
#define get_task_priority(task_t__ptaskhdl)                             (int)(uxTaskPriorityGet(task_t__ptaskhdl) - (CONFIG_RTOS_TASK_MAX_PRIORITIES / 2))
#define set_task_priority(task_t__ptaskhdl, int__priority)              vTaskPrioritySet(task_t__ptaskhdl, PRIORITY(int__priority))
#define set_priority(int__priority)                                     vTaskPrioritySet(THIS_TASK, PRIORITY(int__priority))
#define get_priority()                                                  (int)(uxTaskPriorityGet(THIS_TASK) - (CONFIG_RTOS_TASK_MAX_PRIORITIES / 2))
#define get_free_stack()                                                uxTaskGetStackHighWaterMark(THIS_TASK)
#define get_task_free_stack(task_t__ptaskhdl)                           uxTaskGetStackHighWaterMark(task_t__ptaskhdl)
#define get_number_of_tasks()                                           uxTaskGetNumberOfTasks()
#define _set_task_tag(task_t__ptaskhdl, void__ptag)                     vTaskSetApplicationTaskTag(task_t__ptaskhdl, void__ptag)
#define _get_task_tag(task_t__ptaskhdl)                                 (void*)xTaskGetApplicationTaskTag(task_t__ptaskhdl)
#define _get_this_task_data()                                           ((struct task_data*)_get_task_tag(THIS_TASK))
#define _get_task_data(task_t__ptaskhdl)                                ((struct task_data*)_get_task_tag(task_t__ptaskhdl))
#define get_parent_handle()                                             _get_task_data(THIS_TASK)->f_parent_task
#define set_global_variables(void__pmem)                                _get_task_data(THIS_TASK)->f_global_vars = void__pmem
#define set_stdin(FILE__pfile)                                          _get_task_data(THIS_TASK)->f_stdin = FILE__pfile
#define set_stdout(FILE__pfile)                                         _get_task_data(THIS_TASK)->f_stdout = FILE__pfile
#define set_stderr(FILE__pfile)                                         _get_task_data(THIS_TASK)->f_stderr = FILE__pfile
#define set_user_data(void__pmem)                                       _get_task_data(THIS_TASK)->f_user = void__pmem
#define get_user_data()                                                 _get_task_data(THIS_TASK)->f_user
#define _set_task_monitor_data(task_t__ptaskhdl, void__pmem)            _get_task_data(task_t__ptaskhdl)->f_monitor = void__pmem
#define _get_task_monitor_data(task_t__ptaskhdl)                        _get_task_data(task_t__ptaskhdl)->f_monitor

/** SEMAPHORE */
#define new_semaphore()                                                 kwrap_create_binary_semaphore()
#define delete_semaphore(sem_t__psem)                                   vSemaphoreDelete(sem_t__psem)
#define take_semaphore(sem_t__psem, uint__blocktime_ms)                 xSemaphoreTake(sem_t__psem, MS2TICK((portTickType)(uint__blocktime_ms)))
#define give_semaphore(sem_t__psem)                                     xSemaphoreGive(sem_t__psem)
#define take_semaphore_from_ISR(sem_t__psem, int__pwoke)                xSemaphoreTakeFromISR(sem_t__psem, (signed portBASE_TYPE*) int__pwoke)
#define give_semaphore_from_ISR(sem_t__psem, int__pwoke)                xSemaphoreGiveFromISR(sem_t__psem, (signed portBASE_TYPE*) int__pwoke)

#define new_counting_semaphore(uint__cnt_max, uint__init)               xSemaphoreCreateCounting(uint__cnt_max, uint__init)
#define delete_counting_semaphore(sem_t__psem)                          vSemaphoreDelete(sem_t__psem)
#define take_counting_semaphore(sem_t__psem, uint__blocktime_ms)        xSemaphoreTake(sem_t__psem, MS2TICK((portTickType)(uint__blocktime_ms)))
#define give_counting_semaphore(sem_t__psem)                            xSemaphoreGive(sem_t__psem)
#define take_counting_semaphore_from_ISR(sem_t__psem, int__pwoke)       xSemaphoreTakeFromISR(sem_t__psem, (signed portBASE_TYPE*) int__pwoke)
#define give_counting_semaphore_from_ISR(sem_t__psem, int__pwoke)       xSemaphoreGiveFromISR(sem_t__psem, (signed portBASE_TYPE*) int__pwoke)

#define SEMAPHORE_TAKEN                                                 true

/** MUTEX */
#define new_mutex()                                                     xSemaphoreCreateMutex()
#define delete_mutex(mutex_t__pmutex)                                   vSemaphoreDelete(mutex_t__pmutex)
#define lock_mutex(mutex_t__pmutex, uint__blocktime_ms)                 xSemaphoreTake(mutex_t__pmutex, MS2TICK((portTickType)(uint__blocktime_ms)))
#define unlock_mutex(mutex_t__pmutex)                                   xSemaphoreGive(mutex_t__pmutex)

#define new_recursive_mutex()                                           xSemaphoreCreateRecursiveMutex()
#define delete_recursive_mutex(mutex_t__pmutex)                         vSemaphoreDelete(mutex_t__pmutex)
#define lock_recursive_mutex(mutex_t__pmutex, uint__blocktime_ms)       xSemaphoreTakeRecursive(mutex_t__pmutex, MS2TICK((portTickType)(uint__blocktime_ms)))
#define unlock_recursive_mutex(mutex_t__pmutex)                         xSemaphoreGiveRecursive(mutex_t__pmutex)

#define MUTEX_LOCKED                                                    true

/** QUEUE */
#define new_queue(uint__length, uint__item_size)                        xQueueCreate((unsigned portBASE_TYPE)uint__length, (unsigned portBASE_TYPE)uint__item_size)
#define delete_queue(queue_t__pqueue)                                   vQueueDelete(queue_t__pqueue)
#define reset_queue(queue_t__pqueue)                                    xQueueReset(queue_t__pqueue)
#define send_queue(queue_t__pqueue, void__pitem, uint__waittime_ms)     xQueueSend(queue_t__pqueue, void__pitem, MS2TICK((portTickType)(uint__waittime_ms)))
#define send_queue_from_ISR(queue_t__pqueue, void__pitem, int__pwoke)   xQueueSendFromISR(queue_t__pqueue, void__pitem, int__pwoke)
#define receive_queue(queue_t__pqueue, void__pitem, uint__waittime_ms)  xQueueReceive(queue_t__pqueue, void__pitem, MS2TICK((portTickType)(uint__waittime_ms)))
#define receive_queue_from_ISR(queue_t__pqueue, void__pitem, int__pwoke)xQueueReceiveFromISR(queue_t__pqueue, void__pitem, int__pwoke)
#define receive_peek_queue(queue_t__pqueue, void__pitem, uint__wait_ms) xQueuePeek(queue_t__pqueue, void__pitem, MS2TICK((portTickType)(uint__wait_ms)))
#define receive_peek_queue_from_ISR(queue_t__pqueue, void__pitem)       xQueuePeekFromISR(queue_t__pqueue, void__pitem)
#define get_number_of_items_in_queue(queue_t__pqueue)                   uxQueueMessagesWaiting(queue_t__pqueue)
#define get_number_of_items_in_queue_from_ISR(queue_t__pqueue)          uxQueueMessagesWaitingFromISR(queue_t__pqueue)

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/
struct task_data {
        struct vfs_file *f_stdin;        /* stdin file                         */
        struct vfs_file *f_stdout;       /* stdout file                        */
        struct vfs_file *f_stderr;       /* stderr file                        */
        const char      *f_cwd;          /* current working path               */
        void            *f_global_vars;  /* address to global variables        */
        void            *f_user;         /* pointer to user data               */
        void            *f_monitor;      /* pointer to task monitor data       */
        task_t          *f_parent_task;  /* program's parent task              */
        u32_t            f_cpu_usage;    /* counter used to calculate CPU load */
        bool             f_program;      /* true if task is complex program    */
};

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
  Exported function prototypes
==============================================================================*/
extern task_t *kwrap_new_task(void (*func)(void*), const char*, uint, void*);
extern void    kwrap_delete_task(task_t *taskHdl);
extern sem_t  *kwrap_create_binary_semaphore(void);
extern void    kwrap_task_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* KWRAPPER_H_ */
/*==============================================================================
  End of file
==============================================================================*/
