/*
 * Copyright 2015-2017,2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include <string.h>
#include "tmeas.h"

/* -------------------------------------------------------------------------
 * Private function prototypes
 * ------------------------------------------------------------------------- */

static int Convert(TMEAS_FORMAT_T format, int input);

/* -------------------------------------------------------------------------
 * Private variables
 * ------------------------------------------------------------------------- */

static volatile bool sMeasurementInProgress = false;
#if defined(TMEAS_CB)
static volatile TMEAS_FORMAT_T sFormat;
static volatile uint32_t sContext;
#endif

/* -------------------------------------------------------------------------
 * Private functions
 * ------------------------------------------------------------------------- */

#if defined(TMEAS_CB)
void TSEN_IRQHandler(void)
{
    /* If interrupt is reached, we can safely deduct that the RDY bit was set and therefore the
     * TSEN_STATUS_MEASUREMENT_SUCCESS status bit is set. The remaining (RANGE) status bits, even when set, should not
     * invalidate the temperature measurement,
     * hence we can always assume that, at this moment, the value present in the TSEN Value register is always valid.
     */
    /* Measurement ready. Read the data (thereby also clearing the interrupt). */
    int value = Chip_TSen_GetValue(NSS_TSEN);
    int output = Convert(sFormat, value);
    NVIC_DisableIRQ(TSEN_IRQn);
    Chip_TSen_DeInit(NSS_TSEN);
    {
        extern void TMEAS_CB(TSEN_RESOLUTION_T resolution, TMEAS_FORMAT_T format, int value, uint32_t context);
        TMEAS_CB(Chip_TSen_GetResolution(NSS_TSEN), sFormat, output, sContext);
    }
    sMeasurementInProgress = false;
}
#endif

/* ------------------------------------------------------------------------- */

static int Convert(TMEAS_FORMAT_T format, int input)
{
    int output;

    /* Temperature sensor correction is applied in the Native value here.
     * Regardless of the sample, a correction needs to be applied to fix a deviation with the sensor.
     * For a value C in degrees Celsius:
     *  f: C -> C * (1 - 0.6/85)
     * The variable 'input' is in native format. Using the conversion formula
     *  g: C = N/64 - 273.15
     *  g^-1: N = 64 * (C + 273.15)
     * Translating that correction to a correction for a value N in native value (as returned by the TSEN HW block):
     *  N -> g^-1(f(g(N))) = 64 * ((N/64 - 273.15) * (1 - 0.6/85) + 273.15)
     *                     = (2110 * N + 262224) / 2125
     * The nominator overflows for values of N higher than 2035405, which corresponds to values of C in degrees Celsius
     * higher than 31530 - so no worries here.
     */
#if TMEAS_SENSOR_CORRECTION
    input = (2110 * input + 262224) / 2125;
#endif

    switch (format) {
#if TMEAS_KELVIN
        case TMEAS_FORMAT_KELVIN:
        output = Chip_TSen_NativeToKelvin(input, 10);
        break;
#endif
#if TMEAS_CELSIUS
        case TMEAS_FORMAT_CELSIUS:
            output = Chip_TSen_NativeToCelsius(input, 10);
            break;
#endif
#if TMEAS_FAHRENHEIT
            case TMEAS_FORMAT_FAHRENHEIT:
            output = Chip_TSen_NativeToFahrenheit(input, 10);
            break;
#endif
        default:
        case TMEAS_FORMAT_NATIVE:
            output = input; /* Measurement was taken in calibrated mode with the MSBit 0. */
            break;
    }
    return output;
}

/* -------------------------------------------------------------------------
 * Exported functions
 * ------------------------------------------------------------------------- */

int TMeas_Measure(TSEN_RESOLUTION_T resolution, TMEAS_FORMAT_T format, bool synchronous, uint32_t context)
{
#if !defined(TMEAS_CB)
    /* gracefully do nothing and avoid compiler warnings */
    (void)synchronous;
    (void)context;
#endif
    int output = TMEAS_ERROR;
    if (!sMeasurementInProgress) {
        sMeasurementInProgress = true;

        Chip_TSen_Init(NSS_TSEN);
        Chip_TSen_SetResolution(NSS_TSEN, resolution);
#if defined(TMEAS_CB)
        if (!synchronous) {
            sFormat = format;
            sContext = context;
            Chip_TSen_Int_SetEnabledMask(NSS_TSEN, TSEN_INT_MEASUREMENT_RDY);
            NVIC_EnableIRQ(TSEN_IRQn);
        }
#endif
        output = TMEAS_ERROR;

        Chip_TSen_Start(NSS_TSEN);
#if defined(TMEAS_CB)
        if (synchronous)
#endif
        {
            while (!(Chip_TSen_ReadStatus(NSS_TSEN, NULL) & TSEN_STATUS_MEASUREMENT_DONE)) {
                ; /* wait */
            }
            /* The remaining (RANGE) status bits, even when set, should not invalidate the temperature measurement,
             * hence we can always assume that, at this moment, the value present in the TSEN Value register is always valid. */
            /* Measurement ready. Read the data (thereby also clearing the DONE status bit). */
            output = Convert(format, Chip_TSen_GetValue(NSS_TSEN));
            NVIC_DisableIRQ(TSEN_IRQn);
            Chip_TSen_DeInit(NSS_TSEN);
            sMeasurementInProgress = false;
        }
#if defined(TMEAS_CB)
        else {
            output = 0;
            /* sMeasurementInProgress is set to false in TSEN_IRQHandler */
        }
#endif
    }

    return output;
}
