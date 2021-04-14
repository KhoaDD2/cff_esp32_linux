#pragma once
#include "BLE_Type.h"

/*
* @brief BLE callback function to receive data.
* @fn
* @author Dat Le
* @date 20160406
* @param [in] data Pointer to the received bytes
* @param [in] size Number of bytes received
*/
void BLE_UART_RxCallback(const uint8_t* data, const uint32_t size);

/*
* @brief Register Tx function for BLE
* @fn
* @author Dat Le
* @date 20160406
* @param [in] Pointer to Tx function
*/
void BLE_UART_RegisterTxFunction(BLE_UART_SendByteFn fn);

/*
* @brief Register Host baud modify function
* @fn
* @author Dat Le
* @date 20160517
* @param [in] Pointer to fn
*/
void BLE_UART_RegisterHostBaudChangeFn(BLE_UART_HostBaudFn fn);