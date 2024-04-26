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

#include "chip.h"

/* ------------------------------------------------------------------------- */

/** Return value for SLAVE handler, when 1 byte TX'd successfully */
#define RET_SLAVE_TX 6
/** Return value for SLAVE handler, when 1 byte RX'd successfully */
#define RET_SLAVE_RX 5
/** Return value for SLAVE handler, when slave enter idle mode */
#define RET_SLAVE_IDLE 2
/** Return value for SLAVE handler, when slave is busy */
#define RET_SLAVE_BUSY 0

/** I2C state handle return values */
#define I2C_STA_STO_RECV            0x20

/** I2C Control Set Assert acknowledge flag */
#define I2C_I2CONSET_AA             ((0x04))
/** I2C Control Set I2C interrupt flag */
#define I2C_I2CONSET_SI             ((0x08))
/** I2C Control Set STOP flag */
#define I2C_I2CONSET_STO            ((0x10))
/** I2C Control Set START flag */
#define I2C_I2CONSET_STA            ((0x20))
/** I2C Control Set I2C interface enable */
#define I2C_I2CONSET_I2EN           ((0x40))

/** I2C Control Clear Assert acknowledge Clear bit */
#define I2C_I2CONCLR_AAC            ((1 << 2))
/** I2C Control Clear I2C interrupt Clear bit */
#define I2C_I2CONCLR_SIC            ((1 << 3))
/** I2C Control Clear I2C STOP Clear bit */
#define I2C_I2CONCLR_STOC           ((1 << 4))
/** I2C Control Clear START flag Clear bit */
#define I2C_I2CONCLR_STAC           ((1 << 5))
/** I2C Control Clear I2C interface Disable bit */
#define I2C_I2CONCLR_I2ENC          ((1 << 6))

/** I2C Common Control Assert acknowledge bit */
#define I2C_CON_AA            (1UL << 2)
/** I2C Common Control I2C interrupt bit */
#define I2C_CON_SI            (1UL << 3)
/** I2C Common Control I2C STOP bit */
#define I2C_CON_STO           (1UL << 4)
/** I2C Common Control START flag bit */
#define I2C_CON_STA           (1UL << 5)
/** I2C Common Control I2C interface bit */
#define I2C_CON_I2EN          (1UL << 6)

/** Return Code mask in I2C status register */
#define I2C_STAT_CODE_BITMASK       ((0xF8))
/** Return Code error mask in I2C status register */
#define I2C_STAT_CODE_ERROR         ((0xFF))

/** I2C return status code for no relevant information available */
#define I2C_I2STAT_NO_INF                       ((0xF8))
/** I2C return status code for Bus Error */
#define I2C_I2STAT_BUS_ERROR                    ((0x00))

/** I2C Master transmit mode - A start condition has been transmitted */
#define I2C_I2STAT_M_TX_START                   ((0x08))
/** I2C Master transmit mode - A repeat start condition has been transmitted */
#define I2C_I2STAT_M_TX_RESTART                 ((0x10))
/** I2C Master transmit mode - SLA+W has been transmitted, ACK has been received */
#define I2C_I2STAT_M_TX_SLAW_ACK                ((0x18))
/** I2C Master transmit mode - SLA+W has been transmitted, NACK has been received */
#define I2C_I2STAT_M_TX_SLAW_NACK               ((0x20))
/** I2C Master transmit mode - Data has been transmitted, ACK has been received */
#define I2C_I2STAT_M_TX_DAT_ACK                 ((0x28))
/** I2C Master transmit mode - Data has been transmitted, NACK has been received */
#define I2C_I2STAT_M_TX_DAT_NACK                ((0x30))
/** I2C Master transmit mode - Arbitration lost in SLA+R/W or Data bytes */
#define I2C_I2STAT_M_TX_ARB_LOST                ((0x38))

/** I2C Master receive mode - A start condition has been transmitted */
#define I2C_I2STAT_M_RX_START                   ((0x08))
/** I2C Master receive mode - A repeat start condition has been transmitted */
#define I2C_I2STAT_M_RX_RESTART                 ((0x10))
/** I2C Master receive mode - Arbitration lost */
#define I2C_I2STAT_M_RX_ARB_LOST                ((0x38))
/** I2C Master receive mode - SLA+R has been transmitted, ACK has been received */
#define I2C_I2STAT_M_RX_SLAR_ACK                ((0x40))
/** I2C Master receive mode - SLA+R has been transmitted, NACK has been received */
#define I2C_I2STAT_M_RX_SLAR_NACK               ((0x48))
/** I2C Master receive mode - Data has been received, ACK has been returned */
#define I2C_I2STAT_M_RX_DAT_ACK                 ((0x50))
/** I2C Master receive mode - Data has been received, NACK has been returned */
#define I2C_I2STAT_M_RX_DAT_NACK                ((0x58))

/** I2C Slave receive mode - Own slave address has been received, ACK has been returned */
#define I2C_I2STAT_S_RX_SLAW_ACK                ((0x60))
/** I2C Slave receive mode - Arbitration lost in SLA+R/W as master */
#define I2C_I2STAT_S_RX_ARB_LOST_M_SLA          ((0x68))
/** I2C Slave receive mode - General call address has been received, ACK has been returned */
#define I2C_I2STAT_S_RX_GENCALL_ACK             ((0x70))
/** I2C Slave receive mode - Arbitration lost in SLA+R/W (GENERAL CALL) as master */
#define I2C_I2STAT_S_RX_ARB_LOST_M_GENCALL      ((0x78))
/** I2C Slave receive mode - Previously addressed with own SLA; Data has been received, ACK has been returned */
#define I2C_I2STAT_S_RX_PRE_SLA_DAT_ACK         ((0x80))
/** I2C Slave receive mode - Previously addressed with own SLA;Data has been received and NOT ACK has been returned */
#define I2C_I2STAT_S_RX_PRE_SLA_DAT_NACK        ((0x88))
/** I2C Slave receive mode - Previously addressed with General Call; Data has been received and ACK has been returned */
#define I2C_I2STAT_S_RX_PRE_GENCALL_DAT_ACK     ((0x90))
/** I2C Slave receive mode - Previously addressed with General Call; Data has been received and NOT ACK has been returned */
#define I2C_I2STAT_S_RX_PRE_GENCALL_DAT_NACK    ((0x98))
/** I2C Slave receive mode - A STOP condition or repeated START condition has been received while still addressed as
 SLV/REC (Slave Receive) or SLV/TRX (Slave Transmit) */
#define I2C_I2STAT_S_RX_STA_STO_SLVREC_SLVTRX   ((0xA0))

/** I2C Slave transmit mode - Own SLA+R has been received, ACK has been returned */
#define I2C_I2STAT_S_TX_SLAR_ACK                ((0xA8))
/** I2C Slave transmit mode - Arbitration lost in SLA+R/W as master */
#define I2C_I2STAT_S_TX_ARB_LOST_M_SLA          ((0xB0))
/** I2C Slave transmit mode - Data has been transmitted, ACK has been received */
#define I2C_I2STAT_S_TX_DAT_ACK                 ((0xB8))
/** I2C Slave transmit mode - Data has been transmitted, NACK has been received */
#define I2C_I2STAT_S_TX_DAT_NACK                ((0xC0))
/** I2C Slave transmit mode - Last data byte in I2DAT has been transmitted (AA = 0); ACK has been received */
#define I2C_I2STAT_S_TX_LAST_DAT_ACK            ((0xC8))
/** I2C Slave transmit mode - Time out in case of using I2C slave mode */
#define I2C_SLAVE_TIME_OUT                      0x10000000UL

/** Mask for I2C Data register */
#define I2C_I2DAT_BITMASK           ((0xFF))
/** Idle data value will be send out in slave mode in case of the actual expecting data requested from the master is
 greater than its sending data length that can be supported */
#define I2C_I2DAT_IDLE_CHAR         (0xFF)

/** Monitor mode control register description - Monitor mode enable */
#define I2C_I2MMCTRL_MM_ENA         ((1 << 0))
/** Monitor mode control register description - SCL output enable */
#define I2C_I2MMCTRL_ENA_SCL        ((1 << 1))
/** Monitor mode control register description - Select interrupt register match */
#define I2C_I2MMCTRL_MATCH_ALL      ((1 << 2))
/** Monitor mode control register description - Mask for I2MMCTRL register */
#define I2C_I2MMCTRL_BITMASK        ((0x07))

/** I2C Data buffer register bit mask */
#define I2DATA_BUFFER_BITMASK       ((0xFF))

/** I2C Slave Address General Call enable bit */
#define I2C_I2ADR_GC                ((1 << 0))
/** I2C Slave Address register bit mask */
#define I2C_I2ADR_BITMASK           ((0xFF))

/** I2C Mask Register mask field*/
#define I2C_I2MASK_MASK(n)          (((n) & 0xFE))

/** I2C SCL HIGH duty cycle Register bit mask */
#define I2C_I2SCLH_BITMASK          ((0xFFFF))

/** I2C SCL LOW duty cycle Register bit mask */
#define I2C_I2SCLL_BITMASK          ((0xFFFF))

/** I2C status values - Arbitration false */
#define I2C_SETUP_STATUS_ARBF   (1 << 8)
/** I2C status values - No ACK returned */
#define I2C_SETUP_STATUS_NOACKF (1 << 9)
/** I2C status values - Status DONE */
#define I2C_SETUP_STATUS_DONE   (1 << 10)

/** I2C monitor control configuration define for SCL output enable */
#define I2C_MONITOR_CFG_SCL_OUTPUT I2C_I2MMCTRL_ENA_SCL
/** I2C monitor control configuration define for Select interrupt register match */
#define I2C_MONITOR_CFG_MATCHALL I2C_I2MMCTRL_MATCH_ALL

/* Control flags */
#define I2C_CON_FLAGS (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)
#define NSS_I2Cx(id)      ((i2c[id].ip))
#define SLAVE_ACTIVE(iic) (((iic)->flags & 0xFF00) != 0)

/* I2C common interface structure */
struct i2c_interface {
    NSS_I2C_T *ip; /* IP base address of the I2C device */
    CLOCK_PERIPHERAL_T clk; /* Clock used by I2C */
    I2C_EVENTHANDLER_T mEvent; /* Current active Master event handler */
    I2C_EVENTHANDLER_T sEvent; /* Slave transfer events */
    I2C_XFER_T *mXfer; /* Current active xfer pointer */
    I2C_XFER_T *sXfer; /* Pointer to store xfer when bus is busy */
    uint32_t flags; /* Flags used by I2C master and slave */
};

/* Slave interface structure */
struct i2c_slave_interface {
    I2C_XFER_T *xfer;
    I2C_EVENTHANDLER_T event;
};

/* I2C interfaces */
static struct i2c_interface i2c[I2C_NUM_INTERFACE] = { {NSS_I2C,
                                                        CLOCK_PERIPHERAL_I2C0,
                                                        Chip_I2C_EventHandler,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        0}};

static struct i2c_slave_interface i2c_slave[I2C_NUM_INTERFACE][I2C_SLAVE_NUM_INTERFACE];

/* ------------------------------------------------------------------------- */

static inline void enableClk(I2C_ID_T id)
{
    Chip_Clock_Peripheral_EnableClock(i2c[id].clk);
}

static inline void disableClk(I2C_ID_T id)
{
    Chip_Clock_Peripheral_DisableClock(i2c[id].clk);
}

/* Get the ADC Clock Rate */
#pragma GCC diagnostic ignored "-Wunused-parameter"
static inline uint32_t getClkRate(I2C_ID_T id)
{
    return (uint32_t)Chip_Clock_System_GetClockFreq();
}

/* Enable I2C and start master transfer */
static inline void startMasterXfer(NSS_I2C_T *pI2C)
{
    /* Reset STA, STO, SI */
    pI2C->CONCLR = I2C_CON_SI | I2C_CON_STO | I2C_CON_STA | I2C_CON_AA;

    /* Enter to Master Transmitter mode */
    pI2C->CONSET = I2C_CON_I2EN | I2C_CON_STA;
}

/* Enable I2C and enable slave transfers */
static inline void startSlaverXfer(NSS_I2C_T *pI2C)
{
    /* Reset STA, STO, SI */
    pI2C->CONCLR = I2C_CON_SI | I2C_CON_STO | I2C_CON_STA;

    /* Enter to Master Transmitter mode */
    pI2C->CONSET = I2C_CON_I2EN | I2C_CON_AA;
}

/* Check if I2C bus is free */
static inline int isI2CBusFree(NSS_I2C_T *pI2C)
{
    return !(pI2C->CONSET & I2C_CON_STO);
}

/* Get current state of the I2C peripheral */
static inline int getCurState(NSS_I2C_T *pI2C)
{
    return (int)(pI2C->STAT & I2C_STAT_CODE_BITMASK);
}

/* Check if the active state belongs to master mode*/
static inline int isMasterState(NSS_I2C_T *pI2C)
{
    return getCurState(pI2C) < 0x60;
}

/* Set OWN slave address for specific slave ID */
static void setSlaveAddr(NSS_I2C_T *pI2C, I2C_SLAVE_ID sid, uint8_t addr, uint8_t mask)
{
    uint32_t index = (uint32_t)sid - 1;
    pI2C->MASK[index] = mask;
    if (sid == I2C_SLAVE_0) {
        pI2C->ADR0 = addr;
    }
    else {
        volatile uint32_t *abase = &pI2C->ADR1;
        abase[index - 1] = addr;
    }
}

/* Match the slave address */
static int isSlaveAddrMatching(uint8_t addr1, uint8_t addr2, uint8_t mask)
{
    mask |= 1;
    return (addr1 & ~mask) == (addr2 & ~mask);
}

/* Get the index of the active slave */
static I2C_SLAVE_ID lookupSlaveIndex(NSS_I2C_T *pI2C, uint8_t slaveAddr)
{
    if (!(slaveAddr >> 1)) {
        return I2C_SLAVE_GENERAL; /* General call address */
    }
    if (isSlaveAddrMatching((uint8_t)pI2C->ADR0, slaveAddr, (uint8_t)pI2C->MASK[0])) {
        return I2C_SLAVE_0;
    }
    if (isSlaveAddrMatching((uint8_t)pI2C->ADR1, slaveAddr, (uint8_t)pI2C->MASK[1])) {
        return I2C_SLAVE_1;
    }
    if (isSlaveAddrMatching((uint8_t)pI2C->ADR2, slaveAddr, (uint8_t)pI2C->MASK[2])) {
        return I2C_SLAVE_2;
    }
    if (isSlaveAddrMatching((uint8_t)pI2C->ADR3, slaveAddr, (uint8_t)pI2C->MASK[3])) {
        return I2C_SLAVE_3;
    }

    /* If everything is fine the code should never come here */
    return 0xFF; /* internal error code */
}

/* Master transfer state change handler handler */
int handleMasterXferState(NSS_I2C_T *pI2C, I2C_XFER_T *xfer)
{
    uint32_t cclr = I2C_CON_FLAGS;

    switch (getCurState(pI2C)) {
        case 0x08: /* Start condition on bus */
        case 0x10: /* Repeated start condition */
            pI2C->DAT = (uint32_t)((xfer->slaveAddr << 1) | (xfer->txSz == 0));
            break;

            /* Tx handling */
        case 0x18: /* SLA+W sent and ACK received */
            /* fallthrough */
        case 0x28: /* DATA sent and ACK received */
            if (!xfer->txSz) {
                cclr &= ~(xfer->rxSz ? I2C_CON_STA : I2C_CON_STO);
            }
            else {
                pI2C->DAT = *xfer->txBuff++;
                xfer->txSz--;
            }
            break;

            /* Rx handling */
        case 0x58: /* Data Received and NACK sent */
            cclr &= ~I2C_CON_STO;
            /* fallthrough */

        case 0x50: /* Data Received and ACK sent */
            *xfer->rxBuff++ = (uint8_t)pI2C->DAT;
            xfer->rxSz--;
            /* fallthrough */

        case 0x40: /* SLA+R sent and ACK received */
            if (xfer->rxSz > 1) {
                cclr &= ~I2C_CON_AA;
            }
            break;

            /* NAK Handling */
        case 0x30: /* DATA sent NAK received */
            xfer->txBuff--;
            xfer->txSz++;
            /* fallthrough */

        case 0x20: /* SLA+W sent NAK received */
        case 0x48: /* SLA+R sent NAK received */
            xfer->status = I2C_STATUS_NAK;
            cclr &= ~I2C_CON_STO;
            break;

        case 0x38: /* Arbitration lost */
            xfer->status = I2C_STATUS_ARBLOST;
            break;

            /* Bus Error */
        case 0x00:
            xfer->status = I2C_STATUS_BUSERR;
            cclr &= ~I2C_CON_STO;
            break;
    }

    /* Set clear control flags */
    pI2C->CONSET = cclr ^ I2C_CON_FLAGS;
    pI2C->CONCLR = cclr;

    /* If stopped return 0 */
    if (!(cclr & I2C_CON_STO) || (xfer->status == I2C_STATUS_ARBLOST)) {
        if (xfer->status == I2C_STATUS_BUSY) {
            xfer->status = I2C_STATUS_DONE;
        }
        return 0;
    }
    return 1;
}

/* Find the slave address of SLA+W or SLA+R */
I2C_SLAVE_ID getSlaveIndex(NSS_I2C_T *pI2C)
{
    switch (getCurState(pI2C)) {
        case 0x60:
        case 0x68:
        case 0x70:
        case 0x78:
        case 0xA8:
        case 0xB0:
            return lookupSlaveIndex(pI2C, (uint8_t)pI2C->DAT);
    }

    /* If everything is fine code should never come here */
    return 0xFF; /* internal error code */
}

/* Slave state machine handler */
int handleSlaveXferState(NSS_I2C_T *pI2C, I2C_XFER_T *xfer)
{
    uint32_t cclr = I2C_CON_FLAGS;
    int ret = RET_SLAVE_BUSY;

    xfer->status = I2C_STATUS_BUSY;
    switch (getCurState(pI2C)) {
        case 0x80: /* SLA: Data received + ACK sent */
        case 0x90: /* GC: Data received + ACK sent */
            *xfer->rxBuff++ = (uint8_t)pI2C->DAT;
            xfer->rxSz--;
            ret = RET_SLAVE_RX;
            if (xfer->rxSz > 1) {
                cclr &= ~I2C_CON_AA;
            }
            break;

        case 0x60: /* Own SLA+W received */
        case 0x68: /* Own SLA+W received after losing arbitration */
        case 0x70: /* GC+W received */
        case 0x78: /* GC+W received after losing arbitration */
            xfer->slaveAddr = (uint8_t)(pI2C->DAT & ~1u);
            if (xfer->rxSz > 1) {
                cclr &= ~I2C_CON_AA;
            }
            break;

        case 0xA8: /* SLA+R received */ /* fallthrough */
        case 0xB0: /* SLA+R received after losing arbitration */
            xfer->slaveAddr = (uint8_t)(pI2C->DAT & ~1u);
            /* fallthrough */

        case 0xB8: /* DATA sent and ACK received */
            pI2C->DAT = *xfer->txBuff++;
            xfer->txSz--;
            if (xfer->txSz > 0) {
                cclr &= ~I2C_CON_AA;
            }
            ret = RET_SLAVE_TX;
            break;

        case 0xC0: /* Data transmitted and NAK received */
        case 0xC8: /* Last data transmitted and ACK received */
        case 0x88: /* SLA: Data received + NAK sent */
        case 0x98: /* GC: Data received + NAK sent */
        case 0xA0: /* STOP/Repeated START condition received */
            ret = RET_SLAVE_IDLE;
            cclr &= ~I2C_CON_AA;
            xfer->status = I2C_STATUS_DONE;
            if (xfer->slaveAddr & 1) {
                cclr &= ~I2C_CON_STA;
            }
            break;
    }

    /* Set clear control flags */
    pI2C->CONSET = cclr ^ I2C_CON_FLAGS;
    pI2C->CONCLR = cclr;

    return ret;
}

/* ------------------------------------------------------------------------- */

/* Chip event handler interrupt based */
void Chip_I2C_EventHandler(I2C_ID_T id, I2C_EVENT_T event)
{
    struct i2c_interface *iic = &i2c[id];
    volatile I2C_STATUS_T *stat;

    /* Only WAIT event needs to be handled */
    if (event != I2C_EVENT_WAIT) {
        return;
    }

    stat = &iic->mXfer->status;
    /* Wait for the status to change */
    int counter = 100000; /* A safety counter to avoid possible infinite loops. Does its precise value matter that much? */
    do {
        counter--; /* wait */
    } while ((*stat == I2C_STATUS_BUSY) && counter);
//    while (*stat == I2C_STATUS_BUSY) {
//    }
}

/* Chip polling event handler */
void Chip_I2C_EventHandlerPolling(I2C_ID_T id, I2C_EVENT_T event)
{
    struct i2c_interface *iic = &i2c[id];
    volatile I2C_STATUS_T *stat;

    /* Only WAIT event needs to be handled */
    if (event != I2C_EVENT_WAIT) {
        return;
    }

    stat = &iic->mXfer->status;
    /* Call the state change handler till xfer is done */
    int counter = 100000; /* A safety counter to avoid possible infinite loops. Does its precise value matter that much? */
//    while (*stat == I2C_STATUS_BUSY) {
    while ((*stat == I2C_STATUS_BUSY) && counter) {
        counter--; /* wait */
        if (Chip_I2C_IsStateChanged(id)) {
            Chip_I2C_MasterStateHandler(id);
        }
    }
}

/* Initializes the NSS_I2C peripheral with specified parameter */
void Chip_I2C_Init(I2C_ID_T id)
{
    enableClk(id);

    /* Set I2C operation to default */
    NSS_I2Cx(id)->CONCLR = (I2C_CON_AA | I2C_CON_SI | I2C_CON_STA | I2C_CON_I2EN);

    /* Initialize the internal structure */
    i2c[id].flags = 0;
    i2c[id].mEvent = Chip_I2C_EventHandler;
    i2c[id].mXfer = NULL;
    i2c[id].sXfer = NULL;
}

/* De-initializes the I2C peripheral registers to their default reset values */
void Chip_I2C_DeInit(I2C_ID_T id)
{
    /* Disable I2C control */
    NSS_I2Cx(id)->CONCLR = I2C_CON_I2EN | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA | I2C_CON_AA;

    disableClk(id);
}

/* Set up clock rate for NSS_I2C peripheral */
void Chip_I2C_SetClockRate(I2C_ID_T id, uint32_t clockrate)
{
    uint32_t SCLValue;

    SCLValue = (getClkRate(id) / clockrate);
    NSS_I2Cx(id)->SCLH = (uint32_t)(SCLValue >> 1);
    NSS_I2Cx(id)->SCLL = (uint32_t)(SCLValue - NSS_I2Cx(id)->SCLH);
}

/* Get current clock rate for NSS_I2C peripheral */
uint32_t Chip_I2C_GetClockRate(I2C_ID_T id)
{
    return getClkRate(id) / (NSS_I2Cx(id)->SCLH + NSS_I2Cx(id)->SCLL);
}

/* Set the master event handler */
int Chip_I2C_SetMasterEventHandler(I2C_ID_T id, I2C_EVENTHANDLER_T event)
{
    struct i2c_interface *iic = &i2c[id];
    if (!iic->mXfer) {
        iic->mEvent = event;
    }
    return iic->mEvent == event;
}

/* Get the master event handler */
I2C_EVENTHANDLER_T Chip_I2C_GetMasterEventHandler(I2C_ID_T id)
{
    return i2c[id].mEvent;
}

/* Transmit and Receive data in master mode */
I2C_STATUS_T Chip_I2C_MasterTransfer(I2C_ID_T id, I2C_XFER_T *xfer)
{
    struct i2c_interface *iic = &i2c[id];

    iic->mEvent(id, I2C_EVENT_LOCK);
    xfer->status = I2C_STATUS_BUSY;
    iic->mXfer = xfer;

    /* If slave xfer not in progress */
    if (!iic->sXfer) {
        startMasterXfer(iic->ip);
    }
    iic->mEvent(id, I2C_EVENT_WAIT);
    iic->mXfer = 0;

    /* Wait for stop condition to appear on bus */
    while (!isI2CBusFree(iic->ip)) {
    }

    /* Start slave if one is active */
    if (SLAVE_ACTIVE(iic)) {
        startSlaverXfer(iic->ip);
    }

    iic->mEvent(id, I2C_EVENT_UNLOCK);
    return xfer->status;
}

/* Master tx only */
int Chip_I2C_MasterSend(I2C_ID_T id, uint8_t slaveAddr, const uint8_t *buff, int len)
{
    I2C_XFER_T xfer = {0};
    xfer.slaveAddr = slaveAddr;
    xfer.txBuff = buff;
    xfer.txSz = len;
    while (Chip_I2C_MasterTransfer(id, &xfer) == I2C_STATUS_ARBLOST) {
    }
    return len - xfer.txSz;
}

/* Transmit one byte and receive an array of bytes after a repeated start condition is generated in Master mode.
 * This function is useful for communicating with the I2C slave registers
 */
int Chip_I2C_MasterCmdRead(I2C_ID_T id, uint8_t slaveAddr, uint8_t *cmdBuff, uint8_t *buff, int len)
{
    I2C_XFER_T xfer = {0};
    xfer.slaveAddr = slaveAddr;
    xfer.txBuff = cmdBuff;
    xfer.txSz = 1;
    xfer.rxBuff = buff;
    xfer.rxSz = len;
    while (Chip_I2C_MasterTransfer(id, &xfer) == I2C_STATUS_ARBLOST) {
    }
    return len - xfer.rxSz;
}

/* Sequential master read */
int Chip_I2C_MasterRead(I2C_ID_T id, uint8_t slaveAddr, uint8_t *buff, int len)
{
    I2C_XFER_T xfer = {0};
    xfer.slaveAddr = slaveAddr;
    xfer.rxBuff = buff;
    xfer.rxSz = len;
    while (Chip_I2C_MasterTransfer(id, &xfer) == I2C_STATUS_ARBLOST) {
    }
    return len - xfer.rxSz;
}

/* Check if master state is active */
int Chip_I2C_IsMasterActive(I2C_ID_T id)
{
    return isMasterState(i2c[id].ip);
}

/* State change handler for master transfer */
void Chip_I2C_MasterStateHandler(I2C_ID_T id)
{
    if (!handleMasterXferState(i2c[id].ip, i2c[id].mXfer)) {
        i2c[id].mEvent(id, I2C_EVENT_DONE);
    }
}

/* Setup slave function */
void Chip_I2C_SlaveSetup(I2C_ID_T id, I2C_SLAVE_ID sid, I2C_XFER_T *xfer, I2C_EVENTHANDLER_T event, uint8_t addrMask)
{
    struct i2c_interface *iic = &i2c[id];
    struct i2c_slave_interface *si2c = &i2c_slave[id][sid];
    si2c->xfer = xfer;
    si2c->event = event;

    /* Set up the slave address */
    if (sid != I2C_SLAVE_GENERAL) {
        setSlaveAddr(iic->ip, sid, xfer->slaveAddr, addrMask);
    }

    if (!SLAVE_ACTIVE(iic) && !iic->mXfer) {
        startSlaverXfer(iic->ip);
    }
    iic->flags |= 1u << (sid + 8);
}

/* I2C Slave event handler */
void Chip_I2C_SlaveStateHandler(I2C_ID_T id)
{
    /* To handle unexpected interrupts with valid state */
    if ((i2c[id].ip->STAT & I2C_STAT_CODE_BITMASK) == 0xF8) {
        return;
    }

    int ret;
    struct i2c_interface *iic = &i2c[id];

    /* Get the currently addressed slave */
    if (!iic->sXfer) {
        struct i2c_slave_interface *si2c;

        I2C_SLAVE_ID sid = getSlaveIndex(iic->ip);
        if (sid != 0xFF) {
            si2c = &i2c_slave[id][sid];
            iic->sXfer = si2c->xfer;
            iic->sEvent = si2c->event;
        }
        else {
            return;
        }
    }

    iic->sXfer->slaveAddr = (uint8_t)(iic->sXfer->slaveAddr | (iic->mXfer != 0));
    ret = handleSlaveXferState(iic->ip, iic->sXfer);
    if (ret) {
        if (iic->sXfer->status == I2C_STATUS_DONE) {
            iic->sXfer = 0;
        }
        iic->sEvent(id, (I2C_EVENT_T)ret);
    }
}

/* Disable I2C device */
void Chip_I2C_Disable(I2C_ID_T id)
{
    NSS_I2Cx(id)->CONCLR = I2C_I2CONCLR_I2ENC;
}

/* State change checking */
int Chip_I2C_IsStateChanged(I2C_ID_T id)
{
    return (NSS_I2Cx(id)->CONSET & I2C_CON_SI) != 0;
}
