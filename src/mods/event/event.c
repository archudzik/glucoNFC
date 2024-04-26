/*
 * Copyright 2017-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include "event.h"

/**
 * @file
 *
 *  @par Storage model
 *      Events are stored at byte boundaries. For each event, first an instance of #EVENT_T is stored, followed by a
 *      number of bytes of data, indicated by #EVENT_T.len.
 *      - The last two bytes of the assigned EEPROM region are used to indicate the first free byte, where the next
 *          event must be stored: a value of @c 0x0000 indicates no events are stored. See also #sMeta.nextFreeOffset.
 *      - The 4 bytes before that are used to store the full timestamp when #Event_Init was first called.
 *          All other timestamps are delta values. See also #sBaseTimestamp.
 *
 *  @par Searching events
 *      Events can only be searched by traversing them in sequence, from the oldest event to the newest event.
 *      The size of one event equals the size of the structure #EVENT_T plus #EVENT_T.len. Searching stops when the
 *      iterator reaches the value of #sMeta.nextFreeOffset.
 */

/**
 * The absolute offset to the very first byte of the assigned EEPROM region.
 * @note Unless explicitly specified, all bit and byte offsets referring to the EEPROM region in the code are relative
 *  to this value.
 */
#define EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET (EVENT_EEPROM_FIRST_ROW * EEPROM_ROW_SIZE)

/** The absolute offset to the very last byte of the assigned EEPROM region. */
#define EEPROM_ABSOLUTE_LAST_BYTE_OFFSET (((EVENT_EEPROM_LAST_ROW + 1) * EEPROM_ROW_SIZE) - 1)

/**
 * The absolute offset to where the meta information must be stored. The location is read-only in #Event_Init and
 * #Event_DeInit, possibly written once in #Event_DeInit - iff the contents have changed.
 */
#define EEPROM_META_OFFSET (EEPROM_ABSOLUTE_LAST_BYTE_OFFSET + 1 - sizeof(META_T))

#if (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_C) | (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_D)
    #define REDUCE_TIMESTAMP(full) ((full) / 64U)
    #define RESTORE_TIMESTAMP(reduced) ((reduced) * 64U)
#elif (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_E)
    #define REDUCE_TIMESTAMP(full) ((full) * 9U / 32768U)
    #define RESTORE_TIMESTAMP(reduced) ((reduced) * 32768U / 9U)
#else /* EVENT_OVERHEAD_CHOICE_A | EVENT_OVERHEAD_CHOICE_B */
    #define REDUCE_TIMESTAMP(full) (full)
    #define RESTORE_TIMESTAMP(reduced) (reduced)
#endif

#if (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_B) | (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_C)
    #define DELTATIMESTAMP_BITMASK 0x00FFFFFF
#elif (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_D) | (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_E)
    #define DELTATIMESTAMP_BITMASK 0x0000FFFF
#else /* EVENT_OVERHEAD_CHOICE_A */
    #define DELTATIMESTAMP_BITMASK 0xFFFFFFFF
#endif

/* ------------------------------------------------------------------------- */

#pragma pack(push, 1)

/** Data stored at the end of the assigned EEPROM memory. Required for proper working of the event module. */
typedef struct META_S {
    uint32_t baseTimestamp; /**< Time in seconds. The full timestamp when #Event_Init was first called. */
    uint32_t lastTimestamp; /**< Time in seconds. The full timestamp when #Event_Set was last called. */
    uint16_t lastTimeError; /**< Delta time in seconds. The error made when storing the delta time in #EVENT_T.timestamp. */
    uint16_t nextFreeOffset; /**< The absolute offset to where the next event must be stored. */
} META_T;

/** Data stored for each event. */
typedef struct EVENT_S { /* ANY change must be reflected in EVENT_OVERHEAD. */
#if (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_B) | (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_C)
    uint32_t tag : 8;
    /**
     * Delta time with previous event.
     * - B: unit: second
     *  Maximum distance: (2**24) / 60 seconds / 60 minutes / 24 hours = 194.18 days
     * - C: unit: ~minute: 64 seconds
     *  Maximum distance: (2**24) * (64/60) / 60 minutes / 24 hours / 365.25 days = 34.03 years
     */
    uint32_t deltaTimestamp : 24;
    uint8_t len; /**< The length, in bytes, of the extra data written after this structure. */
#elif (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_D) | (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_E)
    uint32_t tag : 8;
    uint32_t len : 8;
    /**
     * Delta time with previous event.
     * - D: unit: ~minute: 64 seconds
     *  Maximum distance: (2**16) * (64/60) / 60 minutes / 24 hours = 48.55 days
     * - E: unit: ~hour: 3641 seconds
     *  Maximum distance: (2**16) * (32768/9/3600) / 24 hours / 365.25 days = 7.56 years
     */
    uint32_t deltaTimestamp : 16;
#else /* EVENT_OVERHEAD_CHOICE_A */
    uint32_t deltaTimestamp; /**< Time in seconds. Delta time with previous event. */
    uint8_t tag;
    uint8_t len;
#endif
} EVENT_T;

#pragma pack(pop)

/**
 * Dummy variable to test the value of #MEMORY_FIRSTUNUSEDEEPROMOFFSET.
 * If the macro is not correct, the dummy variable will have a negative array size and the compiler will raise an error
 * similar to:
 *   ../src/event.c:71:13: error: size of array 'sTestEventoverhead' is negative
 */
static char sTestEventoverhead[(EVENT_OVERHEAD == sizeof(EVENT_T)) - 1] __attribute__((unused));

/* ------------------------------------------------------------------------- */

/** A copy of the information stored at #EEPROM_META_OFFSET. Updated in EEPROM in #Event_DeInit. */
static META_T sMeta;

extern bool EVENT_CB(uint8_t tag, int offset, uint8_t len, unsigned int index, uint32_t timestamp, uint32_t context);
static pEvent_Cb_t sEventCb = EVENT_CB;

static bool GetFirstOrLastByTag(bool first, uint8_t tag, int * pOffset, uint8_t * pLen, unsigned int * pIndex,
                                uint32_t * pTimestamp);

#if EVENT_CB_SELF_DEFINED == 1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * This is a dummy implementation to provoke fallback behavior
 * It is only used when #EVENT_CB is not overridden in an application specific @c app_sel.h header file
 * @see pEvent_Cb_t
 * @param tag unused
 * @param offset unused
 * @param len unused
 * @param index unused
 * @param timestamp unused
 * @param context unused
 * @return @c true
 */
bool DummyEventCb(uint8_t tag, int offset, uint8_t len, unsigned int index, uint32_t timestamp, uint32_t context)
{
    return true;
}
#pragma GCC diagnostic pop
#endif

static bool GetFirstOrLastByTag(bool first, uint8_t tag, int * pOffset, uint8_t * pLen, unsigned int * pIndex,
                                uint32_t * pTimestamp)
{
    bool found = false;
    int eepromOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET;
    uint32_t timestamp = sMeta.baseTimestamp;
    unsigned int index = 0;
    EVENT_T event;

    while (eepromOffset < sMeta.nextFreeOffset) {
        Chip_EEPROM_Read(NSS_EEPROM, eepromOffset, &event, EVENT_OVERHEAD);
        timestamp += RESTORE_TIMESTAMP(event.deltaTimestamp);
        if (tag == event.tag) {
            found = true;
            if (pOffset) {
                *pOffset = event.len ? (eepromOffset + EVENT_OVERHEAD) : -1;
            }
            if (pLen) {
                *pLen = event.len;
            }
            if (pIndex) {
                *pIndex = index;
            }
            if (pTimestamp) {
                *pTimestamp = timestamp;
            }
            if (first) {
                break;
            }
        }
        eepromOffset += EVENT_OVERHEAD + event.len;
        index++;
    }
    return found;
}

/* ------------------------------------------------------------------------- */

void Event_Init(bool reset)
{
    if (reset) {
        Chip_EEPROM_Memset(NSS_EEPROM, EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET, 0, EVENT_EEPROM_SIZE);
    }
    Chip_EEPROM_Read(NSS_EEPROM, EEPROM_META_OFFSET, &sMeta, sizeof(META_T));
    /* After-reset initialization. */
    if (sMeta.baseTimestamp == 0) {
        sMeta.baseTimestamp = (uint32_t)Chip_RTC_Time_GetValue(NSS_RTC);
        sMeta.lastTimestamp = sMeta.baseTimestamp;
        sMeta.lastTimeError = 0;
        sMeta.nextFreeOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET;
    }
    /* Sanity check. */
    if ((sMeta.nextFreeOffset < EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET) || (sMeta.nextFreeOffset >= EEPROM_META_OFFSET)) {
        sMeta.nextFreeOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET;
    }
}

void Event_DeInit(void)
{
    META_T storedMeta;
    Chip_EEPROM_Read(NSS_EEPROM, EEPROM_META_OFFSET, &storedMeta, sizeof(META_T));
    if ((sMeta.baseTimestamp != storedMeta.baseTimestamp)
            || (sMeta.lastTimestamp != storedMeta.lastTimestamp)
            || (sMeta.nextFreeOffset != storedMeta.nextFreeOffset)) {
        Chip_EEPROM_Write(NSS_EEPROM, EEPROM_META_OFFSET, &sMeta, sizeof(META_T));
    }
    Chip_EEPROM_Flush(NSS_EEPROM, true);
}

pEvent_Cb_t Event_SetCb(pEvent_Cb_t cb)
{
    pEvent_Cb_t previousCb = sEventCb;
    if (cb) {
        sEventCb = cb;
    }
    else {
        extern bool EVENT_CB(uint8_t tag, int offset, uint8_t len, unsigned int index, uint32_t timestamp, uint32_t context);
        sEventCb = EVENT_CB;
    }
    return previousCb;
}

bool Event_Set(uint8_t tag, void * data, uint8_t len)
{
    /* When storing a new event, the delta time is stored using less bytes - based on EVENT_OVERHEAD_CHOICE.
     * This results in an error which, if not checked for, accumulates when retrieving tags.
     * The error made in seconds is therefore also stored and added as extra delta value before reducing the timestamp
     * in sMeta.lastTimeError
     */
    if (data == NULL) {
        len = 0;
    }
    bool success = (sMeta.nextFreeOffset + EVENT_OVERHEAD + len < (int)EEPROM_META_OFFSET);
    if (success) {
        uint32_t now = (uint32_t)Chip_RTC_Time_GetValue(NSS_RTC);
        EVENT_T event = {.tag = tag, .len = len};
        event.deltaTimestamp = REDUCE_TIMESTAMP(now - sMeta.lastTimestamp + sMeta.lastTimeError)
                & DELTATIMESTAMP_BITMASK;
        sMeta.lastTimeError = (uint16_t)(now - sMeta.lastTimestamp + sMeta.lastTimeError
                - RESTORE_TIMESTAMP(event.deltaTimestamp));
        Chip_EEPROM_Write(NSS_EEPROM, sMeta.nextFreeOffset, &event, EVENT_OVERHEAD);
        if (len) {
            Chip_EEPROM_Write(NSS_EEPROM, sMeta.nextFreeOffset + EVENT_OVERHEAD, data, len);
        }
        sMeta.nextFreeOffset = (uint16_t)(sMeta.nextFreeOffset + EVENT_OVERHEAD + len);
        sMeta.lastTimestamp = now;
    }
    return success;
}

unsigned int Event_GetByIndex(unsigned int first, unsigned int last, uint32_t context)
{
    bool searching = true;
#if EVENT_CB_OPENING_CALL
    searching = sEventCb(0, -1, 0, EVENT_CB_OPENING_INDEX, 0, context);
#endif
    int eepromOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET;
    uint32_t timestamp = sMeta.baseTimestamp;
    unsigned int index = 0;
    unsigned int count = 0;
    EVENT_T event;

    while (searching && (eepromOffset < sMeta.nextFreeOffset)) {
        Chip_EEPROM_Read(NSS_EEPROM, eepromOffset, &event, EVENT_OVERHEAD);
        timestamp += RESTORE_TIMESTAMP(event.deltaTimestamp);
        if ((first <= index) && (index <= last)) {
        searching = sEventCb(event.tag,
                             event.len ? eepromOffset + EVENT_OVERHEAD : -1,
                             event.len,
                             index,
                             timestamp,
                             context);
            count++;
        }
        eepromOffset += EVENT_OVERHEAD + event.len;
        index++;
    }
#if EVENT_CB_CLOSING_CALL
    (void)sEventCb(0, -1, 0, EVENT_CB_CLOSING_INDEX, 0, context);
#endif
    return count;
}

unsigned int Event_GetByTime(unsigned int begin, unsigned int end, uint32_t context)
{
    bool searching = true;
#if EVENT_CB_OPENING_CALL
    searching = sEventCb(0, -1, 0, EVENT_CB_OPENING_INDEX, 0, context);
#endif
    int eepromOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET;
    uint32_t timestamp = sMeta.baseTimestamp;
    unsigned int index = 0;
    unsigned int count = 0;
    EVENT_T event;

    begin = REDUCE_TIMESTAMP(begin);
    end = REDUCE_TIMESTAMP(end);
    while (searching && (eepromOffset < sMeta.nextFreeOffset)) {
        Chip_EEPROM_Read(NSS_EEPROM, eepromOffset, &event, EVENT_OVERHEAD);
        timestamp += RESTORE_TIMESTAMP(event.deltaTimestamp);
        if ((begin <= event.deltaTimestamp) && (event.deltaTimestamp <= end)) {
            searching = sEventCb(event.tag, eepromOffset, event.len, index, timestamp, context);
            count++;
        }
        eepromOffset += EVENT_OVERHEAD + event.len;
        index++;
    }
#if EVENT_CB_CLOSING_CALL
    (void)sEventCb(0, -1, 0, EVENT_CB_CLOSING_INDEX, 0, context);
#endif
    return count;
}

unsigned int Event_GetByTag(uint8_t tag, uint32_t context)
{
    bool searching = true;
#if EVENT_CB_OPENING_CALL
    searching = sEventCb(0, -1, 0, EVENT_CB_OPENING_INDEX, 0, context);
#endif
    int eepromOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET;
    uint32_t timestamp = sMeta.baseTimestamp;
    unsigned int index = 0;
    unsigned int count = 0;
    EVENT_T event;

    while (searching && (eepromOffset < sMeta.nextFreeOffset)) {
        Chip_EEPROM_Read(NSS_EEPROM, eepromOffset, &event, EVENT_OVERHEAD);
        timestamp += RESTORE_TIMESTAMP(event.deltaTimestamp);
        if (tag == event.tag) {
            int offset = event.len ? (eepromOffset + EVENT_OVERHEAD) : -1;
            searching = sEventCb(event.tag, offset, event.len, index, timestamp, context);
            count++;
        }
        eepromOffset += EVENT_OVERHEAD + event.len;
        index++;
    }
#if EVENT_CB_CLOSING_CALL
    (void)sEventCb(0, -1, 0, EVENT_CB_CLOSING_INDEX, 0, context);
#endif
    return count;
}

bool Event_GetFirstByTag(uint8_t tag, int * pOffset, uint8_t * pLen, unsigned int * pIndex, uint32_t * pTimestamp)
{
    return GetFirstOrLastByTag(true, tag, pOffset, pLen, pIndex, pTimestamp);
}

bool Event_GetLastByTag(uint8_t tag, int * pOffset, uint8_t * pLen, unsigned int * pIndex, uint32_t * pTimestamp)
{
    return GetFirstOrLastByTag(false, tag, pOffset, pLen, pIndex, pTimestamp);
}
