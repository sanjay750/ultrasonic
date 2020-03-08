

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <../CMSIS_RTOS/cmsis_os.h>

#include "__debug__.h"
#include "timer.h"
#include "dcMotorDriver_API.h"
#include "motor.h"
#include "UI.h"

uiInputdHandel motor_work_pause = 
{ 
	.key_no = WORK_PAUSE,
	.pushed = motor_workPause
};

uiInputdHandel morot_select = 
{ 
	.key_no = SELECT_MOTOR,
	.pushed = motor_motorSelect
};

osThreadId motorTaskID;
osThreadDef_t motorTaskDef = { "TRY", motorTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE };

void motorBegin()
{
	motorDriverBegin();
	motorTaskID = osThreadCreate(&motorTaskDef, NULL);
}
void motorTask(void const *args)
{
	
	while (1)
	{
		
	}
}

void motor_workPause(void *args)
{
	
}
void motor_motorSelect(void *args)
{
	
}

void startMotor();
void stopMotor();
void setSpeed(unsigned short speed);
void setDirection(unsigned char direction);
void selectMotor(unsigned char motor);