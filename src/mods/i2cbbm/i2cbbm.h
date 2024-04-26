/*
 * Copyright 2017-2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __I2CBBM_H_
#define __I2CBBM_H_

/**
 * @defgroup MODS_NSS_I2CBBM i2cbbm: I2C bit bang master driver
 * @ingroup MODS_NSS
 *
 * @par Introduction
 * The i2cbbm module is used to configure and operate a I2C master through bitbanging.
 * For this 2 SWM pins are used for the clock @c CLK and data @c DAT lines.
 *
 * The Module works as follows:
 * - To drive the CLK and/or DAT line high, the corresponding pin is configured as input. When available, its own
 *  internal pullup is enabled. The pullup - whether it is originating from the CLK or DAT pin or delivered externally
 *  will then ensure a '1' is generated.
 * - To drive the CLK and/or DAT line low, the corresponding pin is configured as output, and a hard '0' is set on that
 *  pin.
 *
 * @par Diversity
 *  This driver supports diversity settings to adapt to different layouts: the CLK and DAT pins can be assigned -
 *  #I2CBBM_CLK_PIN resp. #I2CBBM_DAT_PIN, and additional pins can be put to use as pull-up resistors - #I2CBBM_PULLUPS.
 *  In addition, the clock stretching leniency can be adjusted - #I2CBBM_MAX_CLK_STRETCH, and the clock pulse
 *  duration - #I2CBBM_PULSE_WIDTH. Check @ref MODS_NSS_I2CBBM_DFT for all diversity parameters
 *
 * @note This mod will use at least 2 pins. The specified clock, data, and any
 *  pull-up pins may NOT be altered until the driver deinit is called.
 *
 * @par Usage
 *  To communicate with an I2C slave:
 *  - First check your layout and adapt the diversity settings accordingly.
 *  - Initialize the module: #I2cbbm_Init
 *  - Optionally change the I2C Slave address: #I2cbbm_SetAddress
 *  - In any order or frequency, call #I2cbbm_Write, #I2cbbm_Read and #I2cbbm_WriteRead as required.
 *  - Last, cleanup by calling #I2cbbm_DeInit
 *
 * @par Example code
 *  @snippet i2cbbm_mod_example_1.c i2cbbm_mod_example_1
 *
 * @{
 */

#include "board.h"
#include "i2cbbm_dft.h"

/**
 * This function must be the first function to call in this module after leaving deep power down or power-off power
 * save mode, or after calling #I2cbbm_DeInit.
 * Initializes the I2C bit bang module and resets the default I2C slave address to #I2CBBM_DEFAULT_I2C_ADDRESS.
 * When finished, the module is initialized and the other API calls may be used.
 * @post The pins listed as diversity settings: #I2CBBM_CLK_PIN, #I2CBBM_DAT_PIN and #I2CBBM_PULLUPS
 *  are now reserved for exclusive use by this module.
 */
void I2cbbm_Init(void);

/**
 * Deinitializes the I2C bit bang module.
 * @post The module is deinitialized and the #I2CBBM_CLK_PIN and #I2CBBM_DAT_PIN pins - @b not the pins listed
 *  in #I2CBBM_PULLUPS - are restored to the state they were in when the module was initiated.
 *  All pins are now free to be used by the rest of the application.
 */
void I2cbbm_DeInit(void);

/**
 * Transmits data to the I2C slave.
 * @param pBuf : Pointer to the data to be placed on the I2C bus
 * @param size : Number of bytes to transmit
 * @return When successful, the number of bytes that have been sent are returned.
 *  When the transmission fails, -1 is returned.
 */
int I2cbbm_Write(const uint8_t * pBuf, unsigned int size);

/**
 * Receives data from the I2C slave.
 * @param pBuf : Pointer to the buffer to store the received data.
 * @param size : Number of bytes to receive
 * @return When successful, the number of bytes that have been read are returned.
 *  When the transmission fails, -1 is returned.
 */
int I2cbbm_Read(uint8_t * pBuf, unsigned int size);

/**
 * Transmits to and immediately receives data from the I2C slave. This is done by implementing a repeated start
 * condition, i.e. no stop condition is generated between the write and the subsequent read operation.
 * @param pWriteBuf : Pointer to the data to be placed on the I2C bus.
 * @param writeSize : Number of bytes to receive, may be @c 0.
 * @param pReadBuf : Pointer to the buffer to store the received data.
 * @param readSize : Number of bytes to receive, may be @c 0.
 * @return The number of received bytes or -1 if an error occurred.
 */
int I2cbbm_WriteRead(const uint8_t * pWriteBuf, unsigned int writeSize, uint8_t * pReadBuf, unsigned int readSize);

/**
 * Changes the I2C slave address to communicate to. This takes effect immediately.
 * @param address : The 7-bit I2C address to read/write from/to. The lowest 7 bits are being used, the MSBit is
 *  disregarded.
 */
void I2cbbm_SetAddress(uint8_t address);

#endif /** @} */
