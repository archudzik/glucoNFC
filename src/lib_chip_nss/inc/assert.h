/*
 * Copyright 2014-2017 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __ASSERT_H_
#define __ASSERT_H_

/**
 * @addtogroup CHIP_NSS
 * @{
 */

/**
 * If condition @c expr is false, the debugger halts (BKPT instruction), otherwise does nothing.
 * @param expr : the condition to check
 * @note If the macro @c DEBUG is not defined, the #ASSERT macro generates no code.
 * @note If the expression @c expr has side-effects, the program behaves differently depending on whether @c DEBUG is
 *  defined.
 */
#ifdef DEBUG
    #define ASSERT(expr) do { if (expr) {} else { __asm__("BKPT"); } } while (0)
#else
    #define ASSERT(expr) /* Prevent a compiler warning for unused variables */
#endif

#endif /** @} */
