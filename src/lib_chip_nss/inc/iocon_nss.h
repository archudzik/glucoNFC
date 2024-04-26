/*
 * Copyright 2014-2016,2018-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __IOCON_NSS_H_
#define __IOCON_NSS_H_

/** @defgroup IOCON_NSS iocon: IO Configuration driver
 * @ingroup DRV_NSS
 * This driver select the peripheral IO (PIO) pin function of digital pins (e.g. PIO or I2C clock) and analog pins.
 * The characteristics include peripheral function, analog pins functionality, GPIO and its electrical modes
 * (e.g. pull-up or pull-down), the low-pass input filter modes, and current drive output mode of all pins.
 * In addition, the I2C-bus pins can be configured for different I2C-bus modes.
 *
 * The pin function determine the peripheral function of a pin, and with that the direction.
 * For example, if the pins are configured as GPIO pins, the #Chip_GPIO_SetPortDIR of @ref GPIO_NSS "GPIO driver"
 * determine whether the pin is configured as an input or output.
 * For other peripheral function, the pin direction is controlled automatically depending on the pin's functionality.
 *
 * The pin resistor modes allow enabling of on-chip pull-up or pull-down resistors or to select the repeater mode.
 * The repeater mode enables the pull-up resistor if the pin is at a logic HIGH and enables the
 * pull-down resistor if the pin is at a logic LOW. This causes the pin to retain its last known state if it is
 * configured as an input and is not driven externally. The state retention is not applicable to the Deep power-down
 * mode. Repeater mode may typically be used to prevent a pin from floating (and potentially using significant power
 * if it floats to an indeterminate state) if it is temporarily not driven.
 *
 * @par I2C function mode:
 *  If the I2C function is selected by the pin function (for designated pins), then the I2C-bus pins can be configured
 *  for two I2C-modes:
 *   -# Standard mode/Fast-mode I2C with input glitch filter (this includes an open-drain output according to the
 *      I2C-bus specification).
 *   -# Standard open-drain I/O functionality without input filter.
 *  The I2C pads have no pull-up / pull down functionality.
 *
 * @anchor iocon_currentdrive_anchor
 * @par Current Drive Capabilities:
 *  Some pins have high current drive output capabilities (up to 20 mA when VDD is 1.8V) to the load. See user manual
 *  for pins with such capabilities. These PIO pins can be set to either digital mode or current drive mode. @n
 *   - In digital mode (#IOCON_CDRIVE_FIXEDVOLTAGE), the output voltage of the pad switches between VDD and VSS.
 *     In addition, #IOCON_DDRIVE_HIGH and #IOCON_DDRIVE_ULTRAHIGH indicates the level of internal amplification that
 *     provides up to 20mA at different level of VDD voltage. @n
 *     For example, active high output with #IOCON_CDRIVE_FIXEDVOLTAGE & #IOCON_DDRIVE_HIGH, gives 20mA when VDD is 3.1V,
 *     while #IOCON_CDRIVE_FIXEDVOLTAGE & #IOCON_DDRIVE_ULTRAHIGH gives 20mA when VDD is 2.4V. @n
 *   - In current drive mode (#IOCON_CDRIVE_PROGRAMMABLECURRENT), the output current switches between IDrive_low (ILO) and
 *     IDrive_high (IHI), as configured with the #IOCON_ILO_VAL and #IOCON_IHI_VAL macro. The current sunk from the load
 *     depends on the supply at the load. An internal circuit equivalent to a variable resistor can be configured to deliver
 *     a maximum of 20 mA at 1.8V supply, i.e. IHI = 0xFF, scaled linearly over 8-bit resolution from ILO = 0 (0mA).
 *     The maximum pad voltage is limited to 5V.
 *
 * @par Analog Functionality:
 *  The analog input pins (IOCON_ANA0_*) have direct analog connections to the external analog busses, and are protected
 *  by the ESD structures. The pin function determines the interconnections, e.g. to VSS or to associated analog bus.
 *  Refer to user manual for pin specific functions.
 *
 * @anchor anaBusGndDesc_anchor
 * @par Analog Bus Grounding:
 *  This feature allows the internal grounding of the analog busses. The chip has analog buses that connect the
 *  respective pin to the muxes of the analog blocks (ADCDAC, I2D, C2D). All these buses are grounded by default.
 *  During operation of analog blocks (ADCDAC, I2D, C2D), analog bus(es) must be ungrounded. This is ensured by
 *  usage of #Chip_IOCON_SetPinConfig or by the respective analog block driver in its Chip_*_SetMux* function.
 *  @warning If the analog bus is no longer used, it is the responsibility of the application to ground it
 *  again using the #Chip_IOCON_GroundAnabus (not doing so might impact the accuracy of the analog block(s)).
 *
 * @par Example 1 - Configuring pins
 *  - PIO0_0 as GPIO with no pull-up/down and LPF disabled
 *  - PIO0_1 as GPIO with pull-up and LPF enabled
 *  - PIO0_2 as peripheral function 1 (see user manual for respective modes)
 *  - PIO0_3 as high current drive mode, with no pull-up/down LPF disabled, output low current of 4mA and output high
 *    current of 16mA
 *  - PIO0_4 as I2C peripheral function, standard or fast I2C mode
 *  @snippet iocon_nss_example_1.c iocon_nss_example_1
 *
 * @{
 */

/** NSS IOCON register block structure */
typedef struct NSS_IOCON_S {
    __IO uint32_t REG[24]; /*!< 12 digital pins and 12 analog pins. */
    __IO uint32_t RESERVED0[8]; /* next field at offset 0x080 */
    __IO uint32_t ANABUSGROUND; /*!< Analog bus grounding control. */
} NSS_IOCON_T;

/** I/O Pins Definitions */
typedef enum IOCON_PIN {
    IOCON_PIO0_0 = (0x000 >> 2), /*!< Represents the digital pin 0 */
    IOCON_PIO0_1 = (0x004 >> 2), /*!< Represents the digital pin 1 */
    IOCON_PIO0_2 = (0x008 >> 2), /*!< Represents the digital pin 2 */
    IOCON_PIO0_3 = (0x00C >> 2), /*!< Represents the digital pin 3 */
    IOCON_PIO0_4 = (0x010 >> 2), /*!< Represents the digital pin 4 */
    IOCON_PIO0_5 = (0x014 >> 2), /*!< Represents the digital pin 5 */
    IOCON_PIO0_6 = (0x018 >> 2), /*!< Represents the digital pin 6 */
    IOCON_PIO0_7 = (0x01C >> 2), /*!< Represents the digital pin 7 */
    IOCON_PIO0_8 = (0x020 >> 2), /*!< Represents the digital pin 8 */
    IOCON_PIO0_9 = (0x024 >> 2), /*!< Represents the digital pin 9 */
    IOCON_PIO0_10 = (0x028 >> 2),/*!< Represents the digital pin 10 */
    IOCON_PIO0_11 = (0x02C >> 2),/*!< Represents the digital pin 11 */

    IOCON_ANA0_0 = (0x030 >> 2), /*!< Represents the analog pin 0 */
    IOCON_ANA0_1 = (0x034 >> 2), /*!< Represents the analog pin 1 */
    IOCON_ANA0_2 = (0x038 >> 2), /*!< Represents the analog pin 2 */
    IOCON_ANA0_3 = (0x03C >> 2), /*!< Represents the analog pin 3 */
    IOCON_ANA0_4 = (0x040 >> 2), /*!< Represents the analog pin 4 */
    IOCON_ANA0_5 = (0x044 >> 2), /*!< Represents the analog pin 5 */
    IOCON_ANA0_6 = (0x048 >> 2), /*!< Represents the analog pin 6 */
    IOCON_ANA0_7 = (0x04C >> 2), /*!< Represents the analog pin 7 */
    IOCON_ANA0_8 = (0x050 >> 2), /*!< Represents the analog pin 8 */
    IOCON_ANA0_9 = (0x054 >> 2), /*!< Represents the analog pin 9 */
    IOCON_ANA0_10 = (0x058 >> 2),/*!< Represents the analog pin 10 */
    IOCON_ANA0_11 = (0x05C >> 2), /*!< Represents the analog pin 11 */
} IOCON_PIN_T;

/** Analog Bus Definitions */
typedef enum IOCON_ANABUS {
    IOCON_ANABUS_EXT0 = (1 << 0), /*!< Represents the ana_extbus0 */
    IOCON_ANABUS_EXT1 = (1 << 1), /*!< Represents the ana_extbus1 */
    IOCON_ANABUS_EXT2 = (1 << 2), /*!< Represents the ana_extbus2 */
    IOCON_ANABUS_EXT3 = (1 << 3), /*!< Represents the ana_extbus3 */
    IOCON_ANABUS_EXT4 = (1 << 4), /*!< Represents the ana_extbus4 */
    IOCON_ANABUS_EXT5 = (1 << 5), /*!< Represents the ana_extbus5 */
    IOCON_ANABUS_EXT6 = (1 << 6), /*!< Represents the ana_extbus6 */
    IOCON_ANABUS_EXT7 = (1 << 7), /*!< Represents the ana_extbus7 */
    IOCON_ANABUS_EXT8 = (1 << 8), /*!< Represents the ana_extbus8 */
    IOCON_ANABUS_EXT9 = (1 << 9), /*!< Represents the ana_extbus9 */
    IOCON_ANABUS_EXT10 = (1 << 10), /*!< Represents the ana_extbus10 */
    IOCON_ANABUS_EXT11 = (1 << 11), /*!< Represents the ana_extbus11 */

    IOCON_ANABUS_INT0 = (1 << 12), /*!< Represents the ana_intbus0 */
    IOCON_ANABUS_INT1 = (1 << 13), /*!< Represents the ana_intbus1 */
    IOCON_ANABUS_INT2 = (1 << 14), /*!< Represents the ana_intbus2 */
    IOCON_ANABUS_INT3 = (1 << 15), /*!< Represents the ana_intbus3 */
    IOCON_ANABUS_INT4 = (1 << 16), /*!< Represents the ana_intbus4 */
    IOCON_ANABUS_INT5 = (1 << 17), /*!< Represents the ana_intbus5 */
    IOCON_ANABUS_INT6 = (1 << 18), /*!< Represents the ana_intbus6 */
    IOCON_ANABUS_INT7 = (1 << 19), /*!< Represents the ana_intbus7 */
    IOCON_ANABUS_INT8 = (1 << 20), /*!< Represents the ana_intbus8 */
    IOCON_ANABUS_INT9 = (1 << 21), /*!< Represents the ana_intbus9 */
    IOCON_ANABUS_INT10 = (1 << 22), /*!< Represents the ana_intbus10 */
    IOCON_ANABUS_INT11 = (1 << 23), /*!< Represents the ana_intbus11 */
    IOCON_ANABUS_INT12 = (1 << 24), /*!< Represents the ana_intbus12 */
    IOCON_ANABUS_INT13 = (1 << 25), /*!< Represents the ana_intbus13 */
    IOCON_ANABUS_INT14 = (1 << 26), /*!< Represents the ana_intbus14 */
    IOCON_ANABUS_INT15 = (1 << 27) /*!< Represents the ana_intbus15 */
} IOCON_ANABUS_T;

#define IOCON_FUNC_0 (0x0) /*!< Selects pin function 0 */
#define IOCON_FUNC_1 (0x1) /*!< Selects pin function 1 */
#define IOCON_FUNC_2 (0x2) /*!< Selects pin function 2 */
#define IOCON_FUNC_3 (0x3) /*!< Selects pin function 3 */
#define IOCON_FUNC_4 (0x4) /*!< Selects pin function 4 */
#define IOCON_FUNC_5 (0x5) /*!< Selects pin function 5 */
#define IOCON_FUNC_6 (0x6) /*!< Selects pin function 6 */
#define IOCON_FUNC_7 (0x7) /*!< Selects pin function 7 */
#define IOCON_FUNC_MASK (0x7) /*!< Pin function mask */
#define IOCON_RMODE_INACT (0x0 << 3) /*!< No addition resistor pin function */
#define IOCON_RMODE_PULLDOWN (0x1 << 3) /*!< Selects pull-down resistor function */
#define IOCON_RMODE_PULLUP (0x2 << 3) /*!< Selects pull-up resistor function */
#define IOCON_RMODE_REPEATER (0x3 << 3) /*!< Selects pin repeater function */
#define IOCON_RMODE_MASK (0x3 << 3) /*!< Selects pin resistor mode mask */
#define IOCON_LPF_DISABLE (0x0 << 5) /*!< Disables Low pass filter */
#define IOCON_LPF_ENABLE (0x1 << 5) /*!< Enables Low pass filter */
#define IOCON_LPF_MASK (0x1 << 5) /*!< Selects pin LPF mask */
#define IOCON_CDRIVE_FIXEDVOLTAGE (0x0 << 6) /*!< Programmable current driver - Fixed Voltage driver */
#define IOCON_CDRIVE_PROGRAMMABLECURRENT (0x1 << 6) /*!< Programmable current driver - Programmable current
                                                         driver */
#define IOCON_CDRIVE_MASK (0x1 << 6) /*!< Selects pin current drive mask */
#define IOCON_DDRIVE_HIGH (0x0 << 7) /*!< Digital drive strength - High Drive (20mA drive at
                                                         3.1V supply) */
#define IOCON_DDRIVE_ULTRAHIGH (0x1 << 7) /*!< Digital drive strength - Ultra-high drive (20mA drive
                                                         at 2.4V supply) */
#define IOCON_DDRIVE_MASK (0x1 << 7) /*!< Selects pin digital drive mask */
#define IOCON_I2CMODE_STDFAST (0x0 << 8) /*!< I2C Standard mode or fast-mode */
#define IOCON_I2CMODE_PIO (0x1 << 8) /*!< Standard I/O functionality */
#define IOCON_I2CMODE_MASK (0x1 << 8) /*!< Selects pin I2C mode mask */
#define IOCON_ILO_VAL(x) (((x) & 0xFF) << 8 ) /*!< Set CDRIVE Low Value */
#define IOCON_IHI_VAL(x) (((x) & 0xFF) << 16) /*!< Set CDRIVE High value */
#define IOCON_ILO_MASK (0xFF << 8) /*!< Set CDRIVE Low mask */
#define IOCON_IHI_MASK (0xFF << 16) /*!< Set CDRIVE High mask */

/**
 * Initialize IOCON block
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 */
void Chip_IOCON_Init(NSS_IOCON_T *pIOCON);

/**
 * De-Initialize IOCON block
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 */
void Chip_IOCON_DeInit(NSS_IOCON_T *pIOCON);

/**
 * Sets I/O pin configuration
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 * @param pin : GPIO pin to configure
 * @param config : I/O pin configuration. Use or'ed values defined by IOCON_* macros
 * @note This function ungrounds the respective analog bus, if an analog pin is configured to be 
 *  connected to analog bus. Refer to @ref anaBusGndDesc_anchor "Analog Bus Grounding".
 * @warning Special care must be taken when changing the pin function for the SWD pins. Once one or both of the SWD
 *  pins have been changed to a function other than SWD, it is not possible to start or continue a debug session.
 *  When this is done immediately after booting, it might never be possible to start a debug session with the IC
 *  anymore. See @ref NSS_DEBUG_CONSIDERATIONS for more details.
 */
void Chip_IOCON_SetPinConfig(NSS_IOCON_T *pIOCON, IOCON_PIN_T pin, int config);

/**
 * Gets I/O pin configuration
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 * @param pin : GPIO pin to retrieve the configuration from
 * @return The respective I/O pin configuration. Composed by or'ed values defined by IOCON_* macros, to be analyzed
 *  with #IOCON_FUNC_MASK, #IOCON_RMODE_MASK, #IOCON_LPF_MASK, #IOCON_CDRIVE_MASK, #IOCON_DDRIVE_MASK,
 *  #IOCON_I2CMODE_MASK, #IOCON_ILO_MASK and #IOCON_IHI_MASK
 */
int Chip_IOCON_GetPinConfig(NSS_IOCON_T *pIOCON, IOCON_PIN_T pin);

/**
 * Grounds/UnGrounds the analog bus(es) described in #IOCON_ANABUS_T
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 * @param bitvector : Bitvector of the analog bus(es) which are to be grounded/ungrounded
 * @note This setting overwrites the existing setting for all the analog buses described in #IOCON_ANABUS_T.
 * @note If the respective bit is set, the analog bus will be grounded, otherwise it will be ungrounded.
 * @note This functionality is overlapped with #Chip_IOCON_GroundAnabus and #Chip_IOCON_UngroundAnabus functions.
 */
void Chip_IOCON_SetAnabusGrounded(NSS_IOCON_T *pIOCON, IOCON_ANABUS_T bitvector);

/**
 * Retrieves a bitvector stating the grounded/ungrounded analog buses described in #IOCON_ANABUS_T
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 * @return Bitvector stating the grounded/ungrounded analog buses
 * @note If the respective bit is set, the analog bus is grounded, otherwise it is ungrounded.
 * @note All analog buses are grounded by default.
 */
IOCON_ANABUS_T Chip_IOCON_GetAnabusGrounded(NSS_IOCON_T *pIOCON);

/**
 * Grounds the required analog bus(es).
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 * @param bitvector : Bitvector of the analog bus(es) which needs to be grounded
 * @note All analog buses are grounded by default.
 * @note Only the analog bus(es) set in @c bitvector are changed. All the others are left untouched.
 * @warning Grounding of analog bus has to be taken care by application. Refer to @ref anaBusGndDesc_anchor
 * "Analog Bus Grounding".
 */
void Chip_IOCON_GroundAnabus(NSS_IOCON_T *pIOCON, IOCON_ANABUS_T bitvector);

/**
 * Ungrounds the required analog bus(es).
 * @param pIOCON : The base address of the IOCON peripheral on the chip
 * @param bitvector : Bitvector of the analog bus(es) which needs to be ungrounded
 * @note All analog buses are grounded by default.
 * @note Only the analog bus(es) set in @c bitvector are changed. All the others are left untouched.
 */
void Chip_IOCON_UngroundAnabus(NSS_IOCON_T *pIOCON, IOCON_ANABUS_T bitvector);

#endif /** @} */
