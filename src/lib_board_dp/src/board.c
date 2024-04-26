/*
 * Copyright 2014-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include <NXP/crp.h>
#include "board.h"
#include "led/led.h"
extern const unsigned int CRP_WORD;

void Board_Init(void)
{
    Chip_IOCON_Init(NSS_IOCON);
    Chip_GPIO_Init(NSS_GPIO);

    /* Reduce power consumption by adding pulls per your specific board. The default register values
     * after a reset do not have enabled these pulls. The functionality of the SWD pins is kept.
     */
    Chip_IOCON_SetPinConfig(NSS_IOCON, 0, IOCON_FUNC_0 | BOARD_PIO0_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 1, IOCON_FUNC_0 | BOARD_PIO1_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 2, IOCON_FUNC_0 | BOARD_PIO2_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 3, IOCON_FUNC_0 | BOARD_PIO3_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 4, IOCON_FUNC_0 | BOARD_PIO4_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 5, IOCON_FUNC_0 | BOARD_PIO5_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 6, IOCON_FUNC_0 | BOARD_PIO6_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 7, IOCON_FUNC_0 | BOARD_PIO7_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 8, IOCON_FUNC_0 | BOARD_PIO8_PULL);
    Chip_IOCON_SetPinConfig(NSS_IOCON, 9, IOCON_FUNC_0 | BOARD_PIO9_PULL);
    if ((CRP_WORD == CRP_CRP2) || (CRP_WORD == CRP_CRP3_CONSUME_PART)) {
        Chip_IOCON_SetPinConfig(NSS_IOCON, 10, IOCON_FUNC_0 | BOARD_PIO10_PULL);
        Chip_IOCON_SetPinConfig(NSS_IOCON, 11, IOCON_FUNC_0 | BOARD_PIO11_PULL);
    }
    else {
        Chip_IOCON_SetPinConfig(NSS_IOCON, 10, IOCON_FUNC_2 | BOARD_PIO10_PULL);
        Chip_IOCON_SetPinConfig(NSS_IOCON, 11, IOCON_FUNC_2 | BOARD_PIO11_PULL);
    }

    Chip_RTC_Init(NSS_RTC);
    Chip_EEPROM_Init(NSS_EEPROM);
    Chip_NFC_Init(NSS_NFC);
    LED_Init();
}
