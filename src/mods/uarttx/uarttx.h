/*
 * Copyright 2016,2018-2019,2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __UARTTX_NSS_H_
#define __UARTTX_NSS_H_

/**
 * @defgroup MODS_NSS_UARTTX uarttx: Uart Tx-only module
 * @ingroup MODS_NSS
 * The Uart Tx module provides a way to transmit byte sequences over the SSP interface using the UART protocol.
 * This module will hide all details of the SPI driver: it will initialize and configure the driver. The user of
 * the Uart module only needs to use the API functions below to have tracing functionality over a UART interface.
 *
 * @par Diversity
 *  This module supports diversity, to ease the transmission of data according to each application's needs.
 *  Check @ref MODS_NSS_UARTTX_DFT for all diversity parameters.
 *
 * @par Setup
 *  - Connect the MOSI pin (PIO9) to the RX pin of e.g. an FTDI FT232R.
 * @note The unused SSP pins (PIOs 2, 6 and 8) can still be used as GPIO for other purposes
 *
 * @par Warning
 *  When this module is in use, it is not possible to directly access the SPI driver for either transmission or
 *  reception. Doing so can result in communication errors or even a hard fault.
 *
 * @par Example 1
 *  @snippet uarttx_mod_example_1.c uarttx_mod_example_1
 *
 * @{
 */

#include "uarttx/uarttx_dft.h"

#ifndef NUL
    #define NUL (char)0 /**< The string demarcation character. */
#endif

/* ------------------------------------------------------------------------- */

/**
 * Initializes the module.
 * @pre This is the module's first function to be called after startup or after #UartTx_DeInit has been called.
 * @pre The IOCON driver must have been initialized beforehand. This is already taken care of by the board library
 *  initialization function #Board_Init.
 * @pre The PIO9 pin is configured for the SSP MOSI functional mode (FUNC_1) using #Chip_IOCON_SetPinConfig.
 * @see Chip_IOCON_Init
 */
void UartTx_Init(void);

/**
 * De-initializes the module.
 * - De-initializes the SSP driver
 * - Configures PIO9 - MOSI - as GPIO with a pull-up or -down, according to #BOARD_PIO9_PULL
 */
void UartTx_DeInit(void);

/**
 * Transmits the given data over the MOSI line, mimicking the UART protocol.
 * This call is blocking until all data has been sent.
 * @param pData : Must point to a contiguous linear array containing the bytes to transmit.
 * @param length : The number of bytes to transmit.
 * @see MODS_NSS_UART_DFT
 */
void UartTx_Tx(const uint8_t * pData, unsigned int length);

/* ------------------------------------------------------------------------- */

/**
 * Convenience function to print a string. Internally, #UartTx_Tx is used.
 * This call is blocking until all data has been sent.
 * @param s : A NUL-terminated string to print.
 * @see PRINTS
 */
void UartTx_PrintString(const char * s);

/**
 * Convenience function to print a number in decimal format. Internally, #UartTx_PrintString is used.
 * Right after the number, the @c end character is printed out.
 * This call is blocking until all data has been sent.
 * @param n : A signed integer to print.
 * @param end : The character to print after the number. May be @c NUL, in which case nothing is printed out after the
 *  number.
 * @see PRINTD
 */
void UartTx_PrintDec(int n, char end);

/**
 * Convenience function to print a number in hexadecimal format. No prefix (like 0x) or suffix (like h) is printed.
 * Right after the number, the @c end character is printed out.
 * Internally, #UartTx_PrintString is used.
 * This call is blocking until all data has been sent.
 * @param n : An unsigned integer to print.
 * @param end : The character to print after the number. May be @c NUL, in which case nothing is printed out after the
 * 	number.
 * @see PRINTH
 */
void UartTx_PrintHex(unsigned int n, char end);

/**
 * Convenience function to print a string and a variable amount of arguments, replacing the % placeholders.
 * Internally, #UartTx_Tx is used.
 * @note This function does not require the inclusion of an extra library.
 * @note Only a subset of the full standard @c printf functionality is supported. @b All formatting options are skipped,
 *  and @b only @c %s, @c %d and @c %X are implemented. @c %x is aliased to @c %X.
 * This call is blocking until all data has been sent.
 * @param fmt : The unformatted NUL-terminated string to format and to print.
 * @see PRINTF
 */
void UartTx_Printf(const char * fmt, ...);

#endif /** @} */
