/*
 * Copyright 2014-2019,2021 NXP
 * NXP confidential
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "chip.h"

static void Write2BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);
static void Write1BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);
static void Read2BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);
static void Read1BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup);

/* ------------------------------------------------------------------------- */

/** Write 2 bytes to FIFO buffer */
static void Write2BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    if (xf_setup->tx_data) {
        Chip_SSP_SendFrame(pSSP, (*(uint16_t *)((uint32_t)xf_setup->tx_data + xf_setup->tx_cnt)));
    }
    else {
        Chip_SSP_SendFrame(pSSP, 0xFFFF);
    }

    xf_setup->tx_cnt += 2;
}

/** Write 1 byte to FIFO buffer */
static void Write1BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    if (xf_setup->tx_data) {
        Chip_SSP_SendFrame(pSSP, (*(uint8_t *)((uint32_t)xf_setup->tx_data + xf_setup->tx_cnt)));
    }
    else {
        Chip_SSP_SendFrame(pSSP, 0xFF);
    }

    xf_setup->tx_cnt++;
}

/** Read continually 2 bytes from FIFO buffer */
static void Read2BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    uint16_t rDat;

    while ((Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE) == SET) && (xf_setup->rx_cnt < xf_setup->length)) {
        rDat = Chip_SSP_ReceiveFrame(pSSP);
        if (xf_setup->rx_data) {
            *(uint16_t *)((uint32_t)xf_setup->rx_data + xf_setup->rx_cnt) = rDat;
        }

        xf_setup->rx_cnt += 2;
    }
}

/** Read continually 1 byte from FIFO buffer */
static void Read1BFifo(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    uint16_t rDat;

    while ((Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE) == SET) && (xf_setup->rx_cnt < xf_setup->length)) {
        rDat = Chip_SSP_ReceiveFrame(pSSP);
        if (xf_setup->rx_data) {
            *(uint8_t *)((uint32_t)xf_setup->rx_data + xf_setup->rx_cnt) = (uint8_t)rDat;
        }

        xf_setup->rx_cnt++;
    }
}

/* ------------------------------------------------------------------------- */

void Chip_SSP_SetClockRate(NSS_SSP_T *pSSP, uint32_t clk_rate, uint32_t prescale)
{
    uint32_t temp = pSSP->CR0 & (~(SSP_CR0_SCR(0xFF)));
    pSSP->CR0 = temp | (SSP_CR0_SCR(clk_rate));
    pSSP->CPSR = prescale;
}

uint32_t Chip_SSP_RWFrames_Blocking(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    /* Clear all remaining frames in RX FIFO */
    while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE)) {
        Chip_SSP_ReceiveFrame(pSSP);
    }

    Chip_SSP_ClearIntPending(pSSP, SSP_INT_CLEAR_BITMASK);

    if (Chip_SSP_GetDataSize(pSSP) > SSP_BITS_8) {
        while (xf_setup->rx_cnt < xf_setup->length || xf_setup->tx_cnt < xf_setup->length) {
            /* write data to buffer */
            if ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF) == SET) && (xf_setup->tx_cnt < xf_setup->length)) {
                Write2BFifo(pSSP, xf_setup);
            }

            /* Check overrun error */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /* Check for any data available in RX FIFO */
            Read2BFifo(pSSP, xf_setup);
        }
    }
    else {
        while (xf_setup->rx_cnt < xf_setup->length || xf_setup->tx_cnt < xf_setup->length) {
            /* write data to buffer */
            if ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF) == SET) && (xf_setup->tx_cnt < xf_setup->length)) {
                Write1BFifo(pSSP, xf_setup);
            }

            /* Check overrun error */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /* Check for any data available in RX FIFO */
            Read1BFifo(pSSP, xf_setup);
        }
    }
    if (xf_setup->tx_data) {
        return xf_setup->tx_cnt;
    }
    else if (xf_setup->rx_data) {
        return xf_setup->rx_cnt;
    }

    return 0;
}

uint32_t Chip_SSP_WriteFrames_Blocking(NSS_SSP_T *pSSP, uint8_t *buffer, uint32_t buffer_len)
{
    uint32_t tx_cnt = 0;
    uint32_t rx_cnt = 0;

    /* Clear all remaining frames in RX FIFO */
    while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE)) {
        Chip_SSP_ReceiveFrame(pSSP);
    }

    Chip_SSP_ClearIntPending(pSSP, SSP_INT_CLEAR_BITMASK);

    if (Chip_SSP_GetDataSize(pSSP) > SSP_BITS_8) {
        uint16_t *wdata16 = (uint16_t *) buffer;

        while (tx_cnt < buffer_len || rx_cnt < buffer_len) {
            /* write data to buffer */
            if ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF) == SET) && (tx_cnt < buffer_len)) {
                Chip_SSP_SendFrame(pSSP, *wdata16);
                wdata16++;
                tx_cnt += 2;
            }

            /* Check overrun error */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /* Check for any data available in RX FIFO */
            while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE) == SET) {
                Chip_SSP_ReceiveFrame(pSSP); /* read dummy data */
                rx_cnt += 2;
            }
        }
    }
    else {
        uint8_t *wdata8 = buffer;
        while (tx_cnt < buffer_len || rx_cnt < buffer_len) {
            /* write data to buffer */
            if ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF) == SET) && (tx_cnt < buffer_len)) {
                Chip_SSP_SendFrame(pSSP, *wdata8);
                wdata8++;
                tx_cnt++;
            }

            /* Check overrun error */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /* Check for any data available in RX FIFO */
            while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE) == SET && rx_cnt < buffer_len) {
                Chip_SSP_ReceiveFrame(pSSP); /* read dummy data */
                rx_cnt++;
            }
        }
    }

    return tx_cnt;
}

uint32_t Chip_SSP_ReadFrames_Blocking(NSS_SSP_T *pSSP, uint8_t *buffer, uint32_t buffer_len)
{
    uint32_t rx_cnt = 0;
    uint32_t tx_cnt = 0;

    /* Clear all remaining frames in RX FIFO */
    while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE)) {
        Chip_SSP_ReceiveFrame(pSSP);
    }

    Chip_SSP_ClearIntPending(pSSP, SSP_INT_CLEAR_BITMASK);

    if (Chip_SSP_GetDataSize(pSSP) > SSP_BITS_8) {
        uint16_t *rdata16 = (uint16_t *)buffer;

        while (tx_cnt < buffer_len || rx_cnt < buffer_len) {
            /* write data to buffer */
            if ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF) == SET) && (tx_cnt < buffer_len)) {
                Chip_SSP_SendFrame(pSSP, 0xFFFF); /* just send dummy data */
                tx_cnt += 2;
            }

            /* Check overrun error */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /* Check for any data available in RX FIFO */
            while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE) == SET && rx_cnt < buffer_len) {
                *rdata16 = Chip_SSP_ReceiveFrame(pSSP);
                rdata16++;
                rx_cnt += 2;
            }
        }
    }
    else {
        uint8_t *rdata8 = buffer;
        while (tx_cnt < buffer_len || rx_cnt < buffer_len) {
            /* write data to buffer */
            if ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF) == SET) && (tx_cnt < buffer_len)) {
                Chip_SSP_SendFrame(pSSP, 0xFF); /* just send dummy data */
                tx_cnt++;
            }

            /* Check overrun error */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /* Check for any data available in RX FIFO */
            while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE) == SET && rx_cnt < buffer_len) {
                *rdata8 = (uint8_t)Chip_SSP_ReceiveFrame(pSSP);
                rdata8++;
                rx_cnt++;
            }
        }
    }

    return rx_cnt;
}

void Chip_SSP_Int_FlushData(NSS_SSP_T *pSSP)
{
    while (Chip_SSP_GetStatus(pSSP, SSP_STAT_BSY)) {
        ;
    }

    /* Clear all remaining frames in RX FIFO */
    while (Chip_SSP_GetStatus(pSSP, SSP_STAT_RNE)) {
        Chip_SSP_ReceiveFrame(pSSP);
    }

    Chip_SSP_ClearIntPending(pSSP, SSP_INT_CLEAR_BITMASK);
}

Status Chip_SSP_Int_RWFrames8Bits(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    /* Check overrun error in RIS register */
    if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
        return ERROR;
    }

    if ((xf_setup->tx_cnt != xf_setup->length) || (xf_setup->rx_cnt != xf_setup->length)) {
        /* check if RX FIFO contains data */
        Read1BFifo(pSSP, xf_setup);

        while ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF)) && (xf_setup->tx_cnt != xf_setup->length)) {
            /* Write data to buffer */
            Write1BFifo(pSSP, xf_setup);

            /* Check overrun error in RIS register */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /*  Check for any data available in RX FIFO */
            Read1BFifo(pSSP, xf_setup);
        }

        return SUCCESS;
    }

    return ERROR;
}

Status Chip_SSP_Int_RWFrames16Bits(NSS_SSP_T *pSSP, Chip_SSP_DATA_SETUP_T *xf_setup)
{
    /* Check overrun error in RIS register */
    if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
        return ERROR;
    }

    if ((xf_setup->tx_cnt != xf_setup->length) || (xf_setup->rx_cnt != xf_setup->length)) {
        /* check if RX FIFO contains data */
        Read2BFifo(pSSP, xf_setup);

        while ((Chip_SSP_GetStatus(pSSP, SSP_STAT_TNF)) && (xf_setup->tx_cnt != xf_setup->length)) {
            /* Write data to buffer */
            Write2BFifo(pSSP, xf_setup);

            /* Check overrun error in RIS register */
            if (Chip_SSP_GetRawIntStatus(pSSP, SSP_RORRIS) == SET) {
                return ERROR;
            }

            /*  Check for any data available in RX FIFO */
            Read2BFifo(pSSP, xf_setup);
        }

        return SUCCESS;
    }

    return ERROR;
}

void Chip_SSP_SetMaster(NSS_SSP_T *pSSP, bool master)
{
    if (master) {
        Chip_SSP_Set_Mode(pSSP, SSP_MODE_MASTER);
    }
    else {
        Chip_SSP_Set_Mode(pSSP, SSP_MODE_SLAVE);
    }
}

void Chip_SSP_SetBitRate(NSS_SSP_T *pSSP, uint32_t bitRate)
{
    uint32_t ssp_clk = (uint32_t)Chip_Clock_SPI0_GetClockFreq();
    uint32_t cr0_div = 0;
    uint32_t cmp_clk = 0xFFFFFFFF;
    uint32_t prescale = 2;

    while (cmp_clk > bitRate) {
        cmp_clk = ssp_clk / ((cr0_div + 1) * prescale);
        if (cmp_clk > bitRate) {
            cr0_div++;
            if (cr0_div > 0xFF) {
                cr0_div = 0;
                prescale += 2;
            }
        }
    }

    Chip_SSP_SetClockRate(pSSP, cr0_div, prescale);
}

uint32_t Chip_SSP_GetBitRate(NSS_SSP_T *pSSP)
{
    uint32_t serialClockRate = (pSSP->CR0 >> 8) & 0xFF;
    uint32_t prescaler = pSSP->CPSR;
    uint32_t sysClk = (uint32_t)Chip_Clock_System_GetClockFreq();
    uint32_t bitrate = (uint32_t)(sysClk / (prescaler * (serialClockRate + 1.0)));
    return bitrate;
}

void Chip_SSP_Init(NSS_SSP_T *pSSP)
{
    ASSERT(pSSP == NSS_SSP0);
    Chip_Clock_Peripheral_EnableClock(CLOCK_PERIPHERAL_SPI0);
    /* The SPI0 Clock divisor needs to be set to the same value as the System Clock divisor to prevent synchronization issues */
    Chip_Clock_SPI0_SetClockDiv(Chip_Clock_System_GetClockDiv());
    Chip_SysCon_Peripheral_DeassertReset(SYSCON_PERIPHERAL_RESET_SPI0);
    Chip_SSP_Set_Mode(pSSP, SSP_MODE_MASTER);
    Chip_SSP_SetFormat(pSSP, SSP_BITS_8, SSP_FRAME_FORMAT_SPI, SSP_CLOCK_CPHA0_CPOL0);
    Chip_SSP_SetBitRate(pSSP, 100000);
}

void Chip_SSP_DeInit(NSS_SSP_T *pSSP)
{
    ASSERT(pSSP == NSS_SSP0);
    Chip_SSP_Disable(pSSP);
    Chip_Clock_Peripheral_DisableClock(CLOCK_PERIPHERAL_SPI0);
    Chip_Clock_SPI0_SetClockDiv(0);
}
