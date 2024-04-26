/*
 * Copyright 2015-2018 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/** @defgroup MODS_NSS_TMEAS_DFT Diversity Settings
 *  @ingroup MODS_NSS_TMEAS
 * These 'defines' capture the diversity settings of the module. The displayed values refer to the default settings.
 * To override the default settings, place the defines with their desired values in the application app_sel.h header
 * file: the compiler will pick up your defines before parsing this file.
 * @{
 */
#ifndef __TMEAS_DFT_H_
#define __TMEAS_DFT_H_

/**
 * Set this define to 1 to enable the format @ref TMEAS_FORMAT_KELVIN
 */
#if (!defined(TMEAS_KELVIN))
    #define TMEAS_KELVIN 0
#endif

/**
 * Set this define to 0 to disable the format @ref TMEAS_FORMAT_CELSIUS
 * @note This format is enabled by default.
 */
#if (!defined(TMEAS_CELSIUS))
    #define TMEAS_CELSIUS 1
#endif

/**
 * Set this define to 1 to enable the format @ref TMEAS_FORMAT_FAHRENHEIT
 */
#if (!defined(TMEAS_FAHRENHEIT))
    #define TMEAS_FAHRENHEIT 0
#endif

/**
 * Set this define to 0 to disable the temperature sensor correction (enabled by default).
 * If, for your IC revision, the correction is applied straight in the TSEN calibration parameters, it must be set to 0.
 * Otherwise, it must be left at default value (1).
 */
#if (!defined(TMEAS_SENSOR_CORRECTION))
    #define TMEAS_SENSOR_CORRECTION 1
#endif

/* Diversity flags below are undefined by default. They are wrapped in a DOXYGEN precompilation flag to enable
 * documenting them properly. To define them and use the corresponding functionality of the module, make the correct
 * defines in app_sel.h or board_sel.h.
 */
#ifdef __DOXYGEN__
#error This block of code may not be parsed using gcc.

/**
 * By default, only synchronous measurements are enabled.
 * To enable asynchronous measurements, where the main thread execution continues and the measurement will be reported
 * later under interrupt by calling a callback function, define that callback function here.
 * Set this define to the function to be called.
 * @note The value set @b must have the same signature as @ref pTMeas_Cb_t
 * @note This must be set to the name of a function, not a pointer to a function: no dereference will be made!
 */
#define TMEAS_CB application function of type pTMeas_Cb_t
#endif

#endif /** @} */
