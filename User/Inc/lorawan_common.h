#ifndef __LORAWAN_COMMON_H__
#define __LORAWAN_COMMON_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "rtc-board.h"
#include "sx126x-board.h"
#include "board-config.h"


enum
{
	LORAWAN_UNCONFIRM_MSG	=	0,
	LORAWAN_CONFIRM_MSG		=	1,
};

enum
{
	LORAWAN_ADR_OFF		=	0,
	LORAWAN_ADR_ON		=	1,
};



#define LORAWAN_CERTIF_PORT			224
#define LORAWAN_CERTIF_DELAY		5000
#define LORAWAN_APP_DATA_MAX_SIZE	242
#define LORAWAN_MAX_APP_DATA_LEN		242


#define LORAWAN_MIN_APP_PORT			1
#define LORAWAN_MAX_APP_PORT			223

#define LORAWAN_MIN_CONFIRM_NBTRIALS	1
#define LORAWAN_MAX_CONFIRM_NBTRIALS	8



#define LORA_DEVEUI_LEN				8
#define LORA_APPEUI_LEN				8
#define LORA_APPKEY_LEN				16
#define	LORA_APPSKEY_LEN			16
#define	LORA_NWKSKEY_LEN			16
#define LORA_MC_APPSKEY_LEN			16
#define	LORA_MC_NWKSKEY_LEN			16



//协议栈接收处理函数指针
//buf为待处理的数据指针，buflen为数据长度，RSSI为接收数据信号强度，SNR为信噪比，port为接收数据端口
typedef  void(*LORAWAN_RECV_DEAL)(uint8_t* /*buf*/,uint8_t  /*buflen*/,int16_t /*rssi*/,int8_t  /*snr*/,uint8_t  /*port*/);

#pragma pack(1)


struct  lorawan_param
{
	LoRaMacRegion_t		region;
	DeviceClass_t   	class_type;
	ActivationType_t 	connect_type;

	//otaa param
	uint8_t  	deveui[LORA_DEVEUI_LEN];
	uint8_t  	appeui[LORA_APPEUI_LEN];
	uint8_t  	appkey[LORA_APPKEY_LEN];
	
	//ABP param
	uint32_t 	devaddr;
	uint8_t		appskey[LORA_APPSKEY_LEN];
	uint8_t 	nwkskey[LORA_NWKSKEY_LEN];
	
	//频段相关参数
	uint32_t 	join_delay1;
	uint32_t 	join_delay2;
	uint32_t	rx1_delay;   //rx2delay=rx1delay+1000ms
	uint16_t 	channels_mask[6];
	int8_t		tx_power;
	uint8_t 	dr;  //join dr 
	uint32_t 	rx2_freq;

	float		antenna_gain;  //天线增益
};

#pragma pack()
extern const char* MacStatusStrings[24];
extern const char* EventInfoStatusStrings[17] ;



void lorawan_init(LoRaMacRegion_t	region);
LoRaMacStatus_t lorawan_deinit(void);
LoRaMacStatus_t lorawan_join_network(uint8_t dr);

/*
FUNC:注册lorawan接收处理函数
PARAM:recv指向一个函数指针	
*/
void lora_app_register_lorawan_recv_func(LORAWAN_RECV_DEAL recv);


/*
FUNC:	lorawan 发送应用数据帧 
PARAM:	
		buf 发送数据的指针
		buflen发送数据长度
		port 发送数据端口，应用数据端口为1-223
		is_confirm	是否是确认帧
		dr		非ADR情况下数据发送的DR
		confirm_nbtrials    确认帧重发次数

RETURN:	
		LORAMAC_STATUS_OK发送成功
		LORAMAC_STATUS_DUTYCYCLE_RESTRICTED 受限制与dutycycle，调用lora_app_get_dutycycle_remain查询需要等待的时间
		LORAMAC_STATUS_LENGTH_ERROR  发送长度受限制，调用lora_app_get_tx_possible_len查询允许发送的长度
*/
LoRaMacStatus_t lora_app_send_frame( uint8_t *buf,uint8_t buflen,uint8_t port,uint8_t is_confirm,uint8_t dr ,uint8_t confirm_nbtrials);



void lorawan_task(void);
LoRaMacStatus_t  lorawan_send_linkcheck(uint8_t dr);
uint8_t  lorawan_get_linkcheck_result(uint8_t *margin,uint8_t *gw_cnt);
LoRaMacStatus_t  lorawan_send_devtime(uint8_t dr);
uint8_t lorawan_get_devtime_result(void);
void lorawan_get_sys_time(uint8_t *time);
uint8_t lora_mac_set_channel( uint16_t *channel_mask);
uint8_t lora_mac_set_dr(uint8_t dr);
uint8_t lora_mac_get_dr(void);
uint8_t lora_mac_set_deveui( uint8_t *deveui);
uint8_t lora_mac_set_appeui( uint8_t *appeui);
uint8_t lora_mac_set_appkey( uint8_t *appkey);
uint8_t lora_mac_set_devaddr( uint32_t devaddr);
uint32_t lora_mac_get_devaddr(void);
uint8_t lora_mac_set_appskey( uint8_t *appskey);
uint8_t lora_mac_set_nwkskey( uint8_t *nwkskey);


/*
FUNC:设置入网接收窗口1延迟时间//与lora_mac_set_join_delay功能一样，为了兼容不删除
PARAM:单位ms
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_join_delay1(uint32_t join_delay1);

/*
FUNC:设置入网接收窗口2延迟时间//与lora_mac_set_join_delay功能一样，为了兼容不删除
PARAM:单位ms
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_join_delay2(uint32_t join_delay2);


/*
FUNC:获取入网接收窗口延迟
PARAM:指向delay1 delay2的指针
*/
void lora_mac_get_join_delay(uint32_t *join_delay1,uint32_t *join_delay2);


/*
FUNC:设置应用接收窗口1延迟时间    ，RX2=RX1+1000
PARAM:单位ms
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_rx1_delay(uint32_t rx1_delay);


/*
FUNC:获取当前协议栈RX1delay，RX2=RX1+1000MS
RETURN:
*/
uint32_t lora_mac_get_rx1_delay(void);


/*
FUNC:设置RX2接收的频率以及dr
PARAM:
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_rx2_channel(uint32_t freq,uint8_t dr);

/*
FUNC:设置发射功率
PARAM:当前频段允许设置的范围,不同频段允许的范围不一样
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_tx_power(int8_t tx_power);

/*
FUNC:获取当前协议栈的class类型
RETURN:当前的class类型
*/
DeviceClass_t lora_mac_get_class_mode(void);

/*
FUNC:设置class类型, 
PARAM: CLASS_A/CLASS_C
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_class_mode(DeviceClass_t class_mode);


/*
FUNC:设置rx窗口容错时间
PARAM: default is10ms
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_rx_error_time(uint32_t error_time);


/*
FUNC:设置网络类型，主要用于设置ABP模式
PARAM: 
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_net_type(ActivationType_t type);


/*
FUNC:获取当前网络类型，可以通过此函数判断入网成功与否
RETURN:ACTIVATION_TYPE_NONE表示OTAA模式未入网，ACTIVATION_TYPE_OTAA表示OTAA模式入网成功
*/
ActivationType_t lora_mac_get_net_type(void);

/*
FUNC:设置天线增益
PARAM: 设置的天线增益值
RETURN:0设置失败，1设置成功
*/
uint8_t lora_mac_set_antenna_gain(float antenna_gain);

void set_region_default(struct  lorawan_param *param);
#endif
