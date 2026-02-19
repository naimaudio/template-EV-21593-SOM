/*
 * sport.h — SPORT and SPDIF configuration
 */

#ifndef AUDIO_SPORT_H_
#define AUDIO_SPORT_H_

#include <drivers/sport/adi_sport.h>
#include <drivers/spdif/adi_spdif_rx.h>
#include <drivers/spdif/adi_spdif_tx.h>

// ADC and DAC
#define SportDeviceNum4  4u // Sport4

// SPDIF
#define SportDeviceNum0  0u //

#define SpdifDeviceNum0  0u //

//debug
extern volatile uint32_t gJackRxDone;
extern volatile uint32_t gJackTxDone;
extern volatile uint32_t gSpdifRxDone;
extern volatile uint32_t gSpdifTxDone;

extern volatile uint32_t gSportErr;
extern volatile uint32_t gTxUnderflow;
extern volatile uint32_t gFsErr;
extern volatile uint32_t gDmaErr;

extern volatile uint32_t gJackRxOverrun;
extern volatile uint32_t gSpdifRxOverrun;
// functions

void SportCallback(void *pAppHandle, uint32_t event, void *pArg);
int jackSportInit(void);
int spdifSportInit(void);
void dumpSportRegisters(void);

#endif /* AUDIO_SPORT_H_ */
