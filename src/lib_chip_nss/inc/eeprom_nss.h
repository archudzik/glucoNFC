/*
 * Copyright 2014-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __EEPROM_NSS_H_
#define __EEPROM_NSS_H_

/**
 * @defgroup EEPROM_NSS eeprom: EEPROM driver
 * @ingroup DRV_NSS
 *
 * The EEPROM HW block contains two parts: a controller and a memory core.
 * - controller @n
 *  The controller is the interface between the AHB and the EEPROM memory core.
 *  All interaction to and from the memory core is done via the controller (using #NSS_EEPROM_T).
 * - memory core
 *  The EEPROM memory for a specific chip starts at address #EEPROM_START. It is organized in rows of
 *  #EEPROM_ROW_SIZE bytes. All rows (see #EEPROM_NR_OF_R_ROWS) are readable. The lower #EEPROM_NR_OF_RW_ROWS are
 *  user-writable.
 *
 * @par Key benefit
 *  This EEPROM driver abstracts away the access to the EEPROM memory. The API
 *  - resolves all alignment requirements imposed by the HW block.
 *  - minimizes the number of flushes, maximizing the lifetime of the EEPROM.
 *
 * @par Usage
 *  - First call: #Chip_EEPROM_Init @n
 *      Before the EEPROM memory can be accessed, it must be initialized. During initialization clock and power are
 *      provided to the EEPROM controller and EEPROM memory.
 *  - Any sequence of #Chip_EEPROM_Read and #Chip_EEPROM_Write @n
 *      After initialization, data can be read and written via the API calls at will. Mind, writes are not immediately
 *      persistent in EEPROM. @n
 *      Both reading and writing, the caller must provide an offset in the EEPROM memory and a buffer with size.
 *      The @c offset for the read and write functions is a byte offset and is relative to #EEPROM_START.
 *  - Last call: #Chip_EEPROM_DeInit @n
 *      Data is only guaranteed to be committed in non-volatile memory after a call to #Chip_EEPROM_Flush or
 *      #Chip_EEPROM_DeInit.
 *
 * @warning EEPROM requires a system clock of 500 kHz or higher. This is not checked for.
 *  For a full list of clock restrictions in effect, see @ref NSS_CLOCK_RESTRICTIONS.
 *
 * @par Example - Read, modify and write EEPROM content
 *  @snippet eeprom_nss_example_1.c eeprom_nss_example_1
 *
 * @{
 */

#include "chip.h"

/** NSS EEPROM register block structure */
typedef struct NSS_EEPROM_S {
    __IO uint32_t CMD; /*!< EEPROM command register */
    __I uint32_t RESERVED1; /*   next field at offset 0x08 */
    __IO uint32_t RWSTATE; /*!< EEPROM read wait state register */
    __IO uint32_t PAUTOPROG; /*!< EEPROM auto programming register */
    __IO uint32_t WSTATE; /*!< EEPROM wait state register */
    __IO uint32_t CLKDIV; /*!< EEPROM clock divider register */
    __IO uint32_t PWRDWN; /*!< EEPROM power down register */
    __I uint32_t RESERVED2; /*   next field at offset 0x20 */
    __IO uint32_t MSSTART; /*!< EEPROM checksum start address register */
    __IO uint32_t MSSTOP; /*!< EEPROM checksum stop address register */
    __I uint32_t MSDATASIG; /*!< EEPROM Data signature register */
    __I uint32_t MSPARSIG; /*!< EEPROM parity signature register */
    __I uint32_t RESERVED3; /*   next field at offset 0x34 */
    __I uint32_t STATUS; /*!< EEPROM device status register */
    __I uint32_t RESERVED4[998]; /*   next field at offset 0xFD0 */
    __I uint32_t MODULE_CONFIG; /*!< Controller configuration options */
    __I uint32_t RESERVED5; /*   next field at offset 0xFD8 */
    __IO uint32_t INT_CLR_ENABLE; /*!< Clear interrupt enable bits */
    __IO uint32_t INT_SET_ENABLE; /*!< Set interrupt enable bits */
    __I uint32_t INT_STATUS; /*!< Interrupt status bits */
    __I uint32_t INT_ENABLE; /*!< Interrupt enable bits */
    __IO uint32_t INT_CLR_STATUS; /*!< Clear interrupt status bits */
    __IO uint32_t INT_SET_STATUS; /*!< Set interrupt status bits */
    __I uint32_t RESERVED6[3]; /*   next field at offset 0xFFC */
    __IO uint32_t MODULE_ID; /*!< Controller memory module identification */
} NSS_EEPROM_T;

/**
 * Initializes EEPROM peripheral. This function must be the first function to call in this driver after going to
 * deep sleep, deep power down, power-off power save mode, or after changing the system clock frequency.
 * - Power and Clock are enabled for EEPROM and for EEPROM controller block.
 * - Based upon the configured system clock, the correct clock divider is selected.
 * - Will wait after enabling the HW block until the HW block can be used.
 * @param pEEPROM : ignored. Argument no longer used but kept for compatibility.
 * @note The EEPROM requires a minimum system clock frequency before it can be used.
 *  See @ref clockrestrictions_anchor_table "Clock Restrictions".
 * @post All others API calls of this driver may now be used.
 */
void Chip_EEPROM_Init(NSS_EEPROM_T *pEEPROM);

/**
 * Disables EEPROM peripheral. If required, data is flushed
 * Power and Clock are disabled for EEPROM and for EEPROM controller block.
 * @param pEEPROM : ignored. Argument no longer used but kept for compatibility.
 * @post Only #Chip_EEPROM_Init may now be used of this driver.
 */
void Chip_EEPROM_DeInit(NSS_EEPROM_T *pEEPROM);

/**
 * Reads data from the EEPROM memory into a user allocated buffer.
 * @param pEEPROM : ignored. Argument no longer used but kept for compatibility.
 * @param offset : EEPROM Offset, in bytes, from where to start writing the data into EEPROM. The offset is relative to
 *  #EEPROM_START
 * @param pBuf : Pointer to the user allocated buffer in ram. There are no alignment requirements.
 * @param size : Number of bytes to copy
 * @pre @c offset an @c size denote a memory region. The caller must ensure this whole region lies inside the
 *  EEPROM memory. This is not checked for.
 */
void Chip_EEPROM_Read(NSS_EEPROM_T *pEEPROM, int offset, void *pBuf, int size);

/**
 * Writes data from a user allocated buffer into the EEPROM memory
 * @param pEEPROM : ignored. Argument no longer used but kept for compatibility.
 * @param offset : EEPROM Offset, in bytes, from where to start writing the data into EEPROM. The offset is relative to
 *  #EEPROM_START
 * @param pBuf : Pointer to the data to be copied into EEPROM. There are no alignment requirements.
 * @param size : Number of bytes to copy
 * @c offset + @c size must not exceed #EEPROM_ROW_SIZE * #EEPROM_NR_OF_RW_ROWS
 * @note This function's timing is non deterministic. Possibly, a pending flush must be executed before data is written;
 *  and when the data to write is spread over multiple EEPROM rows, intermediate flushes are also required.
 * @note The portion written in the last row is not committed an flushed to EEPROM. To be sure written data is
 *  effectively retained in the EEPROM memory, user needs to call #Chip_EEPROM_Flush or #Chip_EEPROM_DeInit
 * @pre @c offset an @c size denote a memory region. The caller must ensure this whole region lies inside the
 *  EEPROM memory. This is not checked for.
 */
void Chip_EEPROM_Write(NSS_EEPROM_T *pEEPROM, int offset, const void *pBuf, int size);

/**
 * Memsets a pattern into the EEPROM memory
 * @param pEEPROM : ignored. Argument no longer used but kept for compatibility.
 * @param offset : EEPROM Offset, in bytes, from where to start writing the data into EEPROM. The offset is relative to
 *  #EEPROM_START
 * @param pattern: Byte pattern to be set into EEPROM
 * @param size : Number of bytes to set
 * @note The @c offset an @c size denote a memory region. If any part of this region lies outside the EEPROM memory,
 *  nothing is written, and the function call is a void operation.
 *  @c offset + @c size must not exceed #EEPROM_ROW_SIZE * #EEPROM_NR_OF_RW_ROWS
 * @note This function's timing is non deterministic. Possibly, a pending flush must be executed before data is written;
 *  and when the data to write is spread over multiple EEPROM rows, intermediate flushes are also required.
 * @note The portion written in the last row is not committed an flushed to EEPROM. To be sure written data is
 *  effectively retained in the EEPROM memory, user needs to call #Chip_EEPROM_Flush or #Chip_EEPROM_DeInit
 */
void Chip_EEPROM_Memset(NSS_EEPROM_T *pEEPROM, int offset, uint8_t pattern, int size);

/**
 * If needed, this function flushes pending data into the EEPROM. When this function returns, the flush operation has
 * been completed. To be used only if the user wants to make sure written data is retained, for example before going to
 * sleep.
 * @param pEEPROM : ignored. Argument no longer used but kept for compatibility.
 * @param wait : ignored. Argument no longer used but kept for compatibility.
 */
void Chip_EEPROM_Flush(NSS_EEPROM_T *pEEPROM, bool wait);

#endif /** @} */
