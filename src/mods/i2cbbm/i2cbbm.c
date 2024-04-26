/*
 * Copyright 2017-2018,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "board.h"
#include "i2cbbm.h"

/*
 * timing adjustments:
 * Changing these parameters changes the time waited for various clock pulses.
 * Each unit adds approximately:
 *
 *   System CLK frequency  |  period increase
 *  -----------------------|----------------------
 *   8Mhz                  |  0.126us to the wait.
 *   4Mhz                  |  0.275us to the wait.
 *   2Mhz                  |  0.50us to the wait.
 *   0.5Mhz                |  2.03us to the wait.
 */
#define DATA_HOLD_TIME (0 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Define the time between setting the data line and the rising edge of the CLK */
#define CLOCK_HIGH_TIME (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Length of the clock pulse during #SendByte */
#define CLOCK_LOW_TIME (1 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Length of the low voltage part of the clock period in #SendByte */
#define ACK_DATA_WAIT_TIME (0 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< 2/2 of the clock pulse. used to measure the ACK bit halfway through the CLK pulse */
#define ACK_LOW_TIME (0 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Low clock time after ACK in #SendByte */
#define INIT_CLOCK_LOW_TIME (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time before starting a receive byte */
#define ACK_HOLD_TIME (5 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time between sending the byte and the rising edge of the clock for the ACK bit*/
#define DATA_HOLD_TIME_R (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time to hold the data before the rising CLK edge */
#define CLOCK_HIGH_TIME_R (0 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time between the rising edge of the CLK and when sampling occurs */
#define CLOCK_HIGH_TIME_R2 (0 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time after sampling and before the falling edge of the CLK */
#define CLOCK_LOW_TIME_R (1 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time after the falling edge of the CLK during #ReceiveByte */
#define ACK_SET_TIME (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time between finishing sending the byte and when the data line is changed for the ACK bit */
#define ACK_SET_TIME2 (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time between setting the ACK bit and CLK rising edge */
#define ACK_SET_TIME3 (0 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Time between the CLK rising edge and the falling edge */
/**
 * Defines several timings:
 * - The time waited before initiating a write/read
 * - The time waited after the rising clk edge and the data rising edge
 * - The time between the data rising edge and the data falling edge
 */
#define START_TIME_1 (7 / I2CBBM_SYSTEM_CLOCK_DIVIDER)
#define START_TIME_2 (7 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Defines the time waited between the the data falling edge and clk falling edge during the start function */
/**
 * Defines several timings:
 * - The time waited before initiating a write/read
 * - The time waited after the rising clk edge and the data being set low
 * - The time between the data rising edge and the data falling edge
 */
#define STOP_TIME_1 (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER)
#define STOP_TIME_2 (10 / I2CBBM_SYSTEM_CLOCK_DIVIDER) /**< Defines the time waited between the the data falling edge and clk falling edge during the stop function */
#define PULSE_WIDTH (I2CBBM_PULSE_WIDTH / I2CBBM_SYSTEM_CLOCK_DIVIDER)

/**
 * Dummy variable to test the values of #I2CBBM_CLK_PIN and #I2CBBM_DAT_PIN.
 * If the macros are the same, the dummy variable will have a negative array size and the compiler will raise an error
 * similar to:
 *   ../i2cbbm.c:71:13: error: size of array 'sTestPinAssignment' is negative
 */
static char sTestPinAssignment[2*(I2CBBM_CLK_PIN != I2CBBM_DAT_PIN) - 1] __attribute__((unused));

/* ------------------------------------------------------------------------- */

static uint8_t sAddressRead;
static uint8_t sAddressWrite;
static int sOriginalClkState;
static int sOriginalDatState;
static bool sOriginalClkDir;
static bool sOriginalDatDir;

/* ------------------------------------------------------------------------- */

static void Wait(uint32_t cycles);
static void WaitForClkHigh(void);
static bool SendByte(uint8_t b);
static uint8_t ReceiveByte(uint8_t ack);
static void Start(void);
static void Stop(void);
static unsigned int SendBytes(const uint8_t * pBuffer, unsigned int length);
static void ReceiveBytes(uint8_t * pBuffer, unsigned int length, uint8_t lastAck);

static void ClkLow(void);
static void ClkHigh(void);
static void DatLow(void);
static void DatHigh(void);
static bool ClkGet(void);
static bool DatGet(void);

/* ------------------------------------------------------------------------- */

/**
 * Generates a start condition on the I2C bus
 * @post CLK is low, DAT is low
 */
static void Start(void)
{
    Wait(START_TIME_1);
    ClkHigh();
    WaitForClkHigh();
    Wait((PULSE_WIDTH / 3) + START_TIME_1); // ~10
    DatHigh();
    Wait((PULSE_WIDTH / 3) + START_TIME_1); // ~10
    DatLow();
    Wait((PULSE_WIDTH / 3) + START_TIME_2); // ~10
    ClkLow();
}

/**
 * Generates a stop condition on the I2C bus
 * @post CLK is high, DAT is high
 */
static void Stop(void)
{
    Wait(STOP_TIME_1);
    ClkLow();
    Wait(STOP_TIME_1);
    DatLow();
    Wait(STOP_TIME_1);
    ClkHigh();
    WaitForClkHigh();
    Wait(STOP_TIME_2);
    DatHigh();
}

/**
 * Transmits data.
 * @pre A start condition must be generated before this function can be used
 * @param buffer the bytes to send
 * @param length the number of bytes to send
 * @return the number of bytes send successfully
 * @post Bus is still locked. A stop condition or other command can be issued
 */
static unsigned int SendBytes(const uint8_t * pBuffer, unsigned int length)
{
    unsigned int i;
    const uint8_t* p = pBuffer;
    for (i = 0; i < length; i++) {
        if (SendByte(*p)) {
            break; /* If the byte isn't ack'ed, abort the transmission. */
        }
        p++;
    }
    return i;
}

/**
 * Receives data.
 * @pre A start condition must be generated before this function can be used
 * @param buffer To storage for the received bytes
 * @param length the number of bytes to receive
 * @param lastAck Non-zero if the last byte also needs to be acked. Zero if no ack
 * @post Bus is still locked. A stop condition or other command can be issued
 */
static void ReceiveBytes(uint8_t * pBuffer, unsigned int length, uint8_t lastAck)
{
    unsigned int i;
    uint8_t * p = pBuffer;
    for (i = 0; i < length; i++) {
        *p = ReceiveByte((i < length - 1) ? 1 : lastAck); /* If the byte isn't ack'ed, we abort the reception. */
        p++;
    }
}

/** Set the CLK line unconditionally low. */
static void ClkLow(void)
{
    Chip_GPIO_SetPinDIROutput(NSS_GPIO, 0, I2CBBM_CLK_PIN);
    Chip_GPIO_SetPinOutLow(NSS_GPIO, 0, I2CBBM_CLK_PIN);
}

/**
 * Release the CLK line. If the I2C slave is not pulling low, the pullup(s) attached to the CLK line will set the line
 * high.
 */
static void ClkHigh(void)
{
    Chip_GPIO_SetPinDIRInput(NSS_GPIO, 0, I2CBBM_CLK_PIN);
}

/** Set the DAT line unconditionally low. */
static void DatLow(void)
{
    Chip_GPIO_SetPinDIROutput(NSS_GPIO, 0, I2CBBM_DAT_PIN);
    Chip_GPIO_SetPinOutLow(NSS_GPIO, 0, I2CBBM_DAT_PIN);
}

/** Release the DAT line. The pullup(s) attached to the CLK line will set the line high. */
static void DatHigh(void)
{
    Chip_GPIO_SetPinDIRInput(NSS_GPIO, 0, I2CBBM_DAT_PIN);
}

/** @return The status of the CLK line. */
static bool ClkGet(void)
{
    return Chip_GPIO_GetPinState(NSS_GPIO, 0, I2CBBM_CLK_PIN);
}

/** @return The status of the DAT line. */
static bool DatGet(void)
{
    return Chip_GPIO_GetPinState(NSS_GPIO, 0, I2CBBM_DAT_PIN);
}

/**
 * Implements a simple waiting loop
 * @param cycles The number of cycles to wait
 * Depending on the system clock frequency, each unit adds a number of microseconds to the waiting time:
 * |@b SysClock|increased waiting time in us, per unit|
 * |--:        |:-                                    |
 * |8Mhz       |0.126 us                              |
 * |4Mhz       |0.275 us                              |
 * |2Mhz       |0.50 us                               |
 * |0.5Mhz     |2.03 us                               |
 */
__attribute__ ((noinline)) /* Inlining would cause multiple definitions of _DELAY_LOOP at the linker level. */
static void Wait(uint32_t cycles)
{
    if (cycles == 0) return;

    __asm ("push {r1}"); /* Backup r1 to the stack since it's used in the assembly code below */
    __asm ("movs r1, %[ticks]" : : [ticks]"r"(cycles)); /* Copy value from cycles to r1 */
    __asm ( "_DELAY_LOOP:    \n" /* Busy wait loop in assembler so that it will never be optimized out */
            "sub r1, #3      \n" /* instruction 1: 3 ticks per loop */
            "bgt _DELAY_LOOP \n" /* instruction 2 */
    );
    __asm ("pop {r1}"); /* Recover r1 from the stack */
}

/**
 * Wait until CLK is high again, or until the maximum clock stretch waiting time #I2CBBM_MAX_CLK_STRETCH has expired.
 */
static void WaitForClkHigh(void)
{
    uint32_t stretch = 0;
    do {
        stretch++;
    } while ((!ClkGet()) && (stretch < I2CBBM_MAX_CLK_STRETCH));
}

/**
 * Sends a byte over the I2C bus. (MSb first)
 * @pre Precondition: CLK should be low. The time it is low may be 0. This function will wait the sCLOCK_LOW_TIME
 * @param b byte to send
 * @return Whether the byte was ACKed (@c true), or not (@c false).
 * @post CLK is low
 */
static bool SendByte(uint8_t b)
{
    bool ack = 0;
    uint8_t mask = 0x80;

    Wait(INIT_CLOCK_LOW_TIME);

    for (int bit = 0; bit < 8 /* 8 bits per byte */; bit++) {
        if (b & mask) {
            DatHigh();
        }
        else {
            DatLow();
        }
        mask >>= 1;

        /* Generate a clock pulse */
        Wait(DATA_HOLD_TIME);
        ClkHigh();
        WaitForClkHigh();
        Wait(PULSE_WIDTH + CLOCK_HIGH_TIME);
        ClkLow();
        Wait(CLOCK_LOW_TIME);
    }

    /* Switch the DAT line to input mode to sample the ACK bit. */
    DatHigh();

    Wait(ACK_HOLD_TIME);
    ClkHigh();
    WaitForClkHigh();
    Wait((PULSE_WIDTH / 2) + ACK_DATA_WAIT_TIME);

    /* Read the ACK bit. */
    ack = DatGet();
    Wait((PULSE_WIDTH / 2) + ACK_DATA_WAIT_TIME);
    ClkLow();
    Wait(ACK_LOW_TIME);

    return ack;
}

/**
 * Receives a byte over the I2C bus
 * @pre CLK should be low. The time it is low may be 0. This function will wait the sCLOCK_LOW_TIME
 * @param ack 0: Do not ack this byte. Non-zero: Ack this byte
 * @return The received byte
 * @post CLK is low
 */
static uint8_t ReceiveByte(uint8_t ack)
{
    uint8_t b = 0;
    uint8_t mask = 0x80;

    DatHigh();
    Wait(INIT_CLOCK_LOW_TIME);

    for (int bit = 0; bit < 8 /* 8 bits per byte */; bit++) {
        Wait(DATA_HOLD_TIME_R);
        ClkHigh();

        WaitForClkHigh();
        Wait((PULSE_WIDTH / 2) + CLOCK_HIGH_TIME_R);

        /* Read the bit */
        if (DatGet()) {
            b |= mask;
        }
        mask >>= 1;

        Wait((PULSE_WIDTH / 2) + CLOCK_HIGH_TIME_R2);
        ClkLow();
        Wait(CLOCK_LOW_TIME_R);
    }

    /* Send the ACK bit */
    Wait(ACK_SET_TIME);
    if (ack) {
        DatLow();
    }
    else {
        DatHigh();
    }

    Wait(ACK_SET_TIME2);

    ClkHigh();
    WaitForClkHigh();
    Wait(PULSE_WIDTH + ACK_SET_TIME3);
    ClkLow();
    Wait(ACK_LOW_TIME);

    DatHigh();

    return b;
}

/* ------------------------------------------------------------------------- */

void I2cbbm_Init(void)
{
    /* Store the original states to restore them in I2cbbm_DeInit. */
    sOriginalClkDir = Chip_GPIO_GetPinDIR(NSS_GPIO, 0, I2CBBM_CLK_PIN);
    sOriginalDatDir = Chip_GPIO_GetPinDIR(NSS_GPIO, 0, I2CBBM_DAT_PIN);
    sOriginalClkState = Chip_IOCON_GetPinConfig(NSS_IOCON, I2CBBM_CLK_PIN);
    sOriginalDatState = Chip_IOCON_GetPinConfig(NSS_IOCON, I2CBBM_DAT_PIN);

#if I2CBBM_PULLUP_COUNT
    /* Configure pull-ups. */
    static const IOCON_PIN_T sPullups[I2CBBM_PULLUP_COUNT] = I2CBBM_PULLUPS;
    uint32_t pinMask = 0;
    for (int i = 0; i < I2CBBM_PULLUP_COUNT; i++) {
        Chip_IOCON_SetPinConfig(NSS_IOCON, sPullups[i], IOCON_FUNC_0 | IOCON_I2CMODE_PIO | IOCON_RMODE_PULLUP);
        pinMask |= (uint32_t)NSS_GPIOn_PINMASK(sPullups[i]);
    }
    if (pinMask) {
        Chip_GPIO_SetPortDIRInput(NSS_GPIO, 0, pinMask);
    }
#endif

    Chip_IOCON_SetPinConfig(NSS_IOCON, I2CBBM_CLK_PIN, IOCON_FUNC_0 | IOCON_RMODE_PULLUP);
    Chip_IOCON_SetPinConfig(NSS_IOCON, I2CBBM_DAT_PIN, IOCON_FUNC_0 | IOCON_RMODE_PULLUP);
    DatHigh();
    ClkHigh();

    I2cbbm_SetAddress(I2CBBM_DEFAULT_I2C_ADDRESS);
}

void I2cbbm_DeInit(void)
{
    Chip_GPIO_SetPinDIR(NSS_GPIO, 0, I2CBBM_CLK_PIN, sOriginalClkDir);
    Chip_GPIO_SetPinDIR(NSS_GPIO, 0, I2CBBM_DAT_PIN, sOriginalDatDir);
    Chip_IOCON_SetPinConfig(NSS_IOCON, I2CBBM_CLK_PIN, sOriginalClkState);
    Chip_IOCON_SetPinConfig(NSS_IOCON, I2CBBM_DAT_PIN, sOriginalDatState);
}

int I2cbbm_Write(const uint8_t * pBuf, unsigned int size)
{
    unsigned int retval = 0;
    Start();
    if (SendBytes(&sAddressWrite, 1) == 1) {
        if (pBuf && size) {
            retval = SendBytes(pBuf, size);
        }
    }
    Stop();
    return (retval < size) ? -1 : (int)retval;
}

int I2cbbm_Read(uint8_t * pBuf, unsigned int size)
{
    int retval = -1;
    if (pBuf && size) {
        Start();
        SendBytes(&sAddressRead, 1);
        ReceiveBytes(pBuf, size, 0);
        Stop();
        retval = (int)size;
    }
    return retval;
}

int I2cbbm_WriteRead(const uint8_t * pWriteBuf, unsigned int writeSize, uint8_t * pReadBuf, unsigned int readSize)
{
    int retval = -1;
    unsigned int writelen = 0;

    Start();
    if (SendBytes(&sAddressWrite, 1) == 1) {
        if (pWriteBuf && writeSize) {
            writelen = SendBytes(pWriteBuf, writeSize);
        }
        retval = 0;
    }

    if (pReadBuf && readSize && (writeSize == writelen) && (retval == 0)) {
        Start(); /* Make a restart condition: just a second start */
        SendBytes(&sAddressRead, 1);
        ReceiveBytes(pReadBuf, readSize, 0);
    }
    Stop();
    if (writelen == writeSize) {
        retval = (int)readSize;
    }
    return retval;
}

void I2cbbm_SetAddress(uint8_t address)
{
    sAddressRead = (uint8_t)(address << 1) | 1;
    sAddressWrite = (uint8_t)(address << 1);
}
