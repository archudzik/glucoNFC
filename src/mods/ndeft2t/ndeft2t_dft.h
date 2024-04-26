/*
 * Copyright 2014-2017,2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @defgroup MODS_NSS_NDEFT2T_DFT Diversity Settings
 * @ingroup MODS_NSS_NDEFT2T
 * These 'defines' capture the diversity settings of the module. The displayed values refer to the default settings.
 * To override the default settings, place the defines with their desired values in the application app_sel.h header
 * file: the compiler will pick up your defines before parsing this file.
 *
 * @par Flags
 *  - Flags enabling notifications
 *      - #NDEFT2T_FIELD_STATUS_CB
 *      - #NDEFT2T_MSG_AVAILABLE_CB
 *      - #NDEFT2T_MSG_READ_CB
 *  - Miscellaneous flags
 *      - #NDEFT2T_EEPROM_COPY_SUPPPORT
 *      - #NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION
 * @{
 */
#ifndef __NDEFT2T_DFT_H_
#define __NDEFT2T_DFT_H_

/**
 * Set this flag to '1' to enable support to copy record payload directly from EEPROM to message buffer and '0' to disable.
 */
#if !defined(NDEFT2T_EEPROM_COPY_SUPPPORT)
    #define NDEFT2T_EEPROM_COPY_SUPPPORT 1
#endif

/**
 * Set this flag to '1' to enable message header length correction and '0' to disable. When set to '1', it allows the
 * message to be created even if the argument 'shortMessage' in #NDEFT2T_CreateMessage API was set wrongly by the caller.
 * When set to '0' the message creation will fail and an error will be returned.
 */
#if !defined(NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION)
    #define NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION 1
#endif

/* Diversity flags below are undefined by default. They are wrapped in a DOXYGEN precompilation flag to enable
 * documenting them properly. To define them and use the corresponding functionality of the module, make the correct
 * defines in app_sel.h or board_sel.h.
 */
#ifdef __DOXYGEN__
#error This block of code may not be parsed using gcc.

/**
 * NDEFT2T MOD does interrupt handling by itself. So, the below callback shall be defined, to get notified on the NFC
 * field status. Refer @ref nfcIntHandling_anchor "NFC Interrupt Handling" for more details.
 * @note The value set @b must have the same signature as #pNdeft2t_FieldStatus_Cb_t.
 * @note This must be set to the name of a function, not a pointer to a function: no dereference will be made!
 */
#define NDEFT2T_FIELD_STATUS_CB application function of type Ndeft2t_FieldStatus_Cb_t

/**
 * The below callback shall be defined, for the application to get notified on the presence of a valid NDEF Message in
 * shared memory. Refer @ref nfcIntHandling_anchor "NFC Interrupt Handling" for more details.
 * @note The value set @b must have the same signature as #pNdeft2t_MsgAvailable_Cb_t.
 * @note This must be set to the name of a function, not a pointer to a function: no dereference will be made!
 */
#define NDEFT2T_MSG_AVAILABLE_CB application function of type pNdeft2t_MsgAvailable_Cb_t

/**
 * NFC tag readers can lack the capability to write (think of all phones running iOS11), or may want to avoid writing
 * to increase the throughput. Support for an 'automatic mode', where the firmware can write consecutive messages in the
 * NFC shared memory as fast as possible, is enabled using this callback.
 * When enabled via #NDEFT2T_EnableAutomaticMode, the provided callback is executed @b under @b interrupt each time the
 * NFC tag reader has read the last page of the NDEF message in the NFC shared memory.
 * @note This functionality stops working when the NFC tag reader starts writing any write in any page disables the
 *  'automatic mode'. Re-enable it by calling #NDEFT2T_EnableAutomaticMode again. Note that the firmware may not get
 *  notified of this: only when a full NDEF message is written, the application supplied #NDEFT2T_MSG_AVAILABLE_CB is
 *  called.
 * @note The value set @b must have the same signature as #pNdeft2t_MsgRead_Cb_t.
 * @note This must be set to the name of a function, not a pointer to a function: no dereference will be made!
 */
#define NDEFT2T_MSG_READ_CB application function of type pNdeft2t_MsgRead_Cb_t
#endif

#endif /** @} */
