/*
 * config.h
 *
 * Shared project configuration.
 * Merged from: sharedConfig.h + SharedConfig.c
 *
 *  Created on: 2 sept. 2025
 *      Author: t.cleton
 */
/*					ONE TDM FRAME OF 3CHANNELS
┌─────────────────────────────────────────────────────────┐
│  		SLOT 1     		 SLOT 2  		    SLOT 3        │
│  		(Ch 1)     		 (Ch 2)    		    (Ch 3)        │
│  ┌───────────────┐ ┌───────────────┐ ┌───────────────┐  │
│  │   BITS        │ │   BITS        │ │   BITS        │  │
│  │<--32 BCLKs--->│ │<--32 BCLKs--->│ │<--32 BCLKs--->│  │
│  │ MSB .... LSB  │ │ MSB .... LSB  │ │ MSB .... LSB  │  │
└─────────────────────────────────────────────────────────┘
*/
#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <stdio.h>

#define APP_SUCCESS             (0)
#define APP_FAILED              (1)

#define FS						48000u		//used for ADAU1962 ADAU1979 and SPORT
#define MCLK_FS_RATIO			128u		//used for ADAU1962 ADAU1979 and SPORT
#define WORD_LEN				32u 		//used for ADAU1962 ADAU1979 and SPORT

// ADAU configs
	// reserved arrays
#define RESERVED1							((uint8_t[]){0})
#define RESERVED2							((uint8_t[]){0,0})
#define RESERVED3							((uint8_t[]){0,0,0})
#define RESERVED4							((uint8_t[]){0,0,0,0})
#define RESERVED5							((uint8_t[]){0,0,0,0,0})
#define RESERVED6							((uint8_t[]){0,0,0,0,0,0})
#define RESERVED7							((uint8_t[]){0,0,0,0,0,0,0})
#define RESERVED8							((uint8_t[]){0,0,0,0,0,0,0,0})
	// shared config parameters
#define PLL_LOCK							((uint8_t[]){0})
#define PLL_MUTE							((uint8_t[]){0})
#define PUP 								((uint8_t[]){1}) // POWER ON
//#define VREF_EN 							((uint8_t[]){0})
#define SDATA_FMT							((uint8_t[]){0,1}) //01->left justified 0block cycle delay
#define SAI									((uint8_t[]){1,0,0}) //100-> TDM16 packed
#define SAI_MSB								((uint8_t[]){0}) //MSB first
#define LRCLK_POL							((uint8_t[]){0})
#define LRCLK_MODE							((uint8_t[]){1}) //0->50% duty cycle DLRCLK 1->pulse mode
#define BCLK_EDGE   						((uint8_t[]){0}) // 0 => ADAU1962A samples on RISING edge
#define BCLK_RATE							((uint8_t[]){0}) //0-> 32 BCLK per TDM channel, only used when generating the BCLK
/* 00 : low power, 01 lowest power, 10 best performance, 11 good performance */
#define DAC_POWER						((uint8_t[]){1,0})

static inline uint8_t getNumFromBits(int numArrays, uint8_t* arrays[], int sizes[]) {
	// this function takes any number of arrays (numArrays), each array contains a certain number of bits given in the sizes array
	// it bascally creates a uint8_t word from all those bits so sum(sizes) should be 8.
	uint8_t result = 0;
    for (int i = 0; i < numArrays; ++i) {
        for (int j = 0; j < sizes[i]; ++j) {
            result = (result << 1) | (arrays[i][j] & 1);
        }
    }
    return result;
}

// Compute a very cheap rolling checksum over the first K words
static inline uint32_t dsum32(const uint32_t *p, uint32_t words)
{
    uint32_t s = 0;
    for (uint32_t i = 0; i < words; ++i) s = (s << 5) - s + p[i]; // s = s*31 + word
    return s;
}

static inline void ddump_head(const char* tag, const uint32_t* p, uint32_t words)
{
    const uint32_t n = (words < 8u) ? words : 8u;
    printf("%s @%p: ", tag, (void*)p);
    for (uint32_t i = 0; i < n; ++i) printf("%08X ", p[i]);
    printf("...  (sum=%08X)\n", dsum32(p, n));
}

static inline void delay(int number)
{
	for(int i = 0 ; i< number; i++)
	{
		int delay11=0xffff;
		while(delay11--)
		{
			asm("nop;");
		}
	}
}

#define CHECK_RESULT(eResult) \
        if(eResult != 0)\
		{\
			return (1);\
        }

#endif /* CONFIG_H_ */
