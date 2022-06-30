#ifndef __LORAWAN_DEMO_H__
#define __LORAWAN_DEMO_H__
#include "lorawan_common.h"

#define htonl(x)    ( (x<<24 &0xff000000)|(x<<8 & 0x00ff0000)|(x>>8 &0x0000ff00)|(x>>24 &0x000000ff))
#define htons(x)	((x<<8 &0xff00)|(x>>8 &0x00ff))


#define OTAA_REJOIN_BASE_INTERVAL			(5*ONE_MIN)	
#define LINKCHECK_BASE_INTERVAL				(ONE_MIN)
#define DEVTIME_BASE_INTERVAL				(5*ONE_MIN)

#define LORA_SEND_MSG_TYPE	 				LORAWAN_UNCONFIRM_MSG			//非确认帧
#define LORA_CONFIRM_NBTRIALS				LORAWAN_MIN_CONFIRM_NBTRIALS	//非确认帧重传次数

enum
{
	LORAWAN_NET_JOINING = 0,
	LORAWAN_NET_JOINED,
};

#pragma pack(1)
struct lorawan_send_list
{
	uint8_t port;
	uint8_t *buf;
	uint8_t buflen;
	struct lorawan_send_list *next;
};
#pragma pack()


void loraapp_init(struct lorawan_param *param);
void loraapp_process(void);
uint8_t loraapp_get_net_state(void);
uint8_t lorawan_send_data( uint8_t *buf,uint8_t len,uint8_t port);

#define LORAAPP_DEBUG_EN		1
#if LORAAPP_DEBUG_EN
#include "SEGGER_RTT.h"
#define LORA_INFO(...)								SEGGER_RTT_printf(0,__VA_ARGS__)
#define LORA_INFO_DUMP_BUF(str,buf,len )							\
			do														\
			{														\
				LORA_INFO("%s ",str);								\
				for(uint16_t i=0;i<len;i++)							\
				{LORA_INFO("%02X ",*(buf+i));}						\
				LORA_INFO("\r\n");									\
			} while (0)


#else
#define LORA_INFO(...)
#define LORA_INFO_DUMP_BUF(str,buf,size)
#endif

#endif
