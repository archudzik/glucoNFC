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

/**
 * @defgroup MODS_NSS_LED_DFT Diversity settings
 * @ingroup MODS_NSS_LED
 * These 'defines' capture the diversity settings of the module. The displayed values refer to the default settings.
 * To override the default settings, place the defines with their desired values in the application app_sel.h header
 * file: the compiler will pick up your defines before parsing this file.
 * @{
 */
#ifndef __LED_DFT_H_
#define __LED_DFT_H_

/**
 * Defines all properties of a LED this module requires to know to drive them in an uniform way.
 * Used by diversity setting #LED_PROPERTIES
 */
typedef struct LED_PROPERTIES_S {
    uint8_t port; /*!< The port number via which the GPIO can access the LED. */
    uint8_t pin; /*!< The pin number via which the GPIO can access the LED. */
    bool polarity; /*!< The polarity of the LED: @c true if the LED is 'on' when a 1 is written - active high,
        @c false otherwise - active low. */
    IOCON_PIN_T pio; /*!< The I/O Pins Definition that corresponds to the values above. */
} LED_PROPERTIES_T;

#if !LED_COUNT && !defined(LED_PROPERTIES)
    #warning LED_COUNT is not defined or is 0. No LEDs can be driven now.
    /**
     * Defines how many LEDs are present and can be controlled by the LED module.
     */
    #define LED_COUNT 0
    /**
     * Refers to an array of type #LED_PROPERTIES_T with size #LED_COUNT.
     * Defines the properties of a LED.
     */
    #define LED_PROPERTIES NULL
#elif LED_COUNT && defined(LED_PROPERTIES)
    /* OK */
#else
    #error Both LED_COUNT and LED_PROPERTIES must be both defined or undefined. Define LED_PROPERTIES as an array of
           type LED_PROPERTIES_T and size LED_COUNT.
#endif

#endif /** @} */
