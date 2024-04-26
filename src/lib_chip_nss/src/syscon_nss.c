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

/* -------------------------------------------------------------------------
 * Defines
 * ------------------------------------------------------------------------- */

/* Highest possible address for the IVT location in Flash (in ARM architecture Flash always start at address 0) */
#define SYSCON_IVT_FLASH_ADDRESS_END   0x00007400

/* Defines valid range for the IVT location in RAM */
#define SYSCON_IVT_RAM_ADDRESS_START   0x10000000
#define SYSCON_IVT_RAM_ADDRESS_END     0x10001C00

/* -------------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------------- */

void Chip_SysCon_IVT_SetAddress(uint32_t address)
{
    /* Address must be on a 1kB boundary */
    ASSERT((address & 0x3FF) == 0);

    if (address <= SYSCON_IVT_FLASH_ADDRESS_END) {
        /* setting Flash offset in number of 1kB segments to bits 1:5 (implicit >>10 followed by <<1 results in >>9) */
        NSS_SYSCON->SYSMEMREMAP = (address >> 9);
    }
    else if ((address >= SYSCON_IVT_RAM_ADDRESS_START) && (address <= SYSCON_IVT_RAM_ADDRESS_END)) {
        /* setting bit 0 to 1 means that IVT is mapped to SRAM
         * setting SRAM offset in number of 1kB segments to bits 1:5 (implicit >>10 followed by <<1 results in >>9) */
        NSS_SYSCON->SYSMEMREMAP = 1 | ((address - SYSCON_IVT_RAM_ADDRESS_START) >> 9);
    }
    else {
        /* Address not in valid range */
        ASSERT(false);
    }
}

uint32_t Chip_SysCon_IVT_GetAddress(void)
{
    /* Store register content to ensure information consistency
     * This variable will only contain the absolute address at the end of the function - for now it is just the register content*/
    uint32_t temp = NSS_SYSCON->SYSMEMREMAP & 0x3F;

    /* bits 1:5 of register contain the offset in number of 1kB segments (implicit <<10 followed by >>1 results in <<9 to
     * translate into an offset in bytes) */
    temp <<= 9;

    /* if bit 0 (now bit 9) is 1 means that IVT is mapped to SRAM */
    if ((temp & 0x200) != 0) {
        /* If IVT is mapped to SRAM, add the SRAM start address to translate into an absolute address */
        temp += SYSCON_IVT_RAM_ADDRESS_START;
    }

    /* returned address must always be on 1kB boundary
     * In case IVT is mapped to SRAM, this also ensures that bit 9 is cleared */
    return temp & (~0x3FFu);
}

void Chip_SysCon_Peripheral_AssertReset(SYSCON_PERIPHERAL_RESET_T bitvector)
{
    /* The reset bits are active low - clear the bit to assert the respective reset */
    NSS_SYSCON->PRESETCTRL &= ~(bitvector & 0xFu);
}

void Chip_SysCon_Peripheral_DeassertReset(SYSCON_PERIPHERAL_RESET_T bitvector)
{
    /* The reset bits are active low - set the bit to deassert the respective reset */
    NSS_SYSCON->PRESETCTRL |= bitvector & 0xF;
}

void Chip_SysCon_Peripheral_EnablePower(SYSCON_PERIPHERAL_POWER_T bitvector)
{
    /* The register actually selects the powered down peripherals - clear the bit to enable the peripheral power */
    NSS_SYSCON->PDRUNCFG &= ~(bitvector & 0x3Fu);
}

void Chip_SysCon_Peripheral_DisablePower(SYSCON_PERIPHERAL_POWER_T bitvector)
{
    /* The register actually selects the powered down peripherals - set the bit to disable the peripheral power */
    NSS_SYSCON->PDRUNCFG |= bitvector & 0x3F;
}

void Chip_SysCon_Peripheral_SetPowerDisabled(SYSCON_PERIPHERAL_POWER_T bitvector)
{
    NSS_SYSCON->PDRUNCFG = bitvector & 0x3F;
}

SYSCON_PERIPHERAL_POWER_T Chip_SysCon_Peripheral_GetPowerDisabled(void)
{
    return (SYSCON_PERIPHERAL_POWER_T)(NSS_SYSCON->PDRUNCFG & 0x3F);
}

SYSCON_RESETSOURCE_T Chip_SysCon_Reset_GetSource(void)
{
    return (SYSCON_RESETSOURCE_T)(NSS_SYSCON->SYSRSTSTAT & 0xF);
}

void Chip_SysCon_Reset_ClearSource(void)
{
    NSS_SYSCON->SYSRSTSTAT = 0;
}

void Chip_SysCon_StartLogic_SetEnabledMask(SYSCON_STARTSOURCE_T mask)
{
    NSS_SYSCON->STARTERP0 = mask & 0x1FFF;
}

SYSCON_STARTSOURCE_T Chip_SysCon_StartLogic_GetEnabledMask(void)
{
    return (SYSCON_STARTSOURCE_T)(NSS_SYSCON->STARTERP0 & 0x1FFF);
}

SYSCON_STARTSOURCE_T Chip_SysCon_StartLogic_GetStatus(void)
{
    return (SYSCON_STARTSOURCE_T)(NSS_SYSCON->STARTSRP0 & 0x1FFF);
}

void Chip_SysCon_StartLogic_ClearStatus(SYSCON_STARTSOURCE_T flags)
{
    NSS_SYSCON->STARTRSRP0CLR = flags & 0x1FFF;
    /* Due to HW peculiarity, the flags have to be manually cleared by SW */
    NSS_SYSCON->STARTRSRP0CLR &= ~(flags & 0x1FFFu);
}

void Chip_SysCon_StartLogic_SetPIORisingEdge(SYSCON_STARTSOURCE_T bitvector)
{
    NSS_SYSCON->STARTAPRP0 = bitvector & 0x7FF;
}

SYSCON_STARTSOURCE_T Chip_SysCon_StartLogic_GetPIORisingEdge(void)
{
    return (SYSCON_STARTSOURCE_T)(NSS_SYSCON->STARTAPRP0 & 0x7FF);
}

uint32_t Chip_SysCon_GetDeviceID(void)
{
    return NSS_SYSCON->DEVICEID;
}
