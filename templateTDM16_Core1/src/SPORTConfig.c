#include "SPORTConfig.h"
#include "templateTDM16_Core1.h"
#include <drivers/sport/adi_sport.h>
#include "templateTDM16_Core1.h"
#include <stdio.h>
#include <sys/adi_core.h>
#include "SPUConfig.h"
#include "sharedConfig.h"
#include "AudioIO.h"
#include <sys/cache.h>
#include <stdio.h>
#include <string.h>
#include "PingPongBuffer.h"
#include <sys/platform.h>
#include <services/pcg/adi_pcg.h>
#include "sharedConfig.h"   /* for FS */

/* ===== SPORT handles and driver memory ===== */
/* SPORT4: jacks (TDM16) */
static ADI_SPORT_HANDLE 	handleSport4ATx = NULL;  /* SPORT4A -> DAC (TX) */
static ADI_SPORT_HANDLE 	handleSport4BRx = NULL;  /* SPORT4B <- ADC (RX) */
static uint8_t          	memorySport4ATx[ADI_SPORT_MEMORY_SIZE];
static uint8_t          	memorySport4BRx[ADI_SPORT_MEMORY_SIZE];

/* SPORT0: S/PDIF (I2S 2-ch) */
static ADI_SPORT_HANDLE		handleSport0ATx = NULL;  /* SPORT0A -> SPDIF_TX (TX) */
static ADI_SPORT_HANDLE 	handleSport0BRx = NULL;  /* SPORT0B <- SPDIF_RX (RX) */
static uint8_t          	memorySport0ATx[ADI_SPORT_MEMORY_SIZE];
static uint8_t          	memorySport0BRx[ADI_SPORT_MEMORY_SIZE];

/* ===== S/PDIF peripheral handles and driver memory ===== */
static ADI_SPDIF_RX_HANDLE	handleSpdifRx = NULL;
static ADI_SPDIF_TX_HANDLE	handleSpdifTx = NULL;
static uint8_t				memorySpdifRx[ADI_SPDIF_RX_MEMORY_SIZE];
static uint8_t				memorySpdifTx[ADI_SPDIF_TX_MEMORY_SIZE];

#define CHECK_RESULT(eResult) \
        if(eResult != 0)\
		{\
			return (1);\
        }

static void printEvent(uint32_t event)
{
	//printf("\n");
    if (event == ADI_SPORT_HW_ERR_NONE) {
        printf("[Callback event] No event\n");
        return;
    }

    // Print all documented bits that are set
    if (event & ADI_SPORT_HW_ERR_PRIMARY_CHNL_UNDERFLOW)
        printf("[Callback event] TX primary underflow\n");

    if (event & ADI_SPORT_HW_ERR_PRIMARY_CHNL_OVERFLOW)
        printf("[Callback event] RX primary overflow\n");

    if (event & ADI_SPORT_HW_ERR_SECONDARY_CHNL_UNDERFLOW)
        printf("[Callback event] TX secondary underflow\n");

    if (event & ADI_SPORT_HW_ERR_SECONDARY_CHNL_OVERFLOW)
        printf("[Callback event] RX secondary overflow\n");

    if (event & ADI_SPORT_HW_ERR_FS)
        printf("[Callback event] Frame‑sync error\n");

    if (event & ADI_SPORT_HW_ERR_DMA)
        printf("[Callback event] DMA error\n");

    if (event & ADI_SPORT_EVENT_RX_BUFFER_PROCESSED)
        printf("[Callback event] RX buffer processed\n");

    if (event & ADI_SPORT_EVENT_TX_BUFFER_PROCESSED)
        printf("[Callback event] TX buffer processed\n");

    if (event & ADI_SPORT_EVENT_BUFFER_PROCESSED)
        printf("[Callback event] Global group buffer processed\n");

    // Print any remaining (undocumented) bits, if present
    {
        uint32_t known =
            ADI_SPORT_HW_ERR_PRIMARY_CHNL_UNDERFLOW   |
            ADI_SPORT_HW_ERR_PRIMARY_CHNL_OVERFLOW    |
            ADI_SPORT_HW_ERR_SECONDARY_CHNL_UNDERFLOW |
            ADI_SPORT_HW_ERR_SECONDARY_CHNL_OVERFLOW  |
            ADI_SPORT_HW_ERR_FS                       |
            ADI_SPORT_HW_ERR_DMA                      |
            ADI_SPORT_EVENT_RX_BUFFER_PROCESSED       |
            ADI_SPORT_EVENT_TX_BUFFER_PROCESSED       |
            ADI_SPORT_EVENT_BUFFER_PROCESSED;

        uint32_t unknown = event & ~known;
        if (unknown) {
            printf("[Callback event] other bits: 0x%08lX\n", (unsigned long)unknown);
        }
    }
}

static inline int _chk(const char* what, ADI_SPORT_RESULT r)
{
    if (r != ADI_SPORT_SUCCESS && r != ADI_SPORT_TRANSFER_IN_PROGRESS) {
        printf("%s rc=%d\n", what, (int)r);
        return APP_FAILED;
    }
    return APP_SUCCESS;
}

void SportCallback(void *pAppHandle, uint32_t event, void *pArg)
{
    switch (event)
    {
    case ADI_SPORT_EVENT_RX_BUFFER_PROCESSED:
        if (pAppHandle == handleSport4BRx) {
            flipPingPong(&jackStream.Rx);          // wrote half becomes readPtr
            jackStream.Rx.isFreshData = true;      // CPU can consume once
            //printf("jack Rx\n");
        }
        if (pAppHandle == handleSport0BRx) {
			flipPingPong(&spdifStream.Rx);          // wrote half becomes readPtr
			spdifStream.Rx.isFreshData = true;      // CPU can consume once
			//printf("spdif Rx\n");
		}
        break;
    case ADI_SPORT_EVENT_TX_BUFFER_PROCESSED:
        if (pAppHandle == handleSport4ATx) {
            flipPingPong(&jackStream.Tx);          // read half becomes writePtr
            jackStream.Tx.isFreshData = false;     // no fresh data for SPORT now
            //printf("jack Tx\n");
        }
        if (pAppHandle == handleSport0ATx) {
			flipPingPong(&spdifStream.Tx);          // read half becomes writePtr
			spdifStream.Tx.isFreshData = false;     // no fresh data for SPORT now
			//printf("spdif Tx\n");
		}
        break;
    default:
    	//break;
    	printEvent(event);//debug
    	if (pAppHandle == handleSport4BRx) {
    	    printf("from sport4B RX\n");
    	} else if (pAppHandle == handleSport4ATx) {
    	    printf("from sport4A TX\n");
    	} else if (pAppHandle == handleSport0BRx) {
    	    printf("from sport0B RX\n");
    	} else if (pAppHandle == handleSport0ATx) {
    	    printf("from sport0A TX\n");
    	}
        break;
    }
}

int jackSportInit(void)
{
    ADI_SPORT_RESULT r;

    /* 0) Open SPORT4 halves:
       - 4A = TX  -> ADAU1962A (DAC)
       - 4B = RX <-  ADAU1979 (ADC)
       Both in Multichannel (TDM) mode.
    */
    r = adi_sport_Open(
            SportDeviceNum4,
            ADI_HALF_SPORT_A,
            ADI_SPORT_DIR_TX,
            ADI_SPORT_MC_MODE,
            memorySport4ATx,
            sizeof memorySport4ATx,
            &handleSport4ATx);
    if (_chk("Open 4A TX", r)) return APP_FAILED;

    r = adi_sport_Open(
            SportDeviceNum4,
            ADI_HALF_SPORT_B,
            ADI_SPORT_DIR_RX,
            ADI_SPORT_MC_MODE,
            memorySport4BRx,
            sizeof memorySport4BRx,
            &handleSport4BRx);
    if (_chk("Open 4B RX", r)) return APP_FAILED;

    r = adi_sport_ConfigData(
            handleSport4ATx,
            ADI_SPORT_DTYPE_SIGN_FILL, /* right-justify sign fill */
            31,                         /* SLEN */
            /*LSBFirst*/false,
            /*Pack*/false,
            /*RightJustified*/false);
    if (_chk("ConfigData 4A TX", r)) return APP_FAILED;

    r = adi_sport_ConfigData(
            handleSport4BRx,
            ADI_SPORT_DTYPE_SIGN_FILL,
            31,
            /*LSBFirst*/false,
            /*Pack*/false,
            /*RightJustified*/false);
    if (_chk("ConfigData 4B RX", r)) return APP_FAILED;

    /* 2) Clocks (external from ADAU1962A master):
       - TX should LAUNCH on FALLING (DAC samples on RISING).
       - RX must SAMPLE on FALLING because ADAU1979 changes data on RISING (BCLKEDGE=1).  [ADAU1979 DS]
    */
    r = adi_sport_ConfigClock(
            handleSport4ATx,
            /*ratio*/1,
            /*use internal*/false,      /* external BCLK */
            /*bFallingEdge*/true,       /* TX (feeding DAC): DAC latches on falling → SPORT TX should launch on rising → bFallingEdge = false.*/
            /*gated*/false);
    if (_chk("ConfigClock 4A TX (launch on FALLING)", r)) return APP_FAILED;

    r = adi_sport_ConfigClock(
            handleSport4BRx,
            /*ratio*/1,
            /*use internal*/false,      /* external BCLK */
            /*bFallingEdge*/false,       /* RX (listening to ADC): ADC changes on rising → SPORT RX should sample on falling → bFallingEdge = true. */
            /*gated*/false);
    if (_chk("ConfigClock 4B RX (sample on FALLING)", r)) return APP_FAILED;

    /* 3) Frame-sync (LRCLK) in TDM16:
       - Pulse LRCLK (you set LRCLK_MODE=1 in sharedConfig -> codecs).
       - Edge-sensitive FS (not level).
       - Detect the RISING edge of the pulse (active-high FS) in MC mode.  [SPORT API]
       - No late FS; frameDelay=0 for LJ+pulse.
    */
    const bool bEdgeSensitive = true;
    const bool bLateFS        = false;
    const bool bActiveHighFS  = true;  /* rising-edge FS in MC mode */
    const int  frameDelay     = 0;
    const bool dmaPacked      = true;  /* frame-interleaved words in memory */

    r = adi_sport_ConfigFrameSync(
            handleSport4ATx,
            /*FsDiv*/31,                /* not used in external FS, keep nominal */
            /*FSRequired*/false,
            /*internalFS*/false,
            /*dataFS*/false,
            bActiveHighFS,
            bLateFS,
            bEdgeSensitive);
    if (_chk("ConfigFS 4A TX", r)) return APP_FAILED;

    r = adi_sport_ConfigFrameSync(
            handleSport4BRx,
            /*FsDiv*/31,
            /*FSRequired*/false,
            /*internalFS*/false,
            /*dataFS*/false,
            bActiveHighFS,
            bLateFS,
            bEdgeSensitive);
    if (_chk("ConfigFS 4B RX", r)) return APP_FAILED;

    /* 4) Multichannel (TDM16):
       - frameDelay=0 (slot 0 starts at FS edge)
       - numSlots=15 (slots 0..15 = 16 slots)
       - windowSize=0 (no windowing)
       - DMAPack=true (DMA buffers are frame-interleaved)
    */
    r = adi_sport_ConfigMC(
            handleSport4ATx,
            /*frameDelay*/frameDelay,
            /*nNumSlots*/15u,
            /*windowSize*/0u,
            /*bEnableDMAPack*/dmaPacked);
    if (_chk("ConfigMC 4A TX", r)) return APP_FAILED;

    r = adi_sport_ConfigMC(
            handleSport4BRx,
            /*frameDelay*/frameDelay,
            /*nNumSlots*/15u,
            /*windowSize*/0u,
            /*bEnableDMAPack*/dmaPacked);
    if (_chk("ConfigMC 4B RX", r)) return APP_FAILED;

    /* 5) Slot selection:
       - RX: read slots 0..3 (ADAU1979 C1..C4 -> 0..3)
       - TX: drive slots 0..11 (ADAU1962A uses first 12 audio slots)
    */
    r = adi_sport_SelectChannel(handleSport4BRx, 0u, 3u);
    if (_chk("SelectCh 4B RX 0..3", r)) return APP_FAILED;

    r = adi_sport_SelectChannel(handleSport4ATx, 0u, 11u);
    if (_chk("SelectCh 4A TX 0..11", r)) return APP_FAILED;

    /* 7) Register callbacks (recommend printing events inside the callback). */

    adi_sport_RegisterCallback(handleSport4BRx, SportCallback, handleSport4BRx);
    adi_sport_RegisterCallback(handleSport4ATx, SportCallback, handleSport4ATx);

    /* 9) Submit DMA transfers for RX and TX. */
    r = adi_sport_DMATransfer(
            handleSport4BRx,
            &jackStream.Rx.dmaDescriptorPing,
            2,                         /* Ping+Pong list */
            ADI_PDMA_DESCRIPTOR_LIST,
            ADI_SPORT_CHANNEL_PRIM);
    if (_chk("DMATransfer 4B RX", r)) return APP_FAILED;

    r = adi_sport_DMATransfer(
            handleSport4ATx,
			&jackStream.Tx.dmaDescriptorPong,
            2,
            ADI_PDMA_DESCRIPTOR_LIST,
            ADI_SPORT_CHANNEL_PRIM);
    if (_chk("DMATransfer 4A TX", r)) return APP_FAILED;

    /* 10) Enable RX first, then TX. */
    r = adi_sport_Enable(handleSport4BRx, true);
    if (_chk("Enable 4B RX", r)) return APP_FAILED;

    r = adi_sport_Enable(handleSport4ATx, true);
    if (_chk("Enable 4A TX", r)) return APP_FAILED;

    printf("SPORT initialized (core=%u)\n", (unsigned)adi_core_id());
    return APP_SUCCESS;
}

int SpdifRxinit(void)
{
	ADI_SPDIF_RX_RESULT    eResult;

	eResult=adi_spdif_Rx_Open(SpdifDeviceNum0,memorySpdifRx,ADI_SPDIF_RX_MEMORY_SIZE,&handleSpdifRx);
    CHECK_RESULT(eResult);
	eResult=adi_spdif_Rx_Enable(handleSpdifRx,true);
    CHECK_RESULT(eResult);
    return 0;
}

int SpdifTxinit(void)
{
	ADI_SPDIF_TX_RESULT    eResult;

	eResult=adi_spdif_Tx_Open(SpdifDeviceNum0,memorySpdifTx,ADI_SPDIF_TX_MEMORY_SIZE,&handleSpdifTx);
    CHECK_RESULT(eResult);
	eResult=adi_spdif_Tx_Enable(handleSpdifTx,true);
    CHECK_RESULT(eResult);
    return 0;

}
int spdifSportInit(void)
{
	// init spdif
	SpdifRxinit();
	SpdifTxinit();

	ADI_SPORT_RESULT    eResult;

    eResult = adi_sport_Open(SportDeviceNum0,ADI_HALF_SPORT_A,ADI_SPORT_DIR_TX, ADI_SPORT_I2S_MODE, memorySport0ATx,ADI_SPORT_MEMORY_SIZE,&handleSport0ATx);
    CHECK_RESULT(eResult);
    eResult = adi_sport_Open(SportDeviceNum0,ADI_HALF_SPORT_B,ADI_SPORT_DIR_RX, ADI_SPORT_I2S_MODE, memorySport0BRx,ADI_SPORT_MEMORY_SIZE,&handleSport0BRx);
    CHECK_RESULT(eResult);

    eResult = adi_sport_DMATransfer(handleSport0ATx,&spdifStream.Tx.dmaDescriptorPing,2 ,ADI_PDMA_DESCRIPTOR_LIST, ADI_SPORT_CHANNEL_PRIM);
	CHECK_RESULT(eResult);
    eResult = adi_sport_DMATransfer(handleSport0BRx,&spdifStream.Rx.dmaDescriptorPing,2 ,ADI_PDMA_DESCRIPTOR_LIST, ADI_SPORT_CHANNEL_PRIM);
	CHECK_RESULT(eResult);

	eResult = adi_sport_RegisterCallback(handleSport0BRx,
										 SportCallback,
										 handleSport0BRx);
	CHECK_RESULT(eResult);
	eResult = adi_sport_RegisterCallback(handleSport0ATx,
										 SportCallback,
										 handleSport0ATx);
	CHECK_RESULT(eResult);

    eResult = adi_sport_Enable(handleSport0ATx,true);
    CHECK_RESULT(eResult);

    eResult = adi_sport_Enable(handleSport0BRx,true);
    CHECK_RESULT(eResult);
    return 0;
}


