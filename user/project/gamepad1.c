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
#if defined GAMEPAD1 && GAMEPAD1
#include "app/emf.h"
#include "app/app_km.h"
#include "api/usb/host/usbh.h"
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
**	public Parameters
*******************************************************************************************************/


/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/


const uint8_t led_channel[] = {0, 1, 2, 3, 4, 5, 10, 11, 12};
bool rgb_driver_show(uint8_t* frame, uint8_t size)
{
	bool ret = false;
    uint8_t brightness;
	uint8_t i;
    
    // logd("show:");dumpd(frame,size);
    // emf_mem_stats();
	for(i=0; i<size; i++){
        brightness = remap(frame[i], 0, 255, 0, 63);
        #ifdef HW_SPI_HOST_MAP
        ret = api_spi_host_write(0,led_channel[i], &brightness, 1);
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

#if API_USBH_BIT_ENABLE
void usbh_class_itf_alt_select(uint8_t id,usbh_class_t* pclass)
{
	usbh_dev_t* pdev = get_usbh_dev(id);
	usbh_class_t *pos;

	list_for_each_entry_type(pos,&pdev->class_list, usbh_class_t, list){
		if(pos->itf.if_num == pclass->itf.if_num){
			free_usbh_class(pclass);			//默认使用alt 0
			// free_usbh_class(pos);
			continue;				
		}
	}
}
#endif



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

    logd("call hw_user_vender_init ok\n" );

}
void user_vender_init(void)	
{
    uint8_t i;
    logd("call user_vender_init ok\n" );


    // api_gpio_dir(PB_00, PIN_OUT,PIN_PULLNONE);
    // app_rumble_set_duty(RUMBLE_L, 0X80, 1000);
    // app_rumble_set_duty(RUMBLE_R, 0X80, 1000);
	
	#if APP_RGB_ENABLE
    for(i=0; i<APP_RGB_NUMS; i++){
        app_rgb_set_blink(i, Color_White, BLINK_SLOW);
    }
	#endif
	
}
void user_vender_deinit(void)			//关机前deinit
{
}

void user_vender_handler(uint32_t period_10us)
{
    static timer_t timer;

    // // adc test 
    // static timer_t adc_times = 0;
    // if(m_systick - adc_times >= 10){
    //     adc_times = m_systick;
    //     logd("adc:%4d,%4d,%4d,%4d,%4d,%4d\n",
	// 	api_adc_value(0),api_adc_value(1),api_adc_value(2),api_adc_value(3),api_adc_value(4),api_adc_value(5));
    // }


    //use test
	#define  TEST_USB_ID	0
	if(m_systick - timer >= 8){
		usbd_dev_t *pdev;
		api_bt_ctb_t* bt_ctbp;
		trp_handle_t bt_handle = {TR_BLE, BT_ID0, 0};;
		trp_handle_t usb_handle = {TR_USBD, TEST_USB_ID, 0}; 
		hid_type_t hid_type;

		timer = m_systick;
		
		//kb
		#if HIDD_SUPPORT & BIT_ENUM(HID_TYPE_KB)
		if(1){
			static kb_t kb={KB_REPORT_ID,0};

			if(kb.key[0]){
				kb.key[0] = 0;
				kb.key[1] = 0;
			}else{
				kb.key[0] = KB_A;
				kb.key[1] = KB_CAP_LOCK;
			}

			#if USBD_HID_SUPPORT & BIT_ENUM(HID_TYPE_KB)
			pdev = usbd_get_dev(TEST_USB_ID);
			usb_handle.index = U16(DEV_TYPE_HID,DEF_HID_TYPE_KB);
			if(pdev->ready){
				api_transport_tx(&usb_handle,&kb,sizeof(kb));
			}
			#endif

			#if BLE_HID_SUPPORT & BIT_ENUM(HID_TYPE_KB)
			bt_ctbp = api_bt_get_ctb(BT_BLE);
			bt_handle.trp = TR_BLE;
			bt_handle.index = U16(DEV_TYPE_HID,DEF_HID_TYPE_KB);
			if(bt_ctbp->hid_ready){
				api_transport_tx(&bt_handle,&kb,sizeof(kb));
			}
			#endif

			#if EDR_HID_SUPPORT & BIT_ENUM(HID_TYPE_KB)
			bt_ctbp = api_bt_get_ctb(BT_EDR);
			bt_handle.trp = TR_EDR;
			bt_handle.index = U16(DEV_TYPE_HID,DEF_HID_TYPE_KB);
			if(bt_ctbp->hid_ready){
				api_transport_tx(&bt_handle,&kb,sizeof(kb));
			}
			#endif
		}
		#endif
	
		//mouse
		#if HIDD_SUPPORT & BIT_ENUM(HID_TYPE_MOUSE)
		if(1){
			static mouse_t mouse={MOUSE_REPORT_ID,0};
			if(mouse.x >= 0){
				mouse.x = -10;
			}else{
				mouse.x = 10;
			}

			#if USBD_HID_SUPPORT & BIT_ENUM(HID_TYPE_MOUSE)
			pdev = usbd_get_dev(TEST_USB_ID);
			usb_handle.index = U16(DEV_TYPE_HID,HID_TYPE_MOUSE);
			if(pdev->ready){
				api_transport_tx(&usb_handle,&mouse,sizeof(mouse));
			}
			#endif

			#if BLE_HID_SUPPORT & BIT_ENUM(HID_TYPE_MOUSE)
			bt_ctbp = api_bt_get_ctb(BT_BLE);
			bt_handle.trp = TR_BLE;
			bt_handle.index = U16(DEV_TYPE_HID,HID_TYPE_MOUSE);
			if(bt_ctbp->hid_ready){
				api_transport_tx(&bt_handle,&mouse,sizeof(mouse));
			}
			#endif

			#if EDR_HID_SUPPORT & BIT_ENUM(HID_TYPE_MOUSE)
			bt_ctbp = api_bt_get_ctb(BT_EDR);
			bt_handle.trp = TR_EDR;
			bt_handle.index = U16(DEV_TYPE_HID,HID_TYPE_MOUSE);
			if(bt_ctbp->hid_ready){
				api_transport_tx(&bt_handle,&mouse,sizeof(mouse));
			}
			#endif
		}
		#endif
		
		//gamepad
		#if HIDD_SUPPORT & HID_GAMEPAD_MASK
		if(1){
			static app_gamepad_key_t key={0};
			key.key ^= HW_KEY_A;
			key.stick_l.x += 10000;
			
			#if USBD_HID_SUPPORT
			pdev = usbd_get_dev(TEST_USB_ID);
			hid_type = app_gamepad_get_hidtype(m_usbd_hid_types[TEST_USB_ID]);
			usb_handle.index = U16(DEV_TYPE_HID,hid_type);
			if(pdev->ready){
				app_gamepad_key_send(&usb_handle,&key);
			}
			#endif

			#if BLE_HID_SUPPORT
			bt_ctbp = api_bt_get_ctb(BT_BLE);
			hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
			bt_handle.trp = TR_BLE;
			bt_handle.index = U16(DEV_TYPE_HID,hid_type);
			if(bt_ctbp->hid_ready){
				app_gamepad_key_send(&bt_handle,&key);
			}
			#endif

			#if EDR_HID_SUPPORT
			bt_ctbp = api_bt_get_ctb(BT_EDR);
			hid_type = app_gamepad_get_hidtype(bt_ctbp->hid_types);
			bt_handle.trp = TR_EDR;
			bt_handle.index = U16(DEV_TYPE_HID,hid_type);
			if(bt_ctbp->hid_ready){
				app_gamepad_key_send(&bt_handle,&key);
			}
			#endif
			
		}
		#endif
    }

}

#endif
