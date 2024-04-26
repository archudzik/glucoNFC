/*
 * Copyright 2014-2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __SSP_NSS_H_
#define __SSP_NSS_H_

/** @defgroup SSP_NSS ssp: Serial Peripheral Interface / Synchronous Serial Port controller
 * @ingroup DRV_NSS
 * This driver provides APIs for the configuration and operation of the SSP hardware block. The SSP interface
 * provides four operating modes:
 *  -# master/slave transmitter mode (polling)
 *  -# master/slave receiver mode (polling)
 *  -# master/slave simultaneous transmitter and receiver mode (polling)
 *  -# master/slave simultaneous transmitter and receiver mode (interrupt)
 * The SSP Driver supports data transfer on SPI, 4-wire SSI or Microwire buses. There can be multiple masters and slaves
 * on the bus, though only one master and slave can communicate with each other at a given point in time. The SSP
 * hardware block supports full-duplex data transfer of frames of length 4 bits to 16 bits. The driver supports both
 * polled or interrupt based operation. Refer @ref SSPPolling_anchor "Polling Mode Usage" for polling mode usage and
 * @ref SSPInterrupt_anchor "Interrupt Mode Usage" for interrupt mode usage @n
 *
 * @par IOCON configuration for SSP:
 *  The SSP-bus pins (#IOCON_PIO0_2, #IOCON_PIO0_6, #IOCON_PIO0_8 and #IOCON_PIO0_9) must be configured for the
 *  SSP (#IOCON_FUNC_1).
 *  See IO Configuration driver @ref IOCON_NSS for details.
 *
 * @anchor SSPClockRates_anchor
 * @par SSP Clock rates:
 *  The maximum allowed SSP clock rate in master or slave mode at a particular system clock can be referred from
 *  @ref NSS_CLOCK_RESTRICTIONS.
 *  SSP bitrate (when operating as a Master) can be set using the API #Chip_SSP_SetBitRate. The resulting SSP bitrate is
 *  determined based on the values of 2 pre-scalers. The API #Chip_SSP_SetBitRate internally calculates and sets the most
 *  appropriate pre-scaler values to achieve the closest bitrate lower or equal to the one that was requested. The first
 *  pre-scaler (P1) divides the SSP clock by an even value between 2 and 254. The second pre-scaler (P2) further divides
 *  the resultant clock by a value between 1 and 256. The resultant SSP clock rate can be obtained by using the API
 *  #Chip_SSP_GetBitRate after setting a certain required bitrate using #Chip_SSP_SetBitRate.
 *
 * @note During driver initialization (#Chip_SSP_Init), there is no implicit check that the system clock/bitrate
 *  combination meets the clock restriction requirements. It is up to the caller to ensure that the respective
 *  restrictions are met.
 * @note During driver initialization (#Chip_SSP_Init), the SSP clock divisor which determines the SSP clock is set to
 *  the same value as the system clock divisor to prevent synchronization issues. The user needs to take care of this in
 *  case the system clock is changed after driver initialization.
 *
 * @par To use this driver:
 *  @anchor SSPInit_anchor
 *  <b> SSP Driver is initialized as follows: </b>
 *      -# SSP pin functions are configured using #Chip_IOCON_SetPinConfig. IOCON driver needs to be initialized before
 *         this using #Chip_IOCON_Init.
 *      -# SSP driver is initialized using #Chip_SSP_Init. The initialization sequence configures the chip to master
 *         mode and sets the default values for frame length (#SSP_BITS_8), frame format (#SSP_FRAME_FORMAT_SPI),
 *         clock mode (#SSP_CLOCK_CPHA0_CPOL0) and bitrate (100kbps).
 *      -# Reconfigure master/slave mode using #Chip_SSP_SetMaster if applicable
 *      -# Reconfigure frame length, frame format and clock mode using #Chip_SSP_SetFormat if applicable
 *      -# Reconfigure bitrate using #Chip_SSP_SetBitRate if applicable
 *
 *  @anchor SSPPolling_anchor
 *  <b> For SSP Master/Slave transmission or reception (polling): </b>
 *      -# Initialize the SSP driver for master/slave mode as given in @ref SSPInit_anchor "SSP Driver Initialisation"
 *      -# Use one of the appropriate transfer API based on the type of transfer required.
 *          - #Chip_SSP_WriteFrames_Blocking for transmission
 *          - #Chip_SSP_ReadFrames_Blocking for reception
 *
 *  <b> For SSP Master/Slave simultaneous transmission and reception mode (polling): </b>
 *      -# Initialize the SSP driver for master/slave mode as given in @ref SSPInit_anchor "SSP Driver Initialisation"
 *      -# Fill in #Chip_SSP_DATA_SETUP_T structure
 *      -# Use #Chip_SSP_RWFrames_Blocking API to complete the transfer
 *
 *  @anchor SSPInterrupt_anchor
 *  <b> For SSP Master/Slave simultaneous transmission and reception mode (using interrupts): </b>
 *      -# Initialize the SSP driver for master/slave mode as given in @ref SSPInit_anchor "SSP Driver Initialisation"
 *      -# Fill in #Chip_SSP_DATA_SETUP_T structure
 *      -# Enable the SSP interrupt using #Chip_SSP_Int_Enable
 *      -# Enable the SSP interrupt in NVIC using #NVIC_EnableIRQ.
 *      -# Now the transmission can be done inside the interrupt handler using #Chip_SSP_Int_RWFrames8Bits or
 *         #Chip_SSP_Int_RWFrames16Bits APIs.
 *         need to be called inside the interrupt handler as well.
 *      -# In application main, wait till transmit FIFO is empty using #Chip_SSP_GetStatus
 *
 * @par Example 1 - Simultaneous master transmission and reception (polling)
 *  - Master mode
 *  - Frame length: #SSP_BITS_8, frame format: #SSP_FRAME_FORMAT_SPI and clock mode: #SSP_CLOCK_MODE0
 *  - Bitrate: 100kbps (User needs to set system clock to 250kHz or more)
 *  - API used for send and receive: #Chip_SSP_RWFrames_Blocking
 *  @note User can select appropriate System and SSP clocks as mentioned at @ref SSPClockRates_anchor "SSP clock rates".
 *
 *  @snippet ssp_nss_example_1.c ssp_nss_example_1
 *
 * @par Example 2 - Simultaneous slave transmission and reception (interrupt)
 *  - Slave mode
 *  - Frame length: #SSP_BITS_8, frame format: #SSP_FRAME_FORMAT_SPI and clock mode: #SSP_CLOCK_MODE0
 *  - Bitrate: 100kbps (User needs to set system clock to 2MHz or more)
 *  - API used for send and receive: #Chip_SSP_Int_RWFrames8Bits
 *
 *  Tx and Rx buffers, setup data structure and transfer completion status flag:
 *  @snippet ssp_nss_example_2.c ssp_nss_example_2_data
 *  Setup and transfer of data:
 *  @snippet ssp_nss_example_2.c ssp_nss_example_2
 *  SSP Interrupt Handler:
 *  @snippet ssp_nss_example_2.c ssp_nss_example_2_irq
 *
 * @{
 */
#include <stdint.h>

/** SSP register block structure */
typedef struct NSS_SSP_S {
    __IO uint32_t CR0; /**< Control Register 0. Selects the serial clock rate, bus type, and data size. */
    __IO uint32_t CR1; /**< Control Register 1. Selects master/slave and other modes. */
    __IO uint32_t DR; /**< Data Register. Writes fill the transmit FIFO, and reads empty the receive FIFO. */
    __I uint32_t SR; /**< Status Register        */
    __IO uint32_t CPSR; /**< Clock Prescale Register */
    __IO uint32_t IMSC; /**< Interrupt Mask Set and Clear Register */
    __I uint32_t RIS; /**< Raw Interrupt Status Register */
    __I uint32_t MIS; /**< Masked Interrupt Status Register */
    __O uint32_t ICR; /**< SSPICR Interrupt Clear Register */
} NSS_SSP_T;

/** Flag Status and Interrupt Flag Status type definition */
typedef enum SSP_INT_STATUS {
    RESET = 0, /**< Reset status */
    SET = !RESET /**< Set status */
} SSP_INT_STATUS_T;

/** Error status type definition */
typedef enum {
    ERROR = 0, /**< Error */
    SUCCESS = !ERROR /**< Success */
} Status;

#define SSP_CR0_DSS(n) ((uint32_t) ((n) & 0xF)) /**< SSP data size select, must be 4 bits to 16 bits */
#define SSP_CR0_FRF_SPI ((uint32_t) (0 << 4)) /**< SSP control 0 Motorola SPI mode */
#define SSP_CR0_FRF_TI ((uint32_t) (1 << 4)) /**< SSP control 0 TI synchronous serial mode */
#define SSP_CR0_FRF_MICROWIRE ((uint32_t) (2 << 4)) /**< SSP control 0 National Micro-wire mode */
#define SSP_CR0_CPOL_LO ((uint32_t) (0)) /**< SPI clock polarity bit Low (used in SPI mode only), maintains the bus clock low between frames */
#define SSP_CR0_CPOL_HI ((uint32_t) (1 << 6)) /**< SPI clock polarity bit High (used in SPI mode only), maintains the bus clock high between frames */
#define SSP_CR0_CPHA_FIRST ((uint32_t) (0)) /**< SPI clock out phase bit FIRST(used in SPI mode only), captures data on the first clock transition of the frame */
#define SSP_CR0_CPHA_SECOND ((uint32_t) (1 << 7)) /**< SPI clock out phase bit SECOND (used in SPI mode only), captures data on the second clock transition of the frame */
#define SSP_CR0_SCR(n) ((uint32_t) ((n & 0xFF) << 8)) /**< SSP serial clock rate value load macro, divider rate is PERIPH_CLK / (cpsr * (SCR + 1)) */
#define SSP_CR0_BITMASK ((uint32_t) (0xFFFF)) /**< SSP CR0 bit mask */

#define SSP_CR1_LBM_EN ((uint32_t) (1 << 0)) /**< SSP control 1 loopback mode enable bit */
#define SSP_CR1_SSP_EN ((uint32_t) (1 << 1)) /**< SSP control 1 enable bit */
#define SSP_CR1_SLAVE_EN ((uint32_t) (1 << 2)) /**< SSP control 1 slave enable */
#define SSP_CR1_MASTER_EN ((uint32_t) (0)) /**< SSP control 1 master enable */
#define SSP_CR1_SO_DISABLE ((uint32_t) (1 << 3)) /**< SSP control 1 slave out disable bit, disables transmit line in slave mode */
#define SSP_CR1_BITMASK ((uint32_t) (0x0F)) /**< SSP CR1 bit mask */

#define SSP_CPSR_BITMASK ((uint32_t) (0xFF)) /**< SSP CPSR bit mask */

#define SSP_DR_BITMASK(n) ((n) & 0xFFFF) /**< SSP data bit mask */

#define SSP_SR_BITMASK ((uint32_t) (0x1F)) /**< SSP SR bit mask */

#define SSP_ICR_BITMASK ((uint32_t) (0x03)) /**< ICR bit mask */

/** SSP Type for Status */
typedef enum SSP_STATUS {
    SSP_STAT_TFE = (uint32_t)(1 << 0), /**< TX FIFO Empty */
    SSP_STAT_TNF = (uint32_t)(1 << 1), /**< TX FIFO not full */
    SSP_STAT_RNE = (uint32_t)(1 << 2), /**< RX FIFO not empty */
    SSP_STAT_RFF = (uint32_t)(1 << 3), /**< RX FIFO full */
    SSP_STAT_BSY = (uint32_t)(1 << 4) /**< SSP Busy */
} SSP_STATUS_T;

/** SSP Type for Interrupt Mask */
typedef enum SSP_INTMASK {
    SSP_RORIM = (uint32_t)(1 << 0), /**< Overrun */
    SSP_RTIM = (uint32_t)(1 << 1), /**< TimeOut */
    SSP_RXIM = (uint32_t)(1 << 2), /**< Rx FIFO is at least half full */
    SSP_TXIM = (uint32_t)(1 << 3), /**< Tx FIFO is at least half empty */
    SSP_INT_MASK_BITMASK = (uint32_t)(0xF) /**< All select Mask */
} SSP_INTMASK_T;

/** SSP Type for Masked Interrupt Status */
typedef enum SSP_MASKINTSTATUS {
    SSP_RORMIS = (uint32_t)(1 << 0), /**< Overrun */
    SSP_RTMIS = (uint32_t)(1 << 1), /**< TimeOut */
    SSP_RXMIS = (uint32_t)(1 << 2), /**< Rx FIFO is at least half full */
    SSP_TXMIS = (uint32_t)(1 << 3), /**< Tx FIFO is at least half empty */
    SSP_MASK_INT_STAT_BITMASK = (uint32_t)(0xF) /**< All select Mask */
} SSP_MASKINTSTATUS_T;

/** SSP Type for Raw Interrupt Status */
typedef enum SSP_RAWINTSTATUS {
    SSP_RORRIS = (uint32_t)(1 << 0), /**< Overrun */
    SSP_RTRIS = (uint32_t)(1 << 1), /**< TimeOut */
    SSP_RXRIS = (uint32_t)(1 << 2), /**< Rx FIFO is at least half full */
    SSP_TXRIS = (uint32_t)(1 << 3), /**< Tx FIFO is at least half empty */
    SSP_RAW_INT_STAT_BITMASK = (uint32_t)(0xF) /**< All select Mask */
} SSP_RAWINTSTATUS_T;

/** SSP Interrupt clear masks */
typedef enum SSP_INTCLEAR {
    SSP_RORIC = 0x0, /**< Overrun Mask */
    SSP_RTIC = 0x1, /**< TimeOut Mask */
    SSP_INT_CLEAR_BITMASK = 0x3 /**< Clear all Mask */
} SSP_INTCLEAR_T;

/** SSP clock format */
typedef enum CHIP_SSP_CLOCK_FORMAT {
    SSP_CLOCK_CPHA0_CPOL0 = (0 << 6), /**< CPHA = 0, CPOL = 0 */
    SSP_CLOCK_CPHA0_CPOL1 = (1u << 6), /**< CPHA = 0, CPOL = 1 */
    SSP_CLOCK_CPHA1_CPOL0 = (2u << 6), /**< CPHA = 1, CPOL = 0 */
    SSP_CLOCK_CPHA1_CPOL1 = (3u << 6), /**< CPHA = 1, CPOL = 1 */
    SSP_CLOCK_MODE0 = SSP_CLOCK_CPHA0_CPOL0, /**< alias for #SSP_CLOCK_CPHA0_CPOL0 */
    SSP_CLOCK_MODE1 = SSP_CLOCK_CPHA1_CPOL0, /**< alias for #SSP_CLOCK_CPHA1_CPOL0 */
    SSP_CLOCK_MODE2 = SSP_CLOCK_CPHA0_CPOL1, /**< alias for #SSP_CLOCK_CPHA0_CPOL1 */
    SSP_CLOCK_MODE3 = SSP_CLOCK_CPHA1_CPOL1 /**< alias for #SSP_CLOCK_CPHA1_CPOL1 */
} CHIP_SSP_CLOCK_MODE_T;

/** SSP frame format */
typedef enum CHIP_SSP_FRAME_FORMAT {
    SSP_FRAME_FORMAT_SPI = (0 << 4), /**< Frame format: SPI */
    SSP_FRAME_FORMAT_TI = (1u << 4), /**< Frame format: TI SSI */
    SSP_FRAME_FORMAT_MICROWIRE = (2u << 4) /**< Frame format: Microwire */
} CHIP_SSP_FRAME_FORMAT_T;

/** Number of bits per frame */
typedef enum CHIP_SSP_BITS {
    SSP_BITS_4 = 3, /**< 4 bits/frame */
    SSP_BITS_5 = 4, /**< 5 bits/frame */
    SSP_BITS_6 = 5, /**< 6 bits/frame */
    SSP_BITS_7 = 6, /**< 7 bits/frame */
    SSP_BITS_8 = 7, /**< 8 bits/frame */
    SSP_BITS_9 = 8, /**< 9 bits/frame */
    SSP_BITS_10 = 9, /**< 10 bits/frame */
    SSP_BITS_11 = 10, /**< 11 bits/frame */
    SSP_BITS_12 = 11, /**< 12 bits/frame */
    SSP_BITS_13 = 12, /**< 13 bits/frame */
    SSP_BITS_14 = 13, /**< 14 bits/frame */
    SSP_BITS_15 = 14, /**< 15 bits/frame */
    SSP_BITS_16 = 15 /**< 16 bits/frame */
} CHIP_SSP_BITS_T;

/** SSP mode */
typedef enum CHIP_SSP_MODE {
    SSP_MODE_MASTER = (0 << 2), /**< Master mode */
    SSP_MODE_SLAVE = (1u << 2), /**< Slave mode */
} CHIP_SSP_MODE_T;

/* ------------------------------------------------------------------------- */

/**
 * Enable SSP operation
 * @param pSSP : The base address of the SSP peripheral on the chip
 */
static inline void Chip_SSP_Enable(NSS_SSP_T *pSSP)
{
    pSSP->CR1 |= SSP_CR1_SSP_EN;
}

/**
 * Disable SSP operation
 * @param pSSP : The base address of the SSP peripheral on the chip
 */
static inline void Chip_SSP_Disable(NSS_SSP_T *pSSP)
{
    pSSP->CR1 &= (~SSP_CR1_SSP_EN) & SSP_CR1_BITMASK;
}

/**
 * Enable loopback mode
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @note In loopback mode, serial input is taken from the serial output (MOSI or MISO) rather than the serial input pin.
 */
static inline void Chip_SSP_EnableLoopBack(NSS_SSP_T *pSSP)
{
    pSSP->CR1 |= SSP_CR1_LBM_EN;
}

/**
 * Disable loopback mode
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @note In loopback mode, serial input is taken from the serial output (MOSI or MISO) rather than the serial input pin.
 */
static inline void Chip_SSP_DisableLoopBack(NSS_SSP_T *pSSP)
{
    pSSP->CR1 &= (~SSP_CR1_LBM_EN) & SSP_CR1_BITMASK;
}

/**
 * Get the current status of SSP controller
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param Stat : Type of status, as given below :
 *  - #SSP_STAT_TFE
 *  - #SSP_STAT_TNF
 *  - #SSP_STAT_RNE
 *  - #SSP_STAT_RFF
 *  - #SSP_STAT_BSY
 * @return SSP controller status, SET or RESET
 */
static inline SSP_INT_STATUS_T Chip_SSP_GetStatus(NSS_SSP_T *pSSP, SSP_STATUS_T Stat)
{
    return (pSSP->SR & Stat) ? SET : RESET;
}

/**
 * Get the masked interrupt status
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @return SSP Masked Interrupt Status Register value
 * @note The return value contains a 1 for each interrupt condition that is asserted and enabled (masked)
 */
static inline uint32_t Chip_SSP_GetIntStatus(NSS_SSP_T *pSSP)
{
    return pSSP->MIS;
}

/**
 * Get the raw interrupt status
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param RawInt : Interrupt condition to get status for, as given below :
 *  - #SSP_RORRIS
 *  - #SSP_RTRIS
 *  - #SSP_RXRIS
 *  - #SSP_TXRIS
 * @return Raw interrupt status corresponding to the requested interrupt condition, SET or RESET
 * @note Get the status of each interrupt condition, regardless of whether or not the interrupt is enabled
 */
static inline SSP_INT_STATUS_T Chip_SSP_GetRawIntStatus(NSS_SSP_T *pSSP, SSP_RAWINTSTATUS_T RawInt)
{
    return (pSSP->RIS & RawInt) ? SET : RESET;
}

/**
 * Get the number of bits transferred in each frame
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @return the number of bits transferred in each frame minus one
 * @note The return value is 0x03 -> 0xF corresponding to 4bit -> 16bit transfer
 */
static inline uint8_t Chip_SSP_GetDataSize(NSS_SSP_T *pSSP)
{
    return SSP_CR0_DSS(pSSP->CR0);
}

/**
 * Clear the corresponding interrupt condition(s) in the SSP controller
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param IntClear : Type of cleared interrupt, as given below :
 *  - #SSP_RORIC
 *  - #SSP_RTIC
 * @note Software can clear one or more interrupt condition(s) in the SSP controller
 */
static inline void Chip_SSP_ClearIntPending(NSS_SSP_T *pSSP, SSP_INTCLEAR_T IntClear)
{
    pSSP->ICR = IntClear;
}

/**
 * Enable interrupt for the SSP
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @note The implementation supports the interrupt type "Tx FIFO is at least half empty" only
 */
static inline void Chip_SSP_Int_Enable(NSS_SSP_T *pSSP)
{
    pSSP->IMSC |= SSP_TXIM;
}

/**
 * Disable interrupt for the SSP
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @note The implementation supports the interrupt type "Tx FIFO is at least half empty" only
 */
static inline void Chip_SSP_Int_Disable(NSS_SSP_T *pSSP)
{
    pSSP->IMSC &= (uint32_t)(~SSP_TXIM);
}

/**
 * Get received 16-bit SSP data
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @return SSP 16-bit data received
 */
static inline uint16_t Chip_SSP_ReceiveFrame(NSS_SSP_T *pSSP)
{
    return (uint16_t)(SSP_DR_BITMASK(pSSP->DR));
}

/**
 * Send SSP 16-bit data
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param tx_data : SSP 16-bit data to be transmitted
 */
static inline void Chip_SSP_SendFrame(NSS_SSP_T *pSSP, uint16_t tx_data)
{
    pSSP->DR = SSP_DR_BITMASK(tx_data);
}

/**
 * Set up output clocks per bit for SSP bus
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param clk_rate : The number of prescaler-output clocks per bit on the bus, minus one
 * @param prescale : The factor by which the Prescaler divides the SSP peripheral clock PCLK
 * @note The bit frequency is PCLK / (prescale x[clk_rate+1])
 */
void Chip_SSP_SetClockRate(NSS_SSP_T *pSSP, uint32_t clk_rate, uint32_t prescale);

/**
 * Set up the SSP frame format
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param bits : The number of bits transferred in each frame, should be between SSP_BITS_4 and SSP_BITS_16
 * @param frameFormat : Frame format, as given below :
 *  - #SSP_FRAME_FORMAT_SPI
 *  - #SSP_FRAME_FORMAT_TI
 *  - #SSP_FRAME_FORMAT_MICROWIRE
 * @param clockMode : Select Clock polarity and Clock phase, as given below :
 *  - #SSP_CLOCK_CPHA0_CPOL0
 *  - #SSP_CLOCK_CPHA0_CPOL1
 *  - #SSP_CLOCK_CPHA1_CPOL0
 *  - #SSP_CLOCK_CPHA1_CPOL1
 * @note The clockFormat is only used in SPI mode
 */
static inline void Chip_SSP_SetFormat(NSS_SSP_T *pSSP, uint32_t bits, uint32_t frameFormat, uint32_t clockMode)
{
    pSSP->CR0 = (pSSP->CR0 & ~0xFFu) | bits | frameFormat | clockMode;
}

/**
 * Set the SSP working as master or slave mode
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param mode : Operating mode, as given below:
 *  - #SSP_MODE_MASTER
 *  - #SSP_MODE_SLAVE
 */
static inline void Chip_SSP_Set_Mode(NSS_SSP_T *pSSP, uint32_t mode)
{
    pSSP->CR1 = (pSSP->CR1 & ~(1u << 2)) | mode;
}

/* ------------------------------------------------------------------------- */

/** SSP data setup structure */
typedef struct {
    void *tx_data; /**< Pointer to transmit data */
    uint32_t tx_cnt; /**< Transmit counter */
    void *rx_data; /**< Pointer to transmit data */
    uint32_t rx_cnt; /**< Receive counter */
    uint32_t length; /**< Length of transfer data */
} Chip_SSP_DATA_SETUP_T;

/* ------------------------------------------------------------------------- */

/**
 * Clean all data in RX FIFO of SSP
 * @param pSSP : The base address of the SSP peripheral on the chip
 */
void Chip_SSP_Int_FlushData(NSS_SSP_T *pSSP);

/**
 * SSP Interrupt Read/Write with 8-bit frame width
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param xf_setup : Pointer to a SSP_DATA_SETUP_T structure that contains specified information about transmit/receive
 *                   data configuration
 * @return SUCCESS or ERROR
 */
Status Chip_SSP_Int_RWFrames8Bits(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);

/**
 * SSP Interrupt Read/Write with 16-bit frame width
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param xf_setup : Pointer to a SSP_DATA_SETUP_T structure that contains specified information about transmit/receive
 *                   data configuration
 * @return SUCCESS or ERROR
 */
Status Chip_SSP_Int_RWFrames16Bits(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);

/**
 * SSP Polling Read/Write in blocking mode
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param xf_setup : Pointer to a SSP_DATA_SETUP_T structure that contains specified information about transmit/receive
 *                   data configuration
 * @return Actual data transfer length
 * @note This function can be used in both master and slave mode. It starts with a writing phase and after that,
 *  a reading phase is generated to read any data available in RX_FIFO. All needed information is prepared
 *  through xf_setup param.
 */
uint32_t Chip_SSP_RWFrames_Blocking(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);

/**
 * SSP Polling Write in blocking mode
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param buffer : Buffer address
 * @param buffer_len : Buffer length
 * @return Actual data transfer length
 * @note This function can be used in both master and slave mode. First, a writing operation will send
 *  the needed data. After that, a dummy reading operation is generated to clear data buffer
 */
uint32_t Chip_SSP_WriteFrames_Blocking(NSS_SSP_T *pSSP, uint8_t *buffer, uint32_t buffer_len);

/**
 * SSP Polling Read in blocking mode
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param buffer : Buffer address
 * @param buffer_len : The length of buffer
 * @return Actual data transfer length
 * @note This function can be used in both master and slave mode. First, a dummy writing operation is generated
 *  to clear data buffer. After that, a reading operation will receive the needed data
 */
uint32_t Chip_SSP_ReadFrames_Blocking(NSS_SSP_T *pSSP, uint8_t *buffer, uint32_t buffer_len);

/**
 * Initialize the SSP
 * @param pSSP : The base address of the SSP peripheral on the chip
 */
void Chip_SSP_Init(NSS_SSP_T *pSSP);

/**
 * De-initialise the SSP
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @note The SSP controller is disabled
 */
void Chip_SSP_DeInit(NSS_SSP_T *pSSP);

/**
 * Set the SSP operating modes, master or slave
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param master : 1 to set master, 0 to set slave
 */
void Chip_SSP_SetMaster(NSS_SSP_T *pSSP, bool master);

/**
 * Set the clock frequency for SSP interface
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @param bitRate : The SSP bit rate in bits per seconds
 * @note Please refer to @ref SSPClockRates_anchor "SSP Clock rates" for more information on SSP clock and bitrate
 */
void Chip_SSP_SetBitRate(NSS_SSP_T *pSSP, uint32_t bitRate);

/**
 * Get the clock frequency for SSP interface
 * @param pSSP : The base address of the SSP peripheral on the chip
 * @return The SSP bit rate in bits per second
 * @note Please refer to @ref SSPClockRates_anchor "SSP Clock rates" for more information on SSP clock and bitrate
 */
uint32_t Chip_SSP_GetBitRate(NSS_SSP_T *pSSP);

#endif /** @} */
