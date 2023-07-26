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
#include  "api/api_tick.h"


#include  "api/api_log.h"
/******************************************************************************************************
** Defined
*******************************************************************************************************/

/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/
uint32_t sys_freq = HAL_SYS_FREQ;

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		  96M: 70ns + (n*50ns) ,delay(1)最小为120ns
** Returns:			  24M: 1.12us + (n*380ns),delay(1)为1.5us
** Description:		
*******************************************************************/
void hal_delay_ns(uint32_t ns)
{
    if(sys_freq == 24000000L){
        if(ns <= 1120) return;
        delay((ns-1120) / 380);                     //24M
    }else{
        delay((ns-70) / 50);                        //96M
    }
}

/*******************************************************************
** Parameters:		  96M: 50ns
** Returns:			  24M: 400ns
** Description:		
*******************************************************************/
void hal_delay_us(uint32_t us)
{
	if(sys_freq == 24000000L){
        delay(us*2);                 //24M
    }else{
        delay(us*(1000/50));		//96M
    }
}
void hal_delay_ms(uint32_t ms)
{
	delay(1000*ms);
}
void hal_tick_init(void)
{	
	sys_freq = hal_get_sysclk(SYSCLK);
    logd_r("sys_clk=%d\n",sys_freq);
}





