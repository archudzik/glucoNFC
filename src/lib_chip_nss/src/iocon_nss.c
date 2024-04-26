/*
 * Copyright 2014-2017 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"

void Chip_IOCON_Init(NSS_IOCON_T *pIOCON)
{
    (void)pIOCON; /* suppress [-Wunused-parameter]: argument is only present for consistency. */
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_IOCON);
}

void Chip_IOCON_DeInit(NSS_IOCON_T *pIOCON)
{
    (void)pIOCON; /* suppress [-Wunused-parameter]: argument is only present for consistency. */
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_IOCON);
}

void Chip_IOCON_SetPinConfig(NSS_IOCON_T *pIOCON, IOCON_PIN_T pin, int config)
{
    if (pin >= IOCON_ANA0_0) /* Analog Pins */{
        if (config == IOCON_FUNC_1) /* Indicates connection to analog bus */{
            Chip_IOCON_UngroundAnabus(pIOCON, (1 << (pin - IOCON_ANA0_0)));
        }
    }
    pIOCON->REG[pin] = config & 0xFFFFFF;
}

int Chip_IOCON_GetPinConfig(NSS_IOCON_T *pIOCON, IOCON_PIN_T pin)
{
    return pIOCON->REG[pin] & 0xFFFFFF;
}

void Chip_IOCON_SetAnabusGrounded(NSS_IOCON_T *pIOCON, IOCON_ANABUS_T bitvector)
{
    pIOCON->ANABUSGROUND = bitvector & 0x0FFFFFFF;
}

IOCON_ANABUS_T Chip_IOCON_GetAnabusGrounded(NSS_IOCON_T *pIOCON)
{
    return (IOCON_ANABUS_T)(pIOCON->ANABUSGROUND & 0x0FFFFFFF);
}

void Chip_IOCON_GroundAnabus(NSS_IOCON_T *pIOCON, IOCON_ANABUS_T bitvector)
{
    pIOCON->ANABUSGROUND |= bitvector & 0x0FFFFFFF;
}

void Chip_IOCON_UngroundAnabus(NSS_IOCON_T *pIOCON, IOCON_ANABUS_T bitvector)
{
    pIOCON->ANABUSGROUND &= ~(bitvector & 0x0FFFFFFF);
}
