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

#include "chip.h"

/* ------------------------------------------------------------------------- */

#define ADCDAC_CR_ADC_START                 (1u << 0)
#define ADCDAC_CR_DAC_START                 (1u << 1)
#define ADCDAC_CR_ADC_CONT                  (1u << 2)
#define ADCDAC_CR_DAC_CONT                  (1u << 3)
#define ADCDAC_CR_ADC_STOP                  (1u << 4)
#define ADCDAC_CR_DAC_STOP                  (1u << 5)
#define ADCDAC_CR_DEFAULT                   (0u) /**< Not started, single-shot */
#define ADCDAC_SP0_DEFAULT                  (1u << 5) /*< SLOWCLK bit must be always set for best ADC linearity */

#define ADCDAC_ADC_VALUE_MASK               (0xFFF) /**< Input/Output resolution is 12-bits */
#define ADCDAC_DAC_VALUE_MASK               (0xFFF) /**< Input/Output resolution is 12-bits */
#define ADCDAC_ADC_VALUE_LOW                (0)
#define ADCDAC_ADC_VALUE_HIGH               ADCDAC_ADC_VALUE_MASK
#define ADCDAC_DAC_VALUE_LOW                (0)
#define ADCDAC_DAC_VALUE_HIGH               ADCDAC_DAC_VALUE_MASK

/**
 * Number of configurable internal inputs of the ADCDAC MUXes
 * Used by #ConvertADCDACIOToIOCONAnabus
 */
#define I2D_INT_INPUT_NUM 8

static IOCON_ANABUS_T ConvertADCDACIOToIOCONAnabus(ADCDAC_IO_T connection);

/* ------------------------------------------------------------------------- */

/** Conversion function to convert I2D_INPUT_T to IOCON_ANABUS_T */
static IOCON_ANABUS_T ConvertADCDACIOToIOCONAnabus(ADCDAC_IO_T connection)
{
    /* Up to 0x0FFF enum values are the same */
    IOCON_ANABUS_T result = (IOCON_ANABUS_T) connection & 0x0FFF;
    int i;

    /* Iterate over "input", bit by bit and build up "result" at even bit locations (0,2,4,...)
     * If an I2D internal input is selected in "input", the corresponding IOCON AnaBus in "result" shall be selected */
    connection >>= 12;
    for (i = 0; i < I2D_INT_INPUT_NUM; i++) {
        result |= (IOCON_ANABUS_T)((connection & 0x1) << (12+(i * 2)));
        connection >>= 1;
    }
    return result;
}

/* ------------------------------------------------------------------------- */

void Chip_ADCDAC_Init(NSS_ADCDAC_T *pADCDAC)
{
    /* Power up ADC/DAC */
    Chip_SysCon_Peripheral_EnablePower(SYSCON_PERIPHERAL_POWER_ADCDAC);
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_ADCDAC);

    /* Disable & clear interrupt */
    Chip_ADCDAC_Int_SetEnabledMask(pADCDAC, ADCDAC_INT_NONE);
    Chip_ADCDAC_Int_ClearRawStatus(pADCDAC, ADCDAC_INT_ALL);

    /* Reset Registers */
    pADCDAC->CR = ADCDAC_CR_DEFAULT;
    pADCDAC->SP0 = ADCDAC_SP0_DEFAULT;
    pADCDAC->TLO = 0;
    pADCDAC->THI = 0xFFFF;

    /* Reset IO Connection */
    Chip_ADCDAC_SetMuxADC(pADCDAC, ADCDAC_IO_NONE);
    Chip_ADCDAC_SetMuxDAC(pADCDAC, ADCDAC_IO_NONE);
}

void Chip_ADCDAC_DeInit(NSS_ADCDAC_T *pADCDAC)
{
    /* Disable interrupt */
    Chip_ADCDAC_Int_SetEnabledMask(pADCDAC, ADCDAC_INT_NONE);

    /* Disconnect */
    Chip_ADCDAC_SetMuxADC(pADCDAC, ADCDAC_IO_NONE);
    Chip_ADCDAC_SetMuxDAC(pADCDAC, ADCDAC_IO_NONE);

    /* Power Down ADC/DAC */
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_ADCDAC);
    Chip_SysCon_Peripheral_DisablePower(SYSCON_PERIPHERAL_POWER_ADCDAC);
}

void Chip_ADCDAC_SetModeADC(NSS_ADCDAC_T *pADCDAC, ADCDAC_MODE_T mode)
{
    pADCDAC->CR = (pADCDAC->CR & (~ADCDAC_CR_ADC_CONT)) | ((mode == ADCDAC_CONTINUOUS) ? ADCDAC_CR_ADC_CONT : 0);
}

void Chip_ADCDAC_SetModeDAC(NSS_ADCDAC_T *pADCDAC, ADCDAC_MODE_T mode)
{
    pADCDAC->CR = (pADCDAC->CR & (~ADCDAC_CR_DAC_CONT)) | ((mode == ADCDAC_CONTINUOUS) ? ADCDAC_CR_DAC_CONT : 0);
}

ADCDAC_MODE_T Chip_ADCDAC_GetModeADC(NSS_ADCDAC_T *pADCDAC)
{
    return (pADCDAC->CR & ADCDAC_CR_ADC_CONT) ? ADCDAC_CONTINUOUS : ADCDAC_SINGLE_SHOT;
}

ADCDAC_MODE_T Chip_ADCDAC_GetModeDAC(NSS_ADCDAC_T *pADCDAC)
{
    return (pADCDAC->CR & ADCDAC_CR_DAC_CONT) ? ADCDAC_CONTINUOUS : ADCDAC_SINGLE_SHOT;
}

void Chip_ADCDAC_SetMuxADC(NSS_ADCDAC_T *pADCDAC, ADCDAC_IO_T connection)
{
    /*First unground the analog busses*/
    Chip_IOCON_UngroundAnabus(NSS_IOCON, ConvertADCDACIOToIOCONAnabus(connection & 0x0FFFFF));

    pADCDAC->ADCMUX = connection & 0x0FFFFF;
}

void Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC_T *pADCDAC, ADCDAC_IO_T connection)
{
    /*First unground the analog busses*/
    Chip_IOCON_UngroundAnabus(NSS_IOCON, ConvertADCDACIOToIOCONAnabus(connection & 0x0FFFFF));

    pADCDAC->DACMUX = connection & 0x0FFFFF;
}

ADCDAC_IO_T Chip_ADCDAC_GetMuxADC(NSS_ADCDAC_T *pADCDAC)
{
    return (ADCDAC_IO_T) (pADCDAC->ADCMUX & 0x0FFFFF);
}

ADCDAC_IO_T Chip_ADCDAC_GetMuxDAC(NSS_ADCDAC_T *pADCDAC)
{
    return (ADCDAC_IO_T) (pADCDAC->DACMUX & 0x0FFFFF);
}

void Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC_T *pADCDAC, ADCDAC_INPUTRANGE_T inputRange)
{
    pADCDAC->SP0 = (pADCDAC->SP0 & (~(0x1u << 1))) | (uint32_t)((inputRange & 0x1) << 1);
}

ADCDAC_INPUTRANGE_T Chip_ADCDAC_GetInputRangeADC(NSS_ADCDAC_T *pADCDAC)
{
    return (ADCDAC_INPUTRANGE_T)((pADCDAC->SP0 >> 1) & 0x1);
}

void Chip_ADCDAC_StartADC(NSS_ADCDAC_T *pADCDAC)
{
    pADCDAC->CR = (pADCDAC->CR & ~ADCDAC_CR_ADC_STOP) | ADCDAC_CR_ADC_START;
}

void Chip_ADCDAC_StartDAC(NSS_ADCDAC_T *pADCDAC)
{
    /* Stop (continuous) ADC in case it's been started in another thread/interrupt */
    pADCDAC->CR = (pADCDAC->CR & ~ADCDAC_CR_DAC_STOP) | ADCDAC_CR_DAC_START;
}

void Chip_ADCDAC_StopADC(NSS_ADCDAC_T *pADCDAC)
{
    pADCDAC->CR |= ADCDAC_CR_ADC_STOP;
}

void Chip_ADCDAC_StopDAC(NSS_ADCDAC_T *pADCDAC)
{
    pADCDAC->CR |= ADCDAC_CR_DAC_STOP;
}

ADCDAC_STATUS_T Chip_ADCDAC_ReadStatus(NSS_ADCDAC_T *pADCDAC)
{
    uint32_t reg;
    int status = 0; /* Idle */

    /* Busy */
    reg = pADCDAC->CR;
    if ((reg & ADCDAC_CR_ADC_START) || (reg & ADCDAC_CR_DAC_START)) {
        status |= ADCDAC_STATUS_CONVERTER_IN_OPERATION;
    }

    reg = pADCDAC->RIS;

    /* Done */
    if (reg & ADCDAC_INT_CONVERSION_RDY_DAC) status |= ADCDAC_STATUS_DAC_DONE;
    if (reg & ADCDAC_INT_CONVERSION_RDY_ADC) status |= ADCDAC_STATUS_ADC_DONE;

    return (ADCDAC_STATUS_T) status;
}

int Chip_ADCDAC_GetValueADC(NSS_ADCDAC_T *pADCDAC)
{
    return pADCDAC->ADCDR & ADCDAC_ADC_VALUE_MASK;
}

void Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC_T *pADCDAC, int native)
{
    /* Truncate native */
    if (native > ADCDAC_DAC_VALUE_HIGH) native = ADCDAC_DAC_VALUE_HIGH;
    if (native < ADCDAC_DAC_VALUE_LOW) native = ADCDAC_DAC_VALUE_LOW;

    pADCDAC->DACDR = native & ADCDAC_DAC_VALUE_MASK;

    /* Auto start */
    Chip_ADCDAC_StartDAC(pADCDAC);
}

void Chip_ADCDAC_Int_SetThresholdLowADC(NSS_ADCDAC_T *pADCDAC, int native)
{
    pADCDAC->TLO = native & ADCDAC_ADC_VALUE_MASK;
}

void Chip_ADCDAC_Int_SetThresholdHighADC(NSS_ADCDAC_T *pADCDAC, int native)
{
    pADCDAC->THI = native & ADCDAC_ADC_VALUE_MASK;
}

int Chip_ADCDAC_Int_GetThresholdLowADC(NSS_ADCDAC_T *pADCDAC)
{
    return pADCDAC->TLO & 0xFFFF;
}

int Chip_ADCDAC_Int_GetThresholdHighADC(NSS_ADCDAC_T *pADCDAC)
{
    return pADCDAC->THI & 0xFFFF;
}

void Chip_ADCDAC_Int_SetEnabledMask(NSS_ADCDAC_T *pADCDAC, ADCDAC_INT_T mask)
{
    pADCDAC->IMSC = mask & 0x0F;
}

ADCDAC_INT_T Chip_ADCDAC_Int_GetEnabledMask(NSS_ADCDAC_T *pADCDAC)
{
    return (ADCDAC_INT_T) (pADCDAC->IMSC & ADCDAC_INT_ALL);
}

ADCDAC_INT_T Chip_ADCDAC_Int_GetRawStatus(NSS_ADCDAC_T *pADCDAC)
{
    return (ADCDAC_INT_T) (pADCDAC->RIS & ADCDAC_INT_ALL);
}

void Chip_ADCDAC_Int_ClearRawStatus(NSS_ADCDAC_T *pADCDAC, ADCDAC_INT_T flags)
{
    pADCDAC->ICR = flags & ADCDAC_INT_ALL;
}
