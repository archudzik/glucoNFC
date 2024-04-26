/*
 * Copyright 2014-2018,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __ADCDAC_NSS_H_
#define __ADCDAC_NSS_H_

/** @defgroup ADCDAC_NSS adcdac: Analog-to-Digital / Digital-to-Analog converter driver
 * @ingroup DRV_NSS
 * This Analog-to-Digital / Digital-to-Analog Converter (ADC/DAC) implements 12-bit successive-approximation charge-redistribution (SAR) converter.
 * Each instance of this driver provides Analog-to-digital as well as Digital-to-analog conversion.
 *
 * @anchor convStateDesc_anchor
 * @par Converter State Description
 *  -# The ADC/DAC driver follows a simple 2 states (Idle <-> In Operation) approach.
 *      After the driver is initialized (#Chip_ADCDAC_Init) it is ensured to be in the "Idle" state. All the driver
 *      settings shall be done in this state. The status information for the ADC/DAC driver can be read at any time
 *      with the #Chip_ADCDAC_ReadStatus function.
 *  -# The transition to the "In Operation" state is triggered with the #Chip_ADCDAC_StartADC or #Chip_ADCDAC_StartDAC functions.
 *      During this transition, the settings loaded during the "Idle" state will be used and the conversion will begin
 *      in the required mode (#ADCDAC_MODE_T). Calling the #Chip_ADCDAC_StartADC or #Chip_ADCDAC_StartDAC functions in
 *      the "In Operation" state will not affect the ongoing conversion. I.e. If an existing ADC conversion is in progress,
 *      calling #Chip_ADCDAC_StartADC again does not trigger another analog-to-digital conversion. Existing conversion
 *      has to complete and return to Idle state before a start operation is called again.
 *  -# Depending on the selected operating mode, the transition back to "Idle" state works differently:
 *      - In case #ADCDAC_CONTINUOUS mode is selected, the converter will automatically start a new conversion at the end of
 *          each one and stay in the "In operation" state. Calling the #Chip_ADCDAC_StopADC or #Chip_ADCDAC_StopDAC
 *          function will trigger the converter to transition to "Idle" state once the ongoing conversion ends.
 *      - In case #ADCDAC_SINGLE_SHOT mode is selected, the transition to "Idle" state is done automatically after the
 *          first conversion is completed.
 *      As long as the converter is in the "In Operation" state, the #ADCDAC_STATUS_CONVERTER_IN_OPERATION status flag is set.
 *  -# Regardless of the operating mode and driver state, at the end of each conversion, the #ADCDAC_STATUS_ADC_DONE or
 *      #ADCDAC_STATUS_DAC_DONE status flag is set. In the case of ADC, the converted result is available for read
 *      (using #Chip_ADCDAC_GetValueADC). Additionally, once the conversion result is read (using #Chip_ADCDAC_GetValueADC),
 *      the #ADCDAC_STATUS_ADC_DONE status flag is automatically cleared.
 *
 * @par In order to perform a conversion:
 *  -# Initialize the ADC/DAC driver with #Chip_ADCDAC_Init.
 *  -# Configure the ADC/DAC driver with appropriate input connection (ADC operation) with #Chip_ADCDAC_SetMuxADC or
 *      output connection (DAC operation) with #Chip_ADCDAC_SetMuxDAC.
 *  -# If required, configure ADC threshold with #Chip_ADCDAC_Int_SetThresholdLowADC and #Chip_ADCDAC_Int_SetThresholdHighADC.
 *  -# Start an ADC conversion with #Chip_ADCDAC_StartADC, or start a DAC output with #Chip_ADCDAC_StartDAC.
 *  -# Wait for the conversion (ADC or DAC) to be ready with #Chip_ADCDAC_ReadStatus.
 *  -# Read the ADC conversion value with #Chip_ADCDAC_GetValueADC.
 *
 * @note The status of the conversion can be read with #Chip_ADCDAC_ReadStatus.
 * @note Interrupt generation when threshold is crossed (low and high) as well as when a conversion is ready can be
 *  set up and handled with Chip_ADCDAC_Int_* functions.
 * @note The interrupt service routine, if required, must be implemented in the scope of the application code.
 * @note The ADCDAC HW block contains only one AD/DA Converter. Therefore it implements a round robin scheduler
 *  that will share the AD/DA Converter between ADC and DAC functionality.
 *  Hence, when using the ADCDAC block for both functions "simultaneously", the conversion timing is non-deterministic.
 *  Refer to the user manual (ADCDAC0) for detailed information about timing and scheduler.
 *
 * @par Example 1 - Continuous Digital-to-Analog conversion with polling
 *  - DAC connection: #ADCDAC_IO_ANA0_0
 *  - DAC mode: Continuous
 *  - DAC interrupt: none
 *  @snippet adcdac_nss_example_1.c adcdac_nss_example_1
 *
 * @par Example 2 - Continuous Analog-to-Digital Conversion with IRQ
 *  - ADC input voltage range: 0.0V to 1.6V (#ADCDAC_INPUTRANGE_WIDE)
 *  - ADC connection: #ADCDAC_IO_ANA0_5
 *  - ADC input thresholds: 1138 and 3412 (approximately at 30% and 70% of the full scale)
 *  - ADC mode: Continuous
 *  set-up code:
 *  @snippet adcdac_nss_example_2.c adcdac_nss_example_2
 *  handle interrupt and read ADC input:
 *  @snippet adcdac_nss_example_2.c adcdac_nss_example_2_irq
 *
 * @par Example 3 - Single-shot Analog-to-Digital Conversion without IRQ
 *  - ADC connection: #ADCDAC_IO_ANA0_5
 *  - ADC mode: Single-shot
 *  - ADC input voltage range: 0.0V to 1.6V (#ADCDAC_INPUTRANGE_WIDE)
 *  @snippet adcdac_nss_example_3.c adcdac_nss_example_3
 * @{
 */

/** ADC/DAC register block structure for Analog-to-digital / digital-to-analog converter */
typedef struct NSS_ADCDAC_S {
    __IO uint32_t CR; /*!< Control register. */
    __I  uint32_t ADCDR; /*!< ADC Data Register. Contains result of analog-to-digital conversion */
    __O  uint32_t DACDR; /*!< DAC Data Register. Contains value to be converted to analog voltage */
    __I  uint32_t SR; /*!< Status Register. Reserved for future use */
    __IO uint32_t SP0; /*!< ADC mode setup register */
    __IO uint32_t RESERVED1; /* next field at offset 0x018 */
    __IO uint32_t ADCMUX; /*!< ADC input multiplexer setting */
    __IO uint32_t DACMUX; /*!< DAC output switch setting */
    __IO uint32_t TLO; /*!< Low value threshold register */
    __IO uint32_t THI; /*!< High value threshold register */
    __IO uint32_t IMSC; /*!< Interrupt Mask Set and Clear Register */
    __I  uint32_t RIS; /*!< Raw Interrupt Status Register */
    __I  uint32_t MIS; /*!< Masked Interrupt Status Register */
    __O  uint32_t ICR; /*!< Interrupt Clear Register */
} NSS_ADCDAC_T;

/** Possible operating modes for the Analog-to-digital / digital-to-analog converter. */
typedef enum ADCDAC_MODE {
    ADCDAC_SINGLE_SHOT = 0, /*!< When #Chip_ADCDAC_StartADC or #Chip_ADCDAC_StartDAC is called a single conversion is done */
    ADCDAC_CONTINUOUS = 1 /*!< When #Chip_ADCDAC_StartADC or #Chip_ADCDAC_StartDAC is called, ADC/DAC mode is continuously
                               being converted until #Chip_ADCDAC_StopADC or #Chip_ADCDAC_StopDAC is called */
} ADCDAC_MODE_T;

/** Possible IO connection setting for the ADC/DAC */
typedef enum ADCDAC_IO {
    ADCDAC_IO_NONE = 0, /*!< No connection */
    ADCDAC_IO_ANA0_0    = 1 << 0, /*!< External Analog Bus pin ana0_0 */
    ADCDAC_IO_ANA0_1    = 1 << 1, /*!< External Analog Bus pin ana0_1 */
    ADCDAC_IO_ANA0_2    = 1 << 2, /*!< External Analog Bus pin ana0_2 */
    ADCDAC_IO_ANA0_3    = 1 << 3, /*!< External Analog Bus pin ana0_3 */
    ADCDAC_IO_ANA0_4    = 1 << 4, /*!< External Analog Bus pin ana0_4 */
    ADCDAC_IO_ANA0_5    = 1 << 5, /*!< External Analog Bus pin ana0_5 */
    ADCDAC_IO_ANA0_6    = 1 << 6, /*!< External Analog Bus pin ana0_6 */
    ADCDAC_IO_ANA0_7    = 1 << 7, /*!< External Analog Bus pin ana0_7 */
    ADCDAC_IO_ANA0_8    = 1 << 8, /*!< External Analog Bus pin ana0_8 */
    ADCDAC_IO_ANA0_9    = 1 << 9, /*!< External Analog Bus pin ana0_9 */
    ADCDAC_IO_ANA0_10   = 1 << 10, /*!< External Analog Bus pin ana0_10 */
    ADCDAC_IO_ANA0_11   = 1 << 11, /*!< External Analog Bus pin ana0_11 */
    ADCDAC_IO_INTBUS_0  = 1 << 12, /*!< Internal Bus Top-Left 1 */
    ADCDAC_IO_INTBUS_2  = 1 << 13, /*!< Internal Bus Top-Left 2 */
    ADCDAC_IO_INTBUS_4  = 1 << 14, /*!< Internal Bus Top-Right 1 */
    ADCDAC_IO_INTBUS_6  = 1 << 15, /*!< Internal Bus Top-Right 2 */
    ADCDAC_IO_INTBUS_8  = 1 << 16, /*!< Internal Bus Bottom-Right 1 */
    ADCDAC_IO_INTBUS_10 = 1 << 17, /*!< Internal Bus Bottom-Right 2 */
    ADCDAC_IO_INTBUS_12 = 1 << 18, /*!< Internal Bus Bottom-Left 1 */
    ADCDAC_IO_INTBUS_14 = 1 << 19, /*!< Internal Bus Bottom-Left 2 */
} ADCDAC_IO_T;

/** Conversion Status */
typedef enum ADCDAC_STATUS {
    ADCDAC_STATUS_DAC_DONE         = 1 << 0, /*!< DAC operation completed */
    ADCDAC_STATUS_ADC_DONE         = 1 << 1, /*!< ADC operation completed */
    ADCDAC_STATUS_CONVERTER_IN_OPERATION = 1 << 4, /*!< ADC or DAC in progress (this status is SW derived) */
} ADCDAC_STATUS_T;

/** ADC Input range */
typedef enum ADCDAC_INPUTRANGE {
    ADCDAC_INPUTRANGE_NARROW = 0, /*!< selects input range of 0V - 1V */
    ADCDAC_INPUTRANGE_WIDE = 1, /*!< selects input range of 0V - 1V6 */
} ADCDAC_INPUTRANGE_T;

/** ADCDAC Interrupt flags */
typedef enum ADCDAC_INT {
    ADCDAC_INT_CONVERSION_RDY_DAC   = 1 << 0, /*!< DAC Done */
    ADCDAC_INT_CONVERSION_RDY_ADC   = 1 << 1, /*!< ADC Done */
    ADCDAC_INT_THRESHOLD_LOW_ADC    = 1 << 2, /*!< ADC Low native value interrupt */
    ADCDAC_INT_THRESHOLD_HIGH_ADC   = 1 << 3, /*!< ADC High native value interrupt */
    ADCDAC_INT_NONE                 = 0, /*!< Disable all Interrupt */
    ADCDAC_INT_ALL                  = 0x0F, /*!< All valid bits in ADCDAC_INT_T */
} ADCDAC_INT_T;

/**
 * Initializes both ADC & DAC peripherals with default settings.
 * This function also enables the ADCDAC peripheral power and clock.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @note Default settings for ADC/DAC are:
 *  - No ADC & DAC connection
 *  - Single-shot mode
 *  - ADC input range of 0V - 1V
 */
void Chip_ADCDAC_Init(NSS_ADCDAC_T *pADCDAC);

/**
 * Disables the ADC & DAC hardware.
 * This function disables the ADCDAC peripheral power, clock and interrupts (if enabled),
 * which means that the result of any ongoing conversion will be lost.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 */
void Chip_ADCDAC_DeInit(NSS_ADCDAC_T *pADCDAC);

/**
 * Sets the operating mode for ADC
 * @param pADCDAC : The base address of the ADC peripheral on the chip
 * @param mode : ADC operating mode to set
 */
void Chip_ADCDAC_SetModeADC(NSS_ADCDAC_T *pADCDAC, ADCDAC_MODE_T mode);

/**
 * Sets the operating mode for DAC
 * @param pADCDAC : The base address of the DAC peripheral on the chip
 * @param mode : DAC operating mode to set
 */
void Chip_ADCDAC_SetModeDAC(NSS_ADCDAC_T *pADCDAC, ADCDAC_MODE_T mode);

/**
 * Gets the Analog-to-digital converter configured operating mode.
 * @param pADCDAC : The base address of the ADC peripheral on the chip
 * @return Configured ADC operating mode
 */
ADCDAC_MODE_T Chip_ADCDAC_GetModeADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Gets the digital-to-analog converter configured operating mode.
 * @param pADCDAC : The base address of the DAC peripheral on the chip
 * @return Configured DAC operating mode
 */
ADCDAC_MODE_T Chip_ADCDAC_GetModeDAC(NSS_ADCDAC_T *pADCDAC);

/**
 * Sets IO connection for ADC
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param connection : bitvector of ADCDAC_IO_T type representing the IO(s) connected to the ADC.
 * @note This function ungrounds the respective analog bus(ses). Refer to @ref anaBusGndDesc_anchor "Analog Bus Grounding".
 */
void Chip_ADCDAC_SetMuxADC(NSS_ADCDAC_T *pADCDAC, ADCDAC_IO_T connection);

/**
 * Sets IO connection DAC
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param connection : bitvector of ADCDAC_IO_T type representing the IO(s) connected to the DAC.
 * @note This function ungrounds the respective analog bus(ses). Refer to @ref anaBusGndDesc_anchor "Analog Bus Grounding".
 */
void Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC_T *pADCDAC, ADCDAC_IO_T connection);

/**
 * Get IO connection for ADC
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return ADC connection bitvector of ADCDAC_IO_T type
 */
ADCDAC_IO_T Chip_ADCDAC_GetMuxADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Get IO connection for DAC
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return DAC connection bitvector of ADCDAC_IO_T type.
 */
ADCDAC_IO_T Chip_ADCDAC_GetMuxDAC(NSS_ADCDAC_T *pADCDAC);

/**
 * Set ADC Input range
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param inputRange : The required ADC input range to set
 */
void Chip_ADCDAC_SetInputRangeADC(NSS_ADCDAC_T *pADCDAC, ADCDAC_INPUTRANGE_T inputRange);

/**
 * Get ADC Input range
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return The selected ADC input range
 */
ADCDAC_INPUTRANGE_T Chip_ADCDAC_GetInputRangeADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Start ADC operation
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @note ADC conversion time is approximately 12us.
 *  Refer to @ref convStateDesc_anchor "Converter State Description" for
 *  more details on all possible analog-to-digital converter status flags with #Chip_ADCDAC_ReadStatus function
 */
void Chip_ADCDAC_StartADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Start DAC operation. Repeat last value written with #Chip_ADCDAC_WriteOutputDAC.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @note DAC settling time is about 25us.
 *  Calling #Chip_ADCDAC_WriteOutputDAC automatically starts DAC output operation.
 *  Refer to @ref convStateDesc_anchor "Converter State Description" for more details on all possible
 *  analog-to-digital converter status flags with #Chip_ADCDAC_ReadStatus function
 */
void Chip_ADCDAC_StartDAC(NSS_ADCDAC_T *pADCDAC);

/**
 * Stops the ADC, when in #ADCDAC_CONTINUOUS mode, No effect in #ADCDAC_SINGLE_SHOT mode.
 * After this function is called, the AD converter will only be stopped once the ongoing conversion ends.
 * However this function immediately returns, not waiting for the end of the ongoing conversion.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @note This function is only applicable when the AD converter operating mode is set to #ADCDAC_CONTINUOUS.
 *  It is not possible to immediately stop an ongoing conversion without powering off the ADCDAC peripheral.
 */
void Chip_ADCDAC_StopADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Stops the DAC, when in #ADCDAC_CONTINUOUS mode, No effect in #ADCDAC_SINGLE_SHOT mode.
 * After this function is called, the ADC converter will only be stopped once the ongoing conversion ends.
 * However this function immediately returns, not waiting for the end of the ongoing conversion.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @note This function is only applicable when the DA converter operating mode is set to #ADCDAC_CONTINUOUS.
 *  It is not possible to immediately stop an ongoing conversion without powering off the ADCDAC peripheral.
 */
void Chip_ADCDAC_StopDAC(NSS_ADCDAC_T *pADCDAC);

/**
 * Gets the status of ADC/DAC driver
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return Status of Analog-to-digital / digital-to-analog converter.
 *  Refer to @ref convStateDesc_anchor "Converter State Description" for more details on all possible
 *  Analog-to-digital / digital-to-analog converter status flags.
 */
ADCDAC_STATUS_T Chip_ADCDAC_ReadStatus(NSS_ADCDAC_T *pADCDAC);

/**
 * Reads the latest ADC value. If operation has started and is still in progress, last value is returned.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return Value of 12-bit ADC input.
 * @note Reading ADC value causes interrupt to be cleared. It is unnecessary to clear
 *      #ADCDAC_INT_CONVERSION_RDY_ADC if a #Chip_ADCDAC_GetValueADC has been called.
 */
int Chip_ADCDAC_GetValueADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Function writes DAC output value and natively calls #Chip_ADCDAC_StartDAC to effect the output.
 * Use #Chip_ADCDAC_ReadStatus and check for #ADCDAC_STATUS_DAC_DONE bit return for DAC complete indication
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param native : 12-bit value to write to DAC
 * @note DAC settling time is about 25us. #Chip_ADCDAC_WriteOutputDAC automatically starts DAC output operation
 *  by calling #Chip_ADCDAC_StartDAC.
 */
void Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC_T *pADCDAC, int native);

/**
 * Set ADC Low Threshold Detection. When sampled ADC value is lower than threshold,
 *  an interrupt is signaled (if enabled), and #Chip_ADCDAC_Int_GetRawStatus returns with
 *  #ADCDAC_INT_THRESHOLD_LOW_ADC bit set.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param native : 12-bit threshold value
 */
void Chip_ADCDAC_Int_SetThresholdLowADC(NSS_ADCDAC_T *pADCDAC, int native);

/**
 * Set ADC High Threshold Detection. When sampled ADC value is higher than threshold,
 * an interrupt is signaled (if enabled), and #Chip_ADCDAC_Int_GetRawStatus returns with
 * #ADCDAC_INT_THRESHOLD_HIGH_ADC bit set.
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param native : 12-bit Threshold value
 */
void Chip_ADCDAC_Int_SetThresholdHighADC(NSS_ADCDAC_T *pADCDAC, int native);

/**
 * Retrieve current configured ADC Low Threshold Value
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return 12-bit ADC low-threshold value
 * @see ADCDAC_INT_THRESHOLD_LOW_ADC
 */
int Chip_ADCDAC_Int_GetThresholdLowADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Retrieve current configured ADC High Threshold Value
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return 12-bit ADC high-threshold value
 * @see ADCDAC_INT_THRESHOLD_HIGH_ADC
 */
int Chip_ADCDAC_Int_GetThresholdHighADC(NSS_ADCDAC_T *pADCDAC);

/**
 * Enables/Disables the ADCDAC interrupts
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param mask : Interrupt enabled mask to set
 * @note Enabling interrupt in continuous mode could cause rapid interrupt events.
 */
void Chip_ADCDAC_Int_SetEnabledMask(NSS_ADCDAC_T *pADCDAC, ADCDAC_INT_T mask);

/**
 * Retrieves the ADCDAC interrupt enabled mask
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return Interrupt enabled mask
 */
ADCDAC_INT_T Chip_ADCDAC_Int_GetEnabledMask(NSS_ADCDAC_T *pADCDAC);

/**
 * Retrieves a bitVector with the RAW ADCDAC interrupt flags
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @return BitVector with the ADCDAC RAW interrupt flags
 * @note A bit set to 1 means that the correspondent interrupt flag is set.
 */
ADCDAC_INT_T Chip_ADCDAC_Int_GetRawStatus(NSS_ADCDAC_T *pADCDAC);

/**
 * Clears the required ADCDAC interrupt flags
 * @param pADCDAC : The base address of the ADC/DAC peripheral on the chip
 * @param flags : BitVector indicating which interrupt flags to clear
 * @note Reading the ADCDAC value (#Chip_ADCDAC_GetValueADC) also causes the interrupt flags to be cleared.
 */
void Chip_ADCDAC_Int_ClearRawStatus(NSS_ADCDAC_T *pADCDAC, ADCDAC_INT_T flags);

/**
 * @}
 */

#endif
