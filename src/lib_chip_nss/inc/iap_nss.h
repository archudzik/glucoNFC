/*
 * Copyright 2014-2016,2018-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __IAP_NSS_H_
#define __IAP_NSS_H_

/**
 * @defgroup IAP_NSS iap: In Application Programming driver
 * @ingroup DRV_NSS
 * The In Application Programming driver (IAP) API implements a wrapper for the actual code which is in ROM, and which
 * is exposed via the binary IAP interface. The driver exposes the following features:
 *  -# Read factory settings.
 *  -# Prepare (unprotect) sectors for flash erase/program operations.
 *  -# Erase flash sectors.
 *  -# Erase flash pages.
 *  -# Copy the data from RAM/EEPROM and write to flash.
 *  -# Perform blank checking of the flash sectors.
 *  -# Read Part Identification.
 *  -# Read boot code version.
 *  -# Read UID (Unique Identification) which is the device serial number.
 *  -# Perform comparison of two memory spaces.
 * The "Prepare" command will unprotect the sectors, and upon success execution of the flash operations
 * including sector erase, page erase, and program commands, the touched sectors will be protected automatically.
 *
 * @warning To erase/program the flash, the system clock must be set to a clock frequency of minimum 125kHz.
 *
 * @see Please refer to the chapter "In Application Programming Firmware" of the User Manual
 *  for details on the IAP interface.
 *
 * @warning During an erase or program operation, the flash is not accessible, hence if the Interrupt Vector Table
 *  is placed in Flash (default setup) the application should make sure that no interrupts are triggered.
 *  To disable all interrupts please refer to #__disable_irq(void).
 *
 * @par Example1 - Erase flash sector 10 and write 4 pages (e.g. 64 bytes per page)
 *  of data to the start address of sector 10
 *  - Data : stored in ram buffer "buffer[256]"
 *  - Flash address : start address of sector 10. (Since each sector contain 16 pages, the first page in sector 10 is
 *      10 * 16 = 160. Since sectors are 1024 bytes, the start address of sector 10 is 10 * 1024 = 10240.)
 *  - System clock : 8 MHz
 *  @snippet iap_nss_example_1.c iap_nss_example_1
 *
 * @par Example 2 - Load RTC factory calibration value into RTC
 *  @snippet iap_nss_example_2.c iap_nss_example_2
 *
 * @par Example 3 - Read Part ID, boot version & UID
 *  @snippet iap_nss_example_3.c iap_nss_example_3
 *
 * @{
 */

/** Possible return codes from the IAP */
typedef enum IAP_STATUS
{
    IAP_STATUS_CMD_SUCCESS         = 0, /*!< SUCCESS */
    IAP_STATUS_INVALID_COMMAND     = 1, /*!< Command number is invalid */
    IAP_STATUS_SRC_ADDR_ERROR      = 2, /*!< Source address not on word boundary */
    IAP_STATUS_DST_ADDR_ERROR      = 3, /*!< Destination address not on word or 64 byte boundary */
    IAP_STATUS_SRC_ADDR_NOT_MAPPED = 4, /*!< Source address is out of range */
    IAP_STATUS_DST_ADDR_NOT_MAPPED = 5, /*!< Destination address is out of range */
    IAP_STATUS_COUNT_ERROR         = 6, /*!< Byte count is not multiple of 4 or is not a permitted value */
    IAP_STATUS_INVALID_SECTOR      = 7, /*!< Sector number is invalid */
    IAP_STATUS_SECTOR_NOT_BLANK    = 8, /*!< Sector is not blank */
    IAP_STATUS_SECTOR_NOT_PREPARED = 9, /*!< Sector is not prepared, or still protected */
    IAP_STATUS_COMPARE_ERROR       = 10, /*!< Compare error which means the data buffers contain different content */
    IAP_STATUS_BUSY                = 11, /*!< Flash hardware is busy */
    IAP_STATUS_PARAM_ERROR         = 12, /*!< Insufficient number of parameters */
    IAP_STATUS_ADDR_ERROR          = 13 /*!< Address is not on word boundary */
} IAP_STATUS_T;

/**
 * Read factory settings for calibration registers
 * @param address : Address of the targeted calibration register that needs to read the factory setting,
 *  Please see the user manual to find out which registers have factory setting available.
 * @return Output parameter for the factory setting
 * @note This function asserts that the requested address does correspond to a supported factory setting.
 * @warning As part of this API call, EEPROM is de-initialized before the IAP command execution, if it was already in
 *  initialized state. Following that, it will get initialized back after the IAP command gets executed. Thus the
 *  execution time for this function will vary depending on the state of EEPROM prior to calling this function.
 */
uint32_t Chip_IAP_ReadFactorySettings(uint32_t address);

/**
 * Read Part Identification Number
 * @return Part Identification Number read via IAP command.
 */
uint32_t Chip_IAP_ReadPartID(void);

/**
 * Read boot version number
 * @return Boot version number. Read as:
 *  - <Byte 3> : Reserved.
 *  - <Byte 2> : Reserved.
 *  - <Byte 1> : Major Version Number, 8 bits.
 *  - <Byte 0> : Minor Version Number, 8 bits, LSB.
 * @note Mask operation is required in order to get the correct version number.
 */
int Chip_IAP_ReadBootVersion(void);

/**
 * Read UID - the device serial number
 * @param uid : array with caller allocated space of 4 words that will be filled with the UID.
 * @warning As part of this API call, EEPROM is de-initialized before the IAP command execution, if it was already in
 *  initialized state. Following that, it will get initialized back after the IAP command gets executed. Thus the
 *  execution time for this function will vary depending on the state of EEPROM prior to calling this function.
 */
void Chip_IAP_ReadUID(uint32_t uid[4]);

/**
 * Prepare (i.e. unprotect) the flash sectors
 * @param sectorStart : The first sector that will be unprotected.
 * @param sectorEnd : The last sector that will be unprotected.
 * @return Result of the operation.
 * @note Both the start sector and end sector are inclusive.
 * @note Please keep the claimed RAM area untouched. Refer to user manual for more details.
 */
IAP_STATUS_T Chip_IAP_Flash_PrepareSector(uint32_t sectorStart, uint32_t sectorEnd);

/**
 * Erase flash sectors
 * @param sectorStart : The first sector to be erased.
 * @param sectorEnd : The last sector to be erased.
 * @param kHzSysClk : The actual CPU clock speed in kHz. This parameter will be ignored for chips without external clock.
 * @return Result of the operation.
 * @note Both the start sector and end sector are inclusive.
 * @note #Chip_IAP_Flash_PrepareSector must be called in advance for all sectors.
 * @note For page level erase, please refer to API #Chip_IAP_Flash_ErasePage.
 * @note Please keep the claimed RAM area untouched. Refer to user manual for more details.
 * @pre If the Interrupt Vector Table is placed in Flash (default setup) disable all interrupts: #__disable_irq(void)
 * @post If interrupts were disabled, enable them again: #__enable_irq(void)
 */
IAP_STATUS_T Chip_IAP_Flash_EraseSector(uint32_t sectorStart, uint32_t sectorEnd, uint32_t kHzSysClk);

/**
 * Erase flash pages
 * @param pageStart : The first page to be erased.
 * @param pageEnd : The last page to be erased.
 * @param kHzSysClk : The actual CPU clock speed in kHz. This parameter will be ignored for chips without external clock.
 * @return Result of the operation.
 * @note Both the start page and end page are inclusive.
 * @note #Chip_IAP_Flash_PrepareSector must be called in advance for all sectors involved.
 * @note For sector level erase, please refer to API #Chip_IAP_Flash_EraseSector.
 * @note Please keep the claimed RAM area untouched. Refer to user manual for more details.
 * @pre If the Interrupt Vector Table is placed in Flash (default setup) disable all interrupts: #__disable_irq(void)
 * @post If interrupts were disabled, enable them again: #__enable_irq(void)
 */
IAP_STATUS_T Chip_IAP_Flash_ErasePage(uint32_t pageStart, uint32_t pageEnd, uint32_t kHzSysClk);

/**
 * Copy data from RAM and write/program to flash
 * @param pSrc : The ram or EEPROM address where the data is copied from. Must be word (32 bits) aligned.
 * @param pFlash : The flash address where the data is copied to. Must be page (64 bytes) aligned.
 * @param size : Number of bytes to be copied from RAM to flash. Must be multiple of 64.
 * @param kHzSysClk : The actual CPU clock speed in kHz. This parameter will be ignored for chips without external clock.
 * @return Result of the operation.
 * @note This API implements the IAP command "Copy RAM to Flash" listed in user manual.
 * @note The involved pages (or sectors) must be erased before this function is called, unless partial page write is
 *  intended (see note below).
 * @note If partial page writes are intended:
 *  - User needs to ensure that the region of the page to be written is previously erased (all 0xFFs)
 *  - User needs to ensure that the region of the page to be left untouched is overwritten with the same data or with
 *      all 0xFFs
 *  - User needs to ensure that partial writes are always in a word boundary (the content of one word shall not be
 *      written more than once after erasing)
 *  Typical flow:
 *      -# Erase page (content is FFFFFFFF FFFFFFFF ... FFFFFFFF)
 *      -# Write first word 0x01234567 by writing "01234567 FFFFFFFF ... FFFFFFFF"
 *          (content becomes 01234567 FFFFFFFF ... FFFFFFFF)
 *      -# Write second word 0x89ABCDEF by either:
 *          - writing "FFFFFFFF 89ABCDEF ... FFFFFFFF", or
 *          - writing "01234567 89ABCDEF ... FFFFFFFF"
 *              (content becomes 01234567 89ABCDEF ... FFFFFFFF)
 *  Writing to the first and second word after this flow requires a new erase procedure, but writing words 3 to 16 not.
 * @note Please keep the claimed RAM area untouched. Refer to user manual for more details.
 * @pre If the Interrupt Vector Table is placed in Flash (default setup) disable all interrupts: #__disable_irq(void)
 * @post If interrupts were disabled, enable them again: #__enable_irq(void)
 */
IAP_STATUS_T Chip_IAP_Flash_Program(const void *pSrc, const void *pFlash, uint32_t size, uint32_t kHzSysClk);

/**
 * Blank check on flash sectors
 * @param sectorStart : The first sector to be checked.
 * @param sectorEnd : The last sector to be checked.
 * @param pOffset : output parameter with the (word) offset for the first non-blank word;
 *  when the result is #IAP_STATUS_SECTOR_NOT_BLANK .
 * @param pContent : output parameter with the value of the first non-blank word;
 *  when the result is #IAP_STATUS_SECTOR_NOT_BLANK .
 * @return Result of the operation.
 * @note @c pOffset and/or @c pContent may be @c NULL. In that case, offset and/or content are not reported,
 *  but the function still performs.
 *
 */
IAP_STATUS_T Chip_IAP_Flash_SectorBlankCheck(uint32_t sectorStart, uint32_t sectorEnd, uint32_t *pOffset, uint32_t *pContent);

/**
 * Compare two memory spaces to find out if their contents are same.
 * @param pAddress1 : The start address of the memory block to be compared. Must be word (32 bits) aligned.
 * @param pAddress2 : The start address of the memory block to compare with. Must be word (32 bits) aligned.
 * @param size : Number of bytes to be compared. Must be multiple of 4.
 * @param pOffset : The output parameter with the (word) offset for the first unequal words;
 *  when the compare result is #IAP_STATUS_COMPARE_ERROR.
 * @return Result of the operation.
 * @note @c pOffset may be @c NULL. In that case, offset is not reported, but the function still performs.
 */
IAP_STATUS_T Chip_IAP_Compare(const void *pAddress1, const void *pAddress2, uint32_t size, uint32_t *pOffset);

#endif /** @} */
