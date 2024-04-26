/*
 * Copyright 2016,2018 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __SENSE_GPIO_DFT_H_
#define __SENSE_GPIO_DFT_H_

#include "sense_gpio_sel.h"
/**
 * @defgroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC_GPIO_DFT Diversity settings for the GPIO sensing principle
 * @ingroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC
 * These 'defines' capture the diversity settings of the compound.
 * It keeps the settings about 'sensing groups', it actually holds the configurations of the demo board/smart blister.
 *
 * @par Principle
 *  This principle is based on GPIO functionality, each pill has a unique combination of Drive/Sense pins.
 *  By putting a Pull-up resistor on the sense line (input) (SW configurable), and Pulling the Drive line LOW (output),
 *  the logic level on the sense line indicates the absence of a pill.
 *
 *@par Limitations
 *  There is one limitation for this principle, all SENSE lines need to have pull-up resistors.
 *  Most pins have the possibility to configure them in SW (in chip pull-up), however,
 *  PIO0_4 and PIO0_5 (I2C) do not have this feature, hence they can only be used as drive line OR in combination
 *  with an external pull-up.
 *
 *@par Setup
 *  There are 2 possible setups, which both use the same sensing principle:
 *      -# No extra components; there are 12 GPIO pins available, at least 1 shall be used as drive line. @n
 *          Setup:
 *          @code
 *              GPIO DRIVE o----------------------     ...
 *                            |     |     |     |
 *                            p1    p2    p3    p4      ...
 *                            |     |     |     |
 *              GPIO SENSE1 o--     |     |     |
 *              GPIO SENSE2 o--------     |     |
 *              GPIO SENSE3 o--------------     |
 *              GPIO SENSE4 o--------------------
 *                  ...
 *          @endcode
 *          This setup results in a maximum of 11 manageable pills.
 *      -# diode matrix; by adding 1 diode per pill, a matrix can be formed. @n
 *          Setup:
 *          @code
 *              GPIO DRIVE2 o-------------------------------- ...
 *              GPIO DRIVE1 o-------- ...             |     |
 *                 ...        |     |                 |     |
 *                            _     _                 _     _
 *                            ^     ^                 ^     ^
 *                            |     |                 |     |
 *                           p1.1  p1.2 ...          p2.1  p2.2 ...
 *                            |     |                 |     |
 *                            |     |                 |     |
 *                            |     --o GPIO SENSE1 o--     |
 *                            --------o GPIO SENSE2 o--------
 *                                          ...
 *          @endcode
 *          This setup results in a maximum of 36 manageable pills, a matrix of 6 drive and 6 sense lines.
 * @{
 */

/**
 * Defines all properties of a GROUP this module requires to know to sense the pills in an uniform way.
 */
typedef struct GROUP_PROPERTIES_S {
    IOCON_PIN_T IO_drivePin; /*!< The drive pin used in the group. */
    IOCON_PIN_T IO_sensePin[GROUP_MAX_PILLS]; /*!< A sense pill for each pill in this group. */
    uint8_t pills; /*!< Amount of pills initially present in the group. */
} GROUP_PROPERTIES_T;

#if !GROUP_COUNT || !defined(GROUP_PROPERTIES)
    #error GROUP_COUNT is not defined or is 0. No pills can be sensed.
#endif

/**
 * @}
 */

#endif
