/*
 * adau1979.h — ADAU1979 ADC driver
 *
 *  Created on: 3 sept. 2025
 *      Author: t.cleton
 */

#ifndef CODEC_ADAU1979_H_
#define CODEC_ADAU1979_H_

#include "config.h"
#include <stdint.h>


// functions :

void ADAU1979_init(void);

// CONFIG
#define I2cAddrAdau1979         (0x11u) // TWI adress

//_____________________________ADAU1979_M_POWER_____________________________
#define ADAU1979_M_POWER			0x00 //memory adress to write to
/* 0 : normal, 1 : software reset */
#define ADAU1979_S_RST								((uint8_t[]){0})
/* 0 : power off, 1 : power on */
#define ADAU1979_PWUP								PUP

//_____________________________ADAU1979_PLL_CONTROL_____________________________
#define ADAU1979_PLL_CONTROL			0x01 //memory adress to write to
/* 0 : PLL not lock, 1 : PLL locked */
#define ADAU1979_PLL_LOCK							RESERVED1 //its read only
/* 0 : 0 nothing, 1 : automatic mute with PLL unlock */
#define ADAU1979_PLL_MUTE							PLL_MUTE
/* 0 MCLK used for PLL input, 1 : LRCLK used */
#define ADAU1979_CLK_S								((uint8_t[]){0})//1
/* 001 : 256*fs, 010 : 384*fs, 011 : 512*fs, 100 : 768*fs, 000 : 128*fs  */
#define ADAU1979_MCS								((uint8_t[]){0,1,1}) //===================== SHARED ????????????????

//_____________________________ADAU1979_BLOCK_POWER_SAI_____________________________
#define ADAU1979_BLOCK_POWER_SAI			0x04 //memory adress to write to
/* 0 : LRCLK Low then High, 1 : LRCLK High then Low */
#define ADAU1979_LR_POL							LRCLK_POL
/* 0 : Data changes on falling Edge, 1 : rising */
#define ADAU1979_BCLKEDGE						BCLK_EDGE
/* 0 : LDO off, 1 : on */
#define ADAU1979_LDO_EN							((uint8_t[]){1})
/* 0 : Voltage Reference power down, 1 : enabled */
#define ADAU1979_VREF_EN						((uint8_t[]){1})
/* 0 : channel powered down, 1 : channel enabled */
#define ADAU1979_ADC_EN4						((uint8_t[]){1})
/* 0 : channel powered down, 1 : channel enabled */
#define ADAU1979_ADC_EN3						((uint8_t[]){1})
/* 0 : channel powered down, 1 : channel enabled */
#define ADAU1979_ADC_EN2						((uint8_t[]){1})
/* 0 : channel powered down, 1 : channel enabled */
#define ADAU1979_ADC_EN1						((uint8_t[]){1})

//_____________________________ADAU1979_SAI_CTRL0_____________________________
#define ADAU1979_SAI_CTRL0			0x05 //memory adress to write to
/* 00 : I S Data delayed from edge of LRCLK by 1 BCLK, 01 : Left justified, 10 : Right J 24b, 11 : Right J 32b */
#define ADAU1979_SDATA_FMT						SDATA_FMT
/* 000 : stereo, 001 : TDM2, 010 : TDM4, 011 : TDM8, 100 : TDM16 */
#define ADAU1979_SAI							SAI //=========================== SHARED ??????????
/* 000 : 8kHz to 12kHz, 001 : 16 to 24, 010 : 32 to 48, 011 : 64 to 96, 100 : 128 to 192 */
#define ADAU1979_FS								((uint8_t[]){0,1,0})

//_____________________________ADAU1979_SAI_CTRL1_____________________________ // ======== SHARED ??????????????
#define ADAU1979_SAI_CTRL1			0x06 //memory adress to write to
/* 0 : SDATAOUT1 used for ouput, 1 : SDATAOUT2 */
#define ADAU1979_SDATA_SEL 					((uint8_t[]){0})
/* 00 : 32BLKs per TDM slot, 01 : 24, 10 : 16, 11 : reserved */
#define ADAU1979_SLOT_WIDTH 				((uint8_t[]){0,0})
/* 0 : 24 bits, 1 : 16bits */
#define ADAU1979_DATA_WIDTH 				((uint8_t[]){0})
/* 0 : 50% duty cycle clock, 1 : pulse */
#define ADAU1979_LR_MODE					LRCLK_MODE
/* 0 : MSB first, 1 : LSB */
#define ADAU1979_SAI_MSB					SAI_MSB
/* 0 : 32BCLK/channel, 1 : 16 */
#define ADAU1979_BCLKRATE					BCLK_RATE
/* 0 : LRCLK/BCLK slave, 1 : LRCLK/BCLK master */
#define ADAU1979_SAI_MS						((uint8_t[]){0})

//_____________________________ADAU1979_SAI_CMAP12_____________________________
#define ADAU1979_SAI_CMAP12			0x07 //memory adress to write to
#define ADAU1979_CMAP_C2						((uint8_t[]){0,0,0,1}) // slot 2
#define ADAU1979_CMAP_C1						((uint8_t[]){0,0,0,0}) // slot 1

//_____________________________ADAU1979_SAI_CMAP34_____________________________
#define ADAU1979_SAI_CMAP34			0x08 //memory adress to write to
#define ADAU1979_CMAP_C4						((uint8_t[]){0,0,1,1}) // slot 4
#define ADAU1979_CMAP_C3						((uint8_t[]){0,0,1,0}) // slot 3

//_____________________________ADAU1979_SAI_OVERTEMP_____________________________
#define ADAU1979_SAI_OVERTEMP			0x09 //memory adress to write to
/* 0 : channel not driven on Serial output port, 1 : channel driven on serial output port slot determined by CMAP_Ci*/
#define ADAU1979_SAI_DRV_C4								((uint8_t[]){1})
#define ADAU1979_SAI_DRV_C3								((uint8_t[]){1})
#define ADAU1979_SAI_DRV_C2								((uint8_t[]){1})
#define ADAU1979_SAI_DRV_C1								((uint8_t[]){1})
/* 0 = unused outputs driven low, 1 = unused outputs high-Z */
#define ADAU1979_DRV_HIZ								((uint8_t[]){0})

//_____________________________ADAU1979_POSTADC_GAIN1_____________________________
#define ADAU1979_POSTADC_GAIN1			0x0A //memory adress to write to
/* 00000000 : +60dB, 00000001 : +59.625 dB, ..., 10100000 : 0dB, 11111110 : -35.625dB, 11111111 : mute */
#define ADAU1979_PADC_GAIN1				((uint8_t[]){1,1,0,0,0,0,0,0}) // 0xC0 = -12 dB

//_____________________________ADAU1979_POSTADC_GAIN2_____________________________
#define ADAU1979_POSTADC_GAIN2			0x0B //memory adress to write to
/* 00000000 : +60dB, 00000001 : +59.625 dB, ..., 10100000 : 0dB, 11111110 : -35.625dB, 11111111 : mute */
#define ADAU1979_PADC_GAIN2				((uint8_t[]){1,1,0,0,0,0,0,0}) // 0xC0 = -12 dB

//_____________________________ADAU1979_POSTADC_GAIN3_____________________________
#define ADAU1979_POSTADC_GAIN3			0x0C //memory adress to write to
/* 00000000 : +60dB, 00000001 : +59.625 dB, ..., 10100000 : 0dB, 11111110 : -35.625dB, 11111111 : mute */
#define ADAU1979_PADC_GAIN3				((uint8_t[]){1,1,0,0,0,0,0,0}) // 0xC0 = -12 dB

//_____________________________ADAU1979_POSTADC_GAIN4_____________________________
#define ADAU1979_POSTADC_GAIN4			0x0D //memory adress to write to
/* 00000000 : +60dB, 00000001 : +59.625 dB, ..., 10100000 : 0dB, 11111110 : -35.625dB, 11111111 : mute */
#define ADAU1979_PADC_GAIN4				((uint8_t[]){1,1,0,0,0,0,0,0}) // 0xC0 = -12 dB

//_____________________________ADAU1979_MISC_CONTROL_____________________________
#define ADAU1979_MISC_CONTROL			0x0E //memory adress to write to
/* 00 : normal 4channel operation, 01 : 2-channel summing operation, 10 : 1channel summing operation */
#define ADAU1979_SUM_MODE				((uint8_t[]){0,0})
/* 0 : normal, 1 : all muted  */
#define ADAU1979_MMUTE				((uint8_t[]){0})
/* 0 : normal, 1 : perform DC calibration */
#define ADAU1979_DC_CAL				((uint8_t[]){0})

//_____________________________ADAU1979_ASDC_CLIP_____________________________ (READ ONLY)
#define ADAU1979_ASDC_CLIP				0x19 // memory adress to read from

//_____________________________ADAU1979_DC_HPF_CAL_____________________________
#define ADAU1979_DC_HPF_CAL				0x1A // memory adress to write to
/* 0 : no DC subtraction, 1 : DC value from DC Calibration is Subtracted */
#define ADAU1979_DC_SUB_C4				((uint8_t[]){0})
#define ADAU1979_DC_SUB_C3				((uint8_t[]){0})
#define ADAU1979_DC_SUB_C2				((uint8_t[]){0})
#define ADAU1979_DC_SUB_C1				((uint8_t[]){0})
/* 0 : Hight pass filter OFF, 1 : ON */
#define ADAU1979_DC_HPF_C4				((uint8_t[]){1})
#define ADAU1979_DC_HPF_C3				((uint8_t[]){1})
#define ADAU1979_DC_HPF_C2				((uint8_t[]){1})
#define ADAU1979_DC_HPF_C1				((uint8_t[]){1})


#endif /* CODEC_ADAU1979_H_ */
