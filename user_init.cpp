#include <stm32f1xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <user_init.h>
#include "__debug__.h"
#include "timer.h"
#include "pwm.h"
#include "ADC.h"
#include "UI.h"
#include "ultrasonic.h"
#include "dcMotorDriver_API.h"

void H_BRIDGE_NEGATIVE();
void H_BRIDGE_POSITIVE();
void H_BRIDGE_SHORTED();

void tryTask(void const *args)
{
	
	
	while (1)
	{
		
	}
}

void debug_loop(void const *args);



osThreadId deb;
osThreadDef_t deb_ = { "deb", debug_loop, osPriorityNormal, 0, configMINIMAL_STACK_SIZE };


void userInit()
{
	timerBegin();
	pwm_begin();
	ADC_begin();
	__DebugSerialBegin();
	uiBegin();
	
	deb = osThreadCreate(&deb_, NULL);
}



