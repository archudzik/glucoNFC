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

#ifndef __NDEFT2T_H_
#define __NDEFT2T_H_

#include "chip.h"
#include "ndeft2t/ndeft2t_dft.h"

/**
 * @defgroup MODS_NSS_NDEFT2T ndeft2t: NDEF Type-2 Tag Message creator/parser Module
 * @ingroup MODS_NSS
 * NDEFT2T module provides the API to handle creation and parsing of NDEF messages on a Type 2 Tag. The record types
 * which are currently supported are NFC Forum well known types TEXT and URI, NFC Forum External types and MIME.
 *
 * The NDEFT2T module provides the following functionalities:
 * - Creation of an NDEF message consisting of one or more supported record types.
 * - Parsing of an NDEF message and extracting the supported record types.
 *
 * @par NDEF:
 *  The NFC Data Exchange Format (NDEF) specification defines a message encapsulation format to exchange information,
 *  e.g. between an NFC Forum Device and another NFC Forum Device or an NFC Forum Tag. NDEF is a lightweight, binary
 *  message format that can be used to encapsulate one or more application-defined payloads of arbitrary type and size
 *  into a single message. More details of this format can be referred from the NFC Forum specification available at
 *  http://members.nfc-forum.org/specs/spec_list/.
 *
 * @par Memory Requirements:
 *  The NDEFT2T module does not allocate any static R/W data memory. Instead, the caller should take care of the
 *  allocation of the memory required for the creation/parsing of the NDEF message. The NDEFT2T module memory
 *  requirements are contributed from the entities given below:
 *  - Instance Buffer: The instance buffer preserves the necessary housekeeping information during an instantiation
 *     of the NDEFT2T module. A new instantiation is made either when #NDEFT2T_CreateMessage() or
 *     #NDEFT2T_GetMessage() is called. The caller must ensure that the argument pInstance passed to these
 *     functions points to a buffer of size #NDEFT2T_INSTANCE_SIZE bytes. The caller must also ensure that
 *     the memory allocated for this buffer starts on a 32-bit aligned address in RAM.
 *  - Message Buffer:  NDEFT2T module uses this buffer to initially create the NDEF message consisting of one or
 *     more records before finally copying the same to the shared memory. Similarly, during parsing, the entire
 *     message is initially copied to the message buffer and the constituent records are then parsed from that. The
 *     caller must ensure that the variable pBuffer passed when calling the functions #NDEFT2T_CreateMessage() or
 *     #NDEFT2T_GetMessage() has sufficient space to create/parse the entire NDEF message. However, it is safe to
 *     allocate a size of #NFC_SHARED_MEM_BYTE_SIZE (which stands for the size of the shared memory) in case the
 *     caller is not sure of the size of the NDEF message being created or parsed. The caller must also ensure that
 *     the memory allocated for this buffer starts on a 32-bit aligned address in RAM and has a size that is a
 *     multiple of 4.
 *  - Record information structure: The exchange of type information (main type and extended properties) for the
 *     record being created or parsed is achieved through the data structures #NDEFT2T_CREATE_RECORD_INFO_T and
 *     #NDEFT2T_PARSE_RECORD_INFO_T respectively. In the case of message creation, the caller must allocate and
 *     fill in the relevant fields of #NDEFT2T_CREATE_RECORD_INFO_T to input the information of the record to
 *     create. Similarly, in the case of parsing, the caller must allocate NDEFT2T_PARSE_RECORD_INFO_T and process
 *     the information received about the record getting parsed.
 *
 * @par Initialization/De-initialization:
 *  The module has to be initialized before a message creation or parsing, at the start of application processing.
 *  This has to be done after initializing the NFC HW block (see #Chip_NFC_Init) by calling #NDEFT2T_Init.
 *  Similarly, the module can be de-initialized by calling #NDEFT2T_DeInit followed by de-initialization of the
 *  NFC HW block (see Chip_NFC_DeInit) after the application processing is complete. Note that the
 *  Initialization/de-initialization of the NFC HW block is not done inside the module and hence has to be done
 *  at application level in the order mentioned above.
 *
 * @par NDEF Message creation:
 *  The below steps outline the NDEF message creation:
 *  - Step1: Call #NDEFT2T_CreateMessage to instantiate a new NDEF message and prepare for adding constituent
 *      records into it. The caller must ensure to allocate sufficient memory for the arguments pInstance and
 *      pBuffer before calling the above function.
 *  - Step2: Create one or more records in the instantiated NDEF message by following the procedure listed below
 *      till all records have been created.
 *      - Create record: Call one of the functions #NDEFT2T_CreateTextRecord, #NDEFT2T_CreateUriRecord
 *          #NDEFT2T_CreateExtRecord or #NDEFT2T_CreateMimeRecord for creating NFC Forum well known types TEXT and URI,
 *          NFC Forum External types or MIME.
 *          The caller must ensure to initialize relevant fields of the record information argument pRecordInfo
 *          before calling the above functions.
 *      - Copy record payload: Call the function #NDEFT2T_WriteRecordPayload to copy the record payload to the
 *          message buffer.
 *      - Finalize record: Call the function #NDEFT2T_CommitRecord to finalize the record.
 *  - Step3: Call function #NDEFT2T_CommitMessage to finalize the NDEF message. The user cannot add any more
 *      records after this. The finalised message is copied to the shared memory at this stage.
 *
 * @par NDEF Message Parsing:
 *  The below steps outline the NDEF message parsing:
 *  - Step1: Call function #NDEFT2T_GetMessage to copy the NDEF message from shared memory into the message buffer.
 *      The caller must ensure to allocate sufficient memory for the arguments pInstance and pBuffer before calling
 *      the above function.
 *  - Step2: Call the function #NDEFT2T_GetNextRecord to search and retrieve the next record present in the NDEF
 *      message. The caller can then check the returned record information from the argument pRecordInfo and decide
 *      whether this record type is relevant for the application scenario.
 *  - Step3: Call the function #NDEFT2T_GetRecordPayload to retrieve record payload.
 *  Continue steps 2 and 3 above till all the relevant records have been retrieved or till end of message.
 *
 * @anchor nfcIntHandling_anchor
 * @par NFC Interrupt Handling:
 *  This module provides an implementation of the interrupt vector #NFC_IRQHandler and controls the interrupt #NFC_IRQn.
 *  By including this module, the application can no longer instantiate the NFC interrupt handler and may no longer
 *  enable, disable or clear the corresponding interrupts.
 *  The interrupts which are enabled all the time are #NFC_INT_RFSELECT, #NFC_INT_TARGETWRITE and #NFC_INT_NFCOFF
 *  interrupts. However, #NFC_INT_MEMWRITE and #NFC_INT_TARGETREAD interrupts are also enabled/disabled internally by
 *  the module during NDEF message creation.
 *  Two callback functions are provided to the application, of types #pNdeft2t_FieldStatus_Cb_t and
 *  #pNdeft2t_MsgAvailable_Cb_t. These are called from the interrupt handler.
 *  - The callback function of type #pNdeft2t_FieldStatus_Cb_t provides the current status of the NFC field (ON or OFF)
 *      as obtained inside the interrupt handler.
 *  - The callback function of type #pNdeft2t_MsgAvailable_Cb_t gets fired once per message, indicating the presence of
 *      a valid NDEF message in the shared memory. This can be used as a trigger for the application to start the
 *      parsing of a new NDEF message.
 *  The application has to implement these callbacks and enable them by using the respective diversity settings
 *  #NDEFT2T_FIELD_STATUS_CB and #NDEFT2T_MSG_AVAILABLE_CB.
 *
 * @note The #NFC_INT_TARGETWRITE and #NFC_INT_TARGETREAD interrupts are internally configured by the module to occur
 *  for page 6 of the NFC Tag memory as seen from RF side or page 2 of the NFC shared memory.
 *
 * @anchor colDetDesc_anchor
 * @par Shared memory access collision detection:
 *  This feature allows detecting a collision in case of a simultaneous read/write access from both the APB-side and
 *  RF-side from/to the shared memory. The data that is getting read from the APB-side can get corrupted due to a
 *  simultaneous data write from the RF side. In a similar way, the data that is getting written from the APB-side
 *  can also get corrupted. The collision detection feature allows detecting such a corruption and the read or write
 *  attempt can be retried for a specified number of times by using respective diversity settings (See
 *  @ref MODS_NSS_NDEFT2T_DFT).
 *
 * @par Record and Message Header overheads
 *  An NDEF message has few record and message header bytes in addition to the record payloads. The overhead in bytes
 *  required for these header bytes can be obtained using the macros #NDEFT2T_TEXT_RECORD_OVERHEAD,
 *  #NDEFT2T_MIME_RECORD_OVERHEAD, #NDEFT2T_EXT_RECORD_OVERHEAD, #NDEFT2T_URI_RECORD_OVERHEAD and
 *  #NDEFT2T_MSG_OVERHEAD. Knowing the overhead will enable the application writer to know the bytes left in the
 *  shared memory for the record payload. An example usage is given below for a message with a TEXT and MIME record each
 *  being a short record and the total message payload size being less than 255 bytes:
 *  - overhead = NDEFT2T_MSG_OVERHEAD(true, (NDEFT2T_TEXT_RECORD_OVERHEAD(true, sizeof("en")-1) +
 *                                           NDEFT2T_MIME_RECORD_OVERHEAD(true, sizeof("nhs31xx/demo.nhs.log")-1));
 *
 * @par Diversity
 *  This module supports diversity, like enabling/disabling collision detection and also to specify the number of
 *  read/write retries in case of a collision.
 *  Check @ref MODS_NSS_NDEFT2T_DFT for all diversity parameters.
 *
 * @par Example1 - Creating an NDEF Message with a single record of type TEXT
 *  NDEF messages consisting of a single record of types MIME, AAR and URI can be created in a similar way as given in
 *  this example.
 *  @snippet ndeft2t_mod_example_1.c ndeft2t_mod_example_1
 *
 * @par Example2 - Creating an NDEF Message with one TEXT record and one MIME record
 *  @snippet ndeft2t_mod_example_2.c ndeft2t_mod_example_2
 *
 * @par Example3 - Parsing of an NDEF Message with a single record of type TEXT
 *  @snippet ndeft2t_mod_example_3.c ndeft2t_mod_example_3
 *
 * @{
 */

/** Size of Instance buffer required by the NDEFT2T module for internal housekeeping. */
#define NDEFT2T_INSTANCE_SIZE 24

/**
 * Calculates the overhead in bytes required for a TEXT record header.
 * @param shortRecord : Set this to @c true if the payload size is known to be <= 255 bytes. Use @c false if it is
 *  bigger, unknown or to get the largest overhead in bytes.
 * @param localeLength : This is the length of the locale string @b without the @c NULL character as given in
 *  #NDEFT2T_CREATE_RECORD_INFO_T.pString.
 */
#define NDEFT2T_TEXT_RECORD_OVERHEAD(shortRecord, localeLength) (4 + ((shortRecord) ? 1 : 4) + (localeLength))

/**
 * Calculates the overhead in bytes required for a MIME record header.
 * @param shortRecord : Set this to @c true if the payload size is known to be <= 255 bytes. Use @c false if it is
 *  bigger, unknown or to get the largest overhead in bytes.
 * @param typeLength : This is the length of the type string @b without the @c NULL character as given in
 *  #NDEFT2T_CREATE_RECORD_INFO_T.pString.
 */
#define NDEFT2T_MIME_RECORD_OVERHEAD(shortRecord, typeLength) (2 + ((shortRecord) ? 1 : 4) + (typeLength))

/**
 * Calculates the overhead in bytes required for a NFC Forum external record header.
 * @param shortRecord : Set this to @c true if the payload size is known to be <= 255 bytes. Use @c false if it is
 *  bigger, unknown or to get the largest overhead in bytes.
 * @param typeLength : This is the length of the payload type string @b without the @c NULL character as given in
 *  #NDEFT2T_CREATE_RECORD_INFO_T.pString.
 */
#define NDEFT2T_EXT_RECORD_OVERHEAD(shortRecord, typeLength) (2 + ((shortRecord) ? 1 : 4) + (typeLength))

/**
 * Calculates the overhead in bytes required for a URI record header.
 * @param shortRecord : Set this to @c true if the payload size is known to be <= 255 bytes. Use @c false if it is
 *  bigger, unknown or to get the largest overhead in bytes.
 */
#define NDEFT2T_URI_RECORD_OVERHEAD(shortRecord) (4 + ((shortRecord) ? 1 : 4))

/**
 * Calculates the total overhead in bytes required to create an NDEF message.
 * @param shortMessage : Set this to @c true if the total message payload size (sum of all the individual record
 *  payloads and record overheads) is known to be <= 254 bytes. Use @c false if it is bigger, unknown or to get the
 *  largest overhead in bytes.
 * @param totalRecordOverhead : This is the total record overhead (sum of all the individual record overheads).
 * @note The limit <= 254 message payload size is as per the standard and is different from the limit used for record
 *  payload.
 */
#define NDEFT2T_MSG_OVERHEAD(shortMessage, totalRecordOverhead) (8 + 2 + ((shortMessage) ? 1 : 3) + (totalRecordOverhead))

/* ------------------------------------------------------------------------- */

/** Supported record Types. */
typedef enum NDEFT2T_RECORD_TYPE {
    NDEFT2T_RECORD_TYPE_EMPTY = 0x00, /*!< Empty Record. */
    NDEFT2T_RECORD_TYPE_UNKNOWN, /*!< Unknown. */
    NDEFT2T_RECORD_TYPE_UNCHANGED, /*!< Unchanged. */
    NDEFT2T_RECORD_TYPE_TEXT, /*!< TEXT Record. */
    NDEFT2T_RECORD_TYPE_MIME, /*!< MIME Record. */
    NDEFT2T_RECORD_TYPE_EXT, /*!< NFC Forum external type Record. */
    NDEFT2T_RECORD_TYPE_URI, /*!< URI Record. */
    NDEFT2T_RECORD_TYPE_PHDC, /*!< PHDC Record. */
    NDEFT2T_RECORD_TYPE_RESERVED /*!< Reserved. */
} NDEFT2T_RECORD_TYPE_T;

/** Record information data structure to be used for Creation */
typedef struct {
    /**
     * This field is used to pass the payload type string as given below. The caller shall ensure NULL termination of
     * this string.
     *  - For a TEXT record, locale string such as 'en', 'fr' to be provided here.
     *  - For NFC Forum external Type record (Eg. AAR), payload type to be provided here.
     *  - For MIME type record: MIME type string to be provided here.
     *  - Ignored for URI records.
     */
    uint8_t *pString;

    bool shortRecord; /*!< Set to @c true to use short records, @c false otherwise. */

    /**
     * URI identifier code. Applicable only when type equals #NDEFT2T_RECORD_TYPE_URI. A full list is available in the
     * technical specification from NFC Forum: NFC URI Record Type Definition Technical Specification.pdf
     * "URI Record Type Definition Technical Specification, RTD-URI 1.0, 2006-07-24, 3.2.2 URI Identifier Code, table 3".
     * The most common ones are:
     * - 0x00 - No prepending is done, and the URI field contains the unabridged URI.
     * - 0x01 - "http://www."
     * - 0x02 - "https://www."
     * - 0x03 - "http://"
     * - 0x04 - "https://"
     * - 0x06 - "mailto:"
     * - 0x07 - "ftp://anonymous:anonymous@"
     * - 0x08 - "ftp://ftp."
     * - 0x09 - "ftps://"
     * - 0x0A - "sftp://"
     */
    uint32_t uriCode;
} NDEFT2T_CREATE_RECORD_INFO_T;

/**
 * Record information data structure to be used for Parsing
 * @note: NDEFT2T module supports extraction of record information for only MIME and TEXT type records. Extraction of
 *  all record information fields for URI and AAR is not supported.
 */
typedef struct {
    NDEFT2T_RECORD_TYPE_T type; /*!< Type of record. */

    /** This field is used to retrieve the payload type string as given below.\n
                          --For a TEXT record, locale string such as 'en', 'fr' can be retrieved from this.\n
                          --For MIME type record: MIME type string can be retrieved from this. */
    uint8_t *pString;

    int stringLength; /*!< Length of type string defined above. */

    /**
     * When set to @c true the current record is a chunk. When set to @c false, the current record is the last chunk
     * of the record, provided the previous record was a chunk. This field will remain @c false when records are not
     * chunked.
     */
    bool chunked;
} NDEFT2T_PARSE_RECORD_INFO_T;

/* ------------------------------------------------------------------------- */

/**
 * Callback function type to chain interrupt status from ISR to application. Refer @ref nfcIntHandling_anchor
 * "NFC Interrupt Handling" for more details.
 * @param isPresent : NFC interrupt status obtained inside NDEFT2T MOD.
 */
typedef void (*pNdeft2t_FieldStatus_Cb_t)(bool isPresent);

/**
 * Callback function type to get indicated on the presence of a valid NDEF message in shared memory. This will occur
 * once per message. Refer @ref nfcIntHandling_anchor "NFC Interrupt Handling" for more details.
 */
typedef void (*pNdeft2t_MsgAvailable_Cb_t)(void);

#ifdef NDEFT2T_MSG_READ_CB
/**
 * Callback function type to get notified when the NFC tag reader has finished reading the NDEF message written by the
 * firmware in the NFC shared memory.
 * @note Called under interrupt.
 */
typedef void (*pNdeft2t_MsgRead_Cb_t)(void);
#endif

/* ------------------------------------------------------------------------- */

/**
 * This function initializes the NDEFT2T module.
 * @pre The NFC HW block has been initialized by calling #Chip_NFC_Init
 * @note During initialization, #NDEFT2T_ResetNfcMemory is executed as well.
 */
void NDEFT2T_Init(void);

/**
 * This function De-initializes the NDEFT2T module.
 * @post De-initialize the NFC HW block by calling #Chip_NFC_DeInit
 */
void NDEFT2T_DeInit(void);

/**
 * (Re-)initialize the NFC shared memory. This allows a phone to recognize the IC as an NDEF object, not just as an
 * NdefFormatable, even if contents are not changed afterwards.
 * It also blocks both tag (via #NDEFT2T_GetMessage) and tag reader from reading what data may be present thereafter.
 */
void NDEFT2T_ResetNfcMemory(void);

#ifdef NDEFT2T_MSG_READ_CB
/**
 * Enables detection of when a message is fully read by a tag reader. It will monitor detection of the first page where
 * the user NDEF message can be written to, and the last page of the NDEF message.
 * #NDEFT2T_MSG_READ_CB is called each time the tag reader has read the complete NDEF message.
 * @param lastPageOfMessage The page number where the last payload can be found. Use this when a message has been
 *  pre-populated in the NFC memory (after calling #NDEFT2T_Init since this will also re-initialize the NFC memory
 *  contents). When @c 0 is provided, the read detection will only start once a new NDEF message is committed
 *  by calling #NDEFT2T_CommitMessage
 * @note This can be used by the application to enter an 'automatic mode' until the NFC tag reader writes any page. The
 *  application can send out subsequent messages without waiting for an explicit command, increasing throughput.
 *  As long as the tag reader does not write, the tag is in control and decides which information to provide.
 * @post When this call ends, all @em new messages committed will be monitored for reading by the tag reader. The last
 *  page of the message is calculated and the detection mechanism will be reconfigured during the call to
 *  #NDEFT2T_CommitMessage
 * @post Once entered, the mode is left:
 *  - when the tag reader writes in the NFC shared memory - generating the #NFC_INT_MEMWRITE interrupt
 *  - when #NDEFT2T_Init is called
 *  - when #NDEFT2T_DisableMessageReadDetection is called
 */
void NDEFT2T_EnableMessageReadDetection(unsigned int lastPageOfMessage);

/** @warning Its use is discouraged. Present only for backward compatibility. Will be removed in a later SDK. */
#define NDEFT2T_EnableAutomaticMode() NDEFT2T_EnableMessageReadDetection(0)

/**
 * Disables detection of when a message is fully read by a tag reader.
 */
void NDEFT2T_DisableMessageReadDetection(void);
#endif

/**
 * This function starts the process of creating an NDEF message and prepares for addition of one or more records into
 * the message. A call to this function makes a new instantiation of the NDEFT2T module for message creation.
 * @param pInstance : Base address of instance Buffer. The instance buffer preserves the necessary housekeeping
 *                    information during an instantiation of the NDEFT2T module. The caller must ensure that the
 *                    argument pInstance points to a buffer of size #NDEFT2T_INSTANCE_SIZE bytes.
 * @param pBuffer : Base address of message buffer used for creation of the message. The NDEF message is
 *                  initially created in this buffer and the completed message is finally copied to the shared memory
 *                  at the end of message creation by calling #NDEFT2T_CommitMessage(). The caller must allocate
 *                  sufficient memory for this buffer. Refer section on "Memory Requirements" for more details.
 * @param bufLen : Length of the message buffer as explained above.
 * @param shortMessage : Set this to @c true if the length of the message payload is <= 254 bytes or not known,
 *  else set to @c false. See also #NDEFT2T_MESSAGE_HEADER_LENGTH_CORRECTION.
 * @note The NFC shared memory is not changed.
 */
void NDEFT2T_CreateMessage(void *pInstance, uint8_t *pBuffer, int bufLen, bool shortMessage);

/**
 * This function creates a TEXT type record. The function will reserve space for the record header, fill known values
 * to the record header and initialize related instance variables. The function has to be called after calling
 * #NDEFT2T_CreateMessage(). Call to this function has to be followed by copying the actual payload to the message
 * buffer and then by finalizing the record header. Copying of payload to the message buffer has to be done by calling
 * #NDEFT2T_WriteRecordPayload() and finalizing of the header by calling #NDEFT2T_CommitRecord() .
 * @param pInstance : Base address of instance Buffer
 * @param pRecordInfo : Base address of the record type information data structure instantiated in application. The
 *                       caller has to initialize the NDEFT2T_CREATE_RECORD_INFO_T structure field pString with locale
 *                       and set or clear shortRecord based on the need to use short records. Refer to
 *                       #NDEFT2T_CREATE_RECORD_INFO_T data structure for more details.
 * @return true/false for success/failure of operation respectively. The function returns false under the below
 *         scenarios.
 *          -# Size of the NDEF message being created exceeds the size of message buffer allocated by caller
 *          -# Size of the NDEF message being created exceeds the size of the shared memory
 * @note The NFC shared memory is not changed. Only the internal buffer as given in the call to #NDEFT2T_CreateMessage
 *  is written to.
 */
bool NDEFT2T_CreateTextRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo);

/**
 * This function creates an NFC Forum external type record. The function will reserve space for the record header, fill
 * known values to the record header and initialize related instance variables. The function has to be called after
 * calling #NDEFT2T_CreateMessage(). Call to this function has to be followed by copying the actual payload to the
 * message buffer and then by finalizing the record header. Copying of payload to the message buffer has to be done by
 * calling #NDEFT2T_WriteRecordPayload() and finalizing of the header by calling #NDEFT2T_CommitRecord().
 * @param pInstance : Base address of instance Buffer
 * @param pRecordInfo : Base address of the record type information data structure instantiated in application. The
 *                       caller has to initialize the NDEFT2T_CREATE_RECORD_INFO_T structure field pString with
 *                       "android.com:pkg" and set or clear shortRecord based on the need to use short records. Refer to
 *                       #NDEFT2T_CREATE_RECORD_INFO_T data structure for more details.
 * @return true/false for success/failure of operation respectively.
 *         The function returns false under the below scenarios.
 *          -# Size of the NDEF message being created exceeds the size of message buffer allocated by caller
 *          -# Size of the NDEF message being created exceeds the size of the shared memory
 * @note The NFC shared memory is not changed. Only the internal buffer as given in the call to #NDEFT2T_CreateMessage
 *  is written to.
 */
bool NDEFT2T_CreateExtRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo);

/**
 * This function creates a MIME type record. The function will reserve space for the record header, fill known values
 * to the record header and initialize related instance variables. The function has to be called after calling
 * #NDEFT2T_CreateMessage(). Call to this function has to be followed by copying the actual payload to the message
 * buffer and then by finalizing the record header. Copying of payload to the message buffer has to be done by calling
 * #NDEFT2T_WriteRecordPayload() and finalizing of the header by calling #NDEFT2T_CommitRecord() .
 * @param pInstance : Base address of instance Buffer
 * @param pRecordInfo : Base address of the record type information data structure instantiated in application. The
 *                      caller has to initialize the NDEFT2T_CREATE_RECORD_INFO_T structure field pString with MIME
 *                      payload type and set or clear the field shortRecord based on the need to use short records.
 *                      Refer to #NDEFT2T_CREATE_RECORD_INFO_T data structure for more details.
 * @return true/false for success/failure of operation respectively.
 *         The function returns false under the below scenarios.
 *          -# Size of the NDEF message being created exceeds the size of message buffer allocated by caller
 *          -# Size of the NDEF message being created exceeds the size of the shared memory
 * @note The NFC shared memory is not changed. Only the internal buffer as given in the call to #NDEFT2T_CreateMessage
 *  is written to.
 */
bool NDEFT2T_CreateMimeRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo);

/**
 * This function creates a URI type record. The function will reserve space for the record header, fill known values
 * to the record header and initialize related instance variables. The function has to be called after calling
 * #NDEFT2T_CreateMessage(). Call to this function has to be followed by copying the actual payload to the message
 * buffer and then by finalizing the record header. Copying of payload to the message buffer has to be done by calling
 * #NDEFT2T_WriteRecordPayload() and finalizing of the header by calling #NDEFT2T_CommitRecord() .
 * @param pInstance : Base address of instance Buffer
 * @param pRecordInfo : Base address of the record type information data structure instantiated in application. The
 *                      caller has to initialize the NDEFT2T_CREATE_RECORD_INFO_T structure field uriCode and set or
 *                      clear shortRecord based on the need to use short records. The pString field is not applicable for
 *                      URI record. Refer to #NDEFT2T_CREATE_RECORD_INFO_T data structure for more details.
 * @return true/false for success/failure of operation respectively.
 *         The function returns false under the below scenarios.
 *          -# Size of the NDEF message being created exceeds the size of message buffer allocated by caller
 *          -# Size of the NDEF message being created exceeds the size of the shared memory
 *          -# The NDEFT2T_CREATE_RECORD_INFO_T data structure field uriCode is not valid
 * @note The NFC shared memory is not changed. Only the internal buffer as given in the call to #NDEFT2T_CreateMessage
 *  is written to.
 */
bool NDEFT2T_CreateUriRecord(void *pInstance, const NDEFT2T_CREATE_RECORD_INFO_T *pRecordInfo);

/**
 * This function appends data to the payload of the record that was previously created. The application can copy the
 * entire data at once, if the same is available immediately. If the entire data is not available immediately, then the
 * same can be copied in parts by calling this function multiple times. The function will keep track of the total size
 * of the payload that gets copied in this manner in its instance variables.
 * @param pInstance : Base address of instance Buffer
 * @param pData : Base address of the record payload.
 * @param size : Length in bytes of the record payload.
 * @return true/false for success/failure of operation respectively.
 *         The function returns false under the below scenarios.
 *          -# Size of the NDEF message being created exceeds the size of message buffer allocated by caller
 *          -# Size of the NDEF message being created exceeds the size of the shared memory
 *          -# The NDEFT2T_CREATE_RECORD_INFO_T data structure field shortRecord is set and payload data size exceeds
 *             255 bytes.
 * @note The NFC shared memory is not changed. Only the internal buffer as given in the call to #NDEFT2T_CreateMessage
 *  is written to.
 * @note When #NDEFT2T_EEPROM_COPY_SUPPPORT is set to '1', pData can be located in EEPROM read/write region. In that
 *  case, @b address must be given, @b not the offset. For example, provide #EEPROM_START to copy the data from the
 *  start of the EEPROM memory. The function will then internally take care of the copying from EEPROM to the message
 *  buffer. However, the EEPROM must already be initialized before calling this function. This is @b not checked for,
 *  and will result in a hang if this precondition is not met.
 */
bool NDEFT2T_WriteRecordPayload(void *pInstance, const void * pData, int size);

/**
 * This function finalizes the record header and has to be called after the caller has copied the payload.
 * @param pInstance : Base address of instance Buffer
 * @note The NFC shared memory is not changed. Only the internal buffer as given in the call to #NDEFT2T_CreateMessage
 *  is written to.
 */
void NDEFT2T_CommitRecord(void *pInstance);

/**
 * This function finalizes the NDEF message header. The function has to be called at the end of an NDEF message
 * creation after creating all records.
 * @param pInstance : Base address of instance Buffer
 * @return @c true
 * @note The NFC shared memory is updated by copying the contents from the internal buffer (as given in the call to
 *  #NDEFT2T_CreateMessage).
 */
bool NDEFT2T_CommitMessage(void *pInstance);

/**
 * This function starts the process of parsing an NDEF message present in shared memory. A call to this function makes
 * a new instantiation of the NDEFT2T module for message parsing.
 * @param pInstance : Base address of instance Buffer. The instance buffer preserves the necessary housekeeping
 *                    information during an instantiation of the NDEFT2T module. The caller must ensure that the
 *                    argument pInstance points to a buffer of size #NDEFT2T_INSTANCE_SIZE bytes.
 * @param pBuffer : Base address of message buffer used for parsing of the message. NDEF message is first copied
 *                  into this buffer from the shared memory. Further parsing of constituent records is then done on
 *                  this buffer. This function also validates the message for correctness of header fields. The
 *                  caller must allocate sufficient memory for this buffer. Refer section on "Memory Requirements"
 *                  for more details.
 * @param bufLen : Length of the message buffer as explained above.
 * @return true/false for success/failure of operation respectively.
 *         The function returns false under the below scenarios.
 *          -# Size of the NDEF message being parsed exceeds the size of message buffer allocated by caller or the size
 *              of the NFC shared memory (#NFC_SHARED_MEM_BYTE_SIZE).
 *          -# Shared memory contains non-NDEF formatted data or an invalid record header field.
 *          -# Unsupported record types or payload types
 */
bool NDEFT2T_GetMessage(void *pInstance, uint8_t *pBuffer, int bufLen);

/**
 * This function parses the NDEF message and retrieves the type and all related information of the next record present
 * in the message. The function has to be called after calling the #NDEFT2T_GetMessage() function. Each call to this
 * function will advance the internal message buffer pointer to the start of the next record after the one that got
 * parsed.
 * @param pInstance : Base address of instance Buffer
 * @param [out] pRecordInfo : Base address of the record type information data structure. The function will
 *  populate the applicable fields of this data structure after parsing the next NDEF record present in the message.
 * @return true/false for success/failure of operation respectively.
 *  The function returns false under the below scenarios.
 *  - End of message reached
 */
bool NDEFT2T_GetNextRecord(void *pInstance, NDEFT2T_PARSE_RECORD_INFO_T *pRecordInfo);

/**
 * This function provides the address of the message buffer location from where the record payload can be retrieved
 * and the length of the payload data as well. The function has to be called after calling the #NDEFT2T_GetNextRecord()
 * function during message parsing. The function can also be used during creation of NDEF message in case the
 * application needs to preserve the buffer location of a particular record payload.
 * @param pInstance : Base address of instance Buffer
 * @param [out] pLen : Length of the record payload can be retrieved via this output parameter. A payload length of
 *        @c 0 means that either the current record being parsed is an empty record or that this function was called
 *        before calling #NDEFT2T_GetNextRecord() function.
 * @return Address in message buffer where the record payload starts. A value of NULL being returned indicates that
 *         either the current record being parsed is an empty record or that this function was called before calling
 *         #NDEFT2T_GetNextRecord() function.
 */
void* NDEFT2T_GetRecordPayload(void *pInstance, int *pLen);

#endif /** @} */
