/*
 * Copyright 2015-2016 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __TMEAS_H_
#define __TMEAS_H_

/** @defgroup MODS_NSS_TMEAS tmeas: Temperature measurement module
 * @ingroup MODS_NSS
 * The temperature measurement module provides a simple yet flexible API allows the user to measure temperatures, 
 * choosing the measurement resolution and the output unit of measurement.
 *
 * @par Diversity
 *  This module supports diversity, like defining a callback at link time.
 *  Check @ref MODS_NSS_TMEAS_DFT for all diversity parameters.
 *
 * @note This mod provides an implementation of the interrupt vector
 *   #TSEN_IRQHandler and enables and disables the interrupt #TSEN_IRQn.
 *   By including this mod you can thus no longer use the interrupt driver functionality.
 * @note This mod will initialize (enable a clock and provide power) the TSEN driver (by calling
 *   #Chip_TSen_Init) prior to making a measurement, and de-initialize it when the measurement
 *   is complete. Using the TSEN HW block directly is thus highly discouraged when using this mod.
 * @note This mod will by default use the temperature calibration values as determined during production at the factory
 *   site. When a recalibration is in order, these values can be changed by changing the registers
 *   #NSS_TSEN_T.SP1, #NSS_TSEN_T.SP2 and #NSS_TSEN_T.SP3 for the
 *   @c a, @c b resp. @c alpha parameters. See the temperature application note for details on this recalibration.
 *
 *  @par Example 1: measure temperature, waiting for the result
 *  @snippet tmeas_mod_example_1.c tmeas_mod_example_1
 *
 *  @par Example 2: measure temperature, receive the result in a callback
 *  Callback:
 *  @snippet tmeas_mod_example_2.c App_TmeasCb
 *  Main code:
 *  @snippet tmeas_mod_example_2.c tmeas_mod_example_2
 *
 * @{
 */

/* -------------------------------------------------------------------------
 * Include files
 * ------------------------------------------------------------------------- */

#include "chip.h"
#include "tmeas_dft.h"

/* -------------------------------------------------------------------------
 * Types and defines
 * ------------------------------------------------------------------------- */

/**
 * Returned value of #TMeas_Measure to indicate a measurement is already in progress.
 */
#define TMEAS_ERROR (-1)

/** Possible temperature output formats. */
typedef enum TMEAS_FORMAT {
    TMEAS_FORMAT_NATIVE, /*!< Signed 10.6 fixed point in Kelvin. */
    TMEAS_FORMAT_KELVIN, /*!< Deci-degrees in Kelvin. @see http://en.wikipedia.org/wiki/Kelvin */
    TMEAS_FORMAT_CELSIUS, /*!< Deci-degrees in Celsius. @see http://en.wikipedia.org/wiki/Celsius */
    TMEAS_FORMAT_FAHRENHEIT /*!< Deci-degrees in Fahrenheit. @see http://en.wikipedia.org/wiki/Fahrenheit */
} TMEAS_FORMAT_T;

/**
 * Callback function type to report temperature measurement results.
 * @see TMeas_Measure
 * @param resolution : The value as given when #TMeas_Measure was called.
 * @param format : The value as given when #TMeas_Measure was called.
 * @param value : The measured temperature, converted to the format given by @c format.
 *   For the @c NATIVE format, the 16 LSBits are to be used.
 *   For all other formats, @c temperature is to be cast to an @c int32_t.
 * @param context : The value as given when #TMeas_Measure was called.
 */
typedef void (*pTMeas_Cb_t)(TSEN_RESOLUTION_T resolution, TMEAS_FORMAT_T format, int value, uint32_t context);

/* -------------------------------------------------------------------------
 * Exported function prototypes
 * ------------------------------------------------------------------------- */

/**
 * Make one temperature measurement.
 * @param resolution : The required resolution. The higher the resolution, the longer it takes to take one measurement.
 * @param format : The required output format.
 * @param synchronous :
 *   Not looked at when @c TMEAS_CB is not defined; @c true is then always assumed. Else:
 *   - If @c true the function is synchronous, and will only return once the measurement is complete.
 *   - Else the function is asynchronous; it will return immediately, and once the temperature sensor has completed
 *     its measurement, the temperature is reported via the callback function @c TMEAS_CB.
 * @param context : Context information for the caller. You can fill in any value here: it is not used by this mod,
 *   only stored and sent back in a later call to @c TMEAS_CB. Use this as an aid for your own housekeeping, or
 *   disregard this argument: it doesn't impede the mod's working in any way.
 * @return
 *   - If no measurement could be taken (TSEN HW block in use), #TMEAS_ERROR is returned.
 *   - Else, if @c synchronous equals @c true, the measured temperature.
 *   - Else, @c 0 to indicate a measurement is ongoing and the callback will be called when the measurement is ready.
 * @note @c TMEAS_CB will be called under interrupt.
 * @note This function is not re-entrant.
 * @see pTMeas_Cb_t
 */
int TMeas_Measure(TSEN_RESOLUTION_T resolution, TMEAS_FORMAT_T format, bool synchronous, uint32_t context);

#endif /** @} */
