#include "codec/adau1979.h"
#include "platform/twi.h"
#include "config.h"
#include <stdio.h>

void writeM_POWER(void) {
    int numArrays = 3;
    int sizes[] = {1,6,1};
    uint8_t* arrays[] = {
        ADAU1979_S_RST,
		RESERVED6,
		ADAU1979_PWUP
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_M_POWER, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_M_POWER write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_M_POWER);
	printf("ADAU1979_M_POWER should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePLL_CONTROL(void) {
    int numArrays = 6;
    int sizes[] = {1,1,1,1,1,3};
    uint8_t* arrays[] = {
        ADAU1979_PLL_LOCK,
		ADAU1979_PLL_MUTE,
		RESERVED1,
		ADAU1979_CLK_S,
		RESERVED1,
		ADAU1979_MCS
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_PLL_CONTROL, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_PLL_CONTROL write failed"); }
	//*
	uint8_t word2 = TwiRead8(ADAU1979_PLL_CONTROL);
	printf("ADAU1979_PLL_CONTROL should be 0x%02X and it is 0x%02X\n", word, word2);
	//*/
}

void writeBLOCK_POWER_SAI(void) {
    int numArrays = 8;
    int sizes[] = {1,1,1,1,1,1,1,1};
    uint8_t* arrays[] = {
        ADAU1979_LR_POL,
		ADAU1979_BCLKEDGE,
		ADAU1979_LDO_EN,
		ADAU1979_VREF_EN,
		ADAU1979_ADC_EN4,
		ADAU1979_ADC_EN3,
		ADAU1979_ADC_EN2,
		ADAU1979_ADC_EN1
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_BLOCK_POWER_SAI, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_BLOCK_POWER_SAI write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_BLOCK_POWER_SAI);
	printf("ADAU1979_BLOCK_POWER_SAI should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeSAI_CTRL0(void) {
    int numArrays = 3;
    int sizes[] = {2,3,3};
    uint8_t* arrays[] = {
        ADAU1979_SDATA_FMT,
		ADAU1979_SAI,
		ADAU1979_FS
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_SAI_CTRL0, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_SAI_CTRL0 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_SAI_CTRL0);
	printf("ADAU1979_SAI_CTRL0 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeSAI_CTRL1(void) {
    int numArrays = 7;
    int sizes[] = {1,2,1,1,1,1,1};
    uint8_t* arrays[] = {
        ADAU1979_SDATA_SEL,
		ADAU1979_SLOT_WIDTH,
		ADAU1979_DATA_WIDTH,
		ADAU1979_LR_MODE,
		ADAU1979_SAI_MSB,
		ADAU1979_BCLKRATE,
		ADAU1979_SAI_MS
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_SAI_CTRL1, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_SAI_CTRL1 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_SAI_CTRL1);
	printf("ADAU1979_SAI_CTRL1 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeSAI_CMAP12(void) {
    int numArrays = 2;
    int sizes[] = {4,4};
    uint8_t* arrays[] = {
        ADAU1979_CMAP_C2,
		ADAU1979_CMAP_C1
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_SAI_CMAP12, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_SAI_CMAP12 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_SAI_CMAP12);
	printf("ADAU1979_SAI_CMAP12 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeSAI_CMAP34(void) {
    int numArrays = 2;
    int sizes[] = {4,4};
    uint8_t* arrays[] = {
        ADAU1979_CMAP_C4,
		ADAU1979_CMAP_C3
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_SAI_CMAP34, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_SAI_CMAP34 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_SAI_CMAP34);
	printf("ADAU1979_SAI_CMAP34 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeSAI_OVERTEMP(void) {
    int numArrays = 8;
    int sizes[] = {1,1,1,1,1,1,1,1};
    uint8_t* arrays[] = {
        ADAU1979_SAI_DRV_C4,
		ADAU1979_SAI_DRV_C3,
		ADAU1979_SAI_DRV_C2,
		ADAU1979_SAI_DRV_C1,
		ADAU1979_DRV_HIZ,
		RESERVED1,
		RESERVED1,
		RESERVED1,
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_SAI_OVERTEMP, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_SAI_OVERTEMP write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_SAI_OVERTEMP);
	printf("ADAU1979_SAI_OVERTEMP should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePOSTADC_GAIN1(void) {
    int numArrays = 1;
    int sizes[] = {8};
    uint8_t* arrays[] = {
        ADAU1979_PADC_GAIN1
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_POSTADC_GAIN1, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_POSTADC_GAIN1 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_POSTADC_GAIN1);
	printf("ADAU1979_POSTADC_GAIN1 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePOSTADC_GAIN2(void) {
    int numArrays = 1;
    int sizes[] = {8};
    uint8_t* arrays[] = {
        ADAU1979_PADC_GAIN2
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_POSTADC_GAIN2, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_POSTADC_GAIN2 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_POSTADC_GAIN2);
	printf("ADAU1979_POSTADC_GAIN2 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePOSTADC_GAIN3(void) {
    int numArrays = 1;
    int sizes[] = {8};
    uint8_t* arrays[] = {
        ADAU1979_PADC_GAIN3
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_POSTADC_GAIN3, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_POSTADC_GAIN3 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_POSTADC_GAIN3);
	printf("ADAU1979_POSTADC_GAIN3 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writePOSTADC_GAIN4(void) {
    int numArrays = 1;
    int sizes[] = {8};
    uint8_t* arrays[] = {
        ADAU1979_PADC_GAIN4
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_POSTADC_GAIN4, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_POSTADC_GAIN4 write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_POSTADC_GAIN4);
	printf("ADAU1979_POSTADC_GAIN4 should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeMISC_CONTROL(void) {
    int numArrays = 5;
    int sizes[] = {2,1,1,3,1};
    uint8_t* arrays[] = {
        ADAU1979_SUM_MODE,
		RESERVED1,
		ADAU1979_MMUTE,
		RESERVED3,
		ADAU1979_DC_CAL
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_MISC_CONTROL, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_MISC_CONTROL write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_MISC_CONTROL);
	printf("ADAU1979_MISC_CONTROL should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}

void writeDC_HPF_CAL(void) {
    int numArrays = 8;
    int sizes[] = {1,1,1,1,1,1,1,1};
    uint8_t* arrays[] = {
        ADAU1979_DC_SUB_C4,
		ADAU1979_DC_SUB_C3,
		ADAU1979_DC_SUB_C2,
		ADAU1979_DC_SUB_C1,
		ADAU1979_DC_HPF_C4,
		ADAU1979_DC_HPF_C3,
		ADAU1979_DC_HPF_C2,
		ADAU1979_DC_HPF_C1,
    };

    uint8_t word = getNumFromBits(numArrays, arrays, sizes);
    ADI_TWI_RESULT r = TwiWrite8(ADAU1979_DC_HPF_CAL, word);

	if (r == ADI_TWI_FAILURE){ printf("ADAU1979_DC_HPF_CAL write failed"); }
	/*
	uint8_t word2 = TwiRead8(ADAU1979_DC_HPF_CAL);
	printf("ADAU1979_DC_HPF_CAL should be 0x%02X and it is 0x%02X\n", word, word2);
	*/
}


void ADAU1979_init(void)
{
    writeM_POWER();
    writePLL_CONTROL();

    writeSAI_CTRL0();
    writeSAI_CTRL1();
    writeSAI_CMAP12();
    writeSAI_CMAP34();
    writeSAI_OVERTEMP();

    writePOSTADC_GAIN1();
    writePOSTADC_GAIN2();
    writePOSTADC_GAIN3();
    writePOSTADC_GAIN4();
    writeDC_HPF_CAL();

    writeBLOCK_POWER_SAI();  // enable ADC_ENx / set BCLKEDGE after SAI is set
    writeMISC_CONTROL();     // ensure MMUTE=0 at the very end

    delay(20);
}
