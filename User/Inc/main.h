/**
  ******************************************************************************
  * @file    Templates/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define FIRMWARE_VERSION		"V0.2"

#define LOG_DEBUG_EN		1
#if LOG_DEBUG_EN
#include "SEGGER_RTT.h"
#define APP_INFO(...)								SEGGER_RTT_printf(0,__VA_ARGS__)
#define APP_INFO_DUMP_BUF(str,buf,len )							\
			do														\
			{														\
				APP_INFO("%s ",str);								\
				for(uint16_t i=0;i<len;i++)							\
				{APP_INFO("%02X ",*(buf+i));}						\
				APP_INFO("\r\n");									\
			} while (0)


#else
#define APP_INFO(...)
#define APP_INFO_DUMP_BUF(str,buf,size)
#endif



#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
