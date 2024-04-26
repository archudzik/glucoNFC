/*
 * Copyright 2016-2018,2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "board.h"
#include "chip.h"
#include "ndeft2t/ndeft2t.h"
#include "msghandler.h"
#include "memorymanager.h"
#include "msghandler_protocol.h"
#include "therapy.h"
#include "text.h"

static void StartHostTimeoutTimer(int seconds);
static void StopHostTimeoutTimer(void);
static uint32_t SetPristineHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload);
static uint32_t SetRhythmHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload);
static uint32_t GetRhythmHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload);
static uint32_t StartHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload);
static uint32_t GetStartHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload);
static uint32_t GetRemovalsHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload);
static void AddMimeRecord(int payloadLength, const uint8_t* payload);
static void AddTextRecords(void);
static bool ResponseCb(int responseLength, const uint8_t* responseData);

/** Flag indicating whether communication is ongoing. */
static volatile bool sCommunicating;

/** Instance buffer to be used by the NDEFT2T module */
__attribute__ ((section(".noinit"))) __attribute__((aligned (4)))
static uint8_t sNdefInstanceTx[NDEFT2T_INSTANCE_SIZE];

__attribute__ ((section(".noinit"))) __attribute__((aligned (4)))
static uint8_t sNdefInstanceRx[NDEFT2T_INSTANCE_SIZE];

/** Buffer for outgoing data (response) (NFC/NDEF)  */
__attribute__ ((section(".noinit"), aligned (4)))
static uint8_t sTxData[NFC_SHARED_MEM_BYTE_SIZE];

/** Buffer for incoming data (command) (NFC/NDEF)  */
__attribute__ ((section(".noinit"), aligned (4)))
static uint8_t sRxData[NFC_SHARED_MEM_BYTE_SIZE];


MSG_CMD_HANDLER_T MsgHandler_CmdHandlers[MSG_APP_HANDLERS_COUNT] = {{MSG_ID_SETPRISTINE, SetPristineHandler},
                                                                    {MSG_ID_SETRHYTHM, SetRhythmHandler},
                                                                    {MSG_ID_GETRHYTHM, GetRhythmHandler},
                                                                    {MSG_ID_START, StartHandler},
                                                                    {MSG_ID_GETSTART, GetStartHandler},
                                                                    {MSG_ID_GETPILLREMOVALS, GetRemovalsHandler}};

/**
 * Called under interrupt.
 * @see NDEFT2T_FIELD_STATUS_CB
 * @see pNdeft2t_FieldStatus_Cb_t
 */
void App_FieldStatusCb(bool status)
{
    if (status) {
        /* As long as an NFC field is present, we can keep the IC in active mode such that the TAG READER
         * can read our initial message. */
        StopHostTimeoutTimer();
    }
    else {
        /* From what was observed, the power off/power on/(re-)select sequence takes place in the order
         * of a few 100ms at most. Waiting a full second seems plenty.
         */
        StartHostTimeoutTimer(1);
    }
}

/**
 * Called under interrupt.
 * @see NDEFT2T_MSG_AVAILABLE_CB
 * @see pNdeft2t_MsgAvailable_Cb_t
 */
void App_MsgAvailableCb(void)
{
    /* A PCD can do very strange things, a.o. power off the field and immediately power on the field again, or 10+ times
     * select the same device as part of its illogic procedure to select a PICC and start communicating with it.
     * Instead of relying on NFC_INT_NFCOFF solely, it is more robust to additionally check if during a small interval
     * no NFC field is started again.
     * The loop in ExecuteNfcMode() is thus not to look at the NFC interrupt status, but at the result of the RTC
     * interrupt: gCommunicating
     */
    /* The NFC memory might contain a complete command now, we can now handle the commands it contains.*/
    if (NDEFT2T_GetMessage(sNdefInstanceRx, sRxData, sizeof(sRxData))) {
        uint8_t * data;
        int length;
        NDEFT2T_PARSE_RECORD_INFO_T recordInfo;
        while (NDEFT2T_GetNextRecord(sNdefInstanceRx, &recordInfo)) {
            if (recordInfo.type == NDEFT2T_RECORD_TYPE_MIME) {
                data = NDEFT2T_GetRecordPayload(sNdefInstanceRx, &length);
                Msg_HandleCommand(length, data);
            }
        }
    }
}

/** Timer interrupt used for NFC communication (HOST timeout). */
void CT16B0_IRQHandler(void)
{
    if (Chip_TIMER_MatchPending(NSS_TIMER16_0, 0)) {
        Chip_TIMER_ClearMatch(NSS_TIMER16_0, 0);
        sCommunicating = false;
    }
}

void MsgHandler_Init(void)
{
    NDEFT2T_Init();

    Msg_Init();
    Msg_SetResponseCb(ResponseCb);

    NDEFT2T_CreateMessage(sNdefInstanceTx, sTxData, sizeof(sTxData), true);
    AddMimeRecord(0, NULL);
    AddTextRecords();
    NDEFT2T_CommitMessage(sNdefInstanceTx);

}

/**
 * The action stage where actual NFC communication is possible.
 */
void MsgHandler_NFCCommunication(void)
{
    /* Initiate the communication by putting a initial message. */
    MsgHandler_Init();

    StartHostTimeoutTimer(1);

    /* Just try to communicate. We start the timer to make sure sCommunicating is set back to false
     * again when nothing is detected any more by the NFC controller (HOST timeout).
     */
    sCommunicating = true;
    while (sCommunicating) { /* Will be set to false after a timeout in CT16B0_IRQHandler */
        ; /* Wait */
        /* Field removal is handled in App_FieldStatusCb */
        /* A new NDEF message written by the tag reader is handled by App_MsgAvailableCb */
    };
}

static uint32_t SetPristineHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload)
{
    MSG_RESPONSE_RESULTONLY_T response;

    (void)pPayload;/* suppress [-Wunused-parameter]: no argument is expected, but if present redundantly, just ignore. */

    response.result = MSG_ERR_INVALID_COMMAND_SIZE;
    if (0 == payloadLen) {
        Therapy_Reset();
        response.result = MSG_OK;
    }
    Msg_AddResponse(msgId, sizeof(response), (uint8_t*)&response);
    return MSG_OK;
}

static uint32_t SetRhythmHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload)
{
    MSG_CMD_SETRHYTHM_T * command;
    MSG_RESPONSE_RESULTONLY_T response;

    command = (MSG_CMD_SETRHYTHM_T *)pPayload;
    response.result = MSG_ERR_INVALID_COMMAND_SIZE;
    if (payloadLen == sizeof(MSG_RHYTHM_T)) {
        Memory_StoreData(MEMORY_SLOT_ID_RHYTHM, command, sizeof(MSG_RHYTHM_T));
        response.result = MSG_OK;
    }
    Msg_AddResponse(msgId, sizeof(response), (uint8_t*)&response);
    return MSG_OK;
}

static uint32_t GetRhythmHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload)
{
    MSG_RESPONSE_GETRHYTHM_T response;

    (void)pPayload;/* suppress [-Wunused-parameter]: no argument is expected, but if present redundantly, just ignore. */

    if (0 == payloadLen) {
        Memory_GetStoredData(MEMORY_SLOT_ID_RHYTHM, &response, sizeof(MSG_RHYTHM_T));
    }
    else {
        response.period = 0;
        response.leniency = 0;
    }
    if (response.period == 0) {
        memset(response.intakeOffset, 0xFF, sizeof(uint32_t) * 16 /* 16 == array size of response.intakeOffset */);
    }
    Msg_AddResponse(msgId, sizeof(response), (uint8_t*)&response);
    return MSG_OK;
}

static uint32_t StartHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload)
{
    MSG_CMD_START_T * command;
    MSG_RESPONSE_RESULTONLY_T response;
    command = (MSG_CMD_START_T *)pPayload;

    if (payloadLen == sizeof(MSG_CMD_START_T)) {
        response.result = MSG_OK;
    }
    else {
        response.result = MSG_ERR_INVALID_COMMAND_SIZE;
    }
    Msg_AddResponse(msgId, sizeof(response), (uint8_t*)&response);
    if (response.result == MSG_OK) {
        Therapy_Start(command->current, command->updateInterval);
    }
    return MSG_OK;
}

static uint32_t GetStartHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload)
{
    MSG_RESPONSE_GETSTART_T response;

    (void)pPayload;/* suppress [-Wunused-parameter]: no argument is expected, but if present redundantly, just ignore. */

    if (0 == payloadLen) {
        response.start = Therapy_GetStartTime(); /* Only becomes a problem after Tue, 19 Jan 2038 03:14:08 GMT */
        response.current = (uint32_t)Chip_RTC_Time_GetValue(NSS_RTC);
    }
    else {
        response.start = 0;
        response.current = 0;
    }
    Msg_AddResponse(msgId, sizeof(response), (uint8_t*)&response);
    return MSG_OK;
}

static uint32_t GetRemovalsHandler(uint8_t msgId, int payloadLen, const uint8_t* pPayload)
{
    MSG_RESPONSE_GETPILLREMOVALS_T response;

    (void)pPayload;/* suppress [-Wunused-parameter]: no argument is expected, but if present redundantly, just ignore. */

    memset(response.removals, 0, sizeof(response.removals));
    if (0 == payloadLen) {
        memcpy(&response.removals, Memory_GetEvents(),
               (sizeof(THERAPY_PILLREMOVAL_INFO_T) * (unsigned int)Therapy_GetIntakeCount()));
    }
    Msg_AddResponse(msgId, sizeof(response), (uint8_t*)&response);
    return MSG_OK;
}

static bool ResponseCb(int responseLength, const uint8_t* responseData)
{
    NDEFT2T_CreateMessage(sNdefInstanceTx, sTxData, sizeof(sTxData), true);
    AddMimeRecord(responseLength, responseData);
    NDEFT2T_CommitMessage(sNdefInstanceTx);
    return true;
}

/**
 * Function which adds a MIME record to #sNdefInstanceTx with the given payload
 * @param payloadLength : Lenght of the payload to be encapsulated by a MIME record
 * @param payload : payload to be encapsulated by a MIME record
 */
static void AddMimeRecord(int payloadLength, const uint8_t* payload)
{
    bool shortRecord = payloadLength <= 255;
    NDEFT2T_CREATE_RECORD_INFO_T recordInfoShortMIME = {(uint8_t*)"tadherence/demo.nhs.nxp.log", shortRecord , 0};
    NDEFT2T_CreateMimeRecord(sNdefInstanceTx, &recordInfoShortMIME);
    if (payloadLength) {
        NDEFT2T_WriteRecordPayload(sNdefInstanceTx, payload, payloadLength);
    }
    NDEFT2T_CommitRecord(sNdefInstanceTx);
}

/**
 * Function which adds txt records to #sNdefInstanceTx (printed on tab of phone)
 */
static void AddTextRecords(void)
{
    uint32_t now;
    int remainingPills;
    int nrOfIntakeStrings;
    char *statusStr;

    const NDEFT2T_CREATE_RECORD_INFO_T recordInfoShortText = {(uint8_t*)"en", true, 0};

    /* First add string describing the current therapy status */
    NDEFT2T_CreateTextRecord(sNdefInstanceTx, &recordInfoShortText);

    switch (Therapy_GetState()) {
        case THERAPY_STATE_ID_ONGOING:
            now = (uint32_t)Chip_RTC_Time_GetValue(NSS_RTC);
            remainingPills = Therapy_GetInitialPillCount() - Therapy_GetIntakeCount();
            statusStr = Text_StatusOngoing(Therapy_GetStartTime(), now, remainingPills);
            break;
        case THERAPY_STATE_ID_STOPPED:
            statusStr = Text_StatusStopped(Therapy_GetStartTime(), Therapy_GetLastIntakeTime());
            break;
        default:
            statusStr = Text_StatusNotStarted();
            break;
    }
    NDEFT2T_WriteRecordPayload(sNdefInstanceTx, statusStr, (int)strlen(statusStr));
    NDEFT2T_CommitRecord(sNdefInstanceTx);

    nrOfIntakeStrings = Therapy_GetIntakeCount();

    if (nrOfIntakeStrings > 0) {
        int n = 0;
        while (n < nrOfIntakeStrings) {
            int count = nrOfIntakeStrings - n;
            if (count >= 5) {
                /* It seems the text message on an Android phone can span 5 lines.
                 * So list at most 5 events at a time.
                 */
                count = 5;
            }
            NDEFT2T_CreateTextRecord(sNdefInstanceTx, &recordInfoShortText);

            while (count > 0) {
                const char* intakeStringN = Memory_GetIntakeString(n);
                NDEFT2T_WriteRecordPayload(sNdefInstanceTx, intakeStringN, (int)strlen(intakeStringN));
                n++;
                count--;
            }
            NDEFT2T_CommitRecord(sNdefInstanceTx);
        }
    }
}

/**
 * Starts the timeout counter, will end communication after a host is gone for @c seconds.
 */
static void StartHostTimeoutTimer(int seconds)
{
    StopHostTimeoutTimer();
    Chip_TIMER16_0_Init();
    Chip_TIMER_PrescaleSet(NSS_TIMER16_0,
                           ((uint32_t)Chip_Clock_System_GetClockFreq() >> 8) - 1 /* 1/1024s tick, -1 for 0-based PC */);
    Chip_TIMER_SetMatch(NSS_TIMER16_0, 0, (uint32_t)seconds << 8);
    Chip_TIMER_MatchEnableInt(NSS_TIMER16_0, 0);

    Chip_TIMER_Reset(NSS_TIMER16_0);
    Chip_TIMER_Enable(NSS_TIMER16_0);
    NVIC_EnableIRQ(CT16B0_IRQn);
}

/**
 * Stops the timeout counter.
 */
static void StopHostTimeoutTimer(void)
{
    NVIC_DisableIRQ(CT16B0_IRQn);
    Chip_TIMER_Disable(NSS_TIMER16_0);
}
