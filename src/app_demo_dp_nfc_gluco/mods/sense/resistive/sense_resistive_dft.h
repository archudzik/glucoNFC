/*
 * Copyright 2016 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __SENSE_RESISTIVE_DFT_H_
#define __SENSE_RESISTIVE_DFT_H_

#include "sense_resistive_sel.h"
/**
 * @defgroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC_RESISTIVE_DFT Diversity settings for the resistive sensing principle.
 * @ingroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC
 * This part defines possible diversity for the resistive sensing principle.
 *
 * With this sensing method there are 2 possible principles as defined by #SENSE_RES_SENS_PRINC_T.
 * For both these principles, the pill vector is deduced from the admittance of the network/ladder.
 * To determine the admittance, both the voltage and the current are measured.
 * The following procedure is followed:
 *  -# The drive pin is driven by the DAC.
 *  -# Using the ADC the voltage of both the drive and the sense pin is measured.
 *  -# These values are combined to calculate the voltage drop caused by the resistive network/ladder.
 *  -# Using the I2D the current is measured (flowing into the sense pin), this current is equal to the current
 *      flowing through the network/ladder.
 *  -# The admittance of the network is then equal to the current divided by the voltage drop.
 * @{
 */

/**
 * Sensing principle used for pill presence sensing
 */
typedef enum SENSE_RES_SENS_PRINC {
    /**
     * A parallel circuit of resister/pill pairs where all resistors values are equal.
     * This principle makes it possible to sense the amount of taken pills in a group.
     *  @code
     *  DRIVE o------------
     *         |   |  |  |
     *         |   R  R  R  ...
     *         R   |  |  |
     *         |   p  p  p  ...
     *         |   |  |  |
     *  SENSE o------------
     *  @endcode
     */
    SENSE_RES_SENS_PRINC_PAR_EVEN,

    /**
     * A parallel circuit of resister/pill pairs where all resistors values are different.
     * This principle makes it possible to sense the amount of taken pills in a group
     * and to distinguish which pills where taken.
     *  @code
     *  DRIVE o-----------------
     *        |   |     |     |
     *        |  R/2   R/4   R/8
     *        R   |     |     |
     *        |   p     p     p
     *        |   |     |     |
     *  SENSE o-----------------
     *  @endcode
     */
    SENSE_RES_SENS_PRINC_PAR_UNEVEN,
} SENSE_RES_SENS_PRINC_T;

/**
 * Defines all properties of a GROUP this module requires to know to sense the pills in an uniform way.
 */
typedef struct GROUP_PROPERTIES_S {
    SENSE_RES_SENS_PRINC_T principle; /*!< The group's sensing principle. */
    ADCDAC_IO_T DAC_drivePin; /*!< The group's DA drive pin */
    ADCDAC_IO_T ADC_senseInput; /*!< The group's AD sense input */
    I2D_INPUT_T I2D_senseInput; /*!< The group's I2D sense input */
    uint8_t pills; /*!< Bitmask of pills initially present in the group. */
} GROUP_PROPERTIES_T;

#if !GROUP_COUNT || !defined(GROUP_PROPERTIES)
#error GROUP_COUNT is not defined or is 0. No pills can be sensed.
#endif

#endif
/**
 * @}
 */
