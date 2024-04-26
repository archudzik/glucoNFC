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

#ifndef __GPIO_NSS_H_
#define __GPIO_NSS_H_

/** @defgroup GPIO_NSS gpio: General Purpose Input/Output (GPIO) driver
 * @ingroup DRV_NSS
 * The general purpose input/output (GPIO) driver:
 *  -# selects directions of digital IO pins as inputs or outputs,
 *  -# configures interrupt events of input pins
 *  -# sets the states of output pins and gets the state of input pins.
 *
 * The IOCON driver has to be used to configure the digital IO pins as GPIO.
 * All digital IO pins, in most cases defaults to GPIO function mode at startup and are configured as inputs without
 * interrupt. See IO Configuration driver @ref IOCON_NSS for details.
 *
 * The following naming convention is used in this GPIO driver:
 *  -# Functions named with "Pin", e.g. Chip_GPIO_SetPinDIROutput, influence one specific pin per function call
 *     (e.g. '0' for pin 0, '1' for pin 1, etc.). However, the APIs Chip_GPIO_SetPinModeEdge and
 *     Chip_GPIO_SetPinModeLevel do not follow this rule and influence a complete port.
 *  -# Functions named with "Port", e.g. Chip_GPIO_SetPortDIROutput, influence multiple pins as specified with a
 *     bitmask (e.g. pin 2 and 3 as (1<<2 | 1<<3), or (NSS_GPIOn_PINMASK(2) | NSS_GPIOn_PINMASK(3)) )
 *  -# "Pin State" refers to the level (high or low) of individual pin
 *  -# "Port Value" refers to the OR'ed value of the "pin states" of all digital pins on the port.
 *
 * @par To use this driver:
 *  -# This GPIO driver is initialised with #Chip_GPIO_Init and disabled with #Chip_GPIO_DeInit
 *  -# The Pin directions are configured with:
 *      -# (Individual pin) #Chip_GPIO_SetPinDIROutput, #Chip_GPIO_SetPinDIRInput, #Chip_GPIO_SetPinDIR and
 *      -# (Port-wide with pin mask) #Chip_GPIO_SetPortDIROutput, #Chip_GPIO_SetPortDIRInput, #Chip_GPIO_SetPortDIR
 *  -# The Pin direction configurations can be retrieved with:
 *      -# (Individual pin) #Chip_GPIO_GetPinDIR and
 *      -# (Port-wide with pin mask) #Chip_GPIO_GetPortDIR
 *  -# The Input states are read with:
 *      -# (Individual pin) #Chip_GPIO_GetPinState
 *      -# (Port-wide with pin mask) #Chip_GPIO_GetPortValue
 *  -# The GPIO interrupts are configured with:
 *      -# #Chip_GPIO_EnableInt, #Chip_GPIO_DisableInt, #Chip_GPIO_GetEnabledInts and
 *      -# #Chip_GPIO_GetRawInts, #Chip_GPIO_GetMaskedInts, #Chip_GPIO_ClearInts
 *      -# #Chip_GPIO_SetupPinInt,
 *      -# #Chip_GPIO_SetPinModeEdge, #Chip_GPIO_SetPinModeLevel, #Chip_GPIO_IsLevelEnabled,
 *      -# #Chip_GPIO_SetEdgeModeBoth, #Chip_GPIO_SetEdgeModeSingle, #Chip_GPIO_GetEdgeModeDir and
 *      -# #Chip_GPIO_SetModeHigh, #Chip_GPIO_SetModeLow, #Chip_GPIO_GetModeHighLow
 *  -# The Output pins are controlled with:
 *      -# (Individual pin) #Chip_GPIO_SetPinOutHigh, #Chip_GPIO_SetPinOutLow, #Chip_GPIO_SetPinToggle,
 *      -# (Individual pin) #Chip_GPIO_SetPinState, #Chip_GPIO_GetPinState,
 *      -# (Port-wide with pin mask) #Chip_GPIO_SetPortOutHigh, #Chip_GPIO_SetPortOutLow, #Chip_GPIO_SetPortToggle and
 *      -# (Port-wide with pin mask) #Chip_GPIO_SetPortValue, #Chip_GPIO_GetPortValue
 *
 *
 * @par Example 1 - Configuring individual pins
 *  - Inputs:
 *      - Pin 0 with falling edge event interrupt
 *      - Pin 1 with both falling and rising edge event interrupt
 *      - Pin 2 with level-sensitive (high) event interrupt
 *      - Pin 3 without event interrupt
 *  - Operate Outputs:
 *      - Pin 4 in High --> Low --> High --> (toggle) --> (toggle) sequence
 *  @snippet gpio_nss_example_1.c gpio_nss_example_1
 *  Interrupt Handler:
 *  @snippet gpio_nss_example_1.c gpio_nss_example_1_irq
 *
 * @par Example 2 - Operate output pin (4, 5, 6, 7) as a group. All other pins shall be inputs
 *  - Operate Outputs:
 *      - Pin 4 in High --> Low --> High --> (toggle) --> (toggle) sequence
 *      - Pin 5 in Low --> High --> Low --> (toggle) --> (toggle) sequence
 *      - Pin 6 in Low --> High --> Low --> (toggle) --> (toggle) sequence
 *      - Pin 7 in High --> Low --> High --> (toggle) --> (toggle) sequence
 *  @snippet gpio_nss_example_2.c gpio_nss_example_2
 *
 * @{
 */

/** GPIO port register block structure */
typedef struct NSS_GPIO_S {
    __IO uint32_t DATA[4096]; /*!< Offset: 0x0000 to 0x3FFC Data address masking register (R/W) */
         uint32_t RESERVED1[4096];
    __IO uint32_t DIR; /*!< Offset: 0x8000 Data direction register (R/W) */
    __IO uint32_t IS; /*!< Offset: 0x8004 Interrupt sense register (R/W) */
    __IO uint32_t IBE; /*!< Offset: 0x8008 Interrupt both edges register (R/W) */
    __IO uint32_t IEV; /*!< Offset: 0x800C Interrupt event register (R/W) */
    __IO uint32_t IE; /*!< Offset: 0x8010 Interrupt mask register (R/W) */
    __I  uint32_t RIS; /*!< Offset: 0x8014 Raw interrupt status register (R) */
    __I  uint32_t MIS; /*!< Offset: 0x8018 Masked interrupt status register (R) */
    __O  uint32_t IC; /*!< Offset: 0x801C Interrupt clear register (W) */
         uint32_t RESERVED2[8184]; /* Padding added for aligning contiguous GPIO blocks */
} NSS_GPIO_T;

/** Pin x mask definition */
#define NSS_GPIOn_PINMASK(x) (1<<(x))

/** GPIO interrupt mode definitions */
typedef enum GPIO_INT_MODE {
    GPIO_INT_ACTIVE_LOW_LEVEL = 0x0, /*!< Selects interrupt on pin to be triggered on LOW level */
    GPIO_INT_ACTIVE_HIGH_LEVEL = 0x1, /*!< Selects interrupt on pin to be triggered on HIGH level */
    GPIO_INT_FALLING_EDGE = 0x2, /*!< Selects interrupt on pin to be triggered on FALLING level */
    GPIO_INT_RISING_EDGE = 0x3, /*!< Selects interrupt on pin to be triggered on RISING level */
    GPIO_INT_BOTH_EDGES = 0x6 /*!< Selects interrupt on pin to be triggered on both edges */
} GPIO_INT_MODE_T;

/**
 * Initialise GPIO block
 * @param pGPIO : The base address of GPIO peripheral on the chip
 */
void Chip_GPIO_Init(NSS_GPIO_T *pGPIO);

/**
 * De-Initialise GPIO block
 * @param pGPIO : The base address of GPIO peripheral on the chip
 */
void Chip_GPIO_DeInit(NSS_GPIO_T *pGPIO);

/**
 * Set a GPIO pin state via the GPIO byte register
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : GPIO pin to set
 * @param setting : true for setting pin to high, false for low
 */
static inline void Chip_GPIO_SetPinState(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin, bool setting)
{
    pGPIO[port].DATA[1 << pin] = (uint32_t)setting << pin;
}

/**
 * Get a GPIO pin state via the GPIO byte register
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : GPIO pin to get state for
 * @return true if the GPIO pin is high, false if low
 */
static inline bool Chip_GPIO_GetPinState(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    return (pGPIO[port].DATA[1 << pin]) != 0;
}

/**
 * Set GPIO direction for a single GPIO pin to an output
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : GPIO pin to set direction as output
 */
static inline void Chip_GPIO_SetPinDIROutput(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    pGPIO[port].DIR |= (1UL << pin);
}

/**
 * Set GPIO direction for a single GPIO pin to an input
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : GPIO pin to set direction as input
 */
static inline void Chip_GPIO_SetPinDIRInput(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    pGPIO[port].DIR &= ~(1UL << pin);
}

/**
 * Set GPIO direction for a single GPIO pin
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : GPIO pin to set direction for
 * @param output : true for setting as output, false for input
 */
void Chip_GPIO_SetPinDIR(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin, bool output);

/**
 * Get GPIO direction for a single GPIO pin
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : GPIO pin to get direction for
 * @return true if the GPIO pin is set as an output, false for input
 */
static inline bool Chip_GPIO_GetPinDIR(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    return (bool)((pGPIO[port].DIR >> pin) & 1);
}

/**
 * Set GPIO direction for all selected GPIO pins to an output
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinMask : GPIO pin mask to set direction on as output (ORed value of bits 0..11 for pins 0..11)
 * @note Sets multiple GPIO pins to the output direction. Each bit's position that is high, sets the corresponding
 *  pin number for that bit to an output.
 */
static inline void Chip_GPIO_SetPortDIROutput(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask)
{
    pGPIO[port].DIR |= pinMask;
}

/**
 * Set GPIO direction for all selected GPIO pins to an input
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinMask : GPIO pin mask to set direction on as input (ORed value of bits 0..11 for pins 0..11)
 * @note Sets multiple GPIO pins to the input direction. Each bit's position that is high, sets the corresponding
 *  pin number for that bit to an input.
 */
static inline void Chip_GPIO_SetPortDIRInput(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask)
{
    pGPIO[port].DIR &= ~pinMask;
}

/**
 * Set GPIO direction for all selected GPIO pins to an input or output
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinMask : GPIO pin mask to set direction on (ORed value of bits 0..11 for pins 0..11)
 * @param outSet : Direction value. Use false to set as inputs and true to set as outputs
 * @note Sets multiple GPIO pins to input/output direction. If the bit is high, then the corresponding pin is set
 *  to output. Similarly, if the bit is low, then the corresponding pin is set as input.
 */
void Chip_GPIO_SetPortDIR(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask, bool outSet);

/**
 * Get GPIO direction for all GPIO pins
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return a bit-field containing the input and output direction setting for each pin
 * @note For pins 0..11, a high in a bit corresponds to an output direction for the same pin, while a low
 *  corresponds to an input direction.
 */
static inline uint32_t Chip_GPIO_GetPortDIR(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].DIR;
}

/**
 * Set all GPIO raw pin states as a group
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param value : A 12-bit value to set all GPIO pin states (0..11) to
 */
static inline void Chip_GPIO_SetPortValue(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t value)
{
    pGPIO[port].DATA[0xFFF] = value;
}

/**
 * Get all GPIO raw pin states as a group
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return Current (raw) state of all GPIO pins (0..11)
 */
static inline uint32_t Chip_GPIO_GetPortValue(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].DATA[0xFFF];
}

/**
 * Set selected GPIO output pins to the high state
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinMask : A pin mask to select pins to be set as high (ORed value of bits 0..11 for pins 0..11)
 * @note Any bit set as a '0' will not have it's state changed. Setting the state only applies to pins configured
 *  as an output.
 */
static inline void Chip_GPIO_SetPortOutHigh(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask)
{
    pGPIO[port].DATA[pinMask] = 0xFFF;
}

/**
 * Set an individual GPIO output pin to the high state
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : pin number (0..11) to be set high
 * @note Setting the state only applies to pins configured as an output.
 */
static inline void Chip_GPIO_SetPinOutHigh(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    pGPIO[port].DATA[1 << pin] = (1u << pin);
}

/**
 * Set selected GPIO output pins to the low state
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinMask : A pin mask to select pin to be set as low (ORed value of bits 0..11 for pins 0..11)
 * @note Any bit set as a '0' will not have it's state changed. Setting the state only applies to pins configured
 *  as an output.
 */
static inline void Chip_GPIO_SetPortOutLow(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask)
{
    pGPIO[port].DATA[pinMask] = 0;
}

/**
 * Set an individual GPIO output pin to the low state
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : pin number (0..11) to set low
 * @note Setting the state only applies to pins configured as an output.
 */
static inline void Chip_GPIO_SetPinOutLow(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    pGPIO[port].DATA[1 << pin] = 0;
}

/**
 * Toggle selected GPIO output pins to the opposite state
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinMask : A pin mask to select pins to be toggled (ORed value of bits 0..11 for pins 0..11)
 * @note Any bit set as a '0' will not have it's state changed. Toggling the state only applies to pins configured
 *  as an output.
 */
static inline void Chip_GPIO_SetPortToggle(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinMask)
{
    pGPIO[port].DATA[pinMask] ^= 0xFFF;
}

/**
 * Toggle an individual GPIO output pin to the opposite state
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : pin number (0..11) to toggle
 * @note Toggling the state only applies to pins configured as an output.
 */
static inline void Chip_GPIO_SetPinToggle(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin)
{
    pGPIO[port].DATA[1 << pin] ^= (1u << pin);
}

/**
 * Configure the pins as edge sensitive for interrupts
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to be set to edge trigger mode (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_SetPinModeEdge(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IS &= ~pinmask;
}

/**
 * Configure the pins as level sensitive for interrupts
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to be set to level trigger mode (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_SetPinModeLevel(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IS |= pinmask;
}

/**
 * Returns current GPIO edge or high level interrupt configuration for all pins of a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return A bit-field containing the edge/level interrupt configuration for each pin for the selected port.
 *  Bit 0 corresponds to pin 0, 1 to pin 1. and so on. For each bit, a 0 means the edge interrupt is configured,
 *  while a 1 means a level interrupt is configured. Mask with this return value to determine the edge/level
 *  configuration for each pin in a port.
 */
static inline uint32_t Chip_GPIO_IsLevelEnabled(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].IS;
}

/**
 * Sets GPIO interrupt configuration for both edges for selected pins
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to be set to dual edge trigger mode (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_SetEdgeModeBoth(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IBE |= pinmask;
}

/**
 * Sets GPIO interrupt configuration for a single edge for selected pins
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to be set to single edge trigger mode (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_SetEdgeModeSingle(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IBE &= ~pinmask;
}

/**
 * Returns current GPIO interrupt single or dual edge configuration for all pins of a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return A bit-field containing the single/dual edge interrupt configuration for each pin for the selected port.
 *  Bit 0 corresponds to pin 0, 1 to pin 1. and so on. For each bit, a 0 means the interrupt triggers on a single edge
 *  (use the Chip_GPIO_SetEdgeModeHigh() and Chip_GPIO_SetEdgeModeLow() functions to configure selected edge),
 *  while a 1 means the interrupt triggers on both edges. Mask with this return value to determine the edge/level
 *  configuration for each pin in a port.
 */
static inline uint32_t Chip_GPIO_GetEdgeModeDir(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].IBE;
}

/**
 * Sets GPIO interrupt configuration, when in single edge or level mode to trigger on high edge or high level
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to be set to high mode (ORed value of bits 0..11)
 * @note Use this function to select high level or high edge interrupt mode for the selected pins on the selected
 *  port when not in dual edge mode.
 */
static inline void Chip_GPIO_SetModeHigh(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IEV |= pinmask;
}

/**
 * Sets GPIO interrupt configuration, when in single edge or level mode to trigger on low edge or low level
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to be set to low mode (ORed value of bits 0..11)
 * @note Use this function to select low level or low edge interrupt mode for the selected pins on the selected port
 *  when not in dual edge mode.
 */
static inline void Chip_GPIO_SetModeLow(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IEV &= ~pinmask;
}

/**
 * Returns current GPIO interrupt edge direction or level mode
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return A bit-field containing the low or high direction of the interrupt configuration for each pin for the
 *  selected port. Bit 0 corresponds to pin 0, 1 to pin 1. and so on. For each bit, a 0 means the interrupt triggers
 *  on a low level or edge, while a 1 means the interrupt triggers on a high level or edge. Mask with this return value
 *  to determine the high/low configuration for each pin in a port.
 */
static inline uint32_t Chip_GPIO_GetModeHighLow(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].IEV;
}

/**
 * Enables interrupts for selected pins on a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to enable interrupts on (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_EnableInt(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IE |= pinmask;
}

/**
 * Disables interrupts for selected pins on a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins to disable interrupts on (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_DisableInt(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IE &= ~pinmask;
}

/**
 * Returns current interrupt enabled pins for a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return A bit-field containing the enabled pin interrupts (0..11)
 */
static inline uint32_t Chip_GPIO_GetEnabledInts(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].IE;
}

/**
 * Returns raw interrupt pending status for all pins for a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return A bit-field containing the pending raw interrupt states for each pin (0..11) on the port
 */
static inline uint32_t Chip_GPIO_GetRawInts(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].RIS;
}

/**
 * Returns masked interrupt pending status for all pins for a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @return A bit-field containing the pending masked interrupt states for each pin (0..11) on the port
 */
static inline uint32_t Chip_GPIO_GetMaskedInts(NSS_GPIO_T *pGPIO, uint8_t port)
{
    return pGPIO[port].MIS;
}

/**
 * Clears pending interrupts for selected pins for a port
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pinmask : A pin mask to select pins on to clear interrupts on (ORed value of bits 0..11)
 */
static inline void Chip_GPIO_ClearInts(NSS_GPIO_T *pGPIO, uint8_t port, uint32_t pinmask)
{
    pGPIO[port].IC = pinmask;
}

/**
 * Composite function for setting up a full interrupt configuration for a single pin
 * @param pGPIO : The base address of GPIO peripheral on the chip
 * @param port : Port number
 * @param pin : Pin number (0..11)
 * @param mode : GPIO interrupt mode selection
 */
void Chip_GPIO_SetupPinInt(NSS_GPIO_T *pGPIO, uint8_t port, uint8_t pin, GPIO_INT_MODE_T mode);

#endif /** @} */
