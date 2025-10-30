/*****************************************************************************
 * templateTDM16_Core1.h
 *****************************************************************************/

#ifndef __TEMPLATETDM16_CORE1_H__
#define __TEMPLATETDM16_CORE1_H__

#include <stdint.h>
#include <drivers/twi/adi_twi.h>
#define APP_SUCCESS             (0)
#define APP_FAILED              (1)

void prepareToPlay(void);
void processBlock(void);

#endif /* __TEMPLATETDM16_CORE1_H__ */
