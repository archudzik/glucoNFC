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

#ifndef MEMORYMENAGER_H_
#define MEMORYMENAGER_H_

/** @defgroup APP_DEMO_TADHERENCE_MEMORY Memory Manager
 *  @ingroup APP_DEMO_TADHERENCE
 *  The Therapy Adherence Demo Memory Manager is responsible for the EEPROM memory management.
 *  This component takes ownership of the chip's EEPROM and provides an API to store/read data from non-volatile memory.
 *
 * @par Memory Requirements:
 *  The Memory component assumes that it has the exclusive use of the EEPROM memory (read/write rows).
 *
 * @par Usage
 *  Most of the other components will need to preserve internal status data, for the Memory Manager
 *  this status data is simply treated as raw data. For this it divides it's memory space (EEPROM)
 *  in slots (see #MEMORY_SLOT_ID_T), it is up to the requesting component to keep track of which data is stored in which
 *  slot and on it's size. The respective read/write API is: #Memory_GetStoredData and #Memory_StoreData.
 *
 *  To store and read intake strings (used in the initial NDEF message.) there is specialized API:
 *   #Memory_StoreIntakeString/#Memory_GetIntakeString.
 *
 *  @{
 */

/** The size of an event stored by the memory module. */

#define MEMORY_EVENTSIZE 9
/** The maximum number of events stored by the memory module. */
#define MEMORY_MAXEVENTS 35

/** Memory slot size for respective slot @{ */
#define THERAPYINSTANCE_SLOTSIZE 26
#define RHYTHM_SLOTSIZE 80
#define SENSEINSTANCE_SLOTSIZE 40
/** @} */

/** List of available memory slots. */
typedef enum MEMORY_SLOT_ID {
    MEMORY_SLOT_ID_THERAPY_INSTANCE, /**< Memory slot to store the instance for therapy module. */
    MEMORY_SLOT_ID_SENSE_INSTANCE, /**< Memory slot to store the instance for sense module. */
    MEMORY_SLOT_ID_RHYTHM, /**< Memory slot to store the configured rhythm. */
} MEMORY_SLOT_ID_T;

/** Init function, it is important to execute this function before any other call to this component. */
void Memory_Init(void);

/**
 * DeInit function, it is important to execute this function before going to state where RAM is not
 * preserved (Deep power down / Power Off).
 */
void Memory_DeInit(void);

/**
 * Function to inform the Memory component a new therapy was started.
 */
void Memory_StartTherapy(void);

/**
 * Function to store raw data
 * @param slot : The memory slot where to store the raw data.
 * @param pSrc : A pointer to the data to be stored.
 * @param size : Amount of bytes to be stored.
 */
void Memory_StoreData(MEMORY_SLOT_ID_T slot, const void* pSrc, int size);

/**
 * Function to read raw data (previously stored by #Memory_StoreData)
 * @param slot : The memory slot where to read the raw data.
 * @param pDst : A pointer to the memory location where to put the data.
 * @param size : Amount of bytes to be read (checked to the slotSize).
 */
void Memory_GetStoredData(MEMORY_SLOT_ID_T slot, void* pDst, int size);

/**
 * Function to store a new pill intake as a string (used in the initial NDEF message).
 * The provided data is used to generate a string representing this event.
 * @param intakeNr : The number of new intake string.
 * @param startTime : The time at which the therapy was started.
 * @param intakeTime : The time at which the pill removal was sensed.
 * @param group : The group of the removed pill.
 * @param positional : Defines whether @c group is positional.
 * @param initialCount : The initial pill count for @c group.
 * @param pill : The number of the removed pill (in the group).
 */
void Memory_StoreIntakeString(int intakeNr, uint32_t startTime, uint32_t intakeTime, int group, bool positional,
                              int initialCount, int pill);

/**
 * Function to get a previously stored 'intake' string.
 * @param intakeNr : The number of requested 'intake' string.
 * @return A pointer to memory where the string can be read.
 */
const char* Memory_GetIntakeString(int intakeNr);

/**
 * Function to store a new event.
 * All events are stored consecutively such that they can be read at once.
 *
 * @param eventNr : The number of the event. Needed to calculate the memory offset to store the event.
 * @param pEvent : A pointer to the event data to be stored.
 * @note Memory module expects an event to be #MEMORY_EVENTSIZE bytes in size.
 */
void Memory_StoreEvent(int eventNr, uint8_t *pEvent);

/**
 * Function to get a memory pointer where to read recorded events.
 * It is up to higher levels to manage the amount of stored events.
 * All events are stored consecutively such that they can be read at once.
 * @return A pointer from where in memory the first event can be read.
 * @note Memory module expects an event to be #MEMORY_EVENTSIZE bytes in size.
 *  Memory module can store up to  #MEMORY_EVENTSIZE events.
 */
const uint8_t* Memory_GetEvents(void);

/** @} */
#endif
