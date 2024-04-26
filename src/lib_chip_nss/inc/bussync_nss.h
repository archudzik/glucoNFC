/*
 * Copyright 2014-2016 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __BUSSYNC_NSS_H_
#define __BUSSYNC_NSS_H_

/** @defgroup BUSSYNC_NSS bussync: Bus Synchronization driver
 * @ingroup DRV_NSS
 * The Bus Synchronization driver (BusSync) provides the API to safely and reliably access registers
 * of "slow blocks", i.e. hardware blocks that run on a slower clock than the APB Bus.
 *
 * @par Problem description:
 *  The peripheral registers (which are the interface to access the chip peripherals) are accessed through the APB Bus
 *  which is clocked at System Clock frequency. Most of the HW blocks are clocked by the same System Clock
 *  (or derivatives), however there are some HW blocks that have a clock with a significantly lower frequency.
 *  Typically these blocks are the ones that are active during a deep power down (e.g. the PMU and RTC).
 *  For these slow blocks, synchronization between the registers (which are accessed through the APB Bus at System
 *  Clock frequency) and the HW block internal logic is needed.
 *
 * @par Synchronization Mechanism:
 *  The synchronization mechanism consists of having periods where the high frequency domain (APB Bus) waits for the
 *  low frequency domain (peripheral block) to complete the register access. These synchronization waits are controlled
 *  by the signal of a synchronization register (ACCSTAT) that each slow HW block provides.
 *  - Register read procedure:
 *      -# Trigger the HW block for a register read 
 *      -# Perform the synchronization wait 
 *      -# Read the content of the register 
 *  - Register write procedure:
 *      -# Trigger the HW block for a register write 
 *      -# Perform the synchronization wait 
 *      -# The new register content is now assimilated by the HW block
 *
 * @par Preemption problem:
 *  It is possible that a given synchronized read or write procedure is preempted by a read or write to the same
 *  hardware block in an ISR or higher priority thread. This could result in incorrect synchronization waits which could
 *  lead to unreliable read or write procedures. To ensure correct synchronization, even when preempted, this driver
 *  must be passed an additional parameter referred to as "access counter".
 *
 * @par Usage:
 *  Safe and reliable reading and writing from/to synchronized registers can be done using, respectively, the
 *  #Chip_BusSync_ReadReg and #Chip_BusSync_WriteReg functions. In addition to the target register address, these
 *  functions also receive the address of the synchronization register ("ACCSTAT") as well as the address of the access
 *  counter. Each slow HW block must declare its own access counter: a volatile integer, which does not need an initial
 *  value.
 *
 * @par Example - Synchronized RTC register read and write:
 *  @snippet bussync_nss_example_1.c bussync_nss_example_1
 *
 * @note
 *  The syncing process, and thus a register read or write, takes time. The synchronization procedure
 *  has a worst-case delay of 3 slow clock cycles. For a slow clock of 32kHz, the worst case delay is nearly 100us (3 * 1/32k).
 *  Also note that if such a read or write is used in an ISR, the interrupt latency for all lower priority interrupts
 *  increases. At 8MHz, the 100us boils down to an increased latency of 800 cpu ticks.
 * @{
 */

/**
 * Write a value into a register of a slow HW block
 * @param pACCSTAT : pointer to the synchronization register (e.g. ACCSTAT) of the HW block whose register to write to
 * @param pAccessCounter : pointer to the access counter declared for the HW block whose register to write to
 * @param pReg : pointer to the register to write to
 * @param value : value to write to the register
 */
void Chip_BusSync_WriteReg(volatile const uint32_t *pACCSTAT, volatile int *pAccessCounter, __IO uint32_t *pReg, uint32_t value);


/**
 * Read a value from a register of a slow HW block
 * @param pACCSTAT : pointer to the synchronization register (e.g. ACCSTAT) of the HW block whose register to read from
 * @param pAccessCounter : pointer to the access counter declared for the HW block whose register to read from
 * @param pReg : pointer to the register to read from
 * @return value read from the register
 */
uint32_t Chip_BusSync_ReadReg(volatile const uint32_t *pACCSTAT, volatile int *pAccessCounter, __I uint32_t *pReg);


/**
 * @}
 */

#endif /* __BUSSYNC_NSS_H_ */
