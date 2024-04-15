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
#if defined HELLOW_KEYBOARD && HELLOW_KEYBOARD
#include "app/emf.h"
#include "app/app_km.h"
#include "api/usb/host/usbh.h"
#include "api/usb/device/usbd.h"
#include "api/bt/api_bt.h"
#include "app/app_led.h"


#include "emf.h"
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
app_km_t m_uart_km;
uint16_t m_mouse_num = 0;
uint16_t mouse_interval = 800;
int16_t mouse_val = 10;

uint16_t mouse_index = 0xffff;
int8_t mouse_tab[][2]={
{1 ,	0} ,
{2 ,	0} ,
{3 ,	0} ,
{4 ,	0} ,
{5 ,	0} ,
{6 ,	0} ,
{7 ,	0} ,
{8 ,	0} ,
{9 ,	0} ,
{10 ,	0} ,
{11 ,	0} ,
{12 ,	0} ,
{13 ,	0} ,
{14 ,	0} ,
{15 ,	0} ,
{16 ,	0} ,
{17 ,	0} ,
{18 ,	0} ,
{19,	0},
{20,	0},
{1, 	1},
{1, 	10},
{1, 	20},
};





/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/

#if API_BT_ENABLE
void api_bt_rx(uint8_t id,bt_t bt, bt_evt_rx_t* pa)
{
    logd("bt%d rx:%d\n",bt,pa->bts);    //dumpd(pa->buf,pa->len);

	api_bt_ctb_t* bt_ctbp;

	bt_ctbp = api_bt_get_ctb(bt);
	if(NULL == bt_ctbp) return;
	    
	if(BT_UART & pa->bts){
		trp_handle_t uart_handle = {TR_BLE, id, U16(DEV_TYPE_VENDOR,0)};
		app_command_rx(&uart_handle, pa->buf,pa->len);
	}
}
#endif



void app_key_event(void)
{
    if(HW_KEY_HOME & m_app_key.pressed_b){
		m_app_km.active = true;
		m_mouse_num = 50000 / mouse_interval;	
		mouse_index = 0;
		logd("m_mouse_num=%d, mouse_interval=%d, mouse_val=%d",m_mouse_num,mouse_interval,mouse_val);	
		logd("m_mouse_index=%d",countof(mouse_tab));
	}
}



void hw_user_vender_init(void)
{
    uint8_t id;

    logd("call hw_user_vender_init ok\n" );

}
void user_vender_init(void)	
{
    uint8_t i;
    logd("call user_vender_init ok\n" );

	
}
void user_vender_deinit(void)			//关机前deinit
{

}


bool app_command_vendor_decode(trp_handle_t *phandle,uint8_t* buf,uint16_t len)
{
	logd("app_command_vendor_decode"); dumpd(buf, len);
	switch(buf[3]){
		case 0xF1:
		mouse_interval = buf[4] * 100;
		mouse_val = (int8_t)buf[5];
		logd("set mouse_interval=%d, mouse_val=%d",mouse_interval,mouse_val);
		break;
	}
	return true;
}

void user_vender_handler(uint32_t period_10us)
{
	uint8_t i;
    static timer_t timer;

    #ifdef HW_UART_MAP
	trp_handle_t uart_handle = {TR_UART, 0, 0};
    app_fifo_t *fifop = api_uart_get_rx_fifo(0);

	while(app_command_rx_fifo(&uart_handle, fifop)){
	}
    #endif


	if(m_task_tick10us - timer >= mouse_interval){
		timer = m_task_tick10us;

		usbd_dev_t* pdev = usbd_get_dev(0);
		trp_handle_t usb_handle = {TR_USBD, 0, U16(DEF_DEV_TYPE_HID,HID_TYPE_MOUSE)}; 

		if(pdev->ready && m_app_km.active){
			app_mouse_key_send(&usb_handle,&m_app_km.mouse);
			app_km_clean(&m_app_km);

			#if 0
			if(m_mouse_num){
				m_app_km.active = true;
				m_app_km.mouse.x = mouse_val;
				m_mouse_num--;
			}
			#else
			if(mouse_index < countof(mouse_tab)){
				m_app_km.active = true;
				m_app_km.mouse.x = mouse_tab[mouse_index][0];
				m_app_km.mouse.y = mouse_tab[mouse_index][1];
				mouse_index++;
				logd("mouse=%d %d",m_app_km.mouse.x,m_app_km.mouse.y);
			}
			#endif
		}
	}

}

#endif
