#include "key.h"
#include "system_time.h"


void key_init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	KEY1_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = KEY1_PIN;
	HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
}


__weak void key_press_callback()
{
}

void key_scan()
{
	static uint8_t state = 0;
	static uint32_t timer = 0;
	
	switch(state)
	{
		case 0:
			if(HAL_GPIO_ReadPin(KEY1_GPIO_PORT,KEY1_PIN)==0)
			{
				state = 1;
				timer = get_sys_time();
			}
			break;
		case 1:
			if(HAL_GPIO_ReadPin(KEY1_GPIO_PORT,KEY1_PIN)==0)
			{
				if(timepassed(timer,50))
				{
					key_press_callback();
					state = 2;
				}
			}
			else
			{
				state = 0;
			}
			break;
		case 2:
			if(HAL_GPIO_ReadPin(KEY1_GPIO_PORT,KEY1_PIN))
			{
				state = 0;
			}
			break;
	}
}

