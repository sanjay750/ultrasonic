
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <pwm.h>


void pwm_begin()
{
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	TIM4->CCMR2 = (1 << 11) | (0b110 << 12) | (1 << 3) | (0b110 << 4);
	TIM4->CCER = (1 << 12) | (1 << 8);
	TIM4->ARR = 4095;
	
	TIM4->CR1 = (1 << 7) | (1 << 0);
	
}


void pwm_write(unsigned char ch_no, unsigned short value)
{
	switch (ch_no)
	{
	case 4:
		TIM4->CCR4 = value;
		break;
	case 3:
		TIM4->CCR3 = value;
		break;
		
	default:
		return;
	}
}

