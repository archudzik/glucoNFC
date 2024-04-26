/*
 * Copyright 2014-2016,2018,2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __TIMER_NSS_H_
#define __TIMER_NSS_H_

/**
 * @defgroup TIMER_NSS timer: 16/32-bit Timer driver
 * @ingroup DRV_NSS
 * This driver provides APIs for the configuration and operation of both the 16-bit "CT16B" and 32-bit "CT32B"
 * counters/timers. The CT16B and CT32B HW blocks (16 and 32 bits timer/counter units) provide 16-bit and 32-bit
 * counters/timers function respectively.
 *
 * @par Introduction:
 *  This note describes the conceptual model of the timer hardware block. The timer hardware block has three extensions
 *  around its counter core: matching, external match control and pulse width modulation.
 *  Each of these are discussed next.
 *
 * @par Counter core:
 *  The core of the timer block is its counter. The counter register is called Timer Counter or TCR and is 32 bits
 *  (or 16 bits) wide. It is incremented when the prescaler of the timer expires, that is, when the Prescale Counter
 *  @ref NSS_TIMER_T.PC "PC" reaches the value in the Prescale Register @ref NSS_TIMER_T.PR "PR". The core has a fourth
 *  register, the Timer Control Register @ref NSS_TIMER_T.TCR "TCR",
 *  with two control bits: enabling counting respectively resetting (both @ref NSS_TIMER_T.TC "TC" and
 *  @ref NSS_TIMER_T.PC "PC").
 *  @dot
 *      digraph "Counter core" {
 *          node [shape=box];
 *          rankdir="BT"
 *          subgraph a {
 *              rank=same
 *              TCR             [label="TCR\nTimer control register", URL="\ref NSS_TIMER_T.TCR"]
 *              TC_up           [label="TC (up)\nTimer counter (upwards)", URL="\ref NSS_TIMER_T.TC"]
 *
 *              TCR -> TC_up    [label="control"]
 *          }
 *          subgraph b {
 *              rank=same
 *              PR              [label="PR\nPrescale Register", URL="\ref NSS_TIMER_T.PR"]
 *              x               [label="compare", shape=none]
 *              PC_up           [label="PC (up)\nPrescale Counter (upwards)", URL="\ref NSS_TIMER_T.PC"]
 *
 *              PR -> x         [dir=back]
 *              x -> PC_up
 *
 *          }
 *
 *          x -> TC_up          [style=dashed]
 *
 *      }
 *  @enddot
 *  The Prescale Counter @ref NSS_TIMER_T.PC "PC" counts cycles of the peripheral clock, which is the same as the
 *  system clock (towards the ARM).
 *
 * @par Matching:
 *  The first extension adds interrupt capabilities. There are 4 match registers @ref NSS_TIMER_T.MR "MR0",
 *  @ref NSS_TIMER_T.MR "MR1", @ref NSS_TIMER_T.MR "MR2" and @ref NSS_TIMER_T.MR "MR3" that are
 *  continuously compared against the Timer Counter @ref NSS_TIMER_T.TC "TC". When the Timer Counter
 *  @ref NSS_TIMER_T.TC "TC" matches @ref NSS_TIMER_T.MR "MRx", several actions can be performed by the hardware:
 *      - generates an interrupt towards the @ref CMSIS_NVIC
 *      - resets the counter to 0 (via reset bit in @ref NSS_TIMER_T.TCR "TCR")
 *      - disables the counter (via enable bit in @ref NSS_TIMER_T.TCR "TCR")
 *  The actions are programmed in the Match Control Register @ref NSS_TIMER_T.MCR "MCR". For each Match Register
 *  @ref NSS_TIMER_T.MR "MRx", the Match Control Register @ref NSS_TIMER_T.MCR "MCR" has three bits MRxI, MRxR and MRxS
 *  that enable the actions Interrupt, Reset and Stop respectively. The Interrupt Register IR records which of the match
 *  registers caused the interrupt.
 *  @dot
 *      digraph "Matching" {
 *          node [shape=box];
 *          rankdir="LR"
 *          subgraph a {
 *              rank=same
 *              TCR             [label="TCR\nTimer control register", URL="\ref NSS_TIMER_T.TCR"]
 *              x               [shape=point]
 *              IR              [label="IR\nInterrupt register", URL="\ref NSS_TIMER_T.IR"]
 *
 *              TCR -> x        [dir=back]
 *              x -> IR
 *          }
 *          subgraph b {
 *              rank=same
 *              TC_up           [label="TC (up)\nTimer counter (upwards)", URL="\ref NSS_TIMER_T.TC"]
 *              MCR             [label="MCR\nMatch Control Register", URL="\ref NSS_TIMER_T.MCR"]
 *          }
 *          compare             [shape=none]
 *          subgraph c {
 *              rank=same
 *              MR0             [label="MR0 - Match Register 0", URL="\ref NSS_TIMER_T.MR"]
 *              MR1             [label="MR1 - Match Register 1", URL="\ref NSS_TIMER_T.MR"]
 *              MR2             [label="MR2 - Match Register 2", URL="\ref NSS_TIMER_T.MR"]
 *              MR3             [label="MR3 - Match Register 3", URL="\ref NSS_TIMER_T.MR"]
 *          }
 *
 *          TCR -> TC_up
 *          x -> MCR            [label="control", style=dashed]
 *
 *          TC_up -> compare:nw [dir=back, style=dashed]
 *          MCR -> compare:sw   [dir=back, style=dashed]
 *
 *          compare -> MR0      [style=dashed]
 *          compare -> MR1      [style=dashed]
 *          compare -> MR2      [style=dashed]
 *          compare -> MR3      [style=dashed]
 *      }
 *  @enddot
 *
 * @par External Match Control:
 *  The timer hardware block has output lines: MATx. As usual, those outputs are routed through the @ref IOCON_NSS to
 *  optionally connect them to external pins (typically with a function name CT32B_MATx or CT16B_MATx).
 *
 *  The External Match Control extension allows controlling the MATx outputs based on the match registers. When the
 *  Timer Counter @ref NSS_TIMER_T.TC "TC" matches @ref NSS_TIMER_T.MR "MRx", the following actions can be performed
 *  by the hardware:
 *      - not changing MATx
 *      - clearing MATx
 *      - setting MATx
 *      - toggling MATx
 *  These actions are programmed in the External Match Register @ref NSS_TIMER_T.EMR "EMR". For each Match Register
 *  @ref NSS_TIMER_T.MR "MRx", the @ref NSS_TIMER_T.EMR "EMR" has a two bit External Match Control field EMCx with
 *  values:
 *      - 00b for not changing MATx
 *      - 01b for clearing MATx
 *      - 10b for setting MATx
 *      - and 11b for toggling MATx
 *  Also note that the @ref NSS_TIMER_T.EMR "EMR" has a one bit status field External Match EMx reflecting the state
 *  for each of the MATx outputs.
 *  @dot
 *      digraph "External Match Control" {
 *          node [shape=box];
 *          rankdir="LR"
 *          subgraph a {
 *              rank=same
 *              MAT0            [label="MAT0\nMatch output line 0"]
 *              MAT1            [label="MAT1\nMatch output line 1"]
 *              MAT2            [label="MAT2\nMatch output line 2"]
 *              MAT3            [label="MAT3\nMatch output line 3"]
 *          }
 *          control             [shape=none]
 *          subgraph b {
 *              EMR             [label="EMR\nExternal Match Register", URL="\ref NSS_TIMER_T.EMR"]
 *              TC_up           [label="TC (up)\nTimer counter (upwards)", URL="\ref NSS_TIMER_T.TC"]
 *          }
 *          compare             [shape=none]
 *          subgraph c {
 *              rank=same
 *              MR0             [label="MR0 - Match Register 0", URL="\ref NSS_TIMER_T.MR"]
 *              MR1             [label="MR1 - Match Register 1", URL="\ref NSS_TIMER_T.MR"]
 *              MR2             [label="MR2 - Match Register 2", URL="\ref NSS_TIMER_T.MR"]
 *              MR3             [label="MR3 - Match Register 3", URL="\ref NSS_TIMER_T.MR"]
 *          }
 *
 *          MAT0:e -> control   [dir=none, style=dotted]
 *          MAT1:e -> control   [dir=none, style=dotted]
 *          MAT2:e -> control   [dir=none, style=dotted]
 *          MAT3:e -> control   [dir=none, style=dotted]
 *          control -> EMR      [dir=none, style=dotted]
 *
 *          EMR -> compare:sw   [dir=back, style=dashed]
 *          TC_up -> compare:nw [dir=back, style=dashed]
 *
 *          compare -> MR0      [style=dashed]
 *          compare -> MR1      [style=dashed]
 *          compare -> MR2      [style=dashed]
 *          compare -> MR3      [style=dashed]
 *      }
 *  @enddot
 *  The External Match Control extension has a limited implementation: MAT2 and MAT3 are not supported, and thus
 *  @ref NSS_TIMER_T.EMR "EMR" does not have the EM2/EM3 status fields, nor the EMC2/EMC3 control fields.
 *
 *  Also note that there is only one MRx that can operate on MATx. As a result it is hard to implement PWM
 *  functionality: by programming EMCx to toggle we can make a 50% duty cycling signal on MATx. For better control use
 *  the Pulse Width Modulation feature of the timer.
 *
 * @par Pulse Width Modulation:
 *  Pulse Width Modulation PWM extension can drive the MATx registers instead of having the External Match Control
 *  extension drive the MATx pins. This is achieved by setting the PWM Enable bit PWMENx in the PWM Control register
 *  @ref NSS_TIMER_T.PWMC "PWMC".
 *
 *  The PWM extension drives MATx in a different way then the External Match Control extension. Basically,
 *  it starts by driving MATx low, until @ref NSS_TIMER_T.TC "TC" reached MRx, then it drives MATx high. When the
 *  Matching extension issues a reset (via MCR.MRyR) of @ref NSS_TIMER_T.TC "TC", MATx is cleared again and the whole
 *  process repeats. So, with the PWM extension the duty cycle is controlled independently (MRx) from the frequency (MRy).
 *
 *  Note that the PWM extension has a limited implementation similar to the External Match Control extension:
 *  MAT2 and MAT3 are not supported, and thus @ref NSS_TIMER_T.PWMC "PWMC" does not have the PWMEN2/PWMEN3 control fields.
 *  Each block features:
 *      -# One 16/32-bit Timer Counter with a programmable 16/32-bit Prescaler
 *      -# Four 16/32-bit Match Registers that allow:
 *          - Continuous operation with optional interrupt generation on match
 *          - Stop timer on match with optional interrupt generation
 *          - Reset timer on match with optional interrupt generation
 *      -# Up to two external outputs corresponding to the match registers with the following capabilities:
 *          - Set LOW on match
 *          - Set HIGH on match
 *          - Toggle on match
 *          - Do nothing on match
 *          - Single-edge controlled PWM outputs
 * @par Operation description of Timer Block:
 *  A Prescale Counter @ref NSS_TIMER_T.PC "PC" counts cycles of the peripheral clock and increments until it matches
 *  the Prescale Register @ref NSS_TIMER_T.PR "PR" value. On the next clock (i.e. NSS_TIMER_T.PR "PR" +1),
 *  the Timer Counter @ref NSS_TIMER_T.TC "TC" is incremented and the @ref NSS_TIMER_T.PC "PC" resets to zero.
 *
 *  The @ref NSS_TIMER_T.TC "TC" is then compared against each of the four Match Registers
 *  (@ref NSS_TIMER_T.MR "MR0, MR1, MR2, MR3"). When the value matches, the corresponding bits in Match Control Register
 *  @ref NSS_TIMER_T.MCR "MCR" and External Match Registers @ref NSS_TIMER_T.EMR "EMR" registers determine the actions
 *  to be executed. Configurable actions on match can be a combination of the following:
 *      - Generate interrupt on match
 *      - Reset TC on match
 *      - Stop TC and PC on match
 *      - No action on match
 *  Refer to timing chart in the user manual for detail.  Match registers can be independently set to generate interrupt
 *  on match, operate as configured by @ref NSS_TIMER_T.EMR "EMR" or in PWM output mode.
 *
 *  The External Match Registers @ref NSS_TIMER_T.EMR "EMR" determines if a match of @ref NSS_TIMER_T.TC "TC" and
 *  associated @ref NSS_TIMER_T.MR "MRn" shall either:
 *      - toggle
 *      - go LOW
 *      - go HIGH
 *      - do nothing with the External Match bits (EM0, EM1).
 *  For EM0 the @ref NSS_TIMER_T.TC "TC" is compared against NSS_TIMER_T.MR "MR0", and for EM1, the @ref
 *  NSS_TIMER_T.TC "TC" is compared against MR1. If the corresponding external match output pins are enabled, the
 *  EM0 state is reflected on the xxxxx_MAT0 output pin, while the EM1 state is reflected on the xxxxx_MAT1 output pin.
 *
 *  However, when the match output pins are configured as PWM output with #Chip_TIMER_SetMatchOutputMode, the function
 *  of the external match output pins are determined by single-edge controlled PWM mode.
 *  Up to two single-edge controlled PWM outputs can be selected on the xxxxx_MAT[0,1] outputs.
 *
 *  In PWM mode, PWM outputs go LOW at the beginning of each PWM cycle (timer is set to zero). PWM outputs goes HIGH
 *  when its match value is reached. The match register for PWM channel0 is NSS_TIMER_T.MR "MR0", while that for
 *  PWM channel1 is NSS_TIMER_T.MR "MR1". One additional match register is necessary. This match register resets the
 *  timer, thus determines the PWM cycle length. When the timer is reset to zero, all currently HIGH PWM match outputs
 *  are cleared (LOW).
 *
 * @par Example 1 - Increment a 10ms timed SW counter:
 *  - NSS_TIMER_T.PR "PR" : SystemClock / 500 (500Hz, 2ms per tick)
 *  - NSS_TIMER_T.MR "MR0" : 1 (5x 2ms)
 *  - Interrupt and reset on match are enabled; Stop-on-match disabled
 *
 *  @snippet timer_nss_example_1.c timer_nss_example_1
 *  Interrupt Handler:
 *  @snippet timer_nss_example_1.c timer_nss_example_1_irq
 *
 * @par Example 2 - Setup 50Hz, 50% duty-cycle External Match Output on 32-bit xxxxx_MAT0 output pin:
 *  Refer to user manual for actual pin names.
 *  - Prescale increment TC one tick every 2ms (500Hz)
 *  - NSS_TIMER_T.TC "TC" counts from 0 to 4 (10ms), matches NSS_TIMER_T.MR "MR0" and toggle xxxxx_MAT0 pin
 *      (Cycle Length is 20ms, or 50Hz output)
 *  - when NSS_TIMER_T.TC "TC" reaches 4, it resets to zero and repeats
 *  @snippet timer_nss_example_2.c timer_nss_example_2
 *
 * @par Example 3 - Setup 2Hz PWM on both xxxxx_MAT0 and xxxxx_MAT1 output pins.
 *  Refer to user manual for actual pin names. PWMEN0 and PWMEN1 are enabled with NSS_TIMER_T.MR "MR0" at 15/125
 *      (88% duty cycle), NSS_TIMER_T.MR "MR1" at 75/125 (40% duty-cycle) and NSS_TIMER_T.MR "MR2"
 *      (with reset on match set) at 124.
 *  - xxxxx_MAT0 & xxxxx_MAT1 are both set LOW at start.
 *  - NSS_TIMER_T.TC "TC" counts until 15 and raise-HIGH xxxxx_MAT0
 *  - NSS_TIMER_T.TC "TC" continue counting until 75 and raise-HIGH xxxxx_MAT1, and
 *  - when NSS_TIMER_T.TC "TC" reaches 124, both xxxxx_MAT0 and xxxxx_MAT1 are set low, TC reset to zero and repeats
 *  @snippet timer_nss_example_3.c timer_nss_example_3
 *
 * @{
 */

/** 16/32-bit Timer register block structure */
typedef struct NSS_TIMER_S {
    __IO uint32_t IR; /*!< Interrupt Register. The IR can be written to clear interrupts. The IR can be read to
                          identify which of eight possible interrupt sources are pending. */
    __IO uint32_t TCR; /*!< Timer Control Register. The TCR is used to control the Timer Counter functions.
                          The Timer Counter can be disabled or reset through the TCR. */
    __IO uint32_t TC; /*!< Timer Counter. This register is incremented every PR+1 cycles of PCLK.
                         The TC is controlled through the TCR. */
    __IO uint32_t PR; /*!< Prescale Register. When the Prescale Counter (below) is equal to this value, the next clock
                         increments the TC and clears the PC. */
    __IO uint32_t PC; /*!< Prescale Counter. The PC is a counter which is incremented to the value stored in PR. When
                         the value in PR is reached, the TC is incremented and the PC is cleared. The PC is observable
                         and controllable through the bus interface. */
    __IO uint32_t MCR; /*!< Match Control Register. The MCR is used to control if an interrupt is generated and if the
                          TC is reset when a Match occurs. */
    __IO uint32_t MR[4]; /*!< Match Register. MR can be enabled through the MCR to reset the TC, stop both the
                            TC and PC, and/or generate an interrupt every time MR matches the TC. */
    __I  uint32_t RESERVED0[5]; /*  Next address is 0x03C */
    __IO uint32_t EMR; /*!< External Match Register. The EMR controls the external match pins MATn.0-3
                          (MAT0.0-3 and MAT1.0-3 respectively). */
    __I  uint32_t RESERVED1[13]; /*  0x040 - 0x070. Next address 0x074 */
    __IO uint32_t PWMC; /*!< The PWMC enables PWM mode of the external match pins */
} NSS_TIMER_T;

/** Standard timer initial match pin state and change state */
typedef enum IP_TIMER_PIN_MATCH_STATE {
    TIMER_EXTMATCH_DO_NOTHING = 0, /*!< Timer match state does nothing on match pin */
    TIMER_EXTMATCH_CLEAR      = 1, /*!< Timer match state sets match pin low */
    TIMER_EXTMATCH_SET        = 2, /*!< Timer match state sets match pin high */
    TIMER_EXTMATCH_TOGGLE     = 3  /*!< Timer match state toggles match pin */
} TIMER_PIN_MATCH_STATE_T;

/** Operating mode of external output pins */
typedef enum NSS_TIMER_MATCH_OUTPUT_MODE {
    TIMER_MATCH_OUTPUT_EMC, /*!< External Match Control mode */
    TIMER_MATCH_OUTPUT_PWM /*!< PWM mode */
} NSS_TIMER_MATCH_OUTPUT_MODE_T;

/** Macro to clear interrupt pending */
#define TIMER_IR_CLR(n)         (1u << (n))

/** Macro for getting a timer match interrupt bit */
#define TIMER_MATCH_INT(n)      ((1u << (n)) & 0x0F)

/** Timer/counter enable bit */
#define TIMER_ENABLE            ((1u << 0))
/** Timer/counter reset bit */
#define TIMER_RESET             ((1u << 1))

/** Bit location for interrupt on MRx match, n = 0 to 3 */
#define TIMER_INT_ON_MATCH(n)   (1u << ((n) * 3))
/** Bit location for reset on MRx match, n = 0 to 3 */
#define TIMER_RESET_ON_MATCH(n) (1u << (((n) * 3) + 1))
/** Bit location for stop on MRx match, n = 0 to 3 */
#define TIMER_STOP_ON_MATCH(n) (1u << (((n) * 3) + 2))

/**
 * Initialize a timer
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param clk : Associated peripheral clock
 */
void Chip_TIMER_Init(NSS_TIMER_T *pTMR, CLOCK_PERIPHERAL_T clk);

/**
 * Shutdown a timer
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param clk : Associated peripheral clock
 */
void Chip_TIMER_DeInit(NSS_TIMER_T *pTMR, CLOCK_PERIPHERAL_T clk);

/**
 * Determines if the match interrupt for the passed timer and match counter is pending.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match interrupt number to check
 * @return @c true if the interrupt is pending, @c false otherwise
 */
static inline bool Chip_TIMER_MatchPending(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    return (bool) ((pTMR->IR & TIMER_MATCH_INT(matchnum)) != 0);
}

/**
 * Clears a (pending) match interrupt
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match interrupt number to clear
 * @note Clears a pending timer match interrupt.
 */
static inline void Chip_TIMER_ClearMatch(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->IR = TIMER_IR_CLR(matchnum);
}

/**
 * Enables the timer (starts count)
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @note Enables the timer to start counting.
 */
static inline void Chip_TIMER_Enable(NSS_TIMER_T *pTMR)
{
    pTMR->TCR |= TIMER_ENABLE;
}

/**
 * Disables the timer (stops count)
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @note Disables the timer to stop counting.
 */
static inline void Chip_TIMER_Disable(NSS_TIMER_T *pTMR)
{
    pTMR->TCR &= ~TIMER_ENABLE;
}

/**
 * Returns the current timer count
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @return Current timer terminal count value
 * @note Returns the current timer terminal count.
 */
static inline uint32_t Chip_TIMER_ReadCount(NSS_TIMER_T *pTMR)
{
    return pTMR->TC;
}

/**
 * Returns the current prescale count
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @return Current timer prescale count value
 * @note Returns the current prescale count.
 */
static inline uint32_t Chip_TIMER_ReadPrescale(NSS_TIMER_T *pTMR)
{
    return pTMR->PC;
}

/**
 * Sets the prescaler value
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param prescale : Prescale value to set the prescale register to
 * @note Sets the prescale count value.
 */
static inline void Chip_TIMER_PrescaleSet(NSS_TIMER_T *pTMR, uint32_t prescale)
{
    pTMR->PR = prescale;
}

/**
 * Sets a timer match value
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer to set match count for
 * @param matchval : Match value for the selected match count
 * @note Sets one of the timer match values.
 */
static inline void Chip_TIMER_SetMatch(NSS_TIMER_T *pTMR, int8_t matchnum, uint32_t matchval)
{
    pTMR->MR[matchnum] = matchval;
}

/**
 * Resets the timer terminal and prescale counts to 0
 * @param pTMR : The base address of the TIMER peripheral on the chip
 */
void Chip_TIMER_Reset(NSS_TIMER_T *pTMR);

/**
 * Enables a match interrupt that fires when the terminal count
 *          matches the match counter value.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer, 0 to 3
 */
static inline void Chip_TIMER_MatchEnableInt(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->MCR |= TIMER_INT_ON_MATCH(matchnum);
}

/**
 * Disables a match interrupt for a match counter.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer, 0 to 3
 */
static inline void Chip_TIMER_MatchDisableInt(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->MCR &= ~TIMER_INT_ON_MATCH(matchnum);
}

/**
 * For the specific match counter, enables reset of the terminal count register when a match occurs
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer, 0 to 3
 */
static inline void Chip_TIMER_ResetOnMatchEnable(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->MCR |= TIMER_RESET_ON_MATCH(matchnum);
}

/**
 * For the specific match counter, disables reset of the terminal count register when a match occurs
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer, 0 to 3
 */
static inline void Chip_TIMER_ResetOnMatchDisable(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->MCR &= ~TIMER_RESET_ON_MATCH(matchnum);
}

/**
 * Enable a match timer to stop the terminal count when a match count equals the terminal count.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer, 0 to 3
 */
static inline void Chip_TIMER_StopOnMatchEnable(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->MCR |= TIMER_STOP_ON_MATCH(matchnum);
}

/**
 * Disable stop on match for a match timer. Disables a match timer to stop the terminal count
 * when a match count equals the terminal count.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : Match timer, 0 to 3
 */
static inline void Chip_TIMER_StopOnMatchDisable(NSS_TIMER_T *pTMR, int8_t matchnum)
{
    pTMR->MCR &= ~TIMER_STOP_ON_MATCH(matchnum);
}

/**
 * Sets external match control (MATn.matchnum) pin control. For the pin selected with matchnum,
 * sets the function of the pin that occurs on a terminal count match for the match count.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param initial_state : Initial state of the pin, high(1) or low(0)
 * @param matchState : Selects the match state for the pin
 * @param matchnum : MATn.matchnum signal to use. 0: Ext match 0, 1: Ext match 1
 * @note For the pin selected with matchnum, sets the function of the pin that occurs
 *  a terminal count match for the match count. PWM functionality must be disabled for the corresponding
 *  external pin, see #Chip_TIMER_SetMatchOutputMode.
 */
void Chip_TIMER_ExtMatchControlSet(NSS_TIMER_T *pTMR, int8_t initial_state,
                                   TIMER_PIN_MATCH_STATE_T matchState, int8_t matchnum);

/**
 * Sets PWM mode of external match pin.
 * Each of the two external match outputs (xxxx_MAT[0,1] or xxxx_MAT[0,1]) can be independently set to
 * perform either as PWM output or as match output whose function is controlled by the External Match Register @ref
 * NSS_TIMER_T.EMR "EMR". If PWM function is disabled, the function of the pin will be controlled by the timer external
 * match control. See detailed description and example above for details. In the user manual, this function sets the
 * PWM control register @ref NSS_TIMER_T.PWMC "PWMC". To drive the output pins, @ref IOCON_NSS has to be configured
 * to CT16/CT32 function.
 * @param pTMR : The base address of the TIMER peripheral on the chip.
 * @param matchnum : 0: xxxx_MAT0, 1: xxxx_MAT1
 * @param mode : The output mode to be set, see #NSS_TIMER_MATCH_OUTPUT_MODE_T
 */
void Chip_TIMER_SetMatchOutputMode (NSS_TIMER_T *pTMR, int matchnum, NSS_TIMER_MATCH_OUTPUT_MODE_T mode );

/**
 * Get external match output mode. This returns status of PWM control registers.
 * @param pTMR : The base address of the TIMER peripheral on the chip
 * @param matchnum : 0: xxxxx_MAT0, 1: xxxxx_MAT1
 * @return Current external match output mode
 */
NSS_TIMER_MATCH_OUTPUT_MODE_T Chip_TIMER_GetMatchOutputMode(NSS_TIMER_T *pTMR, int matchnum );

#endif /** @} */
