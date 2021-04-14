#ifndef _BLE_LEGACY_H
#define _BLE_LEGACY_H
#include <stdint.h>

#warning "PRO14/34 legacy compatible support"
/*
* @NOTE Use BLE_SetMsgBuffer
* @func BLE_GetMsgBuffer
* @brief Get message buffer for response message
* @retval Pointer to buffer, NULL if no buffer available.
*/
uint8_t *gf_USB_VCI_GetTempBuff(void);
#define BLE_GetMsgBuffer() gf_USB_VCI_GetTempBuff()

/*
* @NOTE Use BLE_SetMsgBuffer
* @brief Send message via BLE
* @param [in] msgId Message ID - defined in Bluetooth Commandset
* @param [in] msgLen Length of the payload data in buffer provided bye BLE_GetMsgBuffer
* @retval
*     eBLE_SUCCESS: Message Sent
*     fail otherwise
*/
#define BLE_SendMsg(msgid,len) BLE_SendMsgBuffer(msgid,len,BLE_GetMsgBuffer())

#endif /*_BLE_LEGACY_H*/