#ifndef __SYSTEM_TIME_H__
#define __SYSTEM_TIME_H__

#include "stm32l1xx_hal.h"

#define ONE_SEC                 (1000)
#define ONE_MIN                 (60*ONE_SEC)
#define ONE_HOUR                (60*ONE_MIN)


struct time_struct
{
	uint16_t year;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t sec;
};
#pragma pack()

uint32_t get_sys_time(void);
uint8_t timepassed(uint32_t timer,uint32_t passed_ms);
#endif

