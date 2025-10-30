/*
 * SharedConfig.c
 *
 *  Created on: 13 oct. 2025
 *      Author: t.cleton
 */

#include "sharedConfig.h"

uint8_t getNumFromBits(int numArrays, uint8_t* arrays[], int sizes[]) {
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
