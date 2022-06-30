#include "led.h"
#include "system_time.h"
#include "lorawan_demo.h"


void led_init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	LED1_GPIO_CLK_ENABLE();
	LED2_GPIO_CLK_ENABLE();
	
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	GPIO_InitStruct.Pin = LED1_PIN;
	HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED2_PIN;
	HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(LED1_GPIO_PORT,LED1_PIN,GPIO_PIN_RESET);	//Net Led Off	
	HAL_GPIO_WritePin(LED2_GPIO_PORT,LED2_PIN,GPIO_PIN_SET);	//Power Led On
	
}



void led_process()
{
	static uint8_t state = 0;
	static uint32_t timer = 0;
	
	switch(state)
	{
		case 0:
			state = 1;
			timer = get_sys_time();
			break;
		case 1:
			if(loraapp_get_net_state())
			{
				HAL_GPIO_WritePin(LED1_GPIO_PORT,LED1_PIN,GPIO_PIN_SET);
				state = 2;
				break;
			}

			if(timepassed(timer,500))
			{
				timer = get_sys_time();
				HAL_GPIO_TogglePin(LED1_GPIO_PORT,LED1_PIN);
			}
			break;
		case 2:
			if(loraapp_get_net_state()==0)
			{
				state = 0;
			}
			break;
	}
}

