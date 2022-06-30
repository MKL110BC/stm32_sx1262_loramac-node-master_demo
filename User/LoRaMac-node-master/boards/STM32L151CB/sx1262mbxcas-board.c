/*!
	20200810 by ysh
	
 */
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "board-config.h"
#include "delay.h"
#include "radio.h"
#include "sx126x-board.h"
#include "spi-board.h"
#include "stm32l1xx_hal.h"
#include "log.h"

/*
 无线芯片PA控制， drive radio RX/TX pins 
 */
void sx126x_pa_control (int8_t state)
{
	//发送接收都置高，由DIO2控制发送还是接收
	if(state>=0)
	{
		HAL_GPIO_WritePin(LORA_TXEN_PORT,LORA_TXEN_PIN,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LORA_RXEN_PORT,LORA_RXEN_PIN,GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(LORA_TXEN_PORT,LORA_TXEN_PIN,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LORA_RXEN_PORT,LORA_RXEN_PIN,GPIO_PIN_SET);
	}
}
static  uint8_t inbuf[256+20];
static  uint8_t outbuf[256+20];
//芯片IO初始化
void SX126xIoInit( void )
{
	GPIO_InitTypeDef  gpioinitstruct = {0};

	/* Enable the GPIO clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Configure the GPIO pin */
	gpioinitstruct.Pull   = GPIO_NOPULL;
	gpioinitstruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;

	gpioinitstruct.Pin    = LORA_NRST_PIN;
	gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(LORA_NRST_PORT, &gpioinitstruct);

	gpioinitstruct.Pin    = LORA_BUSY_PIN;
	gpioinitstruct.Mode   = GPIO_MODE_INPUT;
	HAL_GPIO_Init(LORA_BUSY_PORT, &gpioinitstruct);

	gpioinitstruct.Pin    = LORA_DIO1_PIN;
	gpioinitstruct.Mode   = GPIO_MODE_INPUT;
	HAL_GPIO_Init(LORA_DIO1_PORT, &gpioinitstruct);


	gpioinitstruct.Pin    = LORA_RXEN_PIN;
	gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(LORA_RXEN_PORT, &gpioinitstruct);

	gpioinitstruct.Pin    = LORA_TXEN_PIN;
	gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(LORA_TXEN_PORT, &gpioinitstruct);


	HAL_GPIO_WritePin(LORA_RXEN_PORT,LORA_RXEN_PIN,GPIO_PIN_SET);

	spi_io_init();
}

DioIrqHandler *loradioIrq;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_8)
	{
		loradioIrq(0);
//		LORA_DEBUG("loradioIrq(0) \n");
	}
	
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
}

void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
}


//芯片IO中断初始化
void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
	loradioIrq=dioIrq;

	//20210702  by ysh ,中断只能使用上升沿中断，如果使用翻转中断，会造成清除中断后又
	//触发一次中断，这样在radio.process中又执行一次寄存器的读写操作，这样会导致
	//SX1262芯片无法进入休眠模式

	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Enable GPIOA clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure PA.8 pin as input floating */
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0x0F, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
}

void SX126xIoDeInit( void )
{
}

void SX126xIoDbgInit( void )
{

}

void SX126xIoTcxoInit( void )
{
	SX126xSetDio3AsTcxoCtrl( TCXO_CTRL_1_8V, 255 );
    // No TCXO component available on this board design.
}


uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
	#define BOARD_TCXO_WAKEUP_TIME 5
	return BOARD_TCXO_WAKEUP_TIME;
}

void SX126xIoRfSwitchInit( void )
{
    SX126xSetDio2AsRfSwitchCtrl( true );
}

static RadioOperatingModes_t OperatingMode;
RadioOperatingModes_t SX126xGetOperatingMode( void )
{
    return OperatingMode;
}

void SX126xSetOperatingMode( RadioOperatingModes_t mode )
{
    OperatingMode = mode;
}

//控制复位管脚，(0=low, 1=high, 2=floating)
void SX126X_PIN_RST_CONTROL(uint8_t state)
{
	GPIO_InitTypeDef  gpioinitstruct = {0};

	if(state == 0 || state == 1)   // drive pin
	{
		gpioinitstruct.Pull   = GPIO_NOPULL;
		gpioinitstruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
		gpioinitstruct.Pin    = LORA_NRST_PIN;
		gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(LORA_NRST_PORT, &gpioinitstruct);

		if(state)
		{
			HAL_GPIO_WritePin(LORA_NRST_PORT,LORA_NRST_PIN,GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(LORA_NRST_PORT,LORA_NRST_PIN,GPIO_PIN_RESET);
		}
	}
	else     // keep pin floating
	{
		gpioinitstruct.Pull   = GPIO_NOPULL;
		gpioinitstruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
		gpioinitstruct.Pin    = LORA_NRST_PIN;
		gpioinitstruct.Mode   = GPIO_MODE_INPUT;
		HAL_GPIO_Init(LORA_NRST_PORT, &gpioinitstruct);
	}
}
void SX126xReset( void )
{
	do
	{
		DelayMs( 10 );
		SX126X_PIN_RST_CONTROL(0);
		DelayMs( 20 );
		SX126X_PIN_RST_CONTROL(2); // internal pull-up
		DelayMs( 10 );
	}
	while( SX126xReadRegister(0x0741) != 0x24 );

	LORA_DEBUG("SX126x chip_id = %x \n",SX126xReadRegister(0x0741));
}

void SX126xWaitOnBusy( void )
{
	while (	HAL_GPIO_ReadPin(LORA_BUSY_PORT,LORA_BUSY_PIN) != 0);
}

void SX126xWakeup( void )
{
	uint16_t len=0;
	inbuf[len++]=RADIO_GET_STATUS;
	inbuf[len++]=0x00;
	spi_transdata(inbuf,len,outbuf);

	// Wait for chip to be ready.
	SX126xWaitOnBusy( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
	uint16_t i,len=0;
	SX126xCheckDeviceReady( );
	inbuf[len++]=command;
	for(  i = 0; i < size; i++ )
	{
	    	inbuf[len++]= buffer[i];
	}
	spi_transdata(inbuf,len,outbuf);
	if( command != RADIO_SET_SLEEP )
	{
	    	SX126xWaitOnBusy( );
	}
}

uint8_t SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
	uint16_t i,len=0;

	SX126xCheckDeviceReady( );


	inbuf[len++]=command;
	for(  i = 0; i < size+1; i++ )
	{
	    	inbuf[len++]= 0;
	}
	spi_transdata(inbuf,len,outbuf);


	memcpy1(buffer,outbuf+2,size);

	SX126xWaitOnBusy( );

	return outbuf[1];
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
	uint16_t i,len=0;

	SX126xCheckDeviceReady( );

	inbuf[len++]=RADIO_WRITE_REGISTER;
	inbuf[len++]=( address & 0xFF00 ) >> 8 ;
	inbuf[len++]=address & 0x00FF;
	for(  i = 0; i < size; i++ )
	{
	    	inbuf[len++]= buffer[i];
	}
	spi_transdata(inbuf,len,outbuf);

	SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
	SX126xWriteRegisters( address, &value, 1 );	
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
	uint16_t i,len=0;
	SX126xCheckDeviceReady( );

	inbuf[len++]=RADIO_READ_REGISTER;
	inbuf[len++]=( address & 0xFF00 ) >> 8 ;
	inbuf[len++]=address & 0x00FF;
	inbuf[len++]=0;
	for(  i = 0; i < size; i++ )
	{
	   	inbuf[len++]= 0;
	}
	spi_transdata(inbuf,len,outbuf);
	memcpy1(buffer,outbuf+4,size);
	SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
	uint8_t data;
    	SX126xReadRegisters( address, &data, 1 );
    	return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	uint16_t i,len=0;
	SX126xCheckDeviceReady( );

	inbuf[len++]=RADIO_WRITE_BUFFER;
	inbuf[len++]=offset ;
	for(  i = 0; i < size; i++ )
	{
	    	inbuf[len++]= buffer[i];
	}
	spi_transdata(inbuf,len,outbuf);
	memcpy(buffer,outbuf+2,size);
	SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	uint16_t i,len=0;

	SX126xCheckDeviceReady( );

	inbuf[len++]=RADIO_READ_BUFFER;
	inbuf[len++]=offset ;
	inbuf[len++]=0;
	for(  i = 0; i < size; i++ )
	{
	    	inbuf[len++]= 0;
	}
	spi_transdata(inbuf,len,outbuf);
	memcpy1(buffer,outbuf+3,size);
	SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
	SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetDeviceId( void )
{
	return SX1262;
}

void SX126xAntSwOn( void )
{
    	sx126x_pa_control(0);
}

void SX126xAntSwOff( void )
{
    	sx126x_pa_control(-1);
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}
