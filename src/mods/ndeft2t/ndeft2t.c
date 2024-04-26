/*
 * Copyright 2014-2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include <string.h>
#include "chip.h"
#include "ndeft2t/ndeft2t.h"

/* -------------------------------------------------------------------------
 * Private types and enumerations
 * ------------------------------------------------------------------------- */

#define NDEFT2T_NDEF_TLV_START_OFFSET 0x8 /**< Offset from the shared memory start where the NDEF TLV starts. */
#define NDEFT2T_NDEF_PAYLOAD_START_OFFSET_SHORT (NDEFT2T_NDEF_TLV_START_OFFSET + 0x2) /**< Offset from the shared memory start
                                               where the actual payload or V field starts for a message of length < 255 bytes.
                                               This includes the Lock TLV and the Proprietary TLV lengths as well. */
#define NDEFT2T_NDEF_PAYLOAD_START_OFFSET_LONG (NDEFT2T_NDEF_TLV_START_OFFSET + 0x4) /**< Offset from the shared memory start
                                               where the actual payload or V field starts for a message of length >= 255 bytes.
                                               This includes the Lock TLV and the Proprietary TLV lengths as well. */
#define NDEFT2T_MIN_RECORD_HEADER_FIXED_LENGTH 0x04 /**< Minimum length of the fixed part of an NDEF record header.
                                               These fields are the record header byte, TYPE LENGTH, PAYLOAD LENGTH
                                               and ID LENGTH. The PAYLOAD LENGTH can be 1 or 4 bytes and 1byte is
                                               assumed here. */
#define NDEFT2T_MAX_RECORD_HEADER_FIXED_LENGTH 0x07 /**< Maximum length of the fixed part of an NDEF record header.
                                               These fields are the record header byte, TYPE LENGTH, PAYLOAD LENGTH
                                               and ID LENGTH. The PAYLOAD LENGTH can be 1 or 4 bytes and 4bytes is
                                               assumed here. */
#define NDEFT2T_SHORT_PAYLOAD_LENGTH_LEN 0x01 /**< For a short record, the PAYLOAD LENGTH field has just 1 byte. */
#define NDEFT2T_LONG_PAYLOAD_LENGTH_LEN 0x04 /**< For a long record, the PAYLOAD LENGTH field has 4 bytes. */

/**
 * Defines for the different TLVs that exist: see the technical specification from NFC Forum: NFCForum-TS-T2T-1.0.pdf
 * "Type 2 Tag Technical Specification, Version 1.0, 2017-08-28, 4.9.2 List of Control TLVs for Type 2 Tag", Table 4"
 * @{
 */
#define TLV_NULL 0x00
#define TLV_LOCK 0x01
#define TLV_MEMORY 0x02
#define TLV_NDEF 0x03
#define TLV_PROPRIETARY 0xFD
#define TLV_TERMINATOR 0xFE
/** @} */

#define NDEFT2T_NDEF_SHORT_MSG_LIMIT 0xFE /**< If message size is less than or equal to oxFE, then a one byte length
                                               field is used. Otherwise, a 3 byte format has to be used. */
#define NDEFT2T_NDEF_SHORT_RECORD_LIMIT 0xFF /**< Maximum payload data size that a short record can have. */
#define NDEFT2T_NDEF_3BYTE_LEN_START 0xFF /**< A 3 byte length format starts with a 0xFF. */

#define NDEFT2T_GET_TNF(hdr) ((hdr) & 0x07) /**< Extract TNF from record header byte. */
#define NDEFT2T_GET_IL(hdr) (bool)(((hdr) >> 3) & 0x01) /**< Extract IL from record header byte. */
#define NDEFT2T_GET_SR(hdr) (bool)(((hdr) >> 4) & 0x01) /**< Extract SR from record header byte. */
#define NDEFT2T_GET_CF(hdr) (bool)(((hdr) >> 5) & 0x01) /**< Extract CF from record header byte. */

#define NDEFT2T_TERM_TLV_INIT_VAL 0xFFFFFFFFUL /**< Initialiser value for terminator TLV offset. */

/**
 * Helper. Performs integer division, rounding up.
 * @param n Must be a positive number. Will be evaluated once.
 * @param d Must be a strict positive number Will be evaluated twice.
 */
#define IDIVUP(n, d) (((n)+(d)-1)/(d))

/**
 * Default TLV bytes to be copied to the first 3 pages of the NFC shared memory.
 * It consists of
 * - a lock TLV
 * - a proprietary TLV
 * - an empty NDEF message
 * - Two 0 bytes
 *
 * The lock TLV consists of:
 * - a Type: #TLV_LOCK
 * - a Length: 3
 * - a Value: bytes E8h 0Eh 46h, with
 *  - E8h: the position of the lock region: PagesAddr = E and ByteOffset = 8
 *  - 0Eh: the size of the lock region in bits
 *  - 46h: the 'Page Control': BytesPerPage = 6 and BytesLockedPerLockBit = 4 (i.e. one lock bit lock 2**4 bytes)
 * The lock TLV is used to enable tag readers to retrieve the location of the Dynamic Lock Bits. Using the information
 * above: the first byte can be found at byte offset PagesAddr * 2**BytesPerPage + ByteOffset == 904, i.e. NFC page E2h.
 *
 * The proprietary TLV consists of:
 * - a Type: #TLV_PROPRIETARY
 * - a Length: 1
 * - a Value: 0 (don't care)
 * The proprietary TLV is used as padding, to ensure the NDEF TLV starts at a 32-bit word boundary. A tag reader will
 * not overwrite the lock TLV and proprietary TLV and instead write his own data after them. The addition of the
 * proprietary TLV ensures that the size field of the NDEF message is now always within the same page. When can thus
 * check a single page to monitor changes in the length of an incoming TLV message.
 *
 * The empty NDEF message consists of:
 * - a Type: #TLV_NDEF
 * - a Length: 0 (empty)
 * - no Value.
 *
 * The 2 zero bytes are added to ease the NDEF message creation process: an NDEF message can have a L value of 1 byte
 * or 3 bytes. See #NDEFT2T_CreateMessage.
 */
static const uint8_t __attribute__((aligned (4))) sDefaultBytes[12] = {
    /* T */ TLV_LOCK,
    /* L */ 3,
    /* V */ 0xE8, 0x0E, 0x46,

    /* T */ TLV_PROPRIETARY,
    /* L */ 1,
    /* V */ 0,

    /* T */ TLV_NDEF,
    /* L */ 0,

    0, 0
};

/** TNF Types. */
typedef enum NDEFT2T_TNF {
    NDEFT2T_TNF_EMPTY = 0x00, /**< This TNF value can be used to create empty records. For example, to terminate
     an NDEF message when payload is not present for any reason. */
    NDEFT2T_TNF_NFC_RTD = 0x01, /**< NFC Forum well-known type. */
    NDEFT2T_TNF_MIME_MEDIA = 0x02, /**< Media-type as defined in RFC 2046 [RFC 2046]. */
    NDEFT2T_TNF_ABSOLUTE_URI = 0x03, /**< Absolute URI as defined in RFC 3986 [RFC 3986]. */
    NDEFT2T_TNF_NFC_RTD_EXT = 0x04, /**< NFC Forum external type [NFC RTD]. */
    NDEFT2T_TNF_UNKNOWN = 0x05, /**< Unknown. */
    NDEFT2T_TNF_UNCHANGED = 0x06, /**< Unchanged. */
    NDEFT2T_TNF_RESERVED = 0x07 /**< Reserved. */
} NDEFT2T_TNF_T;

/** NFC Forum well-known types. */
typedef enum NDEFT2T_NFC_RTD {
    NDEFT2T_NFC_RTD_TEXT = 'T', /**< NFC Forum WNT Text, {0x54} or "T". */
    NDEFT2T_NFC_RTD_URI = 'U', /**< NFC Forum WNT URI, {0x55} or "U". */
} NDEFT2T_NFC_RTD_T;

/**
 * NDEF housekeeping data structure.
 */
typedef struct {
    /**
     * Used during both generation and parsing of NDEF message.
     * @{
     */
    uint8_t *pCursor; /**< Running pointer to track current location in message buffer. */
    int msgSize; /**< Message size in bytes. Used to keep track of the buffer usage during message generation. */
    int bufLen; /**< Length of the message buffer. */
    int len; /**< Variable to track the length of record payload. */
    /** @} */

    /**
     * Used only during generation of NDEF message.
     * @{
     */
    uint8_t *pLastRecordHdr; /**< Pointer to last written Record header. Used to update ME bit of the last record while
     committing the message */
    bool msgBegin; /**< Used to track the first record of the message. */
    bool shortRecord; /**< When set to '1' indicates that a short record type is enabled. */
    bool shortMessage; /**< To Track if the length of the message is <= 254 bytes or not. */
    /** @} */
} NDEFT2T_INSTANCE_T;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** If this construct doesn't compile, the define #NDEFT2T_INSTANCE_SIZE is no longer equal to @c sizeof(#NDEFT2T_INSTANCE_T) */
int checkSizeOfNdeft2tInstance[(NDEFT2T_INSTANCE_SIZE == sizeof(NDEFT2T_INSTANCE_T)) ? 1 : -1]; /* Dummy variable since we can't use sizeof during precompilation. */

#pragma GCC diagnostic pop

/* -------------------------------------------------------------------------
 * Private functions and variables
 * ------------------------------------------------------------------------- */

static bool CreateRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo, NDEFT2T_RECORD_TYPE_T type,
                         NDEFT2T_TNF_T tnf, int hdrLen, bool typeStringPresent);
static uint8_t* DecodeNdefTlv(int *lenTlv);
static bool ValidateNdefMsg(void *pInstance);
#if NDEFT2T_EEPROM_COPY_SUPPPORT == 1
static void CopyFromEeprom(uint8_t *pDst, const void * pSrc, int size);
#endif
static void EnableTermTlvDetection(void);
static void DisableTermTlvDetection(void);
static void DisableMessageReadDetection(void);
#ifdef NDEFT2T_MSG_READ_CB
static void ReadOnly_IRQHandler(NFC_INT_T nfcInterruptMaskedStatus);
#endif
static void ReadWrite_IRQHandler(NFC_INT_T nfcInterruptMaskedStatus);

/** Holds the byte offset location of the terminator TLV in the message that is getting parsed. */
static volatile uint32_t sTermTlvOffset;

/**
 * Holds the 32 bit word from the message getting created corresponding to the location where the terminator TLV of
 * the message that was parsed before was present.
 */
static volatile uint32_t sTermTlvPage;

#ifdef NDEFT2T_MSG_READ_CB
static volatile bool sAutomaticMode;

/**
 * Either @c 0, either the page number of the last payload of the NDEF message in the NFC shared memory.
 * - Set in NDEFT2T_CommitMessage
 * - Used in #ReadOnly_IRQHandler
 * - Reset in #ReadOnly_IRQHandler immediately after use.
 * Only used when #sAutomaticMode is @c true.
 */
static uint32_t sLastPageOfPayload;
#endif

/* -------------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------------- */

void NDEFT2T_Init(void)
{
    sTermTlvOffset = NDEFT2T_TERM_TLV_INIT_VAL;
    DisableMessageReadDetection();

    NDEFT2T_ResetNfcMemory();

    Chip_NFC_Int_ClearRawStatus(NSS_NFC, NFC_INT_ALL);
    NVIC_EnableIRQ(NFC_IRQn);
}

void NDEFT2T_DeInit(void)
{
    /* Disable all NFC interrupts. */
    Chip_NFC_Int_SetEnabledMask(NSS_NFC, NFC_INT_NONE);
    NVIC_DisableIRQ(NFC_IRQn);
}

void NDEFT2T_ResetNfcMemory(void)
{
    memcpy((void *)NSS_NFC->BUF, sDefaultBytes, sizeof(sDefaultBytes));
}

#ifdef NDEFT2T_MSG_READ_CB
void NDEFT2T_EnableMessageReadDetection(unsigned int lastPageOfMessage)
{
    sAutomaticMode = true;
    sLastPageOfPayload = lastPageOfMessage;

    if (sLastPageOfPayload) {
        Chip_NFC_SetTargetAddress(NSS_NFC, 2);
        NFC_INT_T rawStatus = Chip_NFC_Int_GetRawStatus(NSS_NFC);
        Chip_NFC_Int_ClearRawStatus(NSS_NFC, rawStatus & (NFC_INT_MEMREAD | NFC_INT_TARGETREAD));
        /* Enable TARGET_READ. */
        Chip_NFC_Int_SetEnabledMask(NSS_NFC, NFC_INT_RFSELECT | NFC_INT_NFCOFF | NFC_INT_MEMWRITE | NFC_INT_TARGETREAD);
    }
}
#endif

void NDEFT2T_DisableMessageReadDetection(void)
{
    DisableMessageReadDetection();
}

void NDEFT2T_CreateMessage(void *pInstance, uint8_t *pBuffer, int bufLen, bool shortMessage)
{
    int len;

    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    ASSERT((pInstance != NULL) && (pBuffer != NULL));
    /* Check for word alignment of message buffer and ensure that the size is a multiple of 4. */
    ASSERT(((int )pBuffer & 0x3) == 0);
    ASSERT((bufLen >= NDEFT2T_NDEF_PAYLOAD_START_OFFSET_LONG) && ((bufLen % 4) == 0));

    /* Initialize instance variables. */
    pInst->bufLen = bufLen;
    pInst->pLastRecordHdr = NULL;
    pInst->len = 0;
    pInst->shortMessage = shortMessage;

    /* Fill predefined bytes to start of buffer. The first is a pre-defined lock TLV (01 03 E8 0E 46) to enable
     * android stacks to resolve location of dynamic lock bits. Then, write an empty NDEF message header. The first
     * 2 bytes in the empty NDEF message header are reserved with NULL TLVs for handling 3-byte 'length field' usage,
     * when payload exceeds 254 bytes. This empty NDEF message header is updated with the actual length at the end of
     * message creation in NDEFT2T_CommitRecord().
     */
    len = sizeof(sDefaultBytes);
    memcpy((uint32_t *)pBuffer, sDefaultBytes, (uint32_t)len);
    if (shortMessage) {
        len -= 2;
    }
    pBuffer += len;

    /* Increment message size by length of NDEFT2T_NDEF_PAYLOAD_START_OFFSET. Additional 1 byte is reserved for
     * terminator TLV. Writing terminator TLV is optional as per the NDEF standard. However, we are enforcing
     * termination in this implementation for inter-operability with all readers. */
    pInst->msgSize = len + 1;

    /* Preserve current message buffer position. */
    pInst->pCursor = pBuffer;

    /* Used for setting MB bit in the header of the first record. This will be reset back to '0' after the creation
     * of the very first record. */
    pInst->msgBegin = 1;
}

bool NDEFT2T_CreateTextRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo)
{
    ASSERT((pRecordInfo != NULL) && (pRecordInfo->pString != NULL));
    return (CreateRecord(pInstance, pRecordInfo, NDEFT2T_RECORD_TYPE_TEXT, NDEFT2T_TNF_NFC_RTD, 5, true));
}

bool NDEFT2T_CreateExtRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo)
{
    ASSERT((pRecordInfo != NULL) && (pRecordInfo->pString != NULL));
    return (CreateRecord(pInstance, pRecordInfo, NDEFT2T_RECORD_TYPE_EXT, NDEFT2T_TNF_NFC_RTD_EXT, 3, true));
}

bool NDEFT2T_CreateMimeRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo)
{
    ASSERT((pRecordInfo != NULL) && (pRecordInfo->pString != NULL));
    return (CreateRecord(pInstance, pRecordInfo, NDEFT2T_RECORD_TYPE_MIME, NDEFT2T_TNF_MIME_MEDIA, 3, true));
}

bool NDEFT2T_CreateUriRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo)
{
    ASSERT(pRecordInfo != NULL);
    return (CreateRecord(pInstance, pRecordInfo, NDEFT2T_RECORD_TYPE_URI, NDEFT2T_TNF_NFC_RTD, 5, false));
}

bool NDEFT2T_WriteRecordPayload(void *pInstance, const void * pData, int size)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    int msgSize;
    int len;
#if NDEFT2T_EEPROM_COPY_SUPPPORT == 1
    bool isSourceEeprom = false;
#endif

    ASSERT((pInst != NULL) && (pInst->pCursor != NULL) && (pData !=NULL));
#if NDEFT2T_EEPROM_COPY_SUPPPORT == 1
    if (((int)pData >= EEPROM_START) && ((int)pData <= (EEPROM_START + (EEPROM_NR_OF_RW_ROWS * EEPROM_ROW_SIZE)))) {
        isSourceEeprom = true;
    }
#endif

    /* Check if the message fits within the shared memory. */
    /* Check if there is sufficient buffer to process the message. */
    msgSize = pInst->msgSize + size;
    if ((msgSize > NFC_SHARED_MEM_BYTE_SIZE ) || (msgSize > pInst->bufLen)) {
        return false;
    }

    /* Check if payload length exceeds 255 for a short record type. */
    len = pInst->len + size;
    if (pInst->shortRecord && (len > NDEFT2T_NDEF_SHORT_RECORD_LIMIT)) {
        return false;
    }

    pInst->msgSize = msgSize; /* Assign message size with incremented value. */
    pInst->len = len; /* Increment payload data length. */

    /* Copy record payload to the message buffer and update the message buffer pointer. */
#if NDEFT2T_EEPROM_COPY_SUPPPORT == 1
    if (isSourceEeprom) {
        CopyFromEeprom(pInst->pCursor, pData, size);
    }
    else
#endif
    {
        memcpy(pInst->pCursor, pData, (uint32_t)size);
    }
    pInst->pCursor += size;
    return true;
}

void NDEFT2T_CommitRecord(void *pInstance)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    uint8_t *pCursor;
    int len;

    ASSERT((pInst != NULL) && (pInst->pCursor != NULL) && (pInst->pLastRecordHdr != NULL));

    /* Set the message buffer pointer to start of length field of the current record header. */
    pCursor = pInst->pLastRecordHdr + 2;

    len = *pCursor; /* Extract the length of any pre-header for the payload. */
    len += pInst->len; /* Increment length by length of payload. */

    /* Fill the payload length field with a single-byte or 4-byte format depending on whether the record type used is
     * a short one or not. */
    if (pInst->shortRecord) {
        *pCursor++ = (uint8_t)len; /* Payload Length. */
    }
    else {
        *pCursor++ = 0x00; /* Payload Length byte 3. Shared memory is only 512 bytes, so this can never be set. */
        *pCursor++ = 0x00; /* Payload Length byte 2. Shared memory is only 512 bytes, so this can never be set. */
        *pCursor++ = (uint8_t)((len >> 8) & 0xFF); /* Payload Length byte 1. */
        *pCursor++ = (uint8_t)(len & 0xFF); /* Payload Length byte 0. */
    }

    /* Clear Message Begin(MB) bit as it is applicable only for the very first record. */
    pInst->msgBegin = 0x00;
}

bool NDEFT2T_CommitMessage(void *pInstance)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    uint32_t *pCursor;
    int ndefHdr;
    int lenTlv;
    int msgSize;
#ifdef NDEFT2T_MSG_READ_CB
    uint32_t lastPageOfPayload = 0;
#endif
    bool statusPayload = true;
    bool statusHdr = true;

    ASSERT((pInstance != NULL) && (pInst->pCursor != NULL));

    msgSize = pInst->msgSize;

    /* Extract L part of the NDEF message TLV to fix the header. */
    /* Remove the length of any other TLVs present before the NDEF TLV and also the NDEF message header. The
     * length of the terminator TLV (1 byte) also needs to be removed. */
    if (pInst->shortMessage) {
        lenTlv = msgSize - (NDEFT2T_NDEF_PAYLOAD_START_OFFSET_SHORT + 1);
    }
    else {
        lenTlv = msgSize - (NDEFT2T_NDEF_PAYLOAD_START_OFFSET_LONG + 1);
    }

    if (lenTlv >= NDEFT2T_MIN_RECORD_HEADER_FIXED_LENGTH) {
        /* A message with an empty short record needs at least 4 bytes of payload. The check above ensures that at least
         * one record is successfully created before the below code is executed. */
        ASSERT(pInst->pLastRecordHdr != NULL);

        /* Setting ME bit for last record header. */
        *pInst->pLastRecordHdr = (*pInst->pLastRecordHdr) | (1 << 6);
    }

    /* Get start of NDEF TLV in message buffer. */
    pCursor = (uint32_t*)(pInst->pCursor - (msgSize - 1) + NDEFT2T_NDEF_TLV_START_OFFSET);

#if NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION == 1
    if ((pInst->shortMessage == true) && (lenTlv > NDEFT2T_NDEF_SHORT_MSG_LIMIT)) {
        msgSize += 2;
        if (msgSize > NFC_SHARED_MEM_BYTE_SIZE) {
            return false;
        }
        pInst->shortMessage = false;
        /* V in the NDEF TLV located at pCursor+2 has to be shifted 2 bytes to the right starting from pCursor+4 to
         * accommodate a 3 byte length field (L). */
        memmove((uint8_t*)pCursor + 4, (uint8_t*)pCursor + 2, (uint32_t)lenTlv);
        pInst->pCursor += 2;
    }
    else if ((pInst->shortMessage == false) && (lenTlv <= NDEFT2T_NDEF_SHORT_MSG_LIMIT)) {
        msgSize -= 2;
        pInst->shortMessage = true;
        /* V in the NDEF TLV located at pCursor+4 has to be shifted 2 bytes to the left starting from pCursor+2,
         * since the length field (L) needs only 1 byte against the initial assumption of 3 byte. */
        memmove((uint8_t*)pCursor + 2, (uint8_t*)pCursor + 4, (uint32_t)lenTlv);
        pInst->pCursor -= 2;
    }

#else /* NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION */
    if ((pInst->shortMessage == true) && (lenTlv > NDEFT2T_NDEF_SHORT_MSG_LIMIT)) {
        return false;
    }
    else if ((pInst->shortMessage == false) && (lenTlv <= NDEFT2T_NDEF_SHORT_MSG_LIMIT)) {
        return false;
    }
#endif /* NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION */

    *pInst->pCursor++ = TLV_TERMINATOR; /* Write Terminator TLV. */
    pCursor = (uint32_t*)(pInst->pCursor - msgSize); /* Get start of message buffer. */

#ifdef NDEFT2T_MSG_READ_CB
    if (sAutomaticMode) {
        /* Monitor the last page to know when the tag reader has read the NDEF message.
         * When that is read, NDEFT2T_MSG_READ_CB is called from within ReadOnly_IRQHandler().
         * msgSize will be made a multiple of 4 below, so the last page that will be written will then be equal to
         * (msgSize / 4) - 1.
         * However, it is possible that the last page only occupies a single byte from the complete NDEF message:
         * only the very last byte TLV_TERMINATOR. The chance of this occurring is 25%. iOS 11 is not reading
         * out that marker, solely depending on the L (length) value of the TLV. No interrupt will be raised when iOS
         * reads out such an NDEF message, since it does not read the last page (the one that contains only
         * TLV_TERMINATOR).
         * Examples (with xx indicating payload bytes of the NDEF message:
         * - msgSize is initially 19, becomes 20, last page #4 (the fifth) contains xx xx FE 00, page to monitor is #4,
         * -                      20          20            #4 (the fifth)          xx xx xx FE                     #4,
         * -                      21          24            #5 (the sixth)          FE 00 00 00                     #4,
         * -                      22          24            #5 (the sixth)          xx FE 00 00                     #5.
         * The last page to monitor that contains NDEF payload bytes can thus be calculated before adjusting msgSize:
         */
        ASSERT(msgSize >= 2);
        lastPageOfPayload = (uint32_t)((msgSize - 2) / 4);
    }
#endif
    if (msgSize & 0x3) { /* msgSize need to be made a multiple of 4 since writing has to be done as words. */
        msgSize = (int)((uint32_t)msgSize & 0xFFFFFFFC) + 4;
    }

    /* Preserve the buffer pointer and message size which needs to be used later. */
    pInst->pCursor = (uint8_t*)pCursor;
    pInst->msgSize = msgSize;

#ifdef NDEFT2T_MSG_READ_CB
    if (!sAutomaticMode) {
#else
    {
#endif
        /* Check if a message was parsed before, which is true if the variable storing the terminator TLV offset holds
         * a valid value. We also need to check if the terminator TLV offset is within the message boundary of the one
         * that is getting created, as the corruption can happen only then. The terminator TLV detection logic is
         * enabled when both the conditions are met.*/
        if ((sTermTlvOffset != NDEFT2T_TERM_TLV_INIT_VAL) && (sTermTlvOffset < (uint32_t)msgSize)) {
            /* Store the 32-bit word present at the terminator tlv page of the previously parsed message for applying
             * correction later. */
            sTermTlvPage = *(pCursor + (sTermTlvOffset/4));
            /* Enable the terminator TLV detection and correction logic. */
            EnableTermTlvDetection();
        }
    }

    memcpy((void *)NSS_NFC->BUF, pCursor, (uint32_t)msgSize);

#ifdef NDEFT2T_MSG_READ_CB
    if (sAutomaticMode) {
        NDEFT2T_EnableMessageReadDetection(lastPageOfPayload);
    }
#endif

    /* Write NDEF message header into page 5 and 6 of shared memory. */
    if (lenTlv > NDEFT2T_NDEF_SHORT_MSG_LIMIT) {
        ndefHdr = (int)((lenTlv << 24)
                | ((lenTlv << 8) & 0xFF0000)
                | (NDEFT2T_NDEF_3BYTE_LEN_START << 8)
                | TLV_NDEF);
    }
    else {
        ndefHdr = *((int*)(NFC_SHARED_MEM_START + 8)); /* Retrieve the NDEF message header. */
        ndefHdr &= (int)0xFFFF00FF; /* Clear length field, which is at at byte position 1. This might or might not be '0'
                                    depending on initial setting of pInst->shortMessage*/
        ndefHdr |= lenTlv << 8; /* Fill length. */
    }

    *((int*)(NFC_SHARED_MEM_START + 8)) = ndefHdr;

    return statusPayload || statusHdr;
}

bool NDEFT2T_GetMessage(void *pInstance, uint8_t *pBuffer, int bufLen)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    uint8_t *pV;
    int lenTlv;
    bool status = true;

    ASSERT((pInstance != NULL) && (pBuffer != NULL));
    /* Check for word alignment of message buffer and ensure that the size is a multiple of 4. */
    ASSERT(((int )pBuffer & 0x3) == 0);
    ASSERT(bufLen >= NDEFT2T_NDEF_PAYLOAD_START_OFFSET_LONG && (bufLen % 4) == 0);

    /* Detect NDEF, mandatory for read operation. */
    pV = DecodeNdefTlv(&lenTlv);
    if (NULL == pV) {
        return false;
    }

    /* Check if the message fits within the shared memory. Otherwise, either the message length is corrupted or
     * message extends to NFC EEPROM region. An external reader can write into NFC EEPROM as well, though this is
     * not an intended action. */
    if ((uint32_t)(pV + lenTlv) > NFC_SHARED_MEM_END) {
        return false;
    }

    /* Check if there is sufficient buffer to process the message. */
    if (bufLen < lenTlv) {
        return false;
    }

    /* Initialise instance variables. */
    pInst->bufLen = bufLen;
    pInst->pCursor = pBuffer;

    /* Copy the NDEF message from shared memory into the message buffer for further processing. Byte read access to
     * shared memory is supported unlike for writing. */
    /* Message size copied here is exactly the L field of the NDEF message TLV. */
    pInst->msgSize = lenTlv;
    if (lenTlv) {

        memcpy(pBuffer, pV, (uint32_t)lenTlv);

        /* Check the correctness of the NDEF message. */
        status = ValidateNdefMsg(pInstance);
    }
    pInst->len = 0; /* Reset Payload length as it gets set by calling NDEFT2T_GetNextRecord(). */
    return status;
}

bool NDEFT2T_GetNextRecord(void *pInstance, NDEFT2T_PARSE_RECORD_INFO_T *pRecordInfo)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    uint8_t *pCursor;
    int typeLen;
    int len;
    int temp;
    int type;
    int msgSize;
    bool il;
    int ilLen;
    NDEFT2T_TNF_T tnf;
    bool shortRecord;
    int minHdrLen;

    ASSERT((pInstance != NULL) && (pRecordInfo != NULL) && (pInst->pCursor !=NULL));

    /* Initialise the record info structure to default values. */
    pRecordInfo->pString = NULL;
    pRecordInfo->stringLength = 0;
    pInst->len = 0;

    /* Extract current message size and message buffer location. */
    msgSize = pInst->msgSize;
    pCursor = pInst->pCursor;

    if (msgSize) {
        /* Preserve latest record header. Required only in cases where the complete message is read first and then
         * written back to the shared memory. */
        pInst->pLastRecordHdr = pCursor;

        /* Extract the different fields from the record header byte such as TNF, SR, IL etc. */
        temp = *pCursor++;
        tnf = NDEFT2T_GET_TNF(temp);
        il = NDEFT2T_GET_IL(temp);
        shortRecord = NDEFT2T_GET_SR(temp);
        pRecordInfo->chunked = NDEFT2T_GET_CF(temp);
    }
    else {
        /* The message size get's decremented during the parsing of the NDEF message. So, a message size of '0'
         * indicates reaching end of message. */
        return false;
    }

    /* Check if there is enough bytes left to extract TYPE_LENGTH and PAYLOAD_LENGTH and ID Length. Otherwise, it
     * indicates a corrupted message. */
    minHdrLen = NDEFT2T_MAX_RECORD_HEADER_FIXED_LENGTH
            - ((NDEFT2T_LONG_PAYLOAD_LENGTH_LEN - NDEFT2T_SHORT_PAYLOAD_LENGTH_LEN) * shortRecord) - (!il);
    if (msgSize < minHdrLen) {
        return false;
    }
    msgSize -= minHdrLen;

    /* Extract TYPE Length. */
    typeLen = *pCursor++;

    /* Extract payload Length. It is one byte for short record and 4 bytes otherwise. */
    if (shortRecord) {
        len = *pCursor++; /* Payload Length. */
    }
    else {
        len = ((*pCursor++) << 24); /* Payload Length byte 3. */
        len |= ((*pCursor++) << 16); /* Payload Length byte 2. */
        len |= ((*pCursor++) << 8); /* Payload Length byte 1. */
        len |= (*pCursor++); /* Payload Length byte 0. */
    }

    /* Extract ID field Length. This is not passed to the application. However, we need the value to skip ID
     * field if present in the message. */
    ilLen = 0; /* To ensure that some checks below works fine. Otherwise, it will have some junk value. */
    if (il) {
        ilLen = *pCursor++; /* ID Length. */
    }

    /* Check if there is enough bytes left to extract TYPE, ID and PAYLOAD. Otherwise, it indicates a corrupted
     * message. */
    minHdrLen = typeLen + len + ilLen;
    if (msgSize < minHdrLen) {
        return false;
    }
    msgSize -= minHdrLen;
    /* Preserve message bytes left. */
    pInst->msgSize = msgSize;

    /* Rest of the processing for different TNF types. */
    switch (tnf) {
        case NDEFT2T_TNF_EMPTY:
            /* For an empty record, the TYPE_LENGTH, ID_LENGTH, and PAYLOAD_LENGTH fields MUST be zero and
             * the TYPE, ID, and PAYLOAD fields are thus omitted from the record. */
            if (typeLen || ilLen || len) {
                return false;
            }
            pRecordInfo->type = NDEFT2T_RECORD_TYPE_EMPTY;
            break;

        case NDEFT2T_TNF_NFC_RTD:
            /* Do a comparison of the type string with supported types and set the value of RTD type. */
            if (typeLen == 0x01) {
                type = *pCursor++;
                /* Skip ID field, if present as we do not support passing the same to higher layers. */
                if (il) {
                    pCursor += ilLen;
                }

                /* Decrement payload length by length of pre-header (fixed part) preceding the payload. */
                len -= 1;

                if (type == NDEFT2T_NFC_RTD_TEXT) {/* Check for TEXT type record. */
                    pRecordInfo->type = NDEFT2T_RECORD_TYPE_TEXT;
                    /* Extract TEXT record specific fields. */
                    temp = *pCursor++; /* Pre header for text record. */

                    /* Check for any unsupported or invalid encode format. */
                    if (temp & 0x80) {
                        return false;
                    }

                    /* Extract length of locale string. */
                    temp &= 0x3F;
                    pRecordInfo->pString = pCursor;
                    pRecordInfo->stringLength = temp;
                    len -= temp; /* Decrement payload length by length of locale. */
                    pCursor += temp;
                }
                else if (type == NDEFT2T_NFC_RTD_URI) {/* Check for URI type record. */
                    pRecordInfo->type = NDEFT2T_RECORD_TYPE_URI;
                    /* Extract URI record specific fields. */
                    pCursor++; /* Skip URI identifier code. */
                }
                else {
                    return false; /* Record type not supported or invalid. */
                }
            }
            else {
                return false; /* Record type not supported or invalid. */
            }
            break;

        case NDEFT2T_TNF_MIME_MEDIA:
            /* Assign type, type string and type string length. */
            pRecordInfo->type = NDEFT2T_RECORD_TYPE_MIME;
            pRecordInfo->pString = pCursor;
            pRecordInfo->stringLength = typeLen;
            pCursor += typeLen;

            if (il) { /* Skip ID field, if present. */
                pCursor += ilLen;
            }
            break;

        case NDEFT2T_TNF_NFC_RTD_EXT:
            /* Assign type, type string and type string length. */
            pRecordInfo->type = NDEFT2T_RECORD_TYPE_EXT;
            pRecordInfo->pString = pCursor;
            pRecordInfo->stringLength = typeLen;
            pCursor += typeLen;

            if (il) { /* Skip ID field, if present. */
                pCursor += ilLen;
            }
            break;

        case NDEFT2T_TNF_ABSOLUTE_URI:
            /* Unsupported TNF type. */
        default:
            /* Includes TNF=0x05,0x06,0x07. */
            /* Unsupported TNF types. */
            return false;
            break;
    }

    /* Increment by payload length and preserve message buffer position. */
    pInst->pCursor = pCursor + len;

    /* Assign payload length. */
    pInst->len = len;

    return true;
}

void* NDEFT2T_GetRecordPayload(void *pInstance, int *pLen)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    uint8_t *pCursor;

    ASSERT((pInstance != NULL) && (pLen != NULL) && (pInst->pCursor !=NULL));

    if (pInst->len != 0) {
        /* Length of record payload. */
        *pLen = pInst->len;
        pCursor = pInst->pCursor - pInst->len;
    }
    else {
        *pLen = 0;
        pCursor = NULL;
    }
    /* Return start address of the payload in message buffer. The caller can use this to read out the record data. */
    return pCursor;
}

/* ------------------------------------------------------------------------- */

/**
 * This function creates a record of type mentioned by the argument NDEFT2T_RECORD_TYPE_T type. The function will
 * reserve space for the record header, fill known values to the record header and initialise related instance
 * variables. The function has to be called after calling NDEFT2T_CreateMessage(). Call to this function has to be
 * followed by copying the actual payload to the message buffer and then by finalising the record header. Copying of
 * payload to the message buffer has to be done by calling NDEFT2T_WriteRecordPayload() and finalising of the header by
 * calling NDEFT2T_CommitRecord() .
 * @param pInstance : Base address of instance Buffer
 * @param pRecordInfo : Base address of the record type information data structure instantiated in application. The
 *                      caller has to initialise the NDEFT2T_CREATE_RECORD_INFO_T structure field uriCode and set or
 *                      clear shortRecord based on the need to use short records. The pString field is not applicable for
 *                      URI record. Refer to @ref NDEFT2T_CREATE_RECORD_INFO_T data structure for more details.
 * @param type : type of the record. Refer NDEFT2T_RECORD_TYPE_T enum for more details.
 * @return true/false for success/failure of operation respectively. The function returns false under the below
 *  scenarios:\n
 *      -# Size of the NDEF message being created exceeds the size of message buffer allocated by caller
 *      -# Size of the NDEF message being created exceeds the size of the shared memory
 */
static bool CreateRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo, NDEFT2T_RECORD_TYPE_T type,
                         NDEFT2T_TNF_T tnf, int hdrLen, bool typeStringPresent)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    bool shortRecord;
    int msgSize;
    uint8_t *pCursor;
    int typeStringLen;

    ASSERT((pInst != NULL) && (pInst->pCursor!= NULL));

    /* Extract shortRecord into a local variables as it is frequently used. */
    shortRecord = pRecordInfo->shortRecord;

    typeStringLen = 0;
    if (typeStringPresent) {
        typeStringLen = (int)strlen((char *)pRecordInfo->pString);
    }
    hdrLen += typeStringLen; /* Increment header length by the length of the type string. */

    /* Account 3 bytes more for long records. */
    hdrLen += (3 * (!shortRecord)); /* Long records needs 3 bytes more since the payload length field is 4 bytes. */

    /* Increment Message size by total header length and check if it exceeds either the shared memory or the message
     * processing buffer. */
    msgSize = pInst->msgSize + hdrLen;
    if ((msgSize > NFC_SHARED_MEM_BYTE_SIZE ) || (msgSize > pInst->bufLen)) {
        return false;
    }
    pInst->msgSize = msgSize;

    /* Get current position of message buffer pointer. */
    pCursor = pInst->pCursor;
    /* Reset payload length to zero as we are creating a new record. */
    pInst->len = 0;

    /* Preserve the latest record header location to be used to finalise the record header in NDEFT2T_CommitRecord()
     * function and also to set the Message begin bit in the last record of the message. */
    pInst->pLastRecordHdr = pCursor;
    pInst->shortRecord = shortRecord;

    /* Form record header byte. Message End bit is set in NDEFT2T_CommitMessage function. */
    *pCursor++ = (uint8_t)((pInst->msgBegin << 7) | (shortRecord << 4) | tnf);

    if (tnf == NDEFT2T_TNF_NFC_RTD) {
        *pCursor++ = 0x01; /* TYPE Length. */
        /* Preserving the pre-header length. For TEXT, this is the length of locale and the status byte. For URI, this
         * is the URI code. */
        *pCursor++ = (uint8_t)(typeStringLen + 1);
    }
    else {
        *pCursor++ = (uint8_t)typeStringLen; /* TYPE Length. */
        *pCursor++ = 0; /* Preserving the pre-header length which is zero. */
    }

    pCursor += 3 * (!shortRecord); /* payload Length is 4 bytes for normal records. */

    /* Assign type and any pre header status byte. */
    if (type == NDEFT2T_RECORD_TYPE_TEXT) {
        *pCursor++ = NDEFT2T_NFC_RTD_TEXT; /* Type. */
        *pCursor++ = typeStringLen & 0x3F; /* Status byte. */
    }
    else if (type == NDEFT2T_RECORD_TYPE_URI) {
        *pCursor++ = NDEFT2T_NFC_RTD_URI; /* Type. */
        *pCursor++ = (uint8_t)pRecordInfo->uriCode; /* URI code. */
    }

    /* Copy the type string. For TEXT records, locale string gets copied here and for URI nothing gets copied. */
    memcpy(pCursor, pRecordInfo->pString, (uint32_t)typeStringLen);
    pCursor += typeStringLen; /* Increment message buffer by length of the type string. */

    /* Preserve current message buffer position. */
    pInst->pCursor = pCursor;

    return true;
}

/**
 * This function decodes the NDEF TLV Header to find the length (L) of the NDEF Message and to locate the start
 * of the NDEF message payload (V).
 * @param [out] lenTlv : Length of the TLV.
 * @return Address of the starting byte of the NDEF Message payload. Returns @c NULL if the decoding failed.
 */
static uint8_t* DecodeNdefTlv(int *lenTlv)
{
    uint8_t *pMem = ((uint8_t*) NSS_NFC->BUF) + NDEFT2T_NDEF_TLV_START_OFFSET;
    int sharedMemSize = NFC_SHARED_MEM_BYTE_SIZE - NDEFT2T_NDEF_TLV_START_OFFSET;

    /* As per the standard (refer section 2.3 TLV blocks in NFCForum-TS-Type-2-Tag_1.2), NDEF TLV can come only after
     * Lock control TLV and Memory control TLV if they are present. There can also be NULL TLVs and Proprietary TLVs
     * before the NDEF TLV. So, we need to search and skip them first. However, the word offset of the NDEF TLV start
     * in this implementation is known in advance at page 6 as seen from RF side. So, we could have started the search
     * for NDEF TLV directly from there. However, this is not possible as we could encounter a corrupt message. An
     * intentional/unintentional overwrite of the Lock TLV and Proprietary TLV region using an external reader will
     * break this logic if we start searching directly from page 6 and a corrupted message may not get reported.
     */
    /* Search for the NDEF TLV. */
    while (sharedMemSize-- && (*pMem != TLV_NDEF)) {
        pMem++;
    }

    pMem++;
    if (!sharedMemSize--) {
        return NULL; /* Indicates a corrupt message */
    }

    /* Length of a TLV can be represented in a one byte format for a payload size less than 255 bytes. Otherwise, it
     * uses a 3 byte format where the first byte is 0xFF and this is followed by the upper byte and lower byte of the
     * length. So, extract the first byte after type to detect this. */
    *lenTlv = *pMem++;
    if (*lenTlv == NDEFT2T_NDEF_3BYTE_LEN_START) {
        if (sharedMemSize >= 2) {
            *lenTlv = (*pMem++) << 8;
            *lenTlv |= *pMem++;
        }
        else {
            return NULL; /* Indicates a corrupt message */
        }
    }
    return pMem;
}

/**
 * This function validates the message present in shared memory at the start of parsing.
 * @param pInstance : Base address of instance Buffer
 * @return true/false for success/failure of operation respectively.
 */
static bool ValidateNdefMsg(void *pInstance)
{
    NDEFT2T_INSTANCE_T *pInst = (NDEFT2T_INSTANCE_T *)pInstance;
    NDEFT2T_PARSE_RECORD_INFO_T recordInfo;
    uint8_t *pCursor;
    int msgSize;
    bool status = true;

    ASSERT((pInstance != NULL) && (pInst->pCursor !=NULL));

    /* Preserve current message size and message buffer location. */
    msgSize = pInst->msgSize;
    pCursor = pInst->pCursor;

    /* Continue in a loop till end of message Check and extract and verify all the records. */
    while (pInst->msgSize > 0) {
        status = NDEFT2T_GetNextRecord(pInstance, &recordInfo);
        if (true != status) break;
    }

    /* restore current message size and message buffer location. */
    pInst->msgSize = msgSize;
    pInst->pCursor = pCursor;
    return status;
}

#if NDEFT2T_EEPROM_COPY_SUPPPORT == 1
/**
 * This function does the copying of payload stored in EEPROM data area to the message buffer.
 * @param pDst : Destination pointer located in the message buffer
 * @param pSrc : Source pointer located in EEPROM
 * @param size : payload length
 */
static void CopyFromEeprom(uint8_t *pDst, const void * pSrc, int size)
{
    int offset;
    offset = ((int)pSrc - EEPROM_START);
    Chip_EEPROM_Read(NSS_EEPROM, offset, pDst, size);

}
#endif

/**
 * This function enables the Terminator TLV write detection, by enabling applicable interrupts.
 */
static void EnableTermTlvDetection(void)
{
    NFC_INT_T mask = Chip_NFC_Int_GetEnabledMask(NSS_NFC);
    Chip_NFC_Int_SetEnabledMask(NSS_NFC, mask | NFC_INT_MEMWRITE | NFC_INT_TARGETREAD);
}

/**
 * This function disables the Terminator TLV write detection, by disabling applicable interrupts and initializing the
 * required state variables.
 */
static void DisableTermTlvDetection(void)
{
    NFC_INT_T mask = Chip_NFC_Int_GetEnabledMask(NSS_NFC);
    Chip_NFC_Int_SetEnabledMask(NSS_NFC, mask & (~(NFC_INT_MEMWRITE | NFC_INT_TARGETREAD)));
    sTermTlvOffset = NDEFT2T_TERM_TLV_INIT_VAL;
}

static void DisableMessageReadDetection(void)
{
#ifdef NDEFT2T_MSG_READ_CB
    sAutomaticMode = false;
    sLastPageOfPayload = 0;
#endif
    /* Leave 'read-only mode. */

    /* Set the target page address used for NFC_INT_TARGETWRITE interrupt. When a new NDEF message is written, the type
     * T is written first, along with a length L set to 0. Then the value bytes V are written. When all the payload is
     * present, the length L is set correct. This means that the first page of the NDEF message is written twice: the
     * first time it indicates writing a new NDEF message has started; the second time it indicates writing a new NDEF
     * message has finished.
     * We monitor that page, for the terminator TLV detection and correction logic, and to call NDEFT2T_MSG_AVAILABLE_CB
     * when necessary.
     */
    Chip_NFC_SetTargetAddress(NSS_NFC, 2);

    /* Enable the applicable NFC interrupts and disable the rest. */
    Chip_NFC_Int_SetEnabledMask(NSS_NFC, NFC_INT_RFSELECT | NFC_INT_TARGETWRITE | NFC_INT_NFCOFF);
}

void NFC_IRQHandler(void)
{
    NFC_INT_T nfcRawInterruptStatus = Chip_NFC_Int_GetRawStatus(NSS_NFC);
#if defined(NDEFT2T_FIELD_STATUS_CB)
    NFC_STATUS_T status = Chip_NFC_GetStatus(NSS_NFC);
#endif
    NFC_INT_T nfcMaskedInterruptStatus = (NFC_INT_T)(NSS_NFC->MIS & NFC_INT_ALL);
    Chip_NFC_Int_ClearRawStatus(NSS_NFC, nfcRawInterruptStatus);
#ifdef NDEFT2T_MSG_READ_CB
    if (sAutomaticMode) {
        ReadOnly_IRQHandler(nfcMaskedInterruptStatus);
    }
#endif
    /* No } else {, as the boolean can be changed in the if-block, in which case we want to enter the block below too. */
#ifdef NDEFT2T_MSG_READ_CB
    if (!sAutomaticMode) {
#else
    {
#endif
        ReadWrite_IRQHandler(nfcMaskedInterruptStatus);
    }

#if defined(NDEFT2T_FIELD_STATUS_CB)
    extern void NDEFT2T_FIELD_STATUS_CB(bool isPresent);
    /* Translate the NFC_INT_NFCOFF (Indicating Field OFF) and NFC_INT_RFSELECT (Indicating Field ON) interrupts to a
     * boolean value to indicate the status of the NFC field. Care must be taken when the two interrupts are set
     * simultaneously. To take care of this, the NFC status register is checked additionally to see, if the last
     * RF operation was an RFSELECT.
     */
    static bool sIsOn = false;

    if (nfcRawInterruptStatus & NFC_INT_RFSELECT) {
        if (nfcRawInterruptStatus & NFC_INT_NFCOFF) {
            bool on = (status & NFC_STATUS_SEL) != 0;
            if (on && !sIsOn) {
                sIsOn = true;
                NDEFT2T_FIELD_STATUS_CB(true);
            }
            else if (!on && sIsOn) {
                sIsOn = false;
                NDEFT2T_FIELD_STATUS_CB(false);
            }
        }
        else {
            if (!sIsOn) {
                sIsOn = true;
                NDEFT2T_FIELD_STATUS_CB(true);
            }
        }
    }
    else if ((nfcRawInterruptStatus & NFC_INT_NFCOFF) && sIsOn) {
        sIsOn = false;
        NDEFT2T_FIELD_STATUS_CB(false);
    }
#endif
}

#if defined(NDEFT2T_MSG_READ_CB)
/**
 * The interrupt handling to be done when the tag reader has not yet written anything into the tag.
 * Assume a read-only tag reader for now.
 * @pre Called from #NFC_IRQHandler only
 */
static void ReadOnly_IRQHandler(NFC_INT_T nfcInterruptMaskedStatus)
{
    if (nfcInterruptMaskedStatus & NFC_INT_MEMWRITE) {
        DisableMessageReadDetection();
    }
    else {
        if (nfcInterruptMaskedStatus & NFC_INT_TARGETREAD) {
            if (sLastPageOfPayload > 6) {
                /* iPhones perform an NFC read as follows:
                 * - read pages 2 3 4 5 twice
                 * - read pages 4 5 6 7 thrice
                 * - read pages 8 9 a b
                 * - read pages c d e f
                 * - read pages ...
                 * - read pages w x y z once OR twice
                 * Last checked using iOS 12.1.4, using iPhone 8 MQ6G2ZD/A and iPhone 7 MN922ZD/A
                 *
                 * Notes:
                 * - the NHS31xx is compatible with the MiFare UltraLight EV1. A read is always a read of four pages in
                 *  sequence.
                 * - iOS does NOT read out the NFC ID (on pages 0 and 1).
                 * - Android phones do not perform a multiple readout of the last pages - but they behave less during
                 *  selection of the NFC tag.
                 *
                 * To know when a message has been read, and to avoid false positives due to the last pages being read
                 * out multiple times, we monitor two locations in sequence: the start of the NDEF message - page 6 -
                 * and the end of the NDEF message - page sLastPageOfPayload.
                 */
                Chip_NFC_SetTargetAddress(NSS_NFC, sLastPageOfPayload);
                sLastPageOfPayload = 0;
            }
            else {
                /* Disable NFC_INT_TARGETREAD. */
                Chip_NFC_Int_SetEnabledMask(NSS_NFC, NFC_INT_RFSELECT | NFC_INT_NFCOFF | NFC_INT_MEMWRITE);
                extern void NDEFT2T_MSG_READ_CB(void);
                NDEFT2T_MSG_READ_CB();
            }
        }
    }
}
#endif

/**
 * The interrupt handling to be done when the tag reader has demonstrated the capability to write into the tag.
 * @pre Called from #NFC_IRQHandler only
 */
static void ReadWrite_IRQHandler(NFC_INT_T nfcInterruptMaskedStatus)
{
#if defined(NDEFT2T_MSG_AVAILABLE_CB)
    bool msgAvailable = false;
#endif
    uint8_t *pV;
    int lenTlv;

    /* When a new NDEF message is written
     * - the type T is written first, along with a length L set to 0.
     * - Then the value bytes V are written.
     * - When all the payload is present, the length L is set correct.
     * See "Type 2 Tag Technical Specification, NDEF Write Procedure" on nfc-forum.org (version 1.0, 2017-08-28).
     * This means that the first page of the NDEF message is written twice: the first time it indicates writing
     * a new NDEF message has started; the second time it indicates writing a new NDEF message has finished.
     * That page is monitored using NFC_INT_TARGETWRITE.
     */
    if (nfcInterruptMaskedStatus & NFC_INT_TARGETWRITE) {
        DisableTermTlvDetection();
        uint32_t firstBytes = *((uint32_t *)NFC_SHARED_MEM_START + 2);
        /* NFC_INT_MEMWRITE may also be set at this point, but is redundant since the more specific NFC_INT_TARGETWRITE
         * is also fired. So this bit is cleared here.
         */
        nfcInterruptMaskedStatus &= ~NFC_INT_MEMWRITE;
        /* Check, if we have a valid NDEF message detected. A non-empty NDEF message will have the byte after the
         * NDEF TLV (which is 0x03) set to a non-zero value.
         */
        if ((firstBytes & 0x0000FFFF) != 0x03) {
            pV = DecodeNdefTlv(&lenTlv);
            if (pV != NULL) {
                /* Fetch and store the page offset to the terminator TLV. */
                sTermTlvOffset = (uint32_t)((uint32_t)pV - NFC_SHARED_MEM_START + (uint32_t)lenTlv);
#if defined(NDEFT2T_MSG_AVAILABLE_CB)
                /* The NFC shared memory contains a valid NDEF message. Notify the application at the end of the ISR. */
                msgAvailable = true;
#endif
            }
        }
    }

    /* The terminator TLV detection and correction logic is enabled inside the NDEFT2T_CommitMessage function just
     * before the message is written to the NFC shared memory if required - if a corruption can occur due to the tag
     * reader writing the last page containing the terminator TLV twice.
     * The detection is based on the occurrence of NFC_INT_MEMWRITE interrupt during or after writing the NDEF message
     * from the tag to the NFC shared memory. If it occurs, it indicates an extra terminator TLV write corrupting the
     * message from the tag. This corruption is then corrected by re-writing the previously preserved correct page
     * content to the page where this type if corruption can occur. Note that the location of the page write causing the
     * NFC_INT_MEMWRITE interrupt is not checked here: it is assumed that only a second terminator TLV write can happen
     * at this point in time.
     * Note: for more details on NDEF write procedure, refer to section 6.4.2 of the type 2 Tag specification.
     */
    if ((nfcInterruptMaskedStatus & NFC_INT_MEMWRITE) && (sTermTlvOffset != NDEFT2T_TERM_TLV_INIT_VAL)) {
        *((uint32_t*)NFC_SHARED_MEM_START + (sTermTlvOffset / 4)) = sTermTlvPage;
    }

    /* The terminator TLV detection and correction logic is disabled when one of NFC_INT_NFCOFF, NFC_INT_RFSELECT
     * or NFC_INT_TARGETREAD is received.
     */
    if ((nfcInterruptMaskedStatus & NFC_INT_NFCOFF) || (nfcInterruptMaskedStatus & NFC_INT_RFSELECT)
            || (nfcInterruptMaskedStatus & NFC_INT_TARGETREAD)) {
        DisableTermTlvDetection();
    }

#if defined(NDEFT2T_MSG_AVAILABLE_CB)
    if (msgAvailable) {
        /* Notify the application about the presence of a new valid NDEF message in the NFC shared memory. */
        extern void NDEFT2T_MSG_AVAILABLE_CB(void);
        NDEFT2T_MSG_AVAILABLE_CB();
    }
#endif
}
