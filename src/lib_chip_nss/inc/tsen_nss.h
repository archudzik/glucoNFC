/*
 * Copyright 2014-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __TSEN_NSS_H_
#define __TSEN_NSS_H_

/**
 * @defgroup TSEN_NSS tsen: Temperature Sensor driver
 * @ingroup DRV_NSS
 * The Temperature Sensor driver (TSen) provides the API to control all the functionalities of the Temperature Sensor HW
 * block. This block reads the temperature, in Kelvin, using a configured resolution. The block does feature interrupt
 * when measurement complete, but it does not feature continuous mode.
 *
 * @par Sensor State Description:
 *  -# The Temperature Sensor follows a simple 2 state (Idle <-> In Operation) approach.
 *      After the driver is initialized (#Chip_TSen_Init) it is ensured to be in the "Idle" state. The
 *      shall be set in this state (#Chip_TSen_SetResolution). The status information for the Temperature Sensor can be
 *      required resolution read at any time with the #Chip_TSen_ReadStatus function.
 *  -# The transition to the "In Operation" state is triggered with the #Chip_TSen_Start function. During
 *      transition, the settings loaded during the "Idle" state will be used and the measurement will start.
 *      Calling the #Chip_TSen_Start function in the "In Operation" state will not affect the ongoing measurement.
 *  -# The transition to "Idle" state is done automatically after the measurement is completed.
 *      As long as the sensor is in the "In Operation" state, the #TSEN_STATUS_SENSOR_IN_OPERATION status flag is set.
 *  -# At the end of each measurement, the #TSEN_STATUS_MEASUREMENT_DONE status flag is set indicating that the measured
 *      temperature is available for read (using #Chip_TSen_GetValue).
 *      Logically, both the status information provided by HW and the result of a certain measurement is lost when a new
 *      measurement ends.
 *      Additionally, once the measurement result is read (using #Chip_TSen_GetValue), the #TSEN_INT_MEASUREMENT_RDY
 *      interrupt flag and consequently the #TSEN_STATUS_MEASUREMENT_DONE status flag are automatically cleared.
 *
 * @par In order to perform a measurement:
 *  -# Initialize the Temperature Sensor block with default settings with #Chip_TSen_Init.
 *  -# If required, change resolution with #Chip_TSen_SetResolution.
 *  -# Start a measurement with #Chip_TSen_Start.
 *  -# Wait for the measurement to be ready (assess that with #Chip_TSen_ReadStatus).
 *  -# Read the measurement value with #Chip_TSen_GetValue.
 *
 * @par Notes:
 *  - The native format of the temperature sensor uses a signed 10.6 2-complement fixed point representation
 *    and holds a non-human readable temperature in Kelvin. This is outputted by the #Chip_TSen_GetValue and
 *    Chip_TSen_Int_GetThreshold* functions and accepted by the Chip_TSen_NativeTo* and the Chip_TSen_Int_SetThreshold*
 *    functions.
 *  - The status information of the temperature sensor regarding a measurement can be read with #Chip_TSen_ReadStatus.
 *  - Interrupt generation when threshold is crossed (low and high) as well as when a measurement is ready can be set up
 *    and handled with Chip_TSen_Int_* functions.
 *  - The interrupt service routine, when needed, must be implemented in the scope of the application code.
 *  - Use the #Chip_TSen_NativeToCelsius, #Chip_TSen_NativeToFahrenheit and #Chip_TSen_NativeToKelvin functions to
 *    convert from a temperature in the native format into a temperature in, respectively, Celsius, Fahrenheit or Kelvin
 *    in a human readable fixed point decimal representation. In the inverse direction, use the
 *    #Chip_TSen_CelsiusToNative, #Chip_TSen_FahrenheitToNative and #Chip_TSen_KelvinToNative functions to convert from
 *    a temperature in, respectively Celsius, Fahrenheit or Kelvin in a readable fixed point decimal representation into
 *    a temperature in the native format.
 *
 * @par Example 1 - Simple polled measurement.
 *  - resolution: 10 bits
 *  @snippet tsen_nss_example_1.c tsen_nss_example_1
 *
 * @par Example 2 - Measurement with interrupt and thresholds.
 *  - resolution: 12 bits
 *  - interrupt generation: when a conversion is ready and when low or high thresholds are crossed.
 *
 *  @snippet tsen_nss_example_2.c tsen_nss_example_2
 *  #TSEN_IRQHandler:
 *  @snippet tsen_nss_example_2.c tsen_nss_example_2_irq
 *
 * @{
 */

/** Temperature Sensor register block structure */
typedef struct NSS_TSEN_S {
    __IO uint32_t CR; /**< Control register. Controls the start of a Temperature Measurement. */
    __I  uint32_t DR; /**< Data Register. Holds the current temperature data in either raw or calibrated format. */
    __I  uint32_t SR; /**< Status Register. Holds the status of the temperature sensor. */
    __IO uint32_t SP0; /**< Setup Register 0. Controls the resolution and mode settings for the zoom ADC of the
                            temperature sensor */
    __IO uint32_t SP1; /**< Setup Register 1. Contains the calibration parameter "A". */
    __IO uint32_t SP2; /**< Setup Register 2. Contains the calibration parameter "B". */
    __IO uint32_t SP3; /**< Setup Register 3. Contains the calibration parameter "ALPHA". */
    __IO uint32_t TLO; /**< Low temperature threshold register. Contains the low-temperature threshold value. */
    __IO uint32_t THI; /**< High temperature threshold register. Contains the high-temperature threshold value. */
    __IO uint32_t IMSC; /**< Interrupt Mask Set/Clear Register. Controls whether each of the three possible interrupt
                             conditions in the temperature sensor are enabled. */
    __I  uint32_t RIS; /**< Raw Interrupt Status Register. Contains a 1 for each interrupt condition that is asserted,
                            regardless of whether or not the interrupt is enabled in the TSENIMSC register. */
    __I  uint32_t MIS; /**< Masked Interrupt Status Register. Contains a 1 for each interrupt condition that is asserted
                            and enabled in the TSENIMSC registers */
    __O  uint32_t ICR; /**< Interrupt Clear Register. Software can write one or more one(s) to this write-only register
                            to clear the corresponding interrupt condition(s) in the temperature sensor controller. */
} NSS_TSEN_T;

/** Possible resolutions for the Temperature Sensor */
typedef enum TSEN_RESOLUTION {
    TSEN_7BITS = 2, /**< @c 0x02 @n 7 bits resolution. Conversion time: 4 ms. */
    TSEN_8BITS = 3, /**< @c 0x03 @n 8 bits resolution. Conversion time: 7 ms. */
    TSEN_9BITS = 4, /**< @c 0x04 @n 9 bits resolution. Conversion time: 14 ms. */
    TSEN_10BITS = 5, /**< @c 0x05 @n 10 bits resolution. Conversion time: 26 ms. */
    TSEN_11BITS = 6, /**< @c 0x06 @n 11 bits resolution. Conversion time: 50 ms. */
    TSEN_12BITS = 7 /**< @c 0x07 @n 12 bits resolution. Conversion time: 100 ms. */
} TSEN_RESOLUTION_T;

/** Possible status bits within the Temperature Sensor block */
typedef enum TSEN_STATUS {
    TSEN_STATUS_COARSE_RANGE_LOW = (1 << 0), /**< The temperature is too low for the sensor (provided by HW) */
    TSEN_STATUS_COARSE_RANGE_HIGH = (1 << 1), /**< The temperature is too high for the sensor (provided by HW) */
    TSEN_STATUS_FINE_RANGE_LOW = (1 << 2), /**< The temperature decreased too fast during a measurement (provided by HW) */
    TSEN_STATUS_FINE_RANGE_HIGH = (1 << 3), /**< The temperature increased too fast during a measurement (provided by HW) */
    TSEN_STATUS_MEASUREMENT_SUCCESS = (1 << 4), /**< The temperature measurement was successful (provided by HW) */
    TSEN_STATUS_SENSOR_IN_OPERATION = (1 << 8), /**< The sensor is in operation (derived by SW) */
    TSEN_STATUS_MEASUREMENT_DONE = (1 << 9) /**< A measurement is done and a result is available (derived by SW) */
} TSEN_STATUS_T;

/** Possible Temperature Sensor interrupt flags */
typedef enum TSEN_INT {
    TSEN_INT_MEASUREMENT_RDY = (1 << 0), /**< Selects the interrupt that is triggered when a measurement is ready. */
    TSEN_INT_THRESHOLD_LOW = (1 << 1), /**< Selects the interrupt that is triggered when the low threshold is reached. */
    TSEN_INT_THRESHOLD_HIGH = (1 << 2), /**< Selects the interrupt that is triggered when the high threshold is reached. */
    TSEN_INT_NONE = 0, /**< Disable all Interrupts */
    TSEN_INT_ALL = 0x0F /**< Enable all Interrupts */
} TSEN_INT_T;

/**
 * Initializes the TSEN peripheral with default settings.
 * This function also enables the TSEN peripheral power and clock.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @note Default settings for the Temperature Sensor are:
 *  - resolution:     12 bit
 *  - Threshold Low:  0x8000
 *  - Threshold High: 0x7FFF
 */
void Chip_TSen_Init(NSS_TSEN_T *pTSen);

/**
 * Disables the temperature sensor.
 * This function disables the TSEN peripheral power, clock and interrupts (if enabled), which means that the result of
 * any ongoing measurement will be lost.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 */
void Chip_TSen_DeInit(NSS_TSEN_T *pTSen);

/**
 * Sets the resolution for the Temperature Sensor.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @param resolution : Sensor resolution
 */
void Chip_TSen_SetResolution(NSS_TSEN_T *pTSen, TSEN_RESOLUTION_T resolution);

/**
 * Gets the Temperature Sensor configured resolution.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @return Temperature Sensor configured resolution
 */
TSEN_RESOLUTION_T Chip_TSen_GetResolution(NSS_TSEN_T *pTSen);

/**
 * Starts a temperature measurement.
 * After this function is called, the measurement will start.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @note Use the #Chip_TSen_ReadStatus function to find out if the measurement result is ready to be read.
 */
void Chip_TSen_Start(NSS_TSEN_T *pTSen);

/**
 * Returns the status information from the Temperature Sensor
 * In case no previous measurement exists, the output parameter value is undefined.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @param pResolution : pointer to write the sensor resolution used in the last completed measurement
 * @return Status of the Temperature Sensor. Refer to "Sensor State Description" in the "Detailed Description" section
 *  for more details on all possible Temperature Sensor status flags.
 * @note A NULL pointer can be passed to pResolution parameter in case this information is not required.
 * @note The status information (including resolution) always refers to the previous measurement. This is valid even
 *  after a DeInit/Init cycle, except for the #TSEN_STATUS_SENSOR_IN_OPERATION and #TSEN_STATUS_MEASUREMENT_DONE flags,
 *  which are cleared.
 */
TSEN_STATUS_T Chip_TSen_ReadStatus(NSS_TSEN_T *pTSen, TSEN_RESOLUTION_T *pResolution);

/**
 * Gets the measured temperature value (bit density) present in the data register
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @return Temperature value in the native format.
 * @note The native measured temperature value outputted is in a signed 10.6 2-complement fixed point representation
 *  and holds a non-human readable temperature in Kelvin.
 * @note The reading of the temperature value automatically clears the #TSEN_INT_MEASUREMENT_RDY interrupt flag and
 *  consequently the #TSEN_STATUS_MEASUREMENT_DONE status bit flag.
 */
int Chip_TSen_GetValue(NSS_TSEN_T *pTSen);

/**
 * Converts a temperature value in the native format into a human readable format in Kelvin.
 * The output value is a human readable signed temperature value in Kelvin multiplied by parameter "multiplier".
 * @param native : The temperature value in the native format
 * @param multiplier : The multiplication factor of the converted temperature
 * @return The converted temperature value in Kelvin human readable format
 * @note The output value is a signed 32 bit value. A multiplication factor between 1 and 1024 (inclusive) must be
 *  chosen so that there is no overflow.
 */
int Chip_TSen_NativeToKelvin(int native, int multiplier);

/**
 * Converts a temperature value in the human readable format in Kelvin into the native format.
 * The input value is a human readable signed temperature value in Kelvin multiplied by parameter "multiplier".
 * @param kelvin : The input temperature value in the human readable format
 * @param multiplier : The multiplication factor of the input human readable format temperature
 * @return The converted temperature value in the native format
 * @note The output value is a signed 32 bit value. A multiplication factor between 1 and 1024 (inclusive) must be
 *  chosen so that there is no overflow.
 */
int Chip_TSen_KelvinToNative(int kelvin, int multiplier);

/**
 * Converts a temperature value in the native format into a human readable format in Celsius.
 * The output value is a human readable signed temperature value in Celsius multiplied by parameter "multiplier".
 * @param native : The temperature value in the native format
 * @param multiplier : The multiplication factor for the converted temperature
 * @return The correspondent temperature value in Celsius human readable format
 * @note The output value is a signed 32 bit value. A multiplication factor between 1 and 1024 (inclusive) must be
 *  chosen so that there is no overflow.
 */
int Chip_TSen_NativeToCelsius(int native, int multiplier);

/**
 * Converts a temperature value in the human readable format in Celsius into the native format.
 * The input value is a human readable signed temperature value in Celsius multiplied by parameter "multiplier".
 * @param celsius : The input temperature value in the human readable format
 * @param multiplier : The multiplication factor of the input human readable format temperature
 * @return The converted temperature value in the native format
 * @note The output value is a signed 32 bit value. A multiplication factor between 1 and 1024 (inclusive) must be
 *  chosen so that there is no overflow.
 */
int Chip_TSen_CelsiusToNative(int celsius, int multiplier);

/**
 * Converts a temperature value in the native format to a human readable format in Fahrenheit
 * The output value is a human readable signed temperature value in Fahrenheit multiplied by parameter "multiplier".
 * @param native : The temperature value in the native format
 * @param multiplier : The multiplication factor for the converted temperature
 * @return The correspondent temperature value in Fahrenheit human readable format
 * @note The output value is a signed 32 bit value. A multiplication factor between 1 and 1024 (inclusive) must be
 *  chosen so that there is no overflow.
 */
int Chip_TSen_NativeToFahrenheit(int native, int multiplier);

/**
 * Converts a temperature value in the human readable format in Fanhrenheit into the native format.
 * The input value is a human readable signed temperature value in Fanhrenheit multiplied by parameter "multiplier".
 * @param fahrenheit : The input temperature value in the human readable format
 * @param multiplier : The multiplication factor of the input human readable format temperature
 * @return The converted temperature value in the native format
 * @note The output value is a signed 32 bit value. A multiplication factor between 1 and 1024 (inclusive) must be
 *  chosen so that there is no overflow.
 */
int Chip_TSen_FahrenheitToNative(int fahrenheit, int multiplier);

/**
 * Sets the low-temperature threshold value.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @param native : The value to set for the temperature low-threshold
 * @note The temperature threshold value is in the native format.
 * @note Threshold values are compared to measured temperature values.
 */
void Chip_TSen_Int_SetThresholdLow(NSS_TSEN_T *pTSen, int native);

/**
 * Retrieves the low-temperature threshold value.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @return The value for the temperature low-threshold
 * @note The temperature threshold value is in the native format.
 * @note Threshold values are compared to measured temperature values.
 */
int Chip_TSen_Int_GetThresholdLow(NSS_TSEN_T *pTSen);

/**
 * Sets the high-temperature threshold value.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @param native : The value to set for the high-threshold
 * @note The temperature threshold value is in the native format.
 * @note Threshold values are compared to measured temperature values.
 */
void Chip_TSen_Int_SetThresholdHigh(NSS_TSEN_T *pTSen, int native);

/**
 * Retrieves the high-temperature threshold value.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @return The value to for the high-threshold
 * @note The temperature threshold value is in the native format.
 * @note Threshold values are compared to measured temperature values.
 */
int Chip_TSen_Int_GetThresholdHigh(NSS_TSEN_T *pTSen);

/**
 * Enables/Disables Temperature Sensor interrupts.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @param mask : interrupt enabled mask to set
 */
void Chip_TSen_Int_SetEnabledMask(NSS_TSEN_T *pTSen, TSEN_INT_T mask);

/**
 * Retrieves the Temperature Sensor interrupt enabled mask.
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @return Interrupt enabled mask
 */
TSEN_INT_T Chip_TSen_Int_GetEnabledMask(NSS_TSEN_T *pTSen);

/**
 * Retrieves a bitVector with the RAW Temperature Sensor interrupt flags
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @return BitVector with the Temperature Sensor RAW interrupt flags
 * @note A bit set to 1 means that the correspondent interrupt flag is set.
 */
TSEN_INT_T Chip_TSen_Int_GetRawStatus(NSS_TSEN_T *pTSen);

/**
 * Clears the required Temperature Sensor interrupt flags
 * @param pTSen : The base address of the TSEN peripheral on the chip
 * @param flags : BitVector indicating which interrupt flags to clear
 */
void Chip_TSen_Int_ClearRawStatus(NSS_TSEN_T *pTSen, TSEN_INT_T flags);

#endif /** @} */
