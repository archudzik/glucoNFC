/*
 * Copyright 2016-2017,2020-2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include "sense/sense.h"
#include "sense/sense_specific.h"

static int BitCount(unsigned int value);

/**
 * SENSE housekeeping data structure.
 * This is strictly for internal use by the SENSE module implementation.
 */
typedef struct {
    uint32_t lastKnownPillPresence[SENSE_MAX_GROUP_COUNT]; /**< The last known pill presence for each group. */
    uint16_t stateInfo[SENSE_MAX_GROUP_COUNT]; /**< Internal state info (Used if needed by specific implementation). */
} SENSE_RES_INSTANCE_T;

int Sense_StartTherapy(void *pInstance)
{
    SENSE_RES_INSTANCE_T *pInst = (SENSE_RES_INSTANCE_T *)pInstance;
    const int numOfGroups = SenseSpecific_GetAmountOfGroups();
    int group = 0;
    int pillsInGroup = 0;
    int totalPillCount = 0;
    SenseSpecific_Init();

    for (group = 0; group < numOfGroups; group++) {
        bool groupOk = false;
        while (!groupOk) {
            pInst->lastKnownPillPresence[group] = SenseSpecific_GetPillsInGroup(group, &(pInst->stateInfo[group]),
                                                                                false);
            pillsInGroup = BitCount(pInst->lastKnownPillPresence[group]);
            /* At start of therapy we expect all pills to be present... */
            if (pillsInGroup == SenseSpecific_InitialPillCount(group)) {
                groupOk = true;
            }
            else {
                /* This case means that a therapy is started with less pills than the group can contain.
                 * This is unusual so re-measure to be very sure...
                 * Mind that this can only happen with demos where 'pills' can be re-inserted. */
                uint32_t temp = pInst->lastKnownPillPresence[group];
                pInst->lastKnownPillPresence[group] = SenseSpecific_GetPillsInGroup(group, &(pInst->stateInfo[group]),
                                                                                    false);
                if (temp == pInst->lastKnownPillPresence[group]) {
                    groupOk = true;
                }
            }
        }
        totalPillCount += pillsInGroup;
    }
    SenseSpecific_DeInit();
    return totalPillCount;
}

int Sense_SensePillRemoval(void *pInstance, Sense_RemovalCb_t cb)
{
    SENSE_RES_INSTANCE_T *pInst = (SENSE_RES_INSTANCE_T *)pInstance;
    uint8_t group;
    const int numOfGroups = SenseSpecific_GetAmountOfGroups();
    int totalPillCount = 0;
    uint32_t newState;

    SenseSpecific_Init();
    for (group = 0; group < numOfGroups; group++) {
        /* Do not sense if all pills are already out. */
        if (pInst->lastKnownPillPresence[group]) {
            newState = SenseSpecific_GetPillsInGroup(group, &(pInst->stateInfo[group]), true);
            if (newState != pInst->lastKnownPillPresence[group]) {
                uint32_t temp = newState - 1;
                /* theoretically it is possible that due to an external event,
                 * we pick up unwanted signals which harm the accuracy of the measurement. Therefore we measure
                 * until two consecutive measurements are in close range with each other.  */
                while (temp != newState) {
                    temp = newState;
                    newState = SenseSpecific_GetPillsInGroup(group, &(pInst->stateInfo[group]), true);
                }
                newState &= pInst->lastKnownPillPresence[group];

                /* If still different, update the saved state and inform upper level */
                if (newState != pInst->lastKnownPillPresence[group]) {
                    uint32_t now = (uint32_t)Chip_RTC_Time_GetValue(NSS_RTC);
                    SENSE_PILL_REMOVAL_INFO_T pillInfo = {now,
                                                          group,
                                                          SenseSpecific_InitialPillCount(group),
                                                          SenseSpecific_GroupPositional(group),
                                                          (uint8_t)0};

                    /* For all new pill removals, inform the upper layer. */
                    uint32_t pillsTaken = newState ^ pInst->lastKnownPillPresence[group];
                    uint32_t pill = 0x80000000;

                    while (pillsTaken) {
                        if (pillsTaken & pill) {
                            pillsTaken ^= pill;
                            pillInfo.pill = pill;
                            cb(&pillInfo);
                        }
                        pill >>= 1;
                    }
                    pInst->lastKnownPillPresence[group] = newState;
                }
            }
            totalPillCount += BitCount(pInst->lastKnownPillPresence[group]);
        }
    }
    SenseSpecific_DeInit();
    return totalPillCount;
}

/**
 * Counting bits set, Peter Wegner's or Brian Kernighan's way
 * @param value Count the number of bits set in @c value
 * @return The total bits set in @c value
 */
static int BitCount(unsigned int value)
{
    int count;
    for (count = 0; value; count++) { /* goes through as many iterations as there are set bits */
        value &= value - 1; /* clear the least significant bit set */
    }
    return count;
}
