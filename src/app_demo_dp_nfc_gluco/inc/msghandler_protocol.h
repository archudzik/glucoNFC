/*
 * Copyright 2016-2018,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef MSGHANDLER_PROTOCOL_H
#define MSGHANDLER_PROTOCOL_H

/** @defgroup APP_DEMO_TADHERENCE_MSGHANDLER_PROTOCOL 'tadherence' app.spec. messages
 * @ingroup APP_DEMO_TADHERENCE_MSGHANDLER
 *  This file describes the application specific commands and responses used in the Therapy Adherence demo application.
 * @{
 */

#include "msg/msg.h"
#include "therapy.h"

/**
 * List of all message ids, referring to the respective command and response structures.
 */
typedef enum MSGHANDLER_MSG_ID {
    /**
     * @c 0x5A @n
     * Removes all information that was received via any command.
     * Removes all stored events.
     * @param MSG_CMD_TAGONLY_T
     * @return MSG_RESPONSE_RESULTONLY_T
     * @note synchronous command
     */
    MSG_ID_SETPRISTINE = 0x5A,

    /**
     * @c 0x55 @n
     * Defines the rhythm the patient must adhere to when following the therapy. The application will
     * determine based on this information whether the patient still adheres to the therapy.
     * @note In case the application tracks different pills - each pill is specific and the order of intake of each pill
     *  is important - it is assumed the first pill must ideally be taken at the first pill intake moment, the nth pill
     *  must ideally be taken at the nth pill intake moment, and so on.
     * @param MSG_CMD_SETRHYTHM_T
     * @return MSG_RESPONSE_RESULTONLY_T
     * @note synchronous command
     */
    MSG_ID_SETRHYTHM = 0x55,

    /**
     * @c 0x56 @n
     * Retrieves rhythm information that was set by the last command with id #MSG_ID_SETRHYTHM.
     * @param MSG_CMD_TAGONLY_T
     * @return MSG_RESPONSE_GETRHYTHM_T
     * @note synchronous command
     */
    MSG_ID_GETRHYTHM = 0x56,

    /**
     * @c 0x57 @n
     * Sets the time when the therapy starts. When this command is received, events will be tracked and followed up by
     * the IC. All events before the given start time are considered out-of-band; all events after the given start time
     * that do not follow the prescribed rhythm are also considered out-of-band.
     * @note Mandatory configuration messages that must be exchanged before this command is accepted:
     *  - #MSG_ID_SETRHYTHM
     * @note The therapy ends automatically when all drugs have been taken, or when a command with message id
     *  #MSG_ID_SETPRISTINE is received.
     * @pre The therapy has not yet been started.
     * @param MSG_CMD_START_T
     * @return MSG_RESPONSE_RESULTONLY_T
     * @note synchronous command
     */
    MSG_ID_START = 0x57,

    /**
     * @c 0x58 @n
     * Retrieves the start time that was set by the last command with id #MSG_ID_START.
     * @param MSG_CMD_TAGONLY_T
     * @return MSG_RESPONSE_GETSTART_T
     * @note synchronous command
     */
    MSG_ID_GETSTART = 0x58,

    /**
     * @c 0x5D @n
     * Retrieves all pill removals that have been recorded since the start of the therapy.
     * @param MSG_CMD_TAGONLY_T
     * @return MSG_RESPONSE_GETPILLREMOVALS_T
     * @note synchronous command
     * @note Retrieving the pill removals does @b not clear them from the IC. Issuing the same command again will yield the
     *  same response - possibly expanded with new removals.
     */
    MSG_ID_GETPILLREMOVALS = 0x5D,
} MSGHANDLER_MSG_ID_T;

/* ------------------------------------------------------------------------- */

#pragma pack(push, 1)

/**
 * Structure used to define both #MSG_CMD_SETRHYTHM_T and #MSG_RESPONSE_GETRHYTHM_T
 * @see MSG_ID_SETRHYTHM
 * @see MSG_ID_GETRHYTHM
 */
typedef struct MSG_RHYTHM_S {
    /**
     * The total time in seconds of one period, in which all intakes described below must take place.
     * @note For example, to describe a rhythm in which the pill intakes are the same for each day, this value will
     *  equal @c 86400.
     */
    uint32_t period;

    /**
     * An array of times at which a pill intake ideally takes place.
     * The time is expressed in seconds and is relative to the start of a period.
     * A maximum of 16 pill intake moments can be defined: in case less moments are required, a value equal to or
     * bigger than @c period must be assigned to the superfluous elements in the array.
     * @note Continuing the example with @c period equal to 1 day,
     * @pre Must be less than @c period
     * @note Multiple pill intake moments can be defined at the same time.
     * @note The pill intake moments must be sorted ascending, i.e. the (n+1)th entry must be equal to or bigger than
     *  the nth entry.
     * @note Continuing the example with @c period equal to 1 day, an array with values @c 28800, @c 64800, @c 64800
     *  followed by @c 0xffffffff for all remainder array would define two pill intake moments: the first moment at
     *  8:00.00 AM for the first pill, the second moment at 6:00.00 PM for the second and the third pill.
     */
    uint32_t intakeOffset[16];

    /**
     * @b Half the width of the band around each ideal pill intake moment which is still considered adherent to the
     * therapy. Expressed in number of seconds.
     * @pre Must be bigger than 0
     * @pre Must be less than @c period
     * @note The width of the band is @c double the value given here.
     * @note The ideal pill intake moment is centered in this band, meaning this value indicates the allowed deviation
     *  in absolute value.
     * @note The band for a pill intake moment may be as wide as to include the ideal time of an earlier or later
     *  pill intake moment, or may spill over to an earlier or later period.
     * @note It is possible that due to the width of the bands the (n+1)th pill - which was ideally taken after the nth
     *  pill - is taken before the nth pill and still the therapy is considered adherent.
     * @note Continuing the example with @c period equal to 1 day and pill intake moments at 8 AM and 6 PM, a value of
     *  @c 7200 indicates that the first pill must be taken after 6:00.00 AM and before 10:00.00 AM, and the second and
     *  third pills must be taken after 4:00.00 PM and before 8:00.00 PM, in any order.
     */
    uint32_t leniency;
} MSG_RHYTHM_T;

/** @see MSG_ID_SETRHYTHM */
typedef MSG_RHYTHM_T MSG_CMD_SETRHYTHM_T;

/** @see MSG_ID_START */
typedef struct MSG_CMD_START_S {
    uint32_t current; /**< The current time in epoch seconds. */
    uint32_t updateInterval; /**< The number of seconds in between 2 updates (pill sensing). */
} MSG_CMD_START_T;

/* ------------------------------------------------------------------------- */

/** @see MSG_ID_GETRHYTHM */
typedef MSG_RHYTHM_T MSG_RESPONSE_GETRHYTHM_T;

/** @see MSG_ID_GETSTART */
typedef struct MSG_RESPONSE_GETSTART_S {
    uint32_t current; /**< The current time in epoch seconds. */
    uint32_t start; /**< The time when the therapy was started in epoch seconds. */
} MSG_RESPONSE_GETSTART_T;

/** @see MSG_ID_GETEVENTS */
typedef struct MSG_RESPONSE_GETPILLREMOVALS_S {
    /**
     * An array of pill removals. Each slot reports one event:
     * when two events occurred at the same time, two slots are taken with the same value for @c time.
     * When less events occurred than there are slots available, the remainder of the slots are filled with zeros.
     * @note The events are ordered in time, i.e. with @c m less than n, the time given in slot @c m will be less than
     *  or equal to the time given in slot @c n.
     */
    THERAPY_PILLREMOVAL_INFO_T removals[THERAPY_MAX_REMOVALS];
} MSG_RESPONSE_GETPILLREMOVALS_T;

#pragma pack(pop)

#endif /** @} */
