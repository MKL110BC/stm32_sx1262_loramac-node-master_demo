#include "system_time.h"


uint32_t get_sys_time()
{
	return HAL_GetTick();
}

uint8_t timepassed(uint32_t timer,uint32_t pass_ms)
{
	return (HAL_GetTick() - timer)>=(pass_ms)?1:0;
}


void bsp_DelayUS(uint32_t n)
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;
       
	reload = SysTick->LOAD;                
    ticks = n * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */  
    
    tcnt = 0;
    told = SysTick->VAL;             /* 刚进入时的计数器值 */

    while (1)
    {
        tnow = SysTick->VAL;    
        if (tnow != told)
        {    
            /* SYSTICK是一个递减的计数器 */    
            if (tnow < told)
            {
                tcnt += told - tnow;    
            }
            /* 重新装载递减 */
            else
            {
                tcnt += reload - tnow + told;    
            }        
            told = tnow;

            /* 时间超过/等于要延迟的时间,则退出 */
            if (tcnt >= ticks)
            {
            	break;
            }
        }  
    }
} 

