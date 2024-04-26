/*
 * Copyright 2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "batimp.h"

/* Count the number of enabled pins, and map the pin defines to an order, so we can set the levels to the enabled pins
 * only. Used in SetLevel.
 */
#if BATIMP_USE_PIO3
    #define FIRST_PIN IOCON_PIO0_3
    #if BATIMP_USE_PIO7
        #define SECOND_PIN IOCON_PIO0_7
        #if BATIMP_USE_PIO10
            #define THIRD_PIN IOCON_PIO0_10
            #if BATIMP_USE_PIO11
                #define FOURTH_PIN IOCON_PIO0_11
                #define PIN_COUNT 4
            #else
                #define PIN_COUNT 3
            #endif
        #elif BATIMP_USE_PIO11
            #define THIRD_PIN IOCON_PIO0_11
            #define PIN_COUNT 3
        #else
            #define PIN_COUNT 2
        #endif
    #elif BATIMP_USE_PIO10
        #define SECOND_PIN IOCON_PIO0_10
        #if BATIMP_USE_PIO11
            #define THIRD_PIN IOCON_PIO0_11
            #define PIN_COUNT 3
        #else
            #define PIN_COUNT 2
        #endif
    #elif BATIMP_USE_PIO11
        #define SECOND_PIN IOCON_PIO0_11
        #define PIN_COUNT 2
    #else
        #define PIN_COUNT 1
    #endif
#elif BATIMP_USE_PIO7
    #define FIRST_PIN IOCON_PIO0_7
    #if BATIMP_USE_PIO10
        #define SECOND_PIN IOCON_PIO0_10
        #if BATIMP_USE_PIO11
            #define THIRD_PIN IOCON_PIO0_11
            #define PIN_COUNT 3
        #else
            #define PIN_COUNT 2
        #endif
    #elif BATIMP_USE_PIO11
        #define SECOND_PIN IOCON_PIO0_11
        #define PIN_COUNT 3
    #else
        #define PIN_COUNT 1
    #endif
#elif BATIMP_USE_PIO10
    #define FIRST_PIN IOCON_PIO0_10
    #if BATIMP_USE_PIO11
        #define SECOND_PIN IOCON_PIO0_11
        #define PIN_COUNT 2
    #else
        #define PIN_COUNT 1
    #endif
#elif BATIMP_USE_PIO11
    #define FIRST_PIN IOCON_PIO0_11
    #define PIN_COUNT 1
#else
    #define PIN_COUNT 0
#endif

#if PIN_COUNT == 0
    #error PIN_COUNT must be greater than 0
#elif PIN_COUNT == 1
    #define NUM_LEVELS 4
#elif PIN_COUNT == 2
    #define NUM_LEVELS 10
#elif PIN_COUNT == 3
    #define NUM_LEVELS 20
#elif PIN_COUNT == 4
    #define NUM_LEVELS 35
#else
    #error PIN_COUNT must be less than or equal to 4
#endif

/* The internal current consumption of the current drive mode for a pin is different for 3 regions of settings:
 * IHO/ILO 0 .. 31
 * IHO/ILO 32 .. 63
 * IHO/ILO 64 .. 255
 */
#define IQ_DAC_0_31 705 /**< in uA */
#define IQ_DAC_32_63 783 /**< in uA */
#define IQ_DAC_64_255 836 /**< in uA */

/**
 * An array of bitpatterns. Each byte is to be split in 4; each group of two bits indicates the level of a pin - the
 * MSBits the first pin, the LSBits the fourth pin - with:
 * - @c 0 to indicate no quiescent current is to be setup,
 * - @c 1 using the current drive mode to invoke ~#IQ_DAC_0_31
 * - @c 2 using the current drive mode to invoke ~#IQ_DAC_32_63
 * - @c 3 using the current drive mode to invoke ~#IQ_DAC_64_255
 */
static const uint8_t sLevels[NUM_LEVELS] = {
    0x00, 0x01, 0x02, 0x03
#if PIN_COUNT >= 2
    , 0x05, 0x06, 0x07, 0x0a, 0x0b, 0x0f
#endif
#if PIN_COUNT >= 3
    , 0x15, 0x16, 0x17, 0x1a, 0x1b, 0x2a, 0x1f, 0x2b, 0x2f, 0x3f
#endif
#if PIN_COUNT >= 4
    , 0x55, 0x56, 0x57, 0x5a, 0x5b, 0x6a, 0x5f, 0x6b, 0xaa, 0x6f, 0xab, 0x7f, 0xaf, 0xbf, 0xff
#endif
    };

#if PIN_COUNT >= 1
static void SetLevel(IOCON_PIN_T pin, int level)
{
    ASSERT(level <= 3);
    if (level > 0) {
        /* A cheap way to map levels 1, 2 and 3 to any value in the regions [0..31], [32..63], [64..255] is
         * n -> (n-1) * 32
         */
        Chip_IOCON_SetPinConfig(NSS_IOCON, pin, IOCON_CDRIVE_PROGRAMMABLECURRENT
                                | IOCON_ILO_VAL((level - 1) * 32)
                                | IOCON_IHI_VAL((level - 1) * 32)
                                | IOCON_RMODE_PULLDOWN);
        Chip_GPIO_SetPinDIROutput(NSS_GPIO, 0, pin);
    }
    else {
        Chip_GPIO_SetPinDIRInput(NSS_GPIO, 0, pin);
    }
}
#endif

static void SetLevels(const uint8_t levels[4])
{
#if PIN_COUNT == 0
    (void)levels;
#endif
#if PIN_COUNT >= 1
    SetLevel(FIRST_PIN, levels[0]);
#endif
#if PIN_COUNT >= 2
    SetLevel(SECOND_PIN, levels[1]);
#endif
#if PIN_COUNT >= 3
    SetLevel(THIRD_PIN, levels[2]);
#endif
#if PIN_COUNT >= 4
    SetLevel(FOURTH_PIN, levels[3]);
#endif
}

int BatImp_Check(void)
{
    int maxCurrent = -1;

#if BATIMP_USE_PIO10
    int pinConfig10_save = Chip_IOCON_GetPinConfig(NSS_IOCON, IOCON_PIO0_10);
#endif
#if BATIMP_USE_PIO11
    int pinConfig11_save = Chip_IOCON_GetPinConfig(NSS_IOCON, IOCON_PIO0_11);
#endif

    /* Switch off battery comparator to avoid a possible battery disconnect when the voltage drops */
    bool autoPower_save = Chip_PMU_GetAutoPowerEnabled();
    Chip_PMU_SetAutoPowerEnabled(false);

    /* -------------------------------------------------------------------------------- */

    for (int i = 0; i < NUM_LEVELS; i++) {
        const uint8_t levels[4] = {sLevels[i] & 3, (sLevels[i] >> 2) & 3, (sLevels[i] >> 4) & 3, (sLevels[i] >> 6) & 3};
        SetLevels(levels);

        /* Check if BOD is triggered */
        Chip_PMU_SetBODEnabled(true);
        Chip_Clock_System_BusyWait_ms(10);
        bool bod = ((Chip_PMU_GetStatus() & PMU_STATUS_BROWNOUT) != 0);
        Chip_PMU_SetBODEnabled(false);

        /* Remove the extra current consumption. This also ensures the drawn current does not exceed the maximum
         * during the changing of the levels in the next iteration.
         */
        const uint8_t zeroLevels[4] = {0, 0, 0, 0};
        SetLevels(zeroLevels);

        if (bod) {
            /* Each enabled pin can have zero + 3 levels of extra current drawn when quiescent in current dac mode */
            static const int iq[4] = {0, IQ_DAC_0_31, IQ_DAC_32_63, IQ_DAC_64_255};
            /* Calculate the extra current that was drawn when the BOD was triggered. */
            maxCurrent = iq[levels[0]] + iq[levels[1]] + iq[levels[2]] + iq[levels[3]];
            break;
        }
    }

    /* -------------------------------------------------------------------------------- */

    /* Restore the configuration as it was at function entry. */
#if BATIMP_USE_PIO10
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_10, pinConfig10_save);
#endif
#if BATIMP_USE_PIO11
    Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_PIO0_11, pinConfig11_save);
#endif
    Chip_PMU_SetAutoPowerEnabled(autoPower_save);

    return maxCurrent;
}
