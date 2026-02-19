/*
 * process.c — User DSP processing
 *
 * This is where you write your audio processing code.
 * processBlock() is called from the main loop whenever data is available.
 */

#include "app/process.h"
#include "audio/buffer.h"
#include "audio/io.h"
#include <math.h>

/* ---- Diagnostic modes (pick exactly one) ----
 * 0 = Normal ADC→DAC passthrough
 * 1 = Output silence on ALL channels
 * 2 = Output 1 kHz sine on DAC01 ONLY (should be LEFT ear only)
 * 3 = Output 1 kHz sine on DAC02 ONLY (should be RIGHT ear only)
 */
#define DBG_MODE  0

/* Debug: capture first 16 words of RX data for hex dump */
volatile uint32_t dbgRxHead[16];
volatile bool     dbgRxCaptured = false;

/* Debug: capture first 24 words of TX data (2 frames x 12 ch) */
volatile uint32_t dbgTxHead[24];
volatile bool     dbgTxCaptured = false;

#if (DBG_MODE == 2) || (DBG_MODE == 3)
static float sinePhase = 0.0f;
#define SINE_FREQ   1000.0f
#define SAMPLE_RATE 48000.0f
#define SINE_AMP    0x20000000   /* ~-12 dBFS, comfortable headphone level */
#endif

void prepareToPlay(void)
{
	// init things before entering the main loop

	return;
}

void processBlock(void)
{
    /* JACK is master clock — need jack data to process */
    if (!circBuf_canRead(&jackRxRing))   return;
    if (!circBuf_canWrite(&jackTxRing))  return;

    const uint32_t * __restrict jackIn   = circBuf_readSlot(&jackRxRing);
    uint32_t       * __restrict jackOut  = circBuf_writeSlot(&jackTxRing);

    /* SPDIF TX is optional — don't let a stalled SPDIF clock block DAC output */
    const bool canWriteSpdif = circBuf_canWrite(&spdifTxRing);
    uint32_t       * __restrict spdifOut = canWriteSpdif ? circBuf_writeSlot(&spdifTxRing) : NULL;

    const bool haveSpdif = circBuf_canRead(&spdifRxRing);

    /* Capture first 16 words of RX data once for diagnostic */
    if (!dbgRxCaptured) {
        for (uint32_t i = 0; i < 16u; i++) dbgRxHead[i] = jackIn[i];
        dbgRxCaptured = true;
    }

    const uint32_t *spdifIn = haveSpdif ? circBuf_readSlot(&spdifRxRing) : NULL;

    for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; ++f)
    {
        const uint32_t *jIn  = &jackIn [SLOTS_RX * f];
        uint32_t       *jOut = &jackOut[SLOTS_TX * f];
        uint32_t       *sOut = spdifOut ? &spdifOut[SLOTS_SPDIF * f] : NULL;

#if DBG_MODE == 1
        /* Mode 1: silence */
        jOut[OUT_DAC01] = 0;  jOut[OUT_DAC02] = 0;
        jOut[OUT_DAC03] = 0;  jOut[OUT_DAC04] = 0;
        jOut[OUT_DAC05] = 0;  jOut[OUT_DAC06] = 0;
        jOut[OUT_DAC07] = 0;  jOut[OUT_DAC08] = 0;
        jOut[OUT_DAC09] = 0;  jOut[OUT_DAC10] = 0;
        jOut[OUT_DAC11] = 0;  jOut[OUT_DAC12] = 0;
        if (sOut) { sOut[OUT_SPDIF_L] = 0; sOut[OUT_SPDIF_R] = 0; }

#elif DBG_MODE == 2
        /* Mode 2: 1 kHz sine on DAC01 ONLY → should be LEFT ear */
        {
            float s = sinf(sinePhase);
            int32_t sample = (int32_t)(s * (float)SINE_AMP);
            uint32_t u = (uint32_t)sample;
            jOut[OUT_DAC01] = u;   jOut[OUT_DAC02] = 0;
            jOut[OUT_DAC03] = 0;   jOut[OUT_DAC04] = 0;
            jOut[OUT_DAC05] = 0;   jOut[OUT_DAC06] = 0;
            jOut[OUT_DAC07] = 0;   jOut[OUT_DAC08] = 0;
            jOut[OUT_DAC09] = 0;   jOut[OUT_DAC10] = 0;
            jOut[OUT_DAC11] = 0;   jOut[OUT_DAC12] = 0;
            if (sOut) { sOut[OUT_SPDIF_L] = 0; sOut[OUT_SPDIF_R] = 0; }
            sinePhase += 2.0f * 3.14159265f * SINE_FREQ / SAMPLE_RATE;
            if (sinePhase >= 2.0f * 3.14159265f) sinePhase -= 2.0f * 3.14159265f;
        }

#elif DBG_MODE == 3
        /* Mode 3: 1 kHz sine on DAC02 ONLY → should be RIGHT ear */
        {
            float s = sinf(sinePhase);
            int32_t sample = (int32_t)(s * (float)SINE_AMP);
            uint32_t u = (uint32_t)sample;
            jOut[OUT_DAC01] = 0;   jOut[OUT_DAC02] = u;
            jOut[OUT_DAC03] = 0;   jOut[OUT_DAC04] = 0;
            jOut[OUT_DAC05] = 0;   jOut[OUT_DAC06] = 0;
            jOut[OUT_DAC07] = 0;   jOut[OUT_DAC08] = 0;
            jOut[OUT_DAC09] = 0;   jOut[OUT_DAC10] = 0;
            jOut[OUT_DAC11] = 0;   jOut[OUT_DAC12] = 0;
            if (sOut) { sOut[OUT_SPDIF_L] = 0; sOut[OUT_SPDIF_R] = 0; }
            sinePhase += 2.0f * 3.14159265f * SINE_FREQ / SAMPLE_RATE;
            if (sinePhase >= 2.0f * 3.14159265f) sinePhase -= 2.0f * 3.14159265f;
        }

#else
        /* Mode 0: normal passthrough
         *
         * ---- Inputs ----
         * jIn[IN_AN1]  (0)  Analog input 1 (ADC ch1, TDM slot 0)
         * jIn[IN_AN2]  (1)  Analog input 2 (ADC ch2, TDM slot 1)
         * jIn[IN_AN3]  (2)  Analog input 3 (ADC ch3, TDM slot 2)
         * jIn[IN_AN4]  (3)  Analog input 4 (ADC ch4, TDM slot 3)
         * sIn[IN_SPDIF_L] (0)  SPDIF input left
         * sIn[IN_SPDIF_R] (1)  SPDIF input right
         *
         * ---- Outputs ----
         * jOut[OUT_DAC01] (0)   DAC 1  — headphone left
         * jOut[OUT_DAC02] (1)   DAC 2  — headphone right
         * jOut[OUT_DAC03] (2)   DAC 3
         * jOut[OUT_DAC04] (3)   DAC 4
         * jOut[OUT_DAC05] (4)   DAC 5
         * jOut[OUT_DAC06] (5)   DAC 6
         * jOut[OUT_DAC07] (6)   DAC 7
         * jOut[OUT_DAC08] (7)   DAC 8
         * jOut[OUT_DAC09] (8)   DAC 9
         * jOut[OUT_DAC10] (9)   DAC 10
         * jOut[OUT_DAC11] (10)  DAC 11
         * jOut[OUT_DAC12] (11)  DAC 12
         * sOut[OUT_SPDIF_L] (0)  SPDIF output left
         * sOut[OUT_SPDIF_R] (1)  SPDIF output right
         */
        {
            /* DAC 1/2: analog + SPDIF summed (hear both sources simultaneously) */
            const uint32_t *sIn = (haveSpdif) ? &spdifIn[SLOTS_SPDIF * f] : NULL;

            int32_t l = (int32_t)jIn[IN_AN1];
            int32_t r = (int32_t)jIn[IN_AN2];
            if (sIn) {
                l = (l >> 1) + ((int32_t)sIn[IN_SPDIF_L] >> 1);
                r = (r >> 1) + ((int32_t)sIn[IN_SPDIF_R] >> 1);
            }
            jOut[OUT_DAC01] = (uint32_t)l;
            jOut[OUT_DAC02] = (uint32_t)r;

            /* Analog passthrough on DAC 3/4 */
            jOut[OUT_DAC03] = jIn[IN_AN3];    jOut[OUT_DAC04] = jIn[IN_AN4];

            jOut[OUT_DAC05] = 0;              jOut[OUT_DAC06] = 0;
            jOut[OUT_DAC07] = 0;              jOut[OUT_DAC08] = 0;
            jOut[OUT_DAC09] = 0;              jOut[OUT_DAC10] = 0;
            jOut[OUT_DAC11] = 0;              jOut[OUT_DAC12] = 0;

            /* SPDIF TX: forward analog inputs */
            if (sOut) {
                sOut[OUT_SPDIF_L] = jIn[IN_AN1];
                sOut[OUT_SPDIF_R] = jIn[IN_AN2];
            }
        }
#endif
    }

    /* Capture first 24 words of TX data once (2 frames x 12 ch) */
    if (!dbgTxCaptured) {
        for (uint32_t i = 0; i < 24u; i++) dbgTxHead[i] = jackOut[i];
        dbgTxCaptured = true;
    }

    circBuf_commitRead(&jackRxRing);
    if (haveSpdif) circBuf_commitRead(&spdifRxRing);
    circBuf_commitWrite(&jackTxRing);
    if (canWriteSpdif) circBuf_commitWrite(&spdifTxRing);
}
