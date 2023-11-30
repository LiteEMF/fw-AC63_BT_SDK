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


#ifndef _hw_config_h
#define _hw_config_h
#include "utils/emf_defined.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************************************
** Defined
*******************************************************************************************************/
#define HEAP_ID						0					/* used heap_n file*/
#define LOG_ENABLE                  1
#define CRC8_TABLE_EANBLE 			0					/*杰里默认有crc8校验*/
#define API_WDT_ENABLE				1

#define TCFG_ADKEY_ENABLE					0
#define TCFG_MATRIX_KEY_ENABLE              0
#define TCFG_CODE_SWITCH_ENABLE             0 //code switch使能



/******************************************************************************************************
** Project Defined
*******************************************************************************************************/

#define PROJECT_KM                  0           //keyboard and mouse project
#define PROJECT_GAMEPAD             1           //keyboard and mouse

#if PROJECT_KM

	#define HELLOW_KEYBOARD						1
	#if HELLOW_KEYBOARD

		#define APP_KEY_ENABLE						1
		#define APP_CMD_ENABLE						1
		#define APP_KM_ENABLE						1
		#define TCFG_LOWPOWER_LOWPOWER_SEL			0		/*关闭修改防止定时器不准确*/

		#define API_OTG_BIT_ENABLE					(BIT(0) | BIT(1))

		#define API_USBD_BIT_ENABLE					(BIT(0) | BIT(1))
		#define USBD_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_HID))
		#define API_AUDIO_ENABLE					1
		#define USBD_HID_SUPPORT					BIT_ENUM(HID_TYPE_MOUSE)

		//bt 		
		#define BT0_SUPPORT							(BIT_ENUM(TR_BLE))
		#define BLE_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_VENDOR))

		#define DEV_TRPS_DEFAULT					(BIT(TR_BLE)|BIT(TR_USBD))
		#define SW_VERSION                  		0x0100
        #define DEFAULT_NAME			    		"hellow_keyboard"
        #define DEFAULT_MODEL						"HELKB"

	#else

	#endif

#elif PROJECT_GAMEPAD
	#define GAMEPAD1					0				//测试工程
	#define GAMEPAD_DEMO				1				//实际一个gamepad demo例程
	#define KMFC						0
	#define KMFCS						0


	#if GAMEPAD1	
		#define APP_KEY_ENABLE			1
		#define TCFG_LOWPOWER_LOWPOWER_SEL			0		/*关闭修改防止定时器不准确*/
		#define API_WDT_TIME	8000	
		
		// #define API_OTG_BIT_ENABLE			(BIT(0) | BIT(1))
		#define API_USBD_BIT_ENABLE			(BIT(0) | BIT(1))
		// #define USBD_TYPE_SUPPORT 			(BIT_ENUM(DEV_TYPE_MSD))
		// #define USBD_RAM_DISK_ENABLE		1
		#define USBD_TYPE_SUPPORT		(BIT_ENUM(DEV_TYPE_HID))
		#define API_AUDIO_ENABLE			1
		// #define USBD_HID_SUPPORT			(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE) | BIT_ENUM(HID_TYPE_CONSUMER))
		#define USBD_HID_SUPPORT			(BIT_ENUM(HID_TYPE_PS4))
		// #define USBD_HID_SUPPORT			(BIT_ENUM(HID_TYPE_X360) | BIT_ENUM(HID_TYPE_KB)  | BIT_ENUM(HID_TYPE_VENDOR))

		//#define API_USBH_BIT_ENABLE			(BIT(0) | BIT(1))
		#define USBH_TYPE_SUPPORT			(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_HUB))
		#define USBH_HID_SUPPORT			(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE) | BIT_ENUM(HID_TYPE_CONSUMER))

		// #define PS_7105_ENCRYPT_ENABLED
		#define PS_P2_ENCRYPT_ENABLED

		//bt 
		#define BT0_SUPPORT							(BIT_ENUM(TR_BLE))
		// #define BT0_SUPPORT						(BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLEC))
		#define EDR_TYPE_SUPPORT					BIT_ENUM(DEV_TYPE_HID)
		#define BLE_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_VENDOR))
		#define BLE_HID_SUPPORT						(BIT_ENUM(HID_TYPE_XBOX))
		// #define EDR_HID_SUPPORT					BIT_ENUM(HID_TYPE_XBOX)
		// #define BLE_HID_SUPPORT					(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE))
		// #define EDR_HID_SUPPORT					(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE))
		#define BTC_SEARCH_NAME						DEFAULT_NAME
		// #define BTC_SEARCH_RSSI					-50
		// #define BTC_SEARCH_MAC					"JL6321"

		#define EDR_ICON							BD_CLASS_GAMEPAD

		#define BTC_SEARCH_UUID16			    	0xae30
		#define BTC_SEARCH_WRITE_CHARA_UUID16		0xae01
		#define BTC_SEARCH_NOTIFY_CHARA_UUID16		0xae01

		#define DEV_TRPS_DEFAULT					BT0_SUPPORT
		#define SW_VERSION                     		0x0100
        #define DEFAULT_NAME			       		"gamepad"
        #define DEFAULT_MODEL						"GP_dev"
	#elif GAMEPAD_DEMO	
		#define API_STORAGE_ENABLE			1
		#define API_PM_ENABLE				1
		#define APP_KEY_ENABLE				1
		//#define KEY_DUMP_ENABLE 			1
		#define APP_CMD_ENABLE				1
		#define TCFG_LOWPOWER_LOWPOWER_SEL	0		/*关闭修改防止定时器不准确*/
		#define APP_RGB_ENABLE				1
		#define APP_RGB_NUMS				1
		#define APP_RUMBLE_ENABLE			1
		#define APP_GAMEAPD_ENABLE			1

		#define APP_JOYSTICK_ENABLE			1
		#define STICK_CAL_SIDE_DEADZONE		6
		#define TRIGGER_CAL_DEADZONE		5      	//中心死区百分百
		#define APP_STICK_ACTIVE 			{{true, false},{true, false}}
		#define APP_TRIGGER_ACTIVE 			{false, false}
		#define TRIGGER_LIMIT_MIN_R 		350

		#define STICK_CAL_DEFAULT_R			240
		#define TRIGGER_CAL_DEFAULT_R   	400

		#define API_USBD_BIT_ENABLE			BIT(0)
		#define USBD_TYPE_SUPPORT			(BIT_ENUM(DEV_TYPE_HID))
		#define USBD_HID_SUPPORT			(BIT_ENUM(HID_TYPE_X360) | BIT_ENUM(HID_TYPE_VENDOR))

		//Gsensor
		#define IMU_ICM42688_ID				1

		//bt 
		#define BT0_SUPPORT					(BIT_ENUM(TR_EDR))
		// #define EDR_TYPE_SUPPORT			(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_VENDOR))
		// #define EDR_HID_SUPPORT				BIT_ENUM(HID_TYPE_XBOX)
		#define EDR_TYPE_SUPPORT			(BIT_ENUM(DEV_TYPE_HID))
		#define EDR_HID_SUPPORT				BIT_ENUM(HID_TYPE_VENDOR)
		// #define BLE_TYPE_SUPPORT			(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_VENDOR))
		// #define BLE_HID_SUPPORT			BIT_ENUM(HID_TYPE_XBOX)
		#define EDR_ICON					BD_CLASS_GAMEPAD


		#define DISCONNECTED_SLEEP_TIME    	(30*1000*60UL)
		#define CONNECTED_SLEEP_TIME    	(15*1000*60UL)

		#define DEV_TRPS_DEFAULT					BT0_SUPPORT
		#define SW_VERSION                     		0x0200
        #define DEFAULT_NAME			       		"gamepad_demo"
        #define DEFAULT_MODEL						"GP_demo"
	
	#elif KMFC
		#define APP_KEY_ENABLE						1
		#define APP_CMD_ENABLE						1
		#define APP_KM_ENABLE						1
		#define APP_RUMBLE_ENABLE					1
		#define TCFG_LOWPOWER_LOWPOWER_SEL			0		/*关闭修改防止定时器不准确*/

		#define API_OTG_BIT_ENABLE					(BIT(1))

		#define API_USBD_BIT_ENABLE					(BIT(0))
		#define USBD_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_HID))
		#define API_AUDIO_ENABLE					1
		#define USBD_HID_SUPPORT					(HID_GAMEPAD_MASK | BIT_ENUM(HID_TYPE_KB))

		#define API_USBH_BIT_ENABLE					(BIT(1))
		#define USBH_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_HUB))
		#define USBH_HID_SUPPORT					(HID_GAMEPAD_MASK | BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE) | BIT_ENUM(HID_TYPE_CONSUMER))

		#define USBD_SOCKET_ENABLE					1
		#define USBH_SOCKET_ENABLE					1

		//bt 		
		#define BT0_SUPPORT							(BIT_ENUM(TR_BLE))
		#define BLE_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_VENDOR))

		#define DEV_TRPS_DEFAULT					(BIT(TR_BLE)|BIT(TR_USBD))
		#define SW_VERSION                  		0x0100
        #define DEFAULT_NAME			    		"km_for_consoles"
        #define DEFAULT_MODEL						"KMFC"
	#elif KMFCS
		#define APP_KEY_ENABLE						1
		#define APP_LED_ENABLE						1
		#define APP_CMD_ENABLE						1
		#define APP_KM_ENABLE						1
		#define TCFG_LOWPOWER_LOWPOWER_SEL			0		/*关闭修改防止定时器不准确*/

		#define API_OTG_BIT_ENABLE					(BIT(0) | BIT(1))

		#define API_USBD_BIT_ENABLE					(BIT(0) | BIT(1))
		#define USBD_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_HID))
		#define API_AUDIO_ENABLE					1
		#define USBD_HID_SUPPORT					(HID_GAMEPAD_MASK | BIT_ENUM(HID_TYPE_KB))

		#define API_USBH_BIT_ENABLE					(BIT(0) | BIT(1))
		#define USBH_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_HUB))
		#define USBH_HID_SUPPORT					(HID_GAMEPAD_MASK | BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE) | BIT_ENUM(HID_TYPE_CONSUMER))

		//bt 		
		#define BT0_SUPPORT							(BIT_ENUM(TR_BLE))
		#define BLE_TYPE_SUPPORT					(BIT_ENUM(DEV_TYPE_VENDOR))

		#define DEV_TRPS_DEFAULT					(BIT(TR_BLE)|BIT(TR_USBD))
		#define SW_VERSION                  		0x0100
        #define DEFAULT_NAME			    		"km_for_consoless"
        #define DEFAULT_MODEL						"KMFCS"
	#endif
#endif

#include "hw_board.h"
#include "emf_config.h"

//sdk config
// #define TCFG_UART0_TX_PORT  				IO_PORTA_00		在hw_board中设置
// #define TCFG_DCDC_PORT_SEL				NO_CONFIG_PORT  在hw_board中设置
#ifndef TCFG_CHARGE_ENABLE
#define TCFG_CHARGE_ENABLE					DISABLE_THIS_MOUDLE
#endif
#ifndef TCFG_CHARGE_FULL_MA
#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_10
#endif
#ifndef TCFG_CHARGE_MA
#define TCFG_CHARGE_MA						CHARGE_mA_60
#endif
#ifndef TCFG_LOWPOWER_POWER_SEL
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15	/*电源模式设置，可选DCDC和LDO*/
#endif
#ifndef TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL			SLEEP_EN	/*SNIFF状态下芯片是否进入powerdown*/
#endif
#ifndef TCFG_AUDIO_ENABLE
#define TCFG_AUDIO_ENABLE					API_AUDIO_ENABLE
#endif
#ifndef TCFG_AUDIO_DAC_CONNECT_MODE
#define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_MONO_LR_DIFF
#endif
#ifndef CONFIG_FLASH_SIZE
#define CONFIG_FLASH_SIZE                   FLASH_SIZE_2M    /*配置FLASH大小*/
#endif



#if API_OTG_BIT_ENABLE
	#define TCFG_OTG_USB_DEV_EN     API_OTG_BIT_ENABLE
	#if API_USBD_BIT_ENABLE && API_USBH_BIT_ENABLE
		#define TCFG_OTG_MODE       (OTG_HOST_MODE | OTG_SLAVE_MODE | OTG_CHARGE_MODE)
	#elif API_USBD_BIT_ENABLE
		#define TCFG_OTG_MODE       (OTG_SLAVE_MODE | OTG_CHARGE_MODE)
	#elif API_USBH_BIT_ENABLE
		#define TCFG_OTG_MODE       (OTG_HOST_MODE | OTG_CHARGE_MODE )
	#else
		#define TCFG_OTG_MODE       0
	#endif

#else
	#define TCFG_OTG_USB_DEV_EN     0
#endif

#if USBD_TYPE_SUPPORT & BIT_ENUM(DEV_TYPE_AUDIO)
#define TCFG_USB_SLAVE_AUDIO_ENABLE         1
#endif


#if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF) | BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
	#define TCFG_USER_BLE_ENABLE                1
#else
	#define TCFG_USER_BLE_ENABLE                0
#endif
#if BT0_SUPPORT & (BIT_ENUM(TR_EDR) | BIT_ENUM(TR_EDRC))
	#define TCFG_USER_EDR_ENABLE                1
	#define USER_SUPPORT_PROFILE_SPP    BOOL_SET(EDR_TYPE_SUPPORT & BIT_ENUM(DEV_TYPE_VENDOR))
	#define USER_SUPPORT_PROFILE_HID    BOOL_SET(EDR_TYPE_SUPPORT & BIT_ENUM(DEV_TYPE_HID))
#else
	#define TCFG_USER_EDR_ENABLE                0
#endif


#if (BT0_SUPPORT & BIT_ENUM(TR_BLE)) && BLE_HID_SUPPORT
#define CONFIG_BT_SM_SUPPORT_ENABLE        	1
#else
#define CONFIG_BT_SM_SUPPORT_ENABLE        	0
#endif
#if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
#define CONFIG_BT_GATT_CLIENT_NUM          	1 /*配置主机client个数*/
#else
#define CONFIG_BT_GATT_CLIENT_NUM        	0
#endif
#if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
#define CONFIG_BT_GATT_SERVER_NUM          	1 /*配置从机server个数*/
#else
#define CONFIG_BT_GATT_SERVER_NUM        	0
#endif



#ifdef __cplusplus
}
#endif
#endif





