/*
 * Copyright 2016-2017 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include "sense_gpio_dft.h"
#include "sense/sense_specific.h"

static const GROUP_PROPERTIES_T sGroupProp[GROUP_COUNT] = GROUP_PROPERTIES;

void SenseSpecific_Init(void)
{
    Chip_GPIO_Init(NSS_GPIO);
    /* As default state for all pins we define default state to be GPIO */
    for (int pin = IOCON_PIO0_0; pin <= IOCON_PIO0_11; pin++) {
        Chip_IOCON_SetPinConfig(NSS_IOCON, pin, IOCON_FUNC_0);
    }
    Chip_GPIO_SetPortDIRInput(NSS_GPIO, 0, 0xFFF);
}

void SenseSpecific_DeInit(void)
{
    Chip_GPIO_DeInit(NSS_GPIO);
}

int SenseSpecific_GetAmountOfGroups(void)
{
    return GROUP_COUNT;
}

bool SenseSpecific_GroupPositional(int group){
    /* In the GPIO usecase groups are always positional. */
    (void)group; /* suppress [-Wunused-parameter]: argument not needed in this specific implementation, just ignore. */
    return true;
}

uint32_t SenseSpecific_GetPillsInGroup(int group, uint16_t* pStatus, bool initialized)
{
    uint32_t pills = 0;
    IOCON_PIN_T pin;

    (void)pStatus; /* suppress [-Wunused-parameter]: argument not needed in this specific implementation, just ignore. */
    (void)initialized; /* suppress [-Wunused-parameter]: argument not needed in this specific implementation, just ignore. */

    /* Set the drive pin low */
    Chip_GPIO_SetPinDIROutput(NSS_GPIO, 0, sGroupProp[group].IO_drivePin);
    Chip_GPIO_SetPinOutLow(NSS_GPIO, 0, sGroupProp[group].IO_drivePin);

    /* Check all sense pins, 1 by 1 to not have a to big voltage drop. */
    for (int i = 0; i < sGroupProp[group].pills; i++) {
        pin = sGroupProp[group].IO_sensePin[i];
        Chip_IOCON_SetPinConfig(NSS_IOCON, pin, IOCON_FUNC_0 | IOCON_RMODE_PULLUP);

        /* If pin is low, it means the pill is still present. */
        if (!Chip_GPIO_GetPinState(NSS_GPIO, 0, pin)) {
            pills |= (uint32_t)(1U << i);
        }
        /* Remove pull-up */
        Chip_IOCON_SetPinConfig(NSS_IOCON, pin, IOCON_FUNC_0);

#ifdef DEBUG
        /* If pin is SWD  set it back to this. */
        if (IOCON_PIO0_10 == pin || IOCON_PIO0_11 == pin) {
            Chip_IOCON_SetPinConfig(NSS_IOCON, pin, IOCON_FUNC_2);
        }
#endif
    }

    /* Set drive line back as input. */
    Chip_GPIO_SetPinDIRInput(NSS_GPIO, 0, sGroupProp[group].IO_drivePin);

    return pills;
}

int SenseSpecific_InitialPillCount(int group)
{
    return sGroupProp[group].pills;
}
