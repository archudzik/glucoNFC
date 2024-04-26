/*
 * Copyright 2016,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef THERAPY_H_
#define THERAPY_H_
/** @defgroup APP_DEMO_TADHERENCE_THERAPY Therapy Component
 *  @ingroup APP_DEMO_TADHERENCE
 *  The Therapy Adherence Therapy Component is responsible for managing the therapy adherence status.
 *  It makes use of:
 *      - The @ref APP_DEMO_TADHERENCE_MEMORY module to store its internal data into non-volatile memory.
 *      - The @ref APP_DEMO_TADHERENCE_THERAPY_SENSE module to sense the presence/removal of the pills.
 *  Via the Therapy Component, other components can:
 *      -# Check the current state of the therapy.
 *      -# Start a new therapy.
 *      -# Stop an ongoing therapy.
 *      -# Reset the status of the therapy to pristine state.
 *      -# Force an update of the internal status.
 * @par Groups:
 *  The available pills in the blister are divided into groups. Each group is defined by its drive pin (HW connection).
 *  Depending on the sensing principle, a group can have one ore more sensing pins.
 *  Depending on the sensing principle, a group can be capable of:
 *      - Sensing a specific individual pill removal.
 *      - Sensing the number of removed pills, this means all pills in such a group are treated as equals.
 * @{
 */

/** Maximum pill removals to be tracked. This number is defined by HW.
 *  The maximum capacity is obtained by a 6x6 matrix. (GPIO)
 */
#define THERAPY_MAX_REMOVALS 36

#pragma pack(push, 1)
/**
 * All information required to be recorded when a pill removal occurs:, i.e. when a pill removal has been detected.
 */
typedef struct THERAPY_PILLREMOVAL_INFO_S {
    /**
     * The time in epoch seconds when the removal occurred. A value of @c 0 indicates no event is recorded in this
     * slot.
     */
    uint32_t time;

    /**
     * The number of the group where the event occurred.
     */
    uint8_t group;

    /**
     * - When in the group specific pills are to be tracked, this value indicates which pill was taken:
     *      - a value of @c 0 for the first pill
     *      - a value of @c n for the n-th pill
     * - When in the group only the amount of pills taken can be tracked reporting is starting with the highest pill number.
     */
    uint32_t position;
}THERAPY_PILLREMOVAL_INFO_T;
#pragma pack(pop)

/**
 * A list of all possible states a therapy can be in.
 * As state data will be stored in EEPROM, make sure the reset value (0x00) is not a valid state id.
 */
typedef enum THERAPY_STATE {
    THERAPY_STATE_ID_NOTSTARTED = 0x10, /**< State indicating a therapy is not yet started. */
    THERAPY_STATE_ID_ONGOING = 0x11, /**< State indicating a therapy is ongoing. */
    THERAPY_STATE_ID_STOPPED = 0x12, /**< State indicating a therapy is stopped (all pills taken). */
} THERAPY_STATE_T;

/**
 * Init function for the therapy module.
 *  It makes sure that internal status info is retrieved from memory.
 */
void Therapy_Init(void);

/**
 * De-Init function for the therapy module.
 *  It makes sure that internal status info is stored in memory.
 */
void Therapy_DeInit(void);

/**
 * Function to get the current state of the therapy.
 * @return The current status of the therapy. */
THERAPY_STATE_T Therapy_GetState(void);

/**
 * Function to get the start time of the therapy  (expressed in epoch seconds). .
 * @return The therapy start time (not valid if the status is #THERAPY_STATE_ID_NOTSTARTED).
 */
uint32_t Therapy_GetStartTime(void);

/**
 * Function to get the period on which the therapy needs to be updated (pill removal check).
 * @return The therapy's check period (not valid if the status is #THERAPY_STATE_ID_NOTSTARTED).
 */
uint32_t Therapy_GetCheckPeriod(void);

/**
 * Function to get the amount of pills present at start of therapy.
 * @return The therapy's initial pill count (not valid if the status is #THERAPY_STATE_ID_NOTSTARTED).
 */
int Therapy_GetInitialPillCount(void);

/**
 * Function to get the amount of pill intakes currently recorded.
 * @return The amount of sensed pill removals (not valid if the status is #THERAPY_STATE_ID_NOTSTARTED).
 */
int Therapy_GetIntakeCount(void);

/**
 * Function to get the last experienced pill removal time (expressed in epoch seconds).
 * @return The time (expressed in epoch seconds) at which the last pill removal was sensed
 *  (not valid if the status is #THERAPY_STATE_ID_NOTSTARTED).
 */
uint32_t Therapy_GetLastIntakeTime(void);

/**
 * Starts a new therapy
 * @param start : The time (expressed in epoch seconds) at which the therapy is started
 * @param interval : The interval at which an update of the therapy status is needed.
 * */
void Therapy_Start(uint32_t start, uint32_t interval);

/**
 * Resets the therapy (Pristine state)
 * All Status data is reseted, Therapy state is set to #THERAPY_STATE_ID_NOTSTARTED.
 */
void Therapy_Reset(void);

/**
 * Stops the therapy (All pills removed)
 * Therapy state is set to #THERAPY_STATE_ID_STOPPED, restarting the Therapy is not possible,
 * this is since from the moment the Therapy is stopped knowledge of current time is lost.
 */
void Therapy_Stop(void);

/**
 *  Updates the therapy's status, it will check (via the sense module) if since the last call,
 *  extra pills where removed and will update its status accordingly.
 *  If all pills are taken, Therapy will be stopped #THERAPY_STATE_ID_STOPPED
 */
void Therapy_Update(void);

#endif /** @} */
