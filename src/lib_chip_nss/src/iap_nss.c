/*
 * Copyright 2014-2017,2019-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"

/* ------------------------------------------------------------------------- */

/** Commands supported by IAP */
typedef enum IAP_CMD
{
    IAP_CMD_READ_FACTORY_SETTINGS      = 40, /*!< Read factory settings. */
    IAP_CMD_FLASH_PREPARE_SECTOR       = 50, /*!< Prepare flash sectors for erase or program operations. */
    IAP_CMD_FLASH_PROGRAM              = 51, /*!< Copy data from RAM buffer to flash. It's also called "Copy RAM to Flash" in user manual. */
    IAP_CMD_FLASH_ERASE_SECTOR         = 52, /*!< Erase flash sectors. */
    IAP_CMD_FLASH_SECTOR_BLANK_CHECK   = 53, /*!< Perform blank checking for flash sectors. */
    IAP_CMD_READ_PART_ID               = 54, /*!< Read part identification. */
    IAP_CMD_READ_BOOT_VERSION          = 55, /*!< Read boot code version number. */
    IAP_CMD_COMPARE                    = 56, /*!< Compare two memory spaces. */
    IAP_CMD_READ_UID                   = 58, /*!< Read UID. */
    IAP_CMD_FLASH_ERASE_PAGE           = 59  /*!< Erase flash pages. */
} IAP_CMD_T;


/** Type declaration of the function pointer for calling the IAP binary API */
typedef void (*IAP_FUNC) (uint32_t *, uint32_t *);

#define IAP_EXECUTECOMMAND(pCmd, pStatus) ((IAP_FUNC) NSS_IAP_ENTRY)((pCmd), (pStatus))

/* ------------------------------------------------------------------------- */

uint32_t Chip_IAP_ReadFactorySettings(uint32_t address)
{
    uint32_t cmd[2];
    uint32_t status[2] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_READ_FACTORY_SETTINGS;
    cmd[1] = address;

    /* Calling Chip_IAP_ReadFactorySettings resets the EEPROM driver. As a consequence, pending flushes are lost and
     * the EEPROM is not initialized anymore when the IAP call ends. As a workaround, ensure EEPROM is
     * de-initialized before the API call, and re-initialized afterwards (Chip_EEPROM_Init).
     * If the EEPROM controller is powered, we assume it is initialized.
     */
    bool eepromWasInitialised = false;
    if ((!(SYSCON_PERIPHERAL_POWER_EEPROM & Chip_SysCon_Peripheral_GetPowerDisabled()))) {
        eepromWasInitialised = true;
        Chip_EEPROM_DeInit(NSS_EEPROM);
    }

    // Execute the IAP command
    IAP_EXECUTECOMMAND(cmd, status);

    if (eepromWasInitialised) {
        Chip_EEPROM_Init(NSS_EEPROM);
    }

    ASSERT(IAP_STATUS_CMD_SUCCESS == status[0]); // The passed "address" probably is not the address of a register that has factory settings

    // Return the factory setting
    return status[1];
}

uint32_t Chip_IAP_ReadPartID(void)
{
    uint32_t cmd[1];
    uint32_t status[2] = {0xFF};

    // Prepare and execute the IAP command
    cmd[0] = IAP_CMD_READ_PART_ID;
    IAP_EXECUTECOMMAND(cmd, status);

    ASSERT(IAP_STATUS_CMD_SUCCESS == status[0]); // Should never fail except memory corruption like stack overflow.

    return status[1];
}

int Chip_IAP_ReadBootVersion(void)
{
    uint32_t cmd[1];
    uint32_t status[2] = {0xFF};

    // Prepare and execute the IAP command
    cmd[0] = IAP_CMD_READ_BOOT_VERSION;
    IAP_EXECUTECOMMAND(cmd, status);

    ASSERT(IAP_STATUS_CMD_SUCCESS == status[0]); // Should never fail except memory corruption like stack overflow.

    return (int)status[1];
}

void Chip_IAP_ReadUID(uint32_t uid[4])
{
    uint32_t cmd[1];
    uint32_t status[5] = {0xFF};

    // Ensure the uid parameter is valid
    ASSERT(uid != 0); // The uid[] memory is not pre-allocated by the caller.

    // Prepare and execute the IAP command
    cmd[0] = IAP_CMD_READ_UID;

    /* Calling Chip_IAP_ReadUID resets the EEPROM driver. As a consequence, pending flushes are lost and
     * the EEPROM is not initialized any more when the IAP call ends. As a workaround, ensure EEPROM is
     * de-initialized before the API call, and re-initialized afterwards (Chip_EEPROM_Init).
     * If the EEPROM controller is powered, we assume it is initialized.
     */
    bool eepromWasInitialised = false;
    if ((!(SYSCON_PERIPHERAL_POWER_EEPROM & Chip_SysCon_Peripheral_GetPowerDisabled()))) {
        eepromWasInitialised = true;
        Chip_EEPROM_DeInit(NSS_EEPROM);
    }

    IAP_EXECUTECOMMAND(cmd, status);

    if (eepromWasInitialised) {
        Chip_EEPROM_Init(NSS_EEPROM);
    }

    // Ensure the result is success
    ASSERT(IAP_STATUS_CMD_SUCCESS == status[0]); // Should never fail except memory corruption like stack overflow.

    // copy over the UID
    uid[0] = status[1];
    uid[1] = status[2];
    uid[2] = status[3];
    uid[3] = status[4];
}

IAP_STATUS_T Chip_IAP_Flash_PrepareSector(uint32_t sectorStart, uint32_t sectorEnd)
{
    uint32_t cmd[3];
    uint32_t status[1] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_FLASH_PREPARE_SECTOR;
    cmd[1] = sectorStart;
    cmd[2] = sectorEnd;

    IAP_EXECUTECOMMAND(cmd, status);

    // Ensure the IAP command is executed
    ASSERT(0xFF != status[0]); // Should never fail except memory corruption like stack overflow.

    return status[0];
}

IAP_STATUS_T Chip_IAP_Flash_EraseSector(uint32_t sectorStart, uint32_t sectorEnd, uint32_t kHzSysClk)
{
    uint32_t cmd[4];
    uint32_t status[1] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_FLASH_ERASE_SECTOR;
    cmd[1] = sectorStart;
    cmd[2] = sectorEnd;
    cmd[3] = kHzSysClk;

    IAP_EXECUTECOMMAND(cmd, status);

    // Ensure the IAP command is executed
    ASSERT(0xFF != status[0]); // Should never fail except memory corruption like stack overflow.

    return status[0];
}

IAP_STATUS_T Chip_IAP_Flash_ErasePage(uint32_t pageStart, uint32_t pageEnd, uint32_t kHzSysClk)
{
    uint32_t cmd[4];
    uint32_t status[1] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_FLASH_ERASE_PAGE;
    cmd[1] = pageStart;
    cmd[2] = pageEnd;
    cmd[3] = kHzSysClk;

    IAP_EXECUTECOMMAND(cmd, status);

    // Ensure the IAP command is executed
    ASSERT(0xFF != status[0]); // Should never fail except memory corruption like stack overflow.

    return status[0];
}

IAP_STATUS_T Chip_IAP_Flash_Program(const void *pSrc, const void *pFlash, uint32_t size, uint32_t kHzSysClk)
{
    uint32_t cmd[5];
    uint32_t status[1] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_FLASH_PROGRAM;
    cmd[1] = (uint32_t) pFlash;
    cmd[2] = (uint32_t) pSrc;
    cmd[3] = size;
    cmd[4] = kHzSysClk;

    IAP_EXECUTECOMMAND(cmd, status);

    // Ensure the IAP command is executed
    ASSERT(0xFF != status[0]); // Should never fail except memory corruption like stack overflow.

    return status[0];
}

IAP_STATUS_T Chip_IAP_Flash_SectorBlankCheck(uint32_t sectorStart, uint32_t sectorEnd, uint32_t *pOffset, uint32_t *pContent)
{
    uint32_t cmd[3];
    uint32_t status[3] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_FLASH_SECTOR_BLANK_CHECK;
    cmd[1] = sectorStart;
    cmd[2] = sectorEnd;

    // Execute the IAP command
    IAP_EXECUTECOMMAND(cmd, status);

    // Ensure the IAP command is executed
    ASSERT(0xFF != status[0]); // Should never fail except memory corruption like stack overflow.

    // Copy over the offset and content
    if (pOffset != NULL){
        *pOffset = status[1];
    }
    if (pContent != NULL){
        *pContent = status[2];
    }

    return status[0];
}

/* Compare two memory spaces to find out if their contents are same. */
IAP_STATUS_T Chip_IAP_Compare(const void *pAddress1, const void *pAddress2, uint32_t size, uint32_t *pOffset)
{
    uint32_t cmd[4];
    uint32_t status[2] = {0xFF};

    // Prepare the IAP command
    cmd[0] = IAP_CMD_COMPARE;
    cmd[1] = (uint32_t) pAddress1;
    cmd[2] = (uint32_t) pAddress2;
    cmd[3] = size;

    // Execute the IAP command
    IAP_EXECUTECOMMAND(cmd, status);

    // Ensure the IAP command is executed
    ASSERT(0xFF != status[0]); // Should never fail except memory corruption like stack overflow.

    // Copy over the offset
    if (pOffset != NULL){
        *pOffset = status[1];
    }

    return status[0];
}
