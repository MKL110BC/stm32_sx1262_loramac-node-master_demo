/*
	20200810 by ysh
	用于选择lora驱动所需要的硬件IO口
*/
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#include "stm32l1xx_hal.h"
#include "log.h"


#define SET_GPIO(A)   		nrf_gpio_pin_set(A) 
#define CLEAR_GPIO(A) 		nrf_gpio_pin_clear(A)
#define READ_GPIO(A)   		nrf_gpio_pin_read(A)
#define OUTPUT_GPIO(A) 		nrf_gpio_cfg_output(A)
#define INPUT_GPIO(A) 		nrf_gpio_cfg_input(A,NRF_GPIO_PIN_NOPULL)



#define LORA_NSS_PORT		GPIOA
#define LORA_NSS_PIN		GPIO_PIN_4

#define LORA_SCK_PORT		GPIOA
#define LORA_SCK_PIN		GPIO_PIN_5

#define LORA_MISO_PORT		GPIOA
#define LORA_MISO_PIN		GPIO_PIN_6

#define LORA_MOSI_PORT		GPIOA
#define LORA_MOSI_PIN		GPIO_PIN_7

#define LORA_NRST_PORT		GPIOB
#define LORA_NRST_PIN		GPIO_PIN_15


#define LORA_BUSY_PORT		GPIOB
#define LORA_BUSY_PIN		GPIO_PIN_0

#define LORA_DIO1_PORT		GPIOA
#define LORA_DIO1_PIN		GPIO_PIN_8


#define LORA_RXEN_PORT		GPIOB
#define LORA_RXEN_PIN		GPIO_PIN_2

#define LORA_TXEN_PORT		GPIOB
#define LORA_TXEN_PIN		GPIO_PIN_1



#endif // __BOARD_CONFIG_H__

