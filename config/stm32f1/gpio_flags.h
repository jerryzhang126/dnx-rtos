/*=========================================================================*//**
@file    gpio_flags.h

@author  Daniel Zorychta

@brief   GPIO module configuration flags.

@note    Copyright (C) 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

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

/*
 * NOTE: All flags defined as: __FLAG_NAME__ (with doubled underscore as suffix
 *       and prefix) are exported to the single configuration file
 *       (by using Configtool) when entire project configuration is exported.
 *       All other flag definitions and statements are ignored.
 */

#ifndef _GPIO_FLAGS_H_
#define _GPIO_FLAGS_H_

#define __GPIO_PA_ENABLE__ _YES_
#define __GPIO_PB_ENABLE__ _YES_
#define __GPIO_PC_ENABLE__ _YES_
#define __GPIO_PD_ENABLE__ _YES_
#define __GPIO_PE_ENABLE__ _NO_
#define __GPIO_PF_ENABLE__ _NO_
#define __GPIO_PG_ENABLE__ _NO_

#define __GPIO_PA_PIN_0_NAME__ ETH_MII_CRS_WKUP
#define __GPIO_PA_PIN_0_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_0_STATE__ _FLOAT
#define __GPIO_PA_PIN_1_NAME__ ETH_MII_RX_CLK
#define __GPIO_PA_PIN_1_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_1_STATE__ _FLOAT
#define __GPIO_PA_PIN_2_NAME__ ETH_MDIO
#define __GPIO_PA_PIN_2_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PA_PIN_2_STATE__ _LOW
#define __GPIO_PA_PIN_3_NAME__ ETH_MII_COL
#define __GPIO_PA_PIN_3_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_3_STATE__ _FLOAT
#define __GPIO_PA_PIN_4_NAME__ SD_CS
#define __GPIO_PA_PIN_4_MODE__ _GPIO_OUT_PUSH_PULL_50MHZ
#define __GPIO_PA_PIN_4_STATE__ _HIGH
#define __GPIO_PA_PIN_5_NAME__ ADC12_IN5
#define __GPIO_PA_PIN_5_MODE__ _GPIO_ANALOG
#define __GPIO_PA_PIN_5_STATE__ _FLOAT
#define __GPIO_PA_PIN_6_NAME__ ADC12_IN6
#define __GPIO_PA_PIN_6_MODE__ _GPIO_ANALOG
#define __GPIO_PA_PIN_6_STATE__ _FLOAT
#define __GPIO_PA_PIN_7_NAME__ ETH_MII_RX_DV
#define __GPIO_PA_PIN_7_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_7_STATE__ _FLOAT
#define __GPIO_PA_PIN_8_NAME__ TP204
#define __GPIO_PA_PIN_8_MODE__ _GPIO_OUT_PUSH_PULL_50MHZ
#define __GPIO_PA_PIN_8_STATE__ _LOW
#define __GPIO_PA_PIN_9_NAME__ USART1_TX
#define __GPIO_PA_PIN_9_MODE__ _GPIO_ALT_OUT_PUSH_PULL_2MHZ
#define __GPIO_PA_PIN_9_STATE__ _HIGH
#define __GPIO_PA_PIN_10_NAME__ USART1_RX
#define __GPIO_PA_PIN_10_MODE__ _GPIO_IN_PULLED
#define __GPIO_PA_PIN_10_STATE__ _HIGH
#define __GPIO_PA_PIN_11_NAME__ USART1_CTS
#define __GPIO_PA_PIN_11_MODE__ _GPIO_IN_PULLED
#define __GPIO_PA_PIN_11_STATE__ _HIGH
#define __GPIO_PA_PIN_12_NAME__ USART1_RTS
#define __GPIO_PA_PIN_12_MODE__ _GPIO_ALT_OUT_PUSH_PULL_2MHZ
#define __GPIO_PA_PIN_12_STATE__ _HIGH
#define __GPIO_PA_PIN_13_NAME__ JTMS
#define __GPIO_PA_PIN_13_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_13_STATE__ _FLOAT
#define __GPIO_PA_PIN_14_NAME__ JTCK
#define __GPIO_PA_PIN_14_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_14_STATE__ _FLOAT
#define __GPIO_PA_PIN_15_NAME__ JTDI
#define __GPIO_PA_PIN_15_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PA_PIN_15_STATE__ _FLOAT
#define __GPIO_PB_PIN_0_NAME__ ETH_MII_RXD2
#define __GPIO_PB_PIN_0_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PB_PIN_0_STATE__ _FLOAT
#define __GPIO_PB_PIN_1_NAME__ ETH_MII_RXD3
#define __GPIO_PB_PIN_1_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PB_PIN_1_STATE__ _FLOAT
#define __GPIO_PB_PIN_2_NAME__ BOOT1
#define __GPIO_PB_PIN_2_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PB_PIN_2_STATE__ _FLOAT
#define __GPIO_PB_PIN_3_NAME__ JTDO
#define __GPIO_PB_PIN_3_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PB_PIN_3_STATE__ _FLOAT
#define __GPIO_PB_PIN_4_NAME__ JTRST
#define __GPIO_PB_PIN_4_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PB_PIN_4_STATE__ _FLOAT
#define __GPIO_PB_PIN_5_NAME__ TP210
#define __GPIO_PB_PIN_5_MODE__ _GPIO_IN_PULLED
#define __GPIO_PB_PIN_5_STATE__ _LOW
#define __GPIO_PB_PIN_6_NAME__ I2C1_SCL
#define __GPIO_PB_PIN_6_MODE__ _GPIO_ALT_OUT_OPEN_DRAIN_2MHZ
#define __GPIO_PB_PIN_6_STATE__ _HIGH
#define __GPIO_PB_PIN_7_NAME__ I2C1_SDA
#define __GPIO_PB_PIN_7_MODE__ _GPIO_ALT_OUT_OPEN_DRAIN_2MHZ
#define __GPIO_PB_PIN_7_STATE__ _HIGH
#define __GPIO_PB_PIN_8_NAME__ ETH_MII_TXD3
#define __GPIO_PB_PIN_8_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PB_PIN_8_STATE__ _LOW
#define __GPIO_PB_PIN_9_NAME__ TP211
#define __GPIO_PB_PIN_9_MODE__ _GPIO_IN_PULLED
#define __GPIO_PB_PIN_9_STATE__ _LOW
#define __GPIO_PB_PIN_10_NAME__ ETH_MII_RX_ER
#define __GPIO_PB_PIN_10_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PB_PIN_10_STATE__ _FLOAT
#define __GPIO_PB_PIN_11_NAME__ ETH_MII_TX_EN
#define __GPIO_PB_PIN_11_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PB_PIN_11_STATE__ _LOW
#define __GPIO_PB_PIN_12_NAME__ ETH_MII_TXD0
#define __GPIO_PB_PIN_12_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PB_PIN_12_STATE__ _LOW
#define __GPIO_PB_PIN_13_NAME__ ETH_MII_TXD1
#define __GPIO_PB_PIN_13_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PB_PIN_13_STATE__ _LOW
#define __GPIO_PB_PIN_14_NAME__ TP212
#define __GPIO_PB_PIN_14_MODE__ _GPIO_IN_PULLED
#define __GPIO_PB_PIN_14_STATE__ _LOW
#define __GPIO_PB_PIN_15_NAME__ TP213
#define __GPIO_PB_PIN_15_MODE__ _GPIO_IN_PULLED
#define __GPIO_PB_PIN_15_STATE__ _LOW
#define __GPIO_PC_PIN_0_NAME__ ETH_PWRON_IRQ
#define __GPIO_PC_PIN_0_MODE__ _GPIO_OUT_OPEN_DRAIN_2MHZ
#define __GPIO_PC_PIN_0_STATE__ _HIGH
#define __GPIO_PC_PIN_1_NAME__ ETH_MDC
#define __GPIO_PC_PIN_1_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_1_STATE__ _LOW
#define __GPIO_PC_PIN_2_NAME__ ETH_MII_TXD2
#define __GPIO_PC_PIN_2_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_2_STATE__ _LOW
#define __GPIO_PC_PIN_3_NAME__ ETH_MII_TX_CLK
#define __GPIO_PC_PIN_3_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PC_PIN_3_STATE__ _FLOAT
#define __GPIO_PC_PIN_4_NAME__ ETH_MII_RXD0
#define __GPIO_PC_PIN_4_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PC_PIN_4_STATE__ _FLOAT
#define __GPIO_PC_PIN_5_NAME__ ETH_MII_RXD1
#define __GPIO_PC_PIN_5_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PC_PIN_5_STATE__ _FLOAT
#define __GPIO_PC_PIN_6_NAME__ TP218
#define __GPIO_PC_PIN_6_MODE__ _GPIO_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_6_STATE__ _LOW
#define __GPIO_PC_PIN_7_NAME__ TP219
#define __GPIO_PC_PIN_7_MODE__ _GPIO_IN_PULLED
#define __GPIO_PC_PIN_7_STATE__ _LOW
#define __GPIO_PC_PIN_8_NAME__ TP220
#define __GPIO_PC_PIN_8_MODE__ _GPIO_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_8_STATE__ _LOW
#define __GPIO_PC_PIN_9_NAME__ TP221
#define __GPIO_PC_PIN_9_MODE__ _GPIO_IN_PULLED
#define __GPIO_PC_PIN_9_STATE__ _LOW
#define __GPIO_PC_PIN_10_NAME__ SD_SCK
#define __GPIO_PC_PIN_10_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_10_STATE__ _LOW
#define __GPIO_PC_PIN_11_NAME__ SD_MISO
#define __GPIO_PC_PIN_11_MODE__ _GPIO_IN_PULLED
#define __GPIO_PC_PIN_11_STATE__ _HIGH
#define __GPIO_PC_PIN_12_NAME__ SD_MOSI
#define __GPIO_PC_PIN_12_MODE__ _GPIO_ALT_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_12_STATE__ _LOW
#define __GPIO_PC_PIN_13_NAME__ TP222
#define __GPIO_PC_PIN_13_MODE__ _GPIO_IN_PULLED
#define __GPIO_PC_PIN_13_STATE__ _LOW
#define __GPIO_PC_PIN_14_NAME__ TP223
#define __GPIO_PC_PIN_14_MODE__ _GPIO_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_14_STATE__ _LOW
#define __GPIO_PC_PIN_15_NAME__ TP224
#define __GPIO_PC_PIN_15_MODE__ _GPIO_OUT_PUSH_PULL_50MHZ
#define __GPIO_PC_PIN_15_STATE__ _LOW
#define __GPIO_PD_PIN_0_NAME__ OSC_IN
#define __GPIO_PD_PIN_0_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PD_PIN_0_STATE__ _FLOAT
#define __GPIO_PD_PIN_1_NAME__ OSC_OUT
#define __GPIO_PD_PIN_1_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PD_PIN_1_STATE__ _FLOAT
#define __GPIO_PD_PIN_2_NAME__ TP216
#define __GPIO_PD_PIN_2_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_2_STATE__ _LOW
#define __GPIO_PD_PIN_3_NAME__ NC_GPIOD_3
#define __GPIO_PD_PIN_3_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_3_STATE__ _LOW
#define __GPIO_PD_PIN_4_NAME__ NC_GPIOD_4
#define __GPIO_PD_PIN_4_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_4_STATE__ _LOW
#define __GPIO_PD_PIN_5_NAME__ NC_GPIOD_5
#define __GPIO_PD_PIN_5_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_5_STATE__ _LOW
#define __GPIO_PD_PIN_6_NAME__ NC_GPIOD_6
#define __GPIO_PD_PIN_6_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_6_STATE__ _LOW
#define __GPIO_PD_PIN_7_NAME__ NC_GPIOD_7
#define __GPIO_PD_PIN_7_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_7_STATE__ _LOW
#define __GPIO_PD_PIN_8_NAME__ NC_GPIOD_8
#define __GPIO_PD_PIN_8_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_8_STATE__ _LOW
#define __GPIO_PD_PIN_9_NAME__ NC_GPIOD_9
#define __GPIO_PD_PIN_9_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_9_STATE__ _LOW
#define __GPIO_PD_PIN_10_NAME__ NC_GPIOD_10
#define __GPIO_PD_PIN_10_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_10_STATE__ _LOW
#define __GPIO_PD_PIN_11_NAME__ NC_GPIOD_11
#define __GPIO_PD_PIN_11_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_11_STATE__ _LOW
#define __GPIO_PD_PIN_12_NAME__ NC_GPIOD_12
#define __GPIO_PD_PIN_12_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_12_STATE__ _LOW
#define __GPIO_PD_PIN_13_NAME__ NC_GPIOD_13
#define __GPIO_PD_PIN_13_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_13_STATE__ _LOW
#define __GPIO_PD_PIN_14_NAME__ NC_GPIOD_14
#define __GPIO_PD_PIN_14_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_14_STATE__ _LOW
#define __GPIO_PD_PIN_15_NAME__ NC_GPIOD_15
#define __GPIO_PD_PIN_15_MODE__ _GPIO_IN_PULLED
#define __GPIO_PD_PIN_15_STATE__ _LOW
#define __GPIO_PE_PIN_0_NAME__ NC_GPIOE_0
#define __GPIO_PE_PIN_0_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_0_STATE__ _FLOAT
#define __GPIO_PE_PIN_1_NAME__ NC_GPIOE_1
#define __GPIO_PE_PIN_1_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_1_STATE__ _FLOAT
#define __GPIO_PE_PIN_2_NAME__ NC_GPIOE_2
#define __GPIO_PE_PIN_2_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_2_STATE__ _FLOAT
#define __GPIO_PE_PIN_3_NAME__ NC_GPIOE_3
#define __GPIO_PE_PIN_3_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_3_STATE__ _FLOAT
#define __GPIO_PE_PIN_4_NAME__ NC_GPIOE_4
#define __GPIO_PE_PIN_4_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_4_STATE__ _FLOAT
#define __GPIO_PE_PIN_5_NAME__ NC_GPIOE_5
#define __GPIO_PE_PIN_5_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_5_STATE__ _FLOAT
#define __GPIO_PE_PIN_6_NAME__ NC_GPIOE_6
#define __GPIO_PE_PIN_6_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_6_STATE__ _FLOAT
#define __GPIO_PE_PIN_7_NAME__ NC_GPIOE_7
#define __GPIO_PE_PIN_7_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_7_STATE__ _FLOAT
#define __GPIO_PE_PIN_8_NAME__ NC_GPIOE_8
#define __GPIO_PE_PIN_8_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_8_STATE__ _FLOAT
#define __GPIO_PE_PIN_9_NAME__ NC_GPIOE_9
#define __GPIO_PE_PIN_9_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_9_STATE__ _FLOAT
#define __GPIO_PE_PIN_10_NAME__ NC_GPIOE_10
#define __GPIO_PE_PIN_10_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_10_STATE__ _FLOAT
#define __GPIO_PE_PIN_11_NAME__ NC_GPIOE_11
#define __GPIO_PE_PIN_11_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_11_STATE__ _FLOAT
#define __GPIO_PE_PIN_12_NAME__ NC_GPIOE_12
#define __GPIO_PE_PIN_12_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_12_STATE__ _FLOAT
#define __GPIO_PE_PIN_13_NAME__ NC_GPIOE_13
#define __GPIO_PE_PIN_13_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_13_STATE__ _FLOAT
#define __GPIO_PE_PIN_14_NAME__ NC_GPIOE_14
#define __GPIO_PE_PIN_14_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_14_STATE__ _FLOAT
#define __GPIO_PE_PIN_15_NAME__ NC_GPIOE_15
#define __GPIO_PE_PIN_15_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PE_PIN_15_STATE__ _FLOAT
#define __GPIO_PF_PIN_0_NAME__ NC_GPIOF_0
#define __GPIO_PF_PIN_0_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_0_STATE__ _FLOAT
#define __GPIO_PF_PIN_1_NAME__ NC_GPIOF_1
#define __GPIO_PF_PIN_1_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_1_STATE__ _FLOAT
#define __GPIO_PF_PIN_2_NAME__ NC_GPIOF_2
#define __GPIO_PF_PIN_2_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_2_STATE__ _FLOAT
#define __GPIO_PF_PIN_3_NAME__ NC_GPIOF_3
#define __GPIO_PF_PIN_3_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_3_STATE__ _FLOAT
#define __GPIO_PF_PIN_4_NAME__ NC_GPIOF_4
#define __GPIO_PF_PIN_4_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_4_STATE__ _FLOAT
#define __GPIO_PF_PIN_5_NAME__ NC_GPIOF_5
#define __GPIO_PF_PIN_5_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_5_STATE__ _FLOAT
#define __GPIO_PF_PIN_6_NAME__ NC_GPIOF_6
#define __GPIO_PF_PIN_6_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_6_STATE__ _FLOAT
#define __GPIO_PF_PIN_7_NAME__ NC_GPIOF_7
#define __GPIO_PF_PIN_7_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_7_STATE__ _FLOAT
#define __GPIO_PF_PIN_8_NAME__ NC_GPIOF_8
#define __GPIO_PF_PIN_8_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_8_STATE__ _FLOAT
#define __GPIO_PF_PIN_9_NAME__ NC_GPIOF_9
#define __GPIO_PF_PIN_9_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_9_STATE__ _FLOAT
#define __GPIO_PF_PIN_10_NAME__ NC_GPIOF_10
#define __GPIO_PF_PIN_10_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_10_STATE__ _FLOAT
#define __GPIO_PF_PIN_11_NAME__ NC_GPIOF_11
#define __GPIO_PF_PIN_11_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_11_STATE__ _FLOAT
#define __GPIO_PF_PIN_12_NAME__ NC_GPIOF_12
#define __GPIO_PF_PIN_12_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_12_STATE__ _FLOAT
#define __GPIO_PF_PIN_13_NAME__ NC_GPIOF_13
#define __GPIO_PF_PIN_13_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_13_STATE__ _FLOAT
#define __GPIO_PF_PIN_14_NAME__ NC_GPIOF_14
#define __GPIO_PF_PIN_14_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_14_STATE__ _FLOAT
#define __GPIO_PF_PIN_15_NAME__ NC_GPIOF_15
#define __GPIO_PF_PIN_15_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PF_PIN_15_STATE__ _FLOAT
#define __GPIO_PG_PIN_0_NAME__ NC_GPIOG_0
#define __GPIO_PG_PIN_0_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_0_STATE__ _FLOAT
#define __GPIO_PG_PIN_1_NAME__ NC_GPIOG_1
#define __GPIO_PG_PIN_1_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_1_STATE__ _FLOAT
#define __GPIO_PG_PIN_2_NAME__ NC_GPIOG_2
#define __GPIO_PG_PIN_2_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_2_STATE__ _FLOAT
#define __GPIO_PG_PIN_3_NAME__ NC_GPIOG_3
#define __GPIO_PG_PIN_3_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_3_STATE__ _FLOAT
#define __GPIO_PG_PIN_4_NAME__ NC_GPIOG_4
#define __GPIO_PG_PIN_4_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_4_STATE__ _FLOAT
#define __GPIO_PG_PIN_5_NAME__ NC_GPIOG_5
#define __GPIO_PG_PIN_5_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_5_STATE__ _FLOAT
#define __GPIO_PG_PIN_6_NAME__ NC_GPIOG_6
#define __GPIO_PG_PIN_6_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_6_STATE__ _FLOAT
#define __GPIO_PG_PIN_7_NAME__ NC_GPIOG_7
#define __GPIO_PG_PIN_7_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_7_STATE__ _FLOAT
#define __GPIO_PG_PIN_8_NAME__ NC_GPIOG_8
#define __GPIO_PG_PIN_8_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_8_STATE__ _FLOAT
#define __GPIO_PG_PIN_9_NAME__ NC_GPIOG_9
#define __GPIO_PG_PIN_9_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_9_STATE__ _FLOAT
#define __GPIO_PG_PIN_10_NAME__ NC_GPIOG_10
#define __GPIO_PG_PIN_10_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_10_STATE__ _FLOAT
#define __GPIO_PG_PIN_11_NAME__ NC_GPIOG_11
#define __GPIO_PG_PIN_11_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_11_STATE__ _FLOAT
#define __GPIO_PG_PIN_12_NAME__ NC_GPIOG_12
#define __GPIO_PG_PIN_12_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_12_STATE__ _FLOAT
#define __GPIO_PG_PIN_13_NAME__ NC_GPIOG_13
#define __GPIO_PG_PIN_13_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_13_STATE__ _FLOAT
#define __GPIO_PG_PIN_14_NAME__ NC_GPIOG_14
#define __GPIO_PG_PIN_14_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_14_STATE__ _FLOAT
#define __GPIO_PG_PIN_15_NAME__ NC_GPIOG_15
#define __GPIO_PG_PIN_15_MODE__ _GPIO_IN_FLOAT
#define __GPIO_PG_PIN_15_STATE__ _FLOAT

#endif /* _GPIO_FLAGS_H_ */
/*==============================================================================
  End of file
==============================================================================*/
