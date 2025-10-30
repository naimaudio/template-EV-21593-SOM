/*
 * PingPongBuffer.h
 *
 *  Created on: 10 oct. 2025
 *      Author: t.cleton
 */

#ifndef PINGPONGBUFFER_H_
#define PINGPONGBUFFER_H_

//#include "adi_initialize.h"
#include <drivers/sport/adi_sport.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/adi_core.h>
#include <services/int/adi_int.h>

#define SAMPLES_PER_BLOCK 128u

#define SLOTS_TX     		12u
#define SLOTS_RX     		4u
#define SLOTS_SPDIF  		2u
#define TX_WORDS     		(SLOTS_TX * SAMPLES_PER_BLOCK)
#define RX_WORDS     		(SLOTS_RX * SAMPLES_PER_BLOCK)
#define SPDIF_WORDS  		(SLOTS_SPDIF * SAMPLES_PER_BLOCK)
#define GLOBAL_TX_WORDS 	(TX_WORDS + SPDIF_WORDS)
#define GLOBAL_RX_WORDS 	(RX_WORDS + SPDIF_WORDS)

typedef enum {
	FIRST_BUFFER_IS_READ,
	FIRST_BUFFER_IS_WRITE
} FIRST_BUFFER;

typedef struct PingPong_
{
    uint32_t               *ping, *pong;

    uint32_t               *readPtr, *writePtr;

    volatile bool		   isFreshData;

    ADI_PDMA_DESC_LIST     dmaDescriptorPing;
    ADI_PDMA_DESC_LIST     dmaDescriptorPong;

} PingPong;

typedef struct bidirectionalStream
{
	PingPong				Rx;
	PingPong				Tx;
}bidirectionalStream;


extern uint32_t jackBufferTxPing[TX_WORDS];
extern uint32_t jackBufferTxPong[TX_WORDS];
extern uint32_t jackBufferRxPing[RX_WORDS];
extern uint32_t jackBufferRxPong[RX_WORDS];

extern uint32_t spdifBufferRxPing[SPDIF_WORDS]; // 2 because it is stereo stream
extern uint32_t spdifBufferRxPong[SPDIF_WORDS]; // 2 because it is stereo stream
extern uint32_t spdifBufferTxPing[SPDIF_WORDS]; // 2 because it is stereo stream
extern uint32_t spdifBufferTxPong[SPDIF_WORDS]; // 2 because it is stereo stream

extern uint32_t globalBufferRxPing[GLOBAL_RX_WORDS]; // 2 because it is stereo stream
extern uint32_t globalBufferRxPong[GLOBAL_RX_WORDS]; // 2 because it is stereo stream
extern uint32_t globalBufferTxPing[GLOBAL_TX_WORDS]; // 2 because it is stereo stream
extern uint32_t globalBufferTxPong[GLOBAL_TX_WORDS]; // 2 because it is stereo stream

extern bidirectionalStream jackStream;
extern bidirectionalStream spdifStream;
extern bidirectionalStream globalStream;

void initPingPongBuffer(	PingPong* pingPong,
							uint32_t* bufferPing,
							uint32_t* bufferPong,
							FIRST_BUFFER firstBuffer,
							uint32_t  numberOfChannels);

void initPingPongBuffers(void);

static inline void flipPingPong(PingPong* pingPong)
{
    if (!pingPong) return;
    void* tmp = pingPong->readPtr;
    pingPong->readPtr = pingPong->writePtr;
    pingPong->writePtr = tmp;
}

static inline void printPingPongStates(void)
{
    printf(
        "JackStreamRx.isFreshData   : %d\n"
        "JackStreamTx.isFreshData   : %d\n"
        "spdifStreamRx.isFreshData  : %d\n"
        "spdifStreamTx.isFreshData  : %d\n"
        "globalStreamRx.isFreshData : %d\n"
        "globalStreamTx.isFreshData : %d\n"
        "-------------------------------\n",
        jackStream.Rx.isFreshData,
        jackStream.Tx.isFreshData,
        spdifStream.Rx.isFreshData,
        spdifStream.Tx.isFreshData,
        globalStream.Rx.isFreshData,
        globalStream.Tx.isFreshData
    );
}


#endif /* PINGPONGBUFFER_H_ */
