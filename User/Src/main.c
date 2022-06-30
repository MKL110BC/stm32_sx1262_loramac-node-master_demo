#include "main.h"
#include "system_time.h"
#include "lorawan_demo.h"
#include "led.h"
#include "key.h"


static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};

	/* Enable HSE Oscillator and Activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType	  = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState			  = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState		  = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource 	  = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL		  = RCC_PLL_MUL12;
	RCC_OscInitStruct.PLL.PLLDIV		  = RCC_PLL_DIV3;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Set Voltage scale1 as MCU will run at 32MHz */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
	while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

}





static struct lorawan_param lora_param;
static void lorawan_param_config(void)
{
	/*lora_param param*/
	memset(&lora_param,0,sizeof(lora_param));
	lora_param.region = LORAMAC_REGION_EU868;
	lora_param.class_type = CLASS_A;
	lora_param.connect_type = ACTIVATION_TYPE_OTAA;
	set_region_default(&lora_param);
		
	const uint8_t deveui[LORA_DEVEUI_LEN]= { 0xfa,0xd9,0xa8,0xff,0xff,0x03,0x31,0xe3};
	const uint8_t appeui[LORA_APPEUI_LEN]= { 0x52,0x69,0x73,0x69,0x6e,0x67,0x48,0x46};
	const uint8_t appkey[LORA_APPKEY_LEN]= { 0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
	memcpy(lora_param.deveui,deveui,LORA_DEVEUI_LEN);
	memcpy(lora_param.appeui,appeui,LORA_APPEUI_LEN);
	memcpy(lora_param.appkey,appkey,LORA_APPKEY_LEN);
	memcpy(lora_param.appskey,appkey,LORA_APPSKEY_LEN);
	memcpy(lora_param.nwkskey,appkey,LORA_NWKSKEY_LEN);
	lora_param.devaddr = htonl(*((uint32_t*)(deveui+2)));
}


static uint8_t key_press_state = 0;
void key_press_callback()
{
	key_press_state = 1;
	APP_INFO("key_press_callback \n");
}

void lorawan_demo()
{
	static uint8_t state = 0;
	static uint8_t send_port = 0;
	uint8_t testbuf[10] = {0,1,2,3,4,5,6,7,8,9};
	switch(state)
	{
		case 0:
			if(key_press_state)
			{
				key_press_state = 0;
				
				if(loraapp_get_net_state())
				{
					send_port %= LORAWAN_MAX_APP_PORT;
					send_port++;
					state = 1;
				}
			}
			break;
		case 1:
			if(lorawan_send_data(testbuf,sizeof(testbuf),send_port))
				state = 0;
			break;
	}
}




int main(void)
{
	APP_INFO("\n\n");
	APP_INFO("--------------------------------------\n");
	APP_INFO("Last Modify Time: 2022/4/29 14:30 \n");
	APP_INFO("Current FirmWare Version: %s \n",FIRMWARE_VERSION);
	APP_INFO("--------------------------------------\n");

	HAL_Init();
	SystemClock_Config();
	led_init();
	key_init();

	lorawan_param_config();
	loraapp_init(&lora_param);

	while (1)
	{
		key_scan();
		led_process();
		loraapp_process();
		lorawan_demo();
	}
}


#ifdef	USE_FULL_ASSERT

/**
  * @brief	Reports the name of the source file and the source line number
  * 		where the assert_param error has occurred.
  * @param	file: pointer to the source file name
  * @param	line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
