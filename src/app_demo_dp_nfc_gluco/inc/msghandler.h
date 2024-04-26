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

#ifndef MSGHANDLER_H_
#define MSGHANDLER_H_

/** @defgroup APP_DEMO_TADHERENCE_MSGHANDLER Message Handler Module
 * @ingroup APP_DEMO_TADHERENCE
 * The Therapy Adherence Message Handler is responsible for handling the communication with the host (tag reader/smartphone).
 *  -# It makes use of:
 *      - @ref NFC_NSS as its communication channel.
 *      - @ref TIMER_NSS (Timer 0) to implement a Host-Timeout detection mechanism.
 *      - @ref MODS_NSS_MSG module to implement a command/response mechanism.
 *      - @ref MODS_NSS_NDEFT2T to generate/parse NDEF formatted messages.
 *  -# The supported command ID's are described by #MSGHANDLER_MSG_ID_T enum
 *  -# The specifics of the protocol are described in the documentation section of the @ref MODS_NSS_MSG module
 *  -# The content of the commands and responses defined by the Therapy Adherence demo application
 *      are described by @ref APP_DEMO_TADHERENCE_MSGHANDLER_PROTOCOL.
 *  -# It is also responsible for sending and receiving the data to the underlying physical communication channel
 *      (NFC interface).
 * @{
 */

/**
 * Makes sure the messaging mechanism is initialized.
 * It also initializes the NFC tag with a getVersion response.
 * @pre NFC should be initialized
 * @post Ready to receive commands from NFC.
 */
void MsgHandler_Init(void);

/**
 * Function performing NFC communication, this function is blocking until the host stops sending commands (host timeout).
 */
void MsgHandler_NFCCommunication(void);

/** @} */
#endif
