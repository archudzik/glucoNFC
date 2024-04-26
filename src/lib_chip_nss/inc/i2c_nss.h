/*
 * Copyright 2014-2016,2018-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __I2C_NSS_H_
#define __I2C_NSS_H_

/**
 * @defgroup I2C_NSS i2c: I2C driver
 * @ingroup DRV_NSS
 * This driver provides APIs for the configuration and operation of the I2C hardware block. The I2C interface
 * provides four operating modes:
 *  -# master transmitter mode
 *  -# master receiver mode
 *  -# slave transmitter mode
 *  -# slave receiver mode
 * The I2C-bus interface is byte oriented. This driver supports both polled or interrupt based operation. @n
 * This I2C block is capable of addressing four slaves.
 *
 * @par IOCON configuration for I2C:
 *  The I2C-bus pins (#IOCON_PIO0_4 and #IOCON_PIO0_5) must be configured for the I2C Standard/ Fast-mode (#IOCON_FUNC_1).
 *  See IO Configuration driver @ref IOCON_NSS for details.
 *
 * @anchor I2CClockRates_anchor
 * @par I2C Clock rates:
 *  Details on clock rates that can be used for I2C communication can be referred from @ref NSS_CLOCK_RESTRICTIONS.
 * @note During driver initialization (#Chip_I2C_Init), there is no implicit check that the system clock/bitrate
 *  combination meets the clock restriction requirements. It is up to the caller to ensure that the respective
 *  restrictions are met.
 * @note The I2C clock is derived from system clock. Changing the system clock frequency affects the I2C
 *  bitrate, hence it must be re-set using #Chip_I2C_SetClockRate to ensure the required rate is set.
 *
 * @par I2C Master Event Handling:
 *  The I2C master supports two event handling mechanisms which can be configured using #Chip_I2C_SetMasterEventHandler:
 *   -# Interrupt based handling makes use of #Chip_I2C_EventHandler handler.
 *   -# Polling based handling makes use of #Chip_I2C_EventHandlerPolling handler.
 *  The user can also implement an own handler using #Chip_I2C_SetMasterEventHandler to perform specific actions for
 *   specific events (#I2C_EVENT_T).
 *
 * @par To use this driver:
 *  <b> I2C Driver is initialized as follows: </b>
 *      -# I2C pin functions and the I2C mode are configured using #Chip_IOCON_SetPinConfig.
 *      -# I2C block reset signal is de-asserted using #Chip_SysCon_Peripheral_DeassertReset.
 *      -# I2C driver is initialized using #Chip_I2C_Init.
 *      -# I2C clock rate is set using #Chip_I2C_SetClockRate.
 *  <b> For I2C Master transfers: </b>
 *      -# Set the event handling mechanism using #Chip_I2C_SetMasterEventHandler for the master.
 *      -# Enable the I2C interrupt in NVIC using #NVIC_EnableIRQ for interrupt based event handling.
 *      -# Fill in #I2C_XFER_T structure if #Chip_I2C_MasterTransfer API is used for I2C master transfer.
 *      -# Use one of the appropriate Master transfer API based on the type of transfer required.
 *          - #Chip_I2C_MasterTransfer
 *          - #Chip_I2C_MasterSend
 *          - #Chip_I2C_MasterRead
 *          - #Chip_I2C_MasterCmdRead
 *  <b> For I2C Slave transfers: </b>
 *      -# Fill in #I2C_XFER_T structure for the slave transfer.
 *      -# Use the #Chip_I2C_SlaveSetup to setup the I2C slave.
 *      -# Enable the I2C interrupt in NVIC using #NVIC_EnableIRQ.
 *
 * @par Example 1 - Configuring I2C as Master and use transfer (send/receive) API
 *  - Master mode
 *  - System clock: 4000000 and I2C clock: 100000
 *  - Target SlaveAddr: 0x7f
 *  - Operating mode: Interrupt based
 *  - API used for send and receive: #Chip_I2C_MasterTransfer
 *  @note User can select appropriate System and I2C clocks as mentioned at @ref I2CClockRates_anchor "I2C clock rates".
 *
 *  Tx and Rx data structures:
 *  @snippet i2c_nss_example_1.c i2c_nss_example_1_data
 *  Setup and transfer of data:
 *  @snippet i2c_nss_example_1.c i2c_nss_example_1
 *
 * @par I2C IRQ Handler
 *  @snippet i2c_nss_example_1.c i2c_nss_irq_handler
 *  @note This IRQ Handler is common for Example 1 and Example 2.
 *
 * @par Example 2 - Configuring I2C as Slave and transfer data
 *  - Slave mode
 *  - System clock: 4000000 and I2C clock: 100000
 *  - SlaveAddr: 0x7f
 *  - Operating mode: Interrupt based
 *  - API used for slave setup: #Chip_I2C_SlaveSetup
 *  - Slave event handler: Example2_I2cSlaveEventHandler
 *  @note User can select appropriate System and I2C clocks as mentioned at @ref I2CClockRates_anchor "I2C clock rates".
 *
 *  Tx, Rx and I2C transfer data structures:
 *  @snippet i2c_nss_example_2.c i2c_nss_example_2_data
 *  Slave event handler:
 *  @snippet i2c_nss_example_2.c i2c_nss_example_2_eventhandler
 *  Setup and transfer of data:
 *  @snippet i2c_nss_example_2.c i2c_nss_example_2
 *
 * @{
 */

/** I2C register block structure. */
typedef struct {
    __IO uint32_t CONSET; /*!< I2C Control Set Register. When a one is written to a bit of this register, the
     corresponding bit in the I2C control register is set. Writing a zero has no effect on the corresponding bit in the
     I2C control register. */
    __I uint32_t STAT; /*!< I2C Status Register. During I2C operation, this register provides detailed status codes
     that allow software to determine the next action needed. */
    __IO uint32_t DAT; /*!< I2C Data Register. During master or slave transmit mode, data to be transmitted is written
     to this register. During master or slave receive mode, data that has been received can be read from this register. */
    __IO uint32_t ADR0; /*!< I2C Slave Address Register 0. Contains the 7-bit slave address for operation of the I2C
     interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave
     responds to the General Call address. */
    __IO uint32_t SCLH; /*!< SCH Duty Cycle Register High Half Word. Determines the high time of the I2C clock. */
    __IO uint32_t SCLL; /*!< SCL Duty Cycle Register Low Half Word. Determines the low time of the I2C clock. SCLL and
     SCLH together determine the clock frequency generated by an I2C master and certain times used in slave mode. */
    __O uint32_t CONCLR; /*!< I2C Control Clear Register. When a one is written to a bit of this register, the
     corresponding bit in the I2C control register is cleared. Writing a zero has no effect on the corresponding bit
     in the I2C control register. */
    __IO uint32_t MMCTRL; /*!< Monitor mode control register. */
    __IO uint32_t ADR1; /*!< I2C Slave Address Register. Contains the 7-bit slave address for operation of the I2C
     interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave
     responds to the General Call address. */
    __IO uint32_t ADR2; /*!< I2C Slave Address Register. Contains the 7-bit slave address for operation of the I2C
     interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave
     responds to the General Call address. */
    __IO uint32_t ADR3; /*!< I2C Slave Address Register. Contains the 7-bit slave address for operation of the I2C
     interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave
     responds to the General Call address. */
    __I uint32_t DATA_BUFFER; /*!< Data buffer register. The contents of the 8 MSBs of the DAT shift register will
     be transferred to the DATA_BUFFER automatically after every nine bits (8 bits of data plus ACK or NACK) has
     been received on the bus. */
    __IO uint32_t MASK[4]; /*!< I2C Slave address mask register */
} NSS_I2C_T;

/** I2C Slave Identifiers. */
typedef enum {
    I2C_SLAVE_GENERAL, /*!< Slave ID for general calls */
    I2C_SLAVE_0, /*!< Slave ID for Slave Address 0 */
    I2C_SLAVE_1, /*!< Slave ID for Slave Address 1 */
    I2C_SLAVE_2, /*!< Slave ID for Slave Address 2 */
    I2C_SLAVE_3, /*!< Slave ID for Slave Address 3 */
    I2C_SLAVE_NUM_INTERFACE /*!< Number of slave interfaces. Not to be used as a slave ID. Use this for loops or
     to define array sizes. */
} I2C_SLAVE_ID;

/** I2C transfer status. */
typedef enum {
    I2C_STATUS_DONE, /*!< Transfer done successfully, used for Master and Slave transfers */
    I2C_STATUS_NAK, /*!< NAK received during transfer, used for Master transfer */
    I2C_STATUS_ARBLOST, /*!< Arbitration lost during transfer, used for Master transfer */
    I2C_STATUS_BUSERR, /*!< Bus error in I2C transfer, used for Master transfer */
    I2C_STATUS_BUSY, /*!< I2C is busy doing transfer, used for Master and Slave transfers */
} I2C_STATUS_T;

/** Transfer data structure definition.
 *  For usage in master transfer, refer to #Chip_I2C_MasterTransfer.
 *  For usage in slave transfer, refer to #Chip_I2C_SlaveSetup.
 */
typedef struct {
    uint8_t slaveAddr; /*!< 7-bit I2C Slave address */
    const uint8_t *txBuff; /*!< Pointer to array of bytes to be transmitted */
    int txSz; /*!< Positive value indicating number of bytes in transmit array, if 0 only reception will be carried on */
    uint8_t *rxBuff; /*!< Pointer to memory where bytes received from I2C will be stored */
    int rxSz; /*!< Positive value indicating number of bytes to be received, if 0 only transmission will be carried on */
    I2C_STATUS_T status; /*!< Status of the current I2C transfer, this is read only for user */
} I2C_XFER_T;

/** I2C interface IDs. */
typedef enum I2C_ID {
    I2C0, /*!< ID I2C0, all Chip functions will take this as the first parameter */
    I2C_NUM_INTERFACE /*!< Number of I2C interfaces in the chip, must never be used for calling any Chip functions */
} I2C_ID_T;

/** I2C events.
 *  Provides information on the type of event to the event handler functions (#I2C_EVENTHANDLER_T).
 */
typedef enum {
    I2C_EVENT_WAIT = 1, /*!< I2C Wait event, triggered when waiting for the status (#I2C_STATUS_T) to update */
    I2C_EVENT_DONE, /*!< Done event that wakes up Wait event, triggered when master transactions for slave are completed */
    I2C_EVENT_LOCK, /*!< Re-entrancy lock event for I2C transfer, triggered at the beginning of a Master transfer */
    I2C_EVENT_UNLOCK, /*!< Re-entrancy unlock event for I2C transfer, triggered on completion of a Master transfer */
    I2C_EVENT_SLAVE_RX, /*!< Slave receive event, triggered on reception of data in Slave */
    I2C_EVENT_SLAVE_TX, /*!< Slave transmit event, triggered after data transmitted from Slave */
} I2C_EVENT_T;

/** Event handler function type.
 *  Use this prototype if you want to define your own event handler using #Chip_I2C_SetMasterEventHandler.
 */
typedef void (*I2C_EVENTHANDLER_T)(I2C_ID_T, I2C_EVENT_T);

/**
 * Initializes the NSS_I2C peripheral with specified parameter.
 * @param id : I2C peripheral ID (#I2C0)
 */
void Chip_I2C_Init(I2C_ID_T id);

/**
 * De-initializes the I2C peripheral registers to their default reset values
 * @param id : I2C peripheral ID (#I2C0)
 */
void Chip_I2C_DeInit(I2C_ID_T id);

/**
 * Set up clock rate for NSS_I2C peripheral.
 * @param id : I2C peripheral ID (#I2C0)
 * @param clockrate : Target clock rate value to initialize I2C peripheral (Hz)
 * @note Parameter @a clockrate for #I2C0 must be from 1000 up to 400000 (1 KHz to 400 KHz), as #I2C0 supports Fast Mode.
 * @note Refer to @ref I2CClockRates_anchor "I2C clock rates" for I2C clock restrictions.
 */
void Chip_I2C_SetClockRate(I2C_ID_T id, uint32_t clockrate);

/**
 * Get current clock rate for NSS_I2C peripheral.
 * @param id : I2C peripheral ID (#I2C0)
 * @return The current I2C peripheral clock rate
 */
uint32_t Chip_I2C_GetClockRate(I2C_ID_T id);

/**
 * Transmit and Receive data in master mode
 * @param id : I2C peripheral selected (#I2C0)
 * @param xfer : Pointer to a #I2C_XFER_T structure. Details of structure members is captured below:
 *  - slaveAddr : 7-Bit slave address to which the master will do the transfer, bit0 to bit6 should have the address
 *   and bit7 is ignored.
 *  - txBuff : pointer to the memory from which to pick the data to be transfered to slave
 *  - txSz : number of bytes to send
 *  - rxBuff : pointer to the memory where data received from slave is to be stored
 *  - rxSz : number of bytes to get from slave
 *  Refer to #I2C_XFER_T for additional information.
 * @return Any of #I2C_STATUS_T values, #I2C_XFER_T.txSz will have number of bytes not sent due to error,
 *  #I2C_XFER_T.rxSz will have the number of bytes yet to be received.
 * @note During the transfer, program execution (like event handler) must not change the content of the memory pointed
 *  to by @a xfer.
 * @note Alternate implementations for Master transfers can be referred from #Chip_I2C_MasterSend and #Chip_I2C_MasterRead.
 */
I2C_STATUS_T Chip_I2C_MasterTransfer(I2C_ID_T id, I2C_XFER_T *xfer);

/**
 * Transmit data to I2C slave using I2C Master mode
 * @param id : I2C peripheral ID (#I2C0)
 * @param slaveAddr : Slave address to which the data be written
 * @param buff : Pointer to buffer having the array of data
 * @param len : Number of bytes to be transfered from @a buff
 * @return Number of bytes successfully transfered
 */
int Chip_I2C_MasterSend(I2C_ID_T id, uint8_t slaveAddr, const uint8_t *buff, int len);

/**
 * Transfer a command to slave and receive data from slave after a repeated start
 * @param id : I2C peripheral ID (#I2C0)
 * @param slaveAddr : Slave address of the I2C device
 * @param cmdBuff : Pointer to command buffer of size one byte
 * @param buff : Pointer to memory that will hold the data received
 * @param len : Number of bytes to receive
 * @return Number of bytes successfully received
 * @note This is a specific implementation of Master transfer where a specific command can be send by the Master to
 *  invoke a pre-defined behavior from the Slave.
 */
int Chip_I2C_MasterCmdRead(I2C_ID_T id, uint8_t slaveAddr, uint8_t *cmdBuff, uint8_t *buff, int len);

/**
 * Get pointer to current function handling the events
 * @param id : I2C peripheral ID (#I2C0)
 * @return Pointer to function handing events of I2C
 * @note The user can also implement an own handler using #Chip_I2C_SetMasterEventHandler API.
 */
I2C_EVENTHANDLER_T Chip_I2C_GetMasterEventHandler(I2C_ID_T id);

/**
 * Set function that must handle I2C events
 * @param id : I2C peripheral ID (#I2C0)
 * @param event : Pointer to function that will handle the event, #I2C_EVENTHANDLER_T
 * @return 1 when successful, 0 when a transfer is on going with its own event handler
 * @note @a event should not be NULL
 * @note Live swap of event handlers is not tested.
 */
int Chip_I2C_SetMasterEventHandler(I2C_ID_T id, I2C_EVENTHANDLER_T event);

/**
 * Receive data from I2C slave using I2C Master mode
 * @param id : I2C peripheral ID (#I2C0)
 * @param slaveAddr : Slave address from which data be read
 * @param buff : Pointer to memory where data read be stored
 * @param len : Number of bytes to read from slave
 * @return Number of bytes read successfully
 */
int Chip_I2C_MasterRead(I2C_ID_T id, uint8_t slaveAddr, uint8_t *buff, int len);

/**
 * Default event handler for polling operation
 * @param id : I2C peripheral ID (#I2C0)
 * @param event : Event ID of the event that called the function
 * @note This is the default handler for polling mode where only the #I2C_EVENT_WAIT state is handled.
 */
void Chip_I2C_EventHandlerPolling(I2C_ID_T id, I2C_EVENT_T event);

/**
 * Default event handler for interrupt based operation
 * @param id : I2C peripheral ID (#I2C0)
 * @param event : Event ID of the event that called the function
 * @note This is the default handler for interrupt mode where only the #I2C_EVENT_WAIT state is handled.
 */
void Chip_I2C_EventHandler(I2C_ID_T id, I2C_EVENT_T event);

/**
 * I2C Master transfer state change handler
 * @param id : I2C peripheral ID (#I2C0)
 * @note In case of interrupt based operation, this function is to be invoked from the interrupt handler.
 *  For the polling based operation, this function is implicitly invoked from #Chip_I2C_EventHandlerPolling.
 */
void Chip_I2C_MasterStateHandler(I2C_ID_T id);

/**
 * Disable I2C peripheral's operation
 * @param id : I2C peripheral ID (#I2C0)
 */
void Chip_I2C_Disable(I2C_ID_T id);

/**
 * Checks if master transfer in progress
 * @param id : I2C peripheral ID (#I2C0)
 * @return 1 if master transfer in progress, 0 for slave transfer
 * @note This API is generally used in interrupt handler of the application to decide whether to call
 *  master state handler or to call slave state handler
 */
int Chip_I2C_IsMasterActive(I2C_ID_T id);

/**
 * Setup a slave I2C device
 * @param id : I2C peripheral ID (#I2C0)
 * @param sid : I2C Slave peripheral ID (#I2C_SLAVE_ID)
 * @param xfer : Pointer to transfer structure (#I2C_XFER_T). Details of structure members is captured below:
 *  - slaveAddr : 7 bit Slave address (from bit1 to bit7), bit0 when set enables general call handling. This along with
 *   @a addrMask will be used to match the slave address.
 *  - txBuff : pointer to valid buffers where slave can send data from
 *  - txSz : size of txBuff
 *  - rxBuff : pointer to valid buffers where slave can receive data from
 *  - rxSz : size of rxBuff
 * @param event : Event handler for slave transfers (#I2C_EVENTHANDLER_T)
 * @param addrMask : Address mask to use along with slave address, see notes below for more info
 * @note Parameter @a xfer should point to a valid #I2C_XFER_T structure object. @n
 *  Function pointed to by @a event will be called for the following events:
 *   - #I2C_EVENT_SLAVE_RX : One byte of data received successfully from the master and stored inside memory pointed
 *    by #I2C_XFER_T.rxBuff, incremented the pointer and decremented the #I2C_XFER_T.rxSz
 *   - #I2C_EVENT_SLAVE_TX : One byte of data from #I2C_XFER_T.txBuff was sent to master successfully, incremented the
 *    pointer and decremented #I2C_XFER_T.txSz
 *   - #I2C_EVENT_DONE : Master is done doing its transfers with the slave
 *  Bit-0 of the parameter @a addrMask is reserved and should always be 0. Any bit (BIT1 to BIT7) set in @a addrMask
 *  will make the corresponding bit in #I2C_XFER_T.slaveAddr as don't care, that is, if #I2C_XFER_T.slaveAddr is
 *  (0x10 << 1) and @a addrMask is (0x03 << 1) then 0x10, 0x11, 0x12, 0x13 will all be considered as valid slave
 *  addresses for the registered slave. Upon receiving any event #I2C_XFER_T.slaveAddr (BIT1 to BIT7) will hold the
 *  actual address which was received from master. @n
 *  @n <b>General Call Handling</b> @n
 *  Slave can receive data from master using general call address (0x00). General call handling must be setup as given below
 *   - Call Chip_I2C_SlaveSetup() with argument @a sid as #I2C_SLAVE_GENERAL
 *       - #I2C_XFER_T.slaveAddr ignored, argument @a addrMask ignored
 *       - function provided by @a event will registered to be called when slave received data using addr 0x00
 *       - #I2C_XFER_T.rxBuff and #I2C_XFER_T.rxSz should be valid in argument @a xfer
 *   - To handle General Call only (No other slaves are configured)
 *       - Call Chip_I2C_SlaveSetup() with sid as I2C_SLAVE_X (X=0,1,2,3)
 *       - setup @a xfer with slaveAddr member set to 0, @a event is ignored hence can be NULL
 *       - provide @a addrMask (typically 0, if not you better be knowing what you are doing)
 *   - To handle General Call when other slave is active
 *       - Call Chip_I2C_SlaveSetup() with sid as I2C_SLAVE_X (X=0,1,2,3)
 *       - setup @a xfer with slaveAddr member set to 7-Bit Slave address [from Bit1 to 7]
 *       - Set Bit0 of @a xfer->slaveAddr as 1
 *       - Provide appropriate @a addrMask
 *       - Argument @a event must point to function, that handles events from actual slaveAddress and not the GC
 * @warning If the slave has only one byte left in its #I2C_XFER_T.txBuff, once that byte is transfered to master the
 *  event handler will be called for event #I2C_EVENT_DONE. If the master attempts to read more bytes in the same transfer
 *  then the slave hardware will send 0xFF to master till the end of transfer, event handler will not be
 *  called to notify this. For more info see section below @n
 *  @n <b> Last data handling in slave </b> @n
 *  If the user wants to implement a slave which will read a byte from a specific location over and over
 *  again whenever master reads the slave. If the user initializes the #I2C_XFER_T.txBuff as the location to read
 *  the byte from and #I2C_XFER_T.txSz as 1, then say, if master reads one byte; slave will send the byte read from
 *  #I2C_XFER_T.txBuff and will call the event handler with #I2C_EVENT_DONE. If the master attempts to read another
 *  byte instead of sending the byte read from #I2C_XFER_T.txBuff the slave hardware will send 0xFF and no event will
 *  occur. To handle this issue, slave should set #I2C_XFER_T.txSz to 2, in which case when master reads the byte
 *  event handler will be called with #I2C_EVENT_SLAVE_TX, in which the slave implementation can reset the buffer
 *  and size back to original location (i.e, #I2C_XFER_T.txBuff--, #I2C_XFER_T.txSz++), if the master reads another byte
 *  in the same transfer, byte read from #I2C_XFER_T.txBuff will be sent and #I2C_EVENT_SLAVE_TX will be called again, and
 *  the process repeats.
 */
void Chip_I2C_SlaveSetup(I2C_ID_T id, I2C_SLAVE_ID sid, I2C_XFER_T *xfer, I2C_EVENTHANDLER_T event, uint8_t addrMask);

/**
 * I2C Slave event handler
 * @param id : I2C peripheral ID (#I2C0)
 */
void Chip_I2C_SlaveStateHandler(I2C_ID_T id);

/**
 * I2C peripheral state change checking
 * @param id : I2C peripheral ID (#I2C0)
 * @return 1 if I2C peripheral @a id has changed its state, 0 if there is no state change
 * @note This function is applicable for polling mode and must be used by the application when the polling has to be done
 *  based on state change.
 */
int Chip_I2C_IsStateChanged(I2C_ID_T id);

#endif /** @} */
