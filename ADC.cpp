

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include "ADC.h"


void ADC_begin()
{
	RCC->CFGR |= (0b11 << 14);   //adc clock devison
	__HAL_RCC_ADC2_CLK_ENABLE();
	
	ADC2->SMPR2 = 0b111;
	ADC2->SQR1 = 0;
	ADC2->SQR3 = 0;
	ADC2->CR2 = 1;
}

unsigned short read_ADC(unsigned char ch_no)
{
	if (ch_no > 16)
		return 0xFFFF;
	
	ADC2->SQR3 = ch_no;
	ADC2->SR &= ~(1 << 1);
	ADC2->CR2 |= 1;
	while (!(ADC2->SR & (1 << 1))) ;
	return ADC2->DR;
}