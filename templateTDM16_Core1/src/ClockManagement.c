#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <services/pcg/adi_pcg.h>
#include <drivers/asrc/adi_asrc.h>

static ADI_PCG_CLK_INFO clkA;
static ADI_PCG_FS_INFO  fsA;
static ADI_PCG_CLK_INFO clkG;
static ADI_PCG_FS_INFO  fsG;


static ADI_ASRC_HANDLE handleASRC = NULL;
static uint8_t         memoryASRC[ADI_ASRC_MEMORY_SIZE];


void Pcginit(void)
{
    // --- existing: PCG0_A / FS-A for I2S/SPDIF (keep as you have) ---
    clkA.bExternalTrigger = 0;
    clkA.eClkInput        = ADI_PCG_CLK_EXT;  // 24.576 MHz via CGU/CDU
    clkA.nDiv             = 8;                // 24.576 / 8 = 3.072 MHz (I2S BCLK) : 2ch*32bits*48kHz = 3.072MHz

    fsA.bExternalTrigger  = 0;
    fsA.eClkInput         = ADI_PCG_FS_EXT;
    fsA.eFsBypassMode     = ADI_PCG_FSBYPASS_MODE_NORMAL;
    fsA.nDiv              = 512;              // 24.576 / 512 = 48 kHz (FS)
    fsA.nPhase            = 4;
    fsA.nPulseWidth       = 256;              // 50% duty cycle
    adi_pcg_Init(ADI_PCG_DEV_A, &clkA, &fsA);

    // Optional HFCLK for SPDIF (6.144 MHz = 24.576 / 4):
    ADI_PCG_CLK_INFO clkB = clkA; clkB.nDiv = 4;
    adi_pcg_Init(ADI_PCG_DEV_B, &clkB, NULL);

    // --- NEW: PCG0_G / FS-G for TDM16 on DAI1 (SPORT4) ---
    // BCLK target for TDM16 x 32-bit @ 48 kHz = 16 * 32 * 48k = 24.576 MHz

    clkG.bExternalTrigger = 0;
    clkG.eClkInput        = ADI_PCG_CLK_EXT;  // 24.576 MHz
    clkG.nDiv             = 1;                // 24.576 / 1 = 24.576 MHz (CLKG) 16ch*32bits*48kHz = 24.576MHz

    fsG.bExternalTrigger  = 0;
    fsG.eClkInput         = ADI_PCG_FS_EXT;
    fsG.eFsBypassMode     = ADI_PCG_FSBYPASS_MODE_NORMAL;
    fsG.nDiv              = 512;              // 24.576 / 512 = 48 kHz (FSG) frame sync is at 48kHz
    fsG.nPhase            = 0;
    fsG.nPulseWidth       = 1;                // 1 BCLK pulse per frame  <-- IMPORTANT

    adi_pcg_Init(ADI_PCG_DEV_G, &clkG, &fsG);
    printf("PCG initialized\n");
}


/* Simple error helper (local) */
static inline int _asrc_chk(const char* what, ADI_ASRC_RESULT r)
{
    if (r != ADI_ASRC_SUCCESS) {
        printf("%s rc=%d\n", what, (int)r);
        return 1;
    }
    return 0;
}

int ASRC_init(void)
{
    ADI_ASRC_RESULT r;

    if (handleASRC != NULL) {
        /* Already open */
        return 0;
    }

    r = adi_asrc_Open(
            0u,                    /* Block number: DAI0-ASRC */
            0u,                    /* Device number: ASRC0 within block */
            memoryASRC,            /* Driver memory */
            ADI_ASRC_MEMORY_SIZE,  /* Memory size */
            &handleASRC);
    if (_asrc_chk("adi_asrc_Open", r)) return 1;

    /* Enable the ASRC block */
    r = adi_asrc_Enable(handleASRC, true);
    if (_asrc_chk("adi_asrc_Enable", r)) return 1;
    printf("ASRC0 (DAI0) enabled\n");
    return 0;
}

int ASRC_deinit(void)
{
    if (!handleASRC) return 0;

    ADI_ASRC_RESULT r = adi_asrc_Enable(handleASRC, false);
    if (_asrc_chk("adi_asrc_Enable(false)", r)) return 1;

    r = adi_asrc_Close(handleASRC);
    if (_asrc_chk("adi_asrc_Close", r)) return 1;

    handleASRC = NULL;
    return 0;
}

