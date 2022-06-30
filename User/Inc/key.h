#ifndef __KEY_H__
#define __KEY_H__
#include "stm32l1xx_hal.h"

#define KEY1_PIN                         GPIO_PIN_15
#define KEY1_GPIO_PORT                   GPIOA
#define KEY1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define KEY1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()


void key_init(void);
void key_scan(void);

#endif

