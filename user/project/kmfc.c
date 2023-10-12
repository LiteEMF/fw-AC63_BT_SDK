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
#if (defined KMFC && KMFC) || (defined KMFCS && KMFCS)
#include "app/emf.h"
#include "app/app_km.h"
#include "api/usb/host/usbh.h"
#include "api/usb/device/usbd.h"
#include "api/bt/api_bt.h"
#include "app/app_led.h"
#if APP_GAMEAPD_ENABLE
#include "app/gamepad/app_gamepad.h"
#endif

#include "emf.h"
#include "api/api_log.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/
uint8_t m_led_period[3];

/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/



#if API_BT_ENABLE
void api_bt_rx(uint8_t id,bt_t bt, bt_evt_rx_t* pa)
{
    logd("bt%d rx:%d\n",bt,pa->len);    //dumpd(pa->buf,pa->len);

	api_bt_ctb_t* bt_ctbp;

	bt_ctbp = api_bt_get_ctb(bt);
	if(NULL == bt_ctbp) return;

	#if (BT_HID_SUPPORT & HID_GAMEPAD_MASK)
	if(BT_HID & pa->bts){
		hid_type_t hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
		trp_handle_t handle = {bt,id,U16(DEV_TYPE_HID,hid_type)};

		app_gamepad_dev_process(&handle, pa->buf, pa->len);
	}
	#endif

	if(BT_UART & pa->bts){

	}
}
#endif

void app_key_vendor_scan(uint32_t *pkey)
{
	
}

#if KMFC
void app_key_event(void)
{
    if(HW_KEY_POWER & m_app_key.pressed_b){
		trp_handle_t handle = {TR_UART, 0, 0};

		if(LED_ON == m_led_period[0]){
			m_led_period[0] = LED_SLOW;
			m_led_period[1] = LED_SLOW;
			m_led_period[2] = LED_SLOW;
		}else{
			m_led_period[0] = LED_ON;
			m_led_period[1] = LED_ON;
			m_led_period[2] = LED_ON;
		}
		api_command_tx(&handle, CMD_LED_PERIOD, m_led_period, sizeof(m_led_period) );
	}
}
#endif

void hw_user_vender_init(void)
{
    uint8_t id;
	
	#ifdef USB_OUT_EN
	api_gpio_out(USB_OUT_EN,1);
	api_gpio_dir(USB_OUT_EN, PIN_OUT, PIN_PULLNONE);
	#endif

	#if API_USBD_BIT_ENABLE
	for(id=0; id<USBD_NUM; id++){
		#if USBD_TYPE_SUPPORT & BIT_ENUM(DEV_TYPE_AUTO)
		m_usbd_types[id] = BIT_ENUM(DEV_TYPE_AUTO);
		#else
		m_usbd_types[id] = BIT_ENUM(DEV_TYPE_HID);
		#endif
        m_usbd_hid_types[id] = BIT_ENUM(HID_TYPE_SWITCH);
	}
	#endif

	memset(m_led_period,0,sizeof(m_led_period));
	
    logd("call hw_user_vender_init ok\n" );

}
void user_vender_init(void)	
{
    uint8_t i;
    logd("call user_vender_init ok\n" );

	
}
void user_vender_deinit(void)			//关机前deinit
{
	#ifdef USB_OUT_EN
	api_gpio_out(USB_OUT_EN,0);
	#endif
}


bool app_command_vendor_decode(trp_handle_t *phandle,uint8_t* buf,uint16_t len)
{
	logd("uart cmd %d:",len); dumpd(buf, len);
	switch(buf[3]){
	case CMD_LED_PERIOD:
		#if KMFC
		if(memcmp(m_led_period, &buf[4], sizeof(m_led_period))){
			api_command_tx(phandle, CMD_LED_PERIOD, m_led_period, sizeof(m_led_period) );
			logd("send led cmd\n");
		}
		
		#else

		if(memcmp(m_led_period, &buf[4], m_led_num)){
			memcpy(m_led_period, &buf[4], m_led_num);
			for(uint8_t i=0; i<m_led_num; i++){
				app_set_led(i,m_led_period[i],0);
			}
			api_command_tx(phandle, CMD_LED_PERIOD, m_led_period, m_led_num );
			logd("app_set_led\n");
		}

		#endif

		break;
	}
	return false;
}

void user_vender_handler(void)
{
	uint8_t i;
    static timer_t timer;

    #ifdef HW_UART_MAP
	trp_handle_t handle = {TR_UART, 0, 0};
    app_fifo_t *fifop = api_uart_get_rx_fifo(0);

	app_command_rx_fifo(&handle, fifop);
    #endif
}

#endif
