#ifndef __SPP_TRANS_H__
#define __SPP_TRANS_H__

#include "typedef.h"
#include "bt_common.h"

void transport_spp_init(void);
int transport_spp_send_data_check(u16 len);
int transport_spp_send_data(u8 *data, u16 len);
void transport_spp_disconnect(void);

#endif//__RCSP_SPP_USER_H__
