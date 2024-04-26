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

#include "chip.h"
#include "text.h"

/* ------------------------------------------------------------------------- */

/*                                    1         2         3         4 */
/*                          01234567890123456789012345678901234567890 */
#define STRING_NOT_STARTED "Adherence monitoring has not yet started."

/*                                1         2         3         4         5         6   */
/*                      012345678901234567890123456789012345678901234567890123456789012 */
#define STRING_STARTED "Adherence monitoring started             ago.    pill  remain  "
/*                                                   CCC UUUUUUU      RR     P       NN */
/*                      Adherence monitoring started 111 seconds ago. 11 pills remain.  */
/*                      Adherence monitoring started  77 minutes ago.  1 pill  remains. */
/*                      Adherence monitoring started   9 days    ago.  9 pills remains  */

#define STRING_STARTED_START_COUNT_POS 31 /**< Last position of field */
#define STRING_STARTED_START_COUNT_LENGTH 3
#define STRING_STARTED_START_UNIT_POS 33 /**< Start position of field */
#define STRING_STARTED_START_UNIT_LENGTH 7
#define STRING_STARTED_REMAINING_COUNT_POS 47 /**< Last position of field */
#define STRING_STARTED_REMAINING_COUNT_LENGTH 2
#define STRING_STARTED_PLURAL_POS 53 /**< Start position of field */
#define STRING_STARTED_PLURAL_LENGTH 1
#define STRING_STARTED_NOUN_POS 61 /**< Start position of field */
#define STRING_STARTED_NOUN_LENGTH 2

/*                                1         2         3         4         5         6         7         8         9 */
/*                      012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012 */
#define STRING_STOPPED "Adherence monitoring started at            epoch and lasted            . All pills are taken."
/*                                                      CCCCCCCCCC                  CCC UUUUUUU                       */
/*                      Adherence monitoring started at 1234567890 epoch and lasted  33 days   . All pills are taken. */

#define STRING_STOPPED_START_TIME_POS 41 /**< Last position of field */
#define STRING_STOPPED_START_LENGTH 10
#define STRING_STOPPED_STOP_COUNT_POS 62 /**< Last position of field */
#define STRING_STOPPED_STOP_COUNT_LENGTH 3
#define STRING_STOPPED_STOP_UNIT_POS 64 /**< Start position of field */
#define STRING_STOPPED_STOP_UNIT_LENGTH 7

/*                                       1         2    */
/*                             0123456789012345678901 2 */
#define STRING_POS_INTAKE_POS " #   after            \n" /* Must match APP_STR_POS_INTAKE_LENGTH */
/*                               NN       CCC UUUUUUU  */
/*                             2# 3 after  77 minutes  */
/*                             0#10 after   2 hours    */

#define STRING_POS_INTAKE_GROUP_POS 0 /**< Last position of group */
#define STRING_POS_INTAKE_GROUP_LENGTH 1
#define STRING_POS_INTAKE_NUMBER_POS 3 /**< Last position of field */
#define STRING_POS_INTAKE_NUMBER_LENGTH 2
#define STRING_POS_INTAKE_COUNT_POS 13 /**< Last position of field */
#define STRING_POS_INTAKE_COUNT_LENGTH 3
#define STRING_POS_INTAKE_UNIT_POS 15 /**< Start position of field */
#define STRING_POS_INTAKE_UNIT_LENGTH 7

/*                                        1         2          */
/*                              01234567890123456789012345678 9 */
#define STRING_NPOS_INTAKE_POS " #x (    ) after             \n" /* Must match APP_STR_NPOS_INTAKE_LENGTH */
/*                                   NNYY        CCC UUUUUUU */
/*                              2#x ( 1st) after  77 minutes */
/*                              3#x ( 5th) after   2 hours   */

#define STRING_NPOS_INTAKE_GROUP_POS 0 /**< Last position of group */
#define STRING_NPOS_INTAKE_GROUP_LENGTH 1
#define STRING_NPOS_INTAKE_NUMBER_POS 6 /**< Last position of field */
#define STRING_NPOS_INTAKE_NUMBER_LENGTH 2
#define STRING_NPOS_INTAKE_SUFFIX_POS 7 /**< Last position of field */
#define STRING_NPOS_INTAKE_SUFFIX_LENGTH 2 /**< Last position of field */
#define STRING_NPOS_INTAKE_COUNT_POS 19 /**< Last position of field */
#define STRING_NPOS_INTAKE_COUNT_LENGTH 3
#define STRING_NPOS_INTAKE_UNIT_POS 21 /**< Start position of field */
#define STRING_NPOS_INTAKE_UNIT_LENGTH 7

#define STRING_SECONDS "seconds"
#define STRING_MINUTES "minutes"
#define STRING_HOURS "hours"
#define STRING_DAYS "days"

/** This string shall contain the status string as printed in the initial NDEF message. */
static char sStatusString[TEXT_STATUS_LENGTH];

static void Duration2String(int duration, char * countEnd, int maxCountLength, char * unitBegin, int maxUnitLength);
static void Number2String(int remaining, char * end, int size);

char* Text_StatusNotStarted(void)
{
    strcpy(sStatusString, (const char*) STRING_NOT_STARTED);
    return sStatusString;
}

char* Text_StatusStopped(uint32_t startTime, uint32_t lastIntakeTime)
{
    int stopDelta = (int)(lastIntakeTime - startTime);
    strcpy(sStatusString, (const char*) STRING_STOPPED);

    Number2String((int)startTime, sStatusString + STRING_STOPPED_START_TIME_POS, STRING_STOPPED_START_LENGTH);
    Duration2String(stopDelta, sStatusString + STRING_STOPPED_STOP_COUNT_POS, STRING_STOPPED_STOP_COUNT_LENGTH,
                    sStatusString + STRING_STOPPED_STOP_UNIT_POS, STRING_STOPPED_STOP_UNIT_LENGTH);
    return sStatusString;
}

char* Text_StatusOngoing(uint32_t startTime, uint32_t now, int pillsRemaining)
{
    int startDelta = (int)(now - startTime);
    strcpy(sStatusString, (const char*) STRING_STARTED);

    Duration2String(startDelta, sStatusString + STRING_STARTED_START_COUNT_POS, STRING_STARTED_START_COUNT_LENGTH,
                    sStatusString + STRING_STARTED_START_UNIT_POS, STRING_STARTED_START_UNIT_LENGTH);
    Number2String(pillsRemaining, sStatusString + STRING_STARTED_REMAINING_COUNT_POS,
    STRING_STARTED_REMAINING_COUNT_LENGTH);
    if (pillsRemaining == 1) {
        sStatusString[STRING_STARTED_PLURAL_POS] = ' ';
        sStatusString[STRING_STARTED_NOUN_POS] = 's';
        sStatusString[STRING_STARTED_NOUN_POS + 1] = '.';
    }
    else {
        sStatusString[STRING_STARTED_PLURAL_POS] = 's';
        sStatusString[STRING_STARTED_NOUN_POS] = '.';
        sStatusString[STRING_STARTED_NOUN_POS + 1] = ' ';
    }
    return sStatusString;
}

char* Text_IntakePositional(uint32_t startTime, uint32_t intakeTime, int group, int position)
{
    static char sIntake[TEXT_POS_INTAKE_LENGTH] = STRING_POS_INTAKE_POS;

    int delta = (int)(intakeTime - startTime);
    Duration2String(delta, sIntake + STRING_POS_INTAKE_COUNT_POS, STRING_POS_INTAKE_COUNT_LENGTH,
                    sIntake + STRING_POS_INTAKE_UNIT_POS, STRING_POS_INTAKE_UNIT_LENGTH);
    Number2String(group, sIntake + STRING_POS_INTAKE_GROUP_POS, STRING_POS_INTAKE_GROUP_LENGTH);
    Number2String(position, sIntake + STRING_POS_INTAKE_NUMBER_POS, STRING_POS_INTAKE_NUMBER_LENGTH);
    return sIntake;
}

char* Text_IntakeNonPositional(uint32_t startTime, uint32_t intakeTime, int group, int removal){
    static char sIntake[TEXT_NPOS_INTAKE_LENGTH] = STRING_NPOS_INTAKE_POS;
    int delta = (int)(intakeTime - startTime);

    Duration2String(delta, sIntake + STRING_NPOS_INTAKE_COUNT_POS, STRING_NPOS_INTAKE_COUNT_LENGTH,
                    sIntake + STRING_NPOS_INTAKE_UNIT_POS, STRING_NPOS_INTAKE_UNIT_LENGTH);
    Number2String(group, sIntake + STRING_NPOS_INTAKE_GROUP_POS, STRING_NPOS_INTAKE_GROUP_LENGTH);
    Number2String(removal, sIntake + STRING_NPOS_INTAKE_NUMBER_POS, STRING_NPOS_INTAKE_NUMBER_LENGTH);

    switch(removal) {
        case 1: memcpy (sIntake + STRING_NPOS_INTAKE_SUFFIX_POS, "st", 2); break;
        case 2: memcpy (sIntake + STRING_NPOS_INTAKE_SUFFIX_POS, "nd", 2); break;
        case 3: memcpy (sIntake + STRING_NPOS_INTAKE_SUFFIX_POS, "rd", 2); break;
        default: memcpy (sIntake + STRING_NPOS_INTAKE_SUFFIX_POS, "th", 2); break;
    }
    return sIntake;
}

/** Helper function to print a duration based on seconds, minutes, hours or days, whichever makes most sense for humans. */
static void Duration2String(int duration, char * countEnd, int maxCountLength, char * unitBegin, int maxUnitLength)
{
    static const char sSeconds[] = STRING_SECONDS;
    static const char sMinutes[] = STRING_MINUTES;
    static const char sHours[] = STRING_HOURS;
    static const char sDays[] = STRING_DAYS;
    const char * unit;
    int size;

    if (duration < 2 * 60) {
        unit = sSeconds;
        size = sizeof(sSeconds) - 1; /* exclude NUL byte */
    }
    else if (duration < 2 * 60 * 60) {
        duration /= 60;
        unit = sMinutes;
        size = sizeof(sMinutes) - 1; /* exclude NUL byte */
    }
    else if (duration < 2 * 60 * 60 * 24) {
        duration /= (60 * 60);
        unit = sHours;
        size = sizeof(sHours) - 1; /* exclude NUL byte */
    }
    else {
        duration /= (60 * 60 * 24);
        unit = sDays;
        size = sizeof(sDays) - 1; /* exclude NUL byte */
    }
    if (maxUnitLength < size) {
        size = maxUnitLength;
    }
    memset(unitBegin, ' ', (size_t)maxUnitLength);
    memcpy(unitBegin, unit, (size_t)size);

    Number2String(duration, countEnd, maxCountLength);
}

/** Helper function to print a number (decimal). */
static void Number2String(int remaining, char * end, int size)
{
    memset(end - size + 1, ' ', (size_t)size);
    *end = '0';
    while ((size > 0) && (remaining > 0)) {
        *end = (char)('0' + (remaining % 10));
        end--;
        remaining /= 10;
        size--;
    }
}
