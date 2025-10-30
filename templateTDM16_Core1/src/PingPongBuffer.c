/*
 * PingPongBuffer.c
 *
 *  Created on: 10 oct. 2025
 *      Author: t.cleton
 */
#include "PingPongBuffer.h"

ADI_CACHE_ALIGN uint32_t jackBufferTxPing[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackBufferTxPong[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackBufferRxPing[RX_WORDS];
ADI_CACHE_ALIGN uint32_t jackBufferRxPong[RX_WORDS];

ADI_CACHE_ALIGN uint32_t spdifBufferRxPing[SPDIF_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t spdifBufferRxPong[SPDIF_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t spdifBufferTxPing[SPDIF_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t spdifBufferTxPong[SPDIF_WORDS]; // 2 because it is stereo stream

ADI_CACHE_ALIGN uint32_t globalBufferRxPing[GLOBAL_RX_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t globalBufferRxPong[GLOBAL_RX_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t globalBufferTxPing[GLOBAL_TX_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t globalBufferTxPong[GLOBAL_TX_WORDS]; // 2 because it is stereo stream

bidirectionalStream jackStream;
bidirectionalStream spdifStream;
bidirectionalStream globalStream;


static inline void* to_uncached(void* p)
{
    // For ADSP-2159x/SC59x L2: set the uncached alias bit(s).
    // Your memory map shows 0x0024_---- becoming 0x2824_---- (i.e., OR with 0x2800_0000).
    return (void*)((uintptr_t)p | 0x28000000u);
}


void initPingPongBuffer(PingPong* pingPong, uint32_t* ping, uint32_t* pong, FIRST_BUFFER firstBuffer, uint32_t numberOfChannels)
{
	if (!pingPong || !ping || !pong) return;

    pingPong->ping = ping;
    pingPong->pong = pong;
    // fill with zeros

    const uint32_t wordsPerBuffer = numberOfChannels * SAMPLES_PER_BLOCK;
    const size_t   bytesPerBuffer = (size_t)wordsPerBuffer * sizeof(uint32_t);

    if (pingPong->ping) memset(pingPong->ping, 0, bytesPerBuffer);
	if (pingPong->pong) memset(pingPong->pong, 0, bytesPerBuffer);

	/* Set initial roles */
	switch (firstBuffer)
	{
	case FIRST_BUFFER_IS_READ :
		pingPong->readPtr  = pingPong->ping;
		pingPong->writePtr = pingPong->pong;
		break;
	case FIRST_BUFFER_IS_WRITE:
		pingPong->readPtr  = pingPong->pong;
		pingPong->writePtr = pingPong->ping;
		break;
	}

	pingPong->dmaDescriptorPing.pStartAddr = pingPong->ping;
	pingPong->dmaDescriptorPing.pNxtDscp   = &pingPong->dmaDescriptorPong;
	pingPong->dmaDescriptorPing.Config     = ENUM_DMA_CFG_XCNT_INT;     /* use your actual PDMA config bits */
	pingPong->dmaDescriptorPing.XCount     = (uint32_t)wordsPerBuffer;  /* elements, not bytes */
	pingPong->dmaDescriptorPing.XModify    = sizeof(int);                         /* contiguous elements */
	pingPong->dmaDescriptorPing.YCount     = 0;
	pingPong->dmaDescriptorPing.YModify    = 0;

	pingPong->dmaDescriptorPong.pStartAddr = pingPong->pong;
	pingPong->dmaDescriptorPong.pNxtDscp   = &pingPong->dmaDescriptorPing;
	pingPong->dmaDescriptorPong.Config     = ENUM_DMA_CFG_XCNT_INT;
	pingPong->dmaDescriptorPong.XCount     = (uint32_t)wordsPerBuffer;
	pingPong->dmaDescriptorPong.XModify    = sizeof(int);
	pingPong->dmaDescriptorPong.YCount     = 0;
	pingPong->dmaDescriptorPong.YModify    = 0;
}

void initPingPongBuffers(void)
{
	initPingPongBuffer(&jackStream.Rx, jackBufferRxPing, jackBufferRxPong, FIRST_BUFFER_IS_READ, SLOTS_RX);
	jackStream.Rx.isFreshData = false;// no data filled yet
	initPingPongBuffer(&jackStream.Tx, jackBufferTxPing, jackBufferTxPong, FIRST_BUFFER_IS_WRITE, SLOTS_TX);
	jackStream.Tx.isFreshData = false;//can already transmit

	initPingPongBuffer(&spdifStream.Rx, spdifBufferRxPing, spdifBufferRxPong, FIRST_BUFFER_IS_READ, SLOTS_SPDIF);
	spdifStream.Rx.isFreshData = false;
	initPingPongBuffer(&spdifStream.Tx, spdifBufferTxPing, spdifBufferTxPong, FIRST_BUFFER_IS_WRITE, SLOTS_SPDIF);
	spdifStream.Tx.isFreshData = false;

	initPingPongBuffer(&globalStream.Rx, globalBufferRxPing, globalBufferRxPong, FIRST_BUFFER_IS_READ, SLOTS_RX + SLOTS_SPDIF);
	globalStream.Rx.isFreshData = false;
	initPingPongBuffer(&globalStream.Tx, globalBufferTxPing, globalBufferTxPong, FIRST_BUFFER_IS_WRITE, SLOTS_TX + SLOTS_SPDIF);
	globalStream.Tx.isFreshData = false;
}
