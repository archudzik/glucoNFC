/*
 * Copyright 2016,2018-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @defgroup MODS_NSS_UARTTX_DFT Diversity settings
 * @ingroup MODS_NSS_UARTTX
 *
 * The application can adapt the Uart Tx-only module to better fit the different application scenarios through the use
 * of diversity flags in the form of defines below. Sensible defaults are chosen; to override the default settings,
 * place the defines with their desired values in the application app_sel.h header file: the compiler will pick up your
 * defines before parsing this file.
 *
 * These flags may be overridden:
 * - #UARTTX_STOPBITS
 * - #UARTTX_BITRATE
 * - #UARTTX_INVERTED
 * - #UARTTX_DEC_END_CHAR
 * - #UARTTX_HEX_END_CHAR
 * - #UARTTX_ENABLE_SHORTHANDS
 *
 * These defines are derived from #UARTTX_ENABLE_SHORTHANDS and may not be redefined in an application:
 * - PRINTD
 * - PRINTH
 * - PRINTS
 * - PRINTF
 *
 * @{
 */
#ifndef __UARTTX_DFT_H_
#define __UARTTX_DFT_H_

/**
 * By default, the uart module will use two stop bits.
 * Explicitly define @c UARTTX_STOPBITS to @c 1 to end each byte with only 1 stopbit.
 * @note If not defined, or if equal to any value different from @c 1, @c 2 stop bits will be used.
 */
#if !defined(UARTTX_STOPBITS)
    #define UARTTX_STOPBITS 2
#endif

/**
 * The uart module will use @c 9600 as default bit rate, assuming the default system clock of 500 kHz.
 * Explicitly define @c UARTTX_BITRATE to switch to a higher bit rate.
 * @warning
 *  Ensure valid bit rate/system clock combinations are used as listed in @ref NSS_CLOCK_RESTRICTIONS
 *  "SW Clock Restrictions" and @ref SSPClockRates_anchor "SSP clock rates".
 *  The Uart Tx module does not perform any check on this.
 */
#if !defined(UARTTX_BITRATE)
    #define UARTTX_BITRATE 9600
#endif

/**
 * In UART, the idle state is high. This means that a pull-up will be set to PIO9. This allows for energy injection
 * into the IO ring, which can cause problems when trying to go to Power-off. If defined to a non-zero value, the idle
 * state will be low and all bits will be transmitted inverted.
 * @note Enabling this requires an inverter, external to the IC, to transform the stream back to UART again.
 */
#if !defined(UARTTX_INVERTED)
    #define UARTTX_INVERTED 0
#endif

/* ------------------------------------------------------------------------- */

/**
 * Defines the extra character to output after printing out a decimal or hexadecimal number using #PRINTD.
 * @pre This define must be a single character.
 */
#if !defined(UARTTX_DEC_END_CHAR)
    #define UARTTX_DEC_END_CHAR '\n'
#endif

/**
 * Defines the extra character to output after printing out a decimal or hexadecimal number using #PRINTH.
 * @pre This define must be a single character.
 */
#if !defined(UARTTX_HEX_END_CHAR)
    #define UARTTX_HEX_END_CHAR '\n'
#endif

/**
 * Defines shorthands for quick debugging, using the uart to output strings.
 * A typical usage for this diversity setting could be to have these enabled in a debug build, but have them disabled
 * in a release build, still allowing the full functionality of the uart module for non-debug usages - using the full
 * API function names.
 */
#if !defined(UARTTX_ENABLE_SHORTHANDS)
    #ifdef DEBUG
        #define UARTTX_ENABLE_SHORTHANDS 1
    #else
        #define UARTTX_ENABLE_SHORTHANDS 0
    #endif
#endif
#if UARTTX_ENABLE_SHORTHANDS
    #define PRINTD(n) UartTx_PrintDec(n, UARTTX_DEC_END_CHAR) /**< A shorthand to quickly output a decimal integer as a string. */
    #define PRINTH(n) UartTx_PrintHex(n, UARTTX_HEX_END_CHAR) /**< A shorthand to quickly output a hexadecimal integer as a string. */
    #define PRINTS(s) UartTx_PrintString(s) /**< A shorthand to quickly output a string. */
    #define PRINTF(...) UartTx_Printf(__VA_ARGS__) /**< A shorthand to quickly format and output a string. */
#else
    #define PRINTD(n) do {} while(0) /**< void */
    #define PRINTH(n) do {} while(0) /**< void */
    #define PRINTS(s) do {} while(0) /**< void */
    #define PRINTF(...) do {} while(0) /**< void */
#endif

#endif /** @} */
