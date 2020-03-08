

#ifndef __DEBUG_H
#define __DEBUG_H

#define D_LED_H					GPIOC->BSRR = 1<<13
#define D_LED_L					GPIOC->BRR = 1<<13
#define LED_INBUILT_GPIO		GPIOC
#define LED_INBUILT_PIN			GPIO_Pin_13

#include <stdio.h>
void __DebugSerialBegin();

#endif // !__DEBUG_H
