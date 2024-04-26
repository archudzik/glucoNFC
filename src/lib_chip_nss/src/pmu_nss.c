/*
 * Copyright 2014-2019,2021 NXP
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

/* Bit definitions for Power Control register. */
#define PMU_PCON_DPENFLAG_POS 1
#define PMU_PCON_DPENFLAG_MASK (1u << PMU_PCON_DPENFLAG_POS)
#define PMU_PCON_SLEEPFLAG_POS 8
#define PMU_PCON_SLEEPFLAG_MASK (1u << PMU_PCON_SLEEPFLAG_POS)
#define PMU_PCON_DPDFLAG_POS 11
#define PMU_PCON_DPDFLAG_MASK (1u << PMU_PCON_DPDFLAG_POS)
#define PMU_PCON_PMULPMFLAG_POS 13
#define PMU_PCON_PMULPMFLAG_MASK (1u << PMU_PCON_PMULPMFLAG_POS)
#define PMU_PCON_VBATFLAG_POS 14
#define PMU_PCON_VBATFLAG_MASK (1u << PMU_PCON_VBATFLAG_POS)
#define PMU_PCON_BODENFLAG_POS 15
#define PMU_PCON_BODENFLAG_MASK (1u << PMU_PCON_BODENFLAG_POS)
#define PMU_PCON_FORCEVBATFLAG_POS 17
#define PMU_PCON_FORCEVBATFLAG_MASK (1u << PMU_PCON_FORCEVBATFLAG_POS)
#define PMU_PCON_FORCENFCFLAG_POS 18
#define PMU_PCON_FORCENFCFLAG_MASK (1u << PMU_PCON_FORCENFCFLAG_POS)
#define PMU_PCON_WAKEUPFLAG_POS 19
#define PMU_PCON_WAKEUPFLAG_MASK (1u << PMU_PCON_WAKEUPFLAG_POS)

/** Retained data section size in words */
#define PMU_RETAINED_DATA_SIZE 5

/** Access counter for BusSync (since RTC is a slow HW block). Used in #PMU_READ and #PMU_WRITE */
static volatile int Chip_PMU_AccessCounter;

/** Macro to read a PMU register: must be done synchronized since PMU is part of the slow RTC HW block. */
#define PMU_READ(pReg) Chip_BusSync_ReadReg(&NSS_PMU->ACCSTAT, &Chip_PMU_AccessCounter, (pReg))

/** Macro to write a PMU register: must be done synchronized since PMU is part of the slow RTC HW block. */
#define PMU_WRITE(pReg, value) Chip_BusSync_WriteReg(&NSS_PMU->ACCSTAT, &Chip_PMU_AccessCounter, (pReg), (value))

static void ModifyRegister(__IO uint32_t *pReg, uint32_t mask, uint32_t value);
static void PrepareLowPowerMode(uint32_t pconDpenMask, uint32_t pconPmulpmMask, uint32_t scrSleepdeepMask);

/* ------------------------------------------------------------------------- */

/**
 * Modifies selected bits of a PMU register: read-modify-write, using #PMU_READ and #PMU_WRITE for correctly accessing
 * the registers residing in the different clock domain.
 * @param pReg Pointer to PMU register. See #NSS_PMU_T
 * @param mask The bits to clear
 * @param value The bits to set
 * @note Both the SLEEPFLAG and DPDFLAG fields in the #NSS_PMU_T.PCON register are write-1-to-clear, with no effect when
 *  writing 0. They are therefore automatically added to the @c mask when modifying that register.
 */
static void ModifyRegister(__IO uint32_t *pReg, uint32_t mask, uint32_t value)
{
    if (pReg == &NSS_PMU->PCON) {
        mask |= PMU_PCON_SLEEPFLAG_MASK | PMU_PCON_DPDFLAG_MASK;
    }
    uint32_t content = PMU_READ(pReg);
    content = (content & (~mask)) | value;
    PMU_WRITE(pReg, content);
}

/**
 * Prepares the PCON PMU register and the SCB ARM register for Sleep, Deep sleep, or Deep power down mode.
 * @param pconDpenMask : Either equal to #PMU_PCON_DPENFLAG_MASK, or @c 0
 * @param pconPmulpmMask : Either equal to #PMU_PCON_PMULPMFLAG_MASK, or @c 0
 * @param scrSleepdeepMask : Either equal to #SCB_SCR_SLEEPDEEP_Msk, or @c 0
 */
static void PrepareLowPowerMode(uint32_t pconDpenMask, uint32_t pconPmulpmMask, uint32_t scrSleepdeepMask)
{
    /* Modify PCON register:
     * - DPEN: pconDpenMask
     * - SLEEPFLAG: clear -> 1
     * - DPDFLAG: clear -> 1
     * - PMULPM: pconPmulpmMask
     * - VBAT: retain
     * - BODEN: retain
     * - FORCEVBAT: retain
     * - FORCEVNFC: retain
     * - WAKEUP: retain
     */
    ModifyRegister(&NSS_PMU->PCON,
                   ~(PMU_PCON_VBATFLAG_MASK | PMU_PCON_BODENFLAG_MASK | PMU_PCON_FORCEVBATFLAG_MASK
                           | PMU_PCON_FORCENFCFLAG_MASK | PMU_PCON_WAKEUPFLAG_MASK),
                   pconDpenMask | PMU_PCON_SLEEPFLAG_MASK | PMU_PCON_DPDFLAG_MASK | pconPmulpmMask);

    /* Set SCB register in ARM core as required */
    SCB->SCR = (SCB->SCR & (~SCB_SCR_SLEEPDEEP_Msk)) | scrSleepdeepMask;
}

/* ------------------------------------------------------------------------- */

void Chip_PMU_PowerMode_EnterSleep(void)
{
    PrepareLowPowerMode(0, 0, 0);
    __WFI();
}

void Chip_PMU_PowerMode_EnterDeepSleep(void)
{
    PrepareLowPowerMode(0, 0, SCB_SCR_SLEEPDEEP_Msk);
    __WFI();
    /* After leaving Deep sleep, make sure a direct WFI instruction does not trigger a Deep sleep mode */
    PrepareLowPowerMode(0, 0, 0);
}

void Chip_PMU_PowerMode_EnterDeepPowerDown(bool enableSwitching)
{
#if defined(DEBUG)
    if (Chip_PMU_GetStatus() & PMU_STATUS_VDD_NFC) {
        Chip_Clock_System_BusyWait_ms(500);
    }
#endif
    PrepareLowPowerMode(PMU_PCON_DPENFLAG_MASK, (uint32_t)(enableSwitching == false) << PMU_PCON_PMULPMFLAG_POS, SCB_SCR_SLEEPDEEP_Msk);
    __WFI();
    /* In case Deep power down was not entered, make sure a direct WFI instruction does not trigger a delayed
     * Deep sleep or Deep power down.
     */
    PrepareLowPowerMode(0, 0, 0);
}

PMU_DPD_WAKEUPREASON_T Chip_PMU_PowerMode_GetDPDWakeupReason(void)
{
    uint32_t pcon = PMU_READ(&NSS_PMU->PCON);

    /* If both DPDFLAG and SLEEPFLAG are set, something went wrong */
    ASSERT(!(((pcon & PMU_PCON_DPDFLAG_MASK) != 0) && ((pcon & PMU_PCON_SLEEPFLAG_MASK) != 0)));

    if (((pcon & PMU_PCON_DPDFLAG_MASK) != 0) && ((pcon & PMU_PCON_SLEEPFLAG_MASK) == 0)) {
        return (PMU_READ(&NSS_PMU->PSTAT) & (0x3 << 3)) >> 3;
    }
    else {
        return PMU_DPD_WAKEUPREASON_NONE;
    }
}

bool Chip_PMU_Switch_GetVDDBat(void)
{
    return (PMU_READ(&NSS_PMU->PSTAT) & (1 << 1)) != 0;
}

bool Chip_PMU_Switch_GetVNFC(void)
{
    return (PMU_READ(&NSS_PMU->PSTAT) & (1 << 0)) != 0;
}

void Chip_PMU_Switch_OpenVDDBat(void)
{
    /* Modify PCON register - VBAT set to "force off" and back to "auto" */
    ModifyRegister(&NSS_PMU->PCON, PMU_PCON_VBATFLAG_MASK, PMU_PCON_VBATFLAG_MASK);
}

void Chip_PMU_SetBODEnabled(bool enabled)
{
    ModifyRegister(&NSS_PMU->PCON, PMU_PCON_BODENFLAG_MASK, (uint32_t)(enabled != 0) << PMU_PCON_BODENFLAG_POS);
}

bool Chip_PMU_GetBODEnabled(void)
{
    return (PMU_READ(&NSS_PMU->PCON) & PMU_PCON_BODENFLAG_MASK) != 0;
}

void Chip_PMU_SetWakeupPinEnabled(bool enabled)
{
    ModifyRegister(&NSS_PMU->PCON, PMU_PCON_WAKEUPFLAG_MASK, (uint32_t)(enabled != 0) << PMU_PCON_WAKEUPFLAG_POS);
}

bool Chip_PMU_GetWakeupPinEnabled(void)
{
    return (PMU_READ(&NSS_PMU->PCON) & PMU_PCON_WAKEUPFLAG_MASK) != 0;
}

void Chip_PMU_SetRTCClockSource(PMU_RTC_CLOCKSOURCE_T source)
{
    ModifyRegister(&NSS_PMU->TMRCLKCTRL, 1 << 0, (source & 0x1) << 0);
}

PMU_RTC_CLOCKSOURCE_T Chip_PMU_GetRTCClockSource(void)
{
    return (PMU_RTC_CLOCKSOURCE_T)(PMU_READ(&NSS_PMU->TMRCLKCTRL) & (1 << 0));
}

void Chip_PMU_SetRetainedData(uint32_t *pData, int offset, int size)
{
    ASSERT((size > 0) && (offset >= 0) && (offset + size <= PMU_RETAINED_DATA_SIZE));
    for (int i = 0; i < size; i++) {
        PMU_WRITE(&NSS_PMU->GPREG[i+offset], pData[i]);
    }
}

void Chip_PMU_GetRetainedData(uint32_t *pData, int offset, int size)
{
    ASSERT((size > 0) && (offset >= 0) && (offset + size <= PMU_RETAINED_DATA_SIZE));
    for (int i = 0; i < size; i++) {
        pData[i] = PMU_READ(&NSS_PMU->GPREG[i+offset]);
    }
}

PMU_STATUS_T Chip_PMU_GetStatus(void)
{
    return (PMU_STATUS_T)(PMU_READ(&NSS_PMU->PSTAT) & (PMU_STATUS_BROWNOUT | PMU_STATUS_VDD_NFC));
}

void Chip_PMU_Int_SetEnabledMask(PMU_INT_T mask)
{
    PMU_WRITE(&NSS_PMU->IMSC, mask & 0x7);
}

PMU_INT_T Chip_PMU_Int_GetEnabledMask(void)
{
    return (PMU_INT_T)(PMU_READ(&NSS_PMU->IMSC) & 0x7);
}

PMU_INT_T Chip_PMU_Int_GetRawStatus(void)
{
    return (PMU_INT_T)(PMU_READ(&NSS_PMU->RIS) & 0x7);
}

void Chip_PMU_Int_ClearRawStatus(PMU_INT_T flags)
{
    PMU_WRITE(&NSS_PMU->ICR, flags & 0x7u);
}

void Chip_PMU_SetAutoPowerEnabled(bool enabled)
{
    ModifyRegister(&NSS_PMU->PCON, PMU_PCON_PMULPMFLAG_MASK, (uint32_t)(enabled == false) << PMU_PCON_PMULPMFLAG_POS);
}

bool Chip_PMU_GetAutoPowerEnabled(void)
{
    return (PMU_READ(&NSS_PMU->PCON) & PMU_PCON_PMULPMFLAG_MASK) == 0;
}
