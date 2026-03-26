/*
 * buffer.c — Ping-pong and circular buffer management
 *
 *  Created on: 10 oct. 2025
 *      Author: t.cleton
 */
#include "audio/buffer.h"

ADI_CACHE_ALIGN uint32_t jackBufferTxPing[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackBufferTxPong[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackBufferRxPing[RX_WORDS];
ADI_CACHE_ALIGN uint32_t jackBufferRxPong[RX_WORDS];

ADI_CACHE_ALIGN uint32_t spdifBufferRxPing[SPDIF_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t spdifBufferRxPong[SPDIF_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t spdifBufferTxPing[SPDIF_WORDS]; // 2 because it is stereo stream
ADI_CACHE_ALIGN uint32_t spdifBufferTxPong[SPDIF_WORDS]; // 2 because it is stereo stream

ADI_CACHE_ALIGN uint32_t jackRxSlot0[RX_WORDS];
ADI_CACHE_ALIGN uint32_t jackRxSlot1[RX_WORDS];
ADI_CACHE_ALIGN uint32_t jackRxSlot2[RX_WORDS];
ADI_CACHE_ALIGN uint32_t jackRxSlot3[RX_WORDS];

ADI_CACHE_ALIGN uint32_t spdifRxSlot0[SPDIF_WORDS];
ADI_CACHE_ALIGN uint32_t spdifRxSlot1[SPDIF_WORDS];
ADI_CACHE_ALIGN uint32_t spdifRxSlot2[SPDIF_WORDS];
ADI_CACHE_ALIGN uint32_t spdifRxSlot3[SPDIF_WORDS];

ADI_CACHE_ALIGN uint32_t jackTxSlot0[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackTxSlot1[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackTxSlot2[TX_WORDS];
ADI_CACHE_ALIGN uint32_t jackTxSlot3[TX_WORDS];

ADI_CACHE_ALIGN uint32_t spdifTxSlot0[SPDIF_WORDS];
ADI_CACHE_ALIGN uint32_t spdifTxSlot1[SPDIF_WORDS];
ADI_CACHE_ALIGN uint32_t spdifTxSlot2[SPDIF_WORDS];
ADI_CACHE_ALIGN uint32_t spdifTxSlot3[SPDIF_WORDS];

bidirectionalStream jackStream;
bidirectionalStream spdifStream;

CircularBuffer jackRxRing;
CircularBuffer spdifRxRing;
CircularBuffer jackTxRing;
CircularBuffer spdifTxRing;



static inline void* to_uncached(void* p)
{
    return (void*)((uintptr_t)p | 0x28000000u);
}



void initPingPongBuffer(PingPong* pingPong, uint32_t* ping, uint32_t* pong, FIRST_BUFFER firstBuffer, uint32_t numberOfChannels)
{
	if (!pingPong || !ping || !pong) return;

    pingPong->ping = (uint32_t*)to_uncached(ping);
    pingPong->pong = (uint32_t*)to_uncached(pong);
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

static void initCircularBuffer(
    CircularBuffer *cb,
    uint32_t *slot0, uint32_t *slot1, uint32_t *slot2, uint32_t *slot3,
    uint32_t wordsPerSlot)
{
    if (!cb) return;

    const size_t bytesPerSlot = (size_t)wordsPerSlot * sizeof(uint32_t);

    cb->slots[0] = (uint32_t*)to_uncached(slot0);
    cb->slots[1] = (uint32_t*)to_uncached(slot1);
    cb->slots[2] = (uint32_t*)to_uncached(slot2);
    cb->slots[3] = (uint32_t*)to_uncached(slot3);

    cb->writeIndex    = 0u;
    cb->readIndex     = 0u;
    cb->wordsPerSlot  = wordsPerSlot;
    cb->overrunCount  = 0u;
    cb->underrunCount = 0u;

    for (uint32_t i = 0; i < CIRC_BUF_SLOTS; i++) {
        memset(cb->slots[i], 0, bytesPerSlot);
    }
}

void initPingPongBuffers(void)
{
	initPingPongBuffer(&jackStream.Rx, jackBufferRxPing, jackBufferRxPong, FIRST_BUFFER_IS_WRITE, SLOTS_RX);
	jackStream.Rx.isFreshData = false;
	initPingPongBuffer(&jackStream.Tx, jackBufferTxPing, jackBufferTxPong, FIRST_BUFFER_IS_WRITE, SLOTS_TX);
	jackStream.Tx.isFreshData = false;

	initPingPongBuffer(&spdifStream.Rx, spdifBufferRxPing, spdifBufferRxPong, FIRST_BUFFER_IS_WRITE, SLOTS_SPDIF);
	spdifStream.Rx.isFreshData = false;
	initPingPongBuffer(&spdifStream.Tx, spdifBufferTxPing, spdifBufferTxPong, FIRST_BUFFER_IS_WRITE, SLOTS_SPDIF);
	spdifStream.Tx.isFreshData = false;

	initCircularBuffer(&jackRxRing,   jackRxSlot0,   jackRxSlot1,   jackRxSlot2,   jackRxSlot3,   RX_WORDS);
	initCircularBuffer(&spdifRxRing, spdifRxSlot0,  spdifRxSlot1,  spdifRxSlot2,  spdifRxSlot3,  SPDIF_WORDS);
	initCircularBuffer(&jackTxRing,  jackTxSlot0,   jackTxSlot1,   jackTxSlot2,   jackTxSlot3,   TX_WORDS);
	initCircularBuffer(&spdifTxRing, spdifTxSlot0,  spdifTxSlot1,  spdifTxSlot2,  spdifTxSlot3,  SPDIF_WORDS);
}
