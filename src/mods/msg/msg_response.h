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
 * @addtogroup MODS_NSS_MSG
 * @{
 */
#ifndef __MSG_RESPONSE_H_
#define __MSG_RESPONSE_H_

/* -------------------------------------------------------------------------
 * Include files
 * ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Types and defines
 * ------------------------------------------------------------------------- */

/**
 * Defines the major API version. This should be incremented each time the API changes.
 */
#define MSG_API_MAJOR_VERSION (0x7)

/**
 * Defines the minor API version. This should be reset each time the API changes, and incremented each time the API
 * doesn't change but the implementation or documentation changes.
 */
#define MSG_API_MINOR_VERSION (0x1)

/** Lists all possible error codes that may be returned. */
typedef enum MSG_ERR {
    /** @c 0x00000000 @n No error was found. */
    MSG_OK = 0,

    /** @c 0x00010007 or <code> [07h 00h 01h 00h] </code> @n No suitable command handler could be found for this id */
    MSG_ERR_UNKNOWN_COMMAND = 0x10007,

    /**
     * @c 0x0001000B or <code> [0Bh 00h 01h 00h] </code> @n Only used in the response to a command with id
     * #MSG_ID_GETRESPONSE, to indicate no stored responses are available in the buffer.
     */
    MSG_ERR_NO_RESPONSE = 0x1000B,

    /** @c 0x0001000D or <code> [0Dh 00h 01h 00h] </code> @n A number of parameters are lacking or were given in excess. */
    MSG_ERR_INVALID_COMMAND_SIZE = 0x1000D,

    /** @c 0x0001000E or <code> [0Eh 00h 01h 00h] </code> @n At least one parameter was missing or had an invalid value. */
    MSG_ERR_INVALID_PARAMETER = 0x1000E,

    /** @c 0x0001000F or <code> [0Fh 00h 01h 00h] </code> @n The command can now not be handled. Check the
     * documentation for a correct command sequence.
     */
    MSG_ERR_INVALID_PRECONDITION = 0x1000F,

    /**
     * @c 0x00010010 or <code> [10h 00h 01h 00h] </code> @n The handler of the command is a stub and still needs to be
     * implemented; or the implementation to handle the given combination of parameters is incomplete and a work in
     * progress. Check with the developer to retrieve updated firmware.
     */
    MSG_ERR_INVALID_NYI = 0x10010,

    /**
     * @c 0x0001003F @n
     * This error code does not encompass an error. It is used to signify the highest id that is reserved
     * for use by the message handler itself. All application specific error codes must use id's greater than this
     * value.
     * @note Only to be used as an offset for your own application specific error codes.
     */
    MSG_ERR_LASTRESERVED = 0x1003F
} MSG_ERR_T;

/* ------------------------------------------------------------------------- */

#pragma pack(push, 1)

/** Used whenever no other information is required to be returned except the outcome of a command. */
typedef struct MSG_RESPONSE_RESULTONLY_S {
    /**
     * The command result.
     * @note If this structure is used to return an immediate result, the interpretation of the result value is
     *  different, depending on the type of command: synchronous or asynchronous:
     * - For synchronous commands, the result indicates the result of executing the command. The command has been
     *  fully executed.
     *  #MSG_OK means that the command was successfully executed,
     *  any other value indicates an error code and signifies that the command has been partially executed or not at
     *  all.
     * - For asynchronous commands the result only indicates the result of receiving the command and checking its
     *  pre-conditions.
     *  #MSG_OK means that the commands was successfully received and execution of the command has started,
     *  any other value indicates an error code and signifies that the command will be partially executed or not at
     *  all. The final result of the execution of the command will be part of the response that will be queued later.
     * @note If this structure is used to respond to the command #MSG_ID_GETRESPONSE, it signifies the final
     *  result of the execution of an asynchronous command.
     */
    uint32_t result;
} MSG_RESPONSE_RESULTONLY_T;

/** @see MSG_ID_GETVERSION */
typedef struct MSG_RESPONSE_GETVERSION_S {
    uint16_t reserved2; /**< Reserved for future use. Must be 0. Does not bear any significance. */
    uint16_t swMajorVersion; /**< The software major version */
    uint16_t swMinorVersion; /**< The software minor version */
    uint16_t apiMajorVersion; /**< Equal to #MSG_API_MAJOR_VERSION */
    uint16_t apiMinorVersion; /**< Equal to #MSG_API_MINOR_VERSION */
    /**
     * This value will be equal to:
     *  - @c 0x4E310020 for NHS3100 devices
     *  - @c 0x4E315220 for NHS3152 devices
     */
    uint32_t deviceId;
} MSG_RESPONSE_GETVERSION_T;

/** @see MSG_ID_READREGISTER */
typedef struct MSG_RESPONSE_READREGISTER_S {
    /**
     * The command result.
     * Only when @c result equals #MSG_OK, the contents of @c data is valid.
     */
    uint32_t result;

    uint32_t data; /**< The value read from the ARM Register */
} MSG_RESPONSE_READREGISTER_T;

/** @see MSG_ID_READMEMORY */
typedef struct MSG_RESPONSE_READMEMORY_S {
    /**
     * The command result.
     * Only when @c result equals #MSG_OK, the contents of @c data is valid.
     */
    uint32_t result;

    uint8_t length; /**< The number of consecutive bytes read. */

    /**
     * The values read from the ARM memory.
     * The extraneous array elements - where the index is greater than @c length - are set to 0.
     */
    uint8_t data[32];
} MSG_RESPONSE_READMEMORY_T;

/** @see MSG_ID_GETUID */
typedef struct MSG_RESPONSE_GETUID_S {
    uint32_t uid[4]; /**< The sequence of 4 32-bit words (LSByte first) is guaranteed unique among all NHS31xx ICs. */
} MSG_RESPONSE_GETUID_T;

/** @see MSG_ID_GETNFCUID */
typedef struct MSG_RESPONSE_GETNFCUID_S {
    uint8_t nfcuid[8]; /**< The sequence of bytes as assigned to the NFC controller. */
} MSG_RESPONSE_GETNFCUID_T;

/** @see MSG_ID_CHECKBATTERY */
typedef struct MSG_RESPONSE_CHECKBATTERY_S {
    /**
     * The command result.
     * Only when @c result equals #MSG_OK, the contents of @c data is valid.
     */
    uint32_t result;

    /**
     * Extra current consumption is enabled in progressive steps. The extra current consumption which triggered the BOD
     * is returned here.
     * A negative value indicates BOD could not be triggered.
     */
    int32_t threshold;
} MSG_RESPONSE_CHECKBATTERY_T;

#if MSG_ENABLE_GETCALIBRATIONTIMESTAMP
/** @see MSG_ID_GETCALIBRATIONTIMESTAMP */
typedef struct MSG_RESPONSE_GETCALIBRATIONTIMESTAMP_S {
    /**
     * The command result.
     * Only when @c result equals #MSG_OK, the contents below are valid.
     */
    uint32_t result;

    uint32_t timestamp; /**< The epoch time in seconds of the second calibration point. */
} MSG_RESPONSE_GETCALIBRATIONTIMESTAMP_T;
#endif

#pragma pack(pop)

/* ------------------------------------------------------------------------- */

/** @cond !MSG_PROTOCOL_DOC */

/**
 * Callback function type to handle responses.
 * @param responseLength The size in bytes of the array @c pResponseData points to
 * @param pResponseData Points to the response, that must be sent back to the host. Data retention is only guaranteed
 *  until the call has ended.
 * @return
 *  - @c true if the response was accepted, the relevant data has been copied, and the memory occupied by it may be
 *      cleared.
 *  - @c false if the response was rejected. The response will be stored internally by the message handler module, and
 *      can be fetched later by issuing the command #MSG_ID_GETRESPONSE. If the internal storage is full, the oldest
 *      response is discarded.
 */
typedef bool (*pMsg_ResponseCb_t)(int responseLength, const uint8_t* pResponseData);

/** @endcond */

#endif /** @} */
