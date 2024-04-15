/*********************************************************************************************
    *   Filename        : app_keyboard.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2019-07-05 10:09

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "system/includes.h"
#include "server/server_core.h"
#include "app_action.h"
#include "app_config.h"

#if(CONFIG_APP_LITEEMF)
#include "system/app_core.h"
#include "os/os_api.h"
#include "rcsp_bluetooth.h"
#include "update_loader_download.h"
#include "app/emf.h"
#if APP_GAMEAPD_ENABLE
#include "app/gamepad/app_gamepad.h"
#endif

#if defined PS_P2_ENCRYPT_ENABLED || defined PS_7105_ENCRYPT_ENABLED
#include  "app/gamepad/ps_crypt.h"
#endif
#include "api/api_log.h"


/******************************************************************************************************
** Defined
*******************************************************************************************************/

#define     TASK_NAME               "emf_task"
#define     HEARTBEAT_EVENT         0x01000000  
#define     UART_RX_EVENT           0x02000000       //低24bit 作为参数
#define     BT_REC_DATA_EVENT       0x03000000       //bt_t len

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/
static volatile u8 is_app_liteemf_active = 0;//1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗

uint8_t heartbeat_msg_cnt = 1;
/*****************************************************************************************************
**  Function
******************************************************************************************************/
AT(.volatile_ram_code)
void api_timer_hook(uint8_t id)
{
    if(0 == id){
        m_systick++;

        if(0 == heartbeat_msg_cnt){
            int err = os_taskq_post_msg(TASK_NAME, 1, HEARTBEAT_EVENT);
            if(err){
                logd("HB post err!!!\n");
            }else{
                heartbeat_msg_cnt = 1;
            }
        }
        
    }
}





extern u32 get_jl_rcsp_update_status();
static u32 check_ota_mode()
{
    if (UPDATE_MODULE_IS_SUPPORT(UPDATE_APP_EN)) {
        #if RCSP_UPDATE_EN
        if (get_jl_rcsp_update_status()) {
            r_printf("OTA ing");
            usb_sie_close_all();//关闭usb
            return 1;
        }
        #endif
    }
    return 0;
}





error_t os_post_buf_msg(uint32_t evt,uint8_t* buffer,uint16_t length)	//如果使用os,并且需要发送消息通知任务开启时使用
{
	uint8_t* p;
	uint8_t err;
    uint32_t event;
	if(0 == length) return ERROR_LENGTH;

	p = emf_malloc(length);
	if(NULL != p){
		memcpy(p,buffer, length);
	}
    event = evt;
	err = os_taskq_post_msg(TASK_NAME, 3, event, p, length);
	if(err){
		emf_free(p);
		logd("msg event %x err=%d\n",event,err);
		return ERROR_NO_MEM;
	}

	return ERROR_SUCCESS;
}

#if API_BT_ENABLE
error_t os_bt_rx(uint8_t id, bt_t bt, bt_evt_rx_t* pa)	
{
    return os_post_buf_msg(BT_REC_DATA_EVENT | U32(0,id,bt,pa->bts), pa->buf, pa->len);
}
#endif


/*******************************************************************
** Parameters:		
** Returns:	
** Description:	task	
*******************************************************************/
static void emf_task_handle(void *arg)
{
    uint8_t *p;
    int ret = 0;
    int msg[16];

    while (1) {
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        if (msg[0] != Q_MSG) {
            continue;
        }

        switch (msg[1] & 0XFF000000) {
            case HEARTBEAT_EVENT:
                heartbeat_msg_cnt = 0;
                m_task_tick10us +=100;      //同步任务tick时钟
                emf_handler(100);

                #if API_USBD_BIT_ENABLE     //无线升级时候关闭usb
                if (check_ota_mode()) {
                    usb_stop(0);
                }
                #endif
                break;
            case UART_RX_EVENT:
                break;
            case BT_REC_DATA_EVENT:{
                uint8_t id; bt_t bt; bt_evt_rx_t pa;
                id = (msg[1] >> 16) & 0XFF;
                bt = (msg[1] >> 8) & 0XFF;
                pa.bts = (msg[1] >> 0) & 0XFF;
                pa.buf = msg[2];
                pa.len = msg[3];
                api_bt_rx(id, bt, &pa);
                emf_free(pa.buf);               //must free
                break;
            }
            default:
                break;
        }
    }
}


#if defined PS_P2_ENCRYPT_ENABLED || defined PS_7105_ENCRYPT_ENABLED
static void ps_task_handle(void *arg)
{
    uint8_t *p;
    int ret = 0;
    int msg[16];

    while (1) {
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        if (msg[0] != Q_MSG) {
            continue;
        }

        ps_encrypt_task(NULL);
		if((PS_ANSWER != ps_encrypt.step) && (PS_IDLE != ps_encrypt.step)){
			os_time_dly(1);
			if(os_taskq_post_msg("ps_task", 1, 0)){
				logd("p2 post err!\n");
			}
		}
     
    }
}
bool os_ps_task_en(bool en)		//如果使用os,用于开始和停止任务
{
    if(en){
        if(os_taskq_post_msg("ps_task", 1, 0)){
            logd("p2 post err!\n");
        }
    }
	return true;
}
#endif


/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
extern void bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);
static void liteemf_app_start()
{
    logi("=======================================");
    logi("-------------HID DEMO-----------------");
    logi("=======================================");
    logi("app_file: %s", __FILE__);

    clk_set("sys", 96 * 1000000L);		//fix clk
    // clk_set("sys", BT_NORMAL_HZ);

    emf_api_init();
    emf_init();

    logd("start end\n");
    os_task_create(emf_task_handle,NULL,2,2048,512,"emf_task");
    heartbeat_msg_cnt = 0;

    #if defined PS_P2_ENCRYPT_ENABLED
	os_task_create(ps_task_handle,NULL,1,1024,64,"ps_task");
	#elif defined PS_7105_ENCRYPT_ENABLED
	os_task_create(ps_task_handle,NULL,1,256,64,"ps_task");
	#endif

 
}

/*******************************************************************
** Parameters:		
** Returns:	
** Description:	app  状态处理	
*******************************************************************/
static int liteemf_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_LITEEMF_MAIN:
            liteemf_app_start();
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        logi("APP_STA_DESTROY\n");
        break;
    }

    return 0;
}
/*******************************************************************
** Parameters:		
** Returns:	
** Description:	app 线程事件处理
*******************************************************************/
static int liteemf_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        #if API_BT_ENABLE
        sys_bt_event_handler(event);
        #endif
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, NULL);    //set_soft_poweroff_call 可以传入关机函数使用系统电池检测
        }
        #if TCFG_CHARGE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
        }
        #endif
        return 0;

    default:
        return 0;
    }

    return 0;
}
/*******************************************************************
** Parameters:		
** Returns:	
** Description:	注册控制是否进入sleep
*******************************************************************/
//system check go sleep is ok
static u8 liteemf_app_idle_query(void)
{
    return !is_app_liteemf_active;
}
REGISTER_LP_TARGET(app_liteemf_lp_target) = {
    .name = "app_liteemf_deal",
    .is_idle = liteemf_app_idle_query,
};


static const struct application_operation app_liteemf_ops = {
    .state_machine  = liteemf_state_machine,
    .event_handler 	= liteemf_event_handler,
};

/*
 * 注册模式
 */
REGISTER_APPLICATION(app_liteemf) = {
    .name 	= "app_liteemf",
    .action	= ACTION_LITEEMF_MAIN,
    .ops 	= &app_liteemf_ops,
    .state  = APP_STA_DESTROY,
};


#endif

