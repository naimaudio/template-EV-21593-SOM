// SRUConfig.c
#include <SRU.h>
#include <stdint.h>

// for clarity redefine the different use of the SRU function for DAI0
#define DSP_LISTEN_TO(PBEN)    SRU(LOW,  PBEN)
#define DSP_DRIVE(PBEN)        SRU(HIGH, PBEN)
#define ROUTE(SRC, DST)        SRU(SRC,  DST)

// for clarity redefine the different use of the SRU2 function for DAI1
#define DSP_LISTEN_TO2(PBEN)   SRU2(LOW,  PBEN)
#define DSP_DRIVE2(PBEN)       SRU2(HIGH, PBEN)
#define ROUTE2(SRC, DST)       SRU2(SRC,  DST)

#define CLOCK_SOURCE			DAI1_PB05_O//DAI1_PB03_O

/*
 pin connection can be found in this datasheet for the EZ-KIT :
 https://www.analog.com/media/en/technical-documentation/eval-board-schematic/ev-somcrr-ezkit-schematic.pdf#page=4.48
*/

void ConfigureSru(void)
{
    /* pads enable */
    *pREG_PADS0_DAI0_IE = 0x1FFFFF;
    *pREG_PADS0_DAI1_IE = 0x1FFFFF;

    /* ===== House ref (DAI1_PB03) -> PCG ===== */
    ROUTE2(CLOCK_SOURCE	, PCG0_EXTCLKA_I);
    // DAI1_PB03->PCGB (HFCLK fan-out as in SPDIF example)
    ROUTE2(CLOCK_SOURCE	, PCG0_EXTCLKB_I);
    ROUTE2(CLOCK_SOURCE	, PCG0_EXTCLKG_I);

    /* ===== PCGA -> SPORT0 ===== */
    ROUTE(PCG0_FSA_O,  SPT0_AFS_I);
    ROUTE(PCG0_CLKA_O, SPT0_ACLK_I);
    ROUTE(PCG0_FSA_O,  SPT0_BFS_I);
    ROUTE(PCG0_CLKA_O, SPT0_BCLK_I);

    /* ===== PCGA -> SPDIF_TX ===== */
    ROUTE(PCG0_FSA_O,  SPDIF0_TX_FS_I);
    ROUTE(PCG0_CLKA_O, SPDIF0_TX_CLK_I);

    /* ===== PCGB -> SPDIF_TX (HFCLK) ===== */
    ROUTE(PCG0_CLKB_O, SPDIF0_TX_HFCLK_I);

    /* ===== SPDIF_RX -> ASRC_IN ===== */
    ROUTE(SPDIF0_RX_FS_O,  SRC0_FS_IP_I);
    ROUTE(SPDIF0_RX_CLK_O, SRC0_CLK_IP_I);
    ROUTE(SPDIF0_RX_DAT_O, SRC0_DAT_IP_I);

    /* ===== PCGA -> ASRC_OUT ===== */
    ROUTE(PCG0_FSA_O,  SRC0_FS_OP_I);
    ROUTE(PCG0_CLKA_O, SRC0_CLK_OP_I);

    /* ===== ASRC_OUT -> SPORT0B ===== */
    ROUTE(SRC0_DAT_OP_O, SPT0_BD0_I);

    /* ===== SPORT0A -> SPDIF_TX ===== */
    ROUTE(SPT0_AD0_O, SPDIF0_TX_DAT_I);

    /* ===== SPDIF pins ===== */
    // SPDIF_TX->DAI0_PB10
    ROUTE(SPDIF0_TX_O, DAI0_PB10_I);
    DSP_DRIVE(DAI0_PBEN10_I);

    // DAI0_PB09 (SPDIF IN)->SPDIF_RX
    DSP_LISTEN_TO(DAI0_PBEN09_I);
    ROUTE(DAI0_PB09_O, SPDIF0_RX_I);

    /* ===== PCGG -> SPORT4 (TDM16 domain) ===== */
    ROUTE(PCG0_FSG_O,  SPT4_AFS_I);
    ROUTE(PCG0_CLKG_O, SPT4_ACLK_I);
    ROUTE(PCG0_FSG_O,  SPT4_BFS_I);
    ROUTE(PCG0_CLKG_O, SPT4_BCLK_I);

    /* ===== PCGG -> ADC (BCLK/LRCLK on DAI1) ===== */
    DSP_DRIVE2(DAI1_PBEN12_I);                 // drive BCLK pin to ADC
    ROUTE2(PCG0_CLKG_O, DAI1_PB12_I);
    DSP_DRIVE2(DAI1_PBEN20_I);                 // drive LRCLK pin to ADC
    ROUTE2(PCG0_FSG_O,  DAI1_PB20_I);

    /* ===== PCGG -> DAC (BCLK/LRCLK on DAI1) ===== */
    //DSP_DRIVE2(DAI1_PBEN05_I);                 // drive BCLK pin to DAC
    //ROUTE2(PCG0_CLKG_O, DAI1_PB05_I);

    //DSP_DRIVE2(DAI1_PBEN04_I);                 // drive LRCLK pin to DAC
    //ROUTE2(PCG0_FSG_O,  DAI1_PB04_I);

    /* ===== ADC -> SPORT4B (TDM in) ===== */
    DSP_LISTEN_TO2(DAI1_PBEN06_I);             // listen to ADC SDOUT
    ROUTE2(DAI1_PB06_O, SPT4_BD0_I);

    /* ===== SPORT4A -> DAC (TDM out) ===== */
    DSP_DRIVE2(DAI1_PBEN01_I);                 // drive DAC SDATAIN
    ROUTE2(SPT4_AD0_O,  DAI1_PB01_I);
}
