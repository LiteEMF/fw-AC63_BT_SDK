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
    logd("bt%d rx:%d\n",bt,pa->len);    dumpd(pa->buf,pa->len);

	api_bt_ctb_t* bt_ctbp;

	bt_ctbp = api_bt_get_ctb(bt);
	if(NULL == bt_ctbp) return;


	if(BT_HID & pa->bts){
	#if EDR_HID_SUPPORT & HID_GAMEPAD_MASK
		hid_type_t hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
		trp_handle_t handle = {bt,id,U16(DEV_TYPE_HID,hid_type)};

		app_gamepad_dev_process(&handle, pa->buf, pa->len);
	#else
		trp_handle_t handle = {bt,id,U16(DEV_TYPE_HID, HID_TYPE_VENDOR)};
		app_command_rx(&handle,pa->buf+1, pa->len-1);	//丢弃hid_report_type_t
	#endif

	}else if(BT_UART == pa->bts){					//uart
		trp_handle_t handle = {bt,id,U16(DEV_TYPE_VENDOR, 0)};
		app_command_rx(&handle,pa->buf, pa->len);
	}
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
    if(m_systick <= 5000 || 1){
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
    uint8_t id,bt;
	
	#if API_USBD_BIT_ENABLE
	for(id=0; id<USBD_NUM; id++){
		api_transport_set_type(id,TR_USBD, m_dev_mode,m_hid_mode);
	}
	#endif

	#if API_BT_ENABLE
	for(bt = 0; bt < BT_MAX; bt++){
		if(BT_BLE == bt){
			#if (BT_SUPPORT & BIT_ENUM(TR_BLE)) && BLE_HID_SUPPORT
			api_transport_set_type(0, bt, m_dev_mode, m_hid_mode);
			#endif
		}else if(BT_EDR == bt){
			#if (BT_SUPPORT & BIT_ENUM(TR_EDR)) && EDR_HID_SUPPORT
			api_transport_set_type(0, bt, m_dev_mode, m_hid_mode);
			#endif
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
	
	// app_rumble_set_duty(0, 250, 500);
	// app_rumble_set_duty(1, 250, 500);
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


/*******************************************************************
** Parameters:		
** Returns:	
** Description:	简化发送流程, 	
*******************************************************************/
bool trp_gamepad_key_send(uint8_t id, trp_t trp, app_gamepad_key_t *keyp)
{
	trp_handle_t handle;
	hid_type_t hid_type = HID_TYPE_NONE;

	if(api_trp_is_bt(trp)){
		#if API_BT_ENABLE
		api_bt_ctb_t* bt_ctbp = api_bt_get_ctb(trp);
		if(NULL == bt_ctbp) return false;
		hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
		#endif
	}else if(api_trp_is_usb(trp)){
		#if API_USBD_BIT_ENABLE
		hid_type = app_gamepad_get_hidtype(m_usbd_hid_types[id]);
		#endif
	}

	handle.id = id;
	handle.trp = trp;
	handle.index = U16(DEF_DEV_TYPE_HID,hid_type);
	return app_gamepad_key_send(&handle, keyp);
}


void led_handler(void)
{
	#if APP_RGB_ENABLE
	static timer_t timer;

	api_bt_ctb_t* edr_ctbp = api_bt_get_ctb(BT_EDR);
	api_bt_ctb_t* ble_ctbp = api_bt_get_ctb(BT_BLE);

	if(m_systick - timer >= 50){
		timer = m_systick;
		if(PM_STA_CHARG_NOT_WORK == m_pm_sta){
			app_rgb_set_static(0, 0x00, RGB_FOREVER);
		}else if(PM_STA_NORMAL == m_pm_sta){
			if(JOYSTICK_CAL_NONE != joystick_cal_sta){
				if(JOYSTICK_CAL_SUCCEED == joystick_cal_sta){
					app_rgb_set_static(0, Color_White, RGB_FOREVER);
				}else{
					app_rgb_set_blink(0, Color_White, BLINK_SLOW);
				}
			}else if(NULL != edr_ctbp){
				if((BT_STA_CONN == edr_ctbp->sta)){
					app_rgb_set_static(0, Color_White, RGB_FOREVER);
				}else if(BT_STA_ADV == edr_ctbp->sta){
					app_rgb_set_blink(0, Color_White, BLINK_FAST);
				}else{
					app_rgb_set_blink(0, Color_White, BLINK_SLOW);
				}
			}else if(NULL != ble_ctbp){
				if((BT_STA_CONN == ble_ctbp->sta)){
					app_rgb_set_static(0, Color_Green, RGB_FOREVER);
				}else if(BT_STA_ADV == ble_ctbp->sta){
					app_rgb_set_blink(0, Color_Green, BLINK_FAST);
				}else{
					app_rgb_set_blink(0, Color_Green, BLINK_SLOW);
				}
			}
		}
	}
	#endif
}

void user_vender_handler(uint32_t period_10us)
{
    static timer_t timer;
	static timer_t report_interval = 1500;

	led_handler();
    //use test
	if(m_task_tick10us - timer >= report_interval){
		timer = m_task_tick10us;
		
		//gamepad key
		app_gamepad_key_t key = m_gamepad_key;

		//改建,连点,宏定义,摇杆
		joystick_cfg_t tick_cfg = {0, 0, 10, 3};
		joystick_cfg_t trigger_cfg = {0, 0, 5, 5};
		app_stick_deadband(&tick_cfg, &key.stick_l);
		app_stick_deadband(&tick_cfg, &key.stick_r);
		app_trigger_deadband(&trigger_cfg, &key.l2);
		app_trigger_deadband(&trigger_cfg, &key.r2);
		// logd("k:%x %d %d %d %d\n",key.key,  key.stick_l.x,key.stick_l.y,key.stick_r.x,key.stick_r.y  );
		//report
		api_bt_ctb_t* edr_ctbp = api_bt_get_ctb(BT_EDR);
		api_bt_ctb_t* ble_ctbp = api_bt_get_ctb(BT_BLE);
		usbd_dev_t* pusb_dev = usbd_get_dev(0);

		if(NULL != edr_ctbp->sta && BT_STA_CONN == edr_ctbp->sta){
			if(edr_ctbp->vendor_ready){
				report_interval = edr_ctbp->inteval_10us;
				trp_handle_t bt_handle = {BT_EDR, BT_ID0, U16(DEV_TYPE_VENDOR,0)};
				app_gamepad_key_swapl(&key);
				api_command_tx(&bt_handle,CMD_GAMEPAD_KEY,&key,sizeof(key));
			}else if(edr_ctbp->hid_ready){

				#if SNIFF_MODE_RESET_ANCHOR
				report_interval = edr_ctbp->inteval_10us;
				#else
				report_interval = 500;
				#endif
				#if EDR_HID_SUPPORT & HID_GAMEPAD_MASK
				trp_gamepad_key_send(BT_ID0,BT_EDR, &key);
				#else
				trp_handle_t bt_handle = {BT_EDR, BT_ID0, U16(DEV_TYPE_HID,HID_TYPE_VENDOR)};
				app_gamepad_key_swapl(&key);
				api_command_tx(&bt_handle,CMD_GAMEPAD_KEY,&key,sizeof(key));
				#endif
			}
		}else if((NULL != ble_ctbp) && ble_ctbp->hid_ready){
			report_interval = ble_ctbp->inteval_10us;
			trp_gamepad_key_send(BT_ID0,BT_BLE, &key);
		#if API_USBD_BIT_ENABLE
		}else if((NULL != pusb_dev) && (pusb_dev->ready)){
			report_interval = 800;
			trp_gamepad_key_send(0,TR_USBD, &key);
		#endif
		}
    }

}

#endif
