/*!

	20200814 by ysh
	将所有函数内容设置为空，后续需要移植BoardGetBatteryLevel函数(认证需要)
 */

#include "board.h"
#include "stm32l1xx_hal.h"


void BoardCriticalSectionBegin( uint32_t *mask )
{
	*mask = __get_PRIMASK();
	__disable_irq();
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
	__set_PRIMASK(*mask);
}

void BoardInitPeriph( void )
{

}

void BoardInitMcu( void )
{

}

void BoardResetMcu( void )
{

}

void BoardDeInitMcu( void )
{

}

uint32_t BoardGetRandomSeed( void )
{
	return 0;  
}

void BoardGetUniqueId( uint8_t *id )
{

}

uint16_t BoardBatteryMeasureVoltage( void )
{
    return 0;
}

uint32_t BoardGetBatteryVoltage( void )
{
    return 0;
}

//这一块的函数需要实现，函数声明处有函数说明
uint8_t BoardGetBatteryLevel( void )
{
    return 0;
}


void SystemClockConfig( void )
{
   
}

void CalibrateSystemWakeupTime( void )
{

}

void SystemClockReConfig( void )
{
 
}

//void SysTick_Handler( void )
//{
//  
//}

uint8_t GetBoardPowerSource( void )
{
	return 0;
}

/**
  * \brief Enters Low Power Stop Mode
  *
  * \note ARM exists the function when waking up
  */
void LpmEnterStopMode( void)
{
 
}

/*!
 * \brief Exists Low Power Stop Mode
 */
void LpmExitStopMode( void )
{

}

/*!
 * \brief Enters Low Power Sleep Mode
 *
 * \note ARM exits the function when waking up
 */
void LpmEnterSleepMode( void)
{
   
}

void BoardLowPowerHandler( void )
{

}

