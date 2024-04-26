/*
 * Copyright 2016-2017 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __APP_SEL_GPIO_H_
#define __APP_SEL_GPIO_H_

/**
 * @defgroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC_GPIO_SEL Configuration settings
 * @ingroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC_GPIO_DFT
 * These 'defines' capture the diversity settings of the compound.
 * It keeps the settings about 'sensing groups', it actually holds the configurations of the demo board/smart blister.
 *
 * @{
 */

/** USECASE to be defined in the build configuration
 *  USECASE 10: (10 pills) Demo without extra components.
 */
#if APP_SENSE_GPIO == 10
/**
 * The number of groups present in this usecase.
 * Matches the length of #GROUP_PROPERTIES.
 */
#define GROUP_COUNT 2

/**
 * The maximum number of pills in a single groups.
 * Matches the maximum length of @ref GROUP_PROPERTIES_T.IO_sensePin.
 */
#define GROUP_MAX_PILLS 9

/**
 * The group properties for the 2 groups present on the demo PCB.
 * Declared global, so they can be assigned to macro GROUP_PROPERTIES.
 */
#define GROUP_PROPERTIES {  /* GROUP 1 */ {IOCON_PIO0_5 , { /* pill 1.1 */ IOCON_PIO0_9}, 1}, \
                            /* GROUP 2 */ {IOCON_PIO0_9 , { /* pill 2.1 */ IOCON_PIO0_7, \
                                                            /* pill 2.2 */ IOCON_PIO0_3, \
                                                            /* pill 2.3 */ IOCON_PIO0_10, \
                                                            /* pill 2.4 */ IOCON_PIO0_11, \
                                                            /* pill 2.5 */ IOCON_PIO0_8, \
                                                            /* pill 2.6 */ IOCON_PIO0_6, \
                                                            /* pill 2.7 */ IOCON_PIO0_2, \
                                                            /* pill 2.8 */ IOCON_PIO0_1, \
                                                            /* pill 2.9 */ IOCON_PIO0_0}, 9}}

/** USECASE 14(default): (14 pills) Demo with diode matrix combining 4 drive and 4 sense lines. */
#else
/**
 * The number of groups present in this use case.
 * Matches the length of #GROUP_PROPERTIES.
 */
#define GROUP_COUNT 4

/**
 * The maximum number of pills in a single groups.
 * Matches the maximum length of @ref GROUP_PROPERTIES_T.IO_sensePin.
 */
#define GROUP_MAX_PILLS 4

/**
 * The group properties for the 4 groups present on the demo PCB.
 * Declared global, so they can be assigned to macro GROUP_PROPERTIES.
 */
#define GROUP_PROPERTIES {  /* GROUP 1 */ {IOCON_PIO0_2 , { /* pill 1.1 */ IOCON_PIO0_6, \
                                                            /* pill 1.2 */ IOCON_PIO0_8, \
                                                            /* pill 1.3 */ IOCON_PIO0_9, \
                                                            /* pill 1.4 */ IOCON_PIO0_3}, 4}, \
                            /* GROUP 2 */ {IOCON_PIO0_1 , { /* pill 2.1 */ IOCON_PIO0_8, \
                                                            /* pill 2.2 */ IOCON_PIO0_9, \
                                                            /* pill 2.3 */ IOCON_PIO0_3}, 3}, \
                            /* GROUP 3 */ {IOCON_PIO0_5 , { /* pill 3.1 */ IOCON_PIO0_6, \
                                                            /* pill 3.2 */ IOCON_PIO0_8, \
                                                            /* pill 3.3 */ IOCON_PIO0_9, \
                                                            /* pill 3.4 */ IOCON_PIO0_3}, 4}, \
                            /* GROUP 4 */ {IOCON_PIO0_4 , { /* pill 4.1 */ IOCON_PIO0_8, \
                                                            /* pill 4.2 */ IOCON_PIO0_9, \
                                                            /* pill 4.3 */ IOCON_PIO0_3}, 3}}

#endif

#endif /** @} */
