/*
 * Copyright 2014-2016,2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __LED_H_
#define __LED_H_

#include "led/led_dft.h"

/**
 * @defgroup MODS_NSS_LED led: LED module
 * @ingroup MODS_NSS
 * The LED module provides an API to control bi-state (on/off) LEDs on a board. Each LED has a "handle" (e.g.
 * @c LED_(2)), and @c LED_On(LED_(2)) switches the third LED 'on'. In other words, the LED module abstracts which
 * physical pin is used and the polarity of the control (active high or active low).
 *
 * It is expected that each board library includes this module, and configures the diversity settings of this module
 * (see led_dft.h) to match the LEDs on the board. The goal is that all applications can use LEDs using the same API
 * with the same behavior.
 *
 * @par How to use the module
 *  - The first step is to initialize the module (via #LED_Init). Note that both the IOCON block and the GPIO block must
 *      have been initialized before.
 *  - Next, the LEDs are controlled via the other provided functions. All arguments are a mask. The first is always
 *      @c leds that identifies which LEDs will be targeted; example values are #LED_(0), #LED_(1) @c | #LED_(3),
 *      #LED_ALL.
 *      The second argument - if present - is @c states that identifies the new states for the LEDs. A bit value of @c 1
 *      indicates an 'on' value, a value of 0 indicates an 'off' value.
 *
 * @par Diversity
 *  This module supports diversity: the number of LEDs it can control and how they are configured.
 *  Check @ref MODS_NSS_LED_DFT for all diversity parameters and their default values.
 *
 * @par Examples
 *  To shorten the examples any timing is left out. You may want to single step intersperse them with calls to
 *  #Chip_Clock_System_BusyWait_ms after each state change to see the full intended effect.
 *
 * @par Example 1 - Switching two LEDs in various ways
 *  @snippet led_mod_example_1.c led_mod_example_1
 *
 * @par Example 2 - Inspecting the state of LEDs
 *  @snippet led_mod_example_2.c led_mod_example_2
 *
 * @par Example 3 - Light bar
 *  @snippet led_mod_example_3.c led_mod_example_3
 *
 * @{
 */

#define LED_(n) (1 << (n)) /**< The mask for LED n. */
#define LED_ALL ((1 << (LED_COUNT)) - 1) /**< The mask for all available LEDs */

/**
 * Configures the pins for LED access, and switches the LEDs off.
 * @pre The IOCON block must have been initialized before. This is already taken care of by the board library
 *  initialization function #Board_Init.
 * @pre The GPIO block must have been initialized before. This is already taken care of by the board library
 *  initialization function #Board_Init.
 * @see Chip_IOCON_Init
 * @see Chip_GPIO_Init
 */
void LED_Init(void);

/**
 * Sets all the LEDs for the given bits to the corresponding given states.
 * @param leds : A mask identifying which LEDs to change state.
 * @note @c bits set outside #LED_ALL are ignored.
 * @param states : A mask identifying the new state for the LEDs.
 * @note @c bits set outside #LED_ALL are ignored.
 * @note This is a low-level function, there are also high-level functions: #LED_On, #LED_Off, and #LED_Toggle.
 */
void LED_SetState(int leds, int states);

/**
 * Gets the state of all the LEDs for the given bits.
 * @param leds : A mask identifying which LEDs to retrieve state.
 * @note @c bits set outside #LED_ALL are ignored.
 * @return The i-th bit in the returned value is the state of LED i.
 *  If the i-th bit in `leds` is 0, the state is not retrieved from the hardware, and the returned bit is 0.
 */
int LED_GetState(int leds);

/**
 * Turn on all LEDs for the given bits.
 * @param leds : A mask identifying which LEDs to turn on.
 * @note @c bits set outside #LED_ALL are ignored.
 * @note This function is a shorthand for @code LED_SetState(leds, leds) @endcode
 */
void LED_On(int leds);

/**
 * Turn off all LEDs for the given bits.
 * @param leds : A mask identifying which LEDs to turn off.
 * @note @c bits set outside #LED_ALL are ignored.
 * @note This function is a shorthand for @code LED_SetState(leds, 0) @endcode
 */
void LED_Off(int leds);

/**
 * Toggle all LEDs for the given bits.
 * @param leds : A mask identifying which LEDs to toggle state.
 * @note @c bits set outside #LED_ALL are ignored.
 * @note This function is a shorthand for @code LED_SetState(leds, ~LED_GetState(leds)) @endcode
 */
void LED_Toggle(int leds);

#endif /** @} */
