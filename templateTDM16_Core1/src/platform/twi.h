/*
 * twi.h — I2C (TWI) driver
 *
 *  Created on: 13 oct. 2025
 *      Author: t.cleton
 */

#ifndef PLATFORM_TWI_H_
#define PLATFORM_TWI_H_

#include <stdint.h>
#include <drivers/twi/adi_twi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TwiDevNum               (2u)
#define TwiBitrateKHz           (100u)
#define TwiDutyCyclePct         (50u)
#define TwiPrescale             (12u)

extern ADI_TWI_HANDLE   sTwiHandle;
uint8_t          sTwiDevMem[ADI_TWI_MEMORY_SIZE];
uint8_t          sTwiBuf[32]; /* small scratch buffer */
int TwiOpen(void);
int TwiSetAddr(uint16_t addr);
int TwiClose(void);
ADI_TWI_RESULT TwiWrite8(uint8_t reg, uint8_t val);
uint8_t TwiRead8(uint8_t reg);
ADI_TWI_RESULT TwiRead8_Checked(uint8_t reg, uint8_t* outValue);

#endif /* PLATFORM_TWI_H_ */
