/*
 * process.c — User DSP processing
 *
 * All audio in this file is float [-1.0, +1.0].
 * Conversion between int32 (DMA) and float happens in io.c fill/drain functions.
 */

#include "app/process.h"
#include "audio/buffer.h"
#include "audio/io.h"
#include <math.h>

/* ---- Diagnostic modes (pick exactly one) ----
 * 0 = Normal: SPDIF IN → DAC 1/2 + SPDIF OUT
 * 1 = Output silence on ALL channels
 * 2 = Output 1 kHz sine on DAC01 ONLY
 * 3 = Output 1 kHz sine on DAC02 ONLY
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
#define SINE_AMP    0.5f   /* -6 dBFS */
#endif

/* ===== Helper: read float from uint32_t ring buffer slot ================= */
static inline float readSampleFloat(const uint32_t *slot)
{
    union { float floatRepresentation; uint32_t integerRepresentation; } converter;
    converter.integerRepresentation = *slot;
    return converter.floatRepresentation;
}

/* ===== Helper: write float to uint32_t ring buffer slot ================== */
static inline void writeSampleFloat(uint32_t *slot, float value)
{
    union { float floatRepresentation; uint32_t integerRepresentation; } converter;
    converter.floatRepresentation = value;
    *slot = converter.integerRepresentation;
}

void prepareToPlay(void)
{
	return;
}

void processBlock(void)
{
    if (!circBuf_canRead(&jackRxRing))   return;
    if (!circBuf_canWrite(&jackTxRing))  return;

    const uint32_t *jackInRaw  = circBuf_readSlot(&jackRxRing);
    uint32_t       *jackOutRaw = circBuf_writeSlot(&jackTxRing);

    /* SPDIF TX is optional */
    const bool canWriteSpdif = circBuf_canWrite(&spdifTxRing);
    uint32_t  *spdifOutRaw   = canWriteSpdif ? circBuf_writeSlot(&spdifTxRing) : NULL;

    /* SPDIF RX: use current data if available, otherwise hold the last block.
       The SPDIF and Jack clocks are asynchronous (ASRC bypassed), so the SPDIF
       ring occasionally runs empty for 1 block. Holding the last block avoids
       an audible click/dropout (silence) on all channels. */
    static uint32_t lastSpdifBlock[SPDIF_WORDS];
    static bool     lastSpdifBlockValid = false;

    const bool haveFreshSpdifInput = circBuf_canRead(&spdifRxRing);
    const uint32_t *spdifInRaw;

    if (haveFreshSpdifInput) {
        spdifInRaw = circBuf_readSlot(&spdifRxRing);
        /* Save a copy for hold-over when ring runs empty */
        for (uint32_t i = 0; i < SPDIF_WORDS; i++) lastSpdifBlock[i] = spdifInRaw[i];
        lastSpdifBlockValid = true;
    } else if (lastSpdifBlockValid) {
        spdifInRaw = lastSpdifBlock;   /* hold last block */
    } else {
        spdifInRaw = NULL;             /* no data yet at all */
    }

    /* Capture first 16 words of RX data once for diagnostic */
    if (!dbgRxCaptured) {
        for (uint32_t i = 0; i < 16u; i++) dbgRxHead[i] = jackInRaw[i];
        dbgRxCaptured = true;
    }

    for (uint32_t frameIndex = 0; frameIndex < SAMPLES_PER_BLOCK; ++frameIndex)
    {
        /* --- Read inputs as float --- */
        const uint32_t *analogInputFrame = &jackInRaw[SLOTS_RX * frameIndex];
        float analogInputLeft   = readSampleFloat(&analogInputFrame[IN_AN1]);
        float analogInputRight  = readSampleFloat(&analogInputFrame[IN_AN2]);
        float analogInput3      = readSampleFloat(&analogInputFrame[IN_AN3]);
        float analogInput4      = readSampleFloat(&analogInputFrame[IN_AN4]);

        float spdifInputLeft  = 0.0f;
        float spdifInputRight = 0.0f;
        if (spdifInRaw) {  /* always true once first SPDIF block arrives */
            const uint32_t *spdifInputFrame = &spdifInRaw[SLOTS_SPDIF * frameIndex];
            spdifInputLeft  = readSampleFloat(&spdifInputFrame[IN_SPDIF_L]);
            spdifInputRight = readSampleFloat(&spdifInputFrame[IN_SPDIF_R]);
        }

        /* --- Compute outputs (all in float) --- */
        float dacOutput[SLOTS_TX];
        float spdifOutputLeft;
        float spdifOutputRight;

#if DBG_MODE == 1
        /* Mode 1: silence */
        for (uint32_t ch = 0; ch < SLOTS_TX; ch++) dacOutput[ch] = 0.0f;
        spdifOutputLeft  = 0.0f;
        spdifOutputRight = 0.0f;

#elif DBG_MODE == 2
        /* Mode 2: 1 kHz sine on DAC01 only */
        {
            float sineValue = sinf(sinePhase) * SINE_AMP;
            for (uint32_t ch = 0; ch < SLOTS_TX; ch++) dacOutput[ch] = 0.0f;
            dacOutput[OUT_DAC01] = sineValue;
            spdifOutputLeft  = 0.0f;
            spdifOutputRight = 0.0f;
            sinePhase += 2.0f * 3.14159265f * SINE_FREQ / SAMPLE_RATE;
            if (sinePhase >= 2.0f * 3.14159265f) sinePhase -= 2.0f * 3.14159265f;
        }

#elif DBG_MODE == 3
        /* Mode 3: 1 kHz sine on DAC02 only */
        {
            float sineValue = sinf(sinePhase) * SINE_AMP;
            for (uint32_t ch = 0; ch < SLOTS_TX; ch++) dacOutput[ch] = 0.0f;
            dacOutput[OUT_DAC02] = sineValue;
            spdifOutputLeft  = 0.0f;
            spdifOutputRight = 0.0f;
            sinePhase += 2.0f * 3.14159265f * SINE_FREQ / SAMPLE_RATE;
            if (sinePhase >= 2.0f * 3.14159265f) sinePhase -= 2.0f * 3.14159265f;
        }

#else
        /* Mode 0: SPDIF IN → DAC 1/2 and SPDIF OUT */
        {
            /* DAC 1/2: SPDIF input (or analog fallback if no SPDIF) */
            if (spdifInRaw) {
                dacOutput[OUT_DAC01] = spdifInputLeft;
                dacOutput[OUT_DAC02] = spdifInputRight;
            } else {
                dacOutput[OUT_DAC01] = analogInputLeft;
                dacOutput[OUT_DAC02] = analogInputRight;
            }

            /* DAC 3/4: analog passthrough */
            dacOutput[OUT_DAC03] = analogInput3;
            dacOutput[OUT_DAC04] = analogInput4;

            /* DAC 5-12: silence */
            for (uint32_t ch = OUT_DAC05; ch < SLOTS_TX; ch++)
                dacOutput[ch] = 0.0f;

            /* SPDIF OUT: forward SPDIF IN */
            spdifOutputLeft  = spdifInputLeft;
            spdifOutputRight = spdifInputRight;
        }
#endif

        /* --- Write outputs as float --- */
        uint32_t *dacOutputFrame = &jackOutRaw[SLOTS_TX * frameIndex];
        for (uint32_t ch = 0; ch < SLOTS_TX; ch++)
            writeSampleFloat(&dacOutputFrame[ch], dacOutput[ch]);

        if (spdifOutRaw) {
            uint32_t *spdifOutputFrame = &spdifOutRaw[SLOTS_SPDIF * frameIndex];
            writeSampleFloat(&spdifOutputFrame[OUT_SPDIF_L], spdifOutputLeft);
            writeSampleFloat(&spdifOutputFrame[OUT_SPDIF_R], spdifOutputRight);
        }
    }

    /* Capture first 24 words of TX data once (2 frames x 12 ch) */
    if (!dbgTxCaptured) {
        for (uint32_t i = 0; i < 24u; i++) dbgTxHead[i] = jackOutRaw[i];
        dbgTxCaptured = true;
    }

    circBuf_commitRead(&jackRxRing);
    if (haveFreshSpdifInput) circBuf_commitRead(&spdifRxRing);
    circBuf_commitWrite(&jackTxRing);
    if (canWriteSpdif) circBuf_commitWrite(&spdifTxRing);
}
