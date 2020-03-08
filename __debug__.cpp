

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_usart.h>
#include <stdio.h>
#include "timer.h"



extern "C" int _write(int file, char *ptr, int length)
{
	for (int i = 0; i < length; i++)
	{
		USART3->DR = ptr[i];
		while (!(USART3->SR & (USART_SR_TXE))) ;
	}
	return length;
}



void __DebugSerialBegin()
{
	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = GPIO_PIN_10;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = GPIO_PIN_13;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	USART_InitTypeDef UART_P = 
	{ 
		.BaudRate = 115200,
		.WordLength = USART_WORDLENGTH_8B,
		.StopBits = USART_STOPBITS_1,
		.Parity = USART_PARITY_NONE,
		.Mode = USART_MODE_TX_RX
	};
	
	USART_HandleTypeDef USART_HandleStruct = 
	{ 
		.Instance = USART3,
		.Init = UART_P
	};
	
	HAL_USART_Init(&USART_HandleStruct);
	
	USART3->CR1 |= (1 << 13);
}



