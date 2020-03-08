

#include <stm32f1xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include <ultrasonic.h>
#include "__debug__.h"
#include "digitalPot.h"
#include "ultrasonic_API.h"
#include "UI.h"


extern "C" void TH1(void const *argument);

osThreadId ultrasonicThreadID;
osThreadDef_t ultrasonic = { "LED1", ultrasonic_MainTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE };
unsigned short resonence_pwm;
bool __ultrasonic_task_running;


uiInputdHandel ultrasonic_work_pause = 
{
	.key_no = WORK_PAUSE,
	.pushed = ultrasonic_workPause
};


uiInputdHandel ultrasonic_select = 
{
	.key_no = SELECT_ULTRASONIC,
	.pushed = ultrasonic_selectUltrasonic
};

void ultrasonicInit()
{
	addEvent(&ultrasonic_select);
	ultrasonic_HardwareInit();
}



unsigned short find_resonence(bool disable_after = true)
{
	unsigned short ret = 0;
	unsigned short pwm_val = 1000;
	unsigned short phase_ = 0;
	
	ultrasonic_DriverEnable();
	ultrasonic_OffsetEnable();
	ultrasonic_SetCurrent(0x7F>>1);
	ultrasonic_Setrequency(pwm_val);
	osDelay(100);
	while (1)
	{
		ultrasonic_Setrequency(pwm_val);
		osDelay(20);
		if (ultrasonic_ReadPhase_CH() > phase_)
		{
			phase_ = ultrasonic_ReadPhase_CH();
			ret = pwm_val;
		}
		
		printf("phase = %d\n", ultrasonic_ReadPhase_CH());
		
		if (pwm_val >= 3000)
			break;
		pwm_val += 10;
	}
	
	if (disable_after)
	{
		ultrasonic_DriverDisable();
		ultrasonic_OffsetDisable();
	}
	
	return ret;
}

bool ultrasonicRunning()
{
	return __ultrasonic_task_running;
}

void startUltrasonic(unsigned char mode)
{
	if (__ultrasonic_task_running)
		return;
	
	__ultrasonic_task_running = true;
	ultrasonicThreadID = osThreadCreate(&ultrasonic, NULL);
}

osStatus osThreadTerminate(osThreadId thread_id);

void stopUltrasonic()
{
	__ultrasonic_task_running = false;
	osThreadTerminate(ultrasonicThreadID);
	ultrasonic_DriverDisable();
	ultrasonic_OffsetDisable();
}

void ultrasonic_MainTask(void const *argument)
{
	
	ultrasonic_Setrequency(find_resonence(false));
	
	while (1)
	{
		printf("ultrasonic running\n");
		osDelay(500);
	}
}

void ultrasonic_selectUltrasonic(void *args)
{
	printf("US: select\n");
	addEvent(WORK_PAUSE, &ultrasonic_work_pause);
}


void ultrasonic_workPause(void *args)
{
	if (ultrasonicRunning())
		stopUltrasonic();
	else
		startUltrasonic();
}