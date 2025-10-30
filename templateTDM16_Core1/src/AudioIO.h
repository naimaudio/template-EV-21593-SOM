/*
 * AudioIO.h
 *
 *  Created on: 8 oct. 2025
 *      Author: t.cleton
 */

#ifndef AUDIOIO_H_
#define AUDIOIO_H_

#include <stdint.h>

// Unified input indices
#define IN_AN1        0
#define IN_AN2        1
#define IN_AN3        2
#define IN_AN4        3
#define IN_SPDIF_L    4  // present only if S/PDIF IN enabled
#define IN_SPDIF_R    5

// Unified output indices
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
#define OUT_SPDIF_L   12 // present only if S/PDIF OUT enabled
#define OUT_SPDIF_R   13
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

/* ===== Glue called by the SPORT callback =================================
   These names reflect the real endpoints (AN for ADC; DAC for outputs).     */

/* Merge SPORT4B RX (AN1..AN4) into the unified input buffer. COAX inputs
   are reserved but currently zero-filled (we'll wire SPDIF later). */
void fillGlobalInput(void);
void fillOutputsFromGlobal(void);


void fillGlobalInputFromAN(void);
void fillGlobalInputFromSpdif(void);
/* Scatter the unified output buffer to SPORT4A TX (DAC1..12) */
void fillDACOutputFromGlobal(void);
void fillSpdifOutputFromGlobal(void);
#endif /* AUDIOIO_H_ */
