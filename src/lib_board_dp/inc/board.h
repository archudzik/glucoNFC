/*
 * Copyright 2015-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __BOARD_H_
#define __BOARD_H_

#include "chip.h"
#include "led/led.h"

/** @defgroup DP_BOARD_NSS dp: board support for the Demo PCB
 * @ingroup BOARDS_NSS
 * The Demo PCB is the promoted HW board for the evaluation of NHS31xx family ICs along with
 * the respective SDK.
 *
 * The Demo PCB provides a couple of features, which are exposed at SW level:
 *  - one controllable LED: #LED_RED
 *  - one button readable via @ref GPIO_NSS "GPIO"
 *
 * The initialization function performs initialization of the LED and adds the correct pulls to all the PIOs.
 *
 * @par Board support concept:
 *  Board support comes in the form of a statically-linked library which includes the bare minimum for an application
 *  to start the HW properly (IO pin configuration and/or clock configuration if required)
 *  as well as SW support for the features provided by the board.
 *  There is a single function that is part of the board support API: the board initialization function (#Board_Init).
 *  The board initialization has two distinct purposes. In one hand, to ensure that IO pin configuration and state are
 *  set to a harmless state for the respective board. On the other hand that the features provided by the board
 *  (LEDs, external memories, etc) are properly initialized in SW and ready to be used directly by the application.
 *  Whenever board support is needed for a different board (different pin layout, reduced/extended feature set, etc), a
 *  separate board library must be created, matching the requirements of the new board. As long as the initialization
 *  function prototype (#Board_Init) and the design is kept consistent between board APIs, applications may switch
 *  boards more easily during development or even be run on different boards without changing any code.
 *
 *  @par Diversity
 *  This board makes use of some higher level modules which support diversity: where possible, the default values have
 *  been kept. Necessary overrides have been made in @ref DP_BOARD_NSS_SEL.
 *
 * @{
 */

/**
 * A unique define for the Development PCB.
 * Applications or modules can use this define to enable/disable code at compile time based on the board being used.
 */
#define BOARD_DP

/**
 * Depending on the peripherals on the board, power consumption is minimized by adding pull-ups or -downs on the digital
 * pins. These are added at startup in #Board_Init and should have these pulls when the pins are not in use.
 * @note possible values are #IOCON_RMODE_PULLDOWN, #IOCON_RMODE_PULLUP or #IOCON_RMODE_INACT (no pull).
 * @{
 */
#define BOARD_PIO0_PULL IOCON_RMODE_PULLUP
#define BOARD_PIO1_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO2_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO3_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO4_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO5_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO6_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO7_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO8_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO9_PULL IOCON_RMODE_PULLUP /* When using pin9 for UARTTX, a pull-up prevents undesired 'break' conditions. */
#define BOARD_PIO10_PULL IOCON_RMODE_PULLDOWN
#define BOARD_PIO11_PULL IOCON_RMODE_PULLDOWN
/** @} */

/** If defined, a push button or similar is attached to PIO0 and can wake up the IC when in Deep Power Down. */
#define BOARD_ENABLE_WAKEUP
#if defined(BOARD_ENABLE_WAKEUP) && (BOARD_PIO0_PULL != IOCON_RMODE_PULLUP)
    #error Wake up functionality cannot work without a pull-up on PIO0.
#endif

/**
 * Sets up and initializes all required blocks and functions related to the board hardware.
 * @note Initialization will add pulls according to #BOARD_PIO0_PULL, #BOARD_PIO1_PULL and so on, to the default IO pin
 *  configuration.
 * @note Initialization does not change the system clock configuration.
 * @pre This function may be only called once, may be only called after a hard reset, and must be called as soon as
 *  possible.
 * @post @ref MODS_NSS_LED "LED" module is initialized.
 * @post These chip level drivers are initialized and ready for use:
 *  - @ref IOCON_NSS "IOCON"
 *  - @ref GPIO_NSS "GPIO"
 *  - @ref RTC_NSS "RTC"
 *  - @ref EEPROM_NSS "EEPROM"
 *  - @ref NFC_NSS "NFC"
 * @note This function may be called @b before Startup_VarInit has been called.
 */
void Board_Init(void);

#endif /** @} */
