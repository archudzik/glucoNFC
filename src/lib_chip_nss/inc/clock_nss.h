/*
 * Copyright 2014-2017,2019-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __CLOCK_NSS_H_
#define __CLOCK_NSS_H_

/**
 * @defgroup CLOCK_NSS clock: Clock driver
 * @ingroup DRV_NSS
 * The Clock driver provides the API to control functionalities related with the handling of clocks.
 * This functionality is part of the SYSCON HW block, however, for clarity it is exposed in an API separate from the
 * SYSCON driver.
 * @note All the registers that belong to the SYSCON HW block (including the ones related with handling of clocks) are
 *  defined in the SYSCON driver.
 *
 * The chip provides two internal clock sources:
 *  - System Oscillator (SFRO) which runs at 8MHz and is the clock source for the System Clock (AHB Clock), Peripheral
 *      Clocks and Watchdog Clock domains.
 *  - Timer Oscillator (TFRO) which runs at 32.768 kHz and is the clock source for the PMU and RTC.
 * The Clock driver allows configuring the separate clocks that drive all the mentioned domains, namely regarding
 * division, gating and selection of the clock sources.
 *
 * @par Clock Division:
 *  The chip provides three separate clock dividers, all acting on the SFRO clock, that output three separate clocks:
 *      -# System Clock divider: Divides the SFRO into the clock that drives the System Clock (AHB Clock) and the
 *          Peripheral Clocks. This divider is configured to 16 by default (500 kHz) and can be configured using the
 *          #Chip_Clock_System_SetClockFreq function.
 *      -# SPI Clock divider: Divides the SFRO into the clock that drives the SPI HW block. This divider is configured
 *          to 0 by default (disabled) and can be configured using the #Chip_Clock_SPI0_SetClockFreq function.
 *      -# Watchdog Clock divider: Divides the selected Watchdog clock source (SFRO by default) into the clock that
 *          drives the Watchdog timer. This divider is configured to 0 by default (disabled) and can be configured using
 *          the #Chip_Clock_Watchdog_SetClockFreq function.
 *
 * @par Clock Gating:
 *  The clocks to peripheral HW blocks can be enabled and disabled using the #Chip_Clock_Peripheral_EnableClock
 *  and #Chip_Clock_Peripheral_DisableClock functions. The HW blocks whose clock can be disabled/enabled are defined in
 *  #CLOCK_PERIPHERAL_T enum. All peripheral clocks are disabled by default except for the RAM memory, Flash controller,
 *  Flash memory, IOCON block, EEPROM controller and EEPROM memory.
 *
 * @par Clock Selection:
 *  The chip allows selecting the clock source for two separate clocks:
 *      - Watchdog Timer: The clock source for the Watchdog timer is configured to the SFRO (#CLOCK_WATCHDOGSOURCE_SFRO)
 *          by default and can be configured using the #Chip_Clock_Watchdog_SetClockSource function.
 *      - Clock Output pin: The clock source for CLKOUT pin is disabled by default (#CLOCK_CLKOUTSOURCE_DISABLED),
 *          however it can be configured using the #Chip_Clock_Clkout_SetClockSource function. Note that the pin must be
 *          properly configured so that it can output the required clock. Please refer to @ref IOCON_NSS "IOCON driver"
 *          for pin configuration.
 *  Note that the TFRO is part of the Power Management Unit (PMU) HW block, which is an always-on power domain. The
 *  handling of this clock source is, thus, covered in the @ref PMU_NSS "PMU driver".
 * @{
 */

/** Possible Peripheral clocks that can be enabled/disabled */
typedef enum CLOCK_PERIPHERAL {
    CLOCK_PERIPHERAL_RAM           = (1 << 2),  /*!< Represents the RAM memory clock */
    CLOCK_PERIPHERAL_FLASH         = (3 << 3),  /*!< Represents the Flash memory clock*/
    CLOCK_PERIPHERAL_I2C0          = (1 << 5),  /*!< Represents the I2C0 clock */
    CLOCK_PERIPHERAL_GPIO          = (1 << 6),  /*!< Represents the GPIO clock */
    CLOCK_PERIPHERAL_SPI0          = (1 << 7),  /*!< Represents the SPI0 register access clock (The SPI0 block itself
                                                     is clocked by a separate clock) */
    CLOCK_PERIPHERAL_16TIMER0      = (1 << 8),  /*!< Represents the 16 bit timer 0 clock */
    CLOCK_PERIPHERAL_32TIMER0      = (1 << 9),  /*!< Represents the 32 bit timer 0 clock */
    CLOCK_PERIPHERAL_RTC           = (1 << 10), /*!< Represents the RTC clock register access clock only (The RTC block
                                                     itself is clocked by a separate clock source) */
    CLOCK_PERIPHERAL_WATCHDOG      = (1 << 11), /*!< Represents the Watchdog register access clock only (The Watchdog
                                                     block itself is clocked by a separate clock) */
    CLOCK_PERIPHERAL_TSEN          = (1 << 12), /*!< Represents the Temperature Sensor clock */
    CLOCK_PERIPHERAL_C2D           = (1 << 13), /*!< Represents the Capacitance to Digital converter clock */
    CLOCK_PERIPHERAL_I2D           = (1 << 15), /*!< Represents the Current to Digital converter clock */
    CLOCK_PERIPHERAL_ADCDAC        = (1 << 16), /*!< Represents the Analog-to-Digital/Digital-to-Analog converter clock */
    CLOCK_PERIPHERAL_IOCON         = (1 << 18), /*!< Represents the I/O configuration block clock */
    CLOCK_PERIPHERAL_EEPROM        = (3 << 19), /*!< Represents the EEPROM memory clock */
} CLOCK_PERIPHERAL_T;

/** Possible clock sources for the Watchdog */
typedef enum CLOCK_WATCHDOGSOURCE {
    CLOCK_WATCHDOGSOURCE_SFRO   = 0, /*!< SFRO (8MHz) as the Watchdog clock source */
    CLOCK_WATCHDOGSOURCE_VSS    = 2 /*!< VSS as the Watchdog clock source (no Clock) */
} CLOCK_WATCHDOGSOURCE_T;

/** Possible clock sources for the CLKOUT pin */
typedef enum CLOCK_CLKOUTSOURCE {
    CLOCK_CLKOUTSOURCE_DISABLED = 0, /*!< Disables the CLKOUT pin output */
    CLOCK_CLKOUTSOURCE_SFRO     = 1, /*!< SFRO (8MHz) as the CLKOUT pin clock source */
    CLOCK_CLKOUTSOURCE_SYSTEM   = 3, /*!< System Clock (62.5 kHz - 8MHz) as the CLKOUT pin clock source */
    CLOCK_CLKOUTSOURCE_TFRO     = 5, /*!< TFRO (32.768 kHz) as the CLKOUT pin clock source */
    CLOCK_CLKOUTSOURCE_NFC      = 7  /*!< NFC clock (1.695 MHz) as the CLKOUT pin clock source */
} CLOCK_CLKOUTSOURCE_T;

/**
 * Sets the division factor that divides the SFRO for the clock that drives the System Clock and the Peripheral Clocks.
 * The maximum division factor is 128 and only powers of 2 are valid (e.g. 1,2,4,8,..,128). 0 is NOT allowed.
 * @param divisor : The division factor to set
 * @pre The precondition listed in #Chip_Clock_System_SetClockFreq applies here as well.
 * @note This setting affects the core execution speed. This is a low level function, use
 *  #Chip_Clock_System_SetClockFreq instead.
 * @note If not valid, the 'divisor' will be clipped to the largest power of 2 lower than or equal to it, however
 *  up-clipped to 128. Use the #Chip_Clock_System_GetClockDiv to retrieve the actual divisor that was set.
 */
void Chip_Clock_System_SetClockDiv(int divisor);

/**
 * Gets the division factor that divides the SFRO for the clock that drives the System Clock and the Peripheral Clocks.
 * @return The division factor. The maximum division factor is 128 and only powers of 2 are valid (e.g. 1,2,4,8,..,128)
 * @note The default value for the division factor is 16 (500kHz).
 */
int Chip_Clock_System_GetClockDiv(void);

/**
 * Sets the System Clock frequency in Hz.
 * @param frequency : The System Clock frequency in Hz to set
 * @note This setting affects the core execution speed.
 * @note Only a set of frequencies is supported. If not valid, the 'frequency' will be clipped to the closest supported
 *  value higher than or equal to it. The System Clock frequency range is (62.5 kHz - 8MHz). Frequencies of 0 and higher
 *  than 8MHz are NOT allowed. Use the #Chip_Clock_System_GetClockFreq to retrieve the actual frequency that was set.
 * @pre If @c frequency equals @c 4000001 or higher, system clock divisor will become @c 1. The caller must ensure that
 *  either
 *  - high power mode is configured - #Chip_Flash_SetHighPowerMode with argument @c true, or
 *  - the number of wait states for flash operations is not 0 - #Chip_Flash_SetNumWaitStates with argument 1 or higher.
 *  Failure to do so will result in a hard fault (flash access error).
 *  See also @ref NSS_CLOCK_RESTRICTIONS
 */
void Chip_Clock_System_SetClockFreq(int frequency);

/**
 * Gets the System Clock frequency in Hz
 * @return The System Clock frequency in Hz
 * @note The default value for the System Clock frequency is 500 kHz
 */
int Chip_Clock_System_GetClockFreq(void);

/**
 * Waits the specified amount of time (using instruction counting)
 * @param us : number of microseconds to wait
 * @note This function does not wait if @c us <= 0.
 * @note The wait time shall not exceed 4 000 000 us.
 * @note The wait time is guaranteed to be at least @c us, but it will be somewhat longer.
 * @note Due to the function overhead, for the error to be under 10\% @c us should be at least the equivalent of 512
 *  system ticks. i.e.: @c us > 512 / system clock frequency (in MHz)
 */
void Chip_Clock_System_BusyWait_us(uint32_t us);

/**
 * Waits the specified amount of time (using instruction counting)
 * @param ms : number of milliseconds to wait
 * @note This function does not wait if @c ms <= 0.
 * @note The wait time is guaranteed to be at least @c ms, but it will be somewhat longer.
 * @note This function is just a wrapper for #Chip_Clock_System_BusyWait_us.
 */
void Chip_Clock_System_BusyWait_ms(int ms);

/**
 * Sets the division factor that divides the SFRO into the clock that drives the SPI0 HW block.
 * The supported division factors are 1 and 2-254 (even numbers) and 0 can be used to disable the clock.
 * @param divisor : The division factor to set
 * @note This is a low level function, use #Chip_Clock_SPI0_SetClockFreq instead.
 * @note If not valid, the divisor will be clipped to 254. Odd divisors are floored to nearest even number, except when
 *  divisor is 1.
 */
void Chip_Clock_SPI0_SetClockDiv(int divisor);

/**
 * Gets the division factor that divides the SFRO into the clock that drives the SPI0 HW block.
 * @return The division factor. The supported division factors are 1 and 2-254 (even numbers) and 0 means that the clock
 *  is disabled.
 * @note The default value for the division factor is 0 (disabled).
 */
int Chip_Clock_SPI0_GetClockDiv(void);

/**
 * Sets the frequency in Hz of the clock that drives the SPI0 HW block.
 * @param frequency : The clock frequency in Hz to set
 * @note Only a set of frequencies is supported. If not valid, the 'frequency' will be clipped to the closest supported
 *  value higher than or equal to it. The SPI0 clock frequency range is (~31.373 kHz - 8MHz). Use the
 *  #Chip_Clock_SPI0_GetClockFreq to retrieve the actual frequency that was set. Setting a frequency value of 0 Hz or
 *  higher than 8MHz disables the clock.
 */
void Chip_Clock_SPI0_SetClockFreq(int frequency);

/**
 * Gets the frequency in Hz of the clock that drives the SPI0 HW block.
 * @return The clock frequency in Hz
 * @note If the clock is disabled, 0 is returned. The clock is disabled by default.
 * @note The returned frequency value is rounded to the closest integer.
 */
int Chip_Clock_SPI0_GetClockFreq(void);

/**
 * Sets the division factor that divides the SFRO into the clock that drives the Watchdog.
 * The supported division factors are 1 and 2-254 (even numbers) and 0 can be used to disable the clock.
 * @param divisor : The division factor to set
 * @note If not valid, the divisor will be clipped to 254. Odd divisors are floored to nearest even number, except when
 *  divisor is 1.
 */
void Chip_Clock_Watchdog_SetClockDiv(int divisor);

/**
 * Gets the division factor that divides the SFRO into the clock that drives the Watchdog.
 * @return The division factor. The supported division factors are 1 and 2-254 (even numbers) and 0 means that the clock
 *  is disabled.
 * @note The default value for the division factor is 0 (disabled).
 */
int Chip_Clock_Watchdog_GetClockDiv(void);

/**
 * Sets the frequency in Hz of the clock that drives the Watchdog.
 * @param frequency : The clock frequency in Hz to set
 * @note Only a set of frequencies is supported. If not valid, the 'frequency' will be clipped to the closest supported
 *  value higher than or equal to it. The Watchdog clock frequency range is (~31.373 kHz - 8MHz). Use the
 *  #Chip_Clock_Watchdog_GetClockFreq to retrieve the actual frequency that was set. Setting a frequency value of 0 Hz
 *  disables the clock.
 */
void Chip_Clock_Watchdog_SetClockFreq(int frequency);

/**
 * Gets the clock frequency in Hz of the clock that drives the Watchdog.
 * @return The clock frequency in Hz
 * @note If the clock is disabled, 0 is returned. The clock is disabled by default.
 * @note The returned frequency value is rounded to the closest integer.
 */
int Chip_Clock_Watchdog_GetClockFreq(void);

/**
 * Enables peripheral clock(s).
 * @param bitvector : Bitvector of the peripheral(s) whose clock to enable
 * @note All peripheral clocks are disabled by default except #CLOCK_PERIPHERAL_RAM, #CLOCK_PERIPHERAL_FLASH,
 *  #CLOCK_PERIPHERAL_IOCON, #CLOCK_PERIPHERAL_EEPROM.
 * @note Only the peripheral clock(s) set in @c bitvector are changed. All the others are left untouched.
 * @note This functionality is overlapped with the #Chip_Clock_Peripheral_SetClockEnabled function
 * @warning Directly accessing Flash or EEPROM controller addresses (#NSS_FLASH_T, #NSS_EEPROM_T) when the respective
 *  clock is disabled, blocks the APB bus indefinitely.
 */
void Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_T bitvector);

/**
 * Disables peripheral clock(s).
 * @param bitvector : Bitvector of the peripheral(s) whose clock to disable
 * @note All peripheral clocks are disabled by default except #CLOCK_PERIPHERAL_RAM, #CLOCK_PERIPHERAL_FLASH,
 *  #CLOCK_PERIPHERAL_IOCON, #CLOCK_PERIPHERAL_EEPROM.
 * @note Only the peripheral clock(s) set in @c bitvector are changed. All the others are left untouched.
 * @note This functionality is overlapped with the #Chip_Clock_Peripheral_SetClockEnabled function
 * @warning Directly accessing Flash or EEPROM controller addresses (#NSS_FLASH_T, #NSS_EEPROM_T) when the respective
 *  clock is disabled, blocks the APB bus indefinitely.
 */
void Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_T bitvector);

/**
 * Enables/Disables the clock of the peripheral(s) described in #CLOCK_PERIPHERAL_T
 * @param bitvector : Bitvector of the peripheral(s) whose clock to enable/disable
 * @note This setting overwrites the existing setting for all the peripherals described in #CLOCK_PERIPHERAL_T.
 * @note If the respective bit is set, the clock of the respective peripheral will be enabled, otherwise it will be
 *  disabled.
 * @note This functionality is overlapped with the #Chip_Clock_Peripheral_EnableClock and #Chip_Clock_Peripheral_DisableClock
 *  functions
 * @warning Directly accessing Flash or EEPROM controller addresses (#NSS_FLASH_T, #NSS_EEPROM_T) when the respective
 *  clock is disabled, blocks the APB bus indefinitely.
 */
void Chip_Clock_Peripheral_SetClockEnabled(CLOCK_PERIPHERAL_T bitvector);

/**
 * Retrieves a bitvector stating the enabled/disabled peripheral clock(s) described in #CLOCK_PERIPHERAL_T
 * @return Bitvector stating the enabled/disabled peripheral clock(s)
 * @note If the respective bit is set, the clock of the respective peripheral is enabled, otherwise it is disabled.
 * @note All peripheral clocks are disabled by default except #CLOCK_PERIPHERAL_RAM, #CLOCK_PERIPHERAL_FLASH,
 *  #CLOCK_PERIPHERAL_IOCON, #CLOCK_PERIPHERAL_EEPROM.
 */
CLOCK_PERIPHERAL_T Chip_Clock_Peripheral_GetClockEnabled(void);

/**
 * Sets the Watchdog clock source.
 * @param source : The Watchdog clock source to set
 */
void Chip_Clock_Watchdog_SetClockSource(CLOCK_WATCHDOGSOURCE_T source);

/**
 * Gets the Watchdog clock source.
 * @return The Watchdog clock source
 * @note The default Watchdog clock source is #CLOCK_WATCHDOGSOURCE_SFRO.
 */
CLOCK_WATCHDOGSOURCE_T Chip_Clock_Watchdog_GetClockSource(void);

/**
 * Sets the CLKOUT pin clock source.
 * @param source : The CLKOUT pin clock source to set
 */
void Chip_Clock_Clkout_SetClockSource(CLOCK_CLKOUTSOURCE_T source);

/**
 * Gets the CLKOUT pin clock source.
 * @return The CLKOUT pin clock source
 * @note The default CLKOUT pin clock source is #CLOCK_CLKOUTSOURCE_DISABLED.
 */
CLOCK_CLKOUTSOURCE_T Chip_Clock_Clkout_GetClockSource(void);

#endif /** @} */
