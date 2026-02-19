// soft_switch.c — templateTDM16_Core1 (Core1)
// Configures the EV-SOMCRR-EZKIT soft-switch (MCP23017 at 0x22)
// to enable ADAU1962A/ADAU1979 and performs an ADAU reset pulse.
// Matches the behavior of ADI's example SoftConfig.
//
// IMPORTANT: Call TwiOpen() first, and TwiSetAddr(I2cAddrSOFTConfig) before Soft_init().

#include <stdint.h>
#include "platform/twi.h"
#include "platform/soft_switch.h"
#include "config.h"


// MCP23017 (Mirrors the addresses used in the ADI example)
#define REG_IODIRA  0x00u
#define REG_IODIRB  0x01u
#define REG_GPIOA   0x12u
#define REG_GPIOB   0x13u
// Direction and output patterns taken from the ADI example:
//  - IODIRA = 0x18 : PA[4:3] inputs, others outputs
//  - IODIRB = 0x00 : all Port B pins as outputs
//  - GPIOB  = 0xFD : preserves SPDIF and others like the example
//  - GPIOA  = 0x05 then 0x25 : assert ~ADAU_RESET low then release it high
//                               (PA5 is ~ADAU_RESET; PA7/PA6 = ~1979_EN/~1962_EN kept low to enable)
// NOTE: These constants replicate the ADI reference behavior.
#define IODIRA_DIR_VALUE   0x18u
#define IODIRB_DIR_VALUE   0x00u
#define GPIOB_INIT_VALUE   0xFDu
#define GPIOA_ASSERT_RST   0x05u
#define GPIOA_RELEASE_RST  0x25u


#define MCP_IODIRA  0x00
#define MCP_IODIRB  0x01
#define MCP_GPIOA   0x12
#define MCP_GPIOB   0x13


// small busy-wait
static void softcfg_delay_ms(uint32_t ms)
{
    volatile uint32_t loops = ms * 100000u; // coarse (tune if needed)
    while (loops--) { __asm__ __volatile__("nop;"); }
}

int Soft_resetAudio(void)
{
    if (TwiSetAddr(I2cAddrSOFTConfig) != APP_SUCCESS) return APP_FAILED;

    // 1) Direction: match EZKIT examples: Port A some inputs (0x18), Port B all outputs (0x00)
    //    (IODIRA=0x00, IODIRB=0x01 per MCP23017 datasheet; values from ADI example)
    if (TwiWrite8(MCP_IODIRA, 0x18) != ADI_TWI_SUCCESS) return APP_FAILED;
    if (TwiWrite8(MCP_IODIRB, 0x00) != ADI_TWI_SUCCESS) return APP_FAILED;

    // 2) Assert reset first (same pattern used in ADC/DAC softswitch example): GPIOA=0x05, GPIOB=0xFD
    //    Then release reset (ADAU_Reset example): GPIOA=0x25, GPIOB=0xFD.
    //    These constants come from ADI's reference SoftConfig files for this board.
    if (TwiWrite8(MCP_GPIOA, 0x05) != ADI_TWI_SUCCESS) return APP_FAILED; // keep ADAU in reset
    if (TwiWrite8(MCP_GPIOB, 0xFD) != ADI_TWI_SUCCESS) return APP_FAILED;
    softcfg_delay_ms(5);

    // release ADAU reset & keep rails enabled
    if (TwiWrite8(MCP_GPIOA, 0x25) != ADI_TWI_SUCCESS) return APP_FAILED;
    // GPIOB unchanged (0xFD), but repeat to be explicit if you want:
    if (TwiWrite8(MCP_GPIOB, 0xFD) != ADI_TWI_SUCCESS) return APP_FAILED;

    int delay11=0xffff;
	while(delay11--)
	{
		asm("nop;");
	}
    return APP_SUCCESS;
}

int Soft_init(void)
{
    // 1) Set directions
    if (TwiWrite8(REG_IODIRA, IODIRA_DIR_VALUE) != 0) return 1;
    if (TwiWrite8(REG_IODIRB, IODIRB_DIR_VALUE) != 0) return 1;

    // 2) Initialize Port B (matches ADI example)
    if (TwiWrite8(REG_GPIOB, GPIOB_INIT_VALUE) != 0) return 1;

    // 3) Pulse ADAU reset via Port A:
    //    - First drive ~ADAU_RESET low (assert) while keeping ~EN lines low (enabled)
    if (TwiWrite8(REG_GPIOA, GPIOA_ASSERT_RST) != 0) return 1;
    softcfg_delay_ms(5);

    //    - Release reset: ~ADAU_RESET high, ~EN still low (= enabled)
    if (TwiWrite8(REG_GPIOA, GPIOA_RELEASE_RST) != 0) return 1;
    softcfg_delay_ms(5);

    // Enable ONLY optical SPDIF input to avoid bus contention on DAI0_PB09:
    // Both coax and optical receivers share DAI0_PB09 via 74CBT1G125 bus switches.
    // If both are enabled simultaneously, the idle receiver fights the active one.
    // 0xFB = bit1=1 (coax OFF) + bit2=0 (optical ON)
    if (TwiWrite8(REG_GPIOB, 0xFB) != 0) return 1;
    return 0; // success
}
