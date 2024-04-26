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

#ifndef __CMSIS_H_
#define __CMSIS_H_

/** @defgroup CHIP_CORE_NSS cmsis: Cortex-M0+ CMSIS support
 * @ingroup DRV_NSS
 * Provides an access layer to the functionality of the Cortex-M0+ core.@n
 * Please refer to @ref CMSIS_Core_FunctionInterface for all the available functions and to
 * @ref CMSIS_Core_InstructionInterface for all the available instructions.
 * @{
 */

// The following definitions configure ARM's cmsis.
// We need to define them, before including it (core_cm0plus.h)
#if defined(__ARMCC_VERSION)
    // Kill warning "#pragma push with no matching #pragma pop"
    #pragma diag_suppress 2525
    #pragma push
    #pragma anon_unions
#elif defined(__CWCC__)
    #pragma push
    #pragma cpp_extensions on
#elif defined(__GNUC__)
    // anonymous unions are enabled by default
#elif defined(__IAR_SYSTEMS_ICC__)
    // #pragma push // FIXME not usable for IAR
    #pragma language=extended
#else
    #error Not supported compiler type
#endif

#if !defined(CORE_M0PLUS)
    #error Please #define CORE_M0PLUS
#endif

/* Configuration of the Cortex-M0+ Processor and Core Peripherals */
/** Cortex-M0 Core Revision */
#define __CM0_REV                 0x0000

/** MPU present or not */
#define __MPU_PRESENT             0

/** Number of Bits used for Priority Levels */
#define __NVIC_PRIO_BITS          2

/** Set to 1 if different SysTick Config is used */
#define __Vendor_SysTickConfig    0

/** Defines the supported NVIC Peripheral interrupts */
typedef enum {

    /*  Cortex-M0 Processor Exceptions Numbers */
    Reset_IRQn          = -15, /*!< 1 Reset Vector, invoked on Power up and warm reset */
    NonMaskableInt_IRQn = -14, /*!< 2 Non Maskable Interrupt */
    HardFault_IRQn      = -13, /*!< 3 Cortex-M0+ Hard Fault Interrupt */
    SVCall_IRQn         = -5, /*!< 11 Cortex-M0+ SV Call Interrupt */
    PendSV_IRQn         = -2, /*!< 14 Cortex-M0+ Pend SV Interrupt */
    SysTick_IRQn        = -1, /*!< 15 Cortex-M0+ System Tick Interrupt */

    /*  Interrupt Numbers */
    PIO0_0_IRQn         = 0, /*!< PIO0_0 Start Logic Interrupt */
    PIO0_1_IRQn         = 1, /*!< PIO0_1 Start Logic Interrupt */
    PIO0_2_IRQn         = 2, /*!< PIO0_2 Start Logic Interrupt */
    PIO0_3_IRQn         = 3, /*!< PIO0_3 Start Logic Interrupt */
    PIO0_4_IRQn         = 4, /*!< PIO0_4 Start Logic Interrupt */
    PIO0_5_IRQn         = 5, /*!< PIO0_5 Start Logic Interrupt */
    PIO0_6_IRQn         = 6, /*!< PIO0_6 Start Logic Interrupt */
    PIO0_7_IRQn         = 7, /*!< PIO0_7 Start Logic Interrupt */
    PIO0_8_IRQn         = 8, /*!< PIO0_8 Start Logic Interrupt */
    PIO0_9_IRQn         = 9, /*!< PIO0_9 Start Logic Interrupt */
    PIO0_10_IRQn        = 10, /*!< PIO0_10 Start Logic Interrupt */
    RFFIELD_IRQn        = 11, /*!< NFC Access Start Logic Interrupt */
    RTCPWREQ_IRQn       = 12, /*!< RTC Wakeup Request Start Logic Interrupt */
    NFC_IRQn            = 13, /*!< NFC Read/Write Interrupt */
    RTC_IRQn            = 14, /*!< RTC Wakeup Interrupt */
    I2C0_IRQn           = 15, /*!< I2C0 Interrupt */
    CT16B0_IRQn         = 16, /*!< 16-bit Timer 0 Interrupt */
    PMUFLD_IRQn         = 17, /*!< RF Power Detection Interrupt */
    CT32B0_IRQn         = 18, /*!< 32-bit Timer 0 Interrupt */
    PMUBOD_IRQn         = 19, /*!< Brown Out Detection Interrupt */
    SSP0_IRQn           = 20, /*!< SSP0 Interrupt */
    TSEN_IRQn           = 21, /*!< Temperature Sensor Interrupt */
    C2D_IRQn            = 22, /*!< Capacitance-to-Digital converter Interrupt */
    Reserved1_IRQn      = 23, /*!< reserved */
    I2D_IRQn            = 24, /*!< Current-to-Digital converter Interrupt */
    ADCDAC_IRQn         = 25, /*!< Analog-to-Digital/Digital-to-Analog converter Interrupt */
    WDT_IRQn            = 26, /*!< Watchdog Timer Interrupt */
    FLASH_IRQn          = 27, /*!< FLASH memory Interrupt */
    EEPROM_IRQn         = 28, /*!< EEPROM memory Interrupt */
    Reserved2_IRQn      = 29, /*!< reserved */
    Reserved3_IRQn      = 30, /*!< reserved */
    PIO0_IRQn           = 31, /*!< GPIO Port 0 Interrupt */

} IRQn_Type;

/* Ignoring -Wsign-conversion in CMSIS (ARM) files */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "core_cm0plus.h"
#pragma GCC diagnostic pop

/**
 * @}
 */

#endif /* __CMSIS_H_ */
