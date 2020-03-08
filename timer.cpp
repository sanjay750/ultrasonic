


#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>

extern "C" void TIM2_IRQHandler();

unsigned long millis_count = 0;

void(*Task[4])() = {0};

void timerBegin()
{
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	
	TIM2->PSC = 72;
	TIM2->ARR = 999;
	TIM2->DIER = (1 << 1);
	TIM2->CR1 = (1 << 7) | (1 << 0);
	NVIC->ISER[0] = 1 << 28;
	TIM2->CCR2 = 999 >> 1;
	
	TIM3->PSC = 10 - 1;
	TIM3->ARR = 7200 - 1;
	TIM3->CR1 = (1 << 7) | (1 << 0);
	NVIC->ISER[0] |= (1 << 29);
}
unsigned long millis()
{
	TIM2->DIER &= ~(1 << 1);
	unsigned long temp = millis_count;
	TIM2->DIER |= (1 << 1);
	return temp;
}
unsigned long micros()
{
	return (millis_count / 1000) + TIM2->CNT;
}

void delay(unsigned int del)
{
	unsigned long temp = millis();
	
	while (millis() - temp < del) ;
		
}

void TIM2_IRQHandler() 
{
	TIM2->SR = RESET;
	millis_count++;
}

bool linkTask(uint8_t place, void(*task)(), uint16_t timing)
{
	if (place < 1)
		return false;
	
	Task[place-1] = task;
	
	switch (place)
	{
	case 1:
		TIM3->CCR1 = timing;
		break;
	case 2:
		TIM3->CCR2 = timing;
		break;
	case 3:
		TIM3->CCR3 = timing;
		break;
	case 4:
		TIM3->CCR4 = timing;
		break;
		
	default:
		return false;
	}
	
	return true;
}

void enableTask(uint8_t place)
{
	if (place < 1 || place > 4 || Task[place-1] == 0)
		return;
	
	TIM3->DIER |= (1 << place);
	
}

void disableTask(uint8_t place)
{
	if (place < 1 || place > 4)
		return;
	
	TIM3->DIER &= ~(1 << place);
}


extern "C" void TIM3_IRQHandler();

void TIM3_IRQHandler()
{
	
	if (TIM3->DIER & (1 << 1) && TIM3->SR & (1 << 1))
	{
		TIM3->SR &= ~(1 << 1);
		Task[0]();
	}
	
	if (TIM3->DIER & (1 << 2) && TIM3->SR & (1 << 2))
	{
		TIM3->SR &= ~(1 << 2);
		Task[1]();
	}
	
	if (TIM3->DIER & (1 << 3) && TIM3->SR & (1 << 3))
	{
		TIM3->SR &= ~(1 << 3);
		Task[2]();
	}
	
	if (TIM3->DIER & (1 << 4) && TIM3->SR & (1 << 4))
	{
		TIM3->SR &= ~(1 << 4);
		Task[3]();
	}
	
}