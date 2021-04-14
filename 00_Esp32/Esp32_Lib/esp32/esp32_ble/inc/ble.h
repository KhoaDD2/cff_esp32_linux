#pragma once

#ifndef _BLE_H
#define _BLE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "BLE_Type.h"

#define BLE_LIB_VERSION "V01.01.15"
#define BLE_API_VERSION "V01.02.05"
#define NAME_MAX_LEN  17

extern uint8_t default_name[];

typedef enum {
  eIdNone = 0,
  eId5XXX = 1,
  eId3XXX = 2,
  eIdHF = 3,
  eIdSnapOn = 4,
  eIdCraftsman = 5,
  eIdFixFinder = 6,

  eIdMax = 0xffff,
} bleBroadCastID_t;

/*
* @brief Main loop, to run BLE Request processing. Always run this routine when using BLE module.
* @author Dat Le
*/
int BLE_main(void);

/*
* @brief Provide location to get version of FW and BL of the Device
* @note The string info is currently fixed at 16 bytes
* @fn
* @author Dat le
* @date 20160408
* @param [in] fwInfoAddr Pointer to version info of FW
* @param [in] blInfoAddr Pointer to version info of BL
*/
void BLE_RegisterFWBLInfo(uint8_t* fwInfoAddr, uint8_t* blInfoAddr);

/*
* @brief Callback function when BLE connection is established
* @author Dat Le
*/
void BLE_RegisterConnectedCallback(BLE_ConnectedCbFn fn);

/*
* @brief Initialize the BLE module
* @author Dat Le
* @date 20150617
* @note !!! Call this function first !!!
*/
eBLE_STATUS BLE_Startup(void);

/*
* @func BLE_SendMsgBufferV2
* @brief Send message via BLE with buffer provided, with isStatus flag
* @param in isStatus 1: buffer is status message (header 0xDD), 0: data message
* @retval
*     eBLE_SUCESS: Message sent
*     fail otherwise
*/
eBLE_STATUS BLE_SendMsgBufferV2(uint8_t msgId, uint16_t msgLen, uint8_t* buffer, uint8_t isStatus);

/*
* @func BLE_SendMsgBuffer
* @brief Send message via BLE with buffer provided
* @retval
*     eBLE_SUCESS: Message sent
*     fail otherwise
*/
eBLE_STATUS BLE_SendMsgBuffer(uint8_t msgId, uint16_t msgLen, uint8_t* buffer);

/*
* @func BLE_SendTransVCIMsgBuffer
* @brief Send VCI message via BLE with buffer provided
* @retval
*     eBLE_SUCESS: Message sent
*     fail otherwise
*/
eBLE_STATUS BLE_SendTransVCIMsgBuffer(uint8_t msgId, uint16_t msgLen, uint8_t* buffer, uint8_t statusVci);

/*
* @brief Retrieve current command params
*/
uint8_t* BLE_GetCurrentCmd(void);

/*
* @brief Register command processing function. Eachtime a message is received via BLE, callback will be called.
* @param [in] function pointer to command handler
*/
void BLE_RegisterCommandCallBack(BleCmdCbFn fn);

/*
* @brief Change BLE module name
* @param [in] Name
* @reval eBLE_SUCESS / eBLE_FAIL
*/
eBLE_STATUS BLE_ChangeModuleName(uint8_t* newName);

/*
* @brief Get module MAC
* @param [in] buffer to store MAC. 12 char address.
* @note Only call this API after module startup.
* @retval eBLE_SUCCESS / eBLE_FAIL
*/
eBLE_STATUS BLE_GetModuleMac(uint8_t* buf);

/*
* @brief Get library version
* @param [in] buffer to store string version (min 10 bytes)
*/
void BLE_GetLibVersion(uint8_t* buf);

/*
* @brief Exclusive SPI control
* @param [in] disable 1 Disable all others Chip Selects / 0 restore previous state
*/
void BLE_ChipSelectDisableOthers(uint8_t disable);

/*
* @brief Generate a random MAC address
* @pram [in] bdaddr buffer to store the adddress (6 bytes)
* @NOTE This is not an API to get current BT Module MAC
*/
void BLE_GetRandomAddress(uint8_t* bdaddr);

/*
* @brief Set MAC Adddress
* @NOTE Must set before any BLE initialization
*/
void BLE_SetMacAddress(uint8_t* bdaddr);

/*
* @brief Register Setting update handler
* @param [in] function pointer to setting handler
*/
void BLE_RegisterSettingCallBack(BLE_SettingCbFn fn);

// BLE Central API
/*
* @brief Central start discovery device
* @param list Pointer to device list storage
*        maxDevice maxium devices to be stored in list
*/
uint32_t BLE_Central_StartScan(bt_deviceInfo_t* list, uint8_t maxDevice);

/*
* @brief Connect to a printer
* @param device printer bt info
*        printer_type ZKC-5804 or SM-L200
*/
int32_t BLE_Central_ConnectPrinter(bt_deviceInfo_t device, bt_printer_type printer_type);

/*
* @brief Print a message
* @param data Pointer to data
*        data_len Data length in byte
*/
int32_t BLE_Central_Print(uint8_t* data, uint32_t data_len);

/*
* @brief Disconnect printer
*/
int32_t BLE_Central_Disconnect(void);

/*
* @brief Set working mode Central or Peripheral
*/
int32_t BLE_SetWorkingMode(ble_mode_t mode);

/*
* @brief Set TX Power level (0 ~ 7), -15 -> 8 dBm
*/
eBLE_STATUS BLE_SetTxPowerLevel(uint8_t newLevel);

/*
* @brief Reset scan device list
*/
void BLE_ResetScanDeviceList(uint32_t maxDevice);


/*
* @brief Set customer broadcast ID
* @note Call this API before BLE_Startup.
* @retval eBLE_SUCCESS / eBLE_FAIL
*/
eBLE_STATUS BLE_SetCustomBroadcastID(bleBroadCastID_t newId);

/*
* @brief Update BLE Adv params and start broadcast again
* @author Dat Le
*/
eBLE_STATUS BLE_UpdateBleAdvert(void);

/*
* @author Dat Le
* @date 2019-08-10
* @brief Check if BLE RX buffer empty or not
* @param none
* @retval 1 empty, 0 not empty
*/
uint8_t BLE_IsRxBufferEmpty(void);

/*
* @date Mar. 22, 2020
* @brief Enable BLE SPP mode (transparent)
* This works like real UART
* @author Dat le
* @param
* @retval
*/
eBLE_STATUS BLE_EnableSPPMode(void);

/*
* @date Mar 25, 2020
* @brief Update BLE Connection
* @author Dat le
* @param
* @retval
*/
eBLE_STATUS BLE_UpdateConnection(void);

/*
* @date May 06, 2020
* @brief Check is BLE is init.
* @author Dat le
* @param
* @retval
*/
uint32_t BLE_IsInit(void);

/*
* @date Oct 24, 2020
* @brief Send BLE message for FFE0 service
* @note For ELM327 support
* @author Dat le
* @param
* @retval
*/
eBLE_STATUS BLE_SendDataELM327(uint8_t* data, uint32_t length);

/*
* @date Oct 24, 2020
* @brief Data callback for ELM327 commands
* @note For ELM327 support
* @author Dat le
* @param [in] cb : Callback handler for raw data
* @retval
*/
void BLE_RegisterElm327DataCallBack(ble_raw_cb_t cb);

#endif /*_BLE_H*/
