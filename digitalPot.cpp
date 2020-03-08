

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_hal_spi.h>
#include <stm32f1xx_hal_rcc.h>


#define CS_GPIO				GPIOB
#define SCK_GPIO			GPIOB
#define SDI_SDO_GPIO		GPIOB

#define CS_PIN				GPIO_PIN_12
#define SCK_PIN				GPIO_PIN_13
#define SDI_SDO_PIN			GPIO_PIN_15



	
void digitalPotInit()
{
	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = CS_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CS_GPIO, &GPIO_InitStructure);
	HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_SET);
	
	GPIO_InitStructure.Pin = SCK_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	//GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SCK_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = SDI_SDO_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	//GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SDI_SDO_GPIO, &GPIO_InitStructure);
	
	
	SPI2->CR1 = SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_DFF | SPI_CR1_SSI |\
		SPI_CR1_SSM | SPI_CR1_SPE | (0b10 << SPI_CR1_BR_Pos) | SPI_CR1_MSTR;
	
	
}


void writeDigitalPot(unsigned char value)
{
	HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_RESET);
	
	SPI2->DR = (unsigned short) value;
	while (SPI2->SR & SPI_SR_BSY) ;
	
	HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_SET);
}