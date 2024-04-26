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

#ifndef __EVENT_H_
#define __EVENT_H_

/**
 * @defgroup MODS_NSS_EVENT event: event bookkeeping module
 * @ingroup MODS_NSS
 * This block of code allows an application to store disjoint events. Events can be retrieved using a few basic
 * filtering techniques. @n
 *
 * Events can:
 * - are stored with a timestamp
 * - can be retrieved in various ways
 * - not be deleted
 * - be changed, in a limited way (only the extra optional data section, if provided for).
 *
 * It will use the designated portion of the EEPROM to:
 * - store data of arbitrary size together with a timestamp and an unrestricted tag value.
 * - allow retrieval of data sequentially, based on a time range, or a specific tag value.
 *
 * The event bookkeeping module main purpose is to store sparse but noteworthy events. Only EEPROM is used, and only a
 * limited portion thereof. Events also take considerable space; once the assigned memory is full, no new events can be
 * stored. Typically, only a handful events of each 'type' - each is then given a different tag value is expected to
 * be stored.
 *
 * Examples are:
 * - Storing configuration, start and stop times.
 * - Storing error conditions such as memory full or BOD.
 * - Storing read-out moments
 * - Storing use case specific data, such as location-aware data given by an NFC reader a few times during the active
 *  lifetime of the IC.
 * - Storing the time of the first excursions, and/or the time when the validation algorithm decided the product is no
 *  longer fit for consumption.
 * - Storing any application-specific data, that is not required for the temperature logging and monitoring
 *  functionality, but is necessary to better integrate within the full solution.
 * When reporting events, a lot of extra information is provided, allowing an application to easily fetch similar
 * events, or neighboring events.
 *
 * It is expected that each application that requires this module includes it and configures the diversity settings of
 * the module according to its specific needs.
 *
 * @par Diversity
 *  This module supports diversity: settings to define the type and size of the EEPROM region placed under control of
 *  this module. Check @ref MODS_NSS_EVENT_DFT for all diversity parameters.
 *
 * @par How to use the module
 *  -# First call #Event_Init to prepare the module.
 *  -# After that, at any time, call #Event_Set to append a new event.
 *  -# To retrieve data, use one of the #Event_GetByIndex, #Event_GetByTime, or #Event_GetByTag.
 *      The configured callback will be called for each event that matches the given constraints until all memory has
 *      been searched.
 *  -# To ensure that all data is flushed in EEPROM, call #Event_DeInit as the last call between two sessions
 *
 * @par Example
 *  We define two different values to be used as 'tag':
 *  @snippet event_mod_example_1.c event_mod_example_1_defines
 *  Two variables are used to check the outcome of the calls to the callback:
 *  @snippet event_mod_example_1.c event_mod_example_1_variables
 *  The function @c EventCb is defined as value for @c EVENT_CB in the diversity settings:
 *  @snippet event_mod_example_1.c event_mod_example_1_callback
 *  The main code is then storing and checking a total of 6 datasets at three distinct times.
 *  @snippet event_mod_example_1.c event_mod_example_1
 *
 * @{
 */

#include "board.h"
#include "event_dft.h"

/* ------------------------------------------------------------------------- */

/**
 * Whenever data is requested via a call to #Event_GetByIndex, #Event_GetByTime or #Event_GetByTag,
 * the function pointed to by #EVENT_CB is called for each matching event. The callback must have this prototype.
 * @param tag The tag value associated with the data.
 * @param offset Absolute EEPROM Offset, in bytes, from where to start reading the associated data.
 *  The offset is relative to #EEPROM_START and can be given unmodified as @c offset argument in a call to
 *  #Chip_EEPROM_Read
 *  If this value equals @c -1, it signifies no data was associated with the tag value. The callback still reports
 *  a valid event in that case.
 * @param len The size of the associated data pointed at, in bytes.
 * @param index The sequential number of the event. The very first event stored will have index @c 0.
 *  There are two special values, #EVENT_CB_OPENING_INDEX and #EVENT_CB_CLOSING_INDEX, whose usage can be controlled via
 *  #EVENT_CB_OPENING_CALL and #EVENT_CB_CLOSING_CALL. When used, an extra call is made, @b not indicating a valid
 *  event. Use these extra calls for preparing or concluding running activities.
 * @param timestamp The RTC value at the time the data was stored.
 * @param context The value as given in the call to #Event_GetByIndex, #Event_GetByTime or #Event_GetByTag that
 *  triggered this callback.
 * @note Events are @b always reported ordered by time: the oldest events are always reported first.
 * @return
 *  - @c true to indicate searching must continue;
 *  - @c false to stop searching and end the call to #Event_GetByIndex, #Event_GetByTag or #Event_GetByTime immediately.
 *      In that case, a possible closing call will also @b not be made.
 */
typedef bool (*pEvent_Cb_t)(uint8_t tag, int offset, uint8_t len, unsigned int index, uint32_t timestamp, uint32_t context);

/* ------------------------------------------------------------------------- */

/**
 * This function must be the first function to call in this module after going to deep power down or power-off power
 * save mode. Not calling this function will result at best in random data being written and read, and possibly generate
 * hard faults.
 * @param reset
 *  - If @c true, any possible existing data stored in EEPROM is disregarded and becomes irretrievable. When this
 *      function returns, @c 0 events are available and the entire assigned EEPROM memory is empty.
 *  - If @c false, any possible existing data stored in EEPROM is kept. When this
 *      function returns, all events are available and can be fetched.
 * @pre EEPROM is initialized
 * @pre RTC is initialized
 */
void Event_Init(bool reset);

/**
 * This function must be the last function to call in this module before going to deep power down or power-off power
 * save mode.
 * @pre EEPROM is initialized and is ready to be used.
 * @post Possibly, an EEPROM flush was necessary, but that has finished when this function returns.
 * @warning Loss of power before end of this call might result in loss of the newly added samples.
 */
void Event_DeInit(void);

/**
 * The callback function as set at compile time using the diversity setting #EVENT_CB can be overridden dynamically
 * here.
 * @param cb The new function that is able to receive the events requested by calling #Event_GetByIndex,
 *  #Event_GetByTime or #Event_GetByTag. Providing @c NULL will reset the callback function to #EVENT_CB.
 * @return The callback function that was previously set.
 */
pEvent_Cb_t Event_SetCb(pEvent_Cb_t cb);

/**
 * Stores @c n samples in persistent storage. The RTC value is used as timestamp and stored together with the data.
 * @pre EEPROM is initialized
 * @pre RTC is initialized
 * @param tag : A value holding meaning for the application only. The value is stored as is and will be returned when
 *  the data is retrieved. Use this value to provide enough information to allow a correct interpretation of that
 *  retrieved data.
 * @param data : A pointer to the associated data to store. May be @c NULL.
 * @param len : The length of the associated data pointed to, in bytes. May be @c 0.
 *  The maximum length is restricted by the type and the overhead: the maximum value is restricted to a maximum of
 *  @c 255 - #EVENT_OVERHEAD.
 * @return
 *  - @c true when the data is stored - possibly not yet flushed - in EEPROM.
 *  - @c false when not enough space was available to store the data.
 */
bool Event_Set(uint8_t tag, void * data, uint8_t len);

/**
 * Retrieves events previously stored with a call to #Event_Set.
 * Events are returned one at a time by calling #EVENT_CB repetitively, until no more events match the constraints.
 * - Using the diversity setting #EVENT_CB_OPENING_CALL, the application can control whether #EVENT_CB is called one
 *  extra time with empty data before the search for matching event starts.
 * - Using the diversity setting #EVENT_CB_CLOSING_CALL, the application can control whether #EVENT_CB is called one
 *  last time with empty data to signify the end of the retrieval.
 * This is a synchronous function: it only returns after all calls to that callback have finished.
 * @pre EEPROM is initialized
 * @param first : the sequence number of the event to retrieve first.
 *  A value of @c 0 will retrieve the very first event stored.
 * @param last : the sequence number of the event to retrieve last.
 *  - If this value equals @c first, precisely one event will be returned. In this case, the callback will be called
 *      twice: the first time reporting the single matched event, the second time with empty data.
 *  - If this value is strictly smaller than @c first, no events will be returned. In this case, the callback will be
 *      called once with empty data.
 * @param context May be any number. Is not stored or looked at, only passed on as last argument in every call to
 *  #EVENT_CB. Use for your own housekeeping, as a means to provide contextual information to the callback.
 * @return The number of events reported, not including the possible opening and closing calls.
 * @see pEvent_Cb_t
 */
unsigned int Event_GetByIndex(unsigned int first, unsigned int last, uint32_t context);

/**
 * Retrieves events previously stored with a call to #Event_Set.
 * Events are returned one at a time by calling #EVENT_CB repetitively, until no more events match the constraints.
 * - Using the diversity setting #EVENT_CB_OPENING_CALL, the application can control whether #EVENT_CB is called one
 *  extra time with empty data before the search for matching event starts.
 * - Using the diversity setting #EVENT_CB_CLOSING_CALL, the application can control whether #EVENT_CB is called one
 *  last time with empty data to signify the end of the retrieval.
 * This is a synchronous function: it only returns after all calls to that callback have finished.
 * @pre EEPROM is initialized
 * @param begin : events are only reported when they have a timestamp equal to or later than @c begin.
 *  A value of @c 0 will disable this check.
 * @param end : events are only reported when they have a timestamp equal to or earlier than @c end.
 *  A value of @c 0xFFFFFFFF will disable this check.
 * @param context May be any number. Is not stored or looked at, only passed on as last argument in every call to
 *  #EVENT_CB. Use for your own housekeeping, as a means to provide contextual information to the callback.
 * @note Since the RTC value is used as timestamp, it is possible that events that are stored at a later time have a
 *  timestamp that is smaller than an earlier event. This function will scan @b all events and report @b all those that
 *  have a timestamp between (and including) @c begin and @c end.
 * @return The number of events reported, not including the possible opening and closing calls.
 * @see pEvent_Cb_t
 */
unsigned int Event_GetByTime(unsigned int begin, unsigned int end, uint32_t context);

/**
 * Retrieves events previously stored with a call to #Event_Set.
 * Events are returned one at a time by calling #EVENT_CB repetitively, until no more events match the constraints.
 * - Using the diversity setting #EVENT_CB_OPENING_CALL, the application can control whether #EVENT_CB is called one
 *  extra time with empty data before the search for matching event starts.
 * - Using the diversity setting #EVENT_CB_CLOSING_CALL, the application can control whether #EVENT_CB is called one
 *  last time with empty data to signify the end of the retrieval.
 * This is a synchronous function: it only returns after all calls to that callback have finished.
 * @pre EEPROM is initialized
 * @param tag : only events which have the same tag value are reported.
 * @param context May be any number. Is not stored or looked at, only passed on as last argument in every call to
 *  #EVENT_CB. Use for your own housekeeping, as a means to provide contextual information to the callback.
 * @return The number of events reported, not including the possible opening and closing calls.
 */
unsigned int Event_GetByTag(uint8_t tag, uint32_t context);

/**
 * Convenience function to retrieve information about the first event that matches the given tag value.
 * @b No callback function is called.
 * @param tag : the first event which has the same tag value is reported.
 * @param pOffset : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @param pLen : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @param pIndex : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @param pTimestamp : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @return @c true if a matching event was found. If @c false, the arguments have not been written to.
 */
bool Event_GetFirstByTag(uint8_t tag, int * pOffset, uint8_t * pLen, unsigned int * pIndex, uint32_t * pTimestamp);

/**
 * Convenience function to retrieve information about the last event that matches the given tag value.
 * @b No callback function is called.
 * @param tag : the last event which has the same tag value is reported.
 * @param pOffset : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @param pLen : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @param pIndex : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @param pTimestamp : May be @c NULL. Only touched when a matching event is found. See #pEvent_Cb_t
 * @return @c true if a matching event was found. If @c false, the arguments have not been written to.
 */
bool Event_GetLastByTag(uint8_t tag, int * pOffset, uint8_t * pLen, unsigned int * pIndex, uint32_t * pTimestamp);

#endif /** @} */
