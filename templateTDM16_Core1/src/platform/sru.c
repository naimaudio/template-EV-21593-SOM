// sru.c — Signal Routing Unit configuration

#include <SRU.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"

void ConfigureSru(void)
{
    /* Enable DAI pin input buffers (recommended) */
    *pREG_PADS0_DAI0_IE = 0x1FFFFF;
    *pREG_PADS0_DAI1_IE = 0x1FFFFF;

    /* ================================================================
     * DAI1: DAC is TDM master (SPORT4A/SPORT4B + forward clocks to ADC)
     *   - DAC BCLK on DAI1_PB05
     *   - DAC FS   on DAI1_PB04
     * ================================================================ */

    /* Listen to incoming clocks from DAC */
    SRU2(LOW, DAI1_PBEN05_I);   /* PB05 is input: DAC drives BCLK */
    SRU2(LOW, DAI1_PBEN04_I);   /* PB04 is input: DAC drives FS   */

    /* Route DAC clocks to SPORT4A and SPORT4B */
    SRU2(DAI1_PB05_O, SPT4_ACLK_I);
    SRU2(DAI1_PB05_O, SPT4_BCLK_I);

    SRU2(DAI1_PB04_O, SPT4_AFS_I);
    SRU2(DAI1_PB04_O, SPT4_BFS_I);

    /* Forward DAC clocks to ADC clock pins (ADC is slave) */
    SRU2(HIGH, DAI1_PBEN12_I);
    SRU2(DAI1_PB05_O, DAI1_PB12_I);   /* BCLK -> ADC BCLK */

    SRU2(HIGH, DAI1_PBEN20_I);
    SRU2(DAI1_PB04_O, DAI1_PB20_I);   /* FS -> ADC FS/LRCLK */

    /* ADC data -> SPORT4B (RX) */
    SRU2(LOW, DAI1_PBEN06_I);
    SRU2(DAI1_PB06_O, SPT4_BD0_I);

    /* SPORT4A data -> DAC (TX) */
    SRU2(HIGH, DAI1_PBEN01_I);
    SRU2(SPT4_AD0_O, DAI1_PB01_I);

    /* ================================================================
     * DAI0: Use PCGs for SPORT0 / ASRC / SPDIF clocks
     * AND feed PCG external clock inputs from DAC BCLK (cross-DAI source)
     *
     * NOTE:
     * - SRU() programs DAI0 SRU, SRU2() programs DAI1 SRU. [1](https://open4tech.com/direct-memory-access-dma-in-embedded-systems/)[5](https://www.analog.com/media/en/technical-documentation/application-notes/EE191.pdf)
     * - On SC59x/2159x, cross-DAI sources are typically selectable via DRU.
     * ================================================================ */

    /* Feed PCG external clock inputs from DAC BCLK (DAI1 PB05 via cross-DAI).
       DAI0_CRS_PB05_O = "DAI1's pin 5 shared into DAI0's SRU"
       (DAI1_CRS_PB05_O is the reverse direction — DAI0's pin 5 into DAI1) */
    SRU(DAI0_CRS_PB05_O, PCG0_EXTCLKA_I);
    SRU(DAI0_CRS_PB05_O, PCG0_EXTCLKB_I);
    /* ------------------------------------------------
     * PCG0_A outputs -> SPORT0 clocks (I2S domain)
     * ------------------------------------------------ */
    SRU(PCG0_FSA_O,  SPT0_AFS_I);
    SRU(PCG0_CLKA_O, SPT0_ACLK_I);

#if !ASRC_BYPASS
    /* ASRC mode: SPORT0B clocked by PCG (ASRC output domain) */
    SRU(PCG0_FSA_O,  SPT0_BFS_I);
    SRU(PCG0_CLKA_O, SPT0_BCLK_I);
#endif
    /* In ASRC_BYPASS mode, SPORT0B clocks are set in the bypass block below */

    /* ------------------------------------------------
     * PCG0_A outputs -> SPDIF TX base clocks
     * ------------------------------------------------ */
    SRU(PCG0_FSA_O,  SPDIF0_TX_FS_I);
    SRU(PCG0_CLKA_O, SPDIF0_TX_CLK_I);

    /* ------------------------------------------------
     * PCG0_B output -> SPDIF TX HFCLK (encoder ref)
     * ------------------------------------------------ */
    SRU(PCG0_CLKB_O, SPDIF0_TX_HFCLK_I);

#if ASRC_BYPASS
    /* ------------------------------------------------
     * ASRC BYPASS: SPDIF RX -> SPORT0B directly (like ADI example)
     * SPORT0B clocked by SPDIF RX recovered clocks
     * ------------------------------------------------ */
    SRU(SPDIF0_RX_CLK_O, SPT0_BCLK_I);
    SRU(SPDIF0_RX_FS_O,  SPT0_BFS_I);
    SRU(SPDIF0_RX_DAT_O, SPT0_BD0_I);
#else
    /* ------------------------------------------------
     * SPDIF RX -> ASRC input clocks/data
     * ------------------------------------------------ */
    SRU(SPDIF0_RX_FS_O,  SRC0_FS_IP_I);
    SRU(SPDIF0_RX_CLK_O, SRC0_CLK_IP_I);
    SRU(SPDIF0_RX_DAT_O, SRC0_DAT_IP_I);

    /* ------------------------------------------------
     * PCG0_A outputs -> ASRC output clocks (local 48k domain)
     * ------------------------------------------------ */
    SRU(PCG0_FSA_O,  SRC0_FS_OP_I);
    SRU(PCG0_CLKA_O, SRC0_CLK_OP_I);

    /* ------------------------------------------------
     * ASRC out -> SPORT0B RX data
     * ------------------------------------------------ */
    SRU(SRC0_DAT_OP_O, SPT0_BD0_I);
#endif

    /* ------------------------------------------------
     * SPORT0A TX data -> SPDIF TX data
     * ------------------------------------------------ */
    SRU(SPT0_AD0_O, SPDIF0_TX_DAT_I);

    /* ------------------------------------------------
     * SPDIF physical pins
     * ------------------------------------------------ */

    /* SPDIF TX output pin (example: DAI0 PB10) */
    SRU(SPDIF0_TX_O, DAI0_PB10_I);
    SRU(HIGH, DAI0_PBEN10_I);

    /* SPDIF RX input pin (example: DAI0 PB09) */
    SRU(LOW, DAI0_PBEN09_I);
    SRU(DAI0_PB09_O, SPDIF0_RX_I);


    printf("DAI0_CLK0=%08lx DAI0_CLK1=%08lx DAI0_CLK2=%08lx DAI0_CLK3=%08lx\n",
           *pREG_DAI0_CLK0, *pREG_DAI0_CLK1, *pREG_DAI0_CLK2, *pREG_DAI0_CLK3);
    printf("DAI0_FS0=%08lx  DAI0_FS1=%08lx  ...\n", *pREG_DAI0_FS0, *pREG_DAI0_FS1);
    printf("DAI1_CLK0=%08lx DAI1_FS0=%08lx ...\n", *pREG_DAI1_CLK0, *pREG_DAI1_FS0);

}
