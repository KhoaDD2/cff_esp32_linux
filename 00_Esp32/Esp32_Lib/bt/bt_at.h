#ifndef _BT_AT_H
#define _BT_AT_H
#include <stdint.h>
#include "bt.h"

extern BT_ConnectedCbFn bt_conn_cb;
extern BTCmdCbFn bt_data_cb;
void BT_Echo(uint8_t* data, uint32_t len);

#endif /*_BT_AT_H*/