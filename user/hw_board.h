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


#ifndef _hw_board_h
#define _hw_board_h
#include "utils/emf_defined.h"

#ifdef __cplusplus
extern "C" {
#endif



/******************************************************************************************************
** Defined
*******************************************************************************************************/
#if PROJECT_KM

	#if HELLOW_KEYBOARD	
	// timer
		#define API_TIMER_BIT_ENABLE 	BIT(2)
		#define HW_TIMER_MAP {\
			{2, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)},	}
			
		// uart
		#define TCFG_UART0_TX_PORT  		PA_05
		#define HW_UART_MAP {\
			{PA_03, PA_04, 0, 0X100, 0, VAL2FLD(UART_BAUD,1000000)}}

			
		#define KEY_HOME_GPIO				PA_01
		#define HW_LED_MAP 					{PB_03, PB_01, PB_00}
		#define HW_LED_ACTIVE_MAP 			{true,true,true}
		
	#endif

#elif PROJECT_GAMEPAD
	#if GAMEPAD1
		// timer
		#define API_TIMER_BIT_ENABLE 	BIT(2)
		#define HW_TIMER_MAP {\
			{2, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)},	}
			
		// uart
		#define TCFG_UART0_TX_PORT  				PA_05
		//#define HW_UART_MAP {\
			{PA_00, PIN_NULL, 0, 0, 0, VAL2FLD(UART_BAUD,1000000)}}

		//adc
		//#define HW_ADC_MAP {	\
			{PA_00,0,VAL2FLD(ADC_CH,0)},								\
			{PA_01,0,VAL2FLD(ADC_CH,1) | VAL2FLD(ADC_PULL,1)}			\
			}

		// iic
		#define  IIC_SOFT_ENABLE  		1
		#define HW_IIC_MAP {	\
			{PC_04,PC_05,PIN_NULL,0,VAL2FLD(IIC_BADU,400000)},	\
			}

		// spi
		// #define HW_SPI_HOST_MAP {\
			{PB_07,PB_05,PB_06,PB_04,SPI1,VAL2FLD(SPI_BADU,1000)},	\
			}

		// pwm			
		// #define HW_PWM_MAP {	\
			{PA_01,   pwm_timer0,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch0|PWM_CH_H)|VAL2FLD(PWM_ACTIVE,1)},\
			{PB_00,   pwm_timer1,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1),VAL2FLD(PWM_ACTIVE,1)},\
			{PB_02,   pwm_timer1,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1|PWM_CH_H)},\
			{PB_06,   pwm_timer2,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch2),VAL2FLD(PWM_ACTIVE,1)},\
			}
	#elif GAMEPAD_DEMO
		// timer
		#define API_TIMER_BIT_ENABLE 	BIT(2)
		#define HW_TIMER_MAP {\
			{2, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)},	}
			
		// uart
		#define TCFG_UART0_TX_PORT  				PA_09
		//#define HW_UART_MAP {\
			{PA_00, PIN_NULL, 0, 0, 0, VAL2FLD(UART_BAUD,1000000)}}

		//adc
        #define ADC_BATTERY_ID	0
        #define ADC_LX_ID		1
        #define ADC_LY_ID		2
        #define ADC_RX_ID		3
        #define ADC_RY_ID		4
        #define ADC_R2_ID       5
        #define ADC_L2_ID       6
        #define HW_ADC_MAP      {\
            {PIN_NULL, 0UL,VAL2FLD(ADC_CH,AD_CH_VBAT)},\
            {PB_08, 0UL,VAL2FLD(ADC_CH,AD_CH_PB8)},\
            {PA_01, 0UL,VAL2FLD(ADC_CH,AD_CH_PA1)},\
            {PC_05, 0UL,VAL2FLD(ADC_CH,AD_CH_PC5)},\
            {PC_04, 0UL,VAL2FLD(ADC_CH,AD_CH_PC4)},\
            {PB_03, 0UL,VAL2FLD(ADC_CH,AD_CH_PB3)},\ 
            {PB_04, 0UL,VAL2FLD(ADC_CH,AD_CH_PB4)},\
        } 

		// iic
		// #define  IIC_SOFT_ENABLE  		1
		// #define HW_IIC_MAP {	\
			{PC_04,PC_05,PIN_NULL,0,VAL2FLD(IIC_BADU,400000)},	\
			}

		// spi
		// #define HW_SPI_HOST_MAP {\
			{PB_07,PB_05,PB_06,PB_04,SPI1,VAL2FLD(SPI_BADU,1000)},	\
			}

		// pwm			
        //motor：0,1    RGB:2,3,4
        #define HW_PWM_MAP {\
            {PB_10, pwm_timer3, VAL2FLD(PWM_CH,pwm_ch3)},\
            {PB_09, pwm_timer3, VAL2FLD(PWM_CH,pwm_ch3 | PWM_CH_H)},\
            {PA_10, pwm_timer4, VAL2FLD(PWM_CH,pwm_ch4)| VAL2FLD(PWM_ACTIVE,1)},\
            {PA_00, pwm_timer0, VAL2FLD(PWM_CH,pwm_ch0 | PWM_CH_H) | VAL2FLD(PWM_ACTIVE,1)},\
            {PB_00, pwm_timer1, VAL2FLD(PWM_CH,pwm_ch1 | PWM_CH_H) | VAL2FLD(PWM_ACTIVE,1)},\
		}

		#define JOYSTICK_VCC_GPIO			PC_03
		#define MCU_EN_GPIO                 IO_PORT_PR_01

        #define KEY_POWER_GPIO              PB_01
		#define KEY_HOME_GPIO               PB_01

        #define KEY_CHARGER_GPIO			IO_PORT_PR_00
        #define KEY_USB_DET_GPIO            PB_06

		#define KEY_M8_GPIO                 PB_02
        #define KEY_L1_GPIO                 PB_07
        #define KEY_R1_GPIO                 PB_05

        #define MATRIX_KEY_OUT_GPIO         {PC_00,PC_01,PC_02}
        #define MATRIX_KEY_IN_GPIO          {PA_03,PA_04,PA_06,PA_07,PA_08}
		#define MATRIX_KEY           {\
            HW_KEY_DOWN,	HW_KEY_RIGHT,	HW_KEY_UP,	HW_KEY_LEFT,	HW_KEY_R3,\
            HW_KEY_SELECT,	HW_KEY_CAPTURE,	HW_KEY_START,0,				0,\
            HW_KEY_Y,		HW_KEY_X,       HW_KEY_A,	HW_KEY_B,		HW_KEY_L3\
        }
		
	#elif KMFC
		// timer
		#define API_TIMER_BIT_ENABLE 	BIT(2)
		#define HW_TIMER_MAP {\
			{2, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)},	}
			
		// uart
		#define TCFG_UART0_TX_PORT  		PA_05
		#define HW_UART_MAP {\
			{PA_03, PA_04, 0, 0X100, 0, VAL2FLD(UART_BAUD,1000000)}}

        #define KEY_HOME_GPIO				PA_01
        // mcu 电源
        #define USB_OUT_EN                  PB_04

	#elif KMFCS
		// timer
		#define API_TIMER_BIT_ENABLE 	BIT(2)
		#define HW_TIMER_MAP {\
			{2, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)},	}
			
		// uart
		#define TCFG_UART0_TX_PORT  		PA_05
		#define HW_UART_MAP {\
			{PA_03, PA_04, 0, 0X100, 0, VAL2FLD(UART_BAUD,1000000)}}

			
		#define KEY_HOME_GPIO				PA_01
		#define HW_LED_MAP 					{PB_03, PB_01, PB_00}
		#define HW_LED_ACTIVE_MAP 			{true,true,true}
	#endif
#endif




#ifdef __cplusplus
}
#endif
#endif





