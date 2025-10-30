#include "SPUConfig.h"
#include <services/spu/adi_spu.h>
#include <stdio.h>

/* =========================================================================
 * SPU Source IDs for SPORT4A/4B on ADSP-2159x/SC59x (EV-SOMCRR/SPORT4).
 * These IDs match ADI’s example (SPORT_4A_SPU=71 / SPORT_4B_SPU=72).
 * ========================================================================= */
#define SPU_SRC_SPORT4A   (71u)
#define SPU_SRC_SPORT4B   (72u)

#define SPU_SRC_SPORT0A   (63u)
#define SPU_SRC_SPORT0B   (64u)

/* Local SPU handle and memory */
static ADI_SPU_HANDLE sSpuHandle = NULL;
static uint8_t        sSpuMem[ADI_SPU_MEMORY_SIZE];

/* On some BSPs adi_spu_Init signature is:
 * adi_spu_Init(uint32_t nDeviceNum, void* pMemory, ADI_CALLBACK pfCallback,
 *              void* pCBParam, ADI_SPU_HANDLE* phDevice);
 * The EV-SOMCRR example uses nDeviceNum = 0 and no callback.
 */
int ConfigureSpu(void)
{
    ADI_SPU_RESULT r;

    if (sSpuHandle != NULL) {
        /* Already configured */
        return 0;
    }

    r = adi_spu_Init(0u, sSpuMem, NULL, NULL, &sSpuHandle);
    if (r != ADI_SPU_SUCCESS) return (int)r;

    /* Set secure/non-secure attribute for SPORT4A/4B masters.
     * ADI’s example toggles the “master secure” bit similarly for its SPORT.
     */
    r = adi_spu_EnableMasterSecure(sSpuHandle, SPU_SRC_SPORT4A, true);
    if (r != ADI_SPU_SUCCESS) return (int)r;
    r = adi_spu_EnableMasterSecure(sSpuHandle, SPU_SRC_SPORT4B, true);
    if (r != ADI_SPU_SUCCESS) return (int)r;

    r = adi_spu_EnableMasterSecure(sSpuHandle, SPU_SRC_SPORT0A, true);
	if (r != ADI_SPU_SUCCESS) return (int)r;
	r = adi_spu_EnableMasterSecure(sSpuHandle, SPU_SRC_SPORT0B, true);
	if (r != ADI_SPU_SUCCESS) return (int)r;

    return 0;
}

int DeinitSpu(void)
{
    ADI_SPU_RESULT r;

    if (sSpuHandle == NULL) {
        return 0;
    }

    r = adi_spu_UnInit(sSpuHandle);
    if (r != ADI_SPU_SUCCESS) return (int)r;

    sSpuHandle = NULL;
    return 0;
}
