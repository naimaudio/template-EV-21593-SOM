/*
 * buffer.h — Ping-pong and circular buffer management
 *
 *  Created on: 10 oct. 2025
 *      Author: t.cleton
 */

#ifndef AUDIO_BUFFER_H_
#define AUDIO_BUFFER_H_

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
/* ===== Circular (ring) buffer ===== */
#define PRIMING_BLOCKS  2u
#define CIRC_BUF_SLOTS  4u
#define CIRC_BUF_MASK   (CIRC_BUF_SLOTS - 1u)

typedef struct CircularBuffer_
{
    uint32_t           *slots[CIRC_BUF_SLOTS];
    volatile uint32_t   writeIndex;
    volatile uint32_t   readIndex;
    uint32_t            wordsPerSlot;
    volatile uint32_t   overrunCount;
    volatile uint32_t   underrunCount;
} CircularBuffer;

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

extern uint32_t jackRxSlot0[RX_WORDS];
extern uint32_t jackRxSlot1[RX_WORDS];
extern uint32_t jackRxSlot2[RX_WORDS];
extern uint32_t jackRxSlot3[RX_WORDS];

extern uint32_t spdifRxSlot0[SPDIF_WORDS];
extern uint32_t spdifRxSlot1[SPDIF_WORDS];
extern uint32_t spdifRxSlot2[SPDIF_WORDS];
extern uint32_t spdifRxSlot3[SPDIF_WORDS];

extern uint32_t jackTxSlot0[TX_WORDS];
extern uint32_t jackTxSlot1[TX_WORDS];
extern uint32_t jackTxSlot2[TX_WORDS];
extern uint32_t jackTxSlot3[TX_WORDS];

extern uint32_t spdifTxSlot0[SPDIF_WORDS];
extern uint32_t spdifTxSlot1[SPDIF_WORDS];
extern uint32_t spdifTxSlot2[SPDIF_WORDS];
extern uint32_t spdifTxSlot3[SPDIF_WORDS];

extern bidirectionalStream jackStream;
extern bidirectionalStream spdifStream;

extern CircularBuffer jackRxRing;
extern CircularBuffer spdifRxRing;
extern CircularBuffer jackTxRing;
extern CircularBuffer spdifTxRing;

void initPingPongBuffer(	PingPong* pingPong,
							uint32_t* bufferPing,
							uint32_t* bufferPong,
							FIRST_BUFFER firstBuffer,
							uint32_t  numberOfChannels);

void initPingPongBuffers(void);
void dumpRingBufferAddresses(void);

static inline void flipPingPong(PingPong* pingPong)
{
    if (!pingPong) return;
    void* tmp = pingPong->readPtr;
    pingPong->readPtr = pingPong->writePtr;
    pingPong->writePtr = tmp;
}

/* Force a re-read of writePtr from memory (ISR may have changed it).
   Use this after spinning on isFreshData in the main loop. */
static inline uint32_t *volatileReadWritePtr(PingPong *pp)
{
    return *(uint32_t * volatile *)&pp->writePtr;
}

/* ===== Circular buffer inline helpers ===== */

static inline uint32_t circBuf_count(const CircularBuffer *cb)
{
    return cb->writeIndex - cb->readIndex;
}

static inline bool circBuf_canWrite(const CircularBuffer *cb)
{
    return circBuf_count(cb) < CIRC_BUF_SLOTS;
}

static inline bool circBuf_canRead(const CircularBuffer *cb)
{
    return circBuf_count(cb) > 0u;
}

static inline uint32_t *circBuf_writeSlot(CircularBuffer *cb)
{
    return cb->slots[cb->writeIndex & CIRC_BUF_MASK];
}

static inline void circBuf_commitWrite(CircularBuffer *cb)
{
    cb->writeIndex++;
}

static inline const uint32_t *circBuf_readSlot(const CircularBuffer *cb)
{
    return cb->slots[cb->readIndex & CIRC_BUF_MASK];
}

static inline void circBuf_commitRead(CircularBuffer *cb)
{
    cb->readIndex++;
}

/* ===== Debug helpers ===== */

static inline void printPingPongStates(void)
{
    printf(
        "JackStreamRx.isFreshData   : %d\n"
        "JackStreamTx.isFreshData   : %d\n"
        "spdifStreamRx.isFreshData  : %d\n"
        "spdifStreamTx.isFreshData  : %d\n"
        "jackRxRing.count           : %u (w=%u r=%u ovr=%u)\n"
        "spdifRxRing.count          : %u (w=%u r=%u ovr=%u)\n"
        "jackTxRing.count           : %u (w=%u r=%u ovr=%u)\n"
        "spdifTxRing.count          : %u (w=%u r=%u ovr=%u)\n"
        "-------------------------------\n",
        jackStream.Rx.isFreshData,
        jackStream.Tx.isFreshData,
        spdifStream.Rx.isFreshData,
        spdifStream.Tx.isFreshData,
        (unsigned)circBuf_count(&jackRxRing),
        (unsigned)jackRxRing.writeIndex,
        (unsigned)jackRxRing.readIndex,
        (unsigned)jackRxRing.overrunCount,
        (unsigned)circBuf_count(&spdifRxRing),
        (unsigned)spdifRxRing.writeIndex,
        (unsigned)spdifRxRing.readIndex,
        (unsigned)spdifRxRing.overrunCount,
        (unsigned)circBuf_count(&jackTxRing),
        (unsigned)jackTxRing.writeIndex,
        (unsigned)jackTxRing.readIndex,
        (unsigned)jackTxRing.overrunCount,
        (unsigned)circBuf_count(&spdifTxRing),
        (unsigned)spdifTxRing.writeIndex,
        (unsigned)spdifTxRing.readIndex,
        (unsigned)spdifTxRing.overrunCount
    );
}


#endif /* AUDIO_BUFFER_H_ */
