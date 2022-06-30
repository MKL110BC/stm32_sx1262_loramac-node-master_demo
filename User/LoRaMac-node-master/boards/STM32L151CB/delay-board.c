/*!
	20200814 by ysh
 */
#include "stm32l1xx_hal.h"
#include "delay-board.h"

void DelayMsMcu( uint32_t ms )
{
    HAL_Delay( ms );
}
