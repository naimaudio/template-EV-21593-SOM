#include "codec/clock.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <services/pcg/adi_pcg.h>
#include <drivers/asrc/adi_asrc.h>

static ADI_PCG_CLK_INFO clkA;
static ADI_PCG_FS_INFO  fsA;
static ADI_PCG_CLK_INFO clkB;
static ADI_PCG_FS_INFO  fsB;


static ADI_ASRC_HANDLE handleASRC0 = NULL;
static ADI_ASRC_HANDLE handleASRC1 = NULL;
static uint8_t         memoryASRC0[ADI_ASRC_MEMORY_SIZE];
static uint8_t         memoryASRC1[ADI_ASRC_MEMORY_SIZE];


void Pcginit(void)
{
    // --- existing: PCG0_A / FS-A for I2S/SPDIF (keep as you have) ---
    clkA.bExternalTrigger = 0;
    clkA.eClkInput        = ADI_PCG_CLK_EXT;  // 24.576 MHz via DAC
    clkA.nDiv             = 8;                // 24.576 / 8 = 3.072 MHz (I2S BCLK) : 2ch*32bits*48kHz = 3.072MHz

    fsA.bExternalTrigger  = 0;
    fsA.eClkInput         = ADI_PCG_FS_EXT;
    fsA.eFsBypassMode     = ADI_PCG_FSBYPASS_MODE_NORMAL;
    fsA.nDiv              = 512;              // 24.576 / 512 = 48 kHz (FS)
    fsA.nPhase            = 4;
    fsA.nPulseWidth       = 256;              // 50% duty cycle


    clkB.bExternalTrigger = 0;
    clkB.eClkInput        = ADI_PCG_CLK_EXT;  // 24.576 MHz via DAC
    clkB.nDiv             = 2;                // 24.576 / 2 = 12.288MHz MHz for SPDIF codec

    /* Use adi_pcg_Init which sets CLKEN/FSEN per-device
       (adi_pcg_Global_Configure does NOT set the enable bits) */
    adi_pcg_Init(ADI_PCG_DEV_A, &clkA, &fsA);
    adi_pcg_Init(ADI_PCG_DEV_B, &clkB, NULL);

    printf("PCG initialized: CTLA0=0x%08lX CTLA1=0x%08lX CTLB0=0x%08lX CTLB1=0x%08lX\n",
        (unsigned long)*pREG_PCG0_CTLA0, (unsigned long)*pREG_PCG0_CTLA1,
        (unsigned long)*pREG_PCG0_CTLB0, (unsigned long)*pREG_PCG0_CTLB1);
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

    if (handleASRC0 != NULL) {
        /* Already open */
        return 0;
    }

    /* ASRC0 = left channel of stereo pair 0/1 */
    r = adi_asrc_Open(
            0u,                    /* Block number: DAI0-ASRC */
            0u,                    /* Device number: ASRC0 (left ch) */
            memoryASRC0,           /* Driver memory */
            ADI_ASRC_MEMORY_SIZE,  /* Memory size */
            &handleASRC0);
    if (_asrc_chk("adi_asrc_Open(0)", r)) return 1;

    /* SPDIF RX decoder outputs I2S serial data on SC594.
       ASRC output also I2S to match SPORT0B configuration. */
    r = adi_asrc_SetSerialFormat(handleASRC0,
            ADI_ASRC_INPUT_I2S,              /* SPDIF RX output is I2S on SC594 */
            ADI_ASRC_OUTPUT_I2S,             /* SPORT0B is in I2S mode */
            ADI_ASRC_WORD_LENGTH_24);
    if (_asrc_chk("adi_asrc_SetSerialFormat(0)", r)) return 1;

    r = adi_asrc_Enable(handleASRC0, true);
    if (_asrc_chk("adi_asrc_Enable(0)", r)) return 1;

    /* ASRC1 = right channel of stereo pair 0/1 */
    r = adi_asrc_Open(
            0u,                    /* Block number: DAI0-ASRC */
            1u,                    /* Device number: ASRC1 (right ch) */
            memoryASRC1,           /* Driver memory */
            ADI_ASRC_MEMORY_SIZE,  /* Memory size */
            &handleASRC1);
    if (_asrc_chk("adi_asrc_Open(1)", r)) return 1;

    r = adi_asrc_SetSerialFormat(handleASRC1,
            ADI_ASRC_INPUT_I2S,
            ADI_ASRC_OUTPUT_I2S,
            ADI_ASRC_WORD_LENGTH_24);
    if (_asrc_chk("adi_asrc_SetSerialFormat(1)", r)) return 1;

    r = adi_asrc_Enable(handleASRC1, true);
    if (_asrc_chk("adi_asrc_Enable(1)", r)) return 1;

    printf("ASRC0+1 (DAI0) enabled: CTL01=0x%08lX\n",
        (unsigned long)*pREG_ASRC0_CTL01);
    return 0;
}

int ASRC_softReset(void)
{
    ADI_ASRC_RESULT r;
    if (handleASRC0 == NULL) return 1;

    /* Disable both ASRCs */
    r = adi_asrc_Enable(handleASRC0, false);
    if (_asrc_chk("ASRC_softReset: disable(0)", r)) return 1;
    r = adi_asrc_Enable(handleASRC1, false);
    if (_asrc_chk("ASRC_softReset: disable(1)", r)) return 1;

    /* Brief pause to let ASRC internal state machine reset */
    for (volatile int i = 0; i < 0xFFFF; i++) { asm("nop;"); }

    /* Re-enable */
    r = adi_asrc_Enable(handleASRC0, true);
    if (_asrc_chk("ASRC_softReset: enable(0)", r)) return 1;
    r = adi_asrc_Enable(handleASRC1, true);
    if (_asrc_chk("ASRC_softReset: enable(1)", r)) return 1;

    printf("ASRC soft-reset done: CTL01=0x%08lX RAT01=0x%08lX\n",
        (unsigned long)*pREG_ASRC0_CTL01, (unsigned long)*pREG_ASRC0_RAT01);
    return 0;
}

int ASRC_deinit(void)
{
    ADI_ASRC_RESULT r;

    if (handleASRC1) {
        r = adi_asrc_Enable(handleASRC1, false);
        if (_asrc_chk("adi_asrc_Enable(1,false)", r)) return 1;
        r = adi_asrc_Close(handleASRC1);
        if (_asrc_chk("adi_asrc_Close(1)", r)) return 1;
        handleASRC1 = NULL;
    }

    if (handleASRC0) {
        r = adi_asrc_Enable(handleASRC0, false);
        if (_asrc_chk("adi_asrc_Enable(0,false)", r)) return 1;
        r = adi_asrc_Close(handleASRC0);
        if (_asrc_chk("adi_asrc_Close(0)", r)) return 1;
        handleASRC0 = NULL;
    }

    return 0;
}
