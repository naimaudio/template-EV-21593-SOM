/*****************************************************************************
 * templateTDM16_Core1.c
 *****************************************************************************/
#include <time.h>
#include "adi_initialize.h"
#include <sys/platform.h>
#include <sys/adi_core.h>

#include "templateTDM16_Core1.h"
#include "PingPongBuffer.h"
#include "ADAU1962Config.h"
#include "ADAU1979Config.h"
#include "sharedConfig.h"
#include "SPORTConfig.h"
#include "SoftConfig.h"
#include "SRUConfig.h"
#include "SPUConfig.h"
#include "ClockManagement.h"
#include "AudioIO.h"
#include "TWI.h"

// If you plan to disable Jacks dynamically, gate it here:
static inline bool isJackActive(void) { return true; /* or a runtime flag */ }

void prepareToPlay(void)
{
	// init things before entering the main loop

	return;
}

void processBlock(void)
{
    if (!globalStream.Rx.isFreshData){
    	//printf("processedBlock returned because globalStream.Rx.isFreshData = false\n");
        return;
    }

    const uint32_t * __restrict in  = (const uint32_t*)globalStream.Rx.readPtr;
    uint32_t       * __restrict out = (uint32_t*)      globalStream.Tx.writePtr;

    for (uint32_t f = 0; f < SAMPLES_PER_BLOCK; ++f)
    {
        const uint32_t *frameIn  = &in [(SLOTS_RX + SLOTS_SPDIF) * f];
        uint32_t       *frameOut = &out[(SLOTS_TX + SLOTS_SPDIF) * f];

        // Use your symbolic indexes for clarity
        frameOut[OUT_DAC01] = frameIn[IN_AN1];		frameOut[OUT_DAC02] = frameIn[IN_AN2];
        frameOut[OUT_DAC03] = frameIn[IN_AN3];		frameOut[OUT_DAC04] = frameIn[IN_AN4];

        frameOut[OUT_DAC05] = frameIn[IN_AN1];		frameOut[OUT_DAC06] = frameIn[IN_AN2];
        frameOut[OUT_DAC07] = frameIn[IN_AN3];  	frameOut[OUT_DAC08] = frameIn[IN_AN4];

        frameOut[OUT_DAC09] = frameIn[IN_AN1];  	frameOut[OUT_DAC10] = frameIn[IN_AN2];
        frameOut[OUT_DAC11] = frameIn[IN_AN3];  	frameOut[OUT_DAC12] = frameIn[IN_AN4];


        // SPDIF passthrough (optional)
        frameOut[OUT_SPDIF_L] = frameIn[IN_SPDIF_L];
        frameOut[OUT_SPDIF_R] = frameIn[IN_SPDIF_R];
    }

    //printf("spdif filled with data\n");
    // Hand-off
    // while (!condiftion) wait, then if condition break
    globalStream.Rx.isFreshData = false;// we just process the data so it is not fresh anymore
    flipPingPong(&globalStream.Tx);// has we processed the data we can flip buffers
    globalStream.Tx.isFreshData = true;// we just filled new data to Tx
}


int main(int argc, char *argv[])
{
    adi_initComponents();

    adi_core_enable(ADI_CORE_SHARC1);

    initPingPongBuffers();

    TwiOpen();

    ConfigureSpu();

    TwiSetAddr(I2cAddrSOFTConfig);
	if (Soft_resetAudio() != APP_SUCCESS) {printf("Soft_resetAudio failed\n");}
	Soft_init();
	Pcginit(); // master clock
	ASRC_init();
    ConfigureSru(); // routing clocks to devices

    TwiSetAddr(I2cAddrAdau1962);
    ADAU1962_init();

    TwiSetAddr(I2cAddrAdau1979);
    ADAU1979_init();

    AudioIO_setSPDIFState(SPDIF_DIGITAL_ON_OPTICAL_ON);
    AudioIO_applyConfiguration();

    jackSportInit();
    spdifSportInit();

    //TwiClose();
    prepareToPlay();

    TwiSetAddr(I2cAddrAdau1979);
    uint8_t word = TwiRead8(ADAU1979_PLL_CONTROL);
    if ((word & 0b10000000) == 0b10000000) { printf("ADAU1979_PLL_LOCKED true\n");}
    else {printf("ADAU1979_PLL_LOCKED false\n");}

    TwiSetAddr(I2cAddrAdau1962);
    uint8_t word2 = TwiRead8(ADAU1962_PLL_CLK_CTRL1);
    if ((word2 & 0b00000100) == 0b00000100) { printf("ADAU1962_PLL_LOCKED true\n");}
    else {printf("ADAU1962_PLL_LOCKED false\n");}

    TwiClose();
    printf("main loop running...\n");
    for (;;)
    {
    	//printPingPongStates();
    	fillGlobalInput();			// publish global RX
    	processBlock();				// build global TX
    	fillOutputsFromGlobal();	// seed JACK + SPDIF TX buffers once
    }
}
