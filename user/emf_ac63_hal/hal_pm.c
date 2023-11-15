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
#include "api/api_pm.h"
#include "asm/power_interface.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/

extern uint32_t power_reset_src;
pm_reson_t hal_get_reset_reson(void)
{
	#if API_STORAGE_ENABLE
	if(SOFT_RESET_MASK == m_storage.reset_reson){
		m_storage.reset_reson = 0;
		return PM_RESON_SOFT;
	}
	#endif

	if(power_reset_src  & BIT(0)){
		return PM_RESON_POR;
	}else{
		return PM_RESON_SYS;
	}
}

extern void go_mask_usb_updata(void);
extern void chip_reboot_entry_uboot_uart_upgrade_mode();
void hal_boot(uint8_t index)
{
	if(2 == index){
		chip_reboot_entry_uboot_uart_upgrade_mode();
	}else{
		go_mask_usb_updata();
	}
}
void hal_reset(void)
{
	cpu_reset();
}
void hal_sleep(void)
{
	power_set_soft_poweroff();
}










