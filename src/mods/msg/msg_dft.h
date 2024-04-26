/*
 * Copyright 2015-2019,2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @defgroup MODS_NSS_MSG_DFT Diversity Settings
 * @ingroup MODS_NSS_MSG
 * The message handler module allows the application to enable or disable certain behavior through the use diversity
 * flags in the form of defines. Sensible defaults are chosen; to deviate, simply define the relevant flags and message
 * ids in your application before including the message handler module.
 *
 * @par Flags
 *  - Flags to enable application specific commands and responses:
 *      - #MSG_APP_HANDLERS,
 *      - #MSG_APP_HANDLERS_COUNT and
 *      - #MSG_CATCHALL_HANDLER.
 *
 * @par Special message ids
 *  - #MSG_ID_GETVERSION is a special message id, as it is always enabled - see #MSG_ENABLE_GETVERSION. @n
 *      There are two related flags for this id:
 *      - #SW_MAJOR_VERSION and
 *      - #SW_MINOR_VERSION.
 *  - #MSG_ID_GETRESPONSE is a special message id, as it is automatically enabled when you expand the functionality of
 *      the message handler module to use a buffer, i.e. by defining both
 *      - #MSG_RESPONSE_BUFFER and
 *      - #MSG_RESPONSE_BUFFER_SIZE,
 *  - #MSG_ENABLE_PREPAREDEBUG is a special message id, as it is automatically enabled and disabled based on the build
 *      configuration - specifically, when DEBUG is defined or not. It can be overridden using #MSG_ENABLE_PREPAREDEBUG
 *
 * @par Additional hooks to control the flow
 *  - #MSG_COMMAND_ACCEPT_CB
 *  - #MSG_RESPONSE_DISCARDED_CB
 *
 * @par Other message ids
 *  A few predefined message id's are also enabled by default, and can be disabled in an application's app_sel.h:
 *      - #MSG_ENABLE_GETUID
 *      - #MSG_ENABLE_GETNFCUID
 *      - #MSG_ENABLE_GETCALIBRATIONTIMESTAMP
 *  All other predefined message id's are disabled by default, and can be enabled in an application's app_sel.h:
 *      - #MSG_ENABLE_RESET
 *      - #MSG_ENABLE_READREGISTER
 *      - #MSG_ENABLE_WRITEREGISTER
 *      - #MSG_ENABLE_READMEMORY
 *      - #MSG_ENABLE_WRITEMEMORY
 *      - #MSG_ENABLE_PREPAREDEBUG
 *      - #MSG_ENABLE_GETUID
 *      - #MSG_ENABLE_CHECKBATTERY
 *
 * @par Example
 *  To override each and every diversity flag that the message handler module offers, create defines that are known to
 *  the precompiler before including the message handler module.
 *  The recommended way to do this is to add the code below to @c app_sel.h
 *  @code
 *      #define MSG_APP_HANDLERS_COUNT 15
 *      #define MSG_APP_HANDLERS <name of MSG_CMD_HANDLER_T array>
 *      #define MSG_CATCHALL_HANDLER <name of pMsg_CmdHandler_t function>
 *      #define SW_MAJOR_VERSION 4
 *      #define SW_MINOR_VERSION 2
 *      #define MSG_RESPONSE_BUFFER_SIZE 65
 *      #define MSG_RESPONSE_BUFFER <name of uint8_t buffer>
 *      #define MSG_COMMAND_ACCEPT_CB <name of pMsg_AcceptCommandCb_t function>
 *      #define MSG_RESPONSE_DISCARDED_CB <name of pMsg_ResponseCb_t function>
 *      #define MSG_ENABLE_RESET 1
 *      #define MSG_ENABLE_READREGISTER 1
 *      #define MSG_ENABLE_WRITEREGISTER 1
 *      #define MSG_ENABLE_READMEMORY 1
 *      #define MSG_ENABLE_WRITEMEMORY 1
 *      #define MSG_ENABLE_PREPAREDEBUG 1
 *      #define MSG_ENABLE_GETUID 1
 *      #define MSG_ENABLE_CHECKBATTERY 1
 *      #define MSG_ENABLE_GETCALIBRATIONTIMESTAMP 1
 *  @endcode
 *
 * @{
 */
#ifndef __MSG_DFT_H_
#define __MSG_DFT_H_

#ifndef MSG_APP_HANDLERS_COUNT
    /**
     * @pre To define custom command handlers, both #MSG_APP_HANDLERS_COUNT and #MSG_APP_HANDLERS must be defined.
     * @pre #MSG_APP_HANDLERS_COUNT must be a strict positive number indicating the size of the array
     *  #MSG_APP_HANDLERS
     */
    #define MSG_APP_HANDLERS_COUNT 0
#endif
#if !MSG_APP_HANDLERS_COUNT
    #undef MSG_APP_HANDLERS
#endif
#if MSG_APP_HANDLERS_COUNT && !defined(MSG_APP_HANDLERS)
    #error MSG_APP_HANDLERS and MSG_APP_HANDLERS_COUNT must be defined jointly.
#endif

/* ------------------------------------------------------------------------- */

/**
 * The command/response for #MSG_ID_GETVERSION is enforced and can not be disabled.
 * @see MSG_ID_GETVERSION
 */
#define MSG_ENABLE_GETVERSION 1

/**
 * Use the major version to distinguish between applications and/or application types.
 * This define is used when generating the response to #MSG_ID_GETVERSION
 */
#if !SW_MAJOR_VERSION
    #define SW_MAJOR_VERSION 0
#endif

/**
 * Use the minor version to distinguish between application updates.
 * This define is used when generating the response to #MSG_ID_GETVERSION
 */
#if !SW_MINOR_VERSION
    #define SW_MINOR_VERSION 0
#endif

/* ------------------------------------------------------------------------- */

#ifndef MSG_RESPONSE_BUFFER_SIZE
    /**
     * Responses that do not get treated immediately can be stored in an internal buffer.
     * @pre To define a buffer to be used by the message handler module, both @c MSG_RESPONSE_BUFFER_SIZE and
     *  #MSG_RESPONSE_BUFFER must be defined.
     * Define here the size of the buffer.
     * @note responses greater than <b>or equal to</b> this size can not be stored.
     * @note #MSG_RESPONSE_DISCARDED_CB will also be used whenever a response is
     *  not immediately accepted and:
     *  - no buffer is set and.
     *  - the response size is bigger than the buffer size.
     *  - the response size is bigger than @c 255 bytes: This is a limitation fully due to the implementation how the
     *      responses are stored in the internal buffer.
     * @note Defining #MSG_RESPONSE_BUFFER_SIZE will automatically enable the command/response for #MSG_ID_GETRESPONSE
     * @see pMsg_ResponseCb_t
     */
    #define MSG_RESPONSE_BUFFER_SIZE 0
#endif
#if !MSG_RESPONSE_BUFFER_SIZE
    #undef MSG_RESPONSE_BUFFER
#endif
#if MSG_RESPONSE_BUFFER_SIZE && !defined(MSG_RESPONSE_BUFFER)
    #error MSG_RESPONSE_BUFFER and MSG_RESPONSE_BUFFER_SIZE must be defined jointly.
#endif

/**
 * This command is automatically enabled when #MSG_RESPONSE_BUFFER_SIZE is set.
 * @see MSG_RESPONSE_BUFFER
 * @see MSG_ID_GETRESPONSE
 */
#define MSG_ENABLE_GETRESPONSE (MSG_RESPONSE_BUFFER_SIZE > 0)

/* ------------------------------------------------------------------------- */

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_RESET
 */
#if !MSG_ENABLE_RESET
   #define MSG_ENABLE_RESET 0
#endif

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_READREGISTER
 * @warning Should be disabled in production code; may be useful for debugging and testing purposes.
 */
#if !MSG_ENABLE_READREGISTER
   #define MSG_ENABLE_READREGISTER 0
#endif

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_WRITEREGISTER
 * @warning Should be disabled in production code; may be useful for debugging and testing purposes.
 */
#if !MSG_ENABLE_WRITEREGISTER
    #define MSG_ENABLE_WRITEREGISTER 0
#endif

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_READMEMORY
 * @warning Should be disabled in production code; may be useful for debugging and testing purposes.
 */
#if !MSG_ENABLE_READMEMORY
    #define MSG_ENABLE_READMEMORY 0
#endif

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_WRITEMEMORY
 * @warning Should be disabled in production code; may be useful for debugging and testing purposes.
 */
#if !MSG_ENABLE_WRITEMEMORY
    #define MSG_ENABLE_WRITEMEMORY 0
#endif

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_PREPAREDEBUG
 * @warning Should be disabled in production code; may be useful for debugging and testing purposes.
 */
#ifndef MSG_ENABLE_PREPAREDEBUG
    #ifdef DEBUG
        #define MSG_ENABLE_PREPAREDEBUG 1
    #else
        #define MSG_ENABLE_PREPAREDEBUG 0
    #endif
#endif

/**
 * Assign a zero value to disable the handling of the command #MSG_ID_GETNFCUID
 */
#ifndef MSG_ENABLE_GETNFCUID
    #define MSG_ENABLE_GETNFCUID 1
#endif

/**
 * Assign a zero value to disable the handling of the command #MSG_ID_GETUID
 */
#ifndef MSG_ENABLE_GETUID
    #define MSG_ENABLE_GETUID 1
#endif

/**
 * Assign a non-zero value to enable the handling of the command #MSG_ID_CHECKBATTERY.
 * @warning When enabled, the module @ref MODS_NSS_BATIMP "batimp" must also be referenced in the project.
 */
#ifndef MSG_ENABLE_CHECKBATTERY
    #define MSG_ENABLE_CHECKBATTERY 0
#endif

/**
 * Assign a zero value to disable the handling of the command #MSG_ID_GETCALIBRATIONTIMESTAMP
 */
#ifndef MSG_ENABLE_GETCALIBRATIONTIMESTAMP
    #define MSG_ENABLE_GETCALIBRATIONTIMESTAMP 1
#endif


/* Diversity flags below are undefined by default. They are wrapped in a DOXYGEN precompilation flag to enable
 * documenting them properly. To define them and use the corresponding functionality of the module, make the correct
 * defines in app_sel.h or board_sel.h.
 */
#ifdef __DOXYGEN__
#error This block of code may not be parsed using gcc.

/**
 *  To define custom command handlers, both #MSG_APP_HANDLERS_COUNT and #MSG_APP_HANDLERS
 *  must be defined.
 *  @pre #MSG_APP_HANDLERS must be an array with elements of type #MSG_CMD_HANDLER_T
 */
#define MSG_APP_HANDLERS application defined array with MSG_APP_HANDLERS_COUNT elements of type #MSG_CMD_HANDLER_T

/**
 *  Responses that do not get treated immediately can be stored in an internal buffer. To define a buffer to be used
 *  by the message handler module, both #MSG_RESPONSE_BUFFER_SIZE and #MSG_RESPONSE_BUFFER must be defined.
 *  Define here the location of the buffer.
 *  @note There are no alignment requirements.
 *  @note The value set must match type #pMsg_ResponseCb_t
 *  @note Defining #MSG_RESPONSE_BUFFER_SIZE will automatically enable the command/response for #MSG_ID_GETRESPONSE
 */
#define MSG_RESPONSE_BUFFER application defined array with MSG_RESPONSE_BUFFER_SIZE elements of type uint8_t

/**
 *  Adds a hook inside #Msg_HandleCommand that allows to block handling each individual command.
 *  @note The value set must match type #pMsg_AcceptCommandCb_t
 */
#define MSG_COMMAND_ACCEPT_CB application function of type pMsg_AcceptCommandCb_t

/**
 *  When the flexibility offered at compile time with #MSG_APP_HANDLERS and
 *  #MSG_APP_HANDLERS_COUNT is not enough, a callback can be defined where all untreated commands are directed to. The
 *  upper layer can then dynamically decide to treat certain commands or not.
 *  @note The value set must match type #pMsg_CmdHandler_t
 *  @note This must be set to the name of a function, not a pointer to a function: no dereference will be made!
 */
#define MSG_CATCHALL_HANDLER application function of type pMsg_CmdHandler_t

/**
 *  Responses that do not get treated immediately can be stored in an internal buffer, and they can be fetched later
 *  using #MSG_ID_GETRESPONSE. Whenever this buffer runs full, the oldest response is discarded. To get notified when
 *  a response is being discarded, set this define to the callback to be called.
 *  @note The value set must match type #pMsg_ResponseCb_t
 *  @note This must be set to the name of a function, not a pointer to a function: no dereference will be made!
 *  @note This callback will also be used when no buffer is set and whenever a response is not immediately
 *      accepted.
 */
#define MSG_RESPONSE_DISCARDED_CB application function of type pMsg_ResponseCb_t
#endif

#endif /** @} */
