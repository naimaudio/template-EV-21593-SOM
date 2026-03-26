/*
 * io.h — Audio I/O routing and channel mapping
 *
 *  Created on: 8 oct. 2025
 *      Author: t.cleton
 */

#ifndef AUDIO_IO_H_
#define AUDIO_IO_H_

#include <stdint.h>

// Jack input indices (relative to jackRxRing frame)
#define IN_AN1        0
#define IN_AN2        1
#define IN_AN3        2
#define IN_AN4        3

// SPDIF input indices (relative to spdifRxRing frame)
#define IN_SPDIF_L    0
#define IN_SPDIF_R    1

// Jack output indices (relative to jackTxRing frame)
#define OUT_DAC01      0
#define OUT_DAC02      1
#define OUT_DAC03      2
#define OUT_DAC04      3
#define OUT_DAC05      4
#define OUT_DAC06      5
#define OUT_DAC07      6
#define OUT_DAC08      7
#define OUT_DAC09      8
#define OUT_DAC10      9
#define OUT_DAC11      10
#define OUT_DAC12      11

// SPDIF output indices (relative to spdifTxRing frame)
#define OUT_SPDIF_L   0
#define OUT_SPDIF_R   1
/* ===== Combined COAX state (exactly one of these) ======================= */
typedef enum {
    SPDIF_DIGITAL_ON_OPTICAL_ON,  // +2 input from RCA IN or OPTICAL IN	;	+2 output to both RCA OUT and OPTICAL OUT
	SPDIF_DIGITAL_ON_OPTICAL_OFF, // +2 input from RCA IN				;	+2 output to RCA OUT
	SPDIF_DIGITAL_OFF_OPTICAL_ON, // +2 input from OPTICAL IN			;	+2 output to OPTICAL OUT
	SPDIF_DIGITAL_OFF_OPTICAL_OFF // +0 input							;	+0 output
} SPDIF_STATE;

/* ===== Global topology (you requested globals for clarity) ============== */
/* Total channel counts for the unified block (depend only on coaxState) */
extern volatile uint32_t numberOfInputChannels;   /* 4 + 2*#(COAX inputs)  */
extern volatile uint32_t numberOfOutputChannels;  /* 12 + 2*#(COAX outputs)*/

/* The current COAX role state driving the counts */
extern volatile SPDIF_STATE SPDIFState;

/* ===== Topology configuration =========================================== */
void AudioIO_resetConfiguration(void);

void AudioIO_setSPDIFState(SPDIF_STATE state);

bool isSPDIFactive(void);

void AudioIO_applyConfiguration(void);

/* ===== Int32 ↔ Float conversion ==========================================
   Ring buffers carry float data (stored as uint32_t bit patterns).
   Conversion happens at the DMA ↔ ring boundary in the fill/drain functions.

   ADC/DAC path (32-bit TDM, 24-bit audio left-justified in bits [31:8]):
     Full-scale 24-bit audio = 0x7FFFFF00 ≈ 2^31.  Scale = 1/2^31.

   SPDIF RX path (24-bit capture via SLEN=23, sign-extended to 32-bit):
     Range ±2^23.  I2S 1-BCLK delay doubles values → full-scale = ±2^23.
     Scale = 1/2^23 so that 0 dBFS input → ±1.0f (the 2× I2S gain is absorbed).

   SPDIF TX path (32-bit TDM via SPORT0A, SLEN=31):
     Encoder reads upper bits.  Scale = 2^31 (same as DAC).                 */
#define SCALE_INT32_TO_FLOAT        (1.0f / 2147483648.0f)   /* 1 / 2^31 — for 32-bit paths (ADC/DAC) */
#define SCALE_FLOAT_TO_INT32        (2147483648.0f)           /* 2^31     — for 32-bit paths (ADC/DAC) */
#define SCALE_INT24_TO_FLOAT        (1.0f / 8388608.0f)       /* 1 / 2^23 — for 24-bit SPDIF RX (absorbs I2S 2× gain) */
#define SCALE_FLOAT_TO_SPDIF_TX     (2147483648.0f)           /* 2^31     — same as DAC.
                                                                  Without SMODEIN, encoder reads bits[31:8]
                                                                  directly. ±1.0f → bits[31:8] = ±2^23 = 24-bit full scale. */

/* ===== Fill/drain functions (each stream independent) ==================== */
void fillGlobalInputFromAN(void);
void fillGlobalInputFromSpdif(void);
void fillDACOutputFromGlobal(void);
void fillSpdifOutputFromGlobal(void);
#endif /* AUDIO_IO_H_ */
