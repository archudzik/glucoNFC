/*
 * Copyright 2014-2017 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"

/**
 * Maximum time (in ARM clock ticks) that a bus synchronization wait should take.
 * Worst-case wait is 3 TFRO ticks. Worst case ARM speed is full SFRO (no divider).
 * Since the TFRO could be somewhat slower than spec'd, and the SFRO somewhat faster,
 * we add 33% as margin - it is just for an emergency time-out.
 */
#define MAX_WAITTIME_SYSCLOCKTICKS (NSS_SFRO_FREQUENCY*(3+1)/NSS_TFRO_FREQUENCY)

/* ------------------------------------------------------------------------- */

/**
 * Waits for synchronization of the bus.
 * This function waits until the synchronization register (ACCSTAT) becomes asserted.
 * There is a timeout to guard against infinite looping, resulting in an assert.
 * @param pACCSTAT : pointer to the synchronization register
 */
static void Chip_BusSync_WaitSync(volatile const uint32_t *pACCSTAT)
{
    int tickswaited = 0;

    while ((0 == ((*pACCSTAT) & 0x1)) /* Test if register access is now possible */ &&
            (tickswaited < MAX_WAITTIME_SYSCLOCKTICKS) /* Timeout to prevent infinite loop */) {
        tickswaited += 6; /* 6 is the estimated worst-case (lowest) instruction/cycle count per while-loop */
    };

    /* Check if timeout occurred while waiting for register access to complete
     * (happens e.g. when device is not powered or clocked).
     */
    ASSERT((0 != ((*pACCSTAT) & 0x1)));
}

/* ------------------------------------------------------------------------- */

void Chip_BusSync_WriteReg(volatile const uint32_t *pACCSTAT, volatile int *pAccessCounter, __IO uint32_t *pReg,
                           uint32_t value)
{
    /* If this function preempts a lower priority task (e.g. an ISR preempting the main loop),
     * wait for access completion of the preempted task.
     */
    Chip_BusSync_WaitSync(pACCSTAT);

    (*pAccessCounter)++; /* Signal preempted task that the preempting task has done a register access */

    /* ========= Critical Section ======= */
    *pReg = value;
    Chip_BusSync_WaitSync(pACCSTAT); /* Wait for write to complete (so that hw is in new state when this function returns) */
    /* ========= Critical Section ======= */
}

uint32_t Chip_BusSync_ReadReg(volatile const uint32_t *pACCSTAT, volatile int *pAccessCounter, __I uint32_t *pReg)
{
    int localAccessCounter;
    uint32_t value;

    (*pAccessCounter)++; /* Signal preempted task that the preempting task has done a register access */

    do {
        /* If this function preempts a lower priority task (e.g. an ISR preempting the main loop)
         * wait for access completion of the preempted task.
         * If this read-cycle is preempted by a higher-priority context,
         * this sync wait ensures that access is available before retrying read register.
         */
        Chip_BusSync_WaitSync(pACCSTAT);

        localAccessCounter = (*pAccessCounter); /* Mark access counter (to check later if we are preempted) */

        /* ========= Critical Section ======= */
        value = *pReg;
        Chip_BusSync_WaitSync(pACCSTAT); /* Wait for read to complete (so that read value can be fetched) */
        value = *pReg;
        /* ========= Critical Section ======= */

        /* If the access counter has changed, this function was preempted, and we need to retry */
    } while (localAccessCounter != (*pAccessCounter));

    return value;
}
