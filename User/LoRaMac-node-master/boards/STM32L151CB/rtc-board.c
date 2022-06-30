#include <math.h>
#include <time.h>
#include "utilities.h"
#include "delay.h"
#include "timer.h"
#include "rtc-board.h"
#include "system_time.h"
#include "log.h"

// MCU Wake Up Time
#define MIN_ALARM_DELAY                             (1)
#define RTC_FREQ									1000


uint32_t get_rtc(void)
{
	return HAL_GetTick();	//unit: 1ms
}

/*!
 * RTC timer context
 */
typedef struct
{
    uint32_t Time;  // Reference time
    uint32_t Delay; // Reference Timeout duration

} RtcTimerContext_t;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the \ref RtcSetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
static RtcTimerContext_t RtcTimerContext;
uint32_t RtcBkupRegisters[] = { 0, 0 };



static TIM_HandleTypeDef htim;
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *tim_handler)
{
	if(tim_handler==(&htim))
	{
		TimerIrqHandler( );
//		LORA_DEBUG("HAL_TIM_PeriodElapsedCallback:timer=%d \n",get_sys_time());
	}
}



static uint8_t rtc_init_flag = 0;
//lorawan rtc初始化
void loraTime_Init(void)
{
	if(rtc_init_flag==0)
	{
		rtc_init_flag = 1;
		
		__HAL_RCC_TIM3_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM3_IRQn, 0x00, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
		
		htim.Instance				= TIM3; 							// 使用定时器3
		htim.Init.Prescaler 		= (SystemCoreClock / 1000) - 1; 	// 预分频系数.定时器时钟为1MHz
		htim.Init.CounterMode		= TIM_COUNTERMODE_UP;				// 向上计数
		htim.Init.Period			= 0xFFFF;							// 自动装载器ARR的值
		htim.Init.ClockDivision 	= TIM_CLOCKDIVISION_DIV1;			// 时钟分频(与输入采样相关)
		htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;	// 不自动重新装载
		
		HAL_TIM_Base_Init(&htim);
	}
}

void RtcInit( void )
{
    loraTime_Init();
    RtcSetTimerContext();
}
uint32_t RtcSetTimerContext( void )
{
	RtcTimerContext.Time = ( uint32_t )get_rtc();
    return ( uint32_t )RtcTimerContext.Time;
}

uint32_t RtcGetTimerContext( void )
{
    return RtcTimerContext.Time;
}

uint32_t RtcGetSysTickLoad( void )
{
    return RtcTimerContext.Time;
}


/*!
 * \brief returns the wake up time in ticks
 *
 * \retval wake up time in ticks
 */
uint32_t RtcGetMinimumTimeout( void )
{
    return( MIN_ALARM_DELAY );
}

/*!
 * \brief converts time in ms to time in ticks
 *
 * \param[IN] milliseconds Time in milliseconds
 * \retval returns time in timer ticks
 */
uint32_t RtcMs2Tick( uint32_t milliseconds )
{
	return milliseconds;
}

/*!
 * \brief converts time in ticks to time in ms
 *
 * \param[IN] time in timer ticks
 * \retval returns time in milliseconds
 */
uint32_t RtcTick2Ms( uint32_t tick )
{
	return tick;
}

/*!
 * \brief a delay of delay ms by polling RTC
 *
 * \param[IN] delay in ms
 */
void RtcDelayMs( uint32_t delay )
{
	HAL_Delay(delay);

#if 0
    uint64_t delayTicks = 0;
    uint64_t refTicks = RtcGetTimerValue( );

    delayTicks = RtcMs2Tick( delay );

    // Wait delay ms
    while( ( ( RtcGetTimerValue( ) - refTicks ) ) < delayTicks )
    {
        __NOP( );
    }
#endif

}






/*!
 * \brief Sets the alarm
 *
 * \note The alarm is set at now (read in this function) + timeout
 *
 * \param timeout Duration of the Timer ticks
 */
void RtcSetAlarm( uint32_t timeout )
{
    RtcStartAlarm( timeout );
}

void RtcStopAlarm( void )
{
	HAL_TIM_Base_Stop_IT(&htim);	// 停止定时器

//	LORA_DEBUG("RtcStopAlarm\n");
}

void RtcStartAlarm( uint32_t timeout )
{
	HAL_TIM_Base_Stop_IT(&htim);
	__HAL_TIM_SET_AUTORELOAD(&htim, timeout);	// 设置定时器自动加载值
	__HAL_TIM_SET_COUNTER(&htim,0);
	__HAL_TIM_CLEAR_FLAG(&htim,TIM_FLAG_UPDATE);
	HAL_TIM_Base_Start_IT(&htim);
	
//	LORA_DEBUG("RtcStartAlarm:%d,current_timer=%d\n",timeout,get_sys_time());
}


uint32_t RtcGetTimerValue( void )
{
    return ( uint32_t )get_rtc();
}

uint32_t RtcGetTimerElapsedTime( void )
{
	uint32_t time = ( uint32_t)( get_rtc()- RtcTimerContext.Time );

    return time;
}


uint32_t RtcGetCalendarTime( uint16_t *milliseconds )
{
    uint32_t sec;
	uint32_t remain_tick;
	sec = (uint32_t)(get_rtc()/RTC_FREQ);
	remain_tick = (uint32_t)(get_rtc()%RTC_FREQ);
    *milliseconds=(uint16_t)RtcTick2Ms(remain_tick);
    return sec;
}



void RtcBkupWrite( uint32_t data0, uint32_t data1 )
{

    RtcBkupRegisters[0] = data0;
    RtcBkupRegisters[1] = data1;

}

void RtcBkupRead( uint32_t* data0, uint32_t* data1 )
{

    *data0 = RtcBkupRegisters[0];
    *data1 = RtcBkupRegisters[1];

}

void RtcProcess( void )
{
}

TimerTime_t RtcTempCompensation( TimerTime_t period, float temperature )
{
    return period;
}


