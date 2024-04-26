/*
 * Copyright 2016-2017,2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include "sense_resistive_dft.h"
#include "sense/sense_specific.h"


static const GROUP_PROPERTIES_T sGroupProp[GROUP_COUNT] = GROUP_PROPERTIES;

static void Prepare(int group);
static int GetADC(ADCDAC_IO_T connection);
static int GetI2D(void);
static uint16_t SenseAdmittance(int group);


static uint32_t AdmittanceToPills(int group, uint16_t admittance, uint16_t calibration);
static int BitCount(unsigned int value);

void SenseSpecific_Init(void)
{
    Chip_ADCDAC_Init(NSS_ADCDAC0);
    Chip_ADCDAC_SetModeDAC(NSS_ADCDAC0, ADCDAC_CONTINUOUS);
    Chip_ADCDAC_SetModeADC(NSS_ADCDAC0, ADCDAC_SINGLE_SHOT);
    Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC0, ADCDAC_INPUTRANGE_WIDE);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_0, IOCON_FUNC_1);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_1);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_2, IOCON_FUNC_1);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_3, IOCON_FUNC_1);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_1);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_5, IOCON_FUNC_1);
    Chip_I2D_Init(NSS_I2D);
}

void SenseSpecific_DeInit(void)
{
    Chip_I2D_DeInit(NSS_I2D);
    Chip_ADCDAC_StopDAC(NSS_ADCDAC0);
    Chip_ADCDAC_StopADC(NSS_ADCDAC0);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_0, IOCON_FUNC_0);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_0);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_2, IOCON_FUNC_0);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_3, IOCON_FUNC_0);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_4, IOCON_FUNC_0);
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_5, IOCON_FUNC_0);
    Chip_ADCDAC_DeInit(NSS_ADCDAC0);
}

int SenseSpecific_GetAmountOfGroups(void)
{
    return GROUP_COUNT;
}

uint32_t SenseSpecific_GetPillsInGroup(int group, uint16_t* pStatus, bool calibrated)
{
    uint32_t pills;
    uint16_t admittance = SenseAdmittance(group);
    /* If not yet initialized, do so now.
     * Since we do not know about the absolute values of the resistors used, it is better to save a
     * calibration measurement at the beginning of the demo-board life cycle. At this moment we expect
     * all possible pills (as configured in #GROUP_PROPERTIES) to be present. */
    if (!calibrated) {
        uint16_t previous_admittance = 0xFFFF;

        /* Since measuring analog signals, theoretically it is possible that due to an external event,
         * we pick up unwanted signals which harm the accuracy of the measurement. Therefore we measure until two
         * consecutive measurements are in close range with each other.
         */
        while (previous_admittance < (admittance - 20) || previous_admittance > (admittance + 20)) {
            previous_admittance = admittance;
            admittance = SenseAdmittance(group);
        }
        *pStatus = admittance;
        pills = sGroupProp[group].pills;
    }
    else {
        pills = AdmittanceToPills(group, admittance, *pStatus);
    }
    return pills;
}

int SenseSpecific_InitialPillCount(int group)
{
    return BitCount(sGroupProp[group].pills);
}

bool SenseSpecific_GroupPositional(int group)
{
    return (sGroupProp[group].principle == SENSE_RES_SENS_PRINC_PAR_UNEVEN);
}

/**
 * Prepares a specific group for a measurement.
 * To be called before sensing a specific group.
 * @param group : group to be prepared
 */
static void Prepare(int group)
{
    Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC0, sGroupProp[group].DAC_drivePin);
    Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, 0xFFF);
    Chip_I2D_SetMuxInput(NSS_I2D, sGroupProp[group].I2D_senseInput);
    Chip_I2D_Setup(NSS_I2D, I2D_SINGLE_SHOT, I2D_SCALER_GAIN_100_1, I2D_CONVERTER_GAIN_LOW, 100);
    /* Wait a bit to make sure the levels are stable. */
    Chip_Clock_System_BusyWait_ms(1);
}


/**
 * Starts an ADC conversion from the given connection pin, waits for completion and returns the ADC value
 * @param connection : The ADC input to be measured.
 * @return The ADC conversion result in native value (12bits).
 */
static int GetADC(ADCDAC_IO_T connection)
{
    Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, connection);

    NSS_ADCDAC0->ICR = 0xF; /* Clear all interrupts */
    Chip_ADCDAC_StartADC(NSS_ADCDAC0);
    while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE)) {
        ; /* Wait until the conversion is finished. */
    }
    return Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
}

/**
 * Starts an I2D conversion, waits for completion and returns the I2D value
 * @pre proper I2D mux should be set.
 * @return The I2D conversion result in native value (12bits).
 */
static int GetI2D(void)
{
    Chip_I2D_Start(NSS_I2D);
    while (!(Chip_I2D_ReadStatus(NSS_I2D) & I2D_STATUS_CONVERSION_DONE)) {
        ; /* Wait until the conversion is finished. */
    }
    return Chip_I2D_GetValue(NSS_I2D);
}

/**
 * Performs the necessary measurements to calculate a group's admittance.
 * @param group : The group for which the admittance needs to be calculated.
 * @return The current Admittance value for @c group
 */
static uint16_t SenseAdmittance(int group)
{
    int drive;
    int sense;
    int adcDiff;
    int i2d;

    Prepare(group);
    drive = GetADC(sGroupProp[group].DAC_drivePin);
    sense = GetADC(sGroupProp[group].ADC_senseInput);
    adcDiff = drive - sense;

    /* In normal circumstances, adcDiff should never become 0 (or even negative), but nevertheless we
     * need to prevent a devision by 0, higher level will make sure that if a significant difference is measured,
     * (new pill removal) it restarts the measurement. */
    if (adcDiff < 1) {
        adcDiff = 1;
    }

    i2d = GetI2D();
    i2d <<= 4; /* To increase the resolution of the calculation. */
    return (uint16_t)((i2d + (adcDiff >> 1)) / adcDiff);
}

/**
 * Function to determine the amount of pills in a group based on its admittance.
 * @param group : The group for which this conversion is needed. This information is needed to determine the proper algorithm.
 * @param admittance : The newly calculated admittance.
 * @param calibration : The admittance calibration value (initial value at start of life time).
 */
static uint32_t AdmittanceToPills(int group, uint16_t admittance, uint16_t calibration)
{
    if (sGroupProp[group].principle == SENSE_RES_SENS_PRINC_PAR_EVEN) {
        /*
         * Pc: Pills Present at time of calibration.
         * Cg: Group calibration value (this is the value of the first measurement taken (with all pills present)).
         * A: Admittance currently measured in the network.
         * note: There is a fixed resistor in the network, therefore we have to count this one in.
         * note: Small rounding trick by adding half of the divisor to the denominator.
         * Remaining pills = (((Pc + 1) * A + Cg/2) / Cg) -1
         */
        int pills = ((1 + BitCount(sGroupProp[group].pills)) * admittance + (calibration >> 1)) / calibration - 1;
        return (uint32_t)(1 << (pills)) - 1;
    }

    else { /* sGroupProp[group].principle == SENSE_RES_SENS_PRINC_PAR_UNEVEN */
            /* Perform a bitmap calculation
             * Pc: Bitmap of pills Present at time of calibration.
             * Cg: Group calibration value (this is the value of the first measurement taken (with all pills present)).
             * A: Admittance currently measured in the network.
             * note: There is a fixed resistor in the network, therefore we have to count this one in by adding a set bit.
             * note: Small rounding trick by adding half of the divisor to the denominator.
             * Remaining pills bitmap = ((((Pc << 1) + 1) * A + Cg/2) / Cg) >> 1
             */
        uint32_t pills = (uint32_t) ((((sGroupProp[group].pills << 1) + 1) * admittance + (calibration >> 1)) / calibration) >> 1;
        return pills;
    }
}

/**
 * Counting bits set, Peter Wegner's or Brian Kernighan's way
 * @param value Count the number of bits set in @c value
 * @return The total bits set in @c value
 */
static int BitCount(unsigned int value)
{
    int count;
    for (count = 0; value; count++) { /* goes through as many iterations as there are set bits */
        value &= value - 1; /* clear the least significant bit set */
    }
    return count;
}
