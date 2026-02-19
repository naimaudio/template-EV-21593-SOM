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

/* ===== Independent fill/drain functions ==================================
   Each ping-pong stream is served on its own schedule — no coupling.
   RX fills are called from the SPORT ISR — use plain word loops
   instead of memcpy to stay ISR-safe on SHARC.                             */

/* ISR-safe word copy (no library dependency) */
static inline void wordcopy(uint32_t * __restrict dst,
                            const uint32_t * __restrict src,
                            uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) dst[i] = src[i];
}

void fillGlobalInputFromAN(void)
{
    if (!jackStream.Rx.isFreshData) return;

    if (!circBuf_canWrite(&jackRxRing)) {
        jackRxRing.overrunCount++;
        jackStream.Rx.isFreshData = false;
        return;
    }

    wordcopy(circBuf_writeSlot(&jackRxRing),
             jackStream.Rx.readPtr,
             RX_WORDS);

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

    wordcopy(circBuf_writeSlot(&spdifRxRing),
             spdifStream.Rx.readPtr,
             SPDIF_WORDS);

    circBuf_commitWrite(&spdifRxRing);
    spdifStream.Rx.isFreshData = false;
}

void fillDACOutputFromGlobal(void)
{
    if (!circBuf_canRead(&jackTxRing)) return;

    /* Spin until DMA has consumed the previous buffer */
    while (jackStream.Tx.isFreshData) { /* ISR clears this */ }

    /* wordcopy instead of memcpy — SHARC memcpy may use block transfers
       that don't work correctly with uncached (0x28xxx) memory regions */
    wordcopy((uint32_t *)volatileReadWritePtr(&jackStream.Tx),
             circBuf_readSlot(&jackTxRing),
             TX_WORDS);

    jackStream.Tx.isFreshData = true;
    circBuf_commitRead(&jackTxRing);
}

void fillSpdifOutputFromGlobal(void)
{
    if (!isSPDIFactive())               return;
    if (!circBuf_canRead(&spdifTxRing)) return;
    if (spdifStream.Tx.isFreshData)     return;  /* non-blocking: SPORT0 clocks
                                                     may be dead if no SPDIF source */

    wordcopy((uint32_t *)volatileReadWritePtr(&spdifStream.Tx),
             circBuf_readSlot(&spdifTxRing),
             SPDIF_WORDS);

    spdifStream.Tx.isFreshData = true;
    circBuf_commitRead(&spdifTxRing);
}
