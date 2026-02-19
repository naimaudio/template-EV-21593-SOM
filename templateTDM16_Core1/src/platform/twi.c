/*
 * twi.c — I2C (TWI) driver
 *
 *  Created on: 13 oct. 2025
 *      Author: t.cleton
 */
#include "platform/twi.h"
#include "config.h"

ADI_TWI_HANDLE   sTwiHandle = NULL;

static void delay_ms(uint32_t ms)
{
    // Simple coarse delay; at 1 GHz core, ~100k iterations ~0.1 ms (tune as needed)
    volatile uint32_t loops = ms * 100000u;
    while (loops--) { __asm__ __volatile__("nop;"); }
}

int TwiOpen(void)
{
    ADI_TWI_RESULT r;
    r = adi_twi_Open(TwiDevNum, ADI_TWI_MASTER, sTwiDevMem, sizeof sTwiDevMem, &sTwiHandle);
    if (r != ADI_TWI_SUCCESS) { printf("TWI open failed 0x%X\n", r); return APP_FAILED; }
    if ((r = adi_twi_SetPrescale(sTwiHandle, TwiPrescale)) != ADI_TWI_SUCCESS) { printf("TWI prescale fail 0x%X\n", r); return APP_FAILED; }
    if ((r = adi_twi_SetBitRate(sTwiHandle, TwiBitrateKHz)) != ADI_TWI_SUCCESS){ printf("TWI bitrate fail 0x%X\n", r);  return APP_FAILED; }
    if ((r = adi_twi_SetDutyCycle(sTwiHandle, TwiDutyCyclePct)) != ADI_TWI_SUCCESS){ printf("TWI duty fail 0x%X\n", r);  return APP_FAILED; }
    printf("TWI opened\n");
    return APP_SUCCESS;
}


/* Change the active 7-bit address without closing */
static uint16_t sActiveI2CAddr = 0xFFFF;
int TwiSetAddr(uint16_t addr)
{
    ADI_TWI_RESULT r = adi_twi_SetHardwareAddress(sTwiHandle, addr);
    if (r != ADI_TWI_SUCCESS) {
        printf("TWI: SetHardwareAddress(0x%02X) FAILED (0x%X)\n", addr, r);
        return APP_FAILED;
    }
    sActiveI2CAddr = addr;
    // Optional: small settle is not required, but the print helps debugging
    //printf("TWI: active 7-bit addr = 0x%02X\n", addr);
    return APP_SUCCESS;
}

int TwiClose(void)
{
    ADI_TWI_RESULT r = adi_twi_Close(sTwiHandle);
    sTwiHandle = NULL;
    return (r == ADI_TWI_SUCCESS) ? APP_SUCCESS : APP_FAILED;
}

/* Write an 8-bit register of the currently-selected device */
ADI_TWI_RESULT TwiWrite8(uint8_t reg, uint8_t val)
{
    sTwiBuf[0] = reg;
    sTwiBuf[1] = val;
    ADI_TWI_RESULT r = adi_twi_Write(sTwiHandle, sTwiBuf, 2u, false);
    delay(3);
    return r;
}

/* Read an 8-bit register of the currently-selected device */
uint8_t TwiRead8(uint8_t reg)
{
    ADI_TWI_RESULT r;
    uint8_t v = 0;
    sTwiBuf[0] = reg;
    r = adi_twi_Write(sTwiHandle, sTwiBuf, 1u, true);  // repeated-start
    if (r == ADI_TWI_SUCCESS) {
        r = adi_twi_Read(sTwiHandle, &v, 1u, false);
    }
    if (r != ADI_TWI_SUCCESS) {
        printf("TWI READ FAIL addr=0x%02X reg=0x%02X err=0x%X\n", sActiveI2CAddr, reg, r);
    }
    return v;
}



ADI_TWI_RESULT TwiRead8_Checked(uint8_t reg, uint8_t* outValue)
{
    if (!outValue) return ADI_TWI_FAILURE;

    ADI_TWI_RESULT r;
    uint8_t v = 0;

    sTwiBuf[0] = reg;
    r = adi_twi_Write(sTwiHandle, sTwiBuf, 1u, true);   // repeated-start
    if (r != ADI_TWI_SUCCESS) { *outValue = 0; return r; }

    r = adi_twi_Read(sTwiHandle, &v, 1u, false);
    *outValue = (r == ADI_TWI_SUCCESS) ? v : 0;
    return r;
}
