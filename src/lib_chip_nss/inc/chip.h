/*
 * Copyright 2014-2023 NXP
 * NXP Confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __CHIP_H_
#define __CHIP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "assert.h"
#include "cmsis.h"
#include "startup/startup.h"

/** @defgroup CHIP_NSS chip: Peripheral addresses and register set declarations
 * @ingroup DRV_NSS
 * Aggregates the addresses and handle definitions for all the peripherals of the IC.
 * @{
 */

#define SDK_VERSION 12_6_nhs3152 /**< SDK version information */

#define NSS_SFRO_FREQUENCY     8000000 /**< System Free-Running Oscillator (SFRO) frequency in Hz */
#define NSS_TFRO_FREQUENCY     32768 /**< Timer Oscillator (TFRO) frequency in Hz */

/* Memories */
#define EEPROM_START           0x30000000 /**< EEPROM start address */
#define EEPROM_ROW_SIZE        64 /**< The EEPROM is organized in rows, this is the row size (in bytes) */
#define EEPROM_NR_OF_R_ROWS    64 /**< Complete EEPROM area is readable */
#define EEPROM_NR_OF_RW_ROWS   58 /**< Last 6 rows (384 bytes) are read-only, so we only publish 58 rows */

#define FLASH_START            0 /**< FLASH start address */
#define FLASH_SECTOR_SIZE      1024 /**< The FLASH is organized in sectors, this is the sector size (in bytes) */
#define FLASH_PAGE_SIZE        64 /**< Each FLASH sector is organized in pages, this is the page size (in bytes) */
#define FLASH_PAGES_PER_SECTOR (FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE) /**< Sector size in number of pages. */
#define FLASH_NR_OF_R_SECTORS 32 /**< Complete FLASH area is readable */
#define FLASH_NR_OF_RW_SECTORS 30 /**< Last 2 sectors (2048 bytes) are read-only, so we only publish 30 sectors */

/* Base addresses of sample specific data */
#define NSS_NFC_UID_BASE (EEPROM_START + 0xF9C) /**< Base address of the NFC unique ID (same value can be found in the NFC tag header and read via NFC) */
#define NSS_TSEN_CALIBRATION_TIMESTAMP (EEPROM_START + 0xFD4) /**< Location of the tsen calibration timestamp */

/* Base address of SW Peripherals */
#define NSS_IAP_ENTRY          0x1FFF1FF1 /**< In-Application-Programming entry address */

/* Base addresses of HW Peripherals */
#define NSS_I2C_BASE           0x40000000 /**< Base address of the I2C peripheral */
#define NSS_WWDT_BASE          0x40004000 /**< Base address of the Watchdog Timer peripheral */
#define NSS_TIMER16_0_BASE     0x4000C000 /**< Base address of the Timer0 16-bit peripheral */
#define NSS_TIMER32_0_BASE     0x40014000 /**< Base address of the Timer0 32-bit peripheral */
#define NSS_ADCDAC0_BASE       0x4001C000 /**< Base address of the Analog-to-Digital/Digital-to-Analog peripheral */
#define NSS_FLASH_BASE         0x4003C000 /**< Base address of the Flash memory controller */
#define NSS_EEPROM_BASE        0x40034000 /**< Base address of the EEPROM memory controller */
#define NSS_PMU_BASE           0x40038000 /**< Base address of the Power Management Unit */
#define NSS_SSP0_BASE          0x40040000 /**< Base address of the Synchronous Serial Port peripheral */
#define NSS_IOCON_BASE         0x40044000 /**< Base address of the I/O configuration */
#define NSS_SYSCON_BASE        0x40048000 /**< Base address of the System configuration */
#define NSS_RTC_BASE           0x40054000 /**< Base address of the Real-Time-Clock peripheral */
#define NSS_NFC_BASE           0x40058000 /**< Base address of the Near-Field Communication peripheral */
#define NSS_TSEN_BASE          0x40060000 /**< Base address of the Temperature sensor peripheral */
#define NSS_I2D_BASE           0x40068000 /**< Base address of the Current-to-Digital converter peripheral */
#define NSS_GPIO_BASE          0x50000000 /**< Base address of the General Purpose I/O peripheral */

/* Handles to HW Peripherals */
#define NSS_I2C                ((NSS_I2C_T   *) NSS_I2C_BASE) /**< Handle for the I2C peripheral */
#define NSS_WWDT               ((NSS_WWDT_T  *) NSS_WWDT_BASE) /**< Handle for the Watchdog Timer peripheral */
#define NSS_TIMER16_0          ((NSS_TIMER_T *) NSS_TIMER16_0_BASE) /**< Handle for the Timer0 16-bit peripheral */
#define NSS_TIMER32_0          ((NSS_TIMER_T *) NSS_TIMER32_0_BASE) /**< Handle for the Timer0 32-bit peripheral */
#define NSS_ADCDAC0            ((NSS_ADCDAC_T*) NSS_ADCDAC0_BASE) /**< Handle for the Analog-to-Digital/Digital-to-Analog peripheral */
#define NSS_FLASH              ((NSS_FLASH_T *) NSS_FLASH_BASE) /**< Handle for the Flash memory controller */
#define NSS_EEPROM             ((NSS_EEPROM_T*) NSS_EEPROM_BASE) /**< Handle for the EEPROM memory controller */
#define NSS_PMU                ((NSS_PMU_T   *) NSS_PMU_BASE) /**< Handle for the Power Management Unit */
#define NSS_SSP0               ((NSS_SSP_T   *) NSS_SSP0_BASE) /**< Handle for the Synchronous Serial Port peripheral */
#define NSS_IOCON              ((NSS_IOCON_T *) NSS_IOCON_BASE) /**< Handle for the I/O configuration */
#define NSS_SYSCON             ((NSS_SYSCON_T*) NSS_SYSCON_BASE) /**< Handle for the System configuration */
#define NSS_RTC                ((NSS_RTC_T   *) NSS_RTC_BASE) /**< Handle for the Real-Time-Clock peripheral */
#define NSS_NFC                ((NSS_NFC_T   *) NSS_NFC_BASE) /**< Handle for the Near-Field Communication peripheral */
#define NSS_TSEN               ((NSS_TSEN_T  *) NSS_TSEN_BASE) /**< Handle for the Temperature sensor peripheral */
#define NSS_I2D                ((NSS_I2D_T   *) NSS_I2D_BASE) /**< Handle for the Current-to-Digital converter peripheral */
#define NSS_GPIO               ((NSS_GPIO_T  *) NSS_GPIO_BASE) /**< Handle for the General Purpose I/O peripheral */

/** NFC ID mapping. Structure allows byte-by-byte access to the same memory region. */
typedef struct NFC_UID_S {
    uint8_t bytes[8]; /**< Use for byte-by-byte access */
} NFC_UID_T;

/**
 * Pointer to a copy of the NFC ID, accessible by the ARM. The NFC ID stored in the NFC controller is not accessible by
 * the ARM.
 * @note This value is stored in the (RO part of the) EEPROM. EEPROM must be initialized before accessing.
 *  See #Chip_EEPROM_Init
 */
#define NSS_NFC_UID ((NFC_UID_T *) NSS_NFC_UID_BASE)

#include "adcdac_nss.h"
#include "bussync_nss.h"
#include "clock_nss.h"
#include "eeprom_nss.h"
#include "flash_nss.h"
#include "gpio_nss.h"
#include "i2c_nss.h"
#include "i2d_nss.h"
#include "iap_nss.h"
#include "iocon_nss.h"
#include "nfc_nss.h"
#include "pmu_nss.h"
#include "rtc_nss.h"
#include "ssp_nss.h"
#include "syscon_nss.h"
#include "timer_nss.h"
#include "tsen_nss.h"
#include "wwdt_nss.h"

/* Driver implementations with multiple instances */
#define Chip_TIMER16_0_Init() Chip_TIMER_Init(NSS_TIMER16_0, CLOCK_PERIPHERAL_16TIMER0) /**< Helper function for Timer16_0 Init */
#define Chip_TIMER16_0_DeInit() Chip_TIMER_DeInit(NSS_TIMER16_0, CLOCK_PERIPHERAL_16TIMER0) /**< Helper function for Timer16_0 DeInit */
#define Chip_TIMER32_0_Init() Chip_TIMER_Init(NSS_TIMER32_0, CLOCK_PERIPHERAL_32TIMER0) /**< Helper function for Timer32_0 Init */
#define Chip_TIMER32_0_DeInit() Chip_TIMER_DeInit(NSS_TIMER32_0, CLOCK_PERIPHERAL_32TIMER0) /**< Helper function for Timer32_0 DeInit */

/**
 * Macro to annotate a function as "weak".
 * @note A weak function is linked-in, unless a function with the exact same name
 *  and signature exists (at application level). The latter function will be
 *  linked-in instead. This is primarily useful in defining library functions
 *  that can be overridden in user code.
 *  @see https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 */
#ifndef WEAK
#define WEAK __attribute__ ((weak))
#endif

#endif /** @} */
