#ifndef _WIFI_H
#define _WIFI_H

#include "./inc/NM.h"
#include "./inc/RESTClient.h"
#include "./inc/HTTPClient.h"

typedef void (*AsyncCbFn) (void* data);

/*
* @author Dat Le
* @brief Async call for NM ScanNetworks
*/
int8_t NM__ScanNetworks_Async(wl_ap_info_t* ap_list, uint32_t max_nbr_ap);

/*
* @author Dat Le
* @brief Async call for NM Connect
*/
bool NM__connect_Async(char* ssid, char* pass);

/*
* @author Dat Le
* @brief Async command complete callback
*/
void Wifi_RegisterAsyncCompleteCallback(AsyncCbFn cbfn);

/*
* @author Dat Le
* @brief Get MAC adress
* @param [out] 17 bytes MAC xx:xx:xx:xx:xx:xx
*/
void Wifi_GetMacAddress(uint8_t* macAddr);

/*
* @date Feb 21, 2020
* @brief Enable/Stop SoftAP mode
* @author Dat le
* @param
* @retval
*/
void Wifi_SoftApControl(uint8_t enable);

/*
* @date Mar 03, 2020
* @brief SoftAP tcp command process callback
* @author Dat Le
* @param
* @retval
*/
void Wifi_SoftApRegisterCallback(void * fn);

/*
* @date Mar 03, 2020
* @brief SoftAP tcp send with BT Commandset message format
* @author Dat le
* @param
* @retval
*/
eBLE_STATUS Wifi_SoftApTcpBleSend(uint8_t msgId, uint16_t msgLen, uint8_t* buffer);

#endif /*_WIFI_ASYNC_H*/
