/*
 * Copyright 2017-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/** @defgroup MODS_NSS_I2CBBM_DFT Diversity Settings
 *  @ingroup MODS_NSS_I2CBBM
 * These 'defines' capture the diversity settings of the module. The displayed values refer to the default settings.
 * To override the default settings, place the defines with their desired values in the application app_sel.h header
 * file: the compiler will pick up your defines before parsing this file.
 *
 * for the I2C bit-banging module, these flags can be overridden:
 * - #I2CBBM_MAX_CLK_STRETCH
 * - #I2CBBM_CLK_PIN
 * - #I2CBBM_DAT_PIN
 * - #I2CBBM_PULLUP_COUNT
 * - #I2CBBM_PULLUPS
 * - #I2CBBM_PULSE_WIDTH
 * - #I2CBBM_DEFAULT_I2C_ADDRESS
 * - #I2CBBM_SYSTEM_CLOCK_DIVIDER
 * @{
 */
#ifndef __I2CBBM_DFT_H_
#define __I2CBBM_DFT_H_

#include "chip.h"

/**
 * This defines how long slaves can stretch the clock before the master stops trying to transmit.
 * Depending on the system clock frequency, each unit adds a number of microseconds to the waiting time:
 * |@b SysClock|increased waiting time in us, per unit|
 * |--:        |:-                                    |
 * |8Mhz       |0.126 us                              |
 * |4Mhz       |0.275 us                              |
 * |2Mhz       |0.50 us                               |
 * |0.5Mhz     |2.03 us                               |
 */
#if (!defined(I2CBBM_MAX_CLK_STRETCH))
    #define I2CBBM_MAX_CLK_STRETCH 100
#endif

/**
 * Both @c I2CBBM_CLK_PIN and #I2CBBM_DAT_PIN must be defined. The defaults provide a valid pair.
 * @note @b The pin listed here is under full and exclusive control of this module.
 */
#if (!defined(I2CBBM_CLK_PIN))
    #define I2CBBM_CLK_PIN IOCON_PIO0_4
#endif

/**
 * Both #I2CBBM_CLK_PIN and @c I2CBBM_DAT_PIN must be defined. The defaults provide a valid pair.
 * @note @b The pin listed here is under full and exclusive control of this module.
 */
#if (!defined(I2CBBM_DAT_PIN))
    #define I2CBBM_DAT_PIN IOCON_PIO0_5
#endif

/**
 * By default no pulls are added by the NHS3100: The default configuration assumes an external pull-up is provided for I2C
 * communication. Here you can list PIOs that are placed under full control of this module, and which must be used to
 * provide the necessary pull-ups to the clock and data lines.
 * Check with your layout which pins are applicable.
 * @note keep in mind that pins 4 and 5 themselves can't be pulled up internally.
 * @note @b All pins listed here are under full and exclusive control of this module.
 * @warning Adding external pullups may prevent the IC to go to power-off mode, as - depending on the layout of your
 *  board - current can keep flowing through the bondpad ring via the pins assigned here - #I2CBBM_CLK_PIN,
 *  #I2CBBM_DAT_PIN and #I2CBBM_PULLUPS - still powering a small part of the VDD_ALON domain.
 */
#if !I2CBBM_PULLUP_COUNT && !defined(I2CBBM_PULLUPS)
    /* If these defines are not overridden, external pull-ups must be provided for the SCL and SDA lines. */
    /**
     * Defines how many GPIO's are used as pull-up lines for the Ucode SCL and SDA lines.
     */
    #define I2CBBM_PULLUP_COUNT 0
    /**
     * Refers to an array of type uint32_t with #I2CBBM_PULLUP_COUNT elements.
     * Defines the GPIO's used as pull-up lines.
     */
    #define I2CBBM_PULLUPS { -1 } /* {} seems more correct. This value avoids the warning 'ISO C forbids empty initializer braces [-Wpedantic]' */
#elif I2CBBM_PULLUP_COUNT && defined(I2CBBM_PULLUPS)
    /* OK */
#else
    #error Both I2CBBM_PULLUP_COUNT and I2CBBM_PULLUPS must be both defined or undefined.
    #error Define I2CBBM_PULLUPS as an array of type IOCON_PIN_T and size I2CBBM_PULLUP_COUNT,
    #error For example: #define I2CBBM_PULLUP_COUNT 3 and #define I2CBBM_PULLUPS = {IOCON_PIO0_x, IOCON_PIO0_y, IOCON_PIO0_z}
#endif

/**
 * I2CBBM_PULSE_WIDTH increases the pulse width of each clock pulse.
 * Depending on the system clock frequency, each unit adds a number of microseconds to the pulse width:
 * |@b SysClock|increased waiting time in us, per unit|
 * |--:        |:-                                    |
 * |8Mhz       |0.126 us                              |
 * |4Mhz       |0.275 us                              |
 * |2Mhz       |0.50 us                               |
 * |0.5Mhz     |2.03 us                               |
 */
#if (!defined(I2CBBM_PULSE_WIDTH))
    #define I2CBBM_PULSE_WIDTH 1
#endif

/**
 * The address of the I2C slave to communicate with can be set with #I2cbbm_SetAddress. After #I2cbbm_Init, a default
 * I2C slave address is used. This default value can be overridden here. The lowest 7 bits are being used, the MSBit is
 * disregarded.
 */
#if !defined(I2CBBM_DEFAULT_I2C_ADDRESS)
    #define I2CBBM_DEFAULT_I2C_ADDRESS 0x7F
#endif

/**
 * The different timings of the CLK and DAT lines are system clock dependent. Provide here the value of
 * #Chip_Clock_System_GetClockDiv() at the time this module is used.
 * If these differ, the timings may be too short - when running at higher clock speeds than anticipated - or
 * communication may be much slower.
 * The default value - @c 4 - corresponds to a system clock frequency of 2 MHz.
 */
#if !defined(I2CBBM_SYSTEM_CLOCK_DIVIDER)
    #define I2CBBM_SYSTEM_CLOCK_DIVIDER 4
#endif

#endif /** @} */
