/*
 * Copyright 2014-2016 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"

void Chip_Flash_SetHighPowerMode(bool highPower)
{
    /* If disabling high power mode, make sure that either system clock divisor is not 1 or
     * wait states are configured. Else core might end up in hard fault (flash access error).
     * See "SW Clock Restrictions" */
    ASSERT(highPower || (Chip_Clock_System_GetClockDiv() > 1) || (Chip_Flash_GetNumWaitStates() > 0));

    /* Control bit 18 (Low Power Mode) as well as bit 19 (Bandgap). Both bits need to be set or cleared together. */
    uint32_t mask = (1 << 18) | (1 << 19);

    if (highPower) {
        NSS_FLASH->FCTR &= ~mask;
    }
    else {
        NSS_FLASH->FCTR |= mask;
    }
}

bool Chip_Flash_GetHighPowerMode(void)
{
    /* Control bit 18 (Low Power Mode) and bit 19 (Bandgap) are set and cleared together. */
    uint32_t mask = (1 << 18) | (1 << 19);

    return (NSS_FLASH->FCTR & mask) != mask;
}

void Chip_Flash_SetNumWaitStates(int waitStates)
{
    ASSERT((waitStates > 0) || (Chip_Clock_System_GetClockDiv() > 1) || Chip_Flash_GetHighPowerMode());

    NSS_FLASH->FBWST = (NSS_FLASH->FBWST & ~0x00FFu) | (uint32_t)(waitStates & 0xFF);
}

int Chip_Flash_GetNumWaitStates(void)
{
    return NSS_FLASH->FBWST & 0xFF;
}
