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

#ifndef TEXT_H_
#define TEXT_H_
#include <string.h>

/** @defgroup APP_DEMO_TADHERENCE_TXT Text Component
 *  @ingroup APP_DEMO_TADHERENCE
 *  This component serves as a helper to generate the text content of the first NDEF message.
 *
 *  It facilitates generation of:
 *      - Status string: presenting the Therapy status.
 *      - Intake string: presenting the data about a pill removal.
 * @{
 */

/** The maximum length of the status string */
#define TEXT_STATUS_LENGTH (94)

/** The length of a single intake string */
#define TEXT_POS_INTAKE_LENGTH (24)

/** The length of a single intake string */
#define TEXT_NPOS_INTAKE_LENGTH (31)

/**
 * Generate the status string for a therapy which is not yet started.
 * @return a pointer to memory address where the status string can be read. The memory contents remain valid until a
 *  next call to any text API call. */
char* Text_StatusNotStarted(void);

/**
 * Generate the status string for a therapy which is stopped (all pills taken).
 * @param startTime : epoch time at which the therapy was started.
 * @param lastIntakeTime : epoch time at which the last pill removal was sensed.
 * @return pointer to memory address where the status string can be read. The memory contents remain valid until a
 *  next call to any text API call. */
char* Text_StatusStopped(uint32_t startTime, uint32_t lastIntakeTime);

/**
 * Generate the status string for a therapy which is ongoing.
 * @param startTime : epoch time at which the therapy was started.
 * @param now : current epoch time.
 * @param pillsRemaining : The current remaining pills.
 * @return a pointer to memory address where the status string can be read. The memory contents remain valid until a
 *  next call to any text API call. */
char* Text_StatusOngoing(uint32_t startTime, uint32_t now, int pillsRemaining);

/**
 * Prepares a string describing a positional pill removal moment.
 * @param startTime : The time at which the therapy started.
 * @param intakeTime : The time at which the pill was taken.
 * @param group : Group containing the pill.
 * @param position : The position of the taken pill in the @c group.
 * @return A pointer to the start of the prepared string. The memory contents remain valid until a
 *  next call to this API call. */
char* Text_IntakePositional(uint32_t startTime, uint32_t intakeTime, int group, int position);

/**
 * Prepares a string describing a non positional pill removal moment.
 * @param startTime : The time at which the therapy started.
 * @param intakeTime : The time at which the pill was taken.
 * @param group : Group containing the pill.
 * @param removal : The removal number.
 * @return A pointer to the start of the prepared string. The memory contents remain valid until a
 *  next call to this API call. */
char* Text_IntakeNonPositional(uint32_t startTime, uint32_t intakeTime, int group, int removal);

/** @} */
#endif
