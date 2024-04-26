/*
 * Copyright 2014-2016,2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __FLASH_NSS_H_
#define __FLASH_NSS_H_

/** @defgroup FLASH_NSS flash: Flash driver
 * @ingroup DRV_NSS
 * The Flash driver provides APIs to select the power mode that the flash memory operates in, and the number
 * of wait states to apply to flash operations.
 * @note Please note that this API provides only a small part of the features.
 *
 * @par Flash Power Modes
 *  The flash memory on the chip supports two power modes:
 *      - low power mode
 *      - high power mode \n
 *          High power mode may be required for some flash operations, please refer to the user manual for details.
 * @warning Directly accessing Flash controller addresses (#NSS_FLASH_T) when the Flash clock is disabled blocks the
 *  APB bus indefinitely.
 *
 * @par Wait States
 *  To manage the flash memory read operation in low power mode, it may be necessary - dependent on the the system clock
 *  frequency - to wait for some number of extra system clocks, which is also called wait states. Please refer to the
 *  user manual for more details. For a full list of clock restrictions in effect, see @ref NSS_CLOCK_RESTRICTIONS.
 *
 * @par Erasing/Writing
 *  For flash erasing and writing, please refer to @ref IAP_NSS.
 *
 * @par Example - Switching between flash modes:
 *  Assume a controller that normally runs at 0.5MHz, but it can also run at 8MHz. When running on 8MHz, the
 *  flash should be either in high power mode(1), or in low power mode with extra wait states(2).
 *  -# Switch to 8MHz in low power mode with extra wait states
 *      @snippet flash_nss_example_1.c flash_nss_example_1_a
 *  -# Switch to 8MHz in high power mode without extra wait states
 *      @snippet flash_nss_example_1.c flash_nss_example_1_b
 * @{
 */

/** NSS Flash register block structure */
typedef struct NSS_FLASH_S {
    __IO uint32_t FCTR;           /*!< Flash control register */
    __I  uint32_t FSTAT;          /*!< Flash status register */
    __IO uint32_t FPTR;           /*!< Flash program-time register */
    __I  uint32_t RESERVED1;      /*   next field at offset 0x010 */
    __IO uint32_t FBWST;          /*!< Flash wait state register */
    __I  uint32_t RESERVED2[2];   /*   next field at offset 0x01C */
    __IO uint32_t FCRA;           /*!< Flash program clock divider */
    __IO uint32_t FMSSTART;       /*!< Flash checksum start address register */
    __IO uint32_t FMSSTOP;        /*!< Flash checksum stop address register */
    __I  uint32_t FMS16;          /*!< Flash parity signature register */
    __I  uint32_t FMSW0;          /*!< Flash data signature register */
    __I  uint32_t RESERVED3[8];   /*   next field at offset 0x050 */
    __IO uint32_t ECCRSTERRCNT;   /*!< Invalid flag and error corrected counter reset */
    __I  uint32_t ECCERRCNT;      /*!< ECC status information */
    __I  uint32_t RESERVED4[990]; /*   next field at offset 0xFD0 */
    __I  uint32_t MODULE_CONFIG;  /*!< Controller configuration options */
    __I  uint32_t RESERVED5;      /*   next field at offset 0xFD8 */
    __O  uint32_t INT_CLR_ENABLE; /*!< Clear interrupt enable bits */
    __O  uint32_t INT_SET_ENABLE; /*!< Set interrupt enable bits */
    __I  uint32_t INT_STATUS;     /*!< Interrupt status bits */
    __I  uint32_t INT_ENABLE;     /*!< Interrupt enable bits */
    __O  uint32_t INT_CLR_STATUS; /*!< Clear interrupt status bits */
    __O  uint32_t INT_SET_STATUS; /*!< Set interrupt status bits */
    __I  uint32_t RESERVED6[3];   /*   next field at offset 0xFFC */
    __I  uint32_t MODULE_ID;      /*!< Controller memory module identification */
} NSS_FLASH_T;

/**
 * Sets the power mode of the flash memory.
 * @param highPower : True for high power mode, false for low power mode
 * @pre If @c highPower equals @c false, the caller must ensure that either
 *  - flash wait states is set to 1 or higher, or
 *  - system clock divisor is not 1 - by calling #Chip_Clock_System_SetClockFreq with argument @c 4000000 or less.
 *  Failure to do so will result in a hard fault (flash access error).
 *  See also @ref NSS_CLOCK_RESTRICTIONS
 */
void Chip_Flash_SetHighPowerMode(bool highPower);

/**
 * Gets the power mode of the flash memory.
 * @return True if the flash memory operates in high power mode, false otherwise.
 */
bool Chip_Flash_GetHighPowerMode(void);

/**
 * Sets the number of wait states for flash operations.
 * @param waitStates : Number of wait states to be added to flash operations.
 * @pre If @c waitStates equals @c 0, the caller must ensure that either
 *  - high power mode is configured - #Chip_Flash_SetHighPowerMode with argument @c true, or
 *  - system clock divisor is not 1 - by calling #Chip_Clock_System_SetClockFreq with argument @c 4000000 or less.
 *  Failure to do so will result in a hard fault (flash access error).
 *  See also @ref NSS_CLOCK_RESTRICTIONS
 */
void Chip_Flash_SetNumWaitStates(int waitStates);

/**
 * Gets the number of wait states applied to flasAh operations.
 * @return The number of wait states that are currently applied to flash operations.
 */
int Chip_Flash_GetNumWaitStates(void);

/**
 * @}
 */

#endif /* __FLASH_NSS_H_ */
