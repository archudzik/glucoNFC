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

#ifndef __SYSCON_NSS_H_
#define __SYSCON_NSS_H_

/** @defgroup SYSCON_NSS syscon: System Configuration driver
 * @ingroup DRV_NSS
 * The System Configuration driver (SYSCON) provides the API to control most of the functionalities handled by the
 * SYSCON HW block. There is an exception for the handling of clocks, which is covered in a separate Clock driver.@n
 * The SYSCON driver allows:
 *  -# Remapping the Interrupt Vector Table location
 *  -# Controlling the reset lines and power for some peripherals
 *  -# Accessing the System Reset status
 *  -# Controlling the system wake-up start logic and state
 *  -# Reading the device ID
 *
 * @par Interrupt Vector Table Remap
 *  By default, after booting, the ARM core Interrupt Vector Table is located in Flash at address 0x0. However, using
 *  the #Chip_SysCon_IVT_SetAddress function it is possible to remap it to any SRAM or Flash location. When remapping
 *  the Interrupt Vector Table, ensure that the configured address places the vector table aligned and in the existing
 *  memories. Note that the two highest sectors of the Flash memory (last 2kB) are reserved.
 *
 * @par Peripheral reset and power control
 *  Using the #Chip_SysCon_Peripheral_AssertReset/#Chip_SysCon_Peripheral_DeassertReset functions is possible to
 *  assert/de-assert the reset line of the peripherals defined in #SYSCON_PERIPHERAL_RESET_T. All the peripherals have
 *  their reset lines asserted by default, except the EEPROM controller.
 *  Using the #Chip_SysCon_Peripheral_EnablePower/#Chip_SysCon_Peripheral_DisablePower functions it is possible to
 *  enable/disable the power of the peripherals included in #SYSCON_PERIPHERAL_POWER_T. All the peripherals are powered
 *  down by default, except the Flash memory.
 *  Note that when the Flash memory is powered down, it must be ensured that both the Interrupt Vector Table and the
 *  program code is executed from SRAM.
 *
 * @par System Reset status
 *  The source of the last reset may be known by using the #Chip_SysCon_Reset_GetSource function. Note that the reset
 *  sources are retained after an ARM core reset and they are only cleared on a full system reset or by calling
 *  #Chip_SysCon_Reset_ClearSource.
 *  In order to always get only the last reset source, #Chip_SysCon_Reset_ClearSource shall be always called after the
 *  reset source is retrieved. All reset sources cause a full system reset except the #SYSCON_RESETSOURCE_SOFTWARE,
 *  which only causes an ARM core reset.
 *
 * @anchor startLogic_anchor
 * @par System wake-up start logic
 *  The Start Logic mechanism consists of a group of internal logical lines that, once enabled and asserted, issue a
 *  start signal that triggers the ARM core to wake up from Deep-Sleep mode (see PMU documentation for more details on
 *  Power Modes).
 *  Each startup logic line is also connected to a separate wake-up interrupt handler in the NVIC (in the same order,
 *  from #PIO0_0_IRQn to #RTCPWREQ_IRQn). The interrupt(s) corresponding to each start logic source must be enabled in
 *  the NVIC so that they can interrupt the ARM core and actually trigger the wake-up (these interrupt lines are
 *  prepared to work without a system clock). Although the start logic is designed to be functional during Deep-Sleep,
 *  it can also be used during Active or Sleep modes to generate normal interrupts on rising or falling edges of the
 *  GPIO pins.
 *  The #SYSCON_STARTSOURCE_T defines all the available sources that are connected to an internal start logic line and
 *  thus can trigger an ARM core wake up from Deep-Sleep.
 *  As in normal interrupt logic, each line can be individually:
 *      - enabled using the #Chip_SysCon_StartLogic_SetEnabledMask function;
 *      - checked for status using the #Chip_SysCon_StartLogic_GetStatus function; and
 *      - cleared using the #Chip_SysCon_StartLogic_ClearStatus function
 *  Additionally, for each Digital IO pin source, using the #Chip_SysCon_StartLogic_SetPIORisingEdge function it can be
 *  configured which transition edge of the pin causes the respective start signal to be asserted.
 *  The state of the Start Logic lines indicates if the wake-up signal to the ARM core is triggered. Thus, before
 *  entering the Deep-Sleep mode, it must be ensured that all the Start Logic lines are cleared.
 *  All start logic lines are disabled by default.
 *
 * @par Device ID
 *  The Device ID of the chip can be retrieved using the #Chip_SysCon_GetDeviceID.
 *
 * @par Example 1 - Initialize/De-initialize peripherals
 *  Peripherals:
 *      - I2C0 (Digital)
 *      - I2D (Analog)
 *  @snippet syscon_nss_example_1.c syscon_nss_example_1
 *
 * @par Example 2 - Remap Interrupt Vector Table to SRAM:
 *  Notes:
 *      - Interrupt Vector Table copied to SRAM location (pointed by "uint32_t *ivt" variable)
 *      - The ISR's in use are already in SRAM and the Interrupt Vector Table pointing to the respective ISR locations
 *      - But, current Interrupt Vector Table is still mapped at default location (Flash at address 0x0)
 *  @snippet syscon_nss_example_2.c syscon_nss_example_2
 *
 * @par Example 3 - Configure chip start logic for wake-up from Deep-Sleep:
 *  Wake up on:
 *      - PIO0_0 rising edge
 *      - PIO0_5 falling edge
 *      - NFC core activation
 *  @snippet syscon_nss_example_3.c syscon_nss_example_3
 *
 * @{
 */

/**
 * System Configuration register block structure
 */
typedef struct NSS_SYSCON_S {
    __IO uint32_t SYSMEMREMAP;    /*!< System memory remap */
    __IO uint32_t PRESETCTRL;     /*!< Peripheral reset control */
    __I  uint32_t RESERVED1[6];   /*   next field at offset 0x020 */
    __IO uint32_t SYSCLKCTRL;     /*!< System clock control register - Used by Clock driver */
    __IO uint32_t SYSCLKUEN;      /*!< System clock update enable - Used by Clock driver */
    __I  uint32_t RESERVED2[2];   /*   next field at offset 0x030 */
    __IO uint32_t SYSRSTSTAT;     /*!< System reset status register */
    __I  uint32_t RESERVED3[19];  /*   next field at offset 0x080 */
    __IO uint32_t SYSAHBCLKCTRL;  /*!< AHB clock control - Used by Clock driver */
    __I  uint32_t RESERVED4[4];   /*   next field at offset 0x094 */
    __IO uint32_t SSP0CLKDIV;     /*!< SSP0 clock divider - Used by Clock driver */
    __I  uint32_t RESERVED5[14];  /*   next field at offset 0x0D0 */
    __IO uint32_t WDTCLKSEL;      /*!< Watchdog timer clock selector - Used by Clock driver */
    __IO uint32_t WDTCLKUEN;      /*!< Watchdog timer clock update enable - Used by Clock driver */
    __IO uint32_t WDTCLKDIV;      /*!< Watchdog timer clock divider - Used by Clock driver */
    __I  uint32_t RESERVED6[3];   /*   next field at offset 0x0E8 */
    __IO uint32_t CLKOUTEN;       /*!< CLKOUT enable - Used by Clock driver */
    __I  uint32_t RESERVED7[26];  /*   next field at offset 0x154 */
    __IO uint32_t SYSTCKCAL;      /*!< System tick counter calibration */
    __I  uint32_t RESERVED8[42];  /*   next field at offset 0x200 */
    __IO uint32_t STARTAPRP0;     /*!< Start logic edge control register 0 */
    __IO uint32_t STARTERP0;      /*!< Start logic signal enable register 0 */
    __IO uint32_t STARTRSRP0CLR;  /*!< Start logic reset register 0 */
    __IO uint32_t STARTSRP0;      /*!< Start logic status register 0 */
    __I  uint32_t RESERVED9[10];  /*   next field at offset 0x238 */
    __IO uint32_t PDRUNCFG;       /*!< Power-down configuration register */
    __I  uint32_t RESERVED10[110];/*   next field at offset 0x3F4 */
    __I  uint32_t DEVICEID;       /*!< Device ID register */
} NSS_SYSCON_T;

/** Peripherals that can be reset */
typedef enum SYSCON_PERIPHERAL_RESET {
    SYSCON_PERIPHERAL_RESET_SPI0   = (1 << 0), /*!< Represents the SPI0/SSP0 block reset */
    SYSCON_PERIPHERAL_RESET_I2C0   = (1 << 1), /*!< Represents the I2C0 block reset */
    SYSCON_PERIPHERAL_RESET_EEPROM = (1 << 2), /*!< Represents the EEPROM controller block reset */
    SYSCON_PERIPHERAL_RESET_NFC    = (1 << 3)  /*!< Represents the NFC shared memory interface (APB side), not the NFC block */
} SYSCON_PERIPHERAL_RESET_T;

/** Peripherals that whose power state can be controlled */
typedef enum SYSCON_PERIPHERAL_POWER {
    SYSCON_PERIPHERAL_POWER_FLASH  = (1 << 0), /*!< Represents the Flash memory power switch */
    SYSCON_PERIPHERAL_POWER_TSEN   = (1 << 1), /*!< Represents the Temperature Sensor power switch */
    SYSCON_PERIPHERAL_POWER_C2D    = (1 << 2), /*!< Represents the Capacitance to Digital converter power switch */
    SYSCON_PERIPHERAL_POWER_EEPROM = (1 << 3), /*!< Represents the EEPROM memory power switch */
    SYSCON_PERIPHERAL_POWER_I2D    = (1 << 4), /*!< Represents the Current to Digital converter power switch */
    SYSCON_PERIPHERAL_POWER_ADCDAC = (1 << 5)  /*!< Represents the Analog-to-Digital/Digital-to-Analog converter power switch */
} SYSCON_PERIPHERAL_POWER_T;

/** Possible chip reset sources */
typedef enum SYSCON_RESETSOURCE {
    SYSCON_RESETSOURCE_POR      = (1 << 0), /*!< Indicates a reset due to a Power-On-Reset */
    SYSCON_RESETSOURCE_RESETPIN = (1 << 1), /*!< Indicates a reset due to the assertion of the external reset pin */
    SYSCON_RESETSOURCE_WATCHDOG = (1 << 2), /*!< Indicates a reset caused by the watchdog */
    SYSCON_RESETSOURCE_SOFTWARE = (1 << 3), /*!< Indicates a reset caused by the SW (Software System Reset) */
    SYSCON_RESETSOURCE_NONE     = 0         /*!< No reset cause */
} SYSCON_RESETSOURCE_T;

/** Possible system wake-up start logic sources */
typedef enum SYSCON_STARTSOURCE {
    SYSCON_STARTSOURCE_PIO0_0  = (1 << 0), /*!< System wake-up start source - pin PIO0_0 */
    SYSCON_STARTSOURCE_PIO0_1  = (1 << 1), /*!< System wake-up start source - pin PIO0_1 */
    SYSCON_STARTSOURCE_PIO0_2  = (1 << 2), /*!< System wake-up start source - pin PIO0_2 */
    SYSCON_STARTSOURCE_PIO0_3  = (1 << 3), /*!< System wake-up start source - pin PIO0_3 */
    SYSCON_STARTSOURCE_PIO0_4  = (1 << 4), /*!< System wake-up start source - pin PIO0_4 */
    SYSCON_STARTSOURCE_PIO0_5  = (1 << 5), /*!< System wake-up start source - pin PIO0_5 */
    SYSCON_STARTSOURCE_PIO0_6  = (1 << 6), /*!< System wake-up start source - pin PIO0_6 */
    SYSCON_STARTSOURCE_PIO0_7  = (1 << 7), /*!< System wake-up start source - pin PIO0_7 */
    SYSCON_STARTSOURCE_PIO0_8  = (1 << 8), /*!< System wake-up start source - pin PIO0_8 */
    SYSCON_STARTSOURCE_PIO0_9  = (1 << 9), /*!< System wake-up start source - pin PIO0_9 */
    SYSCON_STARTSOURCE_PIO0_10 = (1 << 10), /*!< System wake-up start source - pin PIO0_10 */
    SYSCON_STARTSOURCE_NFC     = (1 << 11), /*!< System wake-up start source - NFC block
                                                (This source is triggered when the NFC core is activated) */
    SYSCON_STARTSOURCE_RTC     = (1 << 12), /*!< System wake-up start source - RTC
                                                (This source is triggered when the RTC wake-up downcounter expires)*/
    SYSCON_STARTSOURCE_NONE    = 0,         /*!< This is used to select none of the sources (or deselect all sources)*/
    SYSCON_STARTSOURCE_ALL     = 0x1FFF     /*!< This is used to select all sources */
} SYSCON_STARTSOURCE_T;

/**
 * Maps the Interrupt Vector Table (IVT) to any SRAM or Flash location.
 * Valid addresses are on 1024 byte boundaries in Flash or SRAM.
 * @param address : The absolute memory address (SRAM or Flash) to where to map the IVT
 * @note The function asserts on addresses that are outside of the SRAM or Flash boundaries or that are not on 1024 byte
 *  boundaries.
 */
void Chip_SysCon_IVT_SetAddress(uint32_t address);

/**
 * Gets the absolute address (SRAM or Flash) on where the Interrupt Vector Table (IVT) is mapped.
 * @return The absolute memory address (SRAM or Flash) on where the IVT is mapped
 * @note After booting, the IVT is mapped at address 0x0 (Flash memory) by default
 */
uint32_t Chip_SysCon_IVT_GetAddress(void);

/**
 * Asserts the reset of the required peripheral(s)
 * @param bitvector : Bitvector of the peripheral(s) whose reset to assert
 * @note When enabled, the reset line for the peripheral remains asserted until #Chip_SysCon_Peripheral_DeassertReset
 *  is called for the same peripheral. Thus, to toggle the reset line, a "...EnableReset" followed by a "...DisableReset"
 *  must be executed. Refer to user manual for minimum reset pulse time.
 *  Only the reset(s) of the peripheral(s) set in bitvector are asserted. All the others are left untouched.
 * @note Do not deassert #SYSCON_PERIPHERAL_RESET_EEPROM reset when #SYSCON_PERIPHERAL_POWER_EEPROM power is disabled
 *  as this causes unexpected further behavior of the EEPROM
 */
void Chip_SysCon_Peripheral_AssertReset(SYSCON_PERIPHERAL_RESET_T bitvector);

/**
 * De-asserts the reset of the required peripheral(s)
 * @param bitvector : Bitvector of the peripheral(s) whose reset to de-assert
 * @note All the peripherals have their reset lines asserted by default, except the EEPROM controller.
 *  Only the reset(s) of the peripheral(s) set in bitvector are deasserted. All the others are left untouched.
 * @note Do not deassert #SYSCON_PERIPHERAL_RESET_EEPROM reset when #SYSCON_PERIPHERAL_POWER_EEPROM power is disabled
 *  as this causes unexpected further behavior of the EEPROM
 */
void Chip_SysCon_Peripheral_DeassertReset(SYSCON_PERIPHERAL_RESET_T bitvector);

/**
 * Enables the power of the required peripheral(s)
 * @param bitvector : Bitvector of the peripheral(s) whose power to enable
 * @note All the peripherals are powered down by default, except the Flash memory
 *  Only the power state(s) of the peripheral(s) set in bitvector are changed. All the others are left untouched.
 */
void Chip_SysCon_Peripheral_EnablePower(SYSCON_PERIPHERAL_POWER_T bitvector);

/**
 * Disables the power of the required peripheral(s)
 * @param bitvector : Bitvector of the peripheral(s) whose power to enable
 *  Only the power state(s) of the peripheral(s) set in bitvector are changed. All the others are left untouched.
 */
void Chip_SysCon_Peripheral_DisablePower(SYSCON_PERIPHERAL_POWER_T bitvector);

/**
 * Enables/Disables the power state of the peripheral(s) described in #SYSCON_PERIPHERAL_POWER_T
 * @param bitvector : Bitvector of the peripheral(s) whose power to enable/disable
 * @note This setting overwrites the existing setting for all the peripherals described in #SYSCON_PERIPHERAL_POWER_T.
 *  If the respective bit is set, the power of the respective peripheral will be disabled, otherwise it will be
 *  enabled. This functionality is overlapped with the #Chip_SysCon_Peripheral_EnablePower
 *  and #Chip_SysCon_Peripheral_DisablePower functions
 */
void Chip_SysCon_Peripheral_SetPowerDisabled(SYSCON_PERIPHERAL_POWER_T bitvector);

/**
 * Retrieves a bitvector stating the enabled/disabled peripheral(s) described in #SYSCON_PERIPHERAL_POWER_T
 * @return Bitvector stating the enabled/disabled peripheral(s)
 * @note If the respective bit is set, the power of the respective peripheral is disabled, otherwise it is enabled.
 *  All the peripherals are powered down by default, except the Flash memory
 */
SYSCON_PERIPHERAL_POWER_T Chip_SysCon_Peripheral_GetPowerDisabled(void);

/**
 * Gets the source of the last reset event(s)
 * @return The source of the last reset event(s)
 * @note When the reset source is #SYSCON_RESETSOURCE_SOFTWARE, only the ARM core is reset. Thus, the previous reset
 *  source information is also retained and it might be possible, in this case, to get more than one reset source.
 *  In order to avoid ambiguity, it is recommended to always clear the reset source information after get, using the
 *  #Chip_SysCon_Reset_ClearSource function.
 */
SYSCON_RESETSOURCE_T Chip_SysCon_Reset_GetSource(void);

/**
 * Clears the reset source information
 */
void Chip_SysCon_Reset_ClearSource(void);

/**
 * Enables/Disables the system wake-up start logic interrupts
 * @param mask : Interrupt enabled mask to set
 * @note This setting overwrites the existing setting for all the sources described in #SYSCON_STARTSOURCE_T.
 *  If a bit is set, the respective wake-up start source will be enabled, otherwise it will be disabled.
 */
void Chip_SysCon_StartLogic_SetEnabledMask(SYSCON_STARTSOURCE_T mask);

/**
 * Retrieves the system wake-up start logic source interrupt enabled mask
 * @return System wake-up start logic interrupt enabled mask
 * @note If a bit is set, the respective wake-up start source is enabled, otherwise it is disabled.
 *  By default, all the wake-up start sources are disabled
 */
SYSCON_STARTSOURCE_T Chip_SysCon_StartLogic_GetEnabledMask(void);

/**
 * Retrieves a bitVector with the system wake-up start logic interrupt flags
 * @return BitVector with the system wake-up start logic interrupt flags
 * @note A bit set to 1 means that the correspondent system wake-up start logic interrupt flag is set.
 * @note This function only reports the correct StartLogic status if the corresponding interrupt is enabled.
 *  If interrupts are not required while using this function, disable the respective ISR in the NVIC, but set the
 *  interrupt enable bit with #Chip_SysCon_StartLogic_SetEnabledMask.
 */
SYSCON_STARTSOURCE_T Chip_SysCon_StartLogic_GetStatus(void);

/**
 * Clears the required system wake-up start logic interrupt flags
 * @param flags : Bitvector indicating which system wake-up start logic interrupt flags to clear
 */
void Chip_SysCon_StartLogic_ClearStatus(SYSCON_STARTSOURCE_T flags);

/**
 * Selects a falling or rising edge as the trigger of the corresponding system wake-up PIO start source
 * @param bitvector : Bitvector of the system wake-up PIO start sources to be triggered on a rising
 *  edge (the ones that are not set are triggered on a falling-edge)
 * @note This setting overwrites the existing setting for all the PIO sources described in #SYSCON_STARTSOURCE_T.
 *  If a bit is set, the PIO source will be triggered on a rising edge, otherwise it will be triggered on a falling
 *  edge (provided the required source(s) is(are) enabled with #Chip_SysCon_StartLogic_SetEnabledMask).
 *  This edge selection is only valid for PIO sources, thus the use of #SYSCON_STARTSOURCE_NFC and #SYSCON_STARTSOURCE_RTC
 *  does not result in any configuration change.
 */
void Chip_SysCon_StartLogic_SetPIORisingEdge(SYSCON_STARTSOURCE_T bitvector);

/**
 * Gets the edge (falling or rising) that is selected to be the trigger for the corresponding system wake-up PIO start source
 * @return Bitvector of the system wake-up PIO start sources that are triggered on a rising edge (the ones that are not
 *  set are triggered on a falling-edge)
 * @note If a bit is set, the PIO source is triggered on a rising edge, otherwise it is triggered on a falling
 *  edge (provided the required source(s) is(are) enabled with #Chip_SysCon_StartLogic_SetEnabledMask).
 *  By default, the falling edge is selected as trigger for all system wake-up PIO start sources
 */
SYSCON_STARTSOURCE_T Chip_SysCon_StartLogic_GetPIORisingEdge(void);

/**
 * Gets the Device ID of the chip
 * @return The chip device ID
 * @note For more information about the meaning of the Device ID value returned, please refer to the respective
 *  chip User Manual.
 * @note This call will return the same value as the IAP call #Chip_IAP_ReadPartID.
 * @note This call will return:
 *  - @c 0x4E310020 for NHS3100 devices
 *  - @c 0x4E315220 for NHS3152 devices
 *  - @c 0x4E315320 for NHS3153 devices
 */
uint32_t Chip_SysCon_GetDeviceID(void);

#endif /** @} */
