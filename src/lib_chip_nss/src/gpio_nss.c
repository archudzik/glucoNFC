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

#include "chip.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
void Chip_GPIO_Init(NSS_GPIO_T *pGPIO)
{
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_GPIO);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void Chip_GPIO_DeInit(NSS_GPIO_T *pGPIO)
{
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_GPIO);
}

void Chip_GPIO_SetPinDIR(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin, bool output)
{
    if (output) {
        Chip_GPIO_SetPinDIROutput(pGPIO, port, pin);
    }
    else {
        Chip_GPIO_SetPinDIRInput(pGPIO, port, pin);
    }
}

void Chip_GPIO_SetPortDIR(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask, bool outSet)
{
    if (outSet) {
        Chip_GPIO_SetPortDIROutput(pGPIO, port, pinMask);
    }
    else {
        Chip_GPIO_SetPortDIRInput(pGPIO, port, pinMask);
    }
}

void Chip_GPIO_SetupPinInt(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin, GPIO_INT_MODE_T mode)
{
    uint32_t pinMask = (1u << pin);

    /* Edge mode selected? */
    if ((uint32_t) mode & 0x2) {
        Chip_GPIO_SetPinModeEdge(pGPIO, port, pinMask);

        /* Interrupt on both edges selected? */
        if ((uint32_t) mode & 0x4) {
            Chip_GPIO_SetEdgeModeBoth(pGPIO, port, pinMask);
        }
        else {
            Chip_GPIO_SetEdgeModeSingle(pGPIO, port, pinMask);
        }
    }
    else {
        /* Level mode */
        Chip_GPIO_SetPinModeLevel(pGPIO, port, pinMask);
    }

    /* Level selections will not alter 'dual edge' mode */
    if ((uint32_t) mode & 0x1) {
        /* High edge or level mode selected */
        Chip_GPIO_SetModeHigh(pGPIO, port, pinMask);
    }
    else {
        Chip_GPIO_SetModeLow(pGPIO, port, pinMask);
    }
}
