/*
 * Copyright 2014-2018 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"
#include "string.h"

#define NFC_LAST_ACCESS_START_MASK (0xFF << 8) /*!< Start address mask for last RF access */
#define NFC_LAST_ACCESS_END_MASK (0xFF) /*!< End address mask for last RF access */
#define NFC_LAST_ACCESS_DIR_MASK (0x1 << 16) /*!< Direction mask for last RF access */
#define NFC_SHARED_MEM_PAGE_OFFSET 4 /*!< Page offset for shared memory */

static volatile uint32_t stickyMEM_WRITE = 0; /*!< Page offset for shared memory */

/* ------------------------------------------------------------------------- */

void Chip_NFC_Init(NSS_NFC_T *pNFC)
{
    pNFC->BUF[0] = 0x000000FE; /* Terminate shared RAM with a terminator TLV */
    Chip_NFC_Int_SetEnabledMask(NSS_NFC, NFC_INT_NONE);
    Chip_NFC_Int_ClearRawStatus(NSS_NFC, NFC_INT_ALL);

    /* Disabling bypass mode. */
    pNFC->CFG = 0x0;
}

void Chip_NFC_DeInit(NSS_NFC_T *pNFC)
{
    pNFC->BUF[0] = 0x000000FE; /* Terminate shared RAM with a terminator TLV */
    Chip_NFC_Int_SetEnabledMask(NSS_NFC, NFC_INT_NONE);
    Chip_NFC_Int_ClearRawStatus(NSS_NFC, NFC_INT_ALL);
}

NFC_STATUS_T Chip_NFC_GetStatus(NSS_NFC_T *pNFC)
{
    return (NFC_STATUS_T)(pNFC->SR & 0xFF);
}

void Chip_NFC_Int_SetEnabledMask(NSS_NFC_T *pNFC, NFC_INT_T mask)
{
    pNFC->IMSC = mask & NFC_INT_ALL;
}

NFC_INT_T Chip_NFC_Int_GetEnabledMask(NSS_NFC_T *pNFC)
{
    return (NFC_INT_T)(pNFC->IMSC & NFC_INT_ALL);
}

NFC_INT_T Chip_NFC_Int_GetRawStatus(NSS_NFC_T *pNFC)
{
    return (NFC_INT_T)(pNFC->RIS & NFC_INT_ALL);
}

void Chip_NFC_Int_ClearRawStatus(NSS_NFC_T *pNFC, NFC_INT_T flags)
{
    stickyMEM_WRITE |= (pNFC->RIS & NFC_INT_MEMWRITE);

    pNFC->IC = flags & NFC_INT_ALL;
    /* To ensure at least one other APB access to the RFID/NFC shared memory interface before exiting the ISR.
     * Refer to NFC chapter of User Manual */
    pNFC->IC = flags & NFC_INT_ALL;
}

void Chip_NFC_SetTargetAddress(NSS_NFC_T *pNFC, uint32_t offset)
{
    pNFC->TARGET = offset + NFC_SHARED_MEM_PAGE_OFFSET;
}

uint32_t Chip_NFC_GetTargetAddress(NSS_NFC_T *pNFC)
{
    return (uint32_t)(pNFC->TARGET - NFC_SHARED_MEM_PAGE_OFFSET);
}

bool Chip_NFC_GetLastAccessInfo(NSS_NFC_T *pNFC, uint32_t *pStartOffset, uint32_t *pEndOffset)
{
    *pStartOffset = ((pNFC->LAST_ACCESS & NFC_LAST_ACCESS_START_MASK) >> 8) - NFC_SHARED_MEM_PAGE_OFFSET;
    *pEndOffset = (pNFC->LAST_ACCESS & NFC_LAST_ACCESS_END_MASK) - NFC_SHARED_MEM_PAGE_OFFSET;
    return (bool)((pNFC->LAST_ACCESS & NFC_LAST_ACCESS_DIR_MASK) == NFC_LAST_ACCESS_DIR_MASK);
}

bool Chip_NFC_WordWrite(NSS_NFC_T *pNFC, uint32_t * pDest, const uint32_t * pSrc, int n)
{
    stickyMEM_WRITE = 0;

    pNFC->IC = NFC_INT_MEMWRITE;
    /* To ensure at least one other APB access to the RFID/NFC shared memory interface before exiting the ISR.
     * Refer to NFC chapter of User Manual. */
    pNFC->IC = NFC_INT_MEMWRITE;

    memcpy(pDest, pSrc, (uint32_t)n*4);

    if (((pNFC->RIS & NFC_INT_MEMWRITE) == NFC_INT_MEMWRITE) || stickyMEM_WRITE) {
        return false;
    }
    return true;
}

bool Chip_NFC_ByteRead(NSS_NFC_T *pNFC, uint8_t * pDest, const uint8_t * pSrc, int n)
{
    stickyMEM_WRITE = 0;

    pNFC->IC = NFC_INT_MEMWRITE;
    /* To ensure at least one other APB access to the RFID/NFC shared memory interface before exiting the ISR.
     * Refer to NFC chapter of User Manual. */
    pNFC->IC = NFC_INT_MEMWRITE;

    memcpy(pDest, pSrc, (uint32_t)n);

    if (((pNFC->RIS & NFC_INT_MEMWRITE) == NFC_INT_MEMWRITE) || stickyMEM_WRITE) {
        return false;
    }
    return true;
}
