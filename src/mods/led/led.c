/*
 * Copyright 2014-2017,2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include "led/led.h"

#if LED_COUNT
static const LED_PROPERTIES_T sLeds[LED_COUNT] = LED_PROPERTIES;
#endif

/* -------------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------------- */

void LED_Init(void)
{
#if LED_COUNT
    for (int n = 0; n < LED_COUNT; n++) {
        Chip_IOCON_SetPinConfig(NSS_IOCON, sLeds[n].pio, IOCON_FUNC_0 | IOCON_RMODE_INACT);
        Chip_GPIO_SetPinDIROutput(NSS_GPIO, sLeds[n].port, sLeds[n].pin);
    }
    LED_Off(LED_ALL);
#endif
}

void LED_SetState(int leds, int states)
{
#if LED_COUNT
    for (int n = 0; n < LED_COUNT; n++) {
        if (leds & LED_(n)) {
            Chip_GPIO_SetPinState(NSS_GPIO, sLeds[n].port, sLeds[n].pin, ((states >> n) & 1) == sLeds[n].polarity);
        }
    }
#else
    (void)leds; /* suppress [-Wunused-parameter]: there are no leds defined, so nothing must be done. */
    (void)states; /* suppress [-Wunused-parameter]: there are no leds defined, so nothing must be done. */
#endif
}

int LED_GetState(int leds)
{
#if LED_COUNT
    int result = 0;
    for (int n = 0; n < LED_COUNT; n++) {
        if (leds & LED_(n)) {
            result |= (Chip_GPIO_GetPinState(NSS_GPIO, sLeds[n].port, sLeds[n].pin) == sLeds[n].polarity) << n;
        }
    }
    return result;
#else
    (void)leds; /* suppress [-Wunused-parameter]: there are no leds defined, so nothing must be done. */
    return 0;
#endif
}

void LED_On(int leds)
{
    LED_SetState(leds, leds);
}

void LED_Off(int leds)
{
    LED_SetState(leds, 0);
}

void LED_Toggle(int leds)
{
    LED_SetState(leds, ~LED_GetState(leds));
}
