#include "codec/adau1962.h"
#include "platform/twi.h"
#include "config.h"
#include <stdio.h>

void writePLL_CLK_CTRL0(void) {
    int numArrays = 5;
    int sizes[] = {2, 2, 1, 2, 1};
    uint8_t* arrays[] = {
        ADAU1962_PLLIN,
        ADAU1962_XTAL_SET,
        ADAU1962_SOFT_RST,
        ADAU1962_MCS,
        ADAU1962_PUP
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1962_PLL_CLK_CTRL0, word);
    if (r == ADI_TWI_FAILURE){ printf("PLL_CLK_CTRL0 write failed"); }
    /*
	uint8_t word2 = TwiRead8(ADAU1962_PLL_CLK_CTRL0);
	printf("PLL_CLK_CTRL0 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePLL_CLK_CTRL1(void) {
    int numArrays = 6;
    int sizes[] = {2, 2, 1, 1, 1, 1};
    uint8_t* arrays[] = {
        ADAU1962_LOPWR_MODE,
        ADAU1962_MCLKO_SEL,
        ADAU1962_PLL_MUTE,
        ADAU1962_PLL_LOCK,
        ADAU1962_VREF_EN,
		ADAU1962_CLK_SEL
    };
    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1962_PLL_CLK_CTRL1, word);
    if (r == ADI_TWI_FAILURE){ printf("PLL_CLK_CTRL1 write failed"); }
    //*
	uint8_t word2 = TwiRead8(ADAU1962_PLL_CLK_CTRL1);
	printf("PLL_CLK_CTRL1 should be 0x%02X and it is 0x%02X\n", word, word2);
	//*/
}

void writePDN_THRMSENS_CTRL_1(void) {
    int numArrays = 7;
    int sizes[] = {2, 1, 1, 1, 1, 1, 1};
    uint8_t* arrays[] = {
        ADAU1962_THRM_RATE,
        ADAU1962_THRM_MODE,
        ADAU1962_THRM_GO,
        RESERVED1,
        ADAU1962_TS_PDN,
		ADAU1962_VREG_PDN,
		RESERVED1
    };
    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1962_PDN_THRMSENS_CTRL_1, word);
    if (r == ADI_TWI_FAILURE){ printf("PDN_THRMSENS_CTRL_1 write failed"); }
    /*
	uint8_t word2 = TwiRead8(ADAU1962_PDN_THRMSENS_CTRL_1);
	printf("PDN_THRMSENS_CTRL_1 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePDN_CTRL2(void){
	int numArrays = 8;
	int sizes[] = {1, 1, 1, 1, 1, 1, 1, 1};
	uint8_t* arrays[] = {
		ADAU1962_DAC08_PDN,
		ADAU1962_DAC07_PDN,
		ADAU1962_DAC06_PDN,
		ADAU1962_DAC05_PDN,
		ADAU1962_DAC04_PDN,
		ADAU1962_DAC03_PDN,
		ADAU1962_DAC02_PDN,
		ADAU1962_DAC01_PDN
	};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_PDN_CTRL2, word);
	if (r == ADI_TWI_FAILURE){ printf("PDN_CTRL2 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1962_PDN_CTRL2);
	printf("PDN_CTRL2 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePDN_CTRL3(void){
	int numArrays = 8;
	int sizes[] = {1, 1, 1, 1, 1, 1, 1, 1};
	uint8_t* arrays[] = {
		RESERVED1,
		RESERVED1,
		RESERVED1,
		RESERVED1,
		ADAU1962_DAC12_PDN,
		ADAU1962_DAC11_PDN,
		ADAU1962_DAC10_PDN,
		ADAU1962_DAC09_PDN
	};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_PDN_CTRL3, word);
	if (r == ADI_TWI_FAILURE){ printf("PDN_CTRL3 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1962_PDN_CTRL3);
	printf("PDN_CTRL3 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeDAC_CTRL0(void){
	int numArrays = 4;
	int sizes[] = {2, 3, 2, 1};
	uint8_t* arrays[] = {
		ADAU1962_SDATA_FMT,
		ADAU1962_SAI,
		ADAU1962_FS,
		ADAU1962_MMUTE
	};

	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_DAC_CTRL0, word);
	if (r == ADI_TWI_FAILURE){printf("DAC_CTRL0 write failed");}
}

void writeDAC_CTRL1(void){
	int numArrays = 8;
	int sizes[] = {1, 1, 1, 1, 1, 1, 1, 1};
	uint8_t* arrays[] = {
		ADAU1962_BCLK_GEN,
		ADAU1962_LRCLK_MODE,
		ADAU1962_LRCLK_POL,
		ADAU1962_SAI_MSB,
		RESERVED1,
		ADAU1962_BCLK_RATE,
		ADAU1962_BCLK_EDGE,
		ADAU1962_SAI_MS
	};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_DAC_CTRL1, word);
	if (r == ADI_TWI_FAILURE){ printf("DAC_CRTL1 write failed"); }


}

void writeDAC_CTRL2(void){
	int numArrays = 7;
	int sizes[] = {1, 2, 1, 1, 1, 1, 1};
	uint8_t* arrays[] = {
		RESERVED1,
		RESERVED2,
		ADAU1962_BCLK_TDMC,
		ADAU1962_DAC_POL,
		ADAU1962_AUTO_MUTE_EN,
		ADAU1962_DAC_OSR,
		ADAU1962_DE_EMP_EN
	};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_DAC_CTRL2, word);
	if (r == ADI_TWI_FAILURE){ printf("DAC_CRTL2 write failed"); }
}

void writeDAC_MUTE1(void){
	int numArrays = 8;
	int sizes[] = {1, 1, 1, 1, 1, 1, 1, 1};
	uint8_t* arrays[] = {
		ADAU1962_DAC08_MUTE,
		ADAU1962_DAC07_MUTE,
		ADAU1962_DAC06_MUTE,
		ADAU1962_DAC05_MUTE,
		ADAU1962_DAC04_MUTE,
		ADAU1962_DAC03_MUTE,
		ADAU1962_DAC02_MUTE,
		ADAU1962_DAC01_MUTE
	};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_DAC_MUTE1, word);
	if (r == ADI_TWI_FAILURE){ printf("DAC_MUTE1 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1962_DAC_MUTE1);
	printf("DAC_MUTE1 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeDAC_MUTE2(void){
	int numArrays = 8;
	int sizes[] = {1, 1, 1, 1, 1, 1, 1, 1};
	uint8_t* arrays[] = {
			RESERVED1,
			RESERVED1,
			RESERVED1,
			RESERVED1,
			ADAU1962_DAC12_MUTE,
			ADAU1962_DAC11_MUTE,
			ADAU1962_DAC10_MUTE,
			ADAU1962_DAC09_MUTE
		};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_DAC_MUTE2, word);
	if (r == ADI_TWI_FAILURE){ printf("DAC_MUTE2 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1962_DAC_MUTE2);
	printf("DAC_MUTE2 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeDAC_VOL(void){
	uint8_t adresses[13] = {
				ADAU1962_MSTR_VOL,
				ADAU1962_DAC1_VOL,
				ADAU1962_DAC2_VOL,
				ADAU1962_DAC3_VOL,
				ADAU1962_DAC4_VOL,
				ADAU1962_DAC5_VOL,
				ADAU1962_DAC6_VOL,
				ADAU1962_DAC7_VOL,
				ADAU1962_DAC8_VOL,
				ADAU1962_DAC9_VOL,
				ADAU1962_DAC10_VOL,
				ADAU1962_DAC11_VOL,
				ADAU1962_DAC12_VOL,
		};

	uint8_t* values[13]  = {
			ADAU1962_DACMSTR_VOL,
			ADAU1962_DAC01_VOL_VALUE,
			ADAU1962_DAC02_VOL_VALUE,
			ADAU1962_DAC03_VOL_VALUE,
			ADAU1962_DAC04_VOL_VALUE,
			ADAU1962_DAC05_VOL_VALUE,
			ADAU1962_DAC06_VOL_VALUE,
			ADAU1962_DAC07_VOL_VALUE,
			ADAU1962_DAC08_VOL_VALUE,
			ADAU1962_DAC09_VOL_VALUE,
			ADAU1962_DAC10_VOL_VALUE,
			ADAU1962_DAC11_VOL_VALUE,
			ADAU1962_DAC12_VOL_VALUE
	};
	uint8_t word = 0;
	int numArrays = 1;
	uint8_t* arrays[1];
	int sizes[] = {8};
	for (unsigned i = 0; i < 13; ++i){
		arrays[0] = values[i];
		word = getNumFromBits(numArrays, arrays, sizes);
		ADI_TWI_RESULT r = TwiWrite8(adresses[i], word);
		if (r == ADI_TWI_FAILURE){ printf("ADAU1962_DAC%u_VOL write failed", i); }
	}
}

void writePAD_STRGTH(void){
	int numArrays = 3;
	int sizes[] = {2, 1, 5};
	uint8_t* arrays[] = {
		RESERVED2,
		ADAU1962_PAD_DRV,
		RESERVED5
	};
	uint8_t word = getNumFromBits(numArrays, arrays, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_PAD_STRGTH, word);
	if (r == ADI_TWI_FAILURE){ printf("PAD_STRGTH write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1962_PAD_STRGTH);
	printf("PAD_STRGTH should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeDAC_POWER(void){
	int numArrays = 4;
	int sizes[] = {2, 2, 2, 2};
	uint8_t* arrays1[] = {
			ADAU1962_DAC04_POWER,
			ADAU1962_DAC03_POWER,
			ADAU1962_DAC02_POWER,
			ADAU1962_DAC01_POWER
		};
	uint8_t word1 = getNumFromBits(numArrays, arrays1, sizes);
	ADI_TWI_RESULT r = TwiWrite8(ADAU1962_DAC_PWR1, word1);
	if (r == ADI_TWI_FAILURE){ printf("ADAU1962_DAC_PWR1 write failed"); }

	uint8_t* arrays2[] = {
				ADAU1962_DAC08_POWER,
				ADAU1962_DAC07_POWER,
				ADAU1962_DAC06_POWER,
				ADAU1962_DAC05_POWER
			};
	uint8_t word2 = getNumFromBits(numArrays, arrays2, sizes);
	r = TwiWrite8(ADAU1962_DAC_PWR2, word2);
	if (r == ADI_TWI_FAILURE){ printf("ADAU1962_DAC_PWR2 write failed"); }

	uint8_t* arrays3[] = {
				ADAU1962_DAC12_POWER,
				ADAU1962_DAC11_POWER,
				ADAU1962_DAC10_POWER,
				ADAU1962_DAC09_POWER
			};
	uint8_t word3 = getNumFromBits(numArrays, arrays3, sizes);
	r = TwiWrite8(ADAU1962_DAC_PWR3, word3);
	if (r == ADI_TWI_FAILURE){ 	printf("ADAU1962_DAC_PWR3 write failed"); }
}



void ADAU1962_init(void)
{
    /* 1) Start PLL */
    writePLL_CLK_CTRL0();       /* PUP=1 → PLL begins locking */
    writePLL_CLK_CTRL1();

    /* 2) Wait for PLL lock — ADAU1962A needs up to 5 ms */
    delay(20);

    /* 3) Power-down control: un-power-down all DAC channels
       (default after reset = 0xFF/0x0F = all powered down!) */
    writePDN_CTRL2();
    writePDN_CTRL3();
    writePDN_THRMSENS_CTRL_1();

    /* 4) SAI / clock configuration (requires PLL locked) */
    writeDAC_CTRL0();
    writeDAC_CTRL1();
    writeDAC_CTRL2();

    /* 5) Power mode, mute, volume */
    writeDAC_POWER();
    writeDAC_MUTE1();
    writeDAC_MUTE2();
    writeDAC_VOL();
    writePAD_STRGTH();

    writeDAC_CTRL0();    /* second write ensures MMUTE=0 */

    delay(20);

    /* Verification readback */
    printf("ADAU1962 verify: CTRL0=0x%02X(exp 0x60) CTRL1=0x%02X(exp 0x41) CTRL2=0x%02X(exp 0x00)\n",
        TwiRead8(ADAU1962_DAC_CTRL0),
        TwiRead8(ADAU1962_DAC_CTRL1),
        TwiRead8(ADAU1962_DAC_CTRL2));
}
