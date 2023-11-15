/* 
*   BSD 2-Clause License
*   Copyright (c) 2022, LiteEMF
*   All rights reserved.
*   This software component is licensed by LiteEMF under BSD 2-Clause license,
*   the "License"; You may not use this file except in compliance with the
*   License. You may obtain a copy of the License at:
*       opensource.org/licenses/BSD-2-Clause
* 
*/

#include "hw_config.h"
#if API_BT_ENABLE
#include "api/bt/api_bt.h"
#include "api/api_log.h"
#if API_PM_ENABLE
#include "api/api_pm.h"
#endif



#include "btcontroller_config.h"
#include "btctrler/btctrler_task.h"
#include "config/config_transport.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_common.h"
#include "edr_hid_user.h"
#include "le_common.h"
#include "standard_hid.h"
#include "rcsp_bluetooth.h"
#include "app_comm_bt.h"

/*******************************************************************************************************************
**	Hardware  Defined
********************************************************************************************************************/


/*******************************************************************************************************************
**	public Parameters
********************************************************************************************************************/

/*******************************************************************************************************************
**	static Parameters
********************************************************************************************************************/
uint8_t bas_mac[6];

#if TCFG_USER_BLE_ENABLE
static const ble_init_cfg_t ble_default_config = {
    .same_address = 0,
    .appearance = BLE_ICON,
    .report_map = NULL,         //hid map 在后面设置
    .report_map_size = 0,
};
#endif

#if TCFG_USER_EDR_ENABLE
//==========================================================
#if SNIFF_MODE_RESET_ANCHOR               //键盘鼠标sniff模式,固定小周期发包,多按键响应快
    #define SNIFF_MODE_TYPE               SNIFF_MODE_ANCHOR
    #define SNIFF_CNT_TIME                1/////<空闲100mS之后进入sniff模式

    #define SNIFF_MAX_INTERVALSLOT        8	//从机anchor point锚点间隔,间隔到了就会唤醒收发数据  time=n*0.625ms
    #define SNIFF_MIN_INTERVALSLOT        8
    #define SNIFF_ATTEMPT_SLOT            2		//锚点时间到唤醒后保持监听M->S slot个数, 必须<=SNIFF_MIN_INTERVALSLOT/2(注意监听要包含S-M slot,所以时间是SLOT*(2+1))
    #define SNIFF_TIMEOUT_SLOT            1		//监听到数据后,延长监听的格式,防止太快进入休眠 
    #define SNIFF_CHECK_TIMER_PERIOD      100
#else                                     //待机固定500ms sniff周期,待机功耗较低,按键唤醒有延时
    #define SNIFF_MODE_TYPE               SNIFF_MODE_DEF
    #define SNIFF_CNT_TIME                5/////<空闲5S之后进入sniff模式

    #define SNIFF_MAX_INTERVALSLOT        800
    #define SNIFF_MIN_INTERVALSLOT        100
    #define SNIFF_ATTEMPT_SLOT            4
    #define SNIFF_TIMEOUT_SLOT            1
    #define SNIFF_CHECK_TIMER_PERIOD      1000
#endif

//默认配置
static const edr_sniff_par_t hid_sniff_param = {
    .sniff_mode = SNIFF_MODE_TYPE,
    .cnt_time = SNIFF_CNT_TIME,
    .max_interval_slots = SNIFF_MAX_INTERVALSLOT,
    .min_interval_slots = SNIFF_MIN_INTERVALSLOT,
    .attempt_slots = SNIFF_ATTEMPT_SLOT,
    .timeout_slots = SNIFF_TIMEOUT_SLOT,
    .check_timer_period = SNIFF_CHECK_TIMER_PERIOD,
};

//----------------------------------
static const edr_init_cfg_t edr_default_config = {
    .page_timeout = 8000,
    .super_timeout = 8000,

#if SUPPORT_USER_PASSKEY            //io_capabilities ; /*0: Display only 1: Display YesNo 2: KeyboardOnly 3: NoInputNoOutput*/
    .io_capabilities = 2,
#else
    .io_capabilities = 3,
#endif

    .authentication_req = 0,        //authentication_requirements: 0:NO Bonding  1 :No bonding io cap to auth,2:Bonding,3:boding io cap auth,4: boding ,5:boding io cap auth
									//cesar fix 2 to 0
    .oob_data = 0,          		//cesar fix 2 to 0
    .sniff_param = &hid_sniff_param,
    .class_type = EDR_ICON,
    .report_map = NULL,         //不在这里设置
    .report_map_size = 0,
};
#endif

/*****************************************************************************************************
**	static Function
******************************************************************************************************/
extern void ble_set_pair_addrinfo(u8 *addr_info);


void bt_sniff_param_hook(u8 *addr, u16 t_sniff)
{
    uint32_t interval_us;
    api_bt_ctb_t* bt_ctbp = api_bt_get_ctb(BT_EDR);

    interval_us = t_sniff*625;
    logd_g("bt edr sniff = %d %dus...\n",t_sniff, interval_us);

    if(NULL != bt_ctbp){
        bt_ctbp->inteval_10us = interval_us / 10;
    }
}

//选择蓝牙从机模式
bool hal_bt_select_mode(uint8_t id,uint16_t trps)
{
    uint8_t i;
    api_bt_ctb_t* bt_ctbp;

    logi("bt trps %x\n", trps);

    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    u8 tmp_addr_info[7];
    if(trps & BT0_SUPPORT & BIT(TR_BLE_RF)){
        if (7 == syscfg_read(CFG_AAP_MODE_24G_ADDR, tmp_addr_info, 7)) {    //切换恢复ble配对信息
            ble_set_pair_addrinfo(tmp_addr_info);
        } else {
            ble_set_pair_addrinfo(NULL);
        }
    }else if(trps & BT0_SUPPORT & BIT(TR_BLE)) {
        if (7 == syscfg_read(CFG_AAP_MODE_BLE_ADDR, tmp_addr_info, 7)) {    //切换恢复ble配对信息
            ble_set_pair_addrinfo(tmp_addr_info);
        } else {
            ble_set_pair_addrinfo(NULL);
        }
    }

    if(trps & BT0_SUPPORT & BIT(TR_BLE_RF)){
        bt_ctbp = api_bt_get_ctb(TR_BLE_RF);
	    
        logi("---------app select 24g--------\n");
        rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
        while (ble_hid_is_connected()) {
            putchar('W');
            os_time_dly(1);
        }
		if(api_bt_is_bonded(BT_ID0, TR_BLE_RF)){
            le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 0);
			call_hogp_adv_config_set();
        }
        if(NULL != bt_ctbp){
            hal_bt_enable(BT_ID0,TR_BLE_RF,bt_ctbp->enable);
        }
    }else if(trps & BT0_SUPPORT & BIT(TR_BLE)) {
        logi("---------app select ble--------\n");
        bt_ctbp = api_bt_get_ctb(TR_BLE);
        rf_set_24g_hackable_coded(0);
        while (ble_hid_is_connected()) {
            putchar('W');
            os_time_dly(1);
        }
        if(NULL != bt_ctbp){
            hal_bt_enable(BT_ID0,BT_BLE,bt_ctbp->enable);
        }
    }else{
        hal_bt_enable(BT_ID0, BT_BLE, 0);
    }
    #endif


    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    if(trps & BT0_SUPPORT & BIT(TR_EDR)) {
        #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
        bt_ctbp = api_bt_get_ctb(TR_EDR);
        logi("---------app select edr--------\n");
        hal_bt_enable(BT_ID0,BT_EDR,bt_ctbp->enable);          //打开edr
		#endif
    }else{
        hal_bt_enable(BT_ID0, BT_EDR, 0);
    }
    #endif
    return true;
}


//BLE 从机接收数据
void ble_hid_transfer_channel_recieve(uint8_t* p_attrib_value,uint16_t length)
{
    bt_evt_rx_t evt;
    evt.bts = BT_UART;
	evt.buf = p_attrib_value;
	evt.len = length;
    if(length) api_bt_event(BT_ID0,BT_BLE,BT_EVT_RX,&evt);    
}

//edr 从机接收数据
void edr_out_callback(u8 *buffer, u16 length, u16 channel)
{
    bt_evt_rx_t evt;
    evt.bts = BT_HID;
	evt.buf = buffer+1;     //注意要去掉0XA2
	evt.len = length-1;

    logd("%s,chl=%d,len=%d", __FUNCTION__, channel, length);
    if(length) api_bt_event(BT_ID0,BT_EDR,BT_EVT_RX,&evt);
}

void ble_status_callback(ble_state_e status, u8 reason)
{
    bt_t bt;
    logd("----%s reason %x %x", __FUNCTION__, status, reason);

    if(m_trps & BT0_SUPPORT & BIT(BT_BLE_RF)){
        bt = BT_BLE_RF;
    }else{
        bt = BT_BLE;
    }

    switch (status) {
    #if CONFIG_BT_GATT_SERVER_NUM
    case BLE_ST_IDLE:
        api_bt_event(BT_ID0,bt,BT_EVT_IDLE,NULL);
        break;
    case BLE_ST_ADV:
        api_bt_event(BT_ID0,bt,BT_EVT_ADV,NULL);
        break;
    case BLE_ST_CONNECT:
        api_bt_event(BT_ID0,bt,BT_EVT_CONNECTED,NULL);
        //选择物理层,这里不设置详见SET_SELECT_PHY_CFG
        // if(m_trps & BT0_SUPPORT & BIT(BT_BLE_RF)){
        //     ble_comm_set_connection_data_phy(reason, CONN_SET_1M_PHY, CONN_SET_1M_PHY, CONN_SET_PHY_OPTIONS_NONE);//向对端发起phy通道更改
        // }

        #if (20 != API_BT_LL_MTU)
        ble_comm_set_connection_data_length(ble_hid_is_connected(), API_BT_LL_MTU+7, 2120);
        #endif

        break;
    case BLE_ST_SEND_DISCONN:
        break;
    case BLE_ST_DISCONN:
        api_bt_event(BT_ID0,bt,BT_EVT_DISCONNECTED,NULL);
        break;
    case BLE_ST_NOTIFY_IDICATE:
        api_bt_event(BT_ID0,bt,BT_EVT_READY,NULL);;
        break;
    case BLE_PRIV_PAIR_ENCRYPTION_CHANGE:               //ble 2.4g切换保存当前配对信息
        logd("BLE_PRIV_PAIR_ENCRYPTION_CHANGE\n");
        // pair_info_address_update(bt, ble_cur_connect_addrinfo());   //TODO
        break;
    #endif
    default:
        break;
    }
}

/*************************************************************************************************/
/*  \brief      蓝牙连接状态事件消息处理
/*************************************************************************************************/
static int bt_connction_status_event_handler(struct bt_event *bt)
{
    uint8_t id;

    logd("----%s %d", __FUNCTION__, bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        //蓝牙初始化完成
        logd("BT_STATUS_INIT_OK\n");

        ble_set_fix_pwr(9);//range:0~9

        #if TCFG_USER_BLE_ENABLE
        btstack_ble_start_after_init(0);
        #endif

        #if TCFG_USER_EDR_ENABLE
        btstack_edr_start_after_init(0);
        #endif
        
        for(id=0; id<BT_MAX; id++){
            if(BT0_SUPPORT & BIT(id)){      //全部模式
                api_bt_event(BT_ID0, (bt_t)id, BT_EVT_INIT, NULL);
            }
        }
        hal_bt_select_mode(BT_ID0,m_trps);
        break;

    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_FIRST_CONNECTED:
        api_bt_event(BT_ID0,BT_EDR,BT_EVT_CONNECTED,NULL);
        api_bt_event(BT_ID0,BT_EDR,BT_EVT_READY,NULL);
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        api_bt_event(BT_ID0,BT_EDR,BT_EVT_DISCONNECTED,NULL);
        break;
    #endif
    default:
        #if TCFG_USER_EDR_ENABLE
        bt_comm_edr_status_event_handler(bt);
        #endif

        #if TCFG_USER_BLE_ENABLE
        bt_comm_ble_status_event_handler(bt);
        #endif
        break;
    }
    return 0;
}



/*************************************************************************************************/
/*  \brief      蓝牙HCI事件消息处理
/*************************************************************************************************/
static int bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    logd("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    #if TCFG_USER_EDR_ENABLE
    bt_comm_edr_hci_event_handler(bt);
    #endif

    #if TCFG_USER_BLE_ENABLE
    bt_comm_ble_hci_event_handler(bt);
    #endif

    return 0;
}
/*************************************************************************************************/
/*\brief      蓝牙公共消息处理
/*************************************************************************************************/
static int bt_common_event_handler(struct bt_event *bt)
{
    logd("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_EDR_REMOTE_TYPE:
        logd(" COMMON_EVENT_EDR_REMOTE_TYPE,%d \n", bt->value);
        break;

    case COMMON_EVENT_BLE_REMOTE_TYPE:
        logd(" COMMON_EVENT_BLE_REMOTE_TYPE,%d \n", bt->value);
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        #if API_PM_ENABLE
        m_pm_sleep_timer = m_systick;
        #endif
        break;

    default:
        break;

    }
    return 0;
}

int sys_bt_event_handler(struct sys_event *event)
{
    #if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
    
    if ((u32)event->arg == SYS_BT_EVENT_BLE_STATUS) {
        ble_status_callback(event->u.bt.event, event->u.bt.value);
    }else if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
        bt_connction_status_event_handler(&event->u.bt);
    } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
        bt_hci_event_handler(&event->u.bt);
    } else if ((u32)event->arg == SYS_BT_EVENT_FORM_COMMON) {
        return bt_common_event_handler(&event->u.dev);
    }
    #endif
}

   
/*****************************************************************************************************
**  hal bt Function
******************************************************************************************************/
bool hal_bt_get_mac(uint8_t id, bt_t bt, uint8_t *buf )
{
    if(BT_ID0 != id) return false;

    memcpy(buf,bas_mac,6);        //获取基础mac地址, EDR地址为基地址

    return true;
}

bool hal_bt_is_bonded(uint8_t id, bt_t bt)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        extern bool multi_server_is_bonded(void);
        return  multi_server_is_bonded();
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        ret = multi_client_is_bonded();
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
        extern u8 connect_last_device_from_vm();    
        return  connect_last_device_from_vm();
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        break;
    #endif
    }   
    
    return ret;
}
bool hal_bt_debond(uint8_t id, bt_t bt)
{
    bool ret = false;
    api_bt_ctb_t* bt_ctbp;

    if(BT_ID0 != id) return false;

	bt_ctbp = api_bt_get_ctb(bt);

    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
         if(bt != BT_EDR){
            clear_app_bond_info(bt);
            ble_gatt_server_module_enable(0);			//断开连接
            sdk_bt_adv_set(bt, BLE_ADV_IND);            //说明必须修改设置后再开启广播
            
            ble_gatt_server_module_enable(1);			//重新开启广播
            ble_gatt_server_adv_enable(bt_ctbp->enable);
            ret = true;
        }
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        ret = !multi_client_clear_pair();
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	 
        ret = !delete_last_device_from_vm();   
        user_hid_disconnect();
        bt_wait_phone_connect_control(1);                 //开启蓝牙可发现可链接
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        //unsupport
        break;
    #endif
    }   
    
    return ret;
}

bool hal_bt_disconnect(uint8_t id, bt_t bt)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        le_hogp_disconnect();
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
        user_hid_disconnect();    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        break;
    #endif
    }   
    
    return ret;
}
bool hal_bt_enable(uint8_t id, bt_t bt,bool en)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        ble_gatt_server_module_enable(en);
        if(en) ble_gatt_server_adv_enable(en);
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        if(en){
            ble_gatt_client_module_enable(1);
            ble_gatt_client_scan_enable(1);
        }else{
            ble_gatt_client_scan_enable(0);
        }
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
        if (en) {
            user_hid_enable(1);
            btctrler_task_init_bredr();
            if(!bt_connect_phone_back_start()){     //先回连
                bt_wait_phone_connect_control(1);
            }
            if(!edr_sniff_by_remote){
                sys_auto_sniff_controle(1, NULL);
            }

        } else {
            user_hid_enable(0);
            bt_wait_phone_connect_control(0);
            if(!edr_sniff_by_remote){
                sys_auto_sniff_controle(0, NULL);
            }
            btctrler_task_close_bredr();
        }    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        if(en){
            bt_emitter_start_search_device();
        }else{
            bt_emitter_stop_search_device();
        }
        break;
    #endif
    }   
    
    return ret;
}
bool hal_bt_uart_tx(uint8_t id, bt_t bt,uint8_t *buf, uint16_t len)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        return (0 == ble_hid_transfer_channel_send(buf, len));
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        return  multi_client_user_server_write( buf,  len);
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
	       //unsupport
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        //unsupport
        break;
    #endif
    }   
    
    return ret;
}



bool hal_bt_hid_tx(uint8_t id, bt_t bt,uint8_t*buf, uint16_t len)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        ret = (0 == ble_hid_data_send(buf[0], buf+1, len-1));
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        //unsupport
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 
        ret = (0 == edr_hid_data_send(buf[0], buf+1, len-1));	    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        //unsupport
        break;
    #endif
    }   
    
    return ret;
}



#if BT_HID_SUPPORT
uint8_t *ble_report_mapp = NULL;
uint8_t *edr_report_mapp = NULL;
static void user_hid_set_reportmap (bt_t bt)
{
    api_bt_ctb_t* bt_ctbp;
	bt_ctbp = api_bt_get_ctb(bt);

    if(bt_ctbp->types & BIT(DEV_TYPE_HID)){
        uint8_t i;
        uint8_t *hid_mapp, *report_mapp;
        uint16_t len, map_len = 0;

        //get report map len
        for(i=0; i<16; i++){
            if(bt_ctbp->hid_types & BIT(i)){
                len = get_hid_desc_map((trp_t)bt, i ,NULL);;
                map_len += len;
            }
        }

        if(BT_BLE == bt){
            free(ble_report_mapp);                  //防止多次调用内存溢出
            ble_report_mapp = malloc(map_len);
            report_mapp = ble_report_mapp;
        }else{
            free(edr_report_mapp);                  //防止多次调用内存溢出
            edr_report_mapp = malloc(map_len);
            report_mapp = edr_report_mapp;
        }

        if(NULL == report_mapp) return;
        for(i=0; i<16; i++){
            if(bt_ctbp->hid_types & BIT(i)){
                len = get_hid_desc_map((trp_t)bt, i ,&hid_mapp);
                memcpy(report_mapp, hid_mapp, len);
                report_mapp += len;
            }
        }
        
        if(BT_BLE == bt){
            report_mapp = ble_report_mapp;
            #if TCFG_USER_BLE_ENABLE
            le_hogp_set_ReportMap(ble_report_mapp, map_len);
            #endif
        }else{
            report_mapp = edr_report_mapp;
            #if TCFG_USER_EDR_ENABLE
            user_hid_set_ReportMap(edr_report_mapp, map_len);
            #endif
        }
        logd("bt%d report map %d: ",bt,map_len);dumpd(report_mapp, map_len);
    }
}
#endif

bool hal_bt_init(uint8_t id)
{
    bool ret = false;
    char device_name[BT_NAME_LEN_MAX];
    u8 device_name_len = 0;
    u8 tmp_addr[6];

    if(BT_ID0 != id) return false;

    u32 sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    
    memcpy(bas_mac,bt_get_mac_addr(),6); 
    logd("bas_mac:");dumpd(bas_mac,6);

    #if TCFG_USER_EDR_ENABLE
    btstack_edr_start_before_init(&edr_default_config, 0);
    //change init
    user_hid_init(edr_out_callback);
    #endif

    #if TCFG_USER_BLE_ENABLE
    btstack_ble_start_before_init(&ble_default_config, 0);
    #endif
    
    //设置EDR基础蓝牙地址和蓝牙名称,BLE地址和名称是基于EDR地址和名称上修改
	#if TCFG_USER_EDR_ENABLE
    api_bt_get_mac(BT_ID0,BT_EDR, tmp_addr);
    bt_set_mac_addr(tmp_addr);

    device_name_len = api_bt_get_name(BT_ID0,BT_EDR,device_name,sizeof(device_name) );      //设置EDR蓝牙名称
    bt_set_local_name(device_name,device_name_len);
    logi("edr name(%d): %s \n", device_name_len, device_name);dumpd(tmp_addr,6);
    #if EDR_HID_SUPPORT && TCFG_USER_EDR_ENABLE
    user_hid_set_reportmap (BT_EDR);
    #endif
	#endif

    #if TCFG_USER_BLE_ENABLE
    api_bt_get_mac(BT_ID0,BT_BLE, tmp_addr);
    le_controller_set_mac((void *)tmp_addr);
    device_name_len = api_bt_get_name(BT_ID0,BT_BLE,device_name,sizeof(device_name) );       //设置BLE蓝牙名称
    ble_comm_set_config_name(device_name, 0);
    logi("ble name(%d): %s \n", device_name_len, device_name);dumpd(tmp_addr,6);
    #if BLE_HID_SUPPORT && TCFG_USER_BLE_ENABLE
    user_hid_set_reportmap (BT_BLE);
    #endif
    #endif

    btstack_init();
    return ret;
}


    

bool hal_bt_deinit(uint8_t id)
{
    bool ret = false;

    if(BT_ID0 != id) return false;

    #if TCFG_USER_BLE_ENABLE
    btstack_ble_exit(0);
    #endif

    #if TCFG_USER_EDR_ENABLE
    btstack_edr_exit(0);
    #endif

    
    #if BT_HID_SUPPORT
    free(ble_report_mapp);
    free(edr_report_mapp);
    #endif

    return ret;
}
void hal_bt_task(void* pa)
{
    UNUSED_PARAMETER(pa);
}


#endif

