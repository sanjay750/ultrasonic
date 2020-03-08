

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include "__debug__.h"
#include "timer.h"



#define LEFT_LS_PIN		GPIO_PIN_9
#define LEFT_HS_PIN		GPIO_PIN_8
#define RIGHT_LS_PIN	GPIO_PIN_4
#define RIGHT_HS_PIN	GPIO_PIN_3

#define LEFT_LS_GPIO	GPIOA
#define LEFT_HS_GPIO	GPIOA
#define RIGHT_LS_GPIO	GPIOB
#define RIGHT_HS_GPIO	GPIOB

#define SPEED_PWM_PIN	GPIO_PIN_10
#define BREAK_PWM_PIN	GPIO_PIN_11
#define MOTOR_CURRENT_PIN	GPIO_PIN_4
#define MOTOR_VOLTAGE_PIN	GPIO_PIN_5

#define SPEED_PWM_GPIO		GPIOA
#define BREAK_PWM_GPIO		GPIOA
#define MOTOR_CURRENT_GPIO	GPIOA
#define MOTOR_VOLTAGE_GPIO	GPIOA

#define CHARG_PWM			TIM1->CCR3
#define DISCHARG_PWM			TIM1->CCR4

struct sensing
{
	uint16_t voltage;
	uint16_t current;
};

sensing Feed_back;


bool __voltage_control_enabled;
float __voltageSetPoint = 0;


float chargI;
float dischargI;


bool __speed_control_enable;
float __speedSetPoint = 600;

float speedI;
float speedP;
float speedD, __error_N;


void Delay()
{
	int i;
	for (i = 0; i < 1000000; i++)
		asm("nop");
}

void H_BRIDGE_DISABLE()
{
	HAL_GPIO_WritePin(LEFT_LS_GPIO, LEFT_LS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LEFT_HS_GPIO, LEFT_HS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RIGHT_LS_GPIO, RIGHT_LS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RIGHT_HS_GPIO, RIGHT_HS_PIN, GPIO_PIN_RESET);
}

void H_BRIDGE_POSITIVE()
{
	HAL_GPIO_WritePin(LEFT_HS_GPIO, LEFT_HS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RIGHT_LS_GPIO, RIGHT_LS_PIN, GPIO_PIN_RESET);
	int i = 1000;
	while (i--)
	asm volatile ("nop");
	HAL_GPIO_WritePin(LEFT_LS_GPIO, LEFT_LS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RIGHT_HS_GPIO, RIGHT_HS_PIN, GPIO_PIN_SET);
}

void H_BRIDGE_NEGATIVE()
{
	HAL_GPIO_WritePin(LEFT_LS_GPIO, LEFT_LS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RIGHT_HS_GPIO, RIGHT_HS_PIN, GPIO_PIN_RESET);
	int i = 1000;
	while (i--)
		asm volatile("nop");
	HAL_GPIO_WritePin(LEFT_HS_GPIO, LEFT_HS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RIGHT_LS_GPIO, RIGHT_LS_PIN, GPIO_PIN_SET);
}

void H_BRIDGE_SHORTED()
{
	HAL_GPIO_WritePin(LEFT_HS_GPIO, LEFT_HS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RIGHT_HS_GPIO, RIGHT_HS_PIN, GPIO_PIN_RESET);
	int i = 1000;
	while (i--)
		asm volatile("nop");
	HAL_GPIO_WritePin(LEFT_LS_GPIO, LEFT_LS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RIGHT_LS_GPIO, RIGHT_LS_PIN, GPIO_PIN_SET);
}

void motor_VoltageControl();
void motor_SpeedControl();

void motorDriverBegin()
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_AFIO_REMAP_SWJ_NOJTAG();
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = LEFT_LS_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LEFT_LS_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = LEFT_HS_PIN;
	HAL_GPIO_Init(LEFT_HS_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = RIGHT_LS_PIN;
	HAL_GPIO_Init(RIGHT_LS_GPIO, &GPIO_InitStructure);
	HAL_GPIO_WritePin(RIGHT_LS_GPIO, RIGHT_LS_PIN, GPIO_PIN_RESET);
	
	GPIO_InitStructure.Pin = RIGHT_HS_PIN;
	HAL_GPIO_Init(RIGHT_HS_GPIO, &GPIO_InitStructure);
	HAL_GPIO_WritePin(RIGHT_HS_GPIO, RIGHT_HS_PIN, GPIO_PIN_RESET);
	
	
	GPIO_InitStructure.Pin = SPEED_PWM_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(SPEED_PWM_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = BREAK_PWM_PIN;
	HAL_GPIO_Init(BREAK_PWM_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = MOTOR_CURRENT_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(MOTOR_CURRENT_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = MOTOR_VOLTAGE_PIN;
	HAL_GPIO_Init(MOTOR_VOLTAGE_GPIO, &GPIO_InitStructure);
	
	RCC->CFGR |= (0b11 << 14);   //adc clock devison
	
	DMA1_Channel1->CMAR = (uint32_t)&Feed_back;
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
	DMA1_Channel1->CNDTR = 2;
	DMA1_Channel1->CCR = 0b010110100001;
	
	ADC1->CR1 = ADC_CR1_SCAN;
	ADC1->CR2 = (0b1 << 20) | (0b010<<17) | (1 << 8);
	ADC1->SMPR2 = (0b100 << 27) | (0b100 << 24);
	ADC1->SQR1 = (1 << 20);
	ADC1->SQR3 = (5 << 0) | (4 << 5);
	ADC1->CR2 |= 1;
	
	TIM1->CCMR2 = TIM_CCMR2_OC3PE | (0b110 << TIM_CCMR2_OC3M_Pos) | (0b110 << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;
	TIM1->CCER = TIM_CCER_CC4E | TIM_CCER_CC3E;
	TIM1->PSC = 1;
	TIM1->ARR = 0x3FF;
	TIM1->BDTR = TIM_BDTR_MOE;
	TIM1->CR1 = TIM_CR1_ARPE  | TIM_CR1_CEN;
	
	linkTask(3, motor_SpeedControl, 0xFF << 2);
	linkTask(2, motor_VoltageControl, 0xFF<<1);
	
}

void set_voltage(unsigned short volt)
{
	__disable_irq();
	__voltageSetPoint = volt;
	__enable_irq();
}

void enableVoltageControl()
{
	__voltage_control_enabled = true;
	enableTask(2);
}
void disableVoltageControl()
{
	__voltage_control_enabled = false;
	disableTask(2);
	CHARG_PWM = 0;
	DISCHARG_PWM = 0;
	chargI = 0;
	dischargI = 0;
}

float readVoltage()
{
	printf("voltage = %d\n", Feed_back.voltage);
}
float readSpeed()
{
	printf("current = %d\n", Feed_back.current);
}

void enableSpeedControl()
{
	__disable_irq();
	__speed_control_enable = true;
	enableTask(3);
	__enable_irq();
}
void disableSpeedControl()
{
	__disable_irq();
	__speed_control_enable = false;
	disableTask(3);
	speedI = 0;
	speedP = 0;
	speedD = 0;
	__enable_irq();
}

void setSpeed(unsigned short speed);

void setHbridge(unsigned char mode);


#define KP_dis		2
#define KI_chr		0.1

#define KI_speed	0.1
#define KP_speed	1
#define KD_speed	0


float f_volt, f_current;


void motor_SpeedControl()
{
	if (!__speed_control_enable)
	{
		speedI = 0;
		speedP = 0;
		speedD = 0;
		return;
	}
	
	float back_emf =  Feed_back.voltage - 4 * Feed_back.current / 6;
	
	float __error = __speedSetPoint - back_emf;
	
	float out;
	
	speedI += __error*KI_speed;
	speedP = __error*KP_speed;
	speedD = (__error_N - __error)*KD_speed;
	
	if (speedI > 4000)
		speedI = 4000;
	else if (speedI < 0)
		speedI = 0;
	
	if (speedP > 4000)
		speedP = 4000;
	else if (speedP < 0)
		speedP = 0;
	
	if (speedD > 4000)
		speedD = 4000;
	else if (speedD < 0)
		speedD = 0;
	
	out = speedP + speedI + speedD;
	
	if (out > 4000)
		out = 4000;
	
	set_voltage(out);
	
}

void motor_VoltageControl()
{
	if (!__voltage_control_enabled)
	{
		CHARG_PWM = 0;
		DISCHARG_PWM = 0;
		chargI = 0;
		dischargI = 0;
		return;
	}
	
	//f_volt += (Feed_back.voltage - f_volt)*0.5;
	//f_current += (Feed_back.current - f_current)*0.5;
	
	float __error = __voltageSetPoint - Feed_back.voltage;
	
	chargI += (__error*KI_chr);
	if (chargI > 900.0)	chargI = 900.0;
	else if (chargI < 5)	chargI = 5;
	
	CHARG_PWM = chargI;
	
	
	dischargI = -(__error*KP_dis);
	
	if (dischargI > 500.0)	dischargI = 500.0;
	else if (dischargI < 0.0)	dischargI = 0.0;
	DISCHARG_PWM = dischargI;
}