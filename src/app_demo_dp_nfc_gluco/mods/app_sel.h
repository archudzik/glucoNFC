/*
 * Copyright 2015-2017,2020-2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __APP_SEL_H_
#define __APP_SEL_H_

/* These 'defines' capture the settings of the applications. The displayed values refer to the default settings.
 * To override the default settings, place the defines with their desired values in the application app_sel.h header
 * file: the compiler will pick up your defines before parsing this file.
 */

/* Diversities tweaking msg module for application-specific usage. */

#define MSG_APP_HANDLERS_COUNT 7
#define MSG_APP_HANDLERS MsgHandler_CmdHandlers
#define MSG_ENABLE_RESET 1
#define MSG_ENABLE_PREPAREDEBUG 1
#define MSG_ENABLE_GETUID 1
#define SW_MAJOR_VERSION 1
#define SW_MINOR_VERSION 1

/* Diversities tweaking ndeft2t module for application-specific usage. */

#define NDEFT2T_FIELD_STATUS_CB App_FieldStatusCb
#define NDEFT2T_MSG_AVAILABLE_CB App_MsgAvailableCb

#endif
