/*
 * soft_switch.h — MCP23017 soft-switch driver (EV-SOMCRR-EZKIT)
 */

#ifndef PLATFORM_SOFT_SWITCH_H_
#define PLATFORM_SOFT_SWITCH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I2cAddrSOFTConfig		0x22

int Soft_resetAudio(void);
int Soft_init(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SOFT_SWITCH_H_ */
