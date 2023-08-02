// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _BLE_MUTIL_H
#define _BLE_MUTIL_H

#include <stdint.h>
#include "app_config.h"
#include "le_client_demo.h"

void multi_server_init(void);
void multi_server_exit(void);
void multi_client_init(void);
void multi_client_exit(void);
int multi_client_clear_pair(void);
int multi_server_clear_pair(void);
bool multi_client_set_match_param(uint8_t index, cli_creat_mode_e conn_mode, uint8_t* buf,uint8_t len);
bool multi_client_user_server_write(uint8_t *packet, uint16_t size);
#endif
