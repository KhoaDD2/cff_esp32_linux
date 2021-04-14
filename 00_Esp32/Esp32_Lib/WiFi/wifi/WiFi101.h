/*
  WiFi.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WIFI_H
#define WIFI_H

#define WIFI_FIRMWARE_REQUIRED "19.4.4"

#include "mbed_toolchain.h"

//EXTERN_C
	#include "driver/include/m2m_wifi.h"
	#include "socket/include/socket.h"
//EXTERN_C_END

#include "WiFiClient.h"
//#include "WiFiSSLClient.h"
//#include "WiFiServer.h"

typedef enum {
	WL_NO_SHIELD = 255,
	WL_IDLE_STATUS = 0,
	WL_NO_SSID_AVAIL,
	WL_SCAN_COMPLETED,
	WL_CONNECTED,
	WL_CONNECT_FAILED,
	WL_CONNECTION_LOST,
	WL_DISCONNECTED,
	WL_AP_LISTENING,
	WL_AP_CONNECTED,
	WL_AP_FAILED,
	WL_PROVISIONING,
	WL_PROVISIONING_FAILED
} wl_status_t;

/* Encryption modes */
enum wl_enc_type {  /* Values map to 802.11 encryption suites... */
	ENC_TYPE_WEP  = M2M_WIFI_SEC_WEP,
	ENC_TYPE_TKIP = M2M_WIFI_SEC_WPA_PSK,
	ENC_TYPE_CCMP = M2M_WIFI_SEC_802_1X,
	/* ... except these two, 7 and 8 are reserved in 802.11-2007 */
	ENC_TYPE_NONE = M2M_WIFI_SEC_OPEN,
	ENC_TYPE_AUTO = M2M_WIFI_SEC_INVALID
};

typedef enum {
	WL_RESET_MODE = 0,
	WL_STA_MODE,
	WL_PROV_MODE,
	WL_AP_MODE
} wl_mode_t;

typedef enum {
	WL_PING_DEST_UNREACHABLE = -1,
	WL_PING_TIMEOUT = -2,
	WL_PING_UNKNOWN_HOST = -3,
	WL_PING_ERROR = -4
} wl_ping_result_t;

typedef struct
{
    char ssid[M2M_MAX_SSID_LEN];
    char pass[M2M_MAX_PSK_LEN];
    uint8_t sec_type;
} wl_ap_info_t;

typedef uint8_t byte;

// typedef struct _WiFiClass
// {
//     struct _WiFiClass* (*instance)(void);




// 	int (*init)(void);
// 	char* (*firmwareVersion)(void);

// // 	/* Start Wifi connection with WPA/WPA2 encryption.
// // 	 *
// // 	 * param ssid: Pointer to the SSID string.
// // 	 * param key: Key input buffer.
// // 	 */
//  	uint8_t (*begin_default)(void);
// // 	uint8_t (*begin_open)(const char *ssid);
// // 	uint8_t (*begin_wep)(const char *ssid, uint8_t key_idx, const char* key);
//  	uint8_t (*begin_wpa)(const char *ssid, const char *key);
// // //	uint8_t begin(const String &ssid) { return begin(ssid.c_str()); }
// // //	uint8_t begin(const String &ssid, uint8_t key_idx, const String &key) { return begin(ssid.c_str(), key_idx, key.c_str()); }
// // //	uint8_t begin(const String &ssid, const String &key) { return begin(ssid.c_str(), key.c_str()); }

// // 	/* Start Wifi in Access Point, with open security.
// // 	 * Only one client can connect to the AP at a time.
// // 	 *
// // 	 * param ssid: Pointer to the SSID string.
// // 	 * param channel: Wifi channel to use. Valid values are 1-12.
// // 	 */
// // 	uint8_t (*beginAP)(const char *ssid);
// // 	uint8_t (*beginAP)(const char *ssid, uint8_t channel);
// // 	uint8_t (*beginAP)(const char *ssid, uint8_t key_idx, const char* key);
// // 	uint8_t (*beginAP)(const char *ssid, uint8_t key_idx, const char* key, uint8_t channel);

// // 	uint8_t (*beginProvision)();
// 	uint8_t (*beginProvision)(uint8_t channel);

// 	uint32_t (*provisioned)(void);

// 	// void (*config)(IPAddress local_ip);
// 	// void (*config)(IPAddress local_ip, IPAddress dns_server);
// 	// void (*config)(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
// 	void (*config)(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

// 	void (*disconnect)(void);
// 	void (*end)(void);

// 	uint8_t *(*macAddress)(uint8_t *mac);

// 	uint32_t (*localIP)(void);
// 	uint32_t (*subnetMask)(void);
// 	uint32_t (*gatewayIP)(void);
// 	char* (*SSID)(void);
// 	int32_t (*RSSI)(void);
// 	uint8_t (*encryptionType)(void);
// 	uint8_t* (*BSSID)(uint8_t* bssid);
// 	uint8_t* (*APClientMacAddress)(uint8_t* mac);
// 	int8_t (*scanNetworks)(void);
// 	char* (*SSID_pos)(uint8_t pos);
// 	int32_t (*RSSI_pos)(uint8_t pos);
// 	uint8_t (*encryptionType_pos)(uint8_t pos);

// 	uint8_t (*status)(void);

// 	int (*hostByName)(const char* hostname, IPAddress* result);
// //	int hostByName(const String &hostname, IPAddress& result) { return hostByName(hostname.c_str(), result); }

// 	int (*ping_host)(const char* hostname, uint8_t ttl);
// 	int (*ping_ip)(IPAddress host, uint8_t ttl);

// 	uint32_t (*getTime)(void);

// 	void (*refresh)(void);

// 	void (*lowPowerMode)(void);
// 	void (*maxLowPowerMode)(void);
// 	void( *noLowPowerMode)(void);



// 	uint8_t (*startConnect)(const char *ssid, uint8_t u8SecType, const void *pvAuthInfo);
// 	uint8_t (*startAP)(const char *ssid, uint8_t u8SecType, const void *pvAuthInfo, uint8_t channel);
// 	uint8_t* (*remoteMacAddress)(uint8_t* remoteMacAddress);

// 	uint8_t (*startProvision)(const char *ssid, const char *url, uint8_t channel);
// } WiFiClass;

int WIFI__init(void);
char* WIFI__firmwareVersion(void);
uint8_t WIFI__begin(void);
uint8_t WIFI__begin_default(void);
uint8_t WIFI__begin_open(const char *ssid);
uint8_t WIFI_begin_wep(const char *ssid, uint8_t key_idx, const char* key);
uint8_t WIFI__begin_wpa(const char *ssid, const char *key);
uint8_t WIFI__startConnect(const char *ssid, uint8_t u8SecType, const void *pvAuthInfo);
uint8_t WIFI__beginAP(const char *ssid, uint8_t key_idx, const char* key, uint8_t channel);
uint8_t WIFI__startAP(const char *ssid, uint8_t u8SecType, const void *pvAuthInfo, uint8_t channel);
uint8_t WIFI__beginProvision(uint8_t channel);
uint8_t WIFI__startProvision(const char *ssid, const char *url, uint8_t channel);
uint32_t WIFI__provisioned(void);
void WIFI__config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
void WIFI__config_subnet(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
void WIFI__disconnect(void);
void WIFI__end(void);
uint8_t *WIFI__macAddress(uint8_t *mac);
uint32_t WIFI__localIP(void);
uint32_t WIFI__subnetMask(void);
uint32_t WIFI__gatewayIP(void);
char* WIFI__SSID(void);
uint8_t* WIFI__BSSID(uint8_t* bssid);
uint8_t* WIFI__APClientMacAddress(uint8_t* mac);
uint8_t* WIFI__remoteMacAddress(uint8_t* remoteMacAddress);
int32_t WIFI__RSSI(void);
int8_t WIFI__scanNetworks(void);
char* WIFI__SSID_pos(uint8_t pos);
int32_t WIFI__RSSI_pos(uint8_t pos);
uint8_t WIFI__encryptionType(void);
uint8_t WIFI__encryptionType_pos(uint8_t pos);
uint8_t WIFI__status(void);
int WIFI__hostByName(const char* aHostname, IPAddress* aResult);
void WIFI__refresh(void);
void WIFI__lowPowerMode(void);
void WIFI__maxLowPowerMode(void);
void WIFI__noLowPowerMode(void);
int WIFI__ping_host(const char* hostname, uint8_t ttl);
int WIFI__ping_ip(IPAddress host, uint8_t ttl);
uint32_t WIFI__getTime(void);
char* WIFI__PASS(void);
uint8_t WIFI__beginProvision_noAuto(uint8_t channel, bool autoConnect);
#define WIFI__unlock() do {} while(0)
#define WIFI__trylock()	(1)
#endif /* WIFI_H */
