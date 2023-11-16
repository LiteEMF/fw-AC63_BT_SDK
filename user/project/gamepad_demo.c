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

/************************************************************************************************************
**	Description:	
************************************************************************************************************/
#include "hw_config.h"
#if defined GAMEPAD_DEMO && GAMEPAD_DEMO
#include "app/emf.h"
#include "app/app_km.h"
#include "api/usb/device/usbd.h"
#include  "api/bt/api_bt.h"
#if APP_GAMEAPD_ENABLE
#include "app/gamepad/app_gamepad.h"
#endif
#include "api/api_log.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/



/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/
timer_t edr_interval_10us = 1500;

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/


const uint8_t led_channel[] = {2, 3, 4};
bool rgb_driver_show(uint8_t* frame, uint8_t size)
{
	bool ret = false;
    uint8_t brightness;
	uint8_t i;

	for(i=0; i<size; i++){
		#ifdef HW_PWM_MAP
		ret = api_pwm_set_duty(led_channel[i], frame[i]);
		#endif
	}

	return ret;
}
bool rgb_driver_init(void)
{
	return true;
}
bool rgb_driver_deinit(void)
{
	return true;
}

#if API_BT_ENABLE
void api_bt_rx(uint8_t id,bt_t bt, bt_evt_rx_t* pa)
{
    logd("bt%d rx:%d\n",bt,pa->len);    //dumpd(pa->buf,pa->len);

	api_bt_ctb_t* bt_ctbp;

	bt_ctbp = api_bt_get_ctb(bt);
	if(NULL == bt_ctbp) return;
	#if APP_GAMEAPD_ENABLE
	if(BT_HID & pa->bts){
		hid_type_t hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
		trp_handle_t handle = {bt,id,U16(DEV_TYPE_HID,hid_type)};

		app_gamepad_dev_process(&handle, pa->buf, pa->len);
	}
	#endif
}
#endif



void app_key_vendor_scan(uint32_t *pkey)
{
	
}

void app_key_event(void)
{
    //工厂测试
    static bool key_cal_event = false;
    static bool mode_pair_event = false;
    static bool usbd_mode_event = false;
    static bool reset_reson_event = false;

	if(m_key_power_on){
		if((m_app_key.press_long & HW_KEY_POWER) == HW_KEY_POWER){
			if(PM_STA_CHARG_NOT_WORK == m_pm_sta){
				api_reset();
				return;
			}else if(PM_STA_NORMAL == m_pm_sta){
				api_sleep();
				return;
			}
		}
	}
    if(m_systick <= 5000){            
        if((m_app_key.press_long & (HW_KEY_B | HW_KEY_X)) == (HW_KEY_B | HW_KEY_X)){
            api_boot(1);
        }
    }
	
    //摇杆校准
    if(m_app_key.press_long & (HW_KEY_SELECT | HW_KEY_START)){
        if(!key_cal_event){
            key_cal_event = true;
            app_joystack_cal_start();
        }	
    }else{
		key_cal_event = false;
	}
	if(JOYSTICK_CAL_NONE != joystick_cal_sta){
		if(HW_KEY_A & m_app_key.pressed_b){
			app_joystack_cal_end();
		}
	}
        
    //模式切换
}



void hw_user_vender_init(void)
{
    uint8_t id;
	
	#if API_USBD_BIT_ENABLE
	for(id=0; id<USBD_NUM; id++){
		#if USBD_TYPE_SUPPORT & BIT_ENUM(DEV_TYPE_AUTO)
		m_usbd_types[id] = BIT_ENUM(DEV_TYPE_AUTO);
		#else
		m_usbd_types[id] = USBD_TYPE_SUPPORT;
		#endif
        m_usbd_hid_types[id] = USBD_HID_SUPPORT;
	}
	#endif

	#if API_BT_ENABLE
	for(id = 0; id < BT_MAX; id++){
		api_bt_ctb_t* bt_ctbp;
		bt_ctbp = api_bt_get_ctb(id);
		if(NULL != bt_ctbp){
			if(BT_BLE == id){
				#if (BT_SUPPORT & BIT_ENUM(TR_BLE)) && BLE_HID_SUPPORT
				bt_ctbp->types = BLE_TYPE_SUPPORT;
				bt_ctbp->hid_types = BLE_HID_SUPPORT;
				#endif
			}else if(BT_EDR == id){
				#if (BT_SUPPORT & BIT_ENUM(TR_EDR)) && EDR_HID_SUPPORT
				bt_ctbp->types = EDR_TYPE_SUPPORT;
				bt_ctbp->hid_types = EDR_HID_SUPPORT;
				#endif
			}
		}
	}
	#endif


	api_gpio_dir(JOYSTICK_VCC_GPIO,PIN_OUT,PIN_PULLNONE);
    api_gpio_out(JOYSTICK_VCC_GPIO, 1);

	api_gpio_dir(MCU_EN_GPIO,PIN_OUT,PIN_PULLNONE);
    api_gpio_out(MCU_EN_GPIO, 1);

    logd("call hw_user_vender_init ok\n" );

}
void user_vender_init(void)	
{
    uint8_t i;
    logd("call user_vender_init ok\n" );
	

	#if APP_RGB_ENABLE
    for(i=0; i<APP_RGB_NUMS; i++){
        app_rgb_set_blink(i, Color_White, BLINK_SLOW);
    }
	#endif
	
}
void user_vender_deinit(void)			//关机前deinit
{
	logd("user_vender_deinit\n");
}

bool bt_gamepad_key_send(bt_t bt, app_gamepad_key_t *keyp)
{
	api_bt_ctb_t* bt_ctbp = api_bt_get_ctb(bt);
	trp_handle_t bt_handle;
	hid_type_t hid_type;

	if(NULL == bt_ctbp) return false;
	hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
	bt_handle.id = BT_ID0;
	bt_handle.trp = bt;
	bt_handle.index = U16(DEF_DEV_TYPE_HID,hid_type);
	return app_gamepad_key_send(&bt_handle, keyp);
}

bool usb_gamepad_key_send(uint8_t id, app_gamepad_key_t *keyp)
{
	trp_handle_t usb_handle;
	usbd_dev_t* pdev = usbd_get_dev(id);
	hid_type_t hid_type;

	hid_type = app_gamepad_get_hidtype(m_usbd_hid_types[id]);
	usb_handle.id = id;
	usb_handle.trp = TR_USBD;
	usb_handle.index = U16(DEF_DEV_TYPE_HID,hid_type);
	return app_gamepad_key_send(&usb_handle,keyp);
}


void user_vender_handler(void)
{
    static timer_t timer;
	static timer_t report_interval = 1500;

    //use test
	if(m_task_tick10us - timer >= report_interval){
		timer = m_task_tick10us;
		
		//gamepad key
		app_gamepad_key_t key = m_gamepad_key;

		//report
		api_bt_ctb_t* edr_ctbp = api_bt_get_ctb(BT_EDR);
		api_bt_ctb_t* ble_ctbp = api_bt_get_ctb(BT_BLE);
		usbd_dev_t* pusb_dev = usbd_get_dev(0);
		if(NULL != edr_ctbp->sta && BT_STA_CONN == edr_ctbp->sta){

			if(edr_ctbp->vendor_ready){
				report_interval = edr_ctbp->inteval_10us;
				trp_handle_t bt_handle = {BT_EDR, BT_ID0, U16(DEV_TYPE_VENDOR,0)};
				api_transport_tx(&bt_handle,&key,sizeof(key));
			}else{
				report_interval = edr_ctbp->inteval_10us;
				bt_gamepad_key_send(BT_EDR, &key);
			}
		}else if(NULL != ble_ctbp->sta && BT_STA_CONN == ble_ctbp->sta){
			report_interval = ble_ctbp->inteval_10us;
			bt_gamepad_key_send(BT_BLE, &key);
		#if API_USBD_BIT_ENABLE
		}else if(NULL != pusb_dev && pusb_dev->ready){
			report_interval = 800;
			usb_gamepad_key_send(0,&key);
		}
		#endif
    }

}

#endif
