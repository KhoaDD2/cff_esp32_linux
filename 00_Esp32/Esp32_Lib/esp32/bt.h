#ifndef _BT_H
#define _BT_H
#include <stdint.h>

typedef void (*BT_ConnectedCbFn) (uint8_t);
typedef void (*BTCmdCbFn) (void*, uint32_t);

int BT_SPP_RegisterConnectedCallback(BT_ConnectedCbFn fn);
int BT_SPP_RegsiterDataCallback(BTCmdCbFn fn);

/*
* @brief Bluetooth Clasisc SPP startup
* @author Dat Le
*/
uint32_t BT_Startup(void);

/*
* @brief Send SPP data
* @author Dat Le
*/
int BT_SPP_SendMessage(uint8_t* data, uint32_t length);


#endif /*_BT_H*/