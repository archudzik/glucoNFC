/*
 * Copyright 2014-2016,2019-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __NFC_NSS_H_
#define __NFC_NSS_H_

/**
 * @defgroup NFC_NSS NFC: Near field communication driver
 * @ingroup DRV_NSS
 * Near Field Communication (NFC) driver provides the API to control the functionalities handled by the NFC HW block.
 * The NFC driver provides the following functionalities:
 *  -# Initializing/Deinitializing of the NFC HW block
 *  -# Enabling/Disabling and clearing of the Interrupts
 *  -# Retrieving the NFC HW block status bits
 *  -# Retrieving information about last RF access
 *
 * @par NFC HW Interface:
 *  The NFC HW block is based on a MIFARE Ultralight EV1 block which is responsible for the wireless NFC communication
 *  with an external reader. Detailed functionality of the Mifare Ultralight EV1 can be referred from the user manual.
 *
 * @par Memory organization and access:
 *  The IC has a shared memory region which is overlaid on the EEPROM area of the MIFARE Ultralight EV1 block. The NFC
 *  shared memory can be accessed by the ARM core over the APB bus and also by an external contact-less reader. The
 *  access by the ARM core will henceforth be referred to as "APB side access" and the access by an external reader as
 *  "RF side access". The exchange of data over the RF from an external reader and the MIFARE Ultralight EV1 block is
 *  in units of pages which is of 4 bytes in size. The memory organization of the MIFARE Ultralight EV1 block is also
 *  in the same manner. The first 4 pages is an EEPROM block and that can only be accessed from the RF side and not
 *  from the APB side. The next 128 pages is the shared memory and that can be accessed from both the RF side as well
 *  as the APB side. This shared memory space is followed by 3 special registers which are status register, incoming
 *  command register and outgoing command register. The rest of the EEPROM area follows this and is accessible from
 *  RF side. However, access to this area is not recommended.
 *
 * @note The driver does not provide functions to access the shared memory and direct access has to be used by the
 *  higher layers for this. The shared memory access can be referred from example 1 and example 2 given below.
 *
 * @warning The shared memory is writable only as 32-bit words to 32-bit aligned addresses from the APB side.
 *  Reading can be done as a byte or a 16-bit word or a 32-bit word from 32-bit aligned address space.
 *
 * @par Incoming/Outgoing Command registers:
 *  These are the registers #NSS_NFC_T.CMDIN and #NSS_NFC_T.DATAOUT listed below register structure. These registers
 *  could be used by applications to exchange custom commands between the SW running on the ARM core and an external
 *  NFC Reader. The NFC driver does not provide functions to access these registers. The #NSS_NFC_T.CMDIN register
 *  is writable from the RF side and only readable from the APB side. Similarly the #NSS_NFC_T.DATAOUT register is
 *  writable from the APB side and only readable from the RF side.
 *
 * @par Interrupt Handling:
 *  This block has multiple interrupt sources tied to the same NFC IRQ. Refer to #NFC_INT_T for more details.@n
 *  The interrupts can be enabled/disabled by using the #Chip_NFC_Int_SetEnabledMask function. The interrupt flags can
 *  be selectively cleared using #Chip_NFC_Int_ClearRawStatus function.
 *
 * @par Status information:
 *  The status information of the NFC HW block can be retrieved by using the #Chip_NFC_GetStatus function. Refer to the
 *  enum #NFC_STATUS_T for more details.
 *
 * @par Last RF access information:
 *  This feature provides the following information with respect to the last access made from RF side. This can be
 *  retrieved using the #Chip_NFC_GetLastAccessInfo function.
 *  -# Start word offset in BUF for the last RF access.
 *  -# End word offset in BUF for the last RF access.
 *  -# Direction (read/write) of last RF access.
 *
 * @par Additional Information:
 *  <dl><dt> NDEF: </dt><dd>
 *      This term stands for NFC Data exchange format and more details about this can be referred from the technical
 *      specification available on NFC Forum website. </dd>
 *  <dt> TLV: </dt><dd>
 *      This term stands for "Type Length Value" and more details about this can be referred from Type 2 Tag operation
 *      technical specification which is also available on NFC Forum website. </dd></dl>
 *
 * @par Example 1 - Simple NDEF Message Write
 *  The below example demonstrates how a simple NDEF message can be written to the shared memory which can then be read
 *  by an external reader. Refer to @ref MODS_NSS_NDEFT2T for a simplified API to handle NDEF Messages.
 *  @snippet nfc_nss_example_1.c nfc_nss_example_1
 *
 * @par Example 2 - Simple NDEF Message Read
 *  The below example demonstrates how a simple NDEF message can be read from the shared memory which was previously
 *  written by an external reader. This example assumes that the message written by the external reader is of length
 *  less than 255 bytes and no additional TLVs are present before the NDEF TLV.
 *  @snippet nfc_nss_example_2.c nfc_nss_example_2
 *
 * @par Example 3 - Using NFC interrupt for external reader access detection
 *  @snippet nfc_nss_example_3.c nfc_nss_example_3
 *  Handle interrupt:
 *  @snippet nfc_nss_example_3.c nfc_nss_example_3_irq
 *
 * @par Example 4 - Using NFC interrupt for detecting read/write to particular page offset and getting
 *  information about last RF access.
 *  @snippet nfc_nss_example_4.c nfc_nss_example_4
 *  Handle interrupt:
 *  @snippet nfc_nss_example_4.c nfc_nss_example_4_irq
 *
 * @{
 */

/** Near Field Communication (NFC) register block structure (APB side) */
typedef struct NSS_NFC_S {
    __IO uint32_t CFG; /*!< Configuration register. */
    __I uint32_t SR; /*!< NFC status register. */
    __I uint32_t CMDIN; /*!< NFC incoming command. The driver does not provide functions to access this register */
    __O uint32_t DATAOUT; /*!< NFC outgoing command. The driver does not provide functions to access this register */
    __IO uint32_t TARGET; /*!< NFC target page address register. */
    __I uint32_t LAST_ACCESS; /*!< NFC last accessed page register. */
    __IO uint32_t IMSC; /*!< Interrupt mask register. */
    __I uint32_t RIS; /*!< Raw Interrupt status register. */
    __I uint32_t MIS; /*!< Masked Interrupt status register. */
    __O uint32_t IC; /*!< Interrupt Clear register. */
    __IO uint32_t RESERVED0[54]; /* next field at offset 0x100 */
    __IO uint32_t BUF[128]; /*!< Shared memory buffer memory space, located at offset 0x100-0x2FC
                                        (128 words of 4 bytes each). */
} NSS_NFC_T;

/** NFC interrupt flags provides the information as listed below. The flag can be set using #Chip_NFC_Int_SetEnabledMask.
 *  The status of the interrupt can be retrieved using #Chip_NFC_Int_GetRawStatus. Clearing of the flag can be done
 *  using #Chip_NFC_Int_ClearRawStatus. */
typedef enum NFC_INT {
    NFC_INT_RFPOWER = (1 << 0), /*!< RFID power is detected. */
    NFC_INT_RFSELECT = (1 << 1), /*!< Tag is selected by reader. */
    NFC_INT_MEMREAD = (1 << 2), /*!< Reader reads from shared memory. */
    NFC_INT_MEMWRITE = (1 << 3), /*!< Reader writes to shared memory. */
    NFC_INT_CMDWRITE = (1 << 4), /*!< Reader writes to #NSS_NFC_T.CMDIN register. */
    NFC_INT_CMDREAD = (1 << 5), /*!< Reader reads the #NSS_NFC_T.DATAOUT register. */
    NFC_INT_TARGETWRITE = (1 << 6), /*!< Reader writes to address specified in the #NSS_NFC_T.TARGET register. */
    NFC_INT_TARGETREAD = (1 << 7), /*!< Reader reads from address specified in the #NSS_NFC_T.TARGET register. */
    NFC_INT_NFCOFF = (1 << 8), /*!< NFC front-end is powered down by external reader. */
    NFC_INT_NONE = 0, /*!< De-selects all Interrupts */
    NFC_INT_ALL = 0x1FF /*!< Selects all Interrupts */
} NFC_INT_T;

/** NFC HW block status flags provides information on the NFC HW block as listed below. This can be retrieved using
 *  #Chip_NFC_GetStatus. */
typedef enum NFC_STATUS {
    NFC_STATUS_POR = (1 << 0), /*!< Indicates that the NFC analog core has been powered ON. */
    NFC_STATUS_1V2 = (1 << 1), /*!< Indicates a warning that VDD_RFID is < 1.2 V. */
    NFC_STATUS_1V5 = (1 << 2), /*!< Indicates a warning that VDD_RFID is < 1.5 V. */
    NFC_STATUS_PLL = (1 << 3), /*!< Indicates that NFC PLL has been locked. */
    NFC_STATUS_SEL = (1 << 4), /*!< Indicates that the NFC block has been activated via ISO/IEC 14443 Type A commands.
                                    Or in other words the MIFARE Ultralight EV1 block has reached the ACTIVE state in
                                    it's state machine. All memory operations falling under MIFARE Ultralight EV1
                                    command set can now be carried out over the RF. */
    NFC_STATUS_AUTH = (1 << 5), /*!< Indicates that password authentication is successful. All memory operations to
                                     password protected regions can now be carried out over the RF. */
    NFC_STATUS_BYPASS = (1 << 6) /*!< Indicates that the NFC interface is in bypass mode. */
} NFC_STATUS_T;

#define NFC_SHARED_MEM_BYTE_SIZE (int)(sizeof(NSS_NFC->BUF)) /*!< NFC shared RAM size in bytes. */
#define NFC_SHARED_MEM_WORD_SIZE (NFC_SHARED_MEM_BYTE_SIZE / 4) /*!< NFC shared RAM size in 32bit words. */
#define NFC_SHARED_MEM_START (int)(NSS_NFC->BUF) /*!< NFC shared RAM start address. */
#define NFC_SHARED_MEM_END (NFC_SHARED_MEM_START + NFC_SHARED_MEM_BYTE_SIZE -1) /*!< NFC shared RAM end address. */

/**
 * Initializes the NFC HW block. The function disables the NFC interrupt internally and also clears any pending
 * interrupts. Any previously written NDEF message will be lost due to initialization of shared memory.
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @note Default settings for NFC are:
 *  - Disabled by default
 *  - Status Fields : Cleared by default
 */
void Chip_NFC_Init(NSS_NFC_T *pNFC);

/**
 * De-initializes NFC HW block. The function disables the NFC interrupt internally and also clears any pending
 * interrupts. This will overwrite any NDEF message that was previously present with default values.
 * @param pNFC : The base address of the NFC peripheral on the chip
 */
void Chip_NFC_DeInit(NSS_NFC_T *pNFC);

/**
 * Returns the status information from the NFC block
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @return Status of the NFC HW block.
 */
NFC_STATUS_T Chip_NFC_GetStatus(NSS_NFC_T *pNFC);

/**
 * Enables/Disables the NFC interrupts.
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @param mask : Interrupt enabled mask to set
 */
void Chip_NFC_Int_SetEnabledMask(NSS_NFC_T *pNFC, NFC_INT_T mask);

/**
 * Retrieves the NFC interrupt enabled mask
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @return Interrupt enabled Mask
 */
NFC_INT_T Chip_NFC_Int_GetEnabledMask(NSS_NFC_T *pNFC);

/**
 * Retrieves a bitVector with the RAW NFC interrupt flags
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @return BitVector with the NFC RAW interrupt flags
 * @note A bit set to 1 means that the correspondent interrupt flag is set.
 * @warning When #Chip_NFC_ByteRead and/or #Chip_NFC_WordWrite API is used, application should take care of
 * the following constraints:
 *  -# Application should use #Chip_NFC_Int_ClearRawStatus for clearing interrupts. Direct register access
 *     for clearing interrupt status is not allowed.
 *  -# #NFC_INT_MEMWRITE interrupt status is not sticky. Application should use the flag in the ISR context only.
 */
NFC_INT_T Chip_NFC_Int_GetRawStatus(NSS_NFC_T *pNFC);

/**
 * Clears the required NFC interrupt flags
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @param flags : Bitvector indicating which interrupt flags to clear
 */
void Chip_NFC_Int_ClearRawStatus(NSS_NFC_T *pNFC, NFC_INT_T flags);

/**
 * Sets the target address used for interrupt generation
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @param offset : 32-bit word offset from start of BUF
 */
void Chip_NFC_SetTargetAddress(NSS_NFC_T *pNFC, uint32_t offset);

/**
 * Returns the target address used for interrupt generation
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @return 32-bit word offset from start of BUF
 */
uint32_t Chip_NFC_GetTargetAddress(NSS_NFC_T *pNFC);

/**
 * Returns the start and end 32-bit word offset from start of BUF and the direction of last RF access
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @param [out] pStartOffset : Start word offset for last RF access
 * @param [out] pEndOffset : Last accessed word offset for last RF access
 * @return
 *  - @c true for write access.
 *  - @c false for read access.
 */
bool Chip_NFC_GetLastAccessInfo(NSS_NFC_T *pNFC, uint32_t *pStartOffset, uint32_t *pEndOffset);

/**
 * Writes a block of words to the BUF, and returns success/failure of write operation.
 * Failure indicates corruption of written data due to RF access.
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @param pDest : Destination address in BUF
 * @param pSrc : Source buffer address (32 bit word aligned)
 * @param n : Number of words
 * @return
 *  - @c true for successful write.
 *  - @c false for write failure.
 * @note Write access is word based, and address should be 32-bit word aligned.
 * @warning When this API is used, application should take care of the following constraints:
 *  -# Application should use #Chip_NFC_Int_ClearRawStatus for clearing interrupts. Direct register access
 *     for clearing interrupt status is not allowed.
 *  -# #NFC_INT_MEMWRITE interrupt status is not sticky. Application should use the flag in the ISR context only.
 */
bool Chip_NFC_WordWrite(NSS_NFC_T *pNFC, uint32_t * pDest, const uint32_t * pSrc, int n);

/**
 * Reads a block of bytes from the BUF, and returns success/failure of read operation.
 * Failure indicates corruption of read data due to RF access.
 * @param pNFC : The base address of the NFC peripheral on the chip
 * @param pDest : Destination buffer address
 * @param pSrc : Source address in BUF
 * @param n : Number of bytes
 * @return
 *  - @c true for successful read.
 *  - @c false for read corruption.
 * @note Read access is byte based.
 * @warning When this API is used, application should take care of the following constraints:
 *  -# Application should use #Chip_NFC_Int_ClearRawStatus for clearing interrupts. Direct register access
 *     for clearing interrupt status is not allowed.
 *  -# #NFC_INT_MEMWRITE interrupt status is not sticky. Application should use the flag in the ISR context only.
 */
bool Chip_NFC_ByteRead(NSS_NFC_T *pNFC, uint8_t * pDest, const uint8_t * pSrc, int n);

#endif /** @} */
