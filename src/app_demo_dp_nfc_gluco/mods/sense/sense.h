/*
 * Copyright 2016-2017,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef SENSE_H
#define SENSE_H

/** @defgroup APP_DEMO_TADHERENCE_THERAPY_SENSE Sensing Component
 *  @ingroup APP_DEMO_TADHERENCE_THERAPY
 *  The Therapy Adherence Demo Sensing component is responsible for providing a normalized and simplified interface to handle
 *  pill presence sensing. It is also responsible for managing the sensing circuit and configuration of the sensor(s).
 *  Its functionality is simplified down to on single API function to trigger the sensing.
 *  The application will be triggered via a callback function at the moment that an event occurs.
 *
 * @par Memory Requirements:
 *  The SENSE module does not allocate any static R/W data memory. Instead, the caller should take care of the
 *  allocation of the memory required for the SENSE configuration storage.The SENSE module memory
 *  requires an Instance Buffer: The instance buffer preserves the necessary housekeeping information during an
 *  instantiation of the SENSE module. The caller must ensure that the argument pInstance passed to
 *  #Sense_SensePillRemoval points to a buffer of size #SENSE_INSTANCE_SIZE bytes.
 *  The application is responsible to preserve this instance between 2 sense actions.
 * @{
 */

/** The maximum amount of pill groups. */
#define SENSE_MAX_GROUP_COUNT 6

/** Size of Instance buffer required by the SENSE module for internal housekeeping. */
#define SENSE_INSTANCE_SIZE 40

/** Structure used by the sensing component to inform the upper layer (via callback) about a pill removal */
typedef struct {
    uint32_t time; /**< The time at which the pill was removed. */
    uint8_t group; /**< The group containing the removed pill. */
    int initialPillCount; /**< The @c group 's initial pill count */
    bool positional; /**< Indicates whether @c pill represents an actual pill position. */
    uint32_t pill; /**< If the group is positional, bitmask indicating the removed pill.
     else, a right shifting bit, starting with the highest pill number. In all cases, only one bit
     shall be set */
} SENSE_PILL_REMOVAL_INFO_T;

/**
 * Whenever an event (pill removal) is sensed, the application is notified via a callback of this prototype.
 * It then has a chance to act accordingly. Think about updating the NDEF TEXT message and/or storing this event in
 * non-volatile memory. This Callback can be expected during execution of (and only then) #Sense_SensePillRemoval.
 * The application is in charge of:
 * - reading the information from the returned memory location
 * - hard coping this information if it needs to be preserved
 * @param pInfo : pointer to the info about the pill removal which is the subject of the callback.
 */
typedef void (*Sense_RemovalCb_t)(const SENSE_PILL_REMOVAL_INFO_T *pInfo);

/**
 * This function initialize the module for a new therapy.
 * @param pInstance : pointer to ram location where to store internal data,
 *  this buffer needs to be of size #SENSE_INSTANCE_SIZE.
 * @return Number of pills remaining for the started therapy
 */
int Sense_StartTherapy(void *pInstance);

/**
 * This function will sense all pill groups, if a difference is observed between the newly sensed pill removals
 * and the previous (stored in pInstance), @c SENSE_CB is called to inform the higher levels about new event(s).
 * @param pInstance : Base address of instance Buffer. The instance buffer preserves the necessary housekeeping
 *  information during an instantiation of the SENSE module. The caller must ensure that the argument pInstance
 *  points to a buffer of size #SENSE_INSTANCE_SIZE bytes.
 * @param cb : A function pointer to a callback, this will be called for every new pill removal recorded.
 * @return Number of pills left
 * @note @c SENSE_CB will be called if new pill removal is observed.
 */
int Sense_SensePillRemoval(void *pInstance, Sense_RemovalCb_t cb);

/** @} */
#endif
