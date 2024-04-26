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

/**
 * Number of configurable internal inputs of the I2D MUX
 * To be used by the #ConvertI2DInputToIOCONAnabus function
 */
#define I2D_INT_INPUT_NUM 8

static int MaxCurrentInPicoAmpere(I2D_CONVERTER_GAIN_T converterGain);
static int ScalerGainInCenti(I2D_SCALER_GAIN_T scalerGain);

/* ------------------------------------------------------------------------- */

/** Provides the maximum allowed current in pA at the input of the I/F converter for a given configured gain. */
static int MaxCurrentInPicoAmpere(I2D_CONVERTER_GAIN_T converterGain)
{
    switch (converterGain) {
        case I2D_CONVERTER_GAIN_HIGH: return 50000; /* 50 nA */
        case I2D_CONVERTER_GAIN_LOW: return 2500000; /* 2.5 uA */
        default: return 2500000; /* 2.5 uA is HW default */
    }
}

/** Provides the scaler gain in an integer value (x100) for a given enum tag. */
static int ScalerGainInCenti(I2D_SCALER_GAIN_T scalerGain)
{
    switch (scalerGain) {
        case I2D_SCALER_GAIN_1_1: return 100; /* 1:1 Gain (Multiply by 1) */
        case I2D_SCALER_GAIN_1_2: return 200; /* 1:2 Gain (Multiply by 2) */
        case I2D_SCALER_GAIN_1_10: return 1000; /* 1:10 Gain (Multiply by 10) */
        case I2D_SCALER_GAIN_2_1: return 50; /* 2:1 Gain (Divide by 2) */
        case I2D_SCALER_GAIN_10_1: return 10; /* 10:1 Gain (Divide by 10) */
        case I2D_SCALER_GAIN_100_1: return 1; /* 100:1 Gain (Divide by 100) */
        case I2D_SCALER_GAIN_BYPASS: return 100; /* Scaler Bypass (Multiply by 1) */
        default: return 100; /* 1:1 Gain (Multiply by 1) is HW default */
    }
}

/* Conversion function to convert I2D_INPUT_T to IOCON_ANABUS_T */
IOCON_ANABUS_T ConvertI2DInputToIOCONAnabus(I2D_INPUT_T input)
{
    /* Up to 0x0FFF enum values are the same */
    IOCON_ANABUS_T result = (IOCON_ANABUS_T) (input & 0x0FFF);
    int i;

    /* Iterate over "input", bit by bit and build up "result" at even bit locations (0,2,4,...)
     * If an I2D internal input is selected in "input", the corresponding IOCON AnaBus in "result" shall be selected */
    input >>= 12;
    for (i = 0; i < I2D_INT_INPUT_NUM; i++) {
        result |= (IOCON_ANABUS_T)((input & 0x1) << (12+(i * 2)));
        input >>= 1;
    }
    return result;
}

/* ------------------------------------------------------------------------- */

/* Disable the Current to Digital converter */
void Chip_I2D_Init(NSS_I2D_T *pI2D)
{
    /* Enable power and clock to peripheral
     * Order is relevant because it prevents accessing registers that contain undefined values */
    Chip_SysCon_Peripheral_EnablePower(SYSCON_PERIPHERAL_POWER_I2D);
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_I2D);

    /* Set the Control Register to its default value
     * This ensures that a measurement always starts, when the I2D_Start function is called,
     * regardless of the previous state of the HW block */
    pI2D->CR = 0;

    /* Disable all interrupts for I2D and clear interrupt flag register */
    pI2D->IMSC = 0;
    pI2D->ICR = 0x7;

    /* Set the custom integration time calibration to default value
     * (value is not set by HW and I2D driver uses the custom integration time) */
    pI2D->SP2 = 999;

    /* Enforce default settings */
    Chip_I2D_Setup(pI2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_1_1, I2D_CONVERTER_GAIN_LOW, 100);
    Chip_I2D_SetMuxInput(pI2D, I2D_INPUT_NONE);

    /* Set thresholds to their default values */
    Chip_I2D_Int_SetThresholdLow(pI2D, 0);
    Chip_I2D_Int_SetThresholdHigh(pI2D, 0xFFFF);
}

/* Configure the I2D peripheral with the required settings */
void Chip_I2D_DeInit(NSS_I2D_T *pI2D)
{
    /* Disable all interrupts for I2D */
    pI2D->IMSC = 0;

    /* Disable clock and power to peripheral
     * Order is relevant because it prevents accessing registers that contain undefined values */
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_I2D);
    Chip_SysCon_Peripheral_DisablePower(SYSCON_PERIPHERAL_POWER_I2D);
}

/* Configure the I2D peripheral with the required settings */
void Chip_I2D_Setup(NSS_I2D_T *pI2D, I2D_MODE_T mode, I2D_SCALER_GAIN_T scalerGain, I2D_CONVERTER_GAIN_T converterGain, int converterTimeMs)
{
    Chip_I2D_SetMode(pI2D, mode);
    Chip_I2D_SetScalerGain(pI2D, scalerGain);
    Chip_I2D_SetConverterGain(pI2D, converterGain);
    Chip_I2D_SetConverterIntegrationTime(pI2D, converterTimeMs);
}

/* Set the I2D operating mode */
void Chip_I2D_SetMode(NSS_I2D_T *pI2D, I2D_MODE_T mode)
{
    pI2D->CR = ((pI2D->CR & (~(0x1u << 2))) | ((mode & 0x1u) << 2));
}

/* Get the I2D converter configured operating mode */
I2D_MODE_T Chip_I2D_GetMode(NSS_I2D_T *pI2D)
{
    return (I2D_MODE_T)((pI2D->CR >> 2) & 0x1);
}

/* Set the I2D scaler gain */
void Chip_I2D_SetScalerGain(NSS_I2D_T *pI2D, I2D_SCALER_GAIN_T scalerGain)
{
    pI2D->SP3 = scalerGain & 0xF;
}

/* Get the I2D configured scaler gain */
I2D_SCALER_GAIN_T Chip_I2D_GetScalerGain(NSS_I2D_T *pI2D)
{
    return (I2D_SCALER_GAIN_T)(pI2D->SP3 & 0xF);
}

/* Set the I2D internal converter gain */
void Chip_I2D_SetConverterGain(NSS_I2D_T *pI2D, I2D_CONVERTER_GAIN_T converterGain)
{
    pI2D->SP0 = ((pI2D->SP0 & (~(0x1u << 6))) | ((converterGain & 0x1u) << 6));
}

/* Get the I2D configured internal converter gain */
I2D_CONVERTER_GAIN_T Chip_I2D_GetConverterGain(NSS_I2D_T *pI2D)
{
    return (I2D_CONVERTER_GAIN_T)((pI2D->SP0 >> 6) & 0x1);
}

void Chip_I2D_SetConverterIntegrationTime(NSS_I2D_T *pI2D, int converterTimeMs)
{
    /* Always using custom integration time */
    pI2D->SP0 = ((pI2D->SP0 & (~0x7u)) | 0x7u);
    pI2D->SP1 = converterTimeMs & 0xFFFF;
}

int Chip_I2D_GetConverterIntegrationTime(NSS_I2D_T *pI2D)
{
    /* Ensure that custom integration time is selected, otherwise the returned value wouldn't be valid */
    ASSERT((pI2D->SP0 & 0x7) == 0x7);

    return pI2D->SP1 & 0xFFFF;
}

void Chip_I2D_SetMuxInput(NSS_I2D_T *pI2D, I2D_INPUT_T input)
{
    /*First unground the analog busses*/
    Chip_IOCON_UngroundAnabus(NSS_IOCON, ConvertI2DInputToIOCONAnabus(input));

    pI2D->MUX = input & 0xFFFFF;
}

I2D_INPUT_T Chip_I2D_GetMuxInput(NSS_I2D_T *pI2D)
{
    return (I2D_INPUT_T)(pI2D->MUX & 0xFFFFF);
}

void Chip_I2D_Start(NSS_I2D_T *pI2D)
{
    /* Making sure Stop bit(b1) is cleared*/
    pI2D->CR = (pI2D->CR & ~(0x1u << 1)) | (0x1u << 0);
}

void Chip_I2D_Stop(NSS_I2D_T *pI2D)
{
    pI2D->CR |= 0x1 << 1;
}

I2D_STATUS_T Chip_I2D_ReadStatus(NSS_I2D_T *pI2D)
{
    /* I2D_STATUS_CONVERTER_IN_OPERATION flag derived from Start bit */
    uint32_t result = (pI2D->CR & 0x1) << 8;

    /* I2D_STATUS_CONVERSION_DONE derived from RDY Interrupt Flag */
    result |= (pI2D->RIS & 0x1) << 9;

    /* HW status bits */
    result |= pI2D->SR & 0x3;

    return (I2D_STATUS_T)result;
}

int Chip_I2D_GetValue(NSS_I2D_T *pI2D)
{
    /* Read measured value into data - This clears RDY Interrupt Flag
     * Bit 16 (overflow) is masked off (use I2D_STATUS_T instead - I2D_STATUS_RANGE_TOO_HIGH bit) */
    return pI2D->DR & 0xFFFF;
}

int Chip_I2D_NativeToPicoAmpere(int native, I2D_SCALER_GAIN_T scalerGain, I2D_CONVERTER_GAIN_T converterGain, int converterTimeMs)
{
    /* PicoAmpere = ( MaxCurrent(converterGain) x MinConverterPeriod x native ) / ( scalerGain x converterTimeMs )
     *
     * Notes:
     * - Calculations with Current in pico Ampere and Time in microseconds
     * - MinConverterPeriod corresponds to the maximum frequency of 1 pulse every 4us
     * - scalerGain values are multiplied by 100, so as we divide by scalerGain we must also multiply by 100 */

    /* Ensure that converterTimeMs is 1 or higher */
    ASSERT(converterTimeMs > 0);

    /* Numerator */
    uint64_t numerator = (uint64_t)MaxCurrentInPicoAmpere(converterGain) * 4 * (uint32_t)native * 100;

    /* Denominator */
    uint64_t denominator = (uint64_t)ScalerGainInCenti(scalerGain) * (uint32_t)converterTimeMs * 1000;

    /* Casting safe because even for the lower integration times, the number of pulses (native) will always be limited by the maximum I/F converter
     * frequency of 1 pulse every 4 us
     * Adding denominator/2 to the numerator is a simple rounding operation */
    return (int)((numerator + denominator/2) / denominator);

}

int Chip_I2D_PicoAmpereToNative(int picoAmpere, I2D_SCALER_GAIN_T scalerGain, I2D_CONVERTER_GAIN_T converterGain, int converterTimeMs)
{
    /* Native = ( picoAmpere x converterTimeMs x scalerGain ) / ( MaxCurrent(converterGain) x MinConverterPeriod )
     *
     * Notes:
     * - Calculations with Current in pico Ampere and Time in microseconds
     * - MinConverterPeriod corresponds to maximum frequency of 1 pulse every 4us
     * - scalerGain values are multiplied by 100, so as we multiply by scalerGain we must also divide by 100 */

    /* Ensure that converterTimeMs is 1 or higher and picoAmpere is not negative */
    ASSERT(converterTimeMs > 0);

    /* Numerator */
    uint64_t numerator = (uint64_t)picoAmpere * (uint32_t)converterTimeMs * 1000 * (uint32_t)ScalerGainInCenti(scalerGain);

    /* Denominator */
    uint32_t denominator = (uint32_t)MaxCurrentInPicoAmpere(converterGain) * 4 * 100;

    /* Casting safe due to 0xFFFF cap
     * Adding denominator/2 to the numerator is a simple rounding operation */
    int native = (int)((numerator + denominator/2) / denominator);

    if (native > 0xFFFF) {
        /* Conversion settings caused an overflow of the 16bit native value */
        native = 0xFFFF;
    }

    return native;
}

void Chip_I2D_Int_SetThresholdLow(NSS_I2D_T *pI2D, int native)
{
    pI2D->TLO = native & 0xFFFF;
}

int Chip_I2D_Int_GetThresholdLow(NSS_I2D_T *pI2D)
{
    return (int)(pI2D->TLO & 0xFFFF);
}

void Chip_I2D_Int_SetThresholdHigh(NSS_I2D_T *pI2D, int native)
{
    pI2D->THI = native & 0xFFFF;
}

int Chip_I2D_Int_GetThresholdHigh(NSS_I2D_T *pI2D)
{
    return (int)(pI2D->THI & 0xFFFF);
}

void Chip_I2D_Int_SetEnabledMask(NSS_I2D_T *pI2D, I2D_INT_T mask)
{
    pI2D->IMSC = mask & 0x7;
}

I2D_INT_T Chip_I2D_Int_GetEnabledMask(NSS_I2D_T *pI2D)
{
    return (I2D_INT_T)(pI2D->IMSC & 0x7);
}

I2D_INT_T Chip_I2D_Int_GetRawStatus(NSS_I2D_T *pI2D)
{
    return (I2D_INT_T)(pI2D->RIS & 0x7);
}

void Chip_I2D_Int_ClearRawStatus(NSS_I2D_T *pI2D, I2D_INT_T flags)
{
    pI2D->ICR = flags & 0x7;
}
