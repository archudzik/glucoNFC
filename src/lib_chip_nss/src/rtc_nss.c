/*
 * Copyright 2014-2019,2021 NXP
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

static volatile int Chip_RTC_AccessCounter;

/** Macro to read an RTC register */
#define RTC_READ(pReg) Chip_BusSync_ReadReg(&NSS_RTC->ACCSTAT, &Chip_RTC_AccessCounter, (pReg))

/** Macro to write an RTC register */
#define RTC_WRITE(pReg, value) Chip_BusSync_WriteReg(&NSS_RTC->ACCSTAT, &Chip_RTC_AccessCounter, (pReg), (value))

/* ------------------------------------------------------------------------- */

// Enables ARM access to RTC block via APB clock
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Chip_RTC_Init(NSS_RTC_T * pRTC)
{
    // Enable APB access to RTC domain
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_RTC);
}

void Chip_RTC_DeInit(NSS_RTC_T *pRTC)
{
    // 1. Stop RTC "wake-up down-counter"
    RTC_WRITE(&pRTC->CR, RTC_WAKEUPCTRL_DISABLE);

    // 2. Disable 'wake-up' interrupt 
    RTC_WRITE(&pRTC->IMSC, RTC_INT_NONE);

    // 3. Disable RTC block register access from APB
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_RTC);
}

void Chip_RTC_SetCalibration(NSS_RTC_T *pRTC, int calibValue)
{
    RTC_WRITE(&pRTC->CAL, calibValue & 0x0000FFFF);
}

int Chip_RTC_GetCalibration(NSS_RTC_T *pRTC)
{   // return 16bit masked result
    return 0x0000FFFF & RTC_READ(&pRTC->CAL);
}

void Chip_RTC_Wakeup_SetControl(NSS_RTC_T *pRTC, RTC_WAKEUPCTRL_T control)
{
    RTC_WRITE(&pRTC->CR, (int) control & 0x07);
}

RTC_WAKEUPCTRL_T Chip_RTC_Wakeup_GetControl(NSS_RTC_T *pRTC)
{
    return (RTC_WAKEUPCTRL_T) (RTC_READ(&pRTC->CR) & 0x7);
}

void Chip_RTC_Wakeup_SetReload(NSS_RTC_T *pRTC, int ticks)
{
    RTC_WRITE(&pRTC->SLEEPT, 0xFFFFFF & ticks);
}

int Chip_RTC_Wakeup_GetReload(NSS_RTC_T *pRTC)
{
    return 0x00FFFFFF & RTC_READ(&pRTC->SLEEPT);
}

int Chip_RTC_Wakeup_GetRemaining(NSS_RTC_T *pRTC)
{
    return 0x00FFFFFF & RTC_READ(&pRTC->VAL);
}

bool Chip_RTC_Wakeup_IsRunning(NSS_RTC_T *pRTC)
{
    /* Due to an artifact with this HW block, we need to first trigger a "clear" of the register by writing to it */
    RTC_WRITE(&pRTC->SR, 0xFF);

    return (RTC_READ(&pRTC->SR) & (0x1u << 3)) != 0;
}

int Chip_RTC_Time_GetValue(NSS_RTC_T *pRTC)
{
    return (int)RTC_READ(&pRTC->TIME);
}

void Chip_RTC_Time_SetValue(NSS_RTC_T *pRTC, int tickValue)
{
    RTC_WRITE(&pRTC->TIME, (uint32_t)tickValue);
}

void Chip_RTC_Int_SetEnabledMask(NSS_RTC_T *pRTC, RTC_INT_T mask)
{
    RTC_WRITE(&pRTC->IMSC, RTC_INT_ALL & mask);
}

RTC_INT_T Chip_RTC_Int_GetEnabledMask(NSS_RTC_T *pRTC)
{
    return (RTC_INT_T) (RTC_INT_ALL & RTC_READ(&pRTC->IMSC));
}

RTC_INT_T Chip_RTC_Int_GetRawStatus(NSS_RTC_T *pRTC)
{
    return (RTC_INT_T) (RTC_INT_ALL & RTC_READ(&pRTC->RIS));
}

void Chip_RTC_Int_ClearRawStatus(NSS_RTC_T *pRTC, RTC_INT_T flags)
{
    RTC_WRITE(&pRTC->ICR, RTC_INT_ALL & flags);
}
