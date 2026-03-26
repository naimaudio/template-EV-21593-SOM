/*****************************************************************************
 * main.c — Application entry point
 *****************************************************************************/
#include <time.h>
#include <math.h>
#include "adi_initialize.h"
#include <sys/platform.h>
#include <sys/adi_core.h>

/* Set to 1 to bypass ALL software TX processing and play a sine
   directly from pre-filled DMA buffers. This tests ONLY the
   DMA → SPORT → DAC path.  Set to 0 for normal operation.
   Set to 2 to write sine directly to DMA buffer via isFreshData
   handshake (tests ISR flip coordination, bypasses ring+memcpy). */
#define DIRECT_DMA_TEST  0

#include "app/process.h"
#include "audio/buffer.h"
#include "codec/adau1962.h"
#include "codec/adau1979.h"
#include "config.h"
#include "audio/sport.h"
#include "platform/soft_switch.h"
#include "platform/sru.h"
#include "platform/spu.h"
#include "codec/clock.h"
#include "audio/io.h"
#include "platform/twi.h"

// If you plan to disable Jacks dynamically, gate it here:
static inline bool isJackActive(void) { return true; /* or a runtime flag */ }

/* Debug: RX data snapshot from processBlock */
extern volatile uint32_t dbgRxHead[16];
extern volatile bool     dbgRxCaptured;
/* Debug: TX data snapshot from processBlock */
extern volatile uint32_t dbgTxHead[24];
extern volatile bool     dbgTxCaptured;

/* Glitch detector counters (defined in process.c) */
extern volatile uint32_t gGlitchCount;
extern volatile uint32_t gGlitchCaptureIndex;
extern volatile uint32_t gBlockNumber;

/* TX race detector counters (defined in io.c) */
extern volatile uint32_t gTxRaceDetected;
extern volatile uint32_t gSpdifTxRaceDetected;

/* ISR error detail counters (defined in sport.c) */
extern volatile uint32_t gTxUnderflow;
extern volatile uint32_t gFsErr;
extern volatile uint32_t gDmaErr;

/* DMA buffer integrity verifier (defined in io.c) */
extern volatile uint32_t gDmaVerifyMismatchCount;
extern volatile uint32_t gDmaVerifyFillCallCount;
extern volatile uint32_t gDmaVerifyCaptureIndex;

typedef struct DmaVerifyMismatch_ {
    uint32_t fillCallNumber;
    uint32_t wordIndex;
    uint32_t expectedValue;
    uint32_t actualReadback;
    float    sourceFloat;
    uint32_t sourceRaw;
} DmaVerifyMismatch;
extern DmaVerifyMismatch dmaVerifyCaptures[];  /* defined in io.c */

/* Glitch snapshot structure (must match process.c definition) */
typedef struct GlitchSnapshot_ {
    uint32_t blockNumber;
    uint32_t frameIndexInBlock;
    float    previousSample;
    float    currentSample;
    float    nextSample;
    float    jumpMagnitude;
    uint32_t jackRxRingCount;
    uint32_t jackTxRingCount;
    uint32_t spdifRxRingCount;
    uint32_t jackRxOverrunCount;
    uint32_t spdifRxOverrunCount;
    uint32_t jackRxDoneAtGlitch;
    uint32_t jackTxDoneAtGlitch;
} GlitchSnapshot;
extern GlitchSnapshot glitchCaptures[];  /* defined in process.c */

int main(int argc, char *argv[])
{
	int delayIteration = 500;

    adi_initComponents();
    delay(delayIteration);

    adi_core_enable(ADI_CORE_SHARC1);
    delay(delayIteration);

    initPingPongBuffers();
    dumpRingBufferAddresses();   /* Item #4: verify no ring slot aliasing */
    delay(delayIteration);

    TwiOpen();
    delay(delayIteration);

    ConfigureSpu();
    delay(delayIteration);

    TwiSetAddr(I2cAddrSOFTConfig);
    delay(delayIteration);

	Soft_resetAudio();
	delay(delayIteration);

    ConfigureSru(); // routing clocks to devices
    delay(delayIteration);

	Soft_init();
	delay(delayIteration);

	/* Verify soft-switch GPIOB: bit2=0 → optical ON, bit1=1 → coax OFF */
	{
	    uint8_t gpioB = TwiRead8(0x13);
	    printf("Soft switch GPIOB = 0x%02X  optical=%s coax=%s (expect 0xFB)\n",
	        gpioB, (gpioB & 0x04) ? "OFF" : "ON", (gpioB & 0x02) ? "OFF" : "ON");
	}

    TwiSetAddr(I2cAddrAdau1962);
    delay(delayIteration);

    ADAU1962_init();  /* DAC PLL locks → starts generating BCLK on DAI1_PB05 */
    delay(delayIteration);

    TwiSetAddr(I2cAddrAdau1979);
    delay(delayIteration);

    ADAU1979_init();
    delay(delayIteration);

	Pcginit(); /* PCG0 uses DAC BCLK via cross-DAI — must init AFTER DAC is running */
	delay(delayIteration);

    AudioIO_setSPDIFState(SPDIF_DIGITAL_ON_OPTICAL_ON);
    delay(delayIteration);

    AudioIO_applyConfiguration();
    delay(delayIteration);

#if DIRECT_DMA_TEST == 1
    /* Pre-fill TX DMA buffers with 750 Hz sine on DAC01 only.
       750 Hz × 256 samples / 48000 = 4 complete cycles → seamless loop.
       DMA plays Pong first, then Ping, then loops.
       Pong = samples 0..127, Ping = samples 128..255. */
    {
        const float freq = 750.0f;
        const float amp  = (float)0x20000000;  /* -12 dBFS */
        for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; f++) {
            /* Pong: samples 0..127 (DMA reads this first) */
            float phase0 = 2.0f * 3.14159265f * freq * (float)f / 48000.0f;
            int32_t s0 = (int32_t)(sinf(phase0) * amp);
            jackStream.Tx.pong[f * SLOTS_TX + 0] = (uint32_t)s0 & 0xFFFFFF00u;  /* DAC01, 24-bit aligned */
            for (uint32_t ch = 1; ch < SLOTS_TX; ch++)
                jackStream.Tx.pong[f * SLOTS_TX + ch] = 0;

            /* Ping: samples 128..255 */
            float phase1 = 2.0f * 3.14159265f * freq * (float)(f + SAMPLES_PER_BLOCK) / 48000.0f;
            int32_t s1 = (int32_t)(sinf(phase1) * amp);
            jackStream.Tx.ping[f * SLOTS_TX + 0] = (uint32_t)s1 & 0xFFFFFF00u;  /* DAC01, 24-bit aligned */
            for (uint32_t ch = 1; ch < SLOTS_TX; ch++)
                jackStream.Tx.ping[f * SLOTS_TX + ch] = 0;
        }
        printf("DIRECT DMA TEST: pre-filled TX ping/pong with 750 Hz sine on DAC01\n");
        printf("  Pong[0]=%08lX Pong[12]=%08lX Pong[24]=%08lX\n",
            (unsigned long)jackStream.Tx.pong[0],
            (unsigned long)jackStream.Tx.pong[12],
            (unsigned long)jackStream.Tx.pong[24]);
        printf("  Ping[0]=%08lX Ping[12]=%08lX Ping[24]=%08lX\n",
            (unsigned long)jackStream.Tx.ping[0],
            (unsigned long)jackStream.Tx.ping[12],
            (unsigned long)jackStream.Tx.ping[24]);
    }
#endif

    if (spdifSportInit()) {
        printf("*** spdifSportInit FAILED ***\n");
    }
    delay(delayIteration);

    /* === SPDIF RX diagnostic: dump registers and verify stable lock === */
    {
        printf("=== SPDIF RX Diagnostic ===\n");
        printf("  RX_CTL     = 0x%08lX\n", (unsigned long)*pREG_SPDIF0_RX_CTL);

        uint32_t stat = *pREG_SPDIF0_RX_STAT;
        printf("  RX_STAT    = 0x%08lX  LOCK=%lu LOCKLOSS=%lu VALID=%lu\n",
            (unsigned long)stat,
            (unsigned long)((stat >> 3) & 1u),
            (unsigned long)((stat >> 4) & 1u),
            (unsigned long)((stat >> 2) & 1u));
        printf("  WordLen: ChA=0x%lX ChB=0x%lX\n",
            (unsigned long)((stat >> 8) & 0xFu),
            (unsigned long)((stat >> 12) & 0xFu));

        /* Channel status bytes: reveal sample rate, format, word length */
        uint32_t cs0a = *pREG_SPDIF0_RX_STAT0_A;
        uint32_t cs1a = *pREG_SPDIF0_RX_STAT1_A;
        printf("  CS0_A      = 0x%08lX (bytes 0-3)\n", (unsigned long)cs0a);
        printf("  CS1_A      = 0x%08lX (byte 4)\n", (unsigned long)cs1a);
        /* Byte 3 bits[3:0]+byte4 bits[1:0] = sample freq (IEC 60958-3):
           0100xx=48k, 0000xx=44.1k, 1100xx=32k, 1001xx=96k */
        uint8_t csByte3 = (uint8_t)((cs0a >> 24) & 0xFF);
        uint8_t csByte4 = (uint8_t)(cs1a & 0xFF);
        printf("  CS byte3=0x%02X byte4=0x%02X  (Fs nibble=0x%X)\n",
            csByte3, csByte4, (csByte3 >> 4) & 0xF);

        /* Wait for STABLE lock: require LOCK=1 for 50 consecutive polls.
           If SPDIF source is absent or flaky, this will timeout. */
        uint32_t lockStreak = 0, attempts = 0;
        while (lockStreak < 50u && attempts < 2000u) {
            stat = *pREG_SPDIF0_RX_STAT;
            if (stat & 0x08u) lockStreak++;
            else              lockStreak = 0;
            delay(1);
            attempts++;
        }
        stat = *pREG_SPDIF0_RX_STAT;
        printf("  Lock stability: streak=%lu attempts=%lu  final STAT=0x%08lX\n",
            (unsigned long)lockStreak, (unsigned long)attempts,
            (unsigned long)stat);
        if (lockStreak < 50u) {
            printf("  WARNING: SPDIF lock NOT stable — ASRC may not converge!\n");
            printf("  Check: optical cable connected? Source playing? Correct input?\n");
        } else {
            /* Let PLL settle after achieving stable lock before ASRC starts */
            printf("  Lock stable. Settling 500ms before ASRC...\n");
            delay(500);
        }
    }

#if !ASRC_BYPASS
    ASRC_init();
    delay(delayIteration);

    /* Monitor ASRC ratio convergence: poll RAT01 to see if MUTEOUT clears.
       MUTEOUT0 = bit 15, MUTEOUT1 = bit 31.
       Ratio format is 4.11 fixed-point: unity = 0x0800, expected for 192k->48k = 0x0200.
       A converged ASRC shows MUTEOUT=0 and a stable non-zero RATIO value. */
    {
        const uint32_t MUTEOUT_MASK = 0x80008000u;  /* MUTEOUT0 | MUTEOUT1 */
        printf("ASRC convergence monitor (20 x 500ms):\n");
        uint32_t rat;
        for (uint32_t i = 0; i < 20; i++) {
            delay(500);
            rat = *pREG_ASRC0_RAT01;
            uint32_t spdifStat = *pREG_SPDIF0_RX_STAT;
            printf("  [%2lu] RAT01=0x%08lX  MUTE0=%lu MUTE1=%lu  RATIO0=0x%04lX RATIO1=0x%04lX  SPDIF_LOCK=%lu\n",
                (unsigned long)i, (unsigned long)rat,
                (unsigned long)((rat >> 15) & 1u),
                (unsigned long)((rat >> 31) & 1u),
                (unsigned long)(rat & 0x7FFFu),
                (unsigned long)((rat >> 16) & 0x7FFFu),
                (unsigned long)((spdifStat >> 3) & 1u));
            if ((rat & MUTEOUT_MASK) == 0u) {
                printf("  ASRC converged at iteration %lu!\n", (unsigned long)i);
                break;
            }
        }
        /* If still muted after 10 seconds, try soft reset + wait */
        if ((*pREG_ASRC0_RAT01 & MUTEOUT_MASK) != 0u) {
            printf("  ASRC still MUTED after monitoring. Trying soft reset...\n");
            ASRC_softReset();
            delay(2000);
            rat = *pREG_ASRC0_RAT01;
            printf("  After soft reset + 2s: RAT01=0x%08lX  MUTE0=%lu MUTE1=%lu\n",
                (unsigned long)rat,
                (unsigned long)((rat >> 15) & 1u),
                (unsigned long)((rat >> 31) & 1u));
        }
        printf("ASRC CTL01=0x%08lX  SPDIF_STAT=0x%08lX  SPDIF_CTL=0x%08lX\n",
            (unsigned long)*pREG_ASRC0_CTL01,
            (unsigned long)*pREG_SPDIF0_RX_STAT,
            (unsigned long)*pREG_SPDIF0_RX_CTL);
    }
#else
    printf("ASRC_BYPASS=1: SPDIF RX routed directly to SPORT0B (no ASRC)\n");
    printf("  SPDIF_STAT=0x%08lX\n", (unsigned long)*pREG_SPDIF0_RX_STAT);
#endif

    jackSportInit();
    delay(delayIteration);

    dumpSportRegisters();

    prepareToPlay();
    delay(delayIteration);

    TwiClose();

    /* ---- Priming: build up ring buffer headroom before main loop ---- */
    printf("Pre-prime: JRxDone=%lu SRxDone=%lu  jRing=%u sRing=%u  jOvr=%lu sOvr=%lu\n",
        (unsigned long)gJackRxDone, (unsigned long)gSpdifRxDone,
        (unsigned)circBuf_count(&jackRxRing), (unsigned)circBuf_count(&spdifRxRing),
        (unsigned long)jackRxRing.overrunCount, (unsigned long)spdifRxRing.overrunCount);

    /* Step 1: Wait for RX rings to fill (ISR does this automatically) */
    printf("Priming RX rings (threshold=%u)...\n", PRIMING_BLOCKS);
    while (circBuf_count(&jackRxRing) < PRIMING_BLOCKS)
    { /* ISR fills ring via SportCallback */ }

    /* Step 2: Prime TX rings by running processBlock to move data from RX→TX. */
    printf("Priming TX rings...\n");
    while (circBuf_count(&jackTxRing) < PRIMING_BLOCKS) {
        processBlock();
        while (circBuf_count(&jackRxRing) < 1u) {}
    }

    printf("Priming complete: jackRx=%u spdifRx=%u jackTx=%u spdifTx=%u\n",
        (unsigned)circBuf_count(&jackRxRing),
        (unsigned)circBuf_count(&spdifRxRing),
        (unsigned)circBuf_count(&jackTxRing),
        (unsigned)circBuf_count(&spdifTxRing));

    printf("main loop running...\n");

    /* Set to 0 to disable ALL main-loop printf for a clean audio test.
       Counters still update — read them via CCES debugger after the test.
       Volatile globals to inspect: gGlitchCount, gTxRaceDetected, gDmaVerifyMismatchCount,
       gBlockNumber, jackRxRing.overrunCount, spdifRxRing.overrunCount */
#define ENABLE_MAIN_LOOP_PRINTF  0

    /* One-shot diagnostic dump: fires once after ~30s of running, then never again.
       Single printf = single glitch, but gives us all counters without JTAG. */
    static bool oneTimeDumpDone = false;

    static uint32_t lastPrintRxDone = 0;
    static uint32_t lastGlitchDumpIndex = 0;
    static uint32_t lastDmaVerifyDumpIndex = 0;
    for (;;)
    {
        processBlock();
        fillDACOutputFromGlobal();
        fillSpdifOutputFromGlobal();

        /* One-time counter dump after ~30s (11250 ISR callbacks at 375/sec) */
        if (!oneTimeDumpDone && gJackRxDone >= 11250u) {
            oneTimeDumpDone = true;
            printf("=== ONE-TIME DUMP after %lu ISR cycles ===\n"
                   "GLITCH=%lu  RACE=%lu/%lu  DMA_ERR=%lu  blk=%lu\n"
                   "ovr=%lu/%lu  err=%lu  fsErr=%lu  dmaErr=%lu  txUnder=%lu\n"
                   "jRx=%u sRx=%u jTx=%u sTx=%u\n"
                   "JRx=%lu JTx=%lu SRx=%lu STx=%lu\n",
                (unsigned long)gJackRxDone,
                (unsigned long)gGlitchCount,
                (unsigned long)gTxRaceDetected,
                (unsigned long)gSpdifTxRaceDetected,
                (unsigned long)gDmaVerifyMismatchCount,
                (unsigned long)gBlockNumber,
                (unsigned long)jackRxRing.overrunCount,
                (unsigned long)spdifRxRing.overrunCount,
                (unsigned long)gSportErr,
                (unsigned long)gFsErr,
                (unsigned long)gDmaErr,
                (unsigned long)gTxUnderflow,
                (unsigned)circBuf_count(&jackRxRing),
                (unsigned)circBuf_count(&spdifRxRing),
                (unsigned)circBuf_count(&jackTxRing),
                (unsigned)circBuf_count(&spdifTxRing),
                (unsigned long)gJackRxDone, (unsigned long)gJackTxDone,
                (unsigned long)gSpdifRxDone, (unsigned long)gSpdifTxDone);
        }

#if ENABLE_MAIN_LOOP_PRINTF
        /* Print every ~5s using ISR counter (not loop iterations) */
        if ((gJackRxDone - lastPrintRxDone) >= 1875u) {
            lastPrintRxDone = gJackRxDone;
            printf("jRx=%u sRx=%u jTx=%u sTx=%u  "
                   "JRx=%lu JTx=%lu SRx=%lu STx=%lu  "
                   "ovr=%lu/%lu err=%lu  "
                   "GLITCH=%lu RACE=%lu/%lu DMA_ERR=%lu blk=%lu\n",
                (unsigned)circBuf_count(&jackRxRing),
                (unsigned)circBuf_count(&spdifRxRing),
                (unsigned)circBuf_count(&jackTxRing),
                (unsigned)circBuf_count(&spdifTxRing),
                (unsigned long)gJackRxDone, (unsigned long)gJackTxDone,
                (unsigned long)gSpdifRxDone, (unsigned long)gSpdifTxDone,
                (unsigned long)jackRxRing.overrunCount,
                (unsigned long)spdifRxRing.overrunCount,
                (unsigned long)gSportErr,
                (unsigned long)gGlitchCount,
                (unsigned long)gTxRaceDetected,
                (unsigned long)gSpdifTxRaceDetected,
                (unsigned long)gDmaVerifyMismatchCount,
                (unsigned long)gBlockNumber);

            /* Dump any newly captured glitch snapshots */
            while (lastGlitchDumpIndex < gGlitchCaptureIndex) {
                GlitchSnapshot *snapshot = &glitchCaptures[lastGlitchDumpIndex];
                printf("  GLITCH #%lu: blk=%lu frm=%lu  prev=%.6f cur=%.6f next=%.6f  "
                       "jump=%.6f  jRx=%u jTx=%u sRx=%u  "
                       "ovr=%lu/%lu  ISR:Rx=%lu Tx=%lu\n",
                    (unsigned long)(lastGlitchDumpIndex + 1),
                    (unsigned long)snapshot->blockNumber,
                    (unsigned long)snapshot->frameIndexInBlock,
                    (double)snapshot->previousSample,
                    (double)snapshot->currentSample,
                    (double)snapshot->nextSample,
                    (double)snapshot->jumpMagnitude,
                    (unsigned)snapshot->jackRxRingCount,
                    (unsigned)snapshot->jackTxRingCount,
                    (unsigned)snapshot->spdifRxRingCount,
                    (unsigned long)snapshot->jackRxOverrunCount,
                    (unsigned long)snapshot->spdifRxOverrunCount,
                    (unsigned long)snapshot->jackRxDoneAtGlitch,
                    (unsigned long)snapshot->jackTxDoneAtGlitch);
                lastGlitchDumpIndex++;
            }

            /* Dump any newly captured DMA verify mismatches */
            while (lastDmaVerifyDumpIndex < gDmaVerifyCaptureIndex) {
                DmaVerifyMismatch *capture = &dmaVerifyCaptures[lastDmaVerifyDumpIndex];
                printf("  DMA_ERR #%lu: fill=%lu word=%lu  expected=0x%08lX readback=0x%08lX  "
                       "srcFloat=%.6f srcRaw=0x%08lX\n",
                    (unsigned long)(lastDmaVerifyDumpIndex + 1),
                    (unsigned long)capture->fillCallNumber,
                    (unsigned long)capture->wordIndex,
                    (unsigned long)capture->expectedValue,
                    (unsigned long)capture->actualReadback,
                    (double)capture->sourceFloat,
                    (unsigned long)capture->sourceRaw);
                lastDmaVerifyDumpIndex++;
            }
        }
#endif /* ENABLE_MAIN_LOOP_PRINTF */
    }
}
