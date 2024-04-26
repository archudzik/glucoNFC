/*
 * Copyright 2014-2018 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @defgroup DP_BOARD_NSS_SEL Board diversity overrides
 * @ingroup DP_BOARD_NSS
 * @{
 */

#ifndef __BOARD_SEL_H_
#define __BOARD_SEL_H_

/**
 * The number of LEDs supported by the Demo PCB.
 * Matches the length of #LED_PROPERTIES.
 */
#define LED_COUNT 1

/**
 * The LED properties for the supported LEDs of the Demo PCB.
 * @see LED_PROPERTIES_T
 */
#define LED_PROPERTIES {{0, 7, true, IOCON_PIO0_7}} /* LED_RED */

/** Easier to remember macro name for the LED */
#define LED_RED LED_(0)

#endif /** @} */
