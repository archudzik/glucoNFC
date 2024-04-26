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

#ifndef APP_SENSE_SPECIFIC_H
#define APP_SENSE_SPECIFIC_H

/** @defgroup APP_DEMO_TADHERENCE_THERAPY_SENSE_SPECIFIC Specific part for the Sensing Component
 *  @ingroup APP_DEMO_TADHERENCE_THERAPY_SENSE
 *  This part is responsible for providing a normalized and simplified interface from the Sensing component to the
 *  principle depending implementation.
 *  The specific part is responsible for the measuring principle (implementation) and for the board setup.
 *  @par Groups
 *      The specific part defines the board setup, to do this , it divides the available pills into groups.
 *      A group is defined by its drive line, the number of available groups can be obtained by calling
 *      #SenseSpecific_GetAmountOfGroups. For a specific group, the initial available pills can be obtained via
 *      #SenseSpecific_InitialPillCount, the present pills can be obtained via #SenseSpecific_GetPillsInGroup.
 *
 * @{
 */

/**
 * Function initializes all needed HW blocks.
 */
void SenseSpecific_Init(void);

/**
 * Function DeInitializes all used HW blocks.
 */
void SenseSpecific_DeInit(void);

/**
 * Function to get the number of groups from the specific implementation, needed to call #SenseSpecific_GetPillsInGroup
 * for each available group.
 * @return The number of groups available in the current setup (board/foil dependent).
 * @pre #SenseSpecific_Init is called
 */
int SenseSpecific_GetAmountOfGroups(void);

/**
 * Function to get the pill bitmap of pills in a group. Depending on the sensing principal, the corresponding
 *  specific implementation will perform all needed to determine this value.
 * @param group : The group number for which the pill value is asked.
 * @param pStatus [in/out] : A pointer to a status field to be used internally in this function.
 *  Its up to the specific implementation to decide whether this parameter is used or not,
 *  it can be used to store a calibration value. It's up to the higher level to make sure to preserve this value.
 *  It's up to the higher level(s) to make sure this field is preserved between two consecutive calls to
 *  #SenseSpecific_GetPillsInGroup.
 * @param calibrated : Flag indicating whether the requested group is calibrated, iow.: @c pStatus holds a valid value.
 *  Only the very first time this function is called (at the beginning of the lifetime), this bool should be set false.
 * @return A bitmap with a set bit for each pill present in the group.
 */
uint32_t SenseSpecific_GetPillsInGroup(int group, uint16_t* pStatus, bool calibrated);

/** Helper function returning the initial pill count of a specific group.
 * @param group : Group of which the initial pill count is requested.
 * @return Initial number of pills for @c group
 */
int SenseSpecific_InitialPillCount(int group);

/** Helper function returning whether a specific group is positional.
 * @param group : Group of which one likes to know if it is positional.
 * @return
 * - @c true if in @c group, pills are being monitored positional,  this means that for each pill removal
 *  the sense module will know exactly which pill was removed, pills are monitored individually.
 * - @c false, if for @c group only the amount of present/removed pills is known,
 *  pills are monitored as a group not individually.
 *
 */
bool SenseSpecific_GroupPositional(int group);

/** @} */
#endif
