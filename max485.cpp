


#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_rcc.h>

#include <stm32f1xx_hal_usart.h>
#include <stdio.h>
#include "timer.h"

#include "max485.h"

#define TX_GPIO		GPIOB
#define RX_GPIO		GPIOB
#define IO_SEL_GPIO	GPIOB

#define TX_PIN		GPIO_PIN_6
#define RX_PIN		GPIO_PIN_7
#define IO_SEL_PIN	GPIO_PIN_5

//state of comunication handler
#define IDL			0
#define TXING		1
#define WAIT_RX		2
#define RXING		3

//comunication handler flags
#define TX_compleate		(0x1)
#define RX_compleate		(0x2)

//RX comunication stats
#define RX_IDL			0
#define RX_START		1
#define RX_DATA			2
#define RX_BS			3 //this symbol '\\'

//TX comunication state
#define TX_IDL				0
#define TX_DATA				1
#define TX_BS				2
#define TX_END_BS			3
#define TX_E				4
#define TX_S				5
#define TX_S_BS				6

//comunication modes

void __delay()
{
	int i;
	for (i = 0; i < 100; i++)
		asm("nop");
}
inline void txMode()
{
	USART1->CR1 &= ~USART_CR1_RE;
	__delay();
	IO_SEL_GPIO->BSRR = IO_SEL_PIN;
}

inline void rxMode()
{
	USART1->CR1 |= USART_CR1_RE;
	IO_SEL_GPIO->BRR = IO_SEL_PIN;
}




unsigned char *outputBuffer;
unsigned char *inputBuffer;

unsigned char RX_State;
unsigned char RX_BufferIndex;
unsigned char RX_BufferLength;
unsigned char *RX_Buffer;

unsigned char TX_State;
unsigned char TX_BufferIndex;
unsigned char TX_BufferLength;
unsigned char *TX_Buffer;

unsigned int __ref_ext;
unsigned char __state;
unsigned char __flags;




void max485Init()		//max485 init
{
	//USART1 Clock Enable
	__HAL_RCC_USART1_CLK_ENABLE();
	
	//Remap USART1 TX/RX to GPIOB6/GPIO7
	__HAL_AFIO_REMAP_USART1_ENABLE();
	
	//set Pin Mode of GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = TX_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(TX_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pin = IO_SEL_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(IO_SEL_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pin = RX_PIN;
	GPIO_InitStructure.Pull = 0;
	HAL_GPIO_Init(RX_GPIO, &GPIO_InitStructure);
	
	
	USART_InitTypeDef UART_P = 
	{ 
		.BaudRate = 115200,
		.WordLength = USART_WORDLENGTH_8B,
		.StopBits = USART_STOPBITS_1,
		.Parity = USART_PARITY_NONE,
		.Mode = USART_MODE_TX
	};
	
	USART_HandleTypeDef USART_HandleStruct = 
	{ 
		.Instance = USART1,
		.Init = UART_P
	};
	
	HAL_USART_Init(&USART_HandleStruct);   		//USART1 init
	
	USART1->CR2 &= ~USART_CR2_CLKEN;
	
	USART1->SR &= ~USART_SR_TC; 			//clear interrupt flag
	USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE;    //tx and rx interrupt enable
	NVIC->ISER[1] = 1 << (USART1_IRQn - 32);  			//USART1 global interrupt enable
	USART1->CR1 |= USART_CR1_UE;  		//enable USART1	//enable USART1
}



void max485Handler()		//max485 communication handler
{
	
}



void sendData(unsigned char *tx_buff, unsigned char length)
{
	if (length == 0)
		return;
	
	TX_BufferIndex = 0;
	TX_Buffer = tx_buff;
	TX_BufferLength = length;
	txMode();
	
	TX_State = TX_S_BS;
	USART1->DR = '\\';
}

bool dataSent()
{
	if ((__flags & TX_compleate))
	{
		__flags &= ~TX_compleate;
		return true;
	}
	return false;
}

void setRxBuff(unsigned char *rx_buff)
{
	RX_Buffer = rx_buff;
}

unsigned char  dataAvailable()
{
	if ((__flags & RX_compleate))
	{
		__flags &= ~RX_compleate;
		return RX_BufferLength;
	}
	return 0;
}

void RX_Handler()
{
	unsigned char temp = USART1->DR;
	
	if (RX_Buffer != 0)
	switch (RX_State)
	{
	case RX_START:
	case RX_IDL:
		if (temp == '\\')
		{
			RX_BufferIndex = 0;
			RX_State = RX_START;
		}
		else if (temp == 'S')
		{
			RX_State = RX_DATA;
		}
		break;
		
	case RX_DATA:
		if (temp == '\\')	RX_State = RX_BS;
		else				RX_Buffer[RX_BufferIndex++] = temp;
		break;
		
	case RX_BS:
		switch (temp)
		{
		case 'S':
			RX_State = RX_DATA;
			RX_BufferIndex = 0;
			break;
			
		case 'E':
			RX_State = RX_IDL;
			RX_BufferLength = RX_BufferIndex;
			__flags |= RX_compleate;
			USART1->CR1 &= ~USART_CR1_RE;
			break;
			
		default:
			RX_State = RX_DATA;
			RX_Buffer[RX_BufferIndex++] = temp;
			break;
			
		}
	}
}

void TX_Handler()
{
	USART1->SR &= ~USART_SR_TC;
	
	switch (TX_State)
	{
	case TX_IDL:
		rxMode();
		break;
		
	case TX_DATA:
		if (TX_BufferIndex < TX_BufferLength)
		{
			USART1->DR = TX_Buffer[TX_BufferIndex];
			if (TX_Buffer[TX_BufferIndex++] == '\\')
				TX_State = TX_BS;
		}
		else
		{
			USART1->DR = '\\';
			TX_State = TX_E;
		}
		break;
	
	case TX_S_BS:
	case TX_BS:
		USART1->DR = '\\';
		if (TX_State == TX_BS) TX_State = TX_DATA;
		else TX_State = TX_S;
		break;
		
	case TX_E:
		USART1->DR = 'E';
		TX_State = TX_IDL;
		__flags |= TX_compleate;
		break;
		
	case TX_S:
		USART1->DR = 'S';
		TX_State = TX_DATA;
		break;
	}
}


extern "C" void USART1_IRQHandler()
{	
	if (USART1->SR & (USART_SR_RXNE | USART_SR_ORE))
		RX_Handler();
	
	if (USART1->SR & USART_SR_TC)
		TX_Handler();
	
}
