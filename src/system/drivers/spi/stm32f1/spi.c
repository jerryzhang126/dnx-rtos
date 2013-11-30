/*=========================================================================*//**
@file    spi.c

@author  Daniel Zorychta

@brief   SPI driver

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

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "system/dnxmodule.h"
#include "system/thread.h"
#include "stm32f1/spi_cfg.h"
#include "stm32f1/spi_def.h"
#include "stm32f1/stm32f10x.h"

/*==============================================================================
  Local macros
==============================================================================*/
#define MUTEX_TIMOUT                    MAX_DELAY
#define SEMAPHORE_TIMEOUT               MAX_DELAY
#define MAX_NUMBER_OF_CS                8

/*==============================================================================
  Local object types
==============================================================================*/
/* configuration of single CS line (port and pin) */
struct cs_pin_cfg {
        GPIO_t *const           port;
        u16_t                   pin_mask;
};

/* priority configuration */
struct spi_priority_cfg {
        IRQn_Type               IRQn;
        u32_t                   priority;
};

/* independent SPI instance */
struct spi_virtual {
        u8_t                    major;
        u8_t                    minor;
        struct SPI_config       config;
        dev_lock_t              file_lock;
};

/* general module data */
struct module {
        sem_t                  *wait_irq_sem[_SPI_DEV_NUMBER];
        mutex_t                *device_protect_mtx[_SPI_DEV_NUMBER];
        u8_t                   *buffer[_SPI_DEV_NUMBER];
        size_t                  count[_SPI_DEV_NUMBER];
        bool                    write[_SPI_DEV_NUMBER];
        u8_t                    dummy_byte[_SPI_DEV_NUMBER];
        u8_t                    number_of_virtual_spi[_SPI_DEV_NUMBER];
};

/*==============================================================================
  Local function prototypes
==============================================================================*/
static void     spi_turn_on             (SPI_t *spi);
static void     spi_turn_off            (SPI_t *spi);
static void     spi_apply_config        (struct spi_virtual *vspi);
static void     spi_apply_safe_config   (u8_t major);
static void     spi_select_slave        (u8_t major, u8_t minor);
static void     spi_deselect_slave      (u8_t major);
static void     spi_irq_handle          (u8_t major);

/*==============================================================================
  Local objects
==============================================================================*/
/* SPI peripherals address */
static SPI_t *const spi[_SPI_DEV_NUMBER] = {
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        SPI1,
#endif
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        SPI2,
#endif
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        SPI3
#endif
};

/* CS port configuration */
static const struct cs_pin_cfg spi_cs_pin_cfg[_SPI_DEV_NUMBER][MAX_NUMBER_OF_CS] = {
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        {{port: (GPIO_t *)_SPI1_CS0_PORT, pin_mask: (1 << _SPI1_CS0_PIN)},
         {port: (GPIO_t *)_SPI1_CS1_PORT, pin_mask: (1 << _SPI1_CS1_PIN)},
         {port: (GPIO_t *)_SPI1_CS2_PORT, pin_mask: (1 << _SPI1_CS2_PIN)},
         {port: (GPIO_t *)_SPI1_CS3_PORT, pin_mask: (1 << _SPI1_CS3_PIN)},
         {port: (GPIO_t *)_SPI1_CS4_PORT, pin_mask: (1 << _SPI1_CS4_PIN)},
         {port: (GPIO_t *)_SPI1_CS5_PORT, pin_mask: (1 << _SPI1_CS5_PIN)},
         {port: (GPIO_t *)_SPI1_CS6_PORT, pin_mask: (1 << _SPI1_CS6_PIN)},
         {port: (GPIO_t *)_SPI1_CS7_PORT, pin_mask: (1 << _SPI1_CS7_PIN)}},
#endif
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        {{port: (GPIO_t *)_SPI2_CS0_PORT, pin_mask: (1 << _SPI2_CS0_PIN)},
         {port: (GPIO_t *)_SPI2_CS1_PORT, pin_mask: (1 << _SPI2_CS1_PIN)},
         {port: (GPIO_t *)_SPI2_CS2_PORT, pin_mask: (1 << _SPI2_CS2_PIN)},
         {port: (GPIO_t *)_SPI2_CS3_PORT, pin_mask: (1 << _SPI2_CS3_PIN)},
         {port: (GPIO_t *)_SPI2_CS4_PORT, pin_mask: (1 << _SPI2_CS4_PIN)},
         {port: (GPIO_t *)_SPI2_CS5_PORT, pin_mask: (1 << _SPI2_CS5_PIN)},
         {port: (GPIO_t *)_SPI2_CS6_PORT, pin_mask: (1 << _SPI2_CS6_PIN)},
         {port: (GPIO_t *)_SPI2_CS7_PORT, pin_mask: (1 << _SPI2_CS7_PIN)}},
#endif
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        {{port: (GPIO_t *)_SPI3_CS0_PORT, pin_mask: (1 << _SPI3_CS0_PIN)},
         {port: (GPIO_t *)_SPI3_CS1_PORT, pin_mask: (1 << _SPI3_CS1_PIN)},
         {port: (GPIO_t *)_SPI3_CS2_PORT, pin_mask: (1 << _SPI3_CS2_PIN)},
         {port: (GPIO_t *)_SPI3_CS3_PORT, pin_mask: (1 << _SPI3_CS3_PIN)},
         {port: (GPIO_t *)_SPI3_CS4_PORT, pin_mask: (1 << _SPI3_CS4_PIN)},
         {port: (GPIO_t *)_SPI3_CS5_PORT, pin_mask: (1 << _SPI3_CS5_PIN)},
         {port: (GPIO_t *)_SPI3_CS6_PORT, pin_mask: (1 << _SPI3_CS6_PIN)},
         {port: (GPIO_t *)_SPI3_CS7_PORT, pin_mask: (1 << _SPI3_CS7_PIN)}}
#endif
};

/* number of SPI cs */
static const u8_t spi_number_of_slaves[_SPI_DEV_NUMBER] = {
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        _SPI1_NUMBER_OF_SLAVES,
#endif
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        _SPI2_NUMBER_OF_SLAVES,
#endif
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        _SPI3_NUMBER_OF_SLAVES
#endif
};

/* priorities for specified SPI peripherals */
static const struct spi_priority_cfg spi_priority[_SPI_DEV_NUMBER] = {
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        {IRQn: SPI1_IRQn, priority: _SPI1_IRQ_PRIORITY},
#endif
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        {IRQn: SPI2_IRQn, priority: _SPI2_IRQ_PRIORITY},
#endif
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        {IRQn: SPI3_IRQn, priority: _SPI3_IRQ_PRIORITY}
#endif
};

/* default SPI config */
static const struct SPI_config spi_default_cfg = {
        dummy_byte : _SPI_DEFAULT_CFG_DUMMY_BYTE,
        clk_divider: _SPI_DEFAULT_CFG_CLK_DIVIDER,
        mode       : _SPI_DEFAULT_CFG_MODE,
        msb_first  : _SPI_DEFAULT_CFG_MSB_FIRST
};

/* pointers to memory of specified device */
static struct module *spi_module;

/*==============================================================================
  Exported objects
==============================================================================*/

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
API_MOD_INIT(SPI, void **device_handle, u8_t major, u8_t minor)
{
        if (major >= _SPI_DEV_NUMBER)
                return STD_RET_ERROR;

#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        if (major == _SPI_DEV_1 && minor >= _SPI1_NUMBER_OF_SLAVES)
                return STD_RET_ERROR;
#endif

#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        if (major == _SPI_DEV_2 && minor >= _SPI2_NUMBER_OF_SLAVES)
                return STD_RET_ERROR;
#endif

#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        if (major == _SPI_DEV_3 && minor >= _SPI3_NUMBER_OF_SLAVES)
                return STD_RET_ERROR;
#endif

        /* allocate module general data if initialized first time */
        if (!spi_module) {
                spi_module = calloc(1, sizeof(struct module));
                if (!spi_module)
                        return STD_RET_ERROR;
        }

        /* create irq semaphore */
        if (!spi_module->wait_irq_sem[major]) {
                spi_module->wait_irq_sem[major] = semaphore_new(1, 1);
                if (!spi_module->wait_irq_sem[major]) {
                        return STD_RET_ERROR;
                }
        }

        /* create protection mutex and start device if initialized first time */
        if (!spi_module->device_protect_mtx[major]) {
                spi_module->device_protect_mtx[major] = mutex_new(MUTEX_NORMAL);
                if (!spi_module->device_protect_mtx[major]) {
                        semaphore_delete(spi_module->wait_irq_sem[major]);
                        spi_module->wait_irq_sem[major] = NULL;
                        return STD_RET_ERROR;
                } else {
                        NVIC_SetPriority(spi_priority[major].IRQn, spi_priority[major].priority);
                        spi_turn_on(spi[major]);
                        spi_apply_safe_config(major);
                }
        }

        /* create new instance for specified major-minor number (virtual spi) */
        struct spi_virtual *hdl = calloc(1, sizeof(struct spi_virtual));
        if (!hdl) {
                return STD_RET_ERROR;
        }

        hdl->config     = spi_default_cfg;
        hdl->major      = major;
        hdl->minor      = minor;
        *device_handle  = hdl;

        spi_module->number_of_virtual_spi[major]++;

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
API_MOD_RELEASE(SPI, void *device_handle)
{
        STOP_IF(!device_handle);

        struct spi_virtual *hdl = device_handle;

        stdret_t status = STD_RET_ERROR;

        critical_section_begin();
        if (device_is_unlocked(hdl->file_lock)) {

                spi_module->number_of_virtual_spi[hdl->major]--;

                /* deinitialize major device if all minor devices are deinitialized */
                if (spi_module->number_of_virtual_spi == 0) {
                        mutex_delete(spi_module->device_protect_mtx[hdl->major]);
                        spi_module->device_protect_mtx[hdl->major] = NULL;
                        semaphore_delete(spi_module->wait_irq_sem[hdl->major]);
                        spi_module->wait_irq_sem[hdl->major] = NULL;
                        spi_turn_off(spi[hdl->major]);
                }

                /* free module memory if all devices are deinitialized */
                bool free_module_mem = true;
                for (int i = 0; i < _SPI_DEV_NUMBER && free_module_mem; i++) {
                        free_module_mem = spi_module->device_protect_mtx[i] == NULL;
                }

                if (free_module_mem) {
                        free(spi_module);
                        spi_module = NULL;
                }

                /* free virtual spi memory */
                free(hdl);

                status = STD_RET_OK;
        }
        critical_section_end();

        return status;
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
API_MOD_OPEN(SPI, void *device_handle, int flags)
{
        STOP_IF(!device_handle);
        UNUSED_ARG(flags);

        struct spi_virtual *hdl = device_handle;

        return device_lock(&hdl->file_lock) ? STD_RET_OK : STD_RET_ERROR;
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
API_MOD_CLOSE(SPI, void *device_handle, bool force, const task_t *opened_by_task)
{
        STOP_IF(!device_handle);
        UNUSED_ARG(opened_by_task);

        struct spi_virtual *hdl = device_handle;

        if (device_is_access_granted(&hdl->file_lock)) {
                device_unlock(&hdl->file_lock, force);
                return STD_RET_OK;
        } else {
                return STD_RET_ERROR;
        }
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
API_MOD_WRITE(SPI, void *device_handle, const u8_t *src, size_t count, u64_t *fpos)
{
        UNUSED_ARG(fpos);

        STOP_IF(device_handle == NULL);
        STOP_IF(src == NULL);
        STOP_IF(count == 0);

        struct spi_virtual *hdl = device_handle;

        ssize_t n = -1;

        if (mutex_lock(spi_module->device_protect_mtx[hdl->major], MUTEX_TIMOUT)) {
                spi_apply_config(hdl);
                spi_select_slave(hdl->major, hdl->minor);

                spi_module->buffer[hdl->major] = (u8_t *)src;
                spi_module->count[hdl->major]  = count;
                spi_module->write[hdl->major]  = true;

                SET_BIT(spi[hdl->major]->CR2, SPI_CR2_TXEIE);
                semaphore_wait(spi_module->wait_irq_sem[hdl->major], SEMAPHORE_TIMEOUT);
                while (spi[hdl->major]->SR & SPI_SR_BSY); /* flush buffer */

                n = count - spi_module->count[hdl->major];

                spi_deselect_slave(hdl->major);
                spi_apply_safe_config(hdl->major);
                mutex_unlock(spi_module->device_protect_mtx[hdl->major]);
        } else {
                errno = ETIME;
        }

        return n;
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
API_MOD_READ(SPI, void *device_handle, u8_t *dst, size_t count, u64_t *fpos)
{
        UNUSED_ARG(fpos);

        STOP_IF(device_handle == NULL);
        STOP_IF(dst == NULL);
        STOP_IF(count == 0);

        struct spi_virtual *hdl = device_handle;

        ssize_t n = -1;

        if (mutex_lock(spi_module->device_protect_mtx[hdl->major], MUTEX_TIMOUT)) {
                spi_apply_config(hdl);
                spi_select_slave(hdl->major, hdl->minor);

                spi_module->buffer[hdl->major] = dst;
                spi_module->count[hdl->major]  = count;
                spi_module->write[hdl->major]  = false;

                u8_t tmp;
                while (spi[hdl->major]->SR & SPI_SR_RXNE) {
                        tmp = spi[hdl->major]->DR;
                }

                SET_BIT(spi[hdl->major]->CR2, SPI_CR2_RXNEIE);
                tmp = spi_module->dummy_byte[hdl->major];
                spi[hdl->major]->DR = tmp;
                semaphore_wait(spi_module->wait_irq_sem[hdl->major], SEMAPHORE_TIMEOUT);
                while (spi[hdl->major]->SR & SPI_SR_BSY); /* flush buffer */

                n = count - spi_module->count[hdl->major];

                spi_deselect_slave(hdl->major);
                spi_apply_safe_config(hdl->major);
                mutex_unlock(spi_module->device_protect_mtx[hdl->major]);
        } else {
                errno = ETIME;
        }

        return n;
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
API_MOD_IOCTL(SPI, void *device_handle, int request, void *arg)
{
        STOP_IF(device_handle == NULL);

        struct spi_virtual *hdl    = device_handle;
        stdret_t            status = STD_RET_ERROR;

        if (arg) {
                switch (request) {
                case SPI_IORQ_SET_CONFIGURATION: {
                        struct SPI_config *cfg = arg;
                        hdl->config = *cfg;
                        status = STD_RET_OK;
                        break;
                }

                case SPI_IORQ_GET_CONFIGURATION: {
                        struct SPI_config *cfg = arg;
                        *cfg   = hdl->config;
                        status = STD_RET_OK;
                        break;
                }

                default:
                        break;
                }
        }

        return status;
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
API_MOD_FLUSH(SPI, void *device_handle)
{
        STOP_IF(device_handle == NULL);

        return STD_RET_OK;
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
API_MOD_STAT(SPI, void *device_handle, struct vfs_dev_stat *device_stat)
{
        STOP_IF(device_handle == NULL);
        STOP_IF(device_stat == NULL);

        struct spi_virtual *hdl = device_handle;

        device_stat->st_major = hdl->major;
        device_stat->st_minor = hdl->minor;
        device_stat->st_size  = 0;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Function enable SPI interface
 *
 * @param[in] *spi      spi peripheral
 */
//==============================================================================
static void spi_turn_on(SPI_t *spi)
{
        switch ((uint32_t)spi) {
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        case SPI1_BASE:
                RCC->APB2RSTR |=  RCC_APB2RSTR_SPI1RST;
                RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
                RCC->APB2ENR  |=  RCC_APB2ENR_SPI1EN;
                break;
#endif
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        case SPI2_BASE:
                RCC->APB1RSTR |=  RCC_APB1RSTR_SPI2RST;
                RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;
                RCC->APB1ENR  |=  RCC_APB1ENR_SPI2EN;
                break;
#endif
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        case SPI3_BASE:
                RCC->APB1RSTR |=  RCC_APB1RSTR_SPI3RST;
                RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST;
                RCC->APB1ENR  |=  RCC_APB1ENR_SPI3EN;
                break;
#endif
        }
}

//==============================================================================
/**
 * @brief Function disable SPI interface
 *
 * @param[in] *spi      spi peripheral
 */
//==============================================================================
static void spi_turn_off(SPI_t *spi)
{
        switch ((uint32_t)spi) {
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
        case SPI1_BASE:
                RCC->APB2RSTR |=  RCC_APB2RSTR_SPI1RST;
                RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
                RCC->APB2ENR  &= ~RCC_APB2ENR_SPI1EN;
                break;
#endif
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
        case SPI2_BASE:
                RCC->APB1RSTR |=  RCC_APB1RSTR_SPI2RST;
                RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;
                RCC->APB1ENR  &= ~RCC_APB1ENR_SPI2EN;
                break;
#endif
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
        case SPI3_BASE:
                RCC->APB1RSTR |=  RCC_APB1RSTR_SPI3RST;
                RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST;
                RCC->APB1ENR  &= ~RCC_APB1ENR_SPI3EN;
                break;
#endif
        }
}

//==============================================================================
/**
 * @brief Function apply new configuration for selected SPI
 *
 * @param vspi          virtual spi handler
 */
//==============================================================================
static void spi_apply_config(struct spi_virtual *vspi)
{
        SPI_t *SPI = spi[vspi->major];

        /* configure SPI divider */
        const u16_t divider_mask[SPI_CLK_DIV_256 + 1] = {
                [SPI_CLK_DIV_2  ] 0x00,
                [SPI_CLK_DIV_4  ] SPI_CR1_BR_0,
                [SPI_CLK_DIV_8  ] SPI_CR1_BR_1,
                [SPI_CLK_DIV_16 ] SPI_CR1_BR_1 | SPI_CR1_BR_0,
                [SPI_CLK_DIV_32 ] SPI_CR1_BR_2,
                [SPI_CLK_DIV_64 ] SPI_CR1_BR_2 | SPI_CR1_BR_0,
                [SPI_CLK_DIV_128] SPI_CR1_BR_2 | SPI_CR1_BR_1,
                [SPI_CLK_DIV_256] SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0,
        };

        CLEAR_BIT(SPI->CR1, SPI_CR1_BR);
        SET_BIT(SPI->CR1, divider_mask[vspi->config.clk_divider]);

        /* set SPI as master */
        SET_BIT(SPI->CR1, SPI_CR1_MSTR);

        /* set MSB/LSB */
        if (vspi->config.msb_first == false)
                SET_BIT(SPI->CR1, SPI_CR1_LSBFIRST);

        /* configure SPI mode */
        switch (vspi->config.mode) {
        case SPI_MODE_0:
                /* nothing to do */
                break;
        case SPI_MODE_1:
                SET_BIT(SPI->CR1, SPI_CR1_CPHA);
                break;
        case SPI_MODE_2:
                SET_BIT(SPI->CR1, SPI_CR1_CPOL);
                break;
        case SPI_MODE_3:
                SET_BIT(SPI->CR1, SPI_CR1_CPOL | SPI_CR1_CPHA);
                break;
        }

        /* enable peripheral */
        SET_BIT(SPI->CR1, SPI_CR1_SPE);
}

//==============================================================================
/**
 * @brief Function apply SPI device safe configuration
 *
 * @param major         SPI major number
 */
//==============================================================================
static void spi_apply_safe_config(u8_t major)
{
        SPI_t *SPI = spi[major];

        while (!(SPI->SR & SPI_SR_RXNE));
        while (!(SPI->SR & SPI_SR_TXE));
        while (SPI->SR & SPI_SR_BSY)
        CLEAR_BIT(SPI->CR1, SPI_CR1_SPE);
        SET_BIT(SPI->CR1, SPI_CR1_MSTR);
}

//==============================================================================
/**
 * @brief Function select slave device
 *
 * @param major         major device number (SPI device)
 * @param minor         minor device number (CS number)
 */
//==============================================================================
static void spi_select_slave(u8_t major, u8_t minor)
{
        GPIO_t *GPIO = spi_cs_pin_cfg[major][minor].port;
        u16_t   mask = spi_cs_pin_cfg[major][minor].pin_mask;

        GPIO->BRR = mask;
}

//==============================================================================
/**
 * @brief Function deselect current slave device
 *
 * @param major         major device number (SPI device)
 * @param minor         minor device number (CS number)
 */
//==============================================================================
static void spi_deselect_slave(u8_t major)
{
        for (int minor = 0; minor < spi_number_of_slaves[major]; minor++) {
                GPIO_t *GPIO = spi_cs_pin_cfg[major][minor].port;
                u16_t   mask = spi_cs_pin_cfg[major][minor].pin_mask;
                GPIO->BSRR   = mask;
        }
}

//==============================================================================
/**
 * @brief Function handle SPI IRQ
 *
 * @param major        spi device number
 */
//==============================================================================
static void spi_irq_handle(u8_t major)
{
        SPI_t *SPI = spi[major];

        if (spi_module->write[major]) {
                if (SPI->SR & SPI_SR_TXE) {
                        if (spi_module->count[major] > 0) {
                                SPI->DR = *(spi_module->buffer[major]++);
                                spi_module->count[major]--;
                        } else {
                                bool woken;
                                semaphore_signal_from_ISR(spi_module->wait_irq_sem[major], &woken);
                                CLEAR_BIT(SPI->CR2, SPI_CR2_TXEIE);
                        }
                }
        } else {
                if (SPI->SR & SPI_SR_RXNE) {
                        if (spi_module->count[major] > 0) {
                                *(spi_module->buffer[major]++) = SPI->DR;
                                SPI->DR = spi_module->dummy_byte[major];
                                spi_module->count[major]--;
                        } else {
                                bool woken;
                                semaphore_signal_from_ISR(spi_module->wait_irq_sem[major], &woken);
                                CLEAR_BIT(SPI->CR2, SPI_CR2_RXNEIE);
                        }
                }
        }
}

//==============================================================================
/**
 * @brief SPI1 IRQ handler
 */
//==============================================================================
#if defined(RCC_APB2ENR_SPI1EN) && (_SPI1_ENABLE > 0)
void SPI1_IRQHandler(void)
{
        spi_irq_handle(_SPI_DEV_1);
}
#endif

//==============================================================================
/**
 * @brief SPI2 IRQ handler
 */
//==============================================================================
#if defined(RCC_APB1ENR_SPI2EN) && (_SPI2_ENABLE > 0)
void SPI2_IRQHandler(void)
{
        spi_irq_handle(_SPI_DEV_2);
}
#endif

//==============================================================================
/**
 * @brief SPI3 IRQ handler
 */
//==============================================================================
#if defined(RCC_APB1ENR_SPI3EN) && (_SPI3_ENABLE > 0)
void SPI3_IRQHandler(void)
{
        spi_irq_handle(_SPI_DEV_3);
}
#endif

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
