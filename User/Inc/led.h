#ifndef __LED_H__
#define __LED_H__
#include "stm32l1xx_hal.h"

#define LED1_PIN                         GPIO_PIN_11
#define LED1_GPIO_PORT                   GPIOA
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

#define LED2_PIN                         GPIO_PIN_12
#define LED2_GPIO_PORT                   GPIOA
#define LED2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()


void led_init(void);
void led_process(void);

#endif

