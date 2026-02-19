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

int main(int argc, char *argv[])
{
	int delayIteration = 500;

    adi_initComponents();
    delay(delayIteration);

    adi_core_enable(ADI_CORE_SHARC1);
    delay(delayIteration);

    initPingPongBuffers();
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

    /* Wait for SPDIF decoder to lock before enabling ASRC.
       ASRC must have valid input clocks when enabled, otherwise
       MUTEOUT stays active and output is zero. */
    {
        uint32_t lockWait = 0;
        while (!(*pREG_SPDIF0_RX_STAT & 0x08u) && lockWait < 200u) {
            delay(1);
            lockWait++;
        }
        printf("SPDIF lock wait: %lu iterations, STAT=0x%08lX\n",
            (unsigned long)lockWait, (unsigned long)*pREG_SPDIF0_RX_STAT);
    }

    ASRC_init();
    delay(delayIteration);

    /* Monitor ASRC ratio convergence: poll RAT01 to see if MUTEOUT clears.
       MUTEOUT0 = bit 15, MUTEOUT1 = bit 31.
       A converged ASRC shows MUTEOUT=0 and a stable non-zero RATIO value. */
    {
        const uint32_t MUTEOUT_MASK = 0x80008000u;  /* MUTEOUT0 | MUTEOUT1 */
        printf("ASRC convergence monitor (10 x 200ms):\n");
        uint32_t rat;
        for (uint32_t i = 0; i < 10; i++) {
            delay(100);  /* ~200ms */
            rat = *pREG_ASRC0_RAT01;
            printf("  [%lu] RAT01=0x%08lX  MUTE0=%lu MUTE1=%lu  RATIO0=0x%04lX RATIO1=0x%04lX\n",
                (unsigned long)i, (unsigned long)rat,
                (unsigned long)((rat >> 15) & 1u),
                (unsigned long)((rat >> 31) & 1u),
                (unsigned long)(rat & 0x7FFFu),
                (unsigned long)((rat >> 16) & 0x7FFFu));
            if ((rat & MUTEOUT_MASK) == 0u) {
                printf("  ASRC converged at iteration %lu!\n", (unsigned long)i);
                break;
            }
        }
        /* If still muted after 2 seconds, try soft reset */
        if ((*pREG_ASRC0_RAT01 & MUTEOUT_MASK) != 0u) {
            printf("  ASRC still MUTED after monitoring. Trying soft reset...\n");
            ASRC_softReset();
            /* Give it another second to converge */
            delay(500);
            rat = *pREG_ASRC0_RAT01;
            printf("  After soft reset: RAT01=0x%08lX  MUTE0=%lu MUTE1=%lu\n",
                (unsigned long)rat,
                (unsigned long)((rat >> 15) & 1u),
                (unsigned long)((rat >> 31) & 1u));
        }
        printf("ASRC CTL01=0x%08lX  SPDIF_STAT=0x%08lX  SPDIF_CTL=0x%08lX\n",
            (unsigned long)*pREG_ASRC0_CTL01,
            (unsigned long)*pREG_SPDIF0_RX_STAT,
            (unsigned long)*pREG_SPDIF0_RX_CTL);
    }

    jackSportInit();
    delay(delayIteration);

    dumpSportRegisters();

    prepareToPlay();
    delay(delayIteration);

    TwiClose();

    /* ---- Priming: ISR fills RX rings automatically, just wait ---- */
    printf("Pre-prime: JRxDone=%lu SRxDone=%lu  jRing=%u sRing=%u  jOvr=%lu sOvr=%lu\n",
        (unsigned long)gJackRxDone, (unsigned long)gSpdifRxDone,
        (unsigned)circBuf_count(&jackRxRing), (unsigned)circBuf_count(&spdifRxRing),
        (unsigned long)jackRxRing.overrunCount, (unsigned long)spdifRxRing.overrunCount);
    printf("Priming JACK RX ring (threshold=%u)...\n", PRIMING_BLOCKS);
    while (circBuf_count(&jackRxRing) < PRIMING_BLOCKS)
    { /* ISR fills ring via SportCallback */ }
    printf("Priming complete: jackRx=%u spdifRx=%u\n",
        (unsigned)circBuf_count(&jackRxRing),
        (unsigned)circBuf_count(&spdifRxRing));

    printf("main loop running...\n");

#if !DIRECT_DMA_TEST
    /* One-shot RX hex dump: wait for processBlock to capture, then print */
    while (!dbgRxCaptured) { processBlock(); }
    printf("RX data (first 4 frames x 4 ch = 16 words):\n");
    printf("  f0: %08lX %08lX %08lX %08lX\n",
        (unsigned long)dbgRxHead[0],  (unsigned long)dbgRxHead[1],
        (unsigned long)dbgRxHead[2],  (unsigned long)dbgRxHead[3]);
    printf("  f1: %08lX %08lX %08lX %08lX\n",
        (unsigned long)dbgRxHead[4],  (unsigned long)dbgRxHead[5],
        (unsigned long)dbgRxHead[6],  (unsigned long)dbgRxHead[7]);
    printf("  f2: %08lX %08lX %08lX %08lX\n",
        (unsigned long)dbgRxHead[8],  (unsigned long)dbgRxHead[9],
        (unsigned long)dbgRxHead[10], (unsigned long)dbgRxHead[11]);
    printf("  f3: %08lX %08lX %08lX %08lX\n",
        (unsigned long)dbgRxHead[12], (unsigned long)dbgRxHead[13],
        (unsigned long)dbgRxHead[14], (unsigned long)dbgRxHead[15]);

    /* One-shot TX hex dump: wait for processBlock to capture, then print */
    while (!dbgTxCaptured) { processBlock(); fillDACOutputFromGlobal(); fillSpdifOutputFromGlobal(); }
    printf("TX data (first 2 frames x 12 ch = 24 words):\n");
    printf("  f0: %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n",
        (unsigned long)dbgTxHead[0],  (unsigned long)dbgTxHead[1],
        (unsigned long)dbgTxHead[2],  (unsigned long)dbgTxHead[3],
        (unsigned long)dbgTxHead[4],  (unsigned long)dbgTxHead[5],
        (unsigned long)dbgTxHead[6],  (unsigned long)dbgTxHead[7],
        (unsigned long)dbgTxHead[8],  (unsigned long)dbgTxHead[9],
        (unsigned long)dbgTxHead[10], (unsigned long)dbgTxHead[11]);
    printf("  f1: %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n",
        (unsigned long)dbgTxHead[12], (unsigned long)dbgTxHead[13],
        (unsigned long)dbgTxHead[14], (unsigned long)dbgTxHead[15],
        (unsigned long)dbgTxHead[16], (unsigned long)dbgTxHead[17],
        (unsigned long)dbgTxHead[18], (unsigned long)dbgTxHead[19],
        (unsigned long)dbgTxHead[20], (unsigned long)dbgTxHead[21],
        (unsigned long)dbgTxHead[22], (unsigned long)dbgTxHead[23]);

    /* SPDIF diagnostic: dump raw DMA buffer and ring buffer content */
    printf("=== SPDIF RX diagnostic ===\n");
    printf("  SPDIF0_RX_CTL  = 0x%08lX\n", (unsigned long)*pREG_SPDIF0_RX_CTL);
    printf("  SPDIF0_RX_STAT = 0x%08lX (LOCK=bit3, LOCKLOSS=bit4)\n",
        (unsigned long)*pREG_SPDIF0_RX_STAT);
    printf("  ASRC0_CTL01    = 0x%08lX\n", (unsigned long)*pREG_ASRC0_CTL01);
    printf("  ASRC0_RAT01    = 0x%08lX\n", (unsigned long)*pREG_ASRC0_RAT01);
    printf("  ASRC0_MUTE     = 0x%08lX\n", (unsigned long)*pREG_ASRC0_MUTE);
    printf("  SRxDone=%lu  spdifRxRing count=%u\n",
        (unsigned long)gSpdifRxDone, (unsigned)circBuf_count(&spdifRxRing));
    /* Dump raw SPDIF DMA buffer (ping-pong readPtr) — first 8 words */
    {
        volatile uint32_t *raw = (volatile uint32_t *)spdifStream.Rx.readPtr;
        printf("  spdifStream.Rx.readPtr (raw DMA, 8 words): %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n",
            (unsigned long)raw[0], (unsigned long)raw[1],
            (unsigned long)raw[2], (unsigned long)raw[3],
            (unsigned long)raw[4], (unsigned long)raw[5],
            (unsigned long)raw[6], (unsigned long)raw[7]);
    }
    /* Dump spdifRxRing slot if available */
    if (circBuf_canRead(&spdifRxRing)) {
        const uint32_t *slot = circBuf_readSlot(&spdifRxRing);
        printf("  spdifRxRing slot (8 words): %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n",
            (unsigned long)slot[0], (unsigned long)slot[1],
            (unsigned long)slot[2], (unsigned long)slot[3],
            (unsigned long)slot[4], (unsigned long)slot[5],
            (unsigned long)slot[6], (unsigned long)slot[7]);
    } else {
        printf("  spdifRxRing: EMPTY\n");
    }

    /* Quick TX sanity check: run a few processBlock+drain cycles, then check TX count */
    for (uint32_t warmup = 0; warmup < 10; warmup++) {
        processBlock();
        fillDACOutputFromGlobal();
        fillSpdifOutputFromGlobal();
    }
    printf("TX check: JRxDone=%lu JTxDone=%lu  jTxRing=%u sTxRing=%u\n",
        (unsigned long)gJackRxDone, (unsigned long)gJackTxDone,
        (unsigned)circBuf_count(&jackTxRing), (unsigned)circBuf_count(&spdifTxRing));
#endif

    for (;;)
    {
        static uint32_t lastRx = 0;
        if ((gJackRxDone - lastRx) >= 1875) {  /* ~5s between prints */
            lastRx = gJackRxDone;
            printf("JRx=%lu JTx=%lu SRx=%lu STx=%lu  err=%lu (UF=%lu FS=%lu DMA=%lu)  "
                   "ovJR=%lu ovSR=%lu  "
                   "jRx=%lu sRx=%lu jTx=%lu sTx=%lu  "
                   "SPDIF_STAT=0x%08lX RAT01=0x%08lX\n",
                (unsigned long)gJackRxDone,
                (unsigned long)gJackTxDone,
                (unsigned long)gSpdifRxDone,
                (unsigned long)gSpdifTxDone,
                (unsigned long)gSportErr,
                (unsigned long)gTxUnderflow,
                (unsigned long)gFsErr,
                (unsigned long)gDmaErr,
                (unsigned long)gJackRxOverrun,
                (unsigned long)gSpdifRxOverrun,
                (unsigned long)circBuf_count(&jackRxRing),
                (unsigned long)circBuf_count(&spdifRxRing),
                (unsigned long)circBuf_count(&jackTxRing),
                (unsigned long)circBuf_count(&spdifTxRing),
                (unsigned long)*pREG_SPDIF0_RX_STAT,
                (unsigned long)*pREG_ASRC0_RAT01);

            /* One-shot: dump SPDIF raw data when LOCK is active (bit 3) */
            {
                static bool spdifDumpDone = false;
                if (!spdifDumpDone && (*pREG_SPDIF0_RX_STAT & 0x08u)) {
                    volatile uint32_t *raw = (volatile uint32_t *)spdifStream.Rx.readPtr;
                    printf("  LOCKED SPDIF DMA (8w): %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n",
                        (unsigned long)raw[0], (unsigned long)raw[1],
                        (unsigned long)raw[2], (unsigned long)raw[3],
                        (unsigned long)raw[4], (unsigned long)raw[5],
                        (unsigned long)raw[6], (unsigned long)raw[7]);
                    if (circBuf_canRead(&spdifRxRing)) {
                        const uint32_t *slot = circBuf_readSlot(&spdifRxRing);
                        printf("  LOCKED spdifRing (8w): %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n",
                            (unsigned long)slot[0], (unsigned long)slot[1],
                            (unsigned long)slot[2], (unsigned long)slot[3],
                            (unsigned long)slot[4], (unsigned long)slot[5],
                            (unsigned long)slot[6], (unsigned long)slot[7]);
                    }
                    spdifDumpDone = true;
                }
            }
        }

#if DIRECT_DMA_TEST == 1
        /* Mode 1: DMA plays pre-filled buffers forever — just drain RX */
        if (circBuf_canRead(&jackRxRing))  circBuf_commitRead(&jackRxRing);
        if (circBuf_canRead(&spdifRxRing)) circBuf_commitRead(&spdifRxRing);

#elif DIRECT_DMA_TEST == 2
        /* Mode 2: Write sine directly to DMA buffer via isFreshData handshake.
           Bypasses processBlock, ring buffers, and memcpy.
           Tests ONLY the ISR flip + DMA buffer write coordination. */
        {
            static float phase2 = 0.0f;
            /* Wait for ISR to signal the buffer is free */
            while (jackStream.Tx.isFreshData) {}

            uint32_t *buf = jackStream.Tx.writePtr;
            for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; ++f) {
                int32_t s = (int32_t)(sinf(phase2) * (float)0x20000000);
                buf[f * SLOTS_TX + 0] = (uint32_t)s & 0xFFFFFF00u;
                for (uint32_t ch = 1; ch < SLOTS_TX; ch++)
                    buf[f * SLOTS_TX + ch] = 0;
                phase2 += 2.0f * 3.14159265f * 1000.0f / 48000.0f;
                if (phase2 >= 2.0f * 3.14159265f) phase2 -= 2.0f * 3.14159265f;
            }
            jackStream.Tx.isFreshData = true;

            /* Drain RX to prevent overflow */
            if (circBuf_canRead(&jackRxRing))  circBuf_commitRead(&jackRxRing);
            if (circBuf_canRead(&spdifRxRing)) circBuf_commitRead(&spdifRxRing);
        }

#else
        /* Mode 0: normal software pipeline */
        processBlock();
        fillDACOutputFromGlobal();
        fillSpdifOutputFromGlobal();
#endif
    }
}
