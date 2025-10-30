#ifndef SPORTCONFIG_H_
#define SPORTCONFIG_H_

#include <drivers/sport/adi_sport.h>
#include <drivers/spdif/adi_spdif_rx.h>
#include <drivers/spdif/adi_spdif_tx.h>

// ADC and DAC
#define SportDeviceNum4  4u // Sport4
extern uint8_t memorySport4ATx[ADI_SPORT_MEMORY_SIZE];
extern uint8_t memorySport4BRx[ADI_SPORT_MEMORY_SIZE];

// SPDIF
#define SportDeviceNum0  0u //
extern uint8_t memorySport0ATx[ADI_SPORT_MEMORY_SIZE];
extern uint8_t memorySport0BRx[ADI_SPORT_MEMORY_SIZE];

#define SpdifDeviceNum0  0u //
extern uint8_t memorySpdifRx[ADI_SPDIF_RX_MEMORY_SIZE];
extern uint8_t memorySpdifTx[ADI_SPDIF_TX_MEMORY_SIZE];

// functions

void SportCallback(void *pAppHandle, uint32_t event, void *pArg);
int jackSportInit(void);
int spdifSportInit(void);

#endif /* SPORTCONFIG_H_ */
