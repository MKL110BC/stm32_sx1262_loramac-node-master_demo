#ifndef __LOG__H
#define __LOG__H


#define LOG_DEBUG_EN		1
#if LOG_DEBUG_EN
#include "SEGGER_RTT.h"
#define LORA_DEBUG(...)								SEGGER_RTT_printf(0,__VA_ARGS__)
#define LORA_DEBUG_DUMP_BUF(str,buf,len )							\
			do														\
			{														\
				LORA_DEBUG("%s ",str);								\
				for(uint16_t i=0;i<len;i++)							\
				{LORA_DEBUG("%02X ",*(buf+i));}						\
				LORA_DEBUG("\r\n");									\
			} while (0)


#else
#define LORA_DEBUG(...)
#define LORA_DEBUG_DUMP_BUF(str,buf,size)
#endif



#endif
