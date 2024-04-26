/*
 * Copyright 2014-2016,2018-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __STARTUP_H_
#define __STARTUP_H_

#include "startup/startup_dft.h"

/**
 * @defgroup MODS_NSS_STARTUP startup: Startup and exception handlers
 * @ingroup MODS_NSS
 * This module implements the vector table for all exceptions.
 * It also implements default handlers for all exceptions.
 *
 * There are two flavors of exceptions: exceptions internal to the CPU core (the ARM), and exceptions external to the
 * CPU core. The former are dictated by ARM and occupy the first 16 slots. The latter are dictated by the (peripherals
 * of the ARM) and occupy slot 16 and onwards. The external exceptions are also referred to as (peripheral) interrupts,
 * and have their own numbering.
 *
 * An example of a peripheral interrupt is a timer overflow. An example o an ARM internal exception is the hard fault
 * exception. A special case is the first ARM internal exception: reset.
 *
 * The startup module has a common implementation for every handler. The implementation consists of an ASSERT(false)
 * followed by an infinite loop. Only the reset handler has a real implementation, see below.
 *
 * All handlers have a so-called "weak" implementation. This means that the application can override them by declaring a
 * handler with exactly the same name/signature.
 *
 * @par Details on reset
 *  This module implements the reset handler. The implementation should be sufficient for most applications. However,
 *  the ResetISR is a weak implementation, so an application has the option to override it.
 *  The implementation of the reset handler consists of 3 steps:
 *      -# Initialization (cq zeroing) of all variables
 *      -# Initialization of the runtime lib (C++ only)
 *      -# Calling main
 *  There is a fourth step: when main returns (should never happen), there is an assert and hang.
 *
 * @par Example 1 - Implement timer interrupt handler
 *  When using a timer interrupt, the default handler for the timer needs to be overridden. Declare a function with the
 *  exact name '#CT16B0_IRQHandler':
 *  @snippet startup_mod_example_1.c startup_mod_example_1_ct16b0
 *
 *  The following code configures and starts the timer:
 *  @snippet startup_mod_example_1.c startup_mod_example_1_main
 *
 * @par Example 2 - Firing a hard fault to own handler
 *  Declare a function with the exact name '#HardFault_Handler' to override the default hard fault handler:
 *  @snippet startup_mod_example_2.c startup_mod_example_2_hardfault
 *
 *  The following code causes a hard fault by calling an illegal address:
 *  @snippet startup_mod_example_2.c startup_mod_example_2_main
 *
 * @par Example 3 - Overriding the reset vector
 *  Include the function with the exact name '#ResetISR' to override the default reset handler. The most basic
 *  implementation is shown. It initializes (c.q. zeros) all variables, and calls main():
 *  @snippet startup_mod_example_3.c startup_mod_example_3_resetisr
 * @{
 */

/**
 * Macro to annotate a function as "weak".
 * @note A weak function is linked-in, unless a function with the exact same name and signature exists (at application level).
 *  The latter function will be linked-in instead.
 *  This is primarily useful in defining library functions that can be overridden in user code.
 * @see https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 */
#define WEAK __attribute__ ((weak))

/**
 * Handler for (ARM) Reset Interrupt.
 * This handler takes care of the target's initialization before running application code.
 * This handler has a default (#WEAK) implementation which performs flowing steps:
 *  -# Call #Startup_VarInit (Variable initialization etc.)
 *  -# Initialization of the runtime libraries (if c++)
 *  -# Call @c main to start the actual application
 *  @note If user overrides its #WEAK implementation, he should not forget to call #Startup_VarInit and @c main.
 */
WEAK void ResetISR(void);

/**
 * Handler for (ARM) None Maskable Interrupt.
 * This handler has a default (#WEAK) implementation
 *  @see #WEAK for documentation on overriding this handler's functionality.
 */
WEAK void NMI_Handler(void);

/**
 * Handler for (ARM) HardFault Interrupt.
 * This handler has a default (#WEAK) implementation
 *  @see #WEAK for documentation on overriding this handler's functionality.
 */
WEAK void HardFault_Handler(void);

/**
 * Handler for (ARM) SuperVisor Call Interrupt (Mostly used by an OS).
 * This handler has a default (#WEAK) implementation
 *  @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void SVC_Handler(void);

/**
 * Handler for (ARM) Pending system-level service Interrupt (Mostly used by an OS).
 * This handler has a default (#WEAK) implementation
 *  @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void PendSV_Handler(void);

/**
 * Handler for (ARM) System Tick Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 * @note To enable SysTick please refer to @ref CMSIS_SysTick
 */
WEAK void SysTick_Handler(void);

/**
 * Handler for (Peripheral) the respective PIO Start Logic Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 * @see @ref startLogic_anchor "'System wake-up start logic'" for more info on the Start Logic.
 * @{
 */
WEAK void PIO0_0_IRQHandler(void);
WEAK void PIO0_1_IRQHandler(void);
WEAK void PIO0_2_IRQHandler(void);
WEAK void PIO0_3_IRQHandler(void);
WEAK void PIO0_4_IRQHandler(void);
WEAK void PIO0_5_IRQHandler(void);
WEAK void PIO0_6_IRQHandler(void);
WEAK void PIO0_7_IRQHandler(void);
WEAK void PIO0_8_IRQHandler(void);
WEAK void PIO0_9_IRQHandler(void);
WEAK void PIO0_10_IRQHandler(void);
/**@}*/

/**
 * Handler for (Peripheral) NFC Access Start Logic Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 * @see @ref startLogic_anchor "'System wake-up start logic'" for more info on the Start Logic.
 */
WEAK void RFFIELD_IRQHandler(void);

/**
 * Handler for (Peripheral) RTC Wake Up Request Start Logic Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 * @see @ref startLogic_anchor "'System wake-up start logic'" for more info on the Start Logic.
 */
WEAK void RTCPWREQ_IRQHandler(void);

/**
 * Handler for (Peripheral) NFC Read/Write Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void NFC_IRQHandler(void);

/**
 * Handler for (Peripheral) RTC Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 * @warning A possible spurious interrupt can occur. Be sure to guard the code in the interrupt handler by checking the
 *  interrupt status using #Chip_RTC_Int_GetRawStatus. See
 */
WEAK void RTC_IRQHandler(void);

/**
 * Handler for (Peripheral) I2C0 Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void I2C0_IRQHandler(void);

/**
 * Handler for (Peripheral) Respective Timer Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 * @{
 */
WEAK void CT16B0_IRQHandler(void);
WEAK void CT32B0_IRQHandler(void);
/**@}*/

/**
 * Handler for (Peripheral) PMU RF Power Detection Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void PMUFLD_IRQHandler(void);

/**
 * Handler for (Peripheral) PMU Brown Out Detection Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void PMUBOD_IRQHandler(void);

/**
 * Handler for (Peripheral) SSP Read/Write Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void SSP0_IRQHandler(void);

/**
 * Handler for (Peripheral) Temperature Sensor Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void TSEN_IRQHandler(void);

/**
 * Handler for (Peripheral) Capacitance-to-Digital converter Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void C2D_IRQHandler(void);

/**
 * Handler for (Peripheral) Current-to-Digital converter Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void I2D_IRQHandler(void);

/**
 * Handler for (Peripheral) Analog-to-Digital/Digital-to-Analog converter Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void ADC_IRQHandler(void);

/**
 * Handler for (Peripheral) Watchdog Timer Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void WDT_IRQHandler(void);

/**
 * Handler for (Peripheral) FLASH memory Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void FLASH_IRQHandler(void);

/**
 * Handler for (Peripheral) EEPROM memory Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void EEPROM_IRQHandler(void);

/**
 * Handler for (Peripheral) GPIO Port 0 Interrupt.
 * This handler has a default (#WEAK) implementation
 * @see #WEAK for documentation on overriding this handler's functionality
 */
WEAK void PIO0_IRQHandler(void);

/**
 * Initializes all static variables with an initializer and zeros all other variables.
 * @note This function should only be called when implementing an application specific ResetISR(). The default
 *   ResetISR() already calls this function before calling main().
 */
void Startup_VarInit(void);

#endif /** @} */
