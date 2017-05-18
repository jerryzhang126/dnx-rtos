/*=========================================================================*//**
@file    i2c_lld.c

@author  Daniel Zorychta

@brief   This driver support I2C peripherals.

@note    Copyright (C) 2017  Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation and modified by the dnx RTOS exception.

         NOTE: The modification  to the GPL is  included to allow you to
               distribute a combined work that includes dnx RTOS without
               being obliged to provide the source  code for proprietary
               components outside of the dnx RTOS.

         The dnx RTOS  is  distributed  in the hope  that  it will be useful,
         but WITHOUT  ANY  WARRANTY;  without  even  the implied  warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         Full license text is available on the following file: doc/license.txt.


*//*==========================================================================*/

// NOTE: 10-bit addressing mode is experimental and not tested!

/*==============================================================================
  Include files
==============================================================================*/
#include "drivers/driver.h"
#include "stm32f4/i2c_cfg.h"
#include "stm32f4/stm32f4xx.h"
#include "i2c_ioctl.h"
#include "i2c.h"
#include "lib/stm32f4xx_rcc.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define USE_DMA ((_I2C1_USE_DMA > 0) || (_I2C2_USE_DMA > 0) || (_I2C3_USE_DMA > 0))

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
/// type defines configuration of single I2C peripheral
typedef struct {
    #if USE_DMA > 0
        const bool                use_DMA;              //!< peripheral uses DMA and IRQ (true), or only IRQ (false)
        const DMA_Stream_TypeDef *const DMA_tx;         //!< pointer to the DMA Tx channel peripheral
        const DMA_Stream_TypeDef *const DMA_rx;         //!< pointer to the DMA Rx channel peripheral
        const u8_t                DMA_tx_number;        //!< number of channel of DMA Tx
        const u8_t                DMA_rx_number;        //!< number of channel of DMA Rx
        const u8_t                DMA_channel;          //!< DMA channel number
        const IRQn_Type           DMA_tx_IRQ_n;         //!< number of interrupt in the vector table
        const IRQn_Type           DMA_rx_IRQ_n;         //!< number of interrupt in the vector table
    #endif
        const I2C_TypeDef        *const I2C;            //!< pointer to the I2C peripheral
        const u32_t               freq;                 //!< peripheral SCL frequency [Hz]
        const u32_t               APB1ENR_clk_mask;     //!< mask used to enable I2C clock in the APB1ENR register
        const IRQn_Type           IRQ_EV_n;             //!< number of event IRQ vector
        const IRQn_Type           IRQ_ER_n;             //!< number of error IRQ vector
} I2C_info_t;

/*==============================================================================
  Local function prototypes
==============================================================================*/
static inline I2C_TypeDef *get_I2C(I2C_dev_t *hdl);
static void clear_send_address_event(I2C_dev_t *hdl);
static void IRQ_EV_handler(u8_t major);
static void IRQ_ER_handler(u8_t major);

/*==============================================================================
  Local object definitions
==============================================================================*/
/// peripherals configuration
static const I2C_info_t I2C_INFO[_I2C_NUMBER_OF_PERIPHERALS] = {
        #if defined(RCC_APB1ENR_I2C1EN)
        {
                #if USE_DMA > 0
                .use_DMA          = _I2C1_USE_DMA,
                .DMA_tx           = DMA1_Stream6,
                .DMA_rx           = DMA1_Stream7,
                .DMA_tx_number    = 6,
                .DMA_rx_number    = 5,
                .DMA_channel      = 1,
                .DMA_tx_IRQ_n     = DMA1_Stream6_IRQn,
                .DMA_rx_IRQ_n     = DMA1_Stream5_IRQn,
                #endif
                .I2C              = I2C1,
                .freq             = _I2C1_FREQUENCY,
                .APB1ENR_clk_mask = RCC_APB1ENR_I2C1EN,
                .IRQ_EV_n         = I2C1_EV_IRQn,
                .IRQ_ER_n         = I2C1_ER_IRQn,
        },
        #endif
        #if defined(RCC_APB1ENR_I2C2EN)
        {
                #if USE_DMA > 0
                .use_DMA          = _I2C2_USE_DMA,
                .DMA_tx           = DMA1_Stream7,
                .DMA_rx           = DMA1_Stream3,
                .DMA_tx_number    = 7,
                .DMA_rx_number    = 3,
                .DMA_channel      = 7,
                .DMA_tx_IRQ_n     = DMA1_Stream7_IRQn,
                .DMA_rx_IRQ_n     = DMA1_Stream3_IRQn,
                #endif
                .I2C              = I2C2,
                .freq             = _I2C2_FREQUENCY,
                .APB1ENR_clk_mask = RCC_APB1ENR_I2C2EN,
                .IRQ_EV_n         = I2C2_EV_IRQn,
                .IRQ_ER_n         = I2C2_ER_IRQn,
        },
        #endif
        #if defined(RCC_APB1ENR_I2C3EN)
        {
                #if USE_DMA > 0
                .use_DMA          = _I2C3_USE_DMA,
                .DMA_tx           = DMA1_Stream4,
                .DMA_rx           = DMA1_Stream2,
                .DMA_tx_number    = 4,
                .DMA_rx_number    = 2,
                .DMA_channel      = 3,
                .DMA_tx_IRQ_n     = DMA1_Stream4_IRQn,
                .DMA_rx_IRQ_n     = DMA1_Stream2_IRQn,
                #endif
                .I2C              = I2C3,
                .freq             = _I2C3_FREQUENCY,
                .APB1ENR_clk_mask = RCC_APB1ENR_I2C3EN,
                .IRQ_EV_n         = I2C3_EV_IRQn,
                .IRQ_ER_n         = I2C3_ER_IRQn,
        },
        #endif
};

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/
//==============================================================================
/**
 * @brief  Returns I2C address of current device
 * @param  hdl          device handle
 * @return I2C peripheral address
 */
//==============================================================================
static inline I2C_TypeDef *get_I2C(I2C_dev_t *hdl)
{
        return const_cast(I2C_TypeDef*, I2C_INFO[hdl->major].I2C);
}

//==============================================================================
/**
 * @brief  Clear all DMA IRQ flags (of tx and rx)
 * @param  major        peripheral major number
 * @return None
 */
//==============================================================================
#if USE_DMA > 0
static void clear_DMA_IRQ_flags(u8_t major)
{
        I2C_info_t         *cfg    = const_cast(I2C_info_t*, &I2C_INFO[major]);
        DMA_Stream_TypeDef *DMA_tx = const_cast(DMA_Stream_TypeDef*, cfg->DMA_tx);
        DMA_Stream_TypeDef *DMA_rx = const_cast(DMA_Stream_TypeDef*, cfg->DMA_rx);

        static const struct {
                __IO u32_t *IFCR;
                u32_t       mask;
        } flag[] = {
                    {.IFCR = &DMA1->LIFCR, .mask = (DMA_LIFCR_CFEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTCIF0)},
                    {.IFCR = &DMA1->LIFCR, .mask = (DMA_LIFCR_CFEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTCIF1)},
                    {.IFCR = &DMA1->LIFCR, .mask = (DMA_LIFCR_CFEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTCIF2)},
                    {.IFCR = &DMA1->LIFCR, .mask = (DMA_LIFCR_CFEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CTEIF3 | DMA_LIFCR_CHTIF3 | DMA_LIFCR_CTCIF3)},
                    {.IFCR = &DMA1->HIFCR, .mask = (DMA_HIFCR_CFEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTCIF4)},
                    {.IFCR = &DMA1->HIFCR, .mask = (DMA_HIFCR_CFEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTCIF5)},
                    {.IFCR = &DMA1->HIFCR, .mask = (DMA_HIFCR_CFEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTCIF6)},
                    {.IFCR = &DMA1->HIFCR, .mask = (DMA_HIFCR_CFEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTCIF7)},
        };

        *flag[cfg->DMA_tx_number].IFCR = flag[cfg->DMA_tx_number].mask;
        *flag[cfg->DMA_rx_number].IFCR = flag[cfg->DMA_rx_number].mask;

        CLEAR_BIT(DMA_tx->CR, DMA_SxCR_EN);
        CLEAR_BIT(DMA_rx->CR, DMA_SxCR_EN);
}
#endif

//==============================================================================
/**
 * @brief  Function handle error (try make the interface working)
 * @param  hdl          device handle
 * @return None
 */
//==============================================================================
static void reset(I2C_dev_t *hdl, bool reinit)
{
        I2C_TypeDef *i2c = get_I2C(hdl);

        CLEAR_BIT(i2c->CR2, I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);

        u8_t tmp = i2c->SR1;
             tmp = i2c->SR2;
             tmp = i2c->DR;
             tmp = i2c->DR;

        UNUSED_ARG1(tmp);

        i2c->SR1 = 0;

        sys_sleep_ms(1);

        if (reinit) {
                _I2C_LLD__init(hdl->major);
        }

        _I2C_LLD__stop(hdl);
}

//==============================================================================
/**
 * @brief  Function wait for selected event (IRQ)
 * @param  hdl                  device handle
 * @param  SR1_event_mask       event mask (bits from SR1 register)
 * @return On success true is returned, otherwise false
 */
//==============================================================================
static int wait_for_I2C_event(I2C_dev_t *hdl, u16_t SR1_event_mask)
{
        I2C_TypeDef *i2c = get_I2C(hdl);

        I2C[hdl->major]->SR1_mask = SR1_event_mask;
        I2C[hdl->major]->error    = 0;

        u16_t CR2 = I2C_CR2_ITEVTEN | I2C_CR2_ITERREN;

        if (SR1_event_mask & (I2C_SR1_RXNE | I2C_SR1_TXE)) {
                CR2 |= I2C_CR2_ITBUFEN;
        }

        SET_BIT(i2c->CR2, CR2);

        int err = sys_semaphore_wait(I2C[hdl->major]->event, _I2C_DEVICE_TIMEOUT);
        if (!err && I2C[hdl->major]->error) {
                err = EIO;
        }

        if (err) {
                reset(hdl, true);
        }

        return err;
}

//==============================================================================
/**
 * @brief  Function wait for DMA event (IRQ)
 * @param  hdl                  device handle
 * @param  DMA                  DMA channel
 * @return On success true is returned, otherwise false
 */
//==============================================================================
#if USE_DMA > 0
static bool wait_for_DMA_event(I2C_dev_t *hdl, DMA_Stream_TypeDef *DMA)
{
        I2C[hdl->major]->error = 0;

        I2C_TypeDef *i2c = get_I2C(hdl);
        CLEAR_BIT(i2c->CR2, I2C_CR2_ITBUFEN | I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);
        SET_BIT(i2c->CR2, I2C_CR2_DMAEN);
        SET_BIT(i2c->CR2, I2C_CR2_LAST);
        SET_BIT(DMA->CR, DMA_SxCR_EN);

        int err = sys_semaphore_wait(I2C[hdl->major]->event, _I2C_DEVICE_TIMEOUT);
        if (!err && I2C[hdl->major]->error) {
                err = EIO;
        }

        if (err) {
                reset(hdl, true);
        }

        return false;
}
#endif

//==============================================================================
/**
 * @brief  Clear event of send address
 * @param  hdl                  device handle
 * @return None
 */
//==============================================================================
static void clear_send_address_event(I2C_dev_t *hdl)
{
        I2C_TypeDef *i2c = get_I2C(hdl);

        sys_critical_section_begin();
        {
                u16_t tmp;
                tmp = i2c->SR1;
                tmp = i2c->SR2;
                (void)tmp;
        }
        sys_critical_section_end();
}

//==============================================================================
/**
 * @brief  Enables selected I2C peripheral according with configuration
 * @param  major        peripheral number
 * @return One of errno value.
 */
//==============================================================================
int _I2C_LLD__init(u8_t major)
{
        const I2C_info_t *cfg = &I2C_INFO[major];
        I2C_TypeDef      *i2c = const_cast(I2C_TypeDef*, I2C_INFO[major].I2C);

        SET_BIT(RCC->APB1RSTR, cfg->APB1ENR_clk_mask);
        CLEAR_BIT(RCC->APB1RSTR, cfg->APB1ENR_clk_mask);

        CLEAR_BIT(RCC->APB1ENR, cfg->APB1ENR_clk_mask);
        SET_BIT(RCC->APB1ENR, cfg->APB1ENR_clk_mask);

        RCC_ClocksTypeDef clocks;
        memset(&clocks, 0, sizeof(RCC_ClocksTypeDef));
        RCC_GetClocksFreq(&clocks);
        clocks.PCLK1_Frequency /= 1000000;

        if (clocks.PCLK1_Frequency < 2) {
                return EIO;
        }

        I2C[major]->use_DMA = false;

#if USE_DMA > 0
        if (cfg->use_DMA) {
                if (!(cfg->DMA_rx->CR & DMA_SxCR_EN) && !(cfg->DMA_tx->CR & DMA_SxCR_EN)) {
                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);

                        clear_DMA_IRQ_flags(major);

                        NVIC_EnableIRQ(cfg->DMA_tx_IRQ_n);
                        NVIC_EnableIRQ(cfg->DMA_rx_IRQ_n);
                        NVIC_SetPriority(cfg->DMA_tx_IRQ_n, _CPU_IRQ_SAFE_PRIORITY_);
                        NVIC_SetPriority(cfg->DMA_rx_IRQ_n, _CPU_IRQ_SAFE_PRIORITY_);

                        I2C[major]->use_DMA = true;
                }
        }
#endif

        NVIC_EnableIRQ(cfg->IRQ_EV_n);
        NVIC_EnableIRQ(cfg->IRQ_ER_n);
        NVIC_SetPriority(cfg->IRQ_EV_n, _CPU_IRQ_SAFE_PRIORITY_);
        NVIC_SetPriority(cfg->IRQ_ER_n, _CPU_IRQ_SAFE_PRIORITY_);

        u16_t CCR;
        if (cfg->freq <= 100000) {
                CCR = ((1000000/(2 * cfg->freq)) * clocks.PCLK1_Frequency) & I2C_CCR_CCR;
        } else if (cfg->freq < 400000){
                CCR  = ((10000000/(3 * cfg->freq)) * clocks.PCLK1_Frequency / 10) & I2C_CCR_CCR;
                CCR |= I2C_CCR_FS;
        } else {
                CCR  = ((100000000/(25 * cfg->freq)) * clocks.PCLK1_Frequency / 100 + 1) & I2C_CCR_CCR;
                CCR |= I2C_CCR_FS | I2C_CCR_DUTY;
        }

        i2c->CR1   = I2C_CR1_SWRST;
        i2c->CR1   = 0;
        i2c->CR2   = clocks.PCLK1_Frequency & I2C_CR2_FREQ;;
        i2c->CCR   = CCR;
        i2c->TRISE = clocks.PCLK1_Frequency + 1;
        i2c->CR1   = I2C_CR1_PE;

        I2C[major]->initialized = true;

        return ESUCC;
}

//==============================================================================
/**
 * @brief  Disables selected I2C peripheral
 * @param  major        I2C peripheral number
 * @return None
 */
//==============================================================================
void _I2C_LLD__release(u8_t major)
{
        const I2C_info_t *cfg = &I2C_INFO[major];
        I2C_TypeDef            *i2c = const_cast(I2C_TypeDef*, I2C_INFO[major].I2C);

#if USE_DMA > 0
        if (I2C[major]->use_DMA) {
                NVIC_DisableIRQ(cfg->DMA_tx_IRQ_n);
                NVIC_DisableIRQ(cfg->DMA_rx_IRQ_n);

                DMA_Stream_TypeDef *DMA_tx = const_cast(DMA_Stream_TypeDef*, cfg->DMA_tx);
                DMA_Stream_TypeDef *DMA_rx = const_cast(DMA_Stream_TypeDef*, cfg->DMA_rx);

                DMA_rx->CR = 0;
                DMA_tx->CR = 0;
        }
#endif

        NVIC_DisableIRQ(cfg->IRQ_EV_n);
        NVIC_DisableIRQ(cfg->IRQ_ER_n);

        WRITE_REG(i2c->CR1, 0);
        SET_BIT(RCC->APB1RSTR, cfg->APB1ENR_clk_mask);
        CLEAR_BIT(RCC->APB1RSTR, cfg->APB1ENR_clk_mask);
        CLEAR_BIT(RCC->APB1ENR, cfg->APB1ENR_clk_mask);

        I2C[major]->initialized = false;
}

//==============================================================================
/**
 * @brief  Function generate START sequence on I2C bus
 * @param  hdl                  device handle
 * @return On success true is returned, otherwise false
 */
//==============================================================================
int _I2C_LLD__start(I2C_dev_t *hdl)
{
        I2C_TypeDef *i2c = get_I2C(hdl);

        CLEAR_BIT(i2c->CR1, I2C_CR1_STOP);
        SET_BIT(i2c->CR1, I2C_CR1_START | I2C_CR1_ACK);

        return wait_for_I2C_event(hdl, I2C_SR1_SB);
}

//==============================================================================
/**
 * @brief  Function generate REPEAT START sequence on I2C bus
 * @param  hdl                  device handle
 * @return One of errno value.
 */
//==============================================================================
int _I2C_LLD__repeat_start(I2C_dev_t *hdl)
{
        I2C_TypeDef *i2c = get_I2C(hdl);

        CLEAR_BIT(i2c->CR1, I2C_CR1_STOP);
        SET_BIT(i2c->CR1, I2C_CR1_START | I2C_CR1_ACK);

        u32_t tref = sys_time_get_reference();

        while (!(i2c->SR1 & I2C_SR1_SB)) {
                if (sys_time_is_expired(tref, _I2C_DEVICE_TIMEOUT)) {
                        return EIO;
                }

                if (i2c->SR1 & (I2C_SR1_ARLO | I2C_SR1_BERR)) {
                        return EIO;
                }
        }

        return ESUCC;
}

//==============================================================================
/**
 * @brief  Function generate STOP sequence on I2C bus
 * @param  hdl                  device handle
 * @return On success true is returned, otherwise false
 */
//==============================================================================
void _I2C_LLD__stop(I2C_dev_t *hdl)
{
        I2C_TypeDef *i2c = get_I2C(hdl);

        SET_BIT(i2c->CR1, I2C_CR1_STOP);
}

//==============================================================================
/**
 * @brief  Function send I2C address sequence
 * @param  hdl                  device handle
 * @param  write                true: compose write address
 * @return On success true is returned, otherwise false
 */
//==============================================================================
int _I2C_LLD__send_address(I2C_dev_t *hdl, bool write)
{
        int err = EIO;

        I2C_TypeDef *i2c = get_I2C(hdl);

        if (hdl->config.addr_10bit) {
                u8_t hdr = _I2C_HEADER_ADDR_10BIT | ((hdl->config.address >> 7) & 0x6);

                // send header + 2 most significant bits of 10-bit address
                i2c->DR = _I2C_HEADER_ADDR_10BIT | ((hdl->config.address & 0xFE) >> 7);
                err = wait_for_I2C_event(hdl, I2C_SR1_ADD10);
                if (err) goto finish;

                // send rest 8 bits of 10-bit address
                u8_t  addr = hdl->config.address & 0xFF;
                u16_t tmp  = i2c->SR1;
                (void)tmp;   i2c->DR = addr;
                err = wait_for_I2C_event(hdl, I2C_SR1_ADDR);
                if (err) goto finish;

                clear_send_address_event(hdl);

                // send repeat start
                err = _I2C_LLD__start(hdl);
                if (err) goto finish;

                // send header
                i2c->DR = write ? hdr : hdr | 0x01;
                err = wait_for_I2C_event(hdl, I2C_SR1_ADDR);

        } else {
                u16_t tmp = i2c->SR1;
                      tmp = i2c->SR2;
                (void)tmp;
                i2c->DR = write ? (hdl->config.address & 0xFE) : (hdl->config.address | 0x01);
                err = wait_for_I2C_event(hdl, I2C_SR1_ADDR);
        }

        finish:
        if (!err) {
                clear_send_address_event(hdl);
        }

        return err;
}

//==============================================================================
/**
 * @brief  Function receive bytes from I2C bus (master-receiver)
 * @param  hdl                  device handle
 * @param  dst                  destination buffer
 * @param  count                number of bytes to receive
 * @return Number of received bytes
 */
//==============================================================================
int _I2C_LLD__receive(I2C_dev_t *hdl, u8_t *dst, size_t count, size_t *rdcnt)
{
        int          err = EIO;
        ssize_t      n   = 0;
        I2C_TypeDef *i2c = get_I2C(hdl);

        if (count == 2) {
                CLEAR_BIT(i2c->CR1, I2C_CR1_ACK);
                SET_BIT(i2c->CR1, I2C_CR1_POS);
        } else {
                CLEAR_BIT(i2c->CR1, I2C_CR1_POS);
                SET_BIT(i2c->CR1, I2C_CR1_ACK);
        }

        if (count >= 3) {
                clear_send_address_event(hdl);

#if USE_DMA > 0
                if (I2C[hdl->major]->use_DMA) {
                        DMA_Stream_TypeDef *DMA = const_cast(DMA_Stream_TypeDef*,
                                                             I2C_INFO[hdl->major].DMA_rx);

                        DMA->PAR  = cast(u32_t, &i2c->DR);
                        DMA->M0AR = cast(u32_t, dst);
                        DMA->NDTR = count;

                        clear_DMA_IRQ_flags(hdl->major);

                        DMA->CR = ((I2C_INFO[hdl->major].DMA_channel & 7) << DMA_SxCR_CHSEL_Pos)
                                | DMA_SxCR_MINC | DMA_SxCR_TCIE | DMA_SxCR_TEIE;

                        err = wait_for_DMA_event(hdl, DMA);
                        if (!err) {
                                n = count;
                        }
                } else
#endif
                {
                        while (count) {
                                if (count == 3) {
                                        err = wait_for_I2C_event(hdl, I2C_SR1_BTF);
                                        if (err) break;

                                        CLEAR_BIT(i2c->CR1, I2C_CR1_ACK);
                                        *dst++ = i2c->DR;
                                        n++;

                                        err = wait_for_I2C_event(hdl, I2C_SR1_RXNE);
                                        if (err) break;

                                        _I2C_LLD__stop(hdl);
                                        *dst++ = i2c->DR;
                                        n++;

                                        err = wait_for_I2C_event(hdl, I2C_SR1_RXNE);
                                        if (err) break;

                                        *dst++ = i2c->DR;
                                        n++;

                                        count = 0;
                                } else {
                                        err = wait_for_I2C_event(hdl, I2C_SR1_RXNE);
                                        if (!err) {
                                                *dst++ = i2c->DR;
                                                count--;
                                                n++;
                                        } else {
                                                break;
                                        }
                                }
                        }
                }

        } else if (count == 2) {
                sys_critical_section_begin();
                {
                        clear_send_address_event(hdl);
                        CLEAR_BIT(i2c->CR1, I2C_CR1_ACK);
                }
                sys_critical_section_end();

                err = wait_for_I2C_event(hdl, I2C_SR1_BTF);
                if (!err) {
                        _I2C_LLD__stop(hdl);

                        *dst++ = i2c->DR;
                        *dst++ = i2c->DR;
                        n     += 2;
                }

        } else if (count == 1) {
                CLEAR_BIT(i2c->CR1, I2C_CR1_ACK);
                clear_send_address_event(hdl);
                _I2C_LLD__stop(hdl);

                err = wait_for_I2C_event(hdl, I2C_SR1_RXNE);
                if (!err) {
                        *dst++ = i2c->DR;
                        n     += 1;
                }
        }

        _I2C_LLD__stop(hdl);

        *rdcnt = n;

        return err;
}

//==============================================================================
/**
 * @brief  Function transmit selected amount bytes to I2C bus
 * @param  hdl                  device handle
 * @param  src                  data source
 * @param  count                number of bytes to transfer
 * @return Number of written bytes
 */
//==============================================================================
int _I2C_LLD__transmit(I2C_dev_t *hdl, const u8_t *src, size_t count, size_t *wrcnt)
{
        int          err  = EIO;
        ssize_t      n    = 0;
        I2C_TypeDef *i2c  = get_I2C(hdl);

        clear_send_address_event(hdl);

#if USE_DMA > 0
        if (count >= 3 && I2C[hdl->major]->use_DMA) {
                DMA_Stream_TypeDef *DMA = const_cast(DMA_Stream_TypeDef*,
                                                     I2C_INFO[hdl->major].DMA_tx);

                DMA->PAR  = cast(u32_t, &i2c->DR);
                DMA->M0AR = cast(u32_t, src);
                DMA->NDTR = count;

                DMA->CR = ((I2C_INFO[hdl->major].DMA_channel & 7) << DMA_SxCR_CHSEL_Pos)
                        | DMA_SxCR_MINC | DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DIR_0;

                err = wait_for_DMA_event(hdl, DMA);
                if (!err) {
                        n = count;
                }
        } else
#endif
        {
                while (count) {
                        err = wait_for_I2C_event(hdl, I2C_SR1_TXE);
                        if (!err) {
                                i2c->DR = *src++;
                        } else {
                                break;
                        }

                        n++;
                        count--;
                }
        }

        if (n && !err) {
                err = wait_for_I2C_event(hdl, I2C_SR1_BTF);
                if (err) {
                        n = n - 1;
                }
        }

        *wrcnt = n;

        return err;
}

//==============================================================================
/**
 * @brief  Event IRQ handler (transaction state machine)
 * @param  major        number of peripheral
 * @return If IRQ was woken then true is returned, otherwise false
 */
//==============================================================================
static void IRQ_EV_handler(u8_t major)
{
        bool woken = false;

        I2C_TypeDef *i2c = const_cast(I2C_TypeDef*, I2C_INFO[major].I2C);
        u16_t SR1 = i2c->SR1;
        u16_t SR2 = i2c->SR2;
        UNUSED_ARG1(SR2);

        if (SR1 & I2C[major]->SR1_mask) {
                sys_semaphore_signal_from_ISR(I2C[major]->event, &woken);
                CLEAR_BIT(i2c->CR2, I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
                I2C[major]->unexp_event_cnt = 0;
        } else {
                /*
                 * This counter is used to check if there is no death loop of
                 * not handled IRQ. If counter reach specified value then
                 * the error flag is set.
                 */
                if (++I2C[major]->unexp_event_cnt >= 16) {
                        I2C[major]->error = EIO;
                        sys_semaphore_signal_from_ISR(I2C[major]->event, &woken);
                        CLEAR_BIT(I2C1->CR2, I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
                }
        }

        sys_thread_yield_from_ISR(woken);
}

//==============================================================================
/**
 * @brief  Error IRQ handler
 * @param  major        number of peripheral
 * @return If IRQ was woken then true is returned, otherwise false
 */
//==============================================================================
static void IRQ_ER_handler(u8_t major)
{
        I2C_TypeDef *i2c = const_cast(I2C_TypeDef*, I2C_INFO[major].I2C);
        u16_t SR1 = i2c->SR1;
        u16_t SR2 = i2c->SR2;
        UNUSED_ARG1(SR2);

        if (SR1 & I2C_SR1_ARLO) {
                I2C[major]->error = EAGAIN;

        } else if (SR1 & I2C_SR1_AF) {
                if (I2C[major]->SR1_mask & (I2C_SR1_ADDR | I2C_SR1_ADD10))
                        I2C[major]->error = ENXIO;
                else
                        I2C[major]->error = EIO;
        } else {
                I2C[major]->error = EIO;
        }

        // clear error flags
        i2c->SR1 = 0;

        bool woken = false;
        sys_semaphore_signal_from_ISR(I2C[major]->event, &woken);

        CLEAR_BIT(i2c->CR2, I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
        sys_thread_yield_from_ISR(woken);
}

//==============================================================================
/**
 * @brief  DMA IRQ handler
 * @param  DMA_ch_no    DMA channel number
 * @param  major        number of peripheral
 * @return If IRQ was woken then true is returned, otherwise false
 */
//==============================================================================
#if USE_DMA > 0
static void IRQ_DMA_handler(const int DMA_ch_no, u8_t major)
{
        static const struct {
                __IO u32_t *IFSR;
                u32_t       mask;
        } flag[] = {
                    {.IFSR = &DMA1->LISR, .mask = DMA_LISR_TEIF0},
                    {.IFSR = &DMA1->LISR, .mask = DMA_LISR_TEIF1},
                    {.IFSR = &DMA1->LISR, .mask = DMA_LISR_TEIF2},
                    {.IFSR = &DMA1->LISR, .mask = DMA_LISR_TEIF3},
                    {.IFSR = &DMA1->HISR, .mask = DMA_HISR_TEIF4},
                    {.IFSR = &DMA1->HISR, .mask = DMA_HISR_TEIF5},
                    {.IFSR = &DMA1->HISR, .mask = DMA_HISR_TEIF6},
                    {.IFSR = &DMA1->HISR, .mask = DMA_HISR_TEIF7},
        };

        I2C[major]->error = (*flag[DMA_ch_no].IFSR & flag[DMA_ch_no].mask) ? EIO : ESUCC;

        bool woken = false;
        sys_semaphore_signal_from_ISR(I2C[major]->event, &woken);

        clear_DMA_IRQ_flags(major);
        sys_thread_yield_from_ISR(woken);
}
#endif

//==============================================================================
/**
 * @brief  I2C1 Event IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
#if defined(RCC_APB1ENR_I2C1EN)
void I2C1_EV_IRQHandler(void)
{
        IRQ_EV_handler(_I2C1);
}
#endif

//==============================================================================
/**
 * @brief  I2C1 Error IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
#if defined(RCC_APB1ENR_I2C1EN)
void I2C1_ER_IRQHandler(void)
{
        IRQ_ER_handler(_I2C1);
}
#endif

//==============================================================================
/**
 * @brief  I2C2 Event IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
#if defined(RCC_APB1ENR_I2C2EN)
void I2C2_EV_IRQHandler(void)
{
        IRQ_EV_handler(_I2C2);
}
#endif

//==============================================================================
/**
 * @brief  I2C2 Error IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
#if defined(RCC_APB1ENR_I2C2EN)
void I2C2_ER_IRQHandler(void)
{
        IRQ_ER_handler(_I2C2);
}
#endif

//==============================================================================
/**
 * @brief  I2C3 Event IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
#if defined(RCC_APB1ENR_I2C3EN)
void I2C3_EV_IRQHandler(void)
{
        IRQ_EV_handler(_I2C3);
}
#endif

//==============================================================================
/**
 * @brief  I2C3 Error IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
#if defined(RCC_APB1ENR_I2C3EN)
void I2C3_ER_IRQHandler(void)
{
        IRQ_ER_handler(_I2C3);
}
#endif

#if defined(RCC_APB1ENR_I2C1EN) && (_I2C1_USE_DMA > 0)
//==============================================================================
/**
 * @brief  I2C1 DMA Tx Complete IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
void DMA1_Stream6_IRQHandler(void)
{
        IRQ_DMA_handler(I2C_INFO[_I2C1].DMA_tx_number, _I2C1);
}

//==============================================================================
/**
 * @brief  I2C1 DMA Rx Complete IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
void DMA1_Stream5_IRQHandler(void)
{
        IRQ_DMA_handler(I2C_INFO[_I2C1].DMA_rx_number, _I2C1);
}
#endif

#if defined(RCC_APB1ENR_I2C2EN) && (_I2C2_USE_DMA > 0)
//==============================================================================
/**
 * @brief  I2C2 DMA Tx Complete IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
void DMA1_Stream7_IRQHandler(void)
{
        IRQ_DMA_handler(I2C_INFO[_I2C2].DMA_tx_number, _I2C2);
}

//==============================================================================
/**
 * @brief  I2C2 DMA Rx Complete IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
void DMA1_Stream3_IRQHandler(void)
{
        IRQ_DMA_handler(I2C_INFO[_I2C2].DMA_rx_number, _I2C2);
}
#endif

#if defined(RCC_APB1ENR_I2C3EN) && (_I2C3_USE_DMA > 0)
//==============================================================================
/**
 * @brief  I2C2 DMA Tx Complete IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
void DMA1_Stream4_IRQHandler(void)
{
        IRQ_DMA_handler(I2C_INFO[_I2C3].DMA_tx_number, _I2C3);
}

//==============================================================================
/**
 * @brief  I2C2 DMA Rx Complete IRQ handler
 * @param  None
 * @return None
 */
//==============================================================================
void DMA1_Stream2_IRQHandler(void)
{
        IRQ_DMA_handler(I2C_INFO[_I2C3].DMA_rx_number, _I2C3);
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
