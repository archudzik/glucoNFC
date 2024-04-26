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

/**
 * Internally, after the configurable clock divider, the WWDT has a fixed pre-scaler of 4. The WWDT counter will thus
 * be decremented once every 4 WWDT clock cycles.
 * The number of watchdog ticks per second is equal to SFRO freq / clock divider / 4.
 * The minimum frequency by which the counter is decremented is thus 7874 Hz.
 * @param clockDivider The same value as given to the call #Chip_Clock_Watchdog_SetClockDiv
 * @param timeout The maximum allowed time in number of seconds between two calls to #Chip_WWDT_Feed
 */
#define WATCHDOG_TICKS(clockDivider, timeout) (8*1000*1000 / (clockDivider) / 4 * (timeout))

void Chip_WWDT_Start(uint32_t timeout)
{
    /* Enable, configure and start the watchdog timer in normal (reset) mode.
     * There is no reason not to run the clock as slow as possible: thus using 254 in the SetClockDiv call below.
     */
    Chip_Clock_Watchdog_SetClockSource(CLOCK_WATCHDOGSOURCE_SFRO);
    Chip_Clock_Watchdog_SetClockDiv(254);
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_WATCHDOG);
    NSS_WWDT->TC = WATCHDOG_TICKS(254, timeout);
    NSS_WWDT->MOD |= WWDT_WDMOD_WDRESET | WWDT_WDMOD_WDEN;
    Chip_WWDT_Feed();
    /* Ways to stop the WWDT:
     * - Change the clock source to CLOCK_WATCHDOGSOURCE_VSS
     * - Set the clock divider to 0
     * - Disable the WWDT clock
     */
}

void Chip_WWDT_Feed(void)
{
    __disable_irq();
    NSS_WWDT->FEED = 0xAA;
    NSS_WWDT->FEED = 0x55;
    __enable_irq();
}
