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

#include "board.h"
#include "sense/sense.h"
#include "therapy.h"
#include "memorymanager.h"

/** The minimum value for the sensing interval */
#define INTERVAL_MIN_VALUE 3
/** The maximum value for the sensing interval */
#define INTERVAL_MAX_VALUE 60
/**
 * This structure will contain all status details about the ongoing therapy.
 * Its up to other modules to update this data trough the proper exposed API functions.
 */
typedef struct THERAPY_STATUS_S {
    THERAPY_STATE_T state; /**< Indicates the current state of the therapy. */
    uint32_t startTime; /**< The epoch time at which the therapy was started. */
    uint32_t checkPeriod; /**< The period on which the therapy needs to be checked. If set to 0 no therapy is ongoing. */
    int initialCount; /**< Pill count at start of therapy. */
    int intakeCount; /**< The number of recorded pill intakes. */
    uint32_t lastIntakeTime; /**< The epoch time on which the last pill removal was recorded. */
} THERAPY_STATUS_T;

/** Instance of the therapy status. */
static THERAPY_STATUS_T sTherapyStatus;

static void RecordPillRemoval(const SENSE_PILL_REMOVAL_INFO_T *pInfo);

void Therapy_Init(void)
{
    Memory_GetStoredData(MEMORY_SLOT_ID_THERAPY_INSTANCE, &sTherapyStatus, sizeof(THERAPY_STATUS_T));
}

void Therapy_DeInit(void)
{
    Memory_StoreData(MEMORY_SLOT_ID_THERAPY_INSTANCE, &sTherapyStatus, sizeof(THERAPY_STATUS_T));
}

THERAPY_STATE_T Therapy_GetState(void)
{
    return sTherapyStatus.state;
}

uint32_t Therapy_GetStartTime(void)
{
    return sTherapyStatus.startTime;
}

uint32_t Therapy_GetCheckPeriod(void)
{
    return sTherapyStatus.checkPeriod;
}

int Therapy_GetInitialPillCount(void)
{
    return sTherapyStatus.initialCount;
}

int Therapy_GetIntakeCount(void)
{
    return sTherapyStatus.intakeCount;
}

uint32_t Therapy_GetLastIntakeTime(void)
{
    return sTherapyStatus.lastIntakeTime;
}

void Therapy_Start(uint32_t start, uint32_t interval)
{
    uint8_t senseInstance[SENSE_INSTANCE_SIZE];
    sTherapyStatus.startTime = start;
    Chip_RTC_Time_SetValue(NSS_RTC, (int)start);

    if (interval < INTERVAL_MIN_VALUE) {
        /* Clip to minimal value */
        sTherapyStatus.checkPeriod = INTERVAL_MIN_VALUE;
    }
    else if (interval > INTERVAL_MAX_VALUE) {
        /* Clip to maximal value */
        sTherapyStatus.checkPeriod = INTERVAL_MAX_VALUE;
    }
    else {
        sTherapyStatus.checkPeriod = interval;
    }

    sTherapyStatus.intakeCount = 0;

    /* Fetch the stored Instance */
    Memory_GetStoredData(MEMORY_SLOT_ID_SENSE_INSTANCE, &senseInstance, SENSE_INSTANCE_SIZE);

    sTherapyStatus.initialCount = Sense_StartTherapy(&senseInstance);
    sTherapyStatus.state = THERAPY_STATE_ID_ONGOING;

    Memory_StoreData(MEMORY_SLOT_ID_SENSE_INSTANCE, senseInstance, SENSE_INSTANCE_SIZE);
}

void Therapy_Reset(void)
{
    sTherapyStatus.state = THERAPY_STATE_ID_NOTSTARTED;
    sTherapyStatus.checkPeriod = 0;
    sTherapyStatus.initialCount = 0;
    sTherapyStatus.intakeCount = 0;
    sTherapyStatus.lastIntakeTime = 0;
    sTherapyStatus.startTime = 0;
}

void Therapy_Stop(void)
{
    sTherapyStatus.state = THERAPY_STATE_ID_STOPPED;
}

void Therapy_Update(void)
{
    int remainingPills;
    uint8_t senseInstance[SENSE_INSTANCE_SIZE];
    Sense_RemovalCb_t cb = RecordPillRemoval;

    /* Fetch the stored Instance */
    Memory_GetStoredData(MEMORY_SLOT_ID_SENSE_INSTANCE, &senseInstance, SENSE_INSTANCE_SIZE);
    remainingPills = Sense_SensePillRemoval(&senseInstance, cb);

    /* If no pills remaining, the therapy can be stopped. */
    if (!remainingPills) {
        Therapy_Stop();
    }
    /* Store updated Instance */
    Memory_StoreData(MEMORY_SLOT_ID_SENSE_INSTANCE, &senseInstance, SENSE_INSTANCE_SIZE);
}

/** Function used as CB function for the sense module. */
static void RecordPillRemoval(const SENSE_PILL_REMOVAL_INFO_T *pInfo)
{
    uint8_t group = (uint8_t)(pInfo->group + 1); /* The sensing part start counting from 0, add one to start from 1 */
    uint8_t smallestBit = 0;
    uint32_t pill = pInfo->pill;

    while (pill) {
            smallestBit++;
            pill >>= 1;
        }
    /* Add a text entry in Memory so next time a phone is touched, the text record(s) will contain this new info. */
    Memory_StoreIntakeString(sTherapyStatus.intakeCount, sTherapyStatus.startTime, pInfo->time, group, pInfo->positional,
                             pInfo->initialPillCount, smallestBit);

    /* Store this event in memory such that if messageHandler needs it, he can read it there. */
    THERAPY_PILLREMOVAL_INFO_T removal;
    removal.time = pInfo->time;
    removal.group = group;

    removal.position = smallestBit;

    //make sure memory block is storing right size
    ASSERT(MEMORY_EVENTSIZE == sizeof(THERAPY_PILLREMOVAL_INFO_T));
    Memory_StoreEvent(sTherapyStatus.intakeCount, (uint8_t*)&removal);

    sTherapyStatus.intakeCount += 1;
    sTherapyStatus.lastIntakeTime = pInfo->time;
}
