#include "lorawan_demo.h"
#include "system_time.h"

static struct lorawan_param  lora_param;
static uint8_t lora_param_net_state = LORAWAN_NET_JOINING;


static void lora_param_recv_deal(uint8_t*buf,uint8_t buflen ,int16_t rssi ,int8_t snr,uint8_t  port)
{
	if(buflen)
	{
		LORA_INFO_DUMP_BUF("lora recv:", buf, buflen);
	}
}

static void lora_app_para_init(void)
{
	static uint8_t init_flag = 0;
	if(init_flag == 0)
	{
		init_flag = 1;
		lora_app_register_lorawan_recv_func(lora_param_recv_deal);
	}

	lora_mac_set_channel(lora_param.channels_mask);
	lora_mac_set_tx_power(lora_param.tx_power);
	lora_mac_set_dr(lora_param.dr);

	lora_mac_set_rx_error_time(20);

	if(lora_param.connect_type == ACTIVATION_TYPE_ABP)
	{	
		lora_mac_set_devaddr(lora_param.devaddr);
		lora_mac_set_appskey(lora_param.appskey);
		lora_mac_set_nwkskey(lora_param.nwkskey);
		lora_mac_set_net_type(lora_param.connect_type);

	}
	else if(lora_param.connect_type== ACTIVATION_TYPE_OTAA)
	{
		lora_mac_set_deveui( lora_param.deveui);
		lora_mac_set_appeui(lora_param.appeui);
		lora_mac_set_appkey(lora_param.appkey);
	}
	lora_mac_set_join_delay1(lora_param.join_delay1);
	lora_mac_set_join_delay2(lora_param.join_delay2);
	lora_mac_set_rx1_delay(lora_param.rx1_delay);
	lora_mac_set_class_mode(lora_param.class_type);
	lora_mac_set_antenna_gain(lora_param.antenna_gain);
	lora_mac_set_rx2_channel(lora_param.rx2_freq, lora_param.dr);
}



static void lorawan_set_net_joined(void)
{
	lora_param_net_state = LORAWAN_NET_JOINED;
}

//入网处理
static void loraapp_join_network(void)
{
	static uint8_t	state = 0;
	static uint32_t timer;
	LoRaMacStatus_t status;
	
	switch(state)
	{
		case 0:
			if(lora_param.connect_type ==ACTIVATION_TYPE_OTAA )
			{
				if(lora_param_net_state==LORAWAN_NET_JOINING)
				{
					state = 1;
					lora_mac_set_net_type(ACTIVATION_TYPE_NONE);
					LORA_INFO("----otaa join network\r\n");
				}
			}
			else 
			{
				state = 0xFF;
				lora_mac_set_class_mode(lora_param.class_type);
				LORA_INFO("----abp mode and linkcheck interval is disable\r\n");
				lorawan_set_net_joined();
			}
			break;
		case 1:
			if(LoRaMacIsBusy()==false)
			{
				status = lorawan_join_network(lora_param.dr);
			
				if(status == LORAMAC_STATUS_OK)
				{
					state = 2;
					LORA_DEBUG("---loraapp_join_network status %s\n\n",MacStatusStrings[status]);
				}
				else
				{
					LORA_DEBUG("----loraapp_join_network req false,error is %s\n\n",MacStatusStrings[status]);
					state = 3;
				}
			}
			break;
		case 2:
			if(LoRaMacIsBusy()==false)
			{			
				if(lora_mac_get_net_type() == ACTIVATION_TYPE_OTAA)
				{
					state = 0;
					lorawan_set_net_joined();
					LORA_INFO("----otaa join network success\r\n");
					break;
				}
				else
				{
					timer = get_sys_time();
					state = 3;
				}
			}
			break;
		case 3:
			if(timepassed(timer,OTAA_REJOIN_BASE_INTERVAL))
			{
				state = 1;
			}
			break;
	}
	//classC模式下在OTAA入网之后切换模式
	if(lora_mac_get_net_type()==ACTIVATION_TYPE_OTAA   && lora_param.class_type != lora_mac_get_class_mode())
	{
		lora_mac_set_class_mode( lora_param.class_type);
	}
}


//网络检测
static void loraapp_link_check()
{
	static uint8_t state = 0;
	static uint32_t timer = 0;
	static uint32_t wait_time;

	LoRaMacStatus_t status;

	switch(state)
	{
		case 0:
			if(lora_param.connect_type == ACTIVATION_TYPE_OTAA && lora_param_net_state==LORAWAN_NET_JOINED)
			{
				state = 1;
				timer = get_sys_time();
				wait_time = ONE_HOUR;
			}
			break;
		case 1:
			if(timepassed(timer,wait_time))
			{
				state = 2;
			}
			break;
		case 2:
			if(LoRaMacIsBusy()==false)
			{
				status = lorawan_send_linkcheck(lora_mac_get_dr());
				if(status == LORAMAC_STATUS_OK)
				{	
					state = 3;
				}
				else
				{
					state = 1;
					wait_time = LINKCHECK_BASE_INTERVAL;
					timer = get_sys_time();
				}
				LORA_DEBUG("---lorawan_send_linkcheck req %s\n\n",MacStatusStrings[status]);
			}
			break;
		case 3:
			if(LoRaMacIsBusy()==false)
			{
				uint8_t margin,gw_cnt;
				if(lorawan_get_linkcheck_result(&margin,&gw_cnt))
				{
					state = 0;

					LORA_INFO("linkcheck is sucess ,margin is %d,gw_cnt is %d\r\n",margin,gw_cnt);
				}
				else
				{
					lora_param_net_state = LORAWAN_NET_JOINING;
					state = 0;
					LORA_INFO("linkcheck is false\r\n");
				}
			}
			break;
	}
}



//时间同步
static void loraapp_get_utc_time(void)
{
	static uint8_t state = 0;
	static uint32_t wait_time;
	static uint32_t timer;
	LoRaMacStatus_t status;

	switch(state)
	{
		case 0:
			if(lora_param_net_state == LORAWAN_NET_JOINED && LoRaMacIsBusy()==false)
			{
				status = lorawan_send_devtime(lora_mac_get_dr());
				if(status == LORAMAC_STATUS_OK)
				{
					state = 1;
				}

				LORA_INFO("---lora_param get utc time  status is %d--\n",status);
			}
			break;
		case 1:
			if(LoRaMacIsBusy()==false)
			{
				if(lorawan_get_devtime_result())
				{
					state = 2;
					wait_time = ONE_HOUR;
					
					struct time_struct time;
					lorawan_get_sys_time((uint8_t *)&time);//time是获取到的年月日，格式：年+1900，月+1
					LORA_INFO("----time:%d-%d-%d %d:%d:%d \r\n",time.year,time.mon,time.day,time.hour,time.minute,time.sec);
				}
				else
				{
					wait_time = DEVTIME_BASE_INTERVAL;
					state = 2;
				}

				timer = get_sys_time();
			}
			break;
		case 2:
			if(timepassed( timer, wait_time))
			{
				state = 0;
			}
			break;
	}
}

uint8_t loraapp_get_net_state(void)
{
	return lora_param_net_state;
}

//发送数据
uint8_t lorawan_send_data( uint8_t *buf,uint8_t len,uint8_t port)
{
	LoRaMacStatus_t status;
	static uint8_t state = 0;
	static uint8_t dr = 0;
	static uint32_t timer;
	static uint32_t wait_time;
	uint8_t result = 0;

	switch(state)
	{
		case 0:
			if(LoRaMacIsBusy()==false)
			{
				status = lora_app_send_frame(buf, len, port ,LORA_SEND_MSG_TYPE, lora_mac_get_dr(), LORA_CONFIRM_NBTRIALS);
				
				if(status == LORAMAC_STATUS_OK)
				{	
					state = 1;
				}
				else if(status == LORAMAC_STATUS_LENGTH_ERROR)
				{
					dr	=  lora_mac_get_dr();
					state = 2;
				}
				else
				{
					state = 3;
					timer = get_sys_time();
					wait_time = 10*ONE_SEC;
				}
			}
			break;
		case 1:
			if(LoRaMacIsBusy()==false)
			{
				result = 1;
				state = 0;
				LORA_INFO("[lorawan_send_data]:send complete \r\n");
			}
			break;
		case 2:
			if(lora_mac_set_dr(dr+1))
			{
				dr += 1;
				state = 0; 
				LORA_INFO("[lorawan_send_data]:set dr = %d \r\n",dr);
			}
			else
			{
				state = 0; 
			}
			break;
			
		case 3:
			if(timepassed(timer,wait_time))
			{
				state = 0;
				LORA_INFO("[lorawan_send_data]:send failed,status = %d \r\n",status);
			}
			break;
	}
	
	return result;
}



void loraapp_init(struct lorawan_param *param)
{
	/*lora_param param*/
	memset(&lora_param,0,sizeof(lora_param));
	lora_param = *param;
	lorawan_init(lora_param.region);

	LORA_INFO("lora init,region = %d \r\n",lora_param.region);
	LORA_INFO_DUMP_BUF("deveui is :",lora_param.deveui,8);
	LORA_INFO_DUMP_BUF("appeui is :",lora_param.appeui,8);
}

void loraapp_process(void)
{
	static uint8_t flag = 0;
	if(flag == 0)
	{
		lora_app_para_init();
		flag = 1;
	}
	loraapp_join_network();
	loraapp_get_utc_time();
	loraapp_link_check();
	lorawan_task();
}

