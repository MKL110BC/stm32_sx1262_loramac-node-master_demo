#include "lorawan_common.h"
#include "board.h"
#include "LoRaMacCommands.h"

static LORAWAN_RECV_DEAL   lorawan_recv_deal;
static LoRaMacPrimitives_t macPrimitives;
static LoRaMacCallback_t macCallbacks;
static uint8_t linkcheck_margin;
static uint8_t linkcheck_gw_cnt;
static uint8_t linkcheck_flag = 0;
static uint8_t devtime_flag = 0;


/*!
 * MAC status strings
 */
const char* MacStatusStrings[24] =
{
    "OK",                            // LORAMAC_STATUS_OK
    "Busy",                          // LORAMAC_STATUS_BUSY
    "Service unknown",               // LORAMAC_STATUS_SERVICE_UNKNOWN
    "Parameter invalid",             // LORAMAC_STATUS_PARAMETER_INVALID
    "Frequency invalid",             // LORAMAC_STATUS_FREQUENCY_INVALID
    "Datarate invalid",              // LORAMAC_STATUS_DATARATE_INVALID
    "Frequency or datarate invalid", // LORAMAC_STATUS_FREQ_AND_DR_INVALID
    "No network joined",             // LORAMAC_STATUS_NO_NETWORK_JOINED
    "Length error",                  // LORAMAC_STATUS_LENGTH_ERROR
    "Region not supported",          // LORAMAC_STATUS_REGION_NOT_SUPPORTED
    "Skipped APP data",              // LORAMAC_STATUS_SKIPPED_APP_DATA
    "Duty-cycle restricted",         // LORAMAC_STATUS_DUTYCYCLE_RESTRICTED
    "No channel found",              // LORAMAC_STATUS_NO_CHANNEL_FOUND
    "No free channel found",         // LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND
    "Busy beacon reserved time",     // LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME
    "Busy ping-slot window time",    // LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME
    "Busy uplink collision",         // LORAMAC_STATUS_BUSY_UPLINK_COLLISION
    "Crypto error",                  // LORAMAC_STATUS_CRYPTO_ERROR
    "FCnt handler error",            // LORAMAC_STATUS_FCNT_HANDLER_ERROR
    "MAC command error",             // LORAMAC_STATUS_MAC_COMMAD_ERROR
    "ClassB error",                  // LORAMAC_STATUS_CLASS_B_ERROR
    "Confirm queue error",           // LORAMAC_STATUS_CONFIRM_QUEUE_ERROR
    "Multicast group undefined",     // LORAMAC_STATUS_MC_GROUP_UNDEFINED
    "Unknown error",                 // LORAMAC_STATUS_ERROR
};
/*!
 * MAC event info status strings.
 */
const char* EventInfoStatusStrings[17] =
{
    "OK",                            // LORAMAC_EVENT_INFO_STATUS_OK
    "Error",                         // LORAMAC_EVENT_INFO_STATUS_ERROR
    "Tx timeout",                    // LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT
    "Rx 1 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT
    "Rx 2 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT
    "Rx1 error",                     // LORAMAC_EVENT_INFO_STATUS_RX1_ERROR
    "Rx2 error",                     // LORAMAC_EVENT_INFO_STATUS_RX2_ERROR
    "Join failed",                   // LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL
    "Downlink repeated",             // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED
    "Tx DR payload size error",      // LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR
    "Downlink too many frames loss", // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS
    "Address fail",                  // LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL
    "MIC fail",                      // LORAMAC_EVENT_INFO_STATUS_MIC_FAIL
    "Multicast fail",                // LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL
    "Beacon locked",                 // LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED
    "Beacon lost",                   // LORAMAC_EVENT_INFO_STATUS_BEACON_LOST
    "Beacon not found"               // LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND
};


static void lora_app_combine_channel(LoRaMacRegion_t region,uint8_t channel_start, uint8_t channel_end,uint16_t *channel_mask)
{
	memset(channel_mask,0,12);
	for(uint8_t i=channel_start ; i<(channel_end+1) ; i++)
	{
		channel_mask[i/16]  |= 1<<(i%16);
	}
	if(region==LORAMAC_REGION_US915 ||region == LORAMAC_REGION_AU915)
	{
		for(uint8_t i=0 ; i<4  ;i++) //915 低速0-64信道，8信道一组，如果一组内设置了低速信道，需要开启对应的高速信道
		{
			if(channel_mask[i]&0x00ff)
			{
				channel_mask[4] |= 1<<(i*2);
			}
			if(channel_mask[i]&0xff00)
			{
				channel_mask[4] |= 1<<(i*2+1);
			}
		}
	}
}


static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    LORA_DEBUG( "\r\n###### ===== MCPS-Confirm ==== ######\r\n" );
    LORA_DEBUG( "STATUS      : %s\r\n", EventInfoStatusStrings[mcpsConfirm->Status] );
}
static void McpsIndication( McpsIndication_t *mcpsIndication )
{
	LORA_DEBUG( "\r\n###### ===== MCPS-Indication ==== ######\r\n" );
	LORA_DEBUG( "STATUS      : %s\r\n", EventInfoStatusStrings[mcpsIndication->Status] );
	
	if(lorawan_recv_deal!=NULL)
		lorawan_recv_deal(mcpsIndication->Buffer,mcpsIndication->BufferSize,mcpsIndication->Rssi,mcpsIndication->Snr,mcpsIndication->Port);
}
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    LORA_DEBUG( "\r\n###### ===== MLME-Confirm ==== ######\r\n" );
    LORA_DEBUG( "STATUS      : %s request:%d\r\n", EventInfoStatusStrings[mlmeConfirm->Status] ,mlmeConfirm->MlmeRequest);

    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
				LORA_DEBUG("+JOINING: JOINED\r\nOK\r\n");	
            }
            else
            {
				LORA_DEBUG("+JOINING: JOIN FAILED\r\nOK\r\n");
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                // Check DemodMargin
                // Check NbGateways
                linkcheck_flag = 1;
                linkcheck_gw_cnt = mlmeConfirm->NbGateways;
				linkcheck_margin = mlmeConfirm->DemodMargin;

                LORA_DEBUG("DemodMargin:%d,NbGateways:%d\r\n",mlmeConfirm->DemodMargin,mlmeConfirm->NbGateways);
            }
            break;
		}
		 case MLME_DEVICE_TIME:
		 {
			 if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
			 {
				devtime_flag = 1;
			 }
		 }
        default:
            break;
    }
}
static void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
	LORA_DEBUG( "\r\n###### ===== MLME-Indication ==== ######\r\n" );
	LORA_DEBUG( "STATUS      : %s\r\n", EventInfoStatusStrings[mlmeIndication->Status] );
	LORA_DEBUG( "MlmeIndication is %d\r\n" ,mlmeIndication->MlmeIndication);
}
static void OnMacProcessNotify( void )
{
	LORA_DEBUG("loramac start:%s\r\n",__FUNCTION__);
}




void lorawan_init(LoRaMacRegion_t	region)
{
	LoRaMacStatus_t status;
	RtcInit( );
	SX126xIoInit();

	macPrimitives.MacMcpsConfirm = McpsConfirm;//tx compelete
	macPrimitives.MacMcpsIndication = McpsIndication;

	macPrimitives.MacMlmeConfirm = MlmeConfirm;
	
	macPrimitives.MacMlmeIndication = MlmeIndication;
	macCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
	macCallbacks.GetTemperatureLevel = NULL;
	macCallbacks.NvmContextChange = NULL;
	macCallbacks.MacProcessNotify = OnMacProcessNotify;
	status = LoRaMacInitialization( &macPrimitives, &macCallbacks, region );
	if ( status != LORAMAC_STATUS_OK )
	{
		LORA_DEBUG( "LoRaMac wasn't properly initialized, error: %s", MacStatusStrings[status] );
		// Fatal error, endless loop.
		while ( 1 )
		{
		}
	}
	LoRaMacStart( );
}
LoRaMacStatus_t lorawan_deinit(void)
{
	return LoRaMacDeInitialization();
}
void lorawan_task(void)
{
	if( Radio.IrqProcess != NULL )
	{
		Radio.IrqProcess( );
	}

	LoRaMacProcess( );
}

void lora_app_register_lorawan_recv_func(LORAWAN_RECV_DEAL recv)
{
	lorawan_recv_deal = recv;
}

uint8_t lora_mac_set_channel( uint16_t *channel_mask)
{
	MibRequestConfirm_t mibset;
	mibset.Type  = MIB_CHANNELS_DEFAULT_MASK;
	mibset.Param.ChannelsDefaultMask=channel_mask;
	if(LoRaMacMibSetRequestConfirm( &mibset ) !=LORAMAC_STATUS_OK)
		return 0;
	mibset.Type  = MIB_CHANNELS_MASK;
	mibset.Param.ChannelsMask=channel_mask;
	if(LoRaMacMibSetRequestConfirm( &mibset ) !=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_dr(uint8_t dr)
{
	MibRequestConfirm_t mibset;
	mibset.Type  = MIB_CHANNELS_DATARATE;
	mibset.Param.ChannelsDatarate=dr;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;	
	return 1;
}
uint8_t lora_mac_get_dr(void)
{
	MibRequestConfirm_t mibget;
	mibget.Type = MIB_CHANNELS_DATARATE;
	LoRaMacMibGetRequestConfirm(&mibget);
	return mibget.Param.ChannelsDatarate;
}
uint8_t lora_mac_set_deveui( uint8_t *deveui)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_DEV_EUI;
	mibset.Param.DevEui = deveui;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_appeui( uint8_t *appeui)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_JOIN_EUI;
	mibset.Param.JoinEui = appeui;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_appkey( uint8_t *appkey)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_NWK_KEY;
	mibset.Param.AppKey = appkey;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_devaddr( uint32_t devaddr)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_DEV_ADDR;
	mibset.Param.DevAddr = devaddr;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint32_t lora_mac_get_devaddr(void)
{
	MibRequestConfirm_t mibget;
	mibget.Type = MIB_DEV_ADDR;
	LoRaMacMibGetRequestConfirm(&mibget);
	return mibget.Param.DevAddr;
}
uint8_t lora_mac_set_appskey( uint8_t *appskey)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_APP_S_KEY;
	mibset.Param.AppSKey = appskey;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_nwkskey( uint8_t *nwkskey)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_NWK_S_ENC_KEY;
	mibset.Param.NwkSEncKey = nwkskey;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_join_delay(uint32_t join_delay1,uint32_t join_delay2)
{
	if(join_delay1>=join_delay2)
		return 0;
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_JOIN_ACCEPT_DELAY_1;
	mibset.Param.JoinAcceptDelay1 = join_delay1;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	mibset.Type = MIB_JOIN_ACCEPT_DELAY_2;
	mibset.Param.JoinAcceptDelay2 = join_delay2;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
void lora_mac_get_join_delay(uint32_t *join_delay1,uint32_t *join_delay2)
{
	MibRequestConfirm_t mibget;
	if(join_delay1!=NULL)
	{
		mibget.Type = MIB_JOIN_ACCEPT_DELAY_1;
		LoRaMacMibGetRequestConfirm(&mibget);
		*join_delay1 = mibget.Param.JoinAcceptDelay1;
	}
	if(join_delay2!=NULL)
	{
		mibget.Type = MIB_JOIN_ACCEPT_DELAY_2;
		LoRaMacMibGetRequestConfirm(&mibget);
		*join_delay2 = mibget.Param.JoinAcceptDelay2;
	}
}
uint8_t lora_mac_set_join_delay1(uint32_t join_delay1)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_JOIN_ACCEPT_DELAY_1;
	mibset.Param.JoinAcceptDelay1 = join_delay1;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_join_delay2(uint32_t join_delay2)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_JOIN_ACCEPT_DELAY_2;
	mibset.Param.JoinAcceptDelay2 = join_delay2;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_rx1_delay(uint32_t rx1_delay)
{
	if(rx1_delay==0)
		rx1_delay = 1000;
	if(rx1_delay>=(0xffffffff-1000))
		return 0;
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_RECEIVE_DELAY_1;
	mibset.Param.ReceiveDelay1 = rx1_delay;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;

	mibset.Type = MIB_RECEIVE_DELAY_2;
	mibset.Param.ReceiveDelay2 = rx1_delay+1000;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint32_t lora_mac_get_rx1_delay(void)
{
	MibRequestConfirm_t mibget;
	mibget.Type = MIB_RECEIVE_DELAY_1;
	LoRaMacMibGetRequestConfirm(&mibget);
	return mibget.Param.ReceiveDelay1;
}
uint8_t lora_mac_set_rx2_channel(uint32_t freq,uint8_t dr)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_RX2_CHANNEL;
	mibset.Param.Rx2Channel.Frequency= freq;
	mibset.Param.Rx2Channel.Datarate = dr;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;

	mibset.Type = MIB_RX2_DEFAULT_CHANNEL;
	mibset.Param.Rx2DefaultChannel = mibset.Param.Rx2Channel;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;

	mibset.Type = MIB_RXC_DEFAULT_CHANNEL;
	mibset.Param.RxCDefaultChannel =mibset.Param.Rx2Channel;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;

	mibset.Type = MIB_RXC_CHANNEL;
    	mibset.Param.RxCChannel = mibset.Param.Rx2Channel;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_tx_power(int8_t tx_power)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_CHANNELS_DEFAULT_TX_POWER;
	mibset.Param.ChannelsDefaultTxPower = tx_power;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;

	mibset.Type = MIB_CHANNELS_TX_POWER;
	mibset.Param.ChannelsTxPower = tx_power;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
DeviceClass_t lora_mac_get_class_mode(void)
{
	MibRequestConfirm_t mibget;
	mibget.Type = MIB_DEVICE_CLASS;
	LoRaMacMibGetRequestConfirm(&mibget);
	return  mibget.Param.Class;
}
uint8_t lora_mac_set_class_mode(DeviceClass_t class_mode)
{
	//暂不支持classB、classC
	if(class_mode != CLASS_A)
		return 0;
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_DEVICE_CLASS;
	mibset.Param.Class = class_mode;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_rx_error_time(uint32_t error_time)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_SYSTEM_MAX_RX_ERROR;
	mibset.Param.SystemMaxRxError = error_time;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
uint8_t lora_mac_set_net_type(ActivationType_t type)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_NETWORK_ACTIVATION;
	mibset.Param.NetworkActivation = type;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	return 1;
}
ActivationType_t lora_mac_get_net_type(void)
{
	MibRequestConfirm_t mibget;
	mibget.Type = MIB_NETWORK_ACTIVATION;
	LoRaMacMibGetRequestConfirm(&mibget);
	return mibget.Param.NetworkActivation;
}
uint8_t lora_mac_set_antenna_gain(float antenna_gain)
{
	MibRequestConfirm_t mibset;
	mibset.Type = MIB_DEFAULT_ANTENNA_GAIN;
	mibset.Param.DefaultAntennaGain = antenna_gain;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;

	mibset.Type = MIB_ANTENNA_GAIN;
	mibset.Param.AntennaGain = antenna_gain;
	if(LoRaMacMibSetRequestConfirm(&mibset)!=LORAMAC_STATUS_OK)
		return 0;
	
	return 1;
}
void set_region_default(struct  lorawan_param *param)
{
	GetPhyParams_t getPhy;
	PhyParam_t phyParam;

	getPhy.Attribute = PHY_DEF_TX_POWER;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	param->tx_power = phyParam.Value;

	uint8_t dewelltime;
	getPhy.Attribute = PHY_DEF_UPLINK_DWELL_TIME;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	dewelltime = phyParam.Value;

	getPhy.Attribute = PHY_DEF_TX_DR;
	getPhy.UplinkDwellTime = dewelltime;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	param->dr= phyParam.Value;

	getPhy.Attribute = PHY_RECEIVE_DELAY1;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	param->rx1_delay= phyParam.Value;

	getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY1;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	param->join_delay1= phyParam.Value;

	getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY2;
	phyParam = RegionGetPhyParam(param->region, &getPhy );
	param->join_delay2 = phyParam.Value;

	getPhy.Attribute = PHY_DEF_RX2_FREQUENCY;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	param->rx2_freq= phyParam.Value;

	getPhy.Attribute = PHY_DEF_ANTENNA_GAIN;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	param->antenna_gain= phyParam.fValue;

	InitDefaultsParams_t params;
	params.Type = INIT_TYPE_DEFAULTS;
	params.NvmCtx = NULL;
	RegionInitDefaults( param->region, &params );

	getPhy.Attribute = PHY_CHANNELS_MASK;
	phyParam = RegionGetPhyParam( param->region, &getPhy );
	memcpy1((uint8_t*)(param->channels_mask),(uint8_t*)phyParam.Channels,12);
	

	if(param->region == LORAMAC_REGION_AU915 || param->region == LORAMAC_REGION_US915)
	{
		lora_app_combine_channel(param->region,8,15,param->channels_mask);
	}
	else if(param->region == LORAMAC_REGION_CN470)
	{
		lora_app_combine_channel(param->region,0,7,param->channels_mask);
	}
}

LoRaMacStatus_t lorawan_join_network(uint8_t dr)
{
	LoRaMacStatus_t status;
	MlmeReq_t mlmeReq;

	mlmeReq.Type = MLME_JOIN;
	mlmeReq.Req.Join.Datarate =  dr;

	// Starts the join procedure
	status = LoRaMacMlmeRequest( &mlmeReq );
	if( status == LORAMAC_STATUS_OK )
	{
		LORA_DEBUG("start join network\r\n");	
	}
	else
	{
		LORA_DEBUG("join network false,status is %s\r\n",MacStatusStrings[status]);
	}
	
	return status;
}
LoRaMacStatus_t  lorawan_send_linkcheck(uint8_t dr)
{
	MlmeReq_t mlmeReq;
	mlmeReq.Type = MLME_LINK_CHECK;
	LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
	McpsReq_t mcpsReq;
	mcpsReq.Type = MCPS_UNCONFIRMED;
	mcpsReq.Req.Unconfirmed.fBuffer = NULL;
	mcpsReq.Req.Unconfirmed.fBufferSize = 0;
	mcpsReq.Req.Unconfirmed.Datarate =  dr;
	status = LoRaMacMcpsRequest( &mcpsReq );

	return status;
}

uint8_t  lorawan_get_linkcheck_result(uint8_t *margin,uint8_t *gw_cnt)
{
	if(linkcheck_flag)
	{
		linkcheck_flag = 0;
		if(margin!=NULL)
			*margin = linkcheck_margin;
		if(gw_cnt!=NULL)
			*gw_cnt = linkcheck_gw_cnt;
		return 1;
	}
	else
		return 0;
}
LoRaMacStatus_t  lorawan_send_devtime(uint8_t dr)
{
	MlmeReq_t mlmeReq;
	mlmeReq.Type = MLME_DEVICE_TIME;
	LoRaMacStatus_t status = LoRaMacMlmeRequest( &mlmeReq );
	McpsReq_t mcpsReq;
	mcpsReq.Type = MCPS_UNCONFIRMED;
	mcpsReq.Req.Unconfirmed.fBuffer = NULL;
	mcpsReq.Req.Unconfirmed.fBufferSize = 0;
	mcpsReq.Req.Unconfirmed.Datarate = dr;
	status = LoRaMacMcpsRequest( &mcpsReq );

	return status;
}
uint8_t lorawan_get_devtime_result(void)
{
	if(devtime_flag)
	{
		devtime_flag = 0;
		return 1;
	}
	return 0;	
}
//设置的DR只有在ADR关闭的时候生效。ADR开启不生效
LoRaMacStatus_t lora_app_send_frame( uint8_t *buf,uint8_t buflen,uint8_t port,uint8_t is_confirm,uint8_t dr ,uint8_t confirm_nbtrials)
{
	McpsReq_t mcpsReq;
	LoRaMacTxInfo_t txInfo;
	LoRaMacStatus_t status;

	status = LoRaMacQueryTxPossible(buflen, &txInfo);
	if(status == LORAMAC_STATUS_OK)
	{
		if( is_confirm == false )
		{
			mcpsReq.Type = MCPS_UNCONFIRMED;
			mcpsReq.Req.Unconfirmed.fPort = port;
			mcpsReq.Req.Unconfirmed.fBuffer = buf;
			mcpsReq.Req.Unconfirmed.fBufferSize = buflen;
			mcpsReq.Req.Unconfirmed.Datarate = dr;
		}
		else
		{
			mcpsReq.Type = MCPS_CONFIRMED;
			mcpsReq.Req.Confirmed.fPort = port;
			mcpsReq.Req.Confirmed.fBuffer = buf;
			mcpsReq.Req.Confirmed.fBufferSize = buflen;
			mcpsReq.Req.Confirmed.NbTrials = confirm_nbtrials;
			mcpsReq.Req.Confirmed.Datarate = dr;
		}
		status = LoRaMacMcpsRequest( &mcpsReq );
	}

	return status;
}

void lorawan_get_sys_time(uint8_t *time)
{
	struct tm local_time;
	SysTime_t timestamp=SysTimeGet();
	SysTimeLocalTime(timestamp.Seconds,&local_time);
	*((uint16_t*)time) = 1900+local_time.tm_year;
	time[2] = local_time.tm_mon+1;
	time[3] = local_time.tm_mday;
	time[4] = local_time.tm_hour;
	time[5] = local_time.tm_min;
	time[6] = local_time.tm_sec;
}


