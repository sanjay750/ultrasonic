

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_adc.h>
#include "__debug__.h"
#include "timer.h"
#include "ADC.h"
#include "pwm.h"
#include "digitalPot.h"


#define OFFSET_CAL_VALUE			510

#define FREQ_CONTROL_PWM_CH			4
#define OFFSET_CONTROLL_PWM_CH		3

#define CURRENT_ERROR_FB_ADC_CH		7
#define PHASE_FB_ADC_CH				6

#define DRIVER_ENABLE_GPIO			GPIOA
#define FREQ_CONTROL_GPIO			GPIOB
#define OFFSET_CONTROLL_GPIO		GPIOB
#define CURRENT_ERROR_FB_GPIO		GPIOA
#define PHASE_FB_GPIO				GPIOA

#define DRIVER_ENABLE_PIN			GPIO_PIN_8
#define FREQ_CONTROLL_PIN			GPIO_PIN_9
#define OFFSET_CONTROL_PIN			GPIO_PIN_8
#define CURRENT_ERROR_FB_PIN		GPIO_PIN_7
#define PHASE_FB_PIN				GPIO_PIN_6


unsigned short __frequncy_pwm_val;
unsigned char __set_current;

bool __driver_enabled = false;
bool __offset_enabled = false;

float __filt_phase;
float __filt_currentError;

float __set_frequency_pwm_output;
float __offsetError_pwm_output;
float __set_current_output;




void control_system_handler();

void ultrasonic_HardwareInit()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	digitalPotInit();
	writeDigitalPot(0);
	pwm_begin();
	ADC_begin();
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = DRIVER_ENABLE_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DRIVER_ENABLE_GPIO, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.Pin = FREQ_CONTROLL_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(FREQ_CONTROL_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = OFFSET_CONTROL_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(OFFSET_CONTROLL_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = CURRENT_ERROR_FB_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(CURRENT_ERROR_FB_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = PHASE_FB_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(PHASE_FB_GPIO, &GPIO_InitStructure);
	
	linkTask(1, control_system_handler, 0xFF);
	enableTask(1);
}


void ultrasonic_DriverEnable()
{
	__driver_enabled = true;
	HAL_GPIO_WritePin(DRIVER_ENABLE_GPIO, DRIVER_ENABLE_PIN, GPIO_PIN_SET);
}

void ultrasonic_DriverDisable()
{
	__disable_irq();
	__driver_enabled = false;
	__enable_irq();
	
	HAL_GPIO_WritePin(DRIVER_ENABLE_GPIO, DRIVER_ENABLE_PIN, GPIO_PIN_RESET);
	pwm_write(FREQ_CONTROL_PWM_CH, 0);
	writeDigitalPot(0);
	__set_current_output = 0;
	__set_frequency_pwm_output = 0;
	
}

void ultrasonic_OffsetEnable()
{
	__offset_enabled = true;
}
void ultrasonic_OffsetDisable()
{
	__disable_irq();
	__offset_enabled = false;
	__enable_irq();
	
	pwm_write(OFFSET_CONTROLL_PWM_CH, 0);
	__offsetError_pwm_output = 0;
	
}

void ultrasonic_Setrequency(unsigned short val)
{
	__frequncy_pwm_val = val;
}

void ultrasonic_SetCurrent(unsigned char val)
{
	__set_current = val;
}

unsigned short ultrasonic_ReadOffset_CH()
{
	unsigned short temp;
	__disable_irq();
	temp = __filt_currentError;
	__enable_irq();
	
	return temp;
}
unsigned short ultrasonic_ReadPhase_CH()
{
	unsigned short temp;
	__disable_irq();
	temp = __filt_phase;
	__enable_irq();
	
	return temp;
}

bool ultrasonic_OfsetLocked() // needs change
{
	unsigned short __error =  - ultrasonic_ReadOffset_CH();
	if (__error < 0)
		__error = -__error;
	return __error < 20;
}


void currentHandler()
{
	__set_current_output += (__set_current - __set_current_output) * 0.05;
	writeDigitalPot(__set_current_output);
}

void offSetHandler()
{
	float __error = (__set_current_output * 7.8) - __filt_currentError;
	__offsetError_pwm_output += -__error * 0.05;
	
	if (__offsetError_pwm_output > 2500.0)
		__offsetError_pwm_output = 2500.0;
	else if (__offsetError_pwm_output < 0.0)
		__offsetError_pwm_output = 0.0;
	
	pwm_write(OFFSET_CONTROLL_PWM_CH, __offsetError_pwm_output);
}

void frequencyHandler()
{
	__set_frequency_pwm_output += (__frequncy_pwm_val - __set_frequency_pwm_output) * 0.05;
	pwm_write(FREQ_CONTROL_PWM_CH , __set_frequency_pwm_output);
}


void control_system_handler()
{
	if (ADC2->SQR3 == CURRENT_ERROR_FB_ADC_CH)
	{
		__filt_currentError += (ADC2->DR - __filt_currentError) * 0.1;
		ADC2->SQR3 = PHASE_FB_ADC_CH;
		ADC2->SR &= ~(1 << 1);
		ADC2->CR2 |= 1;
	}
	else
	{
		__filt_phase += (ADC2->DR - __filt_phase) * 0.1;
		ADC2->SQR3 = CURRENT_ERROR_FB_ADC_CH;
		ADC2->SR &= ~(1 << 1);
		ADC2->CR2 |= 1;
	}
	
	if (__driver_enabled)
		frequencyHandler() ,currentHandler();
	
	if (__offset_enabled)
		offSetHandler();
}