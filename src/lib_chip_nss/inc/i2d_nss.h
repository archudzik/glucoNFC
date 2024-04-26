/*
 * Copyright 2014-2016,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __I2D_NSS_H_
#define __I2D_NSS_H_

/** @defgroup I2D_NSS i2d: Current to Digital converter driver
 * @ingroup DRV_NSS
 * The Current to Digital converter (I2D) driver provides the API to control all the functionalities
 * of the Current to Digital HW block.@n
 * The I2D block allows measuring and converting the current flowing through a given analog pin into a digital value.
 * The current sink/source is connected to the I2D converter by means of a Mux.
 * The block is composed of a current scaler with configurable input gain followed by a converter (I/F converter)
 * with configurable integration time and internal gain.
 *
 * @par Mux
 *  The I2D converter Mux allows connecting the external analog pins and the internal bus lines to the I2D converter.
 *  It is allowed to select more than one input simultaneously.
 *
 * @par Current Scaler
 *  This element will divide/multiply the current input. It is possible to select several modes for the scaler and even
 *  to bypass it, connecting the current input directly to the converter. In this last case, a bias voltage of 1.1V
 *  is seen at the input multiplexer and thus the I2D block becomes a current source. Logically, when the scaler is
 *  not bypassed, the bias voltage is 0 and the I2D behaves as a current sink. Please refer to #I2D_SCALER_GAIN_T for
 *  all the supported scaler modes.
 *
 * @par I/F Converter
 *  The actual converter, takes a certain current input and converts it into a frequency signal. This signal consists on
 *  a number of pulses with a frequency proportional to the current. These pulses are then counted during a certain
 *  integration time. The total number of pulses counted over the integration time is the actual digital value that
 *  comes out of the conversion process.
 *  The maximum current that the converter is able to handle (saturation) is determined by the maximum number of pulses
 *  that can be outputted in the frequency signal and by the converter internal gain. The maximum frequency is 1 pulse
 *  every 2us which corresponds to a maximum current of 2.5uA, with #I2D_CONVERTER_GAIN_LOW selected, or 50nA with
 *  #I2D_CONVERTER_GAIN_HIGH selected (High gain is 50x higher than low gain).
 *  The conversion integration time is also configurable and will affect the resolution of the calculated absolute
 *  current. The shorter the integration time, the less pulses are counted (for the same input current) and the less
 *  bits will the resultant digital value use. The optimal integration time must match the maximum frequency of the
 *  converted signal with the maximum bit depth of the counter (16bit). Thus, at 1 pulse every 4 us, it takes
 *  262.14 ms of integration time to reach 65535 pulses.
 *
 * Additionally, the block supports configuration of single shot or continuous mode and even interrupts when conversion
 * is completed or outside a configured window. The native output of the I2D block is a bit density and there are
 * functions to convert a native conversion result to an absolute current value in pico Ampere and back
 * (#Chip_I2D_PicoAmpereToNative and #Chip_I2D_NativeToPicoAmpere).
 * The resolution of the absolute current value is affected by the converter integration time.
 *
 * @par Converter State Description
 *  -# The I2D converter follows a simple 2 state (Idle <-> In Operation) approach.
 *      After the driver is initialized (#Chip_I2D_Init) it is ensured to be in the "Idle" state. All the converter
 *      settings shall be done in this state. The status information for the I2D converter can be read at any time
 *      with the #Chip_I2D_ReadStatus function.
 *
 *  -# The transition to the "In Operation" state is triggered with the #Chip_I2D_Start function. During this transition,
 *      the settings loaded during the "Idle" state will be used and the conversion will start in the required
 *      mode (#I2D_MODE_T). Calling the #Chip_I2D_Start function in the "In Operation" state will not affect
 *      the ongoing conversion.
 *
 *  -# Depending on the selected operating mode, the transition back to "Idle" state works differently:\n
 *      - In case #I2D_CONTINUOUS mode is selected, the converter will automatically start a new conversion at the end
 *          of each one and stay in the "In operation" state. Calling the #Chip_I2D_Stop function will trigger the
 *          converter to transition to "Idle" state once the ongoing conversion ends.
 *      - In case #I2D_SINGLE_SHOT mode is selected, the transition to "Idle" state is done automatically after
 *          the first conversion is completed.
 *      As long as the converter is in the "In Operation" state, the #I2D_STATUS_CONVERTER_IN_OPERATION status flag is set.
 *
 *  -# Regardless of the operating mode and converter state, at the end of each conversion, the
 *      #I2D_STATUS_CONVERSION_DONE status flag is set, the converted result is available for read
 *      (using #Chip_I2D_GetValue), and the remaining status flags (#I2D_STATUS_RANGE_TOO_LOW and
 *      #I2D_STATUS_RANGE_TOO_HIGH) that provide further information regarding the conversion are set when applicable.
 *      Logically, both the value and the #I2D_STATUS_RANGE_TOO_LOW / #I2D_STATUS_RANGE_TOO_HIGH information of
 *      a certain conversion are lost when the next conversion ends.
 *      Additionally, once the conversion result is read (using #Chip_I2D_GetValue), the #I2D_INT_CONVERSION_RDY
 *      interrupt flag and consequently the #I2D_STATUS_CONVERSION_DONE status flag are automatically cleared.
 *
 * @par In order to perform a conversion
 *  -# Initialize the I2D converter block with default settings with #Chip_I2D_Init.
 *  -# If required, change operating mode, scaler gain, converter gain or converter integration time with,
 *      respectively, #Chip_I2D_SetMode, #Chip_I2D_SetScalerGain, #Chip_I2D_SetConverterGain
 *      or #Chip_I2D_SetConverterIntegrationTime, or alternatively with #Chip_I2D_Setup to change all parameters at once.
 *  -# Connect the required inputs via the Mux (using #I2D_INPUT_T enum values) to the I2D converter
 *      with #Chip_I2D_SetMuxInput.
 *  -# Start a conversion with #Chip_I2D_Start.
 *  -# Wait for the conversion to be ready (assess that with #Chip_I2D_ReadStatus).
 *  -# Read the converted value with #Chip_I2D_GetValue.
 *  -# If performing a #I2D_CONTINUOUS conversion, stop it with #Chip_I2D_Stop.
 *
 * @note The status information of the I2D converter can be read with #Chip_I2D_ReadStatus.
 * @note Interrupts on threshold crossing (low and high) as well as on every conversion can be set up and handled with
 *  Chip_I2D_Int_* functions.
 * @note The interrupt service routine, when needed, must be implemented in the scope of the application code.
 * @note The native bit density value (outputted by the #Chip_I2D_GetValue function) from the I2D converter
 *  can be converted to an absolute current in pico ampere with #Chip_I2D_NativeToPicoAmpere and back with
 *  #Chip_I2D_PicoAmpereToNative. Please note that the accuracy of the absolute current value is limited. Refer to
 *  User Manual for more information on the absolute current accuracy.
 *
 * @par Example 1 - Single shot conversion with polling:
 *  - mode : single shot
 *  - scaler gain : 10:1
 *  - converter gain : High
 *  - integration time : 100ms
 *  - input : pin ANA0_4
 *  @snippet i2d_nss_example_1.c i2d_nss_example_1
 *
 * @par Example 2 - Continuous conversion with interrupt:
 *  - mode : continuous
 *  - scaler gain : 1:1
 *  - converter gain : Low
 *  - integration time : 10ms
 *  - input : pin ANA0_5
 *  - interrupt generation: when low threshold is crossed (low threshold value set for 1uA)
 *  set-up code:
 *  @snippet i2d_nss_example_2.c i2d_nss_example_2
 *  Handle interrupt and get conversion code:
 *  @snippet i2d_nss_example_2.c i2d_nss_example_2_irq
 *
 * @{
 */

/** Current to Digital converter register block structure. */
typedef struct NSS_I2D_S { /*!< I2D Structure */
    __IO uint32_t CR; /*!< Control register. The current-to-digital converter control register. */
    __I uint32_t DR; /*!< Data Register. Holds a value proportional to the converted current as determined by the
     current-to-digital converter at the given input scaling. */
    __I uint32_t SR; /*!< Status Register. Holds the status of the current-to-digital converter. */
    __IO uint32_t SP0; /*!< Setup Register 0. Controls the resolution and mode settings for the ADC of
     the current-to-digital converter */
    __IO uint32_t SP1; /*!< Setup Register 1. Contains the custom integration time. */
    __IO uint32_t SP2; /*!< Setup Register 2. Contains the calibration setting for the integration time
     (system clock to time calibration factor). */
    __IO uint32_t SP3; /*!< Setup Register 2. Contains the settings for the input current scaler.
     The dynamic range is the combination of the input gain setting and the internal ADC gain setting. */
    __IO uint32_t MUX; /*!< Input multiplexer settings. Contains the settings for the multiplexer
     of the current-to-digital converter. */
    __IO uint32_t TLO; /*!< Low value threshold register. Contains the low threshold value. */
    __IO uint32_t THI; /*!< High value threshold register. Contains the high threshold value. */
    __IO uint32_t IMSC; /*!< Interrupt Mask Set/Clear Register. Controls whether each of the three possible interrupt
     conditions in the current-to-digital converter are enabled. */
    __I uint32_t RIS; /*!< Raw Interrupt Status Register. Contains a 1 for each interrupt condition that is asserted,
     regardless of whether or not the interrupt is enabled in the I2DIMSC register. */
    __I uint32_t MIS; /*!< Masked Interrupt Status Register. Contains a 1 for each interrupt condition that is
     asserted and enabled in the I2DIMSC registers */
    __O uint32_t ICR; /*!< Interrupt Clear Register. Software can write one or more one(s) to this write-only register,
     to clear the corresponding interrupt condition(s) in the current-to-digital converter. */
} NSS_I2D_T;

/** Possible inputs for the I2D converter */
typedef enum I2D_INPUT {
    I2D_INPUT_NONE = 0, /*!< No Input is connected */
    I2D_INPUT_ANA0_0 = (1 << 0), /*!< Input is the pin ANA0_0 */
    I2D_INPUT_ANA0_1 = (1 << 1), /*!< Input is the pin ANA0_1 */
    I2D_INPUT_ANA0_2 = (1 << 2), /*!< Input is the pin ANA0_2 */
    I2D_INPUT_ANA0_3 = (1 << 3), /*!< Input is the pin ANA0_3 */
    I2D_INPUT_ANA0_4 = (1 << 4), /*!< Input is the pin ANA0_4 */
    I2D_INPUT_ANA0_5 = (1 << 5), /*!< Input is the pin ANA0_5 */
    I2D_INPUT_ANA0_6 = (1 << 6), /*!< Input is the pin ANA0_6 */
    I2D_INPUT_ANA0_7 = (1 << 7), /*!< Input is the pin ANA0_7 */
    I2D_INPUT_ANA0_8 = (1 << 8), /*!< Input is the pin ANA0_8 */
    I2D_INPUT_ANA0_9 = (1 << 9), /*!< Input is the pin ANA0_9 */
    I2D_INPUT_ANA0_10 = (1 << 10), /*!< Input is the pin ANA0_10 */
    I2D_INPUT_ANA0_11 = (1 << 11), /*!< Input is the pin ANA0_11 */
    I2D_INPUT_INT_0 = (1 << 12), /*!< Input is the internal bus 0 */
    I2D_INPUT_INT_2 = (1 << 13), /*!< Input is the internal bus 2 */
    I2D_INPUT_INT_4 = (1 << 14), /*!< Input is the internal bus 4 */
    I2D_INPUT_INT_6 = (1 << 15), /*!< Input is the internal bus 6 */
    I2D_INPUT_INT_8 = (1 << 16), /*!< Input is the internal bus 8 */
    I2D_INPUT_INT_10 = (1 << 17), /*!< Input is the internal bus 10 */
    I2D_INPUT_INT_12 = (1 << 18), /*!< Input is the internal bus 12 */
    I2D_INPUT_INT_14 = (1 << 19) /*!< Input is the internal bus 14 */
} I2D_INPUT_T;

/** Possible operating modes for the I2D converter */
typedef enum I2D_MODE {
    I2D_SINGLE_SHOT = 0, /*!< When #Chip_I2D_Start is called a single conversion is done */
    I2D_CONTINUOUS = 1 /*!< When #Chip_I2D_Start is called, the current is continuously
                            being converted until #Chip_I2D_Stop is called */
} I2D_MODE_T;

/** Possible status bits of the I2D converter block (simultaneous bits are possible) */
typedef enum I2D_STATUS {
    I2D_STATUS_RANGE_TOO_LOW = (1 << 0), /*!< Indicates whether the converted digital value is inaccurate for being too
                                              high for the configured range (this status information is provided by HW) */
    I2D_STATUS_RANGE_TOO_HIGH = (1 << 1), /*!< Indicates whether the converted digital value is inaccurate for being too
                                               low for the configured range (this status information is provided by HW) */
    I2D_STATUS_CONVERTER_IN_OPERATION = (1 << 8), /*!< Indicates whether the converter is in operation or idle
                                                       (this status information is derived by SW) */
    I2D_STATUS_CONVERSION_DONE = (1 << 9) /*!< Indicates whether a conversion is done and a result is available
                                               for reading (this status information is derived by SW) */
} I2D_STATUS_T;

/** Possible current scaler gains */
typedef enum I2D_SCALER_GAIN {
    I2D_SCALER_GAIN_BYPASS = 0x0, /*!< Enables the current scaler bypass (no gain and input is biased by 1.1V) */
    I2D_SCALER_GAIN_1_1 = 0x1, /*!< Selects a current scaler gain of 1:1 (no gain) */
    I2D_SCALER_GAIN_1_2 = 0x3, /*!< Selects a current scaler gain of 1:2 (input multiplied by 2) */
    I2D_SCALER_GAIN_1_10 = 0x5, /*!< Selects a current scaler gain of 1:10 (input multiplied by 10) */
    I2D_SCALER_GAIN_2_1 = 0x7, /*!< Selects a current scaler gain of 2:1 (input divided by 2) */
    I2D_SCALER_GAIN_10_1 = 0x9, /*!< Selects a current scaler gain of 10:1 (input divided by 10) */
    I2D_SCALER_GAIN_100_1 = 0xB, /*!< Selects a current scaler gain of 100:1 (input divided by 100) */
} I2D_SCALER_GAIN_T;

/** Possible converter gains */
typedef enum I2D_CONVERTER_GAIN {
    I2D_CONVERTER_GAIN_HIGH = 0, /*!< Selects a high internal converter gain */
    I2D_CONVERTER_GAIN_LOW = 1 /*!< Selects a low internal converter gain */
} I2D_CONVERTER_GAIN_T;

/** Possible I2D converter interrupt flags */
typedef enum I2D_INT {
    I2D_INT_CONVERSION_RDY = (1 << 0), /*!< Selects the interrupt that is triggered when a conversion is ready. */
    I2D_INT_THRESHOLD_LOW = (1 << 1), /*!< Selects the interrupt that is triggered when the low threshold is reached. */
    I2D_INT_THRESHOLD_HIGH = (1 << 2), /*!< Selects the interrupt that is triggered when the high threshold is reached. */
    I2D_INT_NONE = 0, /*!< Disable all Interrupts */
    I2D_INT_ALL = 0x0F /*!< Enable all Interrupts */
} I2D_INT_T;

/**
 * Initializes the I2D peripheral with default settings.
 * This function also enables the I2D peripheral power and clock
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @note Default settings for I2D are:
 *  - mode : single-shot
 *  - scaler gain : 1:1
 *  - converter gain : Low gain
 *  - integration time : 100ms
 *  - Mux : No input connected
 *  - Threshold Low : 0
 *  - Threshold High : 0xFFFF
 */
void Chip_I2D_Init(NSS_I2D_T *pI2D);

/**
 * Disables the Current to Digital converter.
 * This function disables the I2D peripheral power, clock and interrupts (if enabled), which means that the
 * result of any ongoing conversion will be lost.
 * @param pI2D : The base address of the I2D peripheral on the chip
 */
void Chip_I2D_DeInit(NSS_I2D_T *pI2D);

/**
 * Configures the I2D peripheral with the required settings.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param mode : I2D converter operating mode to set
 * @param scalerGain : Scaler gain to set
 * @param converterGain : Converter gain to set
 * @param converterTimeMs : Converter integration time to set in milliseconds. The maximum value is 65535 ms (the
 *  argument value is clipped with a 16bit mask).
 * @note Not all combinations of #I2D_SCALER_GAIN_T and #I2D_CONVERTER_GAIN_T are allowed by the I2D converter.
 *  Please refer the the User Manual for more information on all the allowed combinations
 */
void Chip_I2D_Setup(NSS_I2D_T *pI2D, I2D_MODE_T mode, I2D_SCALER_GAIN_T scalerGain, I2D_CONVERTER_GAIN_T converterGain,
                    int converterTimeMs);

/**
 * Sets the I2D operating mode.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param mode : I2D operating mode to set
 */
void Chip_I2D_SetMode(NSS_I2D_T *pI2D, I2D_MODE_T mode);

/**
 * Gets the I2D configured operating mode.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Configured I2D operating mode
 */
I2D_MODE_T Chip_I2D_GetMode(NSS_I2D_T *pI2D);

/**
 * Sets the I2D scaler gain. The gain affects the current scaler, not the converter.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param scalerGain : I2D scaler gain to set
 * @note Not all combination of #I2D_SCALER_GAIN_T and #I2D_CONVERTER_GAIN_T are allowed by the I2D converter.
 * Please refer the the User Manual for more information on all the allowed combinations
 */
void Chip_I2D_SetScalerGain(NSS_I2D_T *pI2D, I2D_SCALER_GAIN_T scalerGain);

/**
 * Gets the configured scaler gain.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return configured scaler gain
 */
I2D_SCALER_GAIN_T Chip_I2D_GetScalerGain(NSS_I2D_T *pI2D);

/**
 * Sets the converter gain.
 * The gain affects the internal I/F converter, not the scaler.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param converterGain : Converter gain to set
 * @note The configured converter gain will define the maximum current at the input of the I/F converter that can be
 *  handled by it. Note that the current scaler precedes the I/F converter. If #I2D_CONVERTER_GAIN_HIGH is selected,
 *  the maximum current is 50nA and if #I2D_CONVERTER_GAIN_LOW is selected, the maximum current is 2.5 μA
 *  Not all combinations of #I2D_SCALER_GAIN_T and #I2D_CONVERTER_GAIN_T are allowed by the I2D converter.
 *  Please refer the the User Manual for more information on all the allowed combinations
 */
void Chip_I2D_SetConverterGain(NSS_I2D_T *pI2D, I2D_CONVERTER_GAIN_T converterGain);

/**
 * Gets the configured converter gain.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Configured converter gain
 */
I2D_CONVERTER_GAIN_T Chip_I2D_GetConverterGain(NSS_I2D_T *pI2D);

/**
 * Sets the converter integration time. The integration time affects the resolution of the absolute current,
 * after being converted by #Chip_I2D_NativeToPicoAmpere.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param converterTimeMs : Converter integration time to set in milliseconds. The maximum value is 65535 ms.
 * @note The I/F converter can output, at most, 1 pulse every 2us, which corresponds to the maximum current allowed at
 *  its input. These pulses are then counted into a 16 bit counter, over a certain integration time configured
 *  by this function. This integration time will define the maximum number of pulses that can be counted during the
 *  whole conversion process. As the counter is 16bit wide, the optimum integration time (to maximize resolution) is
 *  such that for the maximum frequency (1 pulse every 2us), 65535 counts are detected. This is 131.07ms.
 */
void Chip_I2D_SetConverterIntegrationTime(NSS_I2D_T *pI2D, int converterTimeMs);

/**
 * Gets the configured converter integration time.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Configured converter integration time in milliseconds
 */
int Chip_I2D_GetConverterIntegrationTime(NSS_I2D_T *pI2D);

/**
 * Connects the required input(s) to the I2D.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param input : Bitvector with the input(s) to be connected to the I2D converter.
 *  Use an OR’ed value of elements of #I2D_INPUT_T
 * @note This function ungrounds the respective analog bus(ses). Refer to @ref anaBusGndDesc_anchor "Analog Bus Grounding".
 */
void Chip_I2D_SetMuxInput(NSS_I2D_T *pI2D, I2D_INPUT_T input);

/**
 * Gets the input(s) connected to the I2D converter.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Bitvector with the input(s) connected to the I2D converter.
 *  An Or'd value of elements of #I2D_INPUT_T
 */
I2D_INPUT_T Chip_I2D_GetMuxInput(NSS_I2D_T *pI2D);

/**
 * Starts a current to digital conversion.
 * After this function is called, the conversion will start.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @note Use the #Chip_I2D_ReadStatus function to find out if the conversion result is ready to be read,
 *  regardless of the #I2D_MODE_T.
 */
void Chip_I2D_Start(NSS_I2D_T *pI2D);

/**
 * Stops the I2D, when in #I2D_CONTINUOUS mode, No effect in #I2D_SINGLE_SHOT mode.
 * After this function is called, the I2D converter will only be stopped once the ongoing conversion ends.
 * However this function immediately returns, not waiting for the end of the ongoing conversion.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @note This function is only applicable when the I2D converter operating mode is set to #I2D_CONTINUOUS.
 *  It is not possible to immediately stop an ongoing conversion without powering off the I2D peripheral.
 */
void Chip_I2D_Stop(NSS_I2D_T *pI2D);

/**
 * Returns the status information from the I2D converter.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Status of the I2D converter. Refer to "Converter State Description" in the "Detailed Description" section
 *  for more details on all possible I2D converter status flags.
 * @note The status information always refers to the previous measurement. This is valid even after a DeInit/Init
 *  cycle, except for the #I2D_STATUS_CONVERTER_IN_OPERATION and #I2D_STATUS_CONVERSION_DONE flags, which are cleared.
 */
I2D_STATUS_T Chip_I2D_ReadStatus(NSS_I2D_T *pI2D);

/**
 * Gets the result of the last completed conversion in native format (bit density).
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Result of the last completed conversion in the native format.
 * @note The native conversion result returned is a bit density that consists on the number of pulses outputted
 *  by the I/F converter over a certain integration time.
 *  The reading of the conversion result automatically clears the #I2D_INT_CONVERSION_RDY interrupt flag and
 *  consequently the #I2D_STATUS_CONVERSION_DONE status bit flag.
 */
int Chip_I2D_GetValue(NSS_I2D_T *pI2D);

/**
 * Convert a given native conversion result (bit density) value to an absolute current value in pico Ampere.
 * @param native : native conversion result (bit density) retrieved with #Chip_I2D_GetValue
 * @param scalerGain : Scaler gain used in the HW conversion
 * @param converterGain : Converter gain used in the HW conversion
 * @param converterTimeMs : Converter integration time in milliseconds used in the HW conversion
 * @return The correspondent absolute current value in pico Ampere.
 * @note Integration times < 1 ms are not allowed.
 */
int Chip_I2D_NativeToPicoAmpere(int native, I2D_SCALER_GAIN_T scalerGain, I2D_CONVERTER_GAIN_T converterGain,
                                int converterTimeMs);

/**
 * Convert a given absolute current in pico Ampere to a native bit density value.
 * @param picoAmpere : absolute current value in pico Ampere
 * @param scalerGain : Scaler gain used in the HW conversion
 * @param converterGain : Converter gain used in the HW conversion
 * @param converterTimeMs : Converter integration time in milliseconds used in the HW conversion
 * @return The native bit density value relative to a scaler gain, converter gain and integration time.
 * @note In order to set threshold limits in the Chip_I2D_Int_SetThreshold* functions, a native conversion value
 *  (bit density) must be used. Integration times < 1 ms are not allowed.
 *  The calculation result is capped to 0xFFFF, as the native converted value is also 16bit wide.
 */
int Chip_I2D_PicoAmpereToNative(int picoAmpere, I2D_SCALER_GAIN_T scalerGain, I2D_CONVERTER_GAIN_T converterGain,
                                int converterTimeMs);

/**
 * Sets the low conversion threshold value.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param native : The value to set for the low-threshold
 * @note The converted bit density value is compared to the threshold and an interrupt is generated if the converted
 *  value is lower, assuming the respective interrupt enable flag is set
 * @see I2D_INT_THRESHOLD_LOW
 */
void Chip_I2D_Int_SetThresholdLow(NSS_I2D_T *pI2D, int native);

/**
 * Retrieves the low conversion threshold value.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return The value for the low-threshold
 * @note The value is a bit density that represents a current value
 */
int Chip_I2D_Int_GetThresholdLow(NSS_I2D_T *pI2D);

/**
 * Sets the high conversion threshold value.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param native : The value to set for the high-threshold
 * @note The converted bit density value is compared to the threshold and an interrupt is generated if the converted
 *  value is higher, assuming the respective interrupt enable flag is set
 * @see I2D_INT_THRESHOLD_HIGH
 */
void Chip_I2D_Int_SetThresholdHigh(NSS_I2D_T *pI2D, int native);

/**
 * Retrieves the high conversion threshold value.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return The value for the high-threshold
 * @note The value is a bit density that represents a current value
 */
int Chip_I2D_Int_GetThresholdHigh(NSS_I2D_T *pI2D);

/**
 * Enables/Disables the I2D converter interrupts.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param mask : interrupt enabled mask to set
 */
void Chip_I2D_Int_SetEnabledMask(NSS_I2D_T *pI2D, I2D_INT_T mask);

/**
 * Retrieves the I2D converter interrupt enabled mask.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return Interrupt enabled mask
 */
I2D_INT_T Chip_I2D_Int_GetEnabledMask(NSS_I2D_T *pI2D);

/**
 * Retrieves a bitVector with the RAW I2D converter interrupt flags.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @return BitVector with the I2D converter RAW interrupt flags
 * @note A bit set to 1 means that the correspondent interrupt flag is set.
 */
I2D_INT_T Chip_I2D_Int_GetRawStatus(NSS_I2D_T *pI2D);

/**
 * Clears the required I2D converter interrupt flags.
 * @param pI2D : The base address of the I2D peripheral on the chip
 * @param flags : Bitvector indicating which interrupt flags to clear
 */
void Chip_I2D_Int_ClearRawStatus(NSS_I2D_T *pI2D, I2D_INT_T flags);

/**
 * @}
 */

#endif /* __I2D_NSS_H_ */
