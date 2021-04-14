#ifndef __ESP32_H
#define __ESP32_H

#include "esp32_ble/inc/ble.h"
#include "inc/NM.h"

#define ESP32_LIB_VERSION             "V01.03.29"
#define ESP32_WIFI_API_VERSION        "V01.01.00"
#define ESP32_BLE_API_VERSION         BLE_API_VERSION

/*
* @author Dat Le
* @brief ESP32 AT Main loop
*/
void ESP32_Main(void);

/*
* @author Dat Le
* @brief ESP32 Init
*/
void ESP32_Init(void);

/*
* @author Dat Le
* @brief ESP32 get version
*/
uint8_t* ESP32_GetModuleFwVersion(void);

/*
* @author Dat Le
* @brief Select boot mode
*/
void ESP32_Bootmode(uint8_t mode);

/*
* @author Dat Le
* @author Module goes to sleep
*/
void ESP32_Sleep(uint32_t sleepTime);

/*
* @author Dat Le
* @date 2019-08-26
* @brief Change ESP32 baudrate
* @param newbaud newbaudrate
* @retval true false
*/
uint8_t ESP32_ChangeBaudRate(uint32_t newbaud);

/*
* @date Apr. 9, 2020
* @brief Status Sync. Enable internal BLE notify flag, cipmux flag.
* @author Dat le
* @param
* @retval
*/
void ESP32_Sync(void);

#endif /*__ESP32_H*/
