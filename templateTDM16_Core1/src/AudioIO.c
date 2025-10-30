#include "AudioIO.h"
#include "PingPongBuffer.h"   // samplesPerBlock, TX/RX words/slots (frame interleaved)
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
/* ===== Fixed base channels from codecs ================================== */
#define BASE_ANALOG_INPUT_CHANNELS   4u    /* AN1..AN4 from ADAU1979 (RX slots 0..3) */
#define BASE_DAC_OUTPUT_CHANNELS     12u   /* DAC1..12 to ADAU1962  (TX slots 0..11) */

/* Maximum possible unified counts when both COAX are inputs and outputs.
   For determinism (no heap), allocate to worst-case. */
#define MAX_INPUT_CHANNELS  (BASE_ANALOG_INPUT_CHANNELS + 2u)  /* 4 + 4 = 8 */
#define MAX_OUTPUT_CHANNELS (BASE_DAC_OUTPUT_CHANNELS   + 2u)  /* 12 + 4 = 16 */

/* ===== Globals requested ================================================= */
volatile uint32_t numberOfInputChannels  = BASE_ANALOG_INPUT_CHANNELS;
volatile uint32_t numberOfOutputChannels = BASE_DAC_OUTPUT_CHANNELS;
volatile SPDIF_STATE SPDIFState = SPDIF_DIGITAL_ON_OPTICAL_ON; /* default */
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

/* ===== Glue for the SPORT callback ======================================
   Your SPORT is in TDM16, DMAPack=true. RX buffer is [frame][slot] with 4
   active slots; TX buffer is [frame][slot] with 12 active slots.           */
/* Fill unified input from AN1..4. COAX inputs are zero for now. */

void fillGlobalInput(void)
{
    // Only publish a new global block when JACK delivered one.
    if (!jackStream.Rx.isFreshData) {
    	//printf("did not fill global input buffer because jackStream.Rx.isfreshData is false\n");
    	return;
    }
    const uint32_t * __restrict srcAN  = (const uint32_t*)jackStream.Rx.readPtr;
    const uint32_t * __restrict srcSPD = (const uint32_t*)spdifStream.Rx.readPtr;
    uint32_t       * __restrict dst    = (uint32_t*)  globalStream.Rx.writePtr;

    for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; ++f) {
        const uint32_t *srcFrameAN  = &srcAN [ 4u           * f];
        const uint32_t *srcFrameSPD = &srcSPD[ SLOTS_SPDIF  * f];
        uint32_t       *dstFrame    = &dst   [(SLOTS_RX + SLOTS_SPDIF) * f];

        // Copy AN1..4
        dstFrame[0] = srcFrameAN[0];
        dstFrame[1] = srcFrameAN[1];
        dstFrame[2] = srcFrameAN[2];
        dstFrame[3] = srcFrameAN[3];

        // Copy SPDIF L/R if we have it; else zero (or keep previous—your choice)
        if (spdifStream.Rx.isFreshData) {
            dstFrame[4] = srcFrameSPD[0];
            dstFrame[5] = srcFrameSPD[1];
        } else {
            dstFrame[4] = 0;
            dstFrame[5] = 0;
        }
    }

    // Publish
    flipPingPong(&globalStream.Rx);       // wrote half becomes readPtr
    globalStream.Rx.isFreshData = true;   // new global RX is ready
    jackStream.Rx.isFreshData = false;
    spdifStream.Rx.isFreshData = false;
}




void fillOutputsFromGlobal(void)
{
    if (!globalStream.Tx.isFreshData) return;

    // ---- JACK (DAC) is the master sink: must be free
    if (!jackStream.Tx.isFreshData)
    {
        const uint32_t * __restrict src  = (const uint32_t*)globalStream.Tx.readPtr;
        uint32_t       * __restrict dst1 = (uint32_t*)      jackStream.Tx.writePtr;

        for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; ++f) {
            const uint32_t *srcFrame  = &src[(SLOTS_TX + SLOTS_SPDIF) * f];
            uint32_t       *dstFrame1 = &dst1[SLOTS_TX * f];

            // Copy 12 DAC lanes
            dstFrame1[ 0] = srcFrame[ 0]; dstFrame1[ 1] = srcFrame[ 1];
            dstFrame1[ 2] = srcFrame[ 2]; dstFrame1[ 3] = srcFrame[ 3];
            dstFrame1[ 4] = srcFrame[ 4]; dstFrame1[ 5] = srcFrame[ 5];
            dstFrame1[ 6] = srcFrame[ 6]; dstFrame1[ 7] = srcFrame[ 7];
            dstFrame1[ 8] = srcFrame[ 8]; dstFrame1[ 9] = srcFrame[ 9];
            dstFrame1[10] = srcFrame[10]; dstFrame1[11] = srcFrame[11];
        }
        jackStream.Tx.isFreshData = true;
    }

    // ---- SPDIF: update if its TX buffer is free; otherwise keep previous one
    if (isSPDIFactive() && !spdifStream.Tx.isFreshData)
    {
        const uint32_t * __restrict src  = (const uint32_t*)globalStream.Tx.readPtr;
        uint32_t       * __restrict dst2 = (uint32_t*)      spdifStream.Tx.writePtr;

        for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; ++f) {
            const uint32_t *srcFrame  = &src[(SLOTS_TX + SLOTS_SPDIF) * f];
            uint32_t       *dstFrame2 = &dst2[SLOTS_SPDIF * f];
            dstFrame2[0] = srcFrame[12];  // SPDIF L
            dstFrame2[1] = srcFrame[13];  // SPDIF R
        }
        spdifStream.Tx.isFreshData = true;
    }

    //printf("output filled\n");
    // We’ve used the current global TX (at least for JACK): advance to the next
    // (No counters; we accept that SPDIF may occasionally keep the previous block.)
    flipPingPong(&globalStream.Tx);
    globalStream.Tx.isFreshData = false;
}
