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

#ifndef __EVENT_DFT_H_
#define __EVENT_DFT_H_

/** @defgroup MODS_NSS_EVENT_DFT Diversity Settings
 *  @ingroup MODS_NSS_EVENT
 *
 * The application can adapt the Event module to better fit the different application scenarios through the use of
 * diversity flags in the form of defines below. Sensible defaults are chosen; to override the default settings, place
 * the defines with their desired values in the application app_sel.h header file: the compiler will pick up your
 * defines before parsing this file.
 *
 * Additional notes regarding some flags:
 * - By default, the assigned EEPROM region takes up 1KB and is located just below the RO EEPROM rows. This storage
 *  space can be moved and resized by adapting #EVENT_EEPROM_FIRST_ROW and #EVENT_EEPROM_LAST_ROW.
 *
 * These flags may be overridden/set:
 * - #EVENT_EEPROM_FIRST_ROW
 * - #EVENT_EEPROM_LAST_ROW
 * - #EVENT_CB
 * - #EVENT_CB_OPENING_CALL
 * - #EVENT_CB_CLOSING_CALL
 * - #EVENT_OVERHEAD_CHOICE
 *
 * These defines are fixed or derived from the above flags and may not be defined or redefined in an application:
 * - #EVENT_EEPROM_ROW_COUNT
 * - #EVENT_EEPROM_SIZE
 * - #EVENT_OVERHEAD_CHOICE_A
 * - #EVENT_OVERHEAD_CHOICE_B
 * - #EVENT_OVERHEAD_CHOICE_C
 * - #EVENT_OVERHEAD_CHOICE_D
 * - #EVENT_OVERHEAD_CHOICE_E
 * - #EVENT_OVERHEAD
 *
 * @{
 */
#include "chip.h"

#ifndef EVENT_EEPROM_FIRST_ROW
    /**
     * The first EEPROM row assigned for storing events. Starting from the first byte in this row, until the last byte
     * in #EVENT_EEPROM_LAST_ROW, the event bookkeeping module has full control: no other code may touch this EEPROM
     * region.
     * @note By default, the EEPROM row one kB above the start of the EEPROM will be chosen.
     */
    #define EVENT_EEPROM_FIRST_ROW (1024 / EEPROM_ROW_SIZE)
#endif
#if !(EVENT_EEPROM_FIRST_ROW >= 0) || !(EVENT_EEPROM_FIRST_ROW < EEPROM_NR_OF_RW_ROWS)
    #error Invalid value for EVENT_EEPROM_FIRST_ROW
#endif

#ifndef EVENT_EEPROM_LAST_ROW
    /**
     * The last EEPROM row assigned for storing events. Starting from the first byte in #EVENT_EEPROM_FIRST_ROW,
     * until the last byte in this row, the event bookkeeping module has full control: no other code may touch this
     * EEPROM region.
     * @note By default, the total assigned size will occupy 1 kB.
     */
    #define EVENT_EEPROM_LAST_ROW (EVENT_EEPROM_FIRST_ROW + (1024 / EEPROM_ROW_SIZE) - 1)
#endif
#if !(EVENT_EEPROM_LAST_ROW >= EVENT_EEPROM_FIRST_ROW) || !(EVENT_EEPROM_LAST_ROW < EEPROM_NR_OF_RW_ROWS)
    #error Invalid value for EVENT_EEPROM_LAST_ROW
#endif

/** The number of EEPROM rows assigned for event bookkeeping. */
#define EVENT_EEPROM_ROW_COUNT (EVENT_EEPROM_LAST_ROW - EVENT_EEPROM_FIRST_ROW + 1)

/** The size in bytes of the EEPROM region assigned for event bookkeeping. */
#define EVENT_EEPROM_SIZE (EVENT_EEPROM_ROW_COUNT * EEPROM_ROW_SIZE)

/* ------------------------------------------------------------------------- */

#ifdef EVENT_CB
    #undef EVENT_CB_SELF_DEFINED
#else
    /**
     * Used internally to know when to use a dummy callback for this.
     * @internal
     */
    #define EVENT_CB_SELF_DEFINED 1
    /**
     * The name of the function - @b not a function pointer - of type #pEvent_Cb_t that is able to receive the events
     * requested by calling #Event_GetByIndex, #Event_GetByTime or #Event_GetByTag.
     * @note @b Not defining this callback results in a fine FINO buffer: First-In, Never-Out.
     * @note The callback can also be overridden dynamically using #Event_SetCb
     */
    #define EVENT_CB DummyEventCb
#endif

/**
 * Used as special index value when making an opening call.
 * @see EVENT_CB_OPENING_CALL
 * @see pEvent_Cb_t
 */
#define EVENT_CB_OPENING_INDEX 0xFFFFFFFE

#ifndef EVENT_CB_OPENING_CALL
    /**
     * By default, the callback function defined by #EVENT_CB or as set by #Event_SetCb is called once per found event.
     * Define this to have the event bookkeeping module call the callback function one extra time, before the first
     * event is searched for and reported. This opening call then acts as a start-of-search signal to allow the
     * application to prepare his activities.
     * @note An opening call will have
     *  the values @c 0, @c -1, @c 0, #EVENT_CB_OPENING_INDEX, @c 0 for
     *  the arguments @c tag, @c offset, @c len, @c index, @c timestamp, respectively.
     * @note The returnvalue for in the opening call is checked for: returning false aborts the retrieval.
     * @see pEvent_Cb_t
     */
    #define EVENT_CB_OPENING_CALL 0
#endif

/**
 * Used as special index value when making an opening call.
 * @see EVENT_CB_CLOSING_CALL
 * @see pEvent_Cb_t
 */
#define EVENT_CB_CLOSING_INDEX 0xFFFFFFFF

#ifndef EVENT_CB_CLOSING_CALL
    /**
     * By default, the callback function defined by #EVENT_CB or as set by #Event_SetCb is called once per found event.
     * Define this to have the event bookkeeping module call the callback function one extra time, after all events have
     * been searched for and reported. This last call then acts as an end-of-search signal to allow the application to
     * conclude his activities.
     * @note An closing call will have
     *  the values @c 0, @c -1, @c 0, #EVENT_CB_CLOSING_INDEX, @c 0 for
     *  the arguments @c tag, @c offset, @c len, @c index, @c timestamp, respectively.
     * @note The returnvalue for in the closing call is ignored.
     * @see pEvent_Cb_t
     */
    #define EVENT_CB_CLOSING_CALL 0
#endif

/**
 * When #EVENT_OVERHEAD_CHOICE is assigned this value, the overhead - defined in #EVENT_OVERHEAD - is @c 6
 * This is the default value when #EVENT_OVERHEAD_CHOICE is not defined in the application's @c app_sel.h file.
 * - All timestamps are stored with full resolution (seconds).
 * - No restrictions on the use case are in effect.
 */
#define EVENT_OVERHEAD_CHOICE_A 0x41

/**
 * When #EVENT_OVERHEAD_CHOICE is assigned this value, the overhead - defined in #EVENT_OVERHEAD - is @c 5
 * The overhead - defined in #EVENT_OVERHEAD - is @c 5
 * - All timestamps are stored with full resolution (second).
 * - Events are spaced maximally 194 days apart.
 */
#define EVENT_OVERHEAD_CHOICE_B 0x42

/**
 * When #EVENT_OVERHEAD_CHOICE is assigned this value, the overhead - defined in #EVENT_OVERHEAD - is @c 5
 * The overhead - defined in #EVENT_OVERHEAD - is @c 5
 * - All timestamps are stored with reduced resolution (~minute).
 * - No restrictions on the use case are in effect.
 */
#define EVENT_OVERHEAD_CHOICE_C 0x43

/**
 * When #EVENT_OVERHEAD_CHOICE is assigned this value, the overhead - defined in #EVENT_OVERHEAD - is @c 4
 * The overhead - defined in #EVENT_OVERHEAD - is @c 4
 * - All timestamps are stored with reduced resolution (~minute).
 * - Events are spaced maximally 48 days apart.
 */
#define EVENT_OVERHEAD_CHOICE_D 0x44

/**
 * When #EVENT_OVERHEAD_CHOICE is assigned this value, the overhead - defined in #EVENT_OVERHEAD - is @c 4
 * The overhead - defined in #EVENT_OVERHEAD - is @c 4
 * - All timestamps are stored with low resolution (~hour).
 * - No restrictions on the use case are in effect.
 */
#define EVENT_OVERHEAD_CHOICE_E 0x45

#ifndef EVENT_OVERHEAD_CHOICE
    /**
     * For each event, a minimum number of bytes is required to correctly store the data. Depending on the needs of the
     * application, this size can be reduced by assigning this define to one of the allowed choices:
     * - #EVENT_OVERHEAD_CHOICE_A
     * - #EVENT_OVERHEAD_CHOICE_B
     * - #EVENT_OVERHEAD_CHOICE_C
     * - #EVENT_OVERHEAD_CHOICE_D
     * - #EVENT_OVERHEAD_CHOICE_E
     * @note The order in which the events are stored - reflected in the @c index passed to the caller upon retrieval -
     *  will always correctly reflect the timing order in which they occurred, regardless of the resolution of the
     *  timestamp.
     * @warning Regardless of the resolution of the storage of the timestamp, upon retrieval it will be converted back
     *  to an epoch time in seconds. In case of a resolution of an hour, the exact time when the event occurred can be
     *  exactly at the reported value, or up to 1 hour later.
     */
    #define EVENT_OVERHEAD_CHOICE EVENT_OVERHEAD_CHOICE_A
#endif

#if (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_B) || (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_C)
    #define EVENT_OVERHEAD 5
#elif (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_D) || (EVENT_OVERHEAD_CHOICE == EVENT_OVERHEAD_CHOICE_E)
    #define EVENT_OVERHEAD 4
#else /* EVENT_OVERHEAD_CHOICE_A */
    /**
     * The number of bytes that are required to be stored in excess of the extra optional data provided, per event.
     * This value includes the space for the tag value and the timestamp. Use this value to estimate the number of
	 * events that can be stored.
     * @note This value can be reduced by defining #EVENT_OVERHEAD_CHOICE differently.
     */
    #define EVENT_OVERHEAD 6
#endif

#endif /** @} */
