/*
 * Copyright 2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @defgroup MODS_NSS_BATIMP_DFT Diversity Settings
 *  @ingroup MODS_NSS_BATIMP
 *
 * The application can adapt the battery impedance check module to fit its use case through the use of
 * diversity flags in the form of defines below. Safe defaults are chosen; to override the default settings, place
 * the defines with their desired values in the application app_sel.h header file: the compiler will pick up your
 * defines before parsing this file.
 *
 * These flags may be overridden:
 * - #BATIMP_WAIT_TIME
 * - #BATIMP_USE_PIO3
 * - #BATIMP_USE_PIO7
 * - #BATIMP_USE_PIO10
 * - #BATIMP_USE_PIO11
 *
 * @{
 */
#ifndef __BATIMP_DFT_H_
#define __BATIMP_DFT_H_

#include "chip.h"

/* ------------------------------------------------------------------------- */

#ifndef BATIMP_WAIT_TIME
    /**
     * Time in milliseconds to wait each time after changing the current DAC quiescent, before checking the BOD.
     * Determines the maximum time it takes for #BatImp_Check to complete.
     * - 1 pin enabled: 4 * @c BATIMP_WAIT_TIME
     * - 2 pins enabled: 10 * @c BATIMP_WAIT_TIME
     * - 3 pins enabled: 20 * @c BATIMP_WAIT_TIME
     * - 4 pins enabled: 35 * @c BATIMP_WAIT_TIME
     * @note The choice of pins that can be used in this module depends on your layout.
     */
    #define BATIMP_WAIT_TIME 10
#endif
#if BATIMP_WAIT_TIME <= 0
    #error Define BATIMP_WAIT_TIME to a strict positive number
#endif

#ifndef BATIMP_USE_PIO3
    /**
     * Enabling more pins for use in the battery impedance check module allows for a more complete check.
     * @note The choice of pins that can be used in this module depends on your layout.
     */
    #define BATIMP_USE_PIO3 0
#endif

#ifndef BATIMP_USE_PIO7
    /**
     * Enabling more pins for use in the battery impedance check module allows for a more complete check.
     * @note The choice of pins that can be used in this module depends on your layout.
     */
    #define BATIMP_USE_PIO7 0
#endif

#ifndef BATIMP_USE_PIO10
    /**
     * Enabling more pins for use in the battery impedance check module allows for a more complete check.
     * @note The choice of pins that can be used in this module depends on your layout.
     * @note Take care when enabling this, as a possible ongoing debugging session will loose its connection when used.
     */
    #define BATIMP_USE_PIO10 1
#endif

#ifndef BATIMP_USE_PIO11
    /**
     * Enabling more pins for use in the battery impedance check module allows for a more complete check.
     * @note The choice of pins that can be used in this module depends on your layout.
     * @note Take care when enabling this, as a possible ongoing debugging session will loose its connection when used.
     */
    #define BATIMP_USE_PIO11 1
#endif

#endif /** @} */
