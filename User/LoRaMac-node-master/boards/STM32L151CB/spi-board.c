/*
	20200810 by ysh
	目前先采用IO模拟的方式来使用SPI
*/

#include "spi-board.h"
#include "board-config.h"
#include "utilities.h"



void spi_io_init(void)
{
	GPIO_InitTypeDef  gpioinitstruct = {0};

	/* Enable the GPIO clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	gpioinitstruct.Pin = LORA_SCK_PIN | LORA_NSS_PIN | LORA_MOSI_PIN;
	gpioinitstruct.Pull   = GPIO_NOPULL;
	gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;
	gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(LORA_NSS_PORT, &gpioinitstruct);

	gpioinitstruct.Pin    = LORA_MISO_PIN;
	gpioinitstruct.Mode   = GPIO_MODE_INPUT;
	HAL_GPIO_Init(LORA_MISO_PORT, &gpioinitstruct);

	HAL_GPIO_WritePin(LORA_SCK_PORT,LORA_SCK_PIN,GPIO_PIN_RESET);
}
int spi_transdata(uint8_t *tx_data,uint16_t tx_len,uint8_t *rxbuf)
{

	uint16_t i,wr_data,j;
	//防止SPI操作被中断打断导致通讯异常
	CRITICAL_SECTION_BEGIN();

	HAL_GPIO_WritePin(LORA_NSS_PORT,LORA_NSS_PIN,GPIO_PIN_RESET);

	for(j=0; j<tx_len; j++)
	{
		wr_data=tx_data[j];
		rxbuf[j]=0;
		for(i=0; i<8; i++) 
		{
			if(wr_data&0x80)
			{
				HAL_GPIO_WritePin(LORA_MOSI_PORT,LORA_MOSI_PIN,GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(LORA_MOSI_PORT,LORA_MOSI_PIN,GPIO_PIN_RESET);
			}

			HAL_GPIO_WritePin(LORA_SCK_PORT,LORA_SCK_PIN,GPIO_PIN_SET);

			wr_data<<=1;
			rxbuf[j]<<=1;
			if(	HAL_GPIO_ReadPin(LORA_MISO_PORT,LORA_MISO_PIN))
				rxbuf[j]|=0x01;
			HAL_GPIO_WritePin(LORA_SCK_PORT,LORA_SCK_PIN,GPIO_PIN_RESET);
		}
	}


	HAL_GPIO_WritePin(LORA_NSS_PORT,LORA_NSS_PIN,GPIO_PIN_SET);
	CRITICAL_SECTION_END();

//	SYS_LOG_INFO_BUF("spi_transdata tx:",tx_data,tx_len);
//	SYS_LOG_INFO_BUF("spi_transdata rx:",rxbuf,tx_len);

	return 0;
}


