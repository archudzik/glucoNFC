/*
 * Copyright 2014-2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include <string.h>

/**
 * EEPROM clock frequency (in Hz)
 * @note Hardware spec: between 200kHz and 400kHz @n
 *  We want it to be as close as possible to BUT never higher than 400kHz. The EEPROM memory is using this as a
 *  reference clock for erasing/programming. If this clock frequency gets higher than 400 kHz, the program time is
 *  reduced and the erasing/programming cycle is not ensured.
 */
#define EEPROM_CLOCK_FREQUENCY_HZ (375 * 1000)

/** Time EEPROM needs to get ready after power-up */
#define EEPROM_ACTIVATION_TIME_US 100

/** Erase/program bits in EEPROM command register */
#define EEPROM_START_ERASE_PROGRAM 6

/** Program done status bit in EEPROM interrupt registers*/
#define EEPROM_PROG_DONE_STATUS_BIT (1 << 2)

/** Macro to convert an EEPROM offset to its row number */
#define EEPROM_OFFSET_TO_ROW(x) ((x) / EEPROM_ROW_SIZE)

/**
 * The offset in the EEPROM starting from where the contents are cashed, with @c 0 pointing to the start of the EEPROM
 * memory.
 * Must always point to the first byte of a row.
 */
__attribute__((section(".noinit")))
static int sCachedOffset;

/**
 * Buffer to hold the cached data.
 */
__attribute__ ((section(".noinit"))) __attribute__((aligned (2)))
static uint8_t sCachedData[EEPROM_ROW_SIZE];

/* ------------------------------------------------------------------------- */

/**
 * Copies the cached data - if any - from #sCachedData to EEPROM, then starts an erase & program operation a.k.a flush.
 * Returns when this is completed.
 * When called redundantly, this function is a void operation.
 */
static void CommitAndFlush(void)
{
    if (sCachedOffset >= 0) {
        /* Assumptions of code below */
        ASSERT(EEPROM_ROW_SIZE == sizeof(sCachedData)); /* The buffer size must be 1 EEPROM row. */
        ASSERT(((int)sCachedData & 1) == 0); /* The buffer must be 16-bit aligned. */
        ASSERT(sCachedOffset % EEPROM_ROW_SIZE == 0); /* Exactly 1 EEPROM row must have been cached. */

        /* Commit */
        uint16_t * src = (uint16_t *)sCachedData;
        uint16_t * dst = (uint16_t *)EEPROM_START + sCachedOffset / 2;
        for (int i = 0; i < EEPROM_ROW_SIZE; i += 2) {
            *dst = *src;
            dst++;
            src++;
        }

        /* Flush */
        NSS_EEPROM->INT_CLR_STATUS = EEPROM_PROG_DONE_STATUS_BIT;
        NSS_EEPROM->CMD = EEPROM_START_ERASE_PROGRAM;
        while ((NSS_EEPROM->INT_STATUS & EEPROM_PROG_DONE_STATUS_BIT) == 0) {
            ; /* wait */
        }

        sCachedOffset = -1;
    }
}

/**
 * Loops over these actions:
 * - Initialize #sCachedData if necessary
 * - Find the portion of @c pBuf that can be copied to #sCachedData
 * - Copies that portion to #sCachedData
 * - If not all data is copied, commits and flushed #sCachedData to EEPROM memory
 * - repeat until all data is copied
 * @param offset See #Chip_EEPROM_Write and #Chip_EEPROM_Memset
 * @param pBuf See #Chip_EEPROM_Write and #Chip_EEPROM_Memset
 * @param size See #Chip_EEPROM_Write and #Chip_EEPROM_Memset
 * @param singleValue If @c true, the same byte value pointed to by @c pBuf must be copied to the indicated memory
 *  region, i.e. memset.
 */
static void Write(int offset, const void * pBuf, int size, bool singleValue)
{
    /* Per the HW UM, the last five contain calibration and test data and are locked. Yet #EEPROM_NR_OF_RW_ROWS excludes
     * the top 6 rows.
     */
#if EEPROM_NR_OF_RW_ROWS != 58
    #error Incorrect assumption on the value of EEPROM_NR_OF_RW_ROWS
#endif
    /* All bytes must be written to a valid EEPROM region */
    ASSERT(offset >= 0);
    ASSERT(offset + size < EEPROM_ROW_SIZE * (EEPROM_NR_OF_RW_ROWS + 1));

    while (size > 0) {
        if (sCachedOffset < 0) { /* If nothing is cached */
            sCachedOffset = (offset / EEPROM_ROW_SIZE) * EEPROM_ROW_SIZE;
            memcpy(sCachedData, (uint8_t *)(EEPROM_START + sCachedOffset), (uint32_t)sizeof(sCachedData));
        }
        int relativeOffset = offset - sCachedOffset;

        /* Find the overlap with what's already cached.
         *   EEPROM: eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
         *   cached:               cccccccc
         * to write:                              wwwww     --> relativeOffset > sizeof(sCachedData)
         *      OR:                      wwwww
         *      OR:                  wwwww
         *      OR:              wwwww                                        --> relativeOffset < 0
         *      OR:     wwwww                                                 --> relativeOffset < 0
         */
        int partialSize = 0;
        if ((0 <= relativeOffset) && (relativeOffset < (int)sizeof(sCachedData))) {
            partialSize = (int)sizeof(sCachedData) - relativeOffset; /* the maximum value that can be copied */
            if (size <= partialSize) {
                partialSize = size;
            }
            if (singleValue) {
                memset(sCachedData + relativeOffset, *(uint8_t *)pBuf, (uint32_t)partialSize);
            }
            else {
                memcpy(sCachedData + relativeOffset, pBuf, (uint32_t)partialSize);
            }
        }

        /* Prepare the next iteration. */
        offset += partialSize;
        if (!singleValue) {
            pBuf = (uint8_t *)pBuf + partialSize;
        }
        size -= partialSize;
        /* commit and flush only if necessary */
        if (size > 0) {
            CommitAndFlush();
        }
    }
}

/* ------------------------------------------------------------------------- */

void Chip_EEPROM_Init(NSS_EEPROM_T *pEEPROM)
{
    (void)pEEPROM; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */

    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_EEPROM);
    Chip_SysCon_Peripheral_AssertReset(SYSCON_PERIPHERAL_RESET_EEPROM);
    Chip_SysCon_Peripheral_EnablePower(SYSCON_PERIPHERAL_POWER_EEPROM);

    /* Wait for the EEPROM to get ready for content access */
    Chip_Clock_System_BusyWait_us(EEPROM_ACTIVATION_TIME_US);

    Chip_SysCon_Peripheral_DeassertReset(SYSCON_PERIPHERAL_RESET_EEPROM);

    /* Set clock division factor, making it 'ceiling' by adding (EEPROM_CLOCK_FREQUENCY_HZ - 1).
     * This ensures the resulting reference clock will not exceed the specified maximum
     */
    int div = ((Chip_Clock_System_GetClockFreq() + (EEPROM_CLOCK_FREQUENCY_HZ - 1)) / EEPROM_CLOCK_FREQUENCY_HZ) - 1;
    if (div < 1) {
        div = 1; /* If divisor is set to 0, the EEPROM reference clock is disabled. So ensure it to be at least 1. */
    }
    NSS_EEPROM->CLKDIV = (uint32_t)div;

    sCachedOffset = -1;
}

void Chip_EEPROM_DeInit(NSS_EEPROM_T *pEEPROM)
{
    (void)pEEPROM; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */
    CommitAndFlush();
    Chip_SysCon_Peripheral_AssertReset(SYSCON_PERIPHERAL_RESET_EEPROM);
    Chip_SysCon_Peripheral_DisablePower(SYSCON_PERIPHERAL_POWER_EEPROM);
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_EEPROM);
}

void Chip_EEPROM_Flush(NSS_EEPROM_T *pEEPROM, bool wait)
{
    (void)pEEPROM; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */
    (void)wait; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */
    CommitAndFlush();
}

void Chip_EEPROM_Read(NSS_EEPROM_T *pEEPROM, int offset, void * pBuf, int size)
{
    (void)pEEPROM; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */

    /* All bytes must be read from a valid EEPROM region */
    ASSERT(offset >= 0);
    ASSERT(size > 0);
    ASSERT(offset + size <= EEPROM_ROW_SIZE * EEPROM_NR_OF_R_ROWS);

    memcpy(pBuf, (uint8_t *)(EEPROM_START + offset), (uint32_t)size);

    if (sCachedOffset >= 0) {
        /* Check if address ranges overlap */
        uint8_t * src;
        uint8_t * dst;
        if (sCachedOffset < offset) {
            /*  EEPROM: eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
             * to read:                              rrrrr
             *  cached:               cccccccc
             *      OR:                        cccccccc
             *      OR:                             cccccccc
             */
            src = sCachedData + (offset - sCachedOffset); /* S below */
            dst = pBuf; /* D below */
            /*  EEPROM: eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
             * to read:                              Drrrr
             *  cached:               cccccccc       S                                 --> out of bound
             *      OR:                        ccccccSc
             *      OR:                             cScccccc
             */
        }
        else {
            /*  EEPROM: eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
             * to read:                              rrrrr
             *  cached:                              cccccccc
             *      OR:                                cccccccc
             *      OR:                                            cccccccc
             */
            src = sCachedData; /* S below */
            dst = (uint8_t *)pBuf + (sCachedOffset - offset); /* D below */
            /*  EEPROM: eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
             * to read:                              Drrrr
             *  cached:                              Sccccccc
             * to read:                              rrDrr
             *  cached:                                Sccccccc
             * to read:                              rrrrr         D                   --> out of bound
             *  cached:                                            Sccccccc
             */
        }
        if ((src < sCachedData + sizeof(sCachedData)) && (dst < (uint8_t *)pBuf + size)) { /* Excluding out of bounds */
            int partialSize = (uint8_t *)pBuf + size - dst; /* Limit the size so it fits in the destination */
            if (partialSize > (int)sizeof(sCachedData)) {
                partialSize = (int)sizeof(sCachedData); /* Limit the size so it fits in the source */
            }
            memcpy(dst, src, (uint32_t)partialSize); /* Overwrite with cached data */
        }
    }
}

void Chip_EEPROM_Write(NSS_EEPROM_T *pEEPROM, int offset, const void * pBuf, int size)
{
    (void)pEEPROM; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */
    Write(offset, pBuf, size, false);
}

void Chip_EEPROM_Memset(NSS_EEPROM_T *pEEPROM, int offset, uint8_t pattern, int size)
{
    (void)pEEPROM; /* suppress [-Wunused-parameter]: argument no longer used but kept for compatibility. */
    Write(offset, &pattern, size, true);
}
