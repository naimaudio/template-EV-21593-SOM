/*
 * clock.h — PCG and ASRC clock management
 */

#ifndef CODEC_CLOCK_H_
#define CODEC_CLOCK_H_

#include <stdint.h>
#include <stdbool.h>

void Pcginit(void);


int ASRC_init(void);
int ASRC_softReset(void);
int ASRC_deinit(void);


#endif /* CODEC_CLOCK_H_ */
