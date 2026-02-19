/*
 * adau1962.h — ADAU1962A DAC driver
 *
 *  Created on: 2 sept. 2025
 *      Author: t.cleton
 */

#ifndef CODEC_ADAU1962_H_
#define CODEC_ADAU1962_H_

#include "config.h"
#include <stdint.h>


// functions :

void ADAU1962_init(void);

// CONFIG
#define I2cAddrAdau1962         (0x04u) // TWI adress

//_____________________________ADAU1962_PLL_CLK_CTRL0_____________________________
#define ADAU1962_PLL_CLK_CTRL0			0x00 //memory adress to write to
/* PLL Input Select. Selects between MCLKI/XTALI and DLRCLK as the input to the PLL. */
#define ADAU1962_PLLIN							((uint8_t[]){0,0})//01
/* 00 XTAL Oscillator Enabled. 01 Reserved. 10 Reserved. 11 XTALO Off. XTAL Oscillator Setting. XTALO pin status. */
#define ADAU1962_XTAL_SET						((uint8_t[]){0,0})
/* 0 : normal operation, 1 device in reset */
#define ADAU1962_SOFT_RST						((uint8_t[]){0})
/* 00 : 256 x fS MCLK, 01 : 384 x fS MCLK, 10 : 512 x fS MCLK, 01 : 768 x fS MCLK */
#define ADAU1962_MCS 							((uint8_t[]){1,0})//32bitx16slots = 512
/* 0 Master PowerDown, 1 Master PowerUp */
#define ADAU1962_PUP							PUP

//_____________________________ADAU1962_PLL_CLK_CTRL1_____________________________
#define ADAU1962_PLL_CLK_CTRL1			0x01//memory adress to write to
/* 00 : I2C Register settings, 01 : Reserved, 10 : Lower Power, 11 Lowest Power*/
#define ADAU1962_LOPWR_MODE 					((uint8_t[]){0,0})
/* 00 : 4-6MHz from PLL, 01 : 8-12MHz from PLL, 10 : copies inputClock (MCLKI or XTALI), 11 : MCLKO Pin disabled */
#define ADAU1962_MCLKO_SEL						((uint8_t[]){1,0})
/*  0 : No DAC automute, 1 : DAC automute on PLL unlock */
#define ADAU1962_PLL_MUTE						PLL_MUTE
/* 0 : PLL not lock, 1 : PLL locked */
#define ADAU1962_PLL_LOCK						RESERVED1 // it's read only
/* 0 : internal voltage ref enabled, 1 : disabled */
#define	ADAU1962_VREF_EN						((uint8_t[]){1})
/* 0 : MCLK from PLL, 1 : MCLK from MCLKI or XTALI */
#define ADAU1962_CLK_SEL						((uint8_t[]){0})

//______________________________PDN_THRMSENS_CTRL_1______________________________
#define ADAU1962_PDN_THRMSENS_CTRL_1 			0x02//memory adress to write to
/* 00 : 4sec, 01 : 0.5sec, 10 : 1sec, 11 : 2sec, time interval between temperature conversions. Only used when THRM_MODE = 0 */
#define ADAU1962_THRM_RATE 						((uint8_t[]){0,0})
/* 0 : continuous temperature measurement at THRM_RATE, 1 : only when asked */
#define ADAU1962_THRM_MODE						((uint8_t[]){1})
/* 0 : reset, 1 : Convert Temperature. Toogle to get temperature */
#define ADAU1962_THRM_GO 						((uint8_t[]){1})
/* 0 : temperature sensor ON, temperature sensor OFF */
#define ADAU1962_TS_PDN							((uint8_t[]){1})
/* 0 : Voltage regulator ON, 1 : Voltage regulator OFF */
#define ADAU1962_VREG_PDN						((uint8_t[]){0})

//__________________________________PDN_CTRL2__________________________________
#define ADAU1962_PDN_CTRL2			    0x03//memory adress to write to
/* 0 : normal operation, 1 : power down */
#define ADAU1962_DAC08_PDN						((uint8_t[]){0})
#define ADAU1962_DAC07_PDN						((uint8_t[]){0})
#define ADAU1962_DAC06_PDN						((uint8_t[]){0})
#define ADAU1962_DAC05_PDN						((uint8_t[]){0})
#define ADAU1962_DAC04_PDN						((uint8_t[]){0})
#define ADAU1962_DAC03_PDN						((uint8_t[]){0})
#define ADAU1962_DAC02_PDN						((uint8_t[]){0})
#define ADAU1962_DAC01_PDN						((uint8_t[]){0})

//__________________________________PDN_CTRL3__________________________________
#define ADAU1962_PDN_CTRL3 			0x04//memory adress to write to
/* 0 : normal operation, 1 : power down */
#define ADAU1962_DAC12_PDN						((uint8_t[]){0})
#define ADAU1962_DAC11_PDN						((uint8_t[]){0})
#define ADAU1962_DAC10_PDN						((uint8_t[]){0})
#define ADAU1962_DAC09_PDN						((uint8_t[]){0})

//__________________________________DAC_CTRL0__________________________________
#define ADAU1962_DAC_CTRL0  			0x06//memory adress to write to
/* 00 : I2S, 1 BLK Cycle Delay, 01 : Left justified, 0 BLCK cycle delay, 10 : Right justified, 24-bit Data, 8 BLCK cycle delay, 11 : Right justified, 24-bit Data, 16 BLCK cycle delay,  */
#define ADAU1962_SDATA_FMT						SDATA_FMT // only used when SAI = 000
/* 000 Stereo (I2S, LJ, RJ). 001 TDM2 Octal Line. 010 TDM4 Quad Line. 011 TDM8 Dual Line. 100 TDM16 Single Line (48 kHz). 101, 110 & 111 reserved */
#define ADAU1962_SAI 							SAI
/* 00 32 kHz/44.1 kHz/48 kHz. 01 64 kHz/88.2 kHz/96 kHz. 10 128 kHz/176.4 kHz/192 kHz. 11 128 kHz/176.4 kHz/192 kHz Low Propagation Delay. */
#define ADAU1962_FS								((uint8_t[]){0,0})
/* 0 : normal, 1 : all channel muted*/
#define ADAU1962_MMUTE 							((uint8_t[]){0})

//__________________________________DAC_CTRL1__________________________________
#define ADAU1962_DAC_CTRL1   			0x07//memory adress to write to
/* 0 : normal, 1 : internal DBCLK generation */
#define ADAU1962_BCLK_GEN						((uint8_t[]){0})
/* 0 : 50% duty Cycle DLRCLK, 1 : pulse mode */
#define ADAU1962_LRCLK_MODE						LRCLK_MODE
/* 0 : normal, 1 : inverted */
#define ADAU1962_LRCLK_POL						LRCLK_POL
/* 0 : MSB first, 1 : LSB first */
#define ADAU1962_SAI_MSB						SAI_MSB
/* 0 : 32 cycles per frame, 1 : 16 cycles per frame */
#define ADAU1962_BCLK_RATE						BCLK_RATE
/* 0 : rising edge, 1 : falling edge */
#define ADAU1962_BCLK_EDGE 						BCLK_EDGE
/* 0 : DLRCLK/DBCLK slave, 1 : Master */
#define ADAU1962_SAI_MS							((uint8_t[]){1}) // ADAU1962 is used as master clock

//__________________________________DAC_CTRL2__________________________________
#define ADAU1962_DAC_CTRL2  			0x08//memory adress to write to
/* 0 : 32 cycles per channel slot, 1 : 16 */
#define ADAU1962_BCLK_TDMC 						((uint8_t[]){0})
/* 0 : non inverted, 1 : inverted signal */
#define ADAU1962_DAC_POL						((uint8_t[]){0})
/* 0 : auto mute disable, 1 : enable (after 1024 zero in)*/
#define ADAU1962_AUTO_MUTE_EN 					((uint8_t[]){0})
/* 0 : 256*fs oversampling, 1 : 128 */
#define ADAU1962_DAC_OSR 						((uint8_t[]){0})
/* 0 : no de-enphasis/flat, 1 : de-emphasis enable */
#define ADAU1962_DE_EMP_EN						((uint8_t[]){0})

//__________________________________DAC_MUTE1__________________________________
#define ADAU1962_DAC_MUTE1			    0x09//memory adress to write to
/* 0 : normal, 1 : muted */
#define ADAU1962_DAC08_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC07_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC06_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC05_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC04_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC03_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC02_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC01_MUTE						((uint8_t[]){0})

//__________________________________DAC_MUTE2__________________________________
#define ADAU1962_DAC_MUTE2  			0x0A//memory adress to write to
/* 0 : normal, 1 : muted */
#define ADAU1962_DAC12_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC11_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC10_MUTE						((uint8_t[]){0})
#define ADAU1962_DAC09_MUTE						((uint8_t[]){0})

//__________________________________DAC_MSTR_VOL__________________________________
#define ADAU1962_MSTR_VOL   			0x0B//memory adress to write to
/* 0000000 : 0dB, 00000001 -0.375dB, ...., 11111111 : -95.625dB */
#define ADAU1962_DACMSTR_VOL 					((uint8_t[]){0,0,1,1,0,1,0,1}) // 0x35 = -19.875 dB (safe for headphones)

//__________________________________DACi_VOL__________________________________
#define ADAU1962_DAC1_VOL   			0x0C//memory adress to write to
#define ADAU1962_DAC2_VOL			    0x0D//memory adress to write to
#define ADAU1962_DAC3_VOL				0x0E//memory adress to write to
#define ADAU1962_DAC4_VOL			    0x0F//memory adress to write to
#define ADAU1962_DAC5_VOL			    0x10//memory adress to write to
#define ADAU1962_DAC6_VOL			    0x11//memory adress to write to
#define ADAU1962_DAC7_VOL			    0x12//memory adress to write to
#define ADAU1962_DAC8_VOL	     		0x13//memory adress to write to
#define ADAU1962_DAC9_VOL   			0x14//memory adress to write to
#define ADAU1962_DAC10_VOL			    0x15//memory adress to write to
#define ADAU1962_DAC11_VOL			    0x16//memory adress to write to
#define ADAU1962_DAC12_VOL			    0x17//memory adress to write to
/* 0000000 : 0dB, 00000001 -0.375dB, ...., 11111111 : -95.625dB */

#define ADAU1962_DAC01_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC02_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC03_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC04_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC05_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC06_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC07_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC08_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC09_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC10_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC11_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB
#define ADAU1962_DAC12_VOL_VALUE						((uint8_t[]){0,0,0,0,0,0,0,0}) // 0dB

//__________________________________PAD_STRGTH__________________________________
#define ADAU1962_PAD_STRGTH			    0x1C//memory adress to write to
/* 0 : 4mA Drive for all Pads, 1 : 8mA Drive for all pads */
#define ADAU1962_PAD_DRV							((uint8_t[]){0})

//__________________________________DACi_POWER1__________________________________
#define ADAU1962_DAC_PWR1			    0x1D//memory adress to write to
/* 00 : low power, 01 lowest power, 10 best performance, 11 good performance */
#define ADAU1962_DAC04_POWER						DAC_POWER
#define ADAU1962_DAC03_POWER						DAC_POWER
#define ADAU1962_DAC02_POWER						DAC_POWER
#define ADAU1962_DAC01_POWER						DAC_POWER

//__________________________________DACi_POWER2__________________________________
#define ADAU1962_DAC_PWR2			    0x1E//memory adress to write to
/* 00 : low power, 01 lowest power, 10 best performance, 11 good performance */
#define ADAU1962_DAC08_POWER						DAC_POWER
#define ADAU1962_DAC07_POWER						DAC_POWER
#define ADAU1962_DAC06_POWER						DAC_POWER
#define ADAU1962_DAC05_POWER						DAC_POWER

//__________________________________DACi_POWER3__________________________________
#define ADAU1962_DAC_PWR3			    0x1F//memory adress to write to
/* 00 : low power, 01 lowest power, 10 best performance, 11 good performance */
#define ADAU1962_DAC12_POWER						DAC_POWER
#define ADAU1962_DAC11_POWER						DAC_POWER
#define ADAU1962_DAC10_POWER						DAC_POWER
#define ADAU1962_DAC09_POWER						DAC_POWER

#endif /* CODEC_ADAU1962_H_ */
