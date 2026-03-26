#include "audio/io.h"
#include "audio/buffer.h"
#include <stdbool.h>
#include <stdint.h>

#define BASE_ANALOG_INPUT_CHANNELS   4u
#define BASE_DAC_OUTPUT_CHANNELS     12u

volatile uint32_t numberOfInputChannels  = BASE_ANALOG_INPUT_CHANNELS;
volatile uint32_t numberOfOutputChannels = BASE_DAC_OUTPUT_CHANNELS;
volatile SPDIF_STATE SPDIFState = SPDIF_DIGITAL_ON_OPTICAL_ON;
/* ===== Public API ======================================================== */
void AudioIO_resetConfiguration(void)
{
    SPDIFState              = SPDIF_DIGITAL_ON_OPTICAL_ON;
    numberOfInputChannels   = BASE_ANALOG_INPUT_CHANNELS;
    numberOfOutputChannels  = BASE_DAC_OUTPUT_CHANNELS;
}

void AudioIO_setSPDIFState(SPDIF_STATE state)
{
    SPDIFState = state;
}

bool isSPDIFactive(void)
{
	return SPDIFState != SPDIF_DIGITAL_OFF_OPTICAL_OFF;
}

void AudioIO_applyConfiguration(void)
{
    const uint32_t numberOfSPDIFChannels  = isSPDIFactive() ? 2 : 0;

    numberOfInputChannels  = BASE_ANALOG_INPUT_CHANNELS + numberOfSPDIFChannels;
    numberOfOutputChannels = BASE_DAC_OUTPUT_CHANNELS   + numberOfSPDIFChannels;
}

/* ===== Conversion helpers ================================================
   Ring buffers store float values in uint32_t slots (same 32-bit width).
   These helpers convert at the DMA ↔ ring boundary.                        */

/* ISR-safe word copy (no library dependency, safe for uncached memory) */
static inline void wordcopy(uint32_t * __restrict dst,
                            const uint32_t * __restrict src,
                            uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) dst[i] = src[i];
}

/* Store a float into a uint32_t slot (bit-preserving, no cast UB) */
static inline void storeFloat(uint32_t *destination, float value)
{
    union { float floatRepresentation; uint32_t integerRepresentation; } converter;
    converter.floatRepresentation = value;
    *destination = converter.integerRepresentation;
}

/* Load a float from a uint32_t slot */
static inline float loadFloat(const uint32_t *source)
{
    union { float floatRepresentation; uint32_t integerRepresentation; } converter;
    converter.integerRepresentation = *source;
    return converter.floatRepresentation;
}

/* Clamp float to [-1.0, 1.0) and convert to int32 with given scale */
static inline int32_t floatToInt32Clamped(float value, float scale)
{
    if (value >= 1.0f)  return  0x7FFFFFFF;
    if (value < -1.0f)  return (int32_t)0x80000000;
    return (int32_t)(value * scale);
}

/* ===== Fill/drain functions ===============================================
   RX fills: convert int32 DMA data → float in ring buffer.
   TX drains: convert float from ring buffer → int32 DMA data.
   Each ping-pong stream is served independently.
   RX fills are called from the SPORT ISR context.                          */

void fillGlobalInputFromAN(void)
{
    if (!jackStream.Rx.isFreshData) return;

    if (!circBuf_canWrite(&jackRxRing)) {
        jackRxRing.overrunCount++;
        jackStream.Rx.isFreshData = false;
        return;
    }

    /* sourceBuffer = uncached DMA buffer (needs volatile).
       destinationSlot = cached ring buffer (no volatile needed). */
    volatile const uint32_t *sourceBuffer = jackStream.Rx.readPtr;
    uint32_t                *destinationSlot = circBuf_writeSlot(&jackRxRing);

    for (uint32_t i = 0; i < RX_WORDS; i++) {
        float normalizedSample = (float)((int32_t)sourceBuffer[i]) * SCALE_INT32_TO_FLOAT;
        storeFloat((uint32_t *)&destinationSlot[i], normalizedSample);
    }

    circBuf_commitWrite(&jackRxRing);
    jackStream.Rx.isFreshData = false;
}

void fillGlobalInputFromSpdif(void)
{
    if (!spdifStream.Rx.isFreshData) return;

    if (!circBuf_canWrite(&spdifRxRing)) {
        spdifRxRing.overrunCount++;
        spdifStream.Rx.isFreshData = false;
        return;
    }

    volatile const uint32_t *sourceBuffer = spdifStream.Rx.readPtr;
    uint32_t                *destinationSlot = circBuf_writeSlot(&spdifRxRing);

    /* SPDIF RX: MC mode captures raw I2S data from the decoder.
       bit31 = 0 always (I2S padding from decoder), bit30 = audio sign.
       Shift left by 1 to move the real sign into bit31 position. */
    for (uint32_t i = 0; i < SPDIF_WORDS; i++) {
        int32_t correctedSample = (int32_t)(sourceBuffer[i] << 1);
        float normalizedSample = (float)correctedSample * SCALE_INT32_TO_FLOAT;
        storeFloat((uint32_t *)&destinationSlot[i], normalizedSample);
    }

    circBuf_commitWrite(&spdifRxRing);
    spdifStream.Rx.isFreshData = false;
}

void fillDACOutputFromGlobal(void)
{
    if (jackStream.Tx.isFreshData) return;
    if (!circBuf_canRead(&jackTxRing)) return;

    const uint32_t          *sourceSlot = circBuf_readSlot(&jackTxRing);
    volatile uint32_t       *destinationBuffer = (volatile uint32_t *)volatileReadWritePtr(&jackStream.Tx);

    for (uint32_t i = 0; i < TX_WORDS; i++) {
        float normalizedSample = loadFloat(&sourceSlot[i]);
        destinationBuffer[i] = (uint32_t)floatToInt32Clamped(normalizedSample, SCALE_FLOAT_TO_INT32);
    }

    jackStream.Tx.isFreshData = true;
    circBuf_commitRead(&jackTxRing);
}

void fillSpdifOutputFromGlobal(void)
{
    if (!isSPDIFactive())               return;
    if (!circBuf_canRead(&spdifTxRing)) return;
    if (spdifStream.Tx.isFreshData)     return;

    const uint32_t          *sourceSlot = circBuf_readSlot(&spdifTxRing);
    volatile uint32_t       *destinationBuffer = (volatile uint32_t *)volatileReadWritePtr(&spdifStream.Tx);

    /* Convert float → int32 for SPDIF TX (same as DAC path). */
    for (uint32_t i = 0; i < SPDIF_WORDS; i++) {
        float normalizedSample = loadFloat(&sourceSlot[i]);
        destinationBuffer[i] = (uint32_t)floatToInt32Clamped(normalizedSample, SCALE_FLOAT_TO_SPDIF_TX);
    }

    spdifStream.Tx.isFreshData = true;
    circBuf_commitRead(&spdifTxRing);
}
