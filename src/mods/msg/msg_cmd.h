/*
 * Copyright 2015-2018 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @addtogroup MODS_NSS_MSG
 * @{
 */
#ifndef __MSG_CMD_H_
#define __MSG_CMD_H_

/* -------------------------------------------------------------------------
 * Include files
 * ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Types and defines
 * ------------------------------------------------------------------------- */

#pragma pack(push, 1)

/** @see MSG_ID_READREGISTER */
typedef struct MSG_CMD_READREGISTER_S {
    uint32_t address; /**< Register address to read from. */
} MSG_CMD_READREGISTER_T;

/** @see MSG_ID_WRITEREGISTER */
typedef struct MSG_CMD_WRITEREGISTER_S {
    uint32_t address; /**< Register address to write to. */
    uint32_t data; /**< Data to write to the ARM register. */
} MSG_CMD_WRITEREGISTER_T;

/** @see MSG_ID_READMEMORY */
typedef struct MSG_CMD_READMEMORY_S {
    uint32_t address; /**< Byte offset in the ARM RAM to start reading from. */
    uint8_t length; /**< The number of consecutive bytes to read with a maximum of 32. */
} MSG_CMD_READMEMORY_T;

/** @see MSG_ID_WRITEMEMORY */
typedef struct MSG_CMD_WRITEMEMORY_S {
    uint32_t address; /**< Byte offset in the ARM RAM to start writing to. */
    uint8_t length; /**< The number of consecutive bytes to write with a maximum of 32. */
    uint8_t data[32]; /**< A container for the data to write. */
} MSG_CMD_WRITEMEMORY_T;

#pragma pack(pop)

/* ------------------------------------------------------------------------- */

/** @cond !MSG_PROTOCOL_DOC */

/**
 * Callback function type to handle custom commands.
 * @see Msg_HandleCommand
 * @param msgId Holds the id of the message
 * @param payloadLen The size in bytes of the array @c pPayload points to
 * @param pPayload Points to the message that requires to be handled. Data retention is only guaranteed until the call
 *  has ended.
 * @return
 *  - Either #MSG_OK is returned: the command has been handled, and an immediate response is given by calling
 *      #Msg_AddResponse.
 *  - Either any other value is returned: the command has not been handled, and no immediate response is given.
 *      The message handler module will use the returned error value to create an immediate response of type
 *      #MSG_RESPONSE_RESULTONLY_T.
 * @post If #MSG_OK is returned, the first call to #Msg_AddResponse during the lifetime of this call must be
 *   the immediate response.
 */
typedef uint32_t (*pMsg_CmdHandler_t)(uint8_t msgId, int payloadLen, const uint8_t* pPayload);

/**
 * Callback function type to notify the upper layer of new commands.
 * This is the signature to use when defining #MSG_COMMAND_ACCEPT_CB
 * The purpose is to provide a single entry point for the upper layer where he can decide whether the command gets
 * accepted and handled or not.
 * @note Do @b not generate responses during this callback! Use this function to inspect the data, for logging purposes
 *  and to enforce a specific communication flow.
 * @see Msg_HandleCommand
 * @param msgId Holds the id of the message
 * @param payloadLen The size in bytes of the array @c pPayload points to
 * @param pPayload Points to the payload of the command that requires to be handled.
 * @return
 *  - @c True: the command is accepted, and the appropriate command handler function is called.
 *  - @c False: the command is not accepted. An immediate response of type #MSG_RESPONSE_RESULTONLY_T is generated with
 *      #MSG_ERR_INVALID_PRECONDITION as #MSG_RESPONSE_RESULTONLY_T.result and sent back as only response.
 */
typedef bool (*pMsg_AcceptCommandCb_t)(uint8_t msgId, int payloadLen, const uint8_t* pPayload);

/**
 * Structure to be used to add the application specific command handlers.
 * @see MSG_APP_HANDLERS_COUNT
 */
typedef struct MSG_CMD_HANDLER_S {
    /** The id of the command this handler takes care of. */
    uint8_t id;

    /**
     * The function that handles a command with #id as message id.
     */
    pMsg_CmdHandler_t handler;
} MSG_CMD_HANDLER_T;

/** @endcond */

#endif /** @} */
