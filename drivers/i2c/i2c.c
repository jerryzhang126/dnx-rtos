/*=============================================================================================*//**
@file    i2c.c

@author  Daniel Zorychta

@brief   This driver support I2C

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
#include "i2c.h"


/*==================================================================================================
                                  Local symbolic constants/macros
==================================================================================================*/
#define PORT_FREE                         (u16_t)EMPTY_TASK

/** define I2C error mask */
#define I2C_ERROR_MASK_BM                 (I2C_SR1_OVR | I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR)

#define SetSCLFreq(SCLFreq)               (((u32_t)I2C_PERIPHERAL_FREQ * 1000000UL) / (2UL * (u32_t)SCLFreq))
#define SetSCLTRise()                     (I2C_PERIPHERAL_FREQ + 1)


/*==================================================================================================
                                   Local types, enums definitions
==================================================================================================*/
/** i2c control structure */
struct i2cCtrl
{
      I2C_t       *Address;             /* peripheral address */
      xTaskHandle TaskHandle;           /* task handle variable for IRQ */
      u16_t       Lock;                 /* port reservation */
      u8_t        SlaveAddress;         /* slave address */
      stdRet_t    status;               /* last operation status */
};

/** type which contain port information */
typedef struct PortHandler
{
      struct i2cCtrl *port[I2C_DEV_LAST];
} PortHandler_t;


/*==================================================================================================
                                      Local function prototypes
==================================================================================================*/
static stdRet_t StartCondition(I2C_t *i2c, u8_t slaveaddr);
static stdRet_t SendData(I2C_t *i2c, u8_t *src, size_t size);
static stdRet_t ReadData(I2C_t *i2c, u8_t *dst, size_t size);
static void     StopCondition(I2C_t *i2c);
static stdRet_t CheckStatus(I2C_t *i2c, u32_t timeout);


/*==================================================================================================
                                      Local object definitions
==================================================================================================*/
/* addresses of I2C devices */
static const I2C_t *i2cAddr[I2C_DEV_LAST] =
{
#ifdef I2C_DEV_1_DEFINED
      I2C1,
#endif
#ifdef I2C_DEV_2_DEFINED
      I2C2,
#endif
};

/* pointer to allocated memory for driver */
static PortHandler_t *i2c;


/*==================================================================================================
                                        Function definitions
==================================================================================================*/

//================================================================================================//
/**
 * @brief Initialize I2C
 *
 * @param[in] dev           I2C device
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//================================================================================================//
stdRet_t I2C_Init(dev_t dev)
{
      stdRet_t status = STD_RET_ERROR;

      if (i2c == NULL)
      {
            i2c = Calloc(1, sizeof(PortHandler_t));

            if (i2c == NULL)
                  goto I2C_Init_end;
      }

      if (dev < I2C_DEV_LAST)
      {
            if (i2c->port[dev] == NULL)
            {
                  i2c->port[dev] = Calloc(1, sizeof(struct i2cCtrl));

                  if (i2c->port[dev] != NULL)
                  {
                        i2c->port[dev]->Address = (I2C_t*)i2cAddr[dev];
                        i2c->port[dev]->Lock    = PORT_FREE;

                        status = STD_RET_OK;
                  }
            }
      }

      I2C_Init_end:
      return status;
}


//================================================================================================//
/**
 * @brief Open selected I2C port
 *
 * @param[in] dev           I2C device
 *
 * @retval STD_RET_OK                     operation success
 * @retval I2C_STATUS_PORTNOTEXIST        port not exist
 * @retval I2C_STATUS_PORTLOCKED          port locked
 */
//================================================================================================//
stdRet_t I2C_Open(dev_t dev)
{
      stdRet_t status  = I2C_STATUS_PORTNOTEXIST;
      I2C_t    *i2cPtr = NULL;

      /* check port range */
      if ((unsigned)dev < I2C_DEV_LAST)
      {
            /* lock task switching */
            TaskSuspendAll();

            /* check that port is free */
            if (i2c->port[dev]->Lock == PORT_FREE)
            {
                  /* registered port for current task */
                  i2c->port[dev]->Lock = TaskGetPID();

                  TaskResumeAll();

                  /* set task handle for IRQs */
                  i2c->port[dev]->TaskHandle = TaskGetCurrentTaskHandle();

                  /* enable I2C clock */
                  switch (dev)
                  {
                        #ifdef RCC_APB1ENR_I2C1EN
                        #if (I2C1_ENABLE > 0)
                        case I2C_DEV_1:
                              RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
                              break;
                        #endif
                        #endif

                        #ifdef RCC_APB1ENR_I2C2EN
                        #if (I2C2_ENABLE > 0)
                        case I2C_DEV_2:
                              RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
                              break;
                        #endif
                        #endif

                        default:
                              break;
                  }

                  /* set port address */
                  i2cPtr = i2c->port[dev]->Address;

                  /* set default port settings */
                  i2cPtr->CR1   |= I2C_CR1_SWRST;
                  i2cPtr->CR1    = 0;
                  i2cPtr->CR2    = I2C_PERIPHERAL_FREQ;
                  i2cPtr->SR1    = 0;
                  i2cPtr->CCR    = SetSCLFreq(I2C_DEFAULT_SCL_FREQ);
                  i2cPtr->TRISE  = SetSCLTRise();
                  i2cPtr->CR1   |= I2C_CR1_PE;

                  status = STD_RET_OK;
            }
            else
            {
                  TaskResumeAll();

                  if (i2c->port[dev]->Lock == TaskGetPID())
                        status = STD_RET_OK;
                  else
                        status = I2C_STATUS_PORTLOCKED;
            }
      }

      return status;
}


//================================================================================================//
/**
 * @brief Close port
 *
 * @param[in] dev           I2C device
 *
 * @retval STD_RET_OK                     operation success
 * @retval I2C_STATUS_PORTNOTEXIST        port not exist
 * @retval I2C_STATUS_PORTLOCKED          port locked
 */
//================================================================================================//
stdRet_t I2C_Close(dev_t dev)
{
      stdRet_t status  = I2C_STATUS_PORTNOTEXIST;
      I2C_t    *i2cPtr = NULL;

      /* check port range */
      if ((unsigned)dev < I2C_DEV_LAST)
      {
            /* check that port is reserved for this task */
            if (i2c->port[dev]->Lock == TaskGetPID())
            {
                  /* set port address */
                  i2cPtr = i2c->port[dev]->Address;

                  /* disable I2C device */
                  i2cPtr->CR1 |= I2C_CR1_SWRST;
                  i2cPtr->CR1  = 0;
                  i2cPtr->CR1 &= ~I2C_CR1_PE;

                  /* turn off device */
                  switch (dev)
                  {
                        #ifdef RCC_APB1ENR_I2C1EN
                        #if (I2C1_ENABLE > 0)
                        case I2C_DEV_1:
                              RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
                              break;
                        #endif
                        #endif

                        #ifdef RCC_APB1ENR_I2C2EN
                        #if (I2C2_ENABLE > 0)
                        case I2C_DEV_2:
                              RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN;
                              break;
                        #endif
                        #endif

                        default:
                              break;
                  }

                  /* unlock device */
                  i2c->port[dev]->Lock = PORT_FREE;

                  /* delete from task handle */
                  i2c->port[dev]->TaskHandle = NULL;

                  status = STD_RET_OK;
            }
            else
            {
                  status = I2C_STATUS_PORTLOCKED;
            }
      }

      return status;
}


//================================================================================================//
/**
 * @brief Function write data to the I2C device
 *
 * @param[in] dev           I2C device
 * @param[in] *src          source data
 * @param[in] size          data size
 * @param[in] seek          register address
 *
 * @retval number of written nitems
 */
//================================================================================================//
size_t I2C_Write(dev_t dev, void *src, size_t size, size_t nitems, size_t seek)
{
      size_t n = 0;

      /* check port range */
      if ((unsigned)dev < I2C_DEV_LAST)
      {
              /* check that port is reserved for this task */
            if (i2c->port[dev]->Lock == TaskGetPID())
            {
                  I2C_t *i2cPtr     = i2c->port[dev]->Address;
                  u8_t  *data       = (u8_t *)src;
                  stdRet_t *i2cstat = &i2c->port[dev]->status;

                  /* check arguments */
                  if (!src || !size || !nitems)
                  {
                        i2c->port[dev]->status = I2C_STATUS_BADARG;
                        goto I2C_Write_end;
                  }

                  /* start condition */
                  if ((*i2cstat = StartCondition(i2cPtr, (i2c->port[dev]->SlaveAddress << 1) & 0xFE))
                     != STD_RET_OK)
                  {
                        goto I2C_Write_end;
                  }

                  /* send register address */
                  u8_t regAddr = seek;

                  if ((*i2cstat = SendData(i2cPtr, &regAddr, 1)) != STD_RET_OK)
                  {
                        goto I2C_Write_end;
                  }

                  /* send data */
                  if ((*i2cstat = SendData(i2cPtr, data, nitems * size)) != STD_RET_OK)
                  {
                        goto I2C_Write_end;
                  }

                  /* stop condition */
                  while (!(i2cPtr->SR1 & I2C_SR1_BTF) || !(i2cPtr->SR1 & I2C_SR1_TXE))
                  {
                        TaskDelay(1);
                  }

                  StopCondition(i2cPtr);

                  n = nitems;
            }
      }

      I2C_Write_end:
            return n;
}


//================================================================================================//
/**
 * @brief Read data wrom I2C device
 *
 * @param[in ] dev           I2C device
 * @param[out] *dst          destination data
 * @param[in ] size          data size
 * @param[in ] seek          register address
 *
 * @retval number of written nitems
 */
//================================================================================================//
size_t I2C_Read(dev_t dev, void *dst, size_t size, size_t nitems, size_t seek)
{
      size_t n = 0;

      /* check port range */
      if ((unsigned)dev < I2C_DEV_LAST)
      {
            /* check that port is reserved for this task */
            if (i2c->port[dev]->Lock == TaskGetPID())
            {
                  I2C_t *i2cPtr     = i2c->port[dev]->Address;
                  stdRet_t *i2cstat = &i2c->port[dev]->status;

                  /* check arguments */
                  if (!dst || !size || !nitems)
                  {
                        *i2cstat = I2C_STATUS_BADARG;
                        goto I2C_Read_end;
                  }

                  /* start condition */
                  if ((*i2cstat = StartCondition(i2cPtr, (i2c->port[dev]->SlaveAddress << 1) & 0xFE) )
                     != STD_RET_OK)
                  {
                        goto I2C_Read_end;
                  }

                  /* send register address */
                  u8_t regAddr = seek;

                  if ((*i2cstat = SendData(i2cPtr, &regAddr, 1)) != STD_RET_OK)
                        goto I2C_Read_end;

                  /* waiting for data send was finished */
                  while (!(i2cPtr->SR1 & I2C_SR1_BTF) || !(i2cPtr->SR1 & I2C_SR1_TXE))
                  {
                        TaskDelay(1);
                  }

                  /* check if is only 1 byte to send */
                  if (nitems * size == 1)
                        i2cPtr->CR1 &= ~(I2C_CR1_ACK);
                  else
                        i2cPtr->CR1 |= I2C_CR1_ACK;

                  /* repeat start condition */
                  if ((*i2cstat = StartCondition(i2cPtr,(i2c->port[dev]->SlaveAddress << 1) | 0x01) )
                     != STD_RET_OK)
                  {
                        goto I2C_Read_end;
                  }

                  /* receive bytes */
                  if ((*i2cstat = ReadData(i2cPtr, dst, nitems * size)) != STD_RET_OK)
                        goto I2C_Read_end;

                  /* stop condition */
                  StopCondition(i2cPtr);

                  n = nitems;
            }
      }

      I2C_Read_end:
            return n;
}


//================================================================================================//
/**
 * @brief Specific settings of I2C port
 *
 * @param[in    ] dev           I2C device
 * @param[in    ] ioRQ          input/output reqest
 * @param[in,out] *data         input/output data
 *
 * @retval STD_RET_OK                     operation success
 * @retval I2C_STATUS_PORTNOTEXIST        port not exist
 * @retval I2C_STATUS_PORTLOCKED          port locked
 * @retval I2C_STATUS_OVERRUN             overrun
 * @retval I2C_STATUS_ACK_FAILURE         ack failure
 * @retval I2C_STATUS_ARB_LOST            arbitration lost error
 * @retval I2C_STATUS_BUS_ERROR           bus error
 * @retval I2C_STATUS_ERROR               more than 1 error
 */
//================================================================================================//
stdRet_t I2C_IOCtl(dev_t dev, IORq_t ioRQ, void *data)
{
      stdRet_t status = I2C_STATUS_PORTNOTEXIST;

      /* check port range */
      if ((unsigned)dev < I2C_DEV_LAST)
      {
            /* check that port is reserved for this task */
            if (i2c->port[dev]->Lock == TaskGetPID())
            {
                  I2C_t *i2cPtr = i2c->port[dev]->Address;

                  status = STD_RET_OK;

                  switch (ioRQ)
                  {
                        case I2C_IORQ_SETSLAVEADDR:
                        {
                              i2c->port[dev]->SlaveAddress = *((u8_t *)data);
                              break;
                        }

                        case I2C_IORQ_GETSLAVEADDR:
                        {
                              *((u8_t *)data) = i2c->port[dev]->SlaveAddress;
                              break;
                        }

                        case I2C_IORQ_SETSCLFREQ:
                        {
                              i2cPtr->CCR = SetSCLFreq(*((u32_t *)data));
                              break;
                        }

                        case I2C_IORQ_GETSTATUS:
                        {
                              *(stdRet_t*)data = status;
                              break;
                        }

                        default:
                        {
                              status = I2C_STATUS_BADRQ;
                              break;
                        }
                  }
            }
            else
            {
                  status = I2C_STATUS_PORTLOCKED;
            }
      }

      return status;
}


/*================================================================================================*/
/**
* @brief Function send via I2C interface start condition and slave address
* Generate start (or repeat start) condition and send slave address to active slave.
*
* @param[in] *I2C        i2c device address
* @param[in] slaveaddr   slave address
*
* @return operation status
**/
/*================================================================================================*/
static stdRet_t StartCondition(I2C_t *i2c, u8_t slaveaddr)
{
      stdRet_t status;
      u8_t     timeout;

      /* generate start condition if bus is free */
      i2c->CR1 |= I2C_CR1_START;

      /* waiting for start condition was generated */
      for (timeout = 150; timeout > 0; timeout--)
      {
            if (i2c->SR1 & I2C_SR1_SB)
                  break;
            else
                  TaskDelay(1);
      }

      /* check errors */
      if ((status = CheckStatus(i2c, timeout)) != STD_RET_OK)
            goto StartCondition_end;

      /* write slave address */
      i2c->DR = slaveaddr;

      /* waiting for slave address transmit */
      for (timeout = 100; timeout > 0; timeout--)
      {
            if (i2c->SR1 & I2C_SR1_ADDR)
                  break;
            else
                  TaskDelay(10);
      }

      /* check errors */
      if ((status = CheckStatus(i2c, timeout)) != STD_RET_OK)
            goto StartCondition_end;

      /* clear ADDR flag */
      volatile u32_t tmp = i2c->SR2;

      StartCondition_end:
            return status;
}


//================================================================================================//
/**
 * @brief Function send data to the slave
 *
 * @param[in] *i2c          i2c device
 * @param[in] *src          data source
 * @param[in] size          data size
 *
 * @retval STD_RET_OK                     operation success
 * @retval I2C_STATUS_PORTNOTEXIST        port not exist
 * @retval I2C_STATUS_PORTLOCKED          port locked
 * @retval I2C_STATUS_OVERRUN             overrun
 * @retval I2C_STATUS_ACK_FAILURE         ack failure
 * @retval I2C_STATUS_ARB_LOST            arbitration lost error
 * @retval I2C_STATUS_BUS_ERROR           bus error
 * @retval I2C_STATUS_ERROR               more than 1 error
 */
//================================================================================================//
static stdRet_t SendData(I2C_t *i2c, u8_t *src, size_t size)
{
      stdRet_t status = STD_RET_OK;
      u8_t     timeout;

      while (size--)
      {
            for (timeout = 100; timeout > 0; timeout--)
            {
                  if (i2c->SR1 & I2C_SR1_TXE)
                  {
                        if ((status = CheckStatus(i2c, timeout)) != STD_RET_OK)
                              goto SendData_end;

                        i2c->DR = *(src++);

                        break;
                  }
                  else
                  {
                        TaskDelay(10);
                  }
            }
      }

      SendData_end:
            return status;
}


//================================================================================================//
/**
 * @brief Read data from slave
 *
 * @param[in ] dev           I2C device
 * @param[out] *dst          data destination
 * @param[in ] size          data size
 *
 * @retval STD_RET_OK                     operation success
 * @retval I2C_STATUS_PORTNOTEXIST        port not exist
 * @retval I2C_STATUS_PORTLOCKED          port locked
 * @retval I2C_STATUS_OVERRUN             overrun
 * @retval I2C_STATUS_ACK_FAILURE         ack failure
 * @retval I2C_STATUS_ARB_LOST            arbitration lost error
 * @retval I2C_STATUS_BUS_ERROR           bus error
 * @retval I2C_STATUS_ERROR               more than 1 error
 */
//================================================================================================//
static stdRet_t ReadData(I2C_t *i2c, u8_t *dst, size_t size)
{
      stdRet_t status  = STD_RET_OK;
      u8_t     timeout;

      /* send buffer */
      do
      {
            timeout = 200;

            while (timeout--)
            {
                  if (i2c->SR1 & I2C_SR1_RXNE)
                  {
                        if ((status = CheckStatus(i2c, timeout)) != STD_RET_OK)
                              goto ReadData_end;

                        /* if second last byte send  */
                        if (size == 2)
                              i2c->CR1 &= ~(I2C_CR1_ACK);

                        *(dst++) = i2c->DR;

                        break;
                  }
                  else
                  {
                        TaskDelay(10);
                  }
            }

            size--;
      }
      while (size);

      /* waiting for finish receiving NACK */
      timeout = 200;

      while (timeout--)
      {
            if (i2c->SR1 & I2C_SR1_RXNE)
            {
                  if ((status = CheckStatus(i2c, timeout)) != STD_RET_OK)
                        goto ReadData_end;

                  timeout = i2c->DR;

                  break;
            }
            else
            {
                  TaskDelay(10);
            }
      }

      ReadData_end:
            return status;
}


//================================================================================================//
/**
 * @brief Function send stop condition
 *
 * @param[in] *i2c          i2c device
 */
//================================================================================================//
static void StopCondition(I2C_t *i2c)
{
      i2c->CR1 |= I2C_CR1_STOP;
}


//================================================================================================//
/**
 * @brief Function check I2C errors
 *
 * @param[in] *i2c          i2c device
 * @param[in] timeout       timeout value after operation
 *
 * @retval STD_RET_OK                 no error
 * @retval I2C_STATUS_OVERRUN         overrun
 * @retval I2C_STATUS_ACK_FAILURE     ack failure
 * @retval I2C_STATUS_ARB_LOST        arbitration lost error
 * @retval I2C_STATUS_BUS_ERROR       bus error
 * @retval I2C_STATUS_ERROR           more than 1 error
 */
//================================================================================================//
static stdRet_t CheckStatus(I2C_t *i2c, u32_t timeout)
{
      stdRet_t status;

      if (!timeout)
      {
            status = I2C_STATUS_TIMEOUT;
      }
      else
      {
            switch (i2c->SR1 & I2C_ERROR_MASK_BM)
            {
                  case 0:            status = STD_RET_OK;             break;
                  case I2C_SR1_OVR:  status = I2C_STATUS_OVERRUN;     break;
                  case I2C_SR1_AF:   status = I2C_STATUS_ACK_FAILURE; break;
                  case I2C_SR1_ARLO: status = I2C_STATUS_ARB_LOST;    break;
                  case I2C_SR1_BERR: status = I2C_STATUS_BUS_ERROR;   break;
                  default:           status = I2C_STATUS_ERROR;       break;
            }
      }

      /* when error occur generate stop condition */
      if (status != STD_RET_OK)
      {
            i2c->SR1 = 0;
            StopCondition(i2c);
      }

      return status;
}


//================================================================================================//
/**
 * @brief Release I2C peripheral
 *
 * @param[in] dev           I2C device
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//================================================================================================//
stdRet_t I2C_Release(dev_t dev)
{
      stdRet_t status = STD_RET_ERROR;

      if (i2c && dev < I2C_DEV_LAST)
      {
            /* free i2c device data */
            if (i2c->port[dev])
            {
                  Free(i2c->port[dev]);
                  i2c->port[dev] = NULL;
                  status = STD_RET_OK;
            }

            /* check if all devices are not used, if yes then free handler */
            for (u8_t i = 0; i < I2C_DEV_LAST; i++)
            {
                  if (i2c->port[i] != NULL)
                        goto I2C_Release_End;
            }

            /* free i2c handler */
            Free(i2c);
            i2c = NULL;
      }

      I2C_Release_End:
      return status;
}


#ifdef __cplusplus
}
#endif

/*==================================================================================================
                                            End of file
==================================================================================================*/
