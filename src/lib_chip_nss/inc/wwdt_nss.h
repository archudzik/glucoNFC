/*
 * Copyright 2014-2017,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __WWDT_NSS_H_
#define __WWDT_NSS_H_

/** @defgroup WWDT_NSS wwdt: Windowed Watchdog Timer driver
 * @ingroup DRV_NSS
 * This driver provides a simplified API to the WWDT hardware block. The WWDT driver provides the following
 * functionalities:
 *  - Configuring the block in reset mode.
 *  - Configuring the expiry timeout.
 *  - Starting the watchdog timer.
 *  - Feeding the watchdog timer.
 *  If you require interrupt mode, need to check the status, or need to do anything else than described above,
 *  you must interact with the #NSS_WWDT register block directly.
 *
 * The WWDT hardware block controls a special timer which can perform a configured action on expiry. This can
 * be used to recover from an anomaly in software: for example an infinite loop or an endless wait for an event or
 * interrupt.
 * The WWDT can be configured to work in 2 modes as given below.
 *  - Reset mode: This mode is the normal intended operation mode and causes an unpreventable chip reset on WWDT expiry.
 *      @warning The watchdog reset status is cleared on system reset. It is thus not possible to distinguish between a
 *          hard reset after toggling the RESETN pin and a WWDT timeout.
 *
 *  - Interrupt mode: This mode can be used for debugging purposes without actually resetting the device on WWDT expiry.
 *      The WWDT interrupt flag cannot be cleared by SW after a WWDT expiry: the firmware must disable the WWDT
 *      interrupt inside the interrupt handler using #NVIC_DisableIRQ with #WDT_IRQn as argument.
 *      @note This mode is not supported by this driver.
 *      @warning Using the interrupt mode will not protect against hangs inside an interrupt handler which has a higher
 *          priority. See #IRQn_Type for a full list of interrupts and their priority.
 *
 * @par To configure and start the WWDT:
 *  -# Call #Chip_WWDT_Start.
 *
 * @par To reset the expiry timer and prevent a reset:
 *  -# Call #Chip_WWDT_Feed periodically.
 *
 * @par Example - WWDT
 *  @snippet wwdt_nss_example_1.c wwdt_nss_example_1
 *
 * @{
 */

/** WWDT register block structure */
typedef struct NSS_WWDT_S {
    __IO uint32_t MOD; /*!< WWDT mode register. This register contains the basic mode and status of the WWDT Timer. */
    __IO uint32_t TC; /*!< WWDT timer constant register. This register determines the time-out value. */
    __O uint32_t FEED; /*!< WWDT feed sequence register. Writing 0xAA followed by 0x55 to this register reloads the
                            WWDT timer with the value contained in WDTC. */
    __I uint32_t TV; /*!< WWDT timer value register. This register reads out the current value of the WWDT timer. */
} NSS_WWDT_T;

#define WWDT_WDMOD_BITMASK ((uint32_t) 0x0FUL) /**< WWDT Mode Bitmask */
#define WWDT_WDMOD_WDEN ((uint32_t) (1 << 0)) /**< WWDT enable bit */
#define WWDT_WDMOD_WDRESET ((uint32_t) (1 << 1)) /**< WWDT reset enable bit */
#define WWDT_WDMOD_WDTOF ((uint32_t) (1 << 2)) /**< WWDT time out flag bit */
#define WWDT_WDMOD_WDINT ((uint32_t) (1 << 3)) /**< WWDT interrupt flag bit */

/**
 * Initialize, configure and start the WWDT hardware block. All necessary calls to the clock driver will also be made.
 * @param timeout The maximum allowed time in number of seconds between two calls to #Chip_WWDT_Feed.
 *  When this function leaves, the application has @c timeout seconds before it must call #Chip_WWDT_Feed.
 * @pre This function may only be called once after a reset.
 */
void Chip_WWDT_Start(uint32_t timeout);

/**
 * Feed the WWDT timer. Calling this function pushes back a reset for the number of seconds as given in a previous call
 * to #Chip_WWDT_Start. The application must continue calling this function periodically within the configured timeout
 * period; failure to do so will cause a reset.
 */
void Chip_WWDT_Feed(void);

/**
 * @}
 */

#endif
