/*
 * Copyright 2016-2018 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include <memorymanager.h>
#include <string.h>
#include <text.h>

#define DATA_OFFSET 0

/** Slot: THERAPY_INSTANCE @{ */
#define SLOT_THERAPYINSTANCE_OFFSET DATA_OFFSET
#define SLOT_THERAPYINSTANCE_END (SLOT_THERAPYINSTANCE_OFFSET + THERAPYINSTANCE_SLOTSIZE)
/** @} */

/** Slot: THERAPY_RHYTHM @{ */
#define SLOT_RHYTHM_OFFSET SLOT_THERAPYINSTANCE_END
#define SLOT_RHYTHM_END (SLOT_RHYTHM_OFFSET + RHYTHM_SLOTSIZE)
/** @} */

/** Slot: SENSE_INSTANCE @{ */
#define SLOT_SENSEINSTANCE_OFFSET SLOT_RHYTHM_END
#define SLOT_SENSEINSTANCE_END (SLOT_SENSEINSTANCE_OFFSET + (SENSEINSTANCE_SLOTSIZE))
/** @} */

/** Memory where the events will be stored. @{ */
#define EVENT_OFFSET SLOT_SENSEINSTANCE_END
#define EVENT_SIZE (MEMORY_EVENTSIZE * MEMORY_MAXEVENTS)
#define EVENT_END (EVENT_OFFSET + EVENT_SIZE)
/** @} */

/** Memory where the intake strings will be stored. @{ */
#define INTAKESTRINGS_OFFSET EVENT_END
#define SINGLE_INTAKESTRING_SIZE 31
#define INTAKEMAXSTRINGS 35
#define INTAKETEXT_SIZE (SINGLE_INTAKESTRING_SIZE * INTAKEMAXSTRINGS)
#define INTAKETEXT_END (INTAKESTRINGS_OFFSET + INTAKETEXT_SIZE)
/** @} */

/** End of the data preserved by memory manager */
#define DATA_END (INTAKETEXT_END)

static void SlotToOffset(MEMORY_SLOT_ID_T slot, int* pOffset, int* pSize);

void Memory_Init(void)
{
    // void
}

void Memory_DeInit(void)
{
    Chip_EEPROM_Flush(NSS_EEPROM, true);
}

void Memory_StoreIntakeString(int intakeNr, uint32_t startTime, uint32_t intakeTime, int group, bool positional,
                              int initialCount, int pill)
{
    char* intake;
    int offset;

    ASSERT(intakeNr< INTAKEMAXSTRINGS);
    if(positional){
        intake = Text_IntakePositional(startTime, intakeTime, group, pill);
    }
    else{
        intake = Text_IntakeNonPositional(startTime, intakeTime, group, initialCount - pill + 1);
    }
    offset = (int) INTAKESTRINGS_OFFSET + (intakeNr * SINGLE_INTAKESTRING_SIZE);
    Chip_EEPROM_Write(NSS_EEPROM, offset, intake, (int)strlen(intake) + 1); /* +1 to include \0 */
    Chip_EEPROM_Flush(NSS_EEPROM, true);
}

const char* Memory_GetIntakeString(int intakeNr)
{
    ASSERT(intakeNr< INTAKEMAXSTRINGS);
    return (char*)(EEPROM_START + INTAKESTRINGS_OFFSET + (uint32_t)(intakeNr * SINGLE_INTAKESTRING_SIZE));
}

void Memory_StoreData(MEMORY_SLOT_ID_T slot, const void* src, int size)
{
    int slotOffset, slotSize;
    SlotToOffset(slot, &slotOffset, &slotSize);

    ASSERT(size <= slotSize);

    /* Check if update is needed, if no update needed, we do not need to stress EEPROM by re-writing it. */
    if (0 != memcmp((void*)(EEPROM_START + slotOffset), src, (size_t)size)) {
        Chip_EEPROM_Write(NSS_EEPROM, slotOffset, (void*)src, size);
        Chip_EEPROM_Flush(NSS_EEPROM, true);
    }
}
void Memory_GetStoredData(MEMORY_SLOT_ID_T slot, void* dst, int size)
{
    int slotOffset, slotSize;
    SlotToOffset(slot, &slotOffset, &slotSize);
    ASSERT(size <= slotSize);

    Chip_EEPROM_Read(NSS_EEPROM, slotOffset, dst, size);
}

void Memory_StoreEvent(int event, uint8_t *pEvent)
{
    ASSERT(event < MEMORY_MAXEVENTS);
    Chip_EEPROM_Write(NSS_EEPROM, EVENT_OFFSET + (event * MEMORY_EVENTSIZE), pEvent, MEMORY_EVENTSIZE);
    Chip_EEPROM_Flush(NSS_EEPROM, true);
}

const uint8_t* Memory_GetEvents(void)
{
    return (const uint8_t*)(EEPROM_START + EVENT_OFFSET);
}

/** Helper function to translate a slot id to its offset and its size */
static void SlotToOffset(MEMORY_SLOT_ID_T slot, int* pOffset, int* pSize)
{
    switch (slot) {
        case MEMORY_SLOT_ID_RHYTHM:
            *pOffset = SLOT_RHYTHM_OFFSET;
            *pSize = RHYTHM_SLOTSIZE;
            break;
        case MEMORY_SLOT_ID_SENSE_INSTANCE:
            *pOffset = SLOT_SENSEINSTANCE_OFFSET;
            *pSize = SENSEINSTANCE_SLOTSIZE;
            break;
        case MEMORY_SLOT_ID_THERAPY_INSTANCE:
            *pOffset = SLOT_THERAPYINSTANCE_OFFSET;
            *pSize = THERAPYINSTANCE_SLOTSIZE;
            break;
        default:
            *pOffset = DATA_END;
            *pSize = 0;
            break;
    }
}
