/*=============================================================================================*//**
@file    mian.c

@author  Daniel Zorychta

@brief   This file provide system initialisation and RTOS start.

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


*//*==============================================================================================*/

#ifdef __cplusplus
   extern "C" {
#endif

/*==================================================================================================
                                             Include files
==================================================================================================*/
#include "system.h"
#include "pll.h"
#include "gpio.h"
#include "uart.h"
#include "terminal.h"
#include "ether.h"


/*==================================================================================================
                                   Local symbolic constants/macros
==================================================================================================*/
/** InitTask stack size */
#define INITTASK_STACK_SIZE               (4 * MINIMAL_STACK_SIZE)


/*==================================================================================================
                                   Local types, enums definitions
==================================================================================================*/


/*==================================================================================================
                                      Local function prototypes
==================================================================================================*/
static void InitSystem(void);
static void InitTask(void *arg);


/*==================================================================================================
                                      Local object definitions
==================================================================================================*/


/*==================================================================================================
                                     Exported object definitions
==================================================================================================*/


/*==================================================================================================
                                         Function definitions
==================================================================================================*/

//================================================================================================//
/**
 * @brief Main function
 */
//================================================================================================//
int main(void)
{
      InitSystem();

      TaskCreate(InitTask, "initd", INITTASK_STACK_SIZE, NULL, 3, NULL);

      vTaskStartScheduler();

      return 0;
}


//================================================================================================//
/**
 * @brief Initialise system
 * Insert here all initialize functions which should be initialized early before application start
 */
//================================================================================================//
static void InitSystem(void)
{
      /* set interrupt vectors and NVIC priority */
      SCB->VTOR  = 0x00 | (0x00 & (uint32_t)0x1FFFFF80);
      SCB->AIRCR = 0x05FA0000 | 0x300;

      /* PLL initialization */
      if (PLL_Init() != STD_STATUS_OK)
            while (TRUE);

      /* GPIO and AFIO initialization */
      GPIO_Init();

      /* initialize UART driver */
      UART_Init();
}


//================================================================================================//
/**
 * @brief Task which initialise high-level devices/applications etc
 * Task is responsible for low-level application runtime environment (stdio). Task connect
 * applications' stdios with hardware layer.
 */
//================================================================================================//
static void InitTask(void *arg)
{
      (void) arg;

      /* short delay and lock task scheduling */
      TaskDelay(2000);
      TaskSuspendAll();

      /*--------------------------------------------------------------------------------------------
       * initialization kprint()
       *------------------------------------------------------------------------------------------*/
      UART_Open(UART_DEV_1);
      kprintEnable();

      /* clear VT100 terminal screen */
      clrscr(k);

      /* something about board and system */
      kprint("Board powered by "); fontGreen(k); kprint("FreeRTOS\n"); resetAttr(k);

      kprint("By "); fontCyan(k); kprint("Daniel Zorychta ");
      fontYellow(k); kprint("<daniel.zorychta@gmail.com>\n\n"); resetAttr(k);

      /* info about system start */
      kprint("initd [%d]: kernel print started\n", TaskGetTickCount());
      kprint("initd [%d]: init daemon started\n", TaskGetTickCount());

      /*--------------------------------------------------------------------------------------------
       * driver initialization
       *------------------------------------------------------------------------------------------*/
      if (ETHER_Init() == STD_STATUS_OK)
      {
            LwIP_Init();
      }

      /*--------------------------------------------------------------------------------------------
       * starting terminal
       *------------------------------------------------------------------------------------------*/
      kprint("initd [%d]: starting interactive console... ", TaskGetTickCount());

      /* try to start terminal */
      appArgs_t *stdio = StartApplication(terminal, "terminal", TERMINAL_STACK_SIZE, NULL);

      if (stdio == NULL)
      {
            fontRed(k); kprint("FAILED\n"); resetAttr(k);
            kprint("Probably no enough free space. Restarting board...");
            TaskResumeAll();
            TaskDelay(5000);
            NVIC_SystemReset();
      }
      else
      {
            fontGreen(k); kprint("SUCCESS\n"); resetAttr(k);
      }

      /* initd info about stack usage */
      kprint("initd [%d]: free stack: %d\n\n", TaskGetTickCount(), TaskGetStackFreeSpace(THIS_TASK));

      /*--------------------------------------------------------------------------------------------
       * main loop which read stdios from applications
       *------------------------------------------------------------------------------------------*/
      TaskResumeAll();

      for (;;)
      {
            ch_t   data;
            bool_t stdoutEmpty = FALSE;
            bool_t RxFIFOEmpty = FALSE;

            TaskSuspendAll();

            if (stdio->stdout.Level > 0)
            {
                  data = stdio->stdout.Buffer[stdio->stdout.RxIdx++];

                  if (stdio->stdout.RxIdx >= configSTDIO_BUFFER_SIZE)
                        stdio->stdout.RxIdx = 0;

                  stdio->stdout.Level--;

                  UART_IOCtl(UART_DEV_1, UART_IORQ_SEND_BYTE, &data);

                  if (data <= 1)
                  {
                        Free(stdio);

                        if (data == 0)
                              kprint("\ninitd [%d]: terminal was terminated.\n", TaskGetTickCount());
                        else
                              kprint("\ninitd [%d]: terminal was terminated with error.\n", TaskGetTickCount());

                        kprint("initd [%d]: disable FreeRTOS scheduler. Bye.\n", TaskGetTickCount());

                        vTaskEndScheduler();

                        while (TRUE)
                              TaskDelay(1000);
                  }

                  stdoutEmpty = FALSE;
            }
            else
            {
                  stdoutEmpty = TRUE;
            }

            TaskResumeAll();

            if (UART_IOCtl(UART_DEV_1, UART_IORQ_GET_BYTE, &data) == STD_STATUS_OK)
            {
                  TaskSuspendAll();

                  if (stdio->stdin.Level < configSTDIO_BUFFER_SIZE)
                  {
                        stdio->stdin.Buffer[stdio->stdin.TxIdx++] = data;

                        if (stdio->stdin.TxIdx >= configSTDIO_BUFFER_SIZE)
                              stdio->stdin.TxIdx = 0;

                        stdio->stdin.Level++;
                  }

                  TaskResumeAll();

                  RxFIFOEmpty = FALSE;
            }
            else
            {
                  RxFIFOEmpty = TRUE;
            }

            if (stdoutEmpty && RxFIFOEmpty)
                  TaskDelay(1);
      }

      /* this should never happen */
      UART_Close(UART_DEV_1);

      TaskTerminate();
}


#ifdef __cplusplus
   }
#endif

/*==================================================================================================
                                             End of file
==================================================================================================*/
