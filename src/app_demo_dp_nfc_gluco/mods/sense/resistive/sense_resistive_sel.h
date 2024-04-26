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

#ifndef __APP_SEL_RES_H_
#define __APP_SEL_RES_H_
/**
 * @defgroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC_RESISTIVE_SEL Configuration settings.
 * @ingroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC_RESISTIVE_DFT
 * These 'defines' capture the diversity settings of the compound.
 * It keeps the settings about 'sensing groups', it actually holds the configurations of the demo board/smart blister.
 *
 * @{
 */

/** USECASE to be defined in the build configuration
 *  USECASE 35: (35 pills) Demo combining 5 EVEN groups both containing 7 pills
 */
#if APP_SENSE_RESISTIVE == 35

/**
 * The number of groups present in this usecase.
 * Matches the length of #GROUP_PROPERTIES.
 */
#define GROUP_COUNT 5

/**
 * The group properties for the five groups present on the demo PCB.
 * Declared global, so they can be assigned to macro GROUP_PROPERTIES.
 * They will be used as an instance of @ref GROUP_PROPERTIES_T.
 */
#define GROUP_PROPERTIES {  /* GROUP 1 */ {SENSE_RES_SENS_PRINC_PAR_EVEN, ADCDAC_IO_ANA0_0, ADCDAC_IO_ANA0_1, I2D_INPUT_ANA0_1, (1<<7)-1},\
                            /* GROUP 2 */ {SENSE_RES_SENS_PRINC_PAR_EVEN, ADCDAC_IO_ANA0_0, ADCDAC_IO_ANA0_2, I2D_INPUT_ANA0_2, (1<<7)-1},\
                            /* GROUP 3 */ {SENSE_RES_SENS_PRINC_PAR_EVEN, ADCDAC_IO_ANA0_0, ADCDAC_IO_ANA0_3, I2D_INPUT_ANA0_3, (1<<7)-1},\
                            /* GROUP 4 */ {SENSE_RES_SENS_PRINC_PAR_EVEN, ADCDAC_IO_ANA0_0, ADCDAC_IO_ANA0_4, I2D_INPUT_ANA0_4, (1<<7)-1},\
                            /* GROUP 5 */ {SENSE_RES_SENS_PRINC_PAR_EVEN, ADCDAC_IO_ANA0_0, ADCDAC_IO_ANA0_5, I2D_INPUT_ANA0_5, (1<<7)-1}}

/** USECASE 13: (13 pills) Mixed demo combining 2 UNEVEN groups both containing 3 pills and 1 EVEN group containing 7 pills
 *  This is the default usecase.
 */
#else
/**
 * The number of groups present in this usecase.
 * Matches the length of #GROUP_PROPERTIES.
 */
#define GROUP_COUNT 3
/**
 * The group properties for the tree groups present on the demo PCB.
 * Declared global, so they can be assigned to macro GROUP_PROPERTIES.
 * They will be used as an instance of @ref GROUP_PROPERTIES_T.
 */
#define GROUP_PROPERTIES {  /* GROUP 1 */ {SENSE_RES_SENS_PRINC_PAR_UNEVEN, ADCDAC_IO_ANA0_4, ADCDAC_IO_ANA0_5, I2D_INPUT_ANA0_5, (1<<3)-1}, \
                            /* GROUP 2 */ {SENSE_RES_SENS_PRINC_PAR_UNEVEN, ADCDAC_IO_ANA0_2, ADCDAC_IO_ANA0_3, I2D_INPUT_ANA0_3, (1<<3)-1},\
                            /* GROUP 3 */ {SENSE_RES_SENS_PRINC_PAR_EVEN, ADCDAC_IO_ANA0_0, ADCDAC_IO_ANA0_1, I2D_INPUT_ANA0_1, (1<<7)-1}}
#endif

/**
 * @}
 */
#endif
