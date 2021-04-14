/*
  WiFi.cpp - Library for Arduino Wifi shield.
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

//#ifdef ARDUINO_ARCH_AVR
//#include <avr/version.h>
//#if (__AVR_LIBC_MAJOR__ < 2)
//#define WIFI_101_NO_TIME_H
//#endif
//#endif


#include <string.h>
#include <stdlib.h>
#ifndef WIFI_101_NO_TIME_H
#include <time.h>
#endif

//#if !defined(_TIME_H_) && !defined(TIME_H)
//// another library overrided the time.h header
//#define WIFI_101_NO_TIME_H
//#endif
//
//#define WIFI_101_NO_TIME_H
#include "WiFi101.h"

EXTERN_C
  #include "bsp/include/nm_bsp.h"
  //#include "bsp/include/nm_bsp_arduino.h"
  #include "socket/include/socket_buffer.h"
  #include "socket/include/m2m_socket_host_if.h"
  #include "driver/source/nmasic.h"
  #include "driver/include/m2m_periph.h"
EXTERN_C_END



static uint32_t _localip;
static uint32_t _submask;
static uint32_t _gateway;
static int _dhcp;
static uint32_t _resolve;
static byte *_remoteMacAddress;
static wl_mode_t _mode;
static wl_status_t _status;
static char _scan_ssid[M2M_MAX_SSID_LEN];
static uint8_t _scan_auth;
//static char _ssid[M2M_MAX_SSID_LEN];
static wl_ap_info_t _ap_info;
//static WiFiClient* _client[TCP_SOCK_MAX];
static int _init;
static char _version[9];

static void wifi_cb(uint8_t u8MsgType, void *pvMsg);
static void resolve_cb(uint8_t * hostName, uint32_t hostIp);
static void ping_cb(uint32 u32IPAddr, uint32 u32RTT, uint8 u8ErrorCode);


static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
	switch (u8MsgType) {
		case M2M_WIFI_RESP_DEFAULT_CONNECT:
		{
			tstrM2MDefaultConnResp *pstrDefaultConnResp = (tstrM2MDefaultConnResp *)pvMsg;
			if (pstrDefaultConnResp->s8ErrorCode) {
				_status = WL_DISCONNECTED;
			}
		}
		break;

		case M2M_WIFI_RESP_CON_STATE_CHANGED:
		{
			tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
			if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
				tr_info("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED");
				if (_mode == WL_STA_MODE && !_dhcp) {
					_status = WL_CONNECTED;

					// WiFi led ON.
					//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 0);
				} else if (_mode == WL_AP_MODE) {
					_status = WL_AP_CONNECTED;
				}
			} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
				tr_warn("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED");
				if (_mode == WL_STA_MODE) {
					_status = WL_DISCONNECTED;
					if (_dhcp) {
						_localip = 0;
						_submask = 0;
						_gateway = 0;
					}
					// Close sockets to clean state
					// Clients will need to reconnect once the physical link will be re-established
#if 0
					for (int i=0; i < TCP_SOCK_MAX; i++) {
						if (_client[i])
							_client[i]->stop();
					}
#else
                    /* TODO: multi connection*/
                    WiFiClient__stop();
#endif
				} else if (_mode == WL_AP_MODE) {
					_status = WL_AP_LISTENING;
				}
				// WiFi led OFF (rev A then rev B).
				//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 1);
				//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1);
			}
		}
		break;

		case M2M_WIFI_REQ_DHCP_CONF:
		{
			if (_mode == WL_STA_MODE) {
				tstrM2MIPConfig *pstrIPCfg = (tstrM2MIPConfig *)pvMsg;
				_localip = pstrIPCfg->u32StaticIP;
				_submask = pstrIPCfg->u32SubnetMask;
				_gateway = pstrIPCfg->u32Gateway;

				_status = WL_CONNECTED;

				// WiFi led ON (rev A then rev B).
				//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 0);
				//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);
			}
			uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
			tr_warn("wifi_cb: dhcp ip got: %d.%d.%d.%d",
			        pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);

			/*SERIAL_PORT_MONITOR.print("wifi_cb: M2M_WIFI_REQ_DHCP_CONF: IP is ");
			SERIAL_PORT_MONITOR.print(pu8IPAddress[0], 10);
			SERIAL_PORT_MONITOR.print(".");
			SERIAL_PORT_MONITOR.print(pu8IPAddress[1], 10);
			SERIAL_PORT_MONITOR.print(".");
			SERIAL_PORT_MONITOR.print(pu8IPAddress[2], 10);
			SERIAL_PORT_MONITOR.print(".");
			SERIAL_PORT_MONITOR.print(pu8IPAddress[3], 10);
			SERIAL_PORT_MONITOR.println("");*/
		}
		break;

		case M2M_WIFI_RESP_CURRENT_RSSI:
		{
			_resolve = *((int8_t *)pvMsg);
		}
		break;

		case M2M_WIFI_RESP_PROVISION_INFO:
		{
			tstrM2MProvisionInfo *pstrProvInfo = (tstrM2MProvisionInfo *)pvMsg;
			tr_info("wifi_cb: M2M_WIFI_RESP_PROVISION_INFO");

			if (pstrProvInfo->u8Status == M2M_SUCCESS) {
				wl_ap_info_t* p_ap = &(_ap_info);
				memset(p_ap, 0, sizeof(wl_ap_info_t));
				memcpy(p_ap->ssid, (char *)pstrProvInfo->au8SSID, strlen((char *)pstrProvInfo->au8SSID));
				strcpy(p_ap->pass, (char*)pstrProvInfo->au8Password);
				p_ap->sec_type = pstrProvInfo->u8SecType;
				_mode = WL_STA_MODE;
				_localip = 0;
				_submask = 0;
				_gateway = 0;
				tr_info("Try to connect to sssid:%s, pass:%s", p_ap->ssid, p_ap->pass);
				m2m_wifi_connect((char *)pstrProvInfo->au8SSID, strlen((char *)pstrProvInfo->au8SSID),
						pstrProvInfo->u8SecType, pstrProvInfo->au8Password, M2M_WIFI_CH_ALL);
			} else {
				_status = WL_PROVISIONING_FAILED;
				tr_warn("wifi_cb: Provision failed.");
				WIFI__beginProvision(7);
			}
		}
		break;

		case M2M_WIFI_RESP_SCAN_DONE:
		{
			tstrM2mScanDone *pstrInfo = (tstrM2mScanDone *)pvMsg;
			if (pstrInfo->u8NumofCh >= 1) {
				_status = WL_SCAN_COMPLETED;
			}
		}
		break;

		case M2M_WIFI_RESP_SCAN_RESULT:
		{
			tstrM2mWifiscanResult *pstrScanResult = (tstrM2mWifiscanResult *)pvMsg;
			uint16_t scan_ssid_len = strlen((const char *)pstrScanResult->au8SSID);
			memset(_scan_ssid, 0, M2M_MAX_SSID_LEN);
			if (scan_ssid_len) {
				memcpy(_scan_ssid, (const char *)pstrScanResult->au8SSID, scan_ssid_len);
			}
			_resolve = pstrScanResult->s8rssi;
			_scan_auth = pstrScanResult->u8AuthType;
			_status = WL_SCAN_COMPLETED;
		}
		break;

		case M2M_WIFI_RESP_CONN_INFO:
		{
			tstrM2MConnInfo	*pstrConnInfo = (tstrM2MConnInfo*)pvMsg;

			if (_remoteMacAddress) {
				// reverse copy the remote MAC
				for(int i = 0; i < 6; i++) {
					_remoteMacAddress[i] = pstrConnInfo->au8MACAddress[5-i];
				}
				_remoteMacAddress = 0;
			}

			strcpy((char *)_ap_info.ssid, pstrConnInfo->acSSID);
		}
		break;

		case M2M_WIFI_RESP_GET_SYS_TIME:
		{
			if (_resolve != 0) {
				memcpy((tstrSystemTime *)_resolve, pvMsg, sizeof(tstrSystemTime));

				_resolve = 0;
			}
		}
		break;

		default:
		break;
	}
}

static void resolve_cb(uint8_t * hostName, uint32_t hostIp)
{
	_resolve = hostIp;
}

static void ping_cb(uint32 u32IPAddr, uint32 u32RTT, uint8 u8ErrorCode)
{
	if (PING_ERR_SUCCESS == u8ErrorCode) {
		// Ensure this ICMP reply comes from requested IP address
		if (_resolve == u32IPAddr) {
			_resolve = (uint32_t)u32RTT;
		} else {
			// Another network device replied to the our ICMP request
			_resolve = (uint32_t)WL_PING_DEST_UNREACHABLE;
		}
	} else if (PING_ERR_DEST_UNREACH == u8ErrorCode) {
		_resolve = (uint32_t)WL_PING_DEST_UNREACHABLE;
	} else if (PING_ERR_TIMEOUT == u8ErrorCode) {
		_resolve = (uint32_t)WL_PING_TIMEOUT;
	} else {
		_resolve = (uint32_t)WL_PING_ERROR;
	}
}

int WIFI__init(void)
{
	_mode = WL_RESET_MODE;
	_status = WL_IDLE_STATUS;
	_init = 0;

	tstrWifiInitParam param;
	int8_t ret;

	// Initialize the WiFi BSP:
	nm_bsp_init();

	// Initialize WiFi module and register status callback:
	param.pfAppWifiCb = wifi_cb;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) {
		// Error led ON (rev A then rev B).
		// m2m_periph_gpio_set_val(M2M_PERIPH_GPIO18, 0);
		// m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO6, 1);
		return ret;
	}

	// Initialize socket API and register socket callback:
	socketInit();
	socketBufferInit();
	registerSocketCallback(socketBufferCb, resolve_cb);
	_init = 1;
	_status = WL_IDLE_STATUS;
	_localip = 0;
	_submask = 0;
	_gateway = 0;
	_dhcp = 1;
	_resolve = 0;
	_remoteMacAddress = 0;
	//memset(_client, 0, sizeof(WiFiClient *) * TCP_SOCK_MAX);

	return ret;
}

EXTERN_C
	sint8 nm_get_firmware_info(tstrM2mRev* M2mRev);
EXTERN_C_END

char* WIFI__firmwareVersion(void)
{
	tstrM2mRev rev;

	if (!_init) {
		WIFI__init();
	}
	nm_get_firmware_info(&rev);
	memset(_version, 0, 9);
	if (rev.u8FirmwareMajor != rev.u8DriverMajor && rev.u8FirmwareMinor != rev.u8DriverMinor) {
		sprintf(_version, "-Err-");
	}
	else {
		sprintf(_version, "%d.%d.%d", rev.u8FirmwareMajor, rev.u8FirmwareMinor, rev.u8FirmwarePatch);
	}
	return _version;
}

uint8_t WIFI__begin_default(void)
{
	if (!_init) {
		WIFI__init();
	}

	// Connect to router:
	if (_dhcp) {
		_localip = 0;
		_submask = 0;
		_gateway = 0;
	}
	if (m2m_wifi_default_connect() < 0) {
		_status = WL_CONNECT_FAILED;
		return _status;
	}
	_status = WL_IDLE_STATUS;
	_mode = WL_STA_MODE;

	// Wait for connection or timeout:
	unsigned long start = millis();
	while (!(_status & WL_CONNECTED) &&
			!(_status & WL_DISCONNECTED) &&
			millis() - start < 60000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	memset(_ap_info.ssid, 0, M2M_MAX_SSID_LEN);

	if (!(_status & WL_CONNECTED)) {
		_mode = WL_RESET_MODE;
	} else {
		m2m_wifi_get_connection_info();

		m2m_wifi_handle_events(NULL);
	}

	return _status;
}

uint8_t WIFI__begin_open(const char *ssid)
{
	return WIFI__startConnect(ssid, M2M_WIFI_SEC_OPEN, (void *)0);
}

uint8_t WIFI_begin_wep(const char *ssid, uint8_t key_idx, const char* key)
{
	tstrM2mWifiWepParams wep_params;

	memset(&wep_params, 0, sizeof(tstrM2mWifiWepParams));
	wep_params.u8KeyIndx = key_idx;
	wep_params.u8KeySz = strlen(key);
	strcpy((char *)&wep_params.au8WepKey[0], key);
	return WIFI__startConnect(ssid, M2M_WIFI_SEC_WEP, &wep_params);
}

uint8_t WIFI__begin_wpa(const char *ssid, const char *key)
{
	return WIFI__startConnect(ssid, M2M_WIFI_SEC_WPA_PSK, key);
}

uint8_t WIFI__startConnect(const char *ssid, uint8_t u8SecType, const void *pvAuthInfo)
{
	if (!_init) {
		WIFI__init();
	}

	// Connect to router:
	if (_dhcp) {
		_localip = 0;
		_submask = 0;
		_gateway = 0;
	}
	if (m2m_wifi_connect(ssid, strlen(ssid), u8SecType, pvAuthInfo, M2M_WIFI_CH_ALL) < 0) {
		_status = WL_CONNECT_FAILED;
		return _status;
	}
	_status = WL_IDLE_STATUS;
	_mode = WL_STA_MODE;

	// Wait for connection or timeout:
	unsigned long start = millis();
	while (!(_status & WL_CONNECTED) &&
			!(_status & WL_DISCONNECTED) &&
			millis() - start < 60000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}
	if (!(_status & WL_CONNECTED)) {
		_mode = WL_RESET_MODE;
	}

	memset(_ap_info.ssid, 0, M2M_MAX_SSID_LEN);
	memcpy(_ap_info.ssid, ssid, strlen(ssid));
	return _status;
}

uint8_t WIFI__beginAP(const char *ssid, uint8_t key_idx, const char* key, uint8_t channel)
{
	tstrM2mWifiWepParams wep_params;

	if (key_idx == 0) {
		key_idx = 1; // 1 is the minimum key index
	}

	memset(&wep_params, 0, sizeof(tstrM2mWifiWepParams));
	wep_params.u8KeyIndx = key_idx;
	wep_params.u8KeySz = strlen(key);
	strcpy((char *)&wep_params.au8WepKey[0], key);

	return WIFI__startAP(ssid, M2M_WIFI_SEC_WEP, &wep_params, channel);
}

uint8_t WIFI__startAP(const char *ssid, uint8_t u8SecType, const void *pvAuthInfo, uint8_t channel)
{
	tstrM2MAPConfig strM2MAPConfig;

	if (!_init) {
		WIFI__init();
	}

	if (channel == 0) {
		channel = 1; // channel 1 is the minium channel
	}

	// Enter Access Point mode:
	memset(&strM2MAPConfig, 0x00, sizeof(tstrM2MAPConfig));
	strcpy((char *)&strM2MAPConfig.au8SSID, ssid);
	strM2MAPConfig.u8ListenChannel = channel - 1;
	strM2MAPConfig.u8SecType = u8SecType;
	if (_localip == 0) {
		strM2MAPConfig.au8DHCPServerIP[0] = 192;
		strM2MAPConfig.au8DHCPServerIP[1] = 168;
		strM2MAPConfig.au8DHCPServerIP[2] = 1;
		strM2MAPConfig.au8DHCPServerIP[3] = 1;
	} else {
		memcpy(strM2MAPConfig.au8DHCPServerIP, &_localip, sizeof(_localip));
		if (strM2MAPConfig.au8DHCPServerIP[3] == 100) {
			// limitation of WINC1500 firmware, IP address of client is always x.x.x.100
			_status = WL_AP_FAILED;
			return _status;
		}
	}

	if (u8SecType == M2M_WIFI_SEC_WEP) {
		tstrM2mWifiWepParams* wep_params = (tstrM2mWifiWepParams*)pvAuthInfo;

		strM2MAPConfig.u8KeyIndx = wep_params->u8KeyIndx;
		strM2MAPConfig.u8KeySz = wep_params->u8KeySz;
		strcpy((char*)strM2MAPConfig.au8WepKey, (char *)wep_params->au8WepKey);
	}

	if (m2m_wifi_enable_ap(&strM2MAPConfig) < 0) {
		_status = WL_AP_FAILED;
		return _status;
	}
	_status = WL_AP_LISTENING;
	_mode = WL_AP_MODE;

	memset(_ap_info.ssid, 0, M2M_MAX_SSID_LEN);
	memcpy(_ap_info.ssid, ssid, strlen(ssid));
	m2m_memcpy((uint8 *)&_localip, (uint8 *)&strM2MAPConfig.au8DHCPServerIP[0], 4);
	_submask = 0x00FFFFFF;
	_gateway = _localip;

	// WiFi led ON (rev A then rev B).
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 0);
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);

	return _status;
}

uint8_t WIFI__beginProvision(uint8_t channel)
{
	// try to connect using begin
	if (WIFI__begin_default() != WL_CONNECTED) {
		// failed, enter provisioning mode

		uint8_t mac[6];
		char provSsid[M2M_MAX_SSID_LEN];

		// get MAC address for provisioning SSID
		WIFI__macAddress(mac);
		snprintf(provSsid, sizeof(provSsid), "3215-%.2X%2X", mac[1], mac[0]);

		// start provisioning mode
		WIFI__startProvision(provSsid, "dongle", channel);
	}

	return WIFI__status();
}

uint8_t WIFI__beginProvision_noAuto(uint8_t channel, bool autoConnect)
{
	// try to connect using begin
	if (autoConnect == true && WIFI__begin_default() != WL_CONNECTED) {
		// failed, enter provisioning mode

		uint8_t mac[6];
		char provSsid[M2M_MAX_SSID_LEN];

		// get MAC address for provisioning SSID
		WIFI__macAddress(mac);
		snprintf(provSsid, sizeof(provSsid), "3215-%.2X%2X", mac[1], mac[0]);

		// start provisioning mode
		WIFI__startProvision(provSsid, "dongle", channel);
	}

	return WIFI__status();
}

uint8_t WIFI__startProvision(const char *ssid, const char *url, uint8_t channel)
{
	tstrM2MAPConfig strM2MAPConfig;

	if (!_init) {
		WIFI__init();
	}

	// Enter Provision mode:
	memset(&strM2MAPConfig, 0x00, sizeof(tstrM2MAPConfig));
	strcpy((char *)&strM2MAPConfig.au8SSID, ssid);
	strM2MAPConfig.u8ListenChannel = channel;
	strM2MAPConfig.u8SecType = M2M_WIFI_SEC_OPEN;
	strM2MAPConfig.u8SsidHide = SSID_MODE_VISIBLE;
	strM2MAPConfig.au8DHCPServerIP[0] = 192;
	strM2MAPConfig.au8DHCPServerIP[1] = 168;
	strM2MAPConfig.au8DHCPServerIP[2] = 1;
	strM2MAPConfig.au8DHCPServerIP[3] = 1;

	if (m2m_wifi_start_provision_mode((tstrM2MAPConfig *)&strM2MAPConfig, (char*)url, 1) < 0) {
		_status = WL_PROVISIONING_FAILED;
		return _status;
	}
	_status = WL_PROVISIONING;
	_mode = WL_PROV_MODE;

	memset(_ap_info.ssid, 0, M2M_MAX_SSID_LEN);
	memcpy(_ap_info.ssid, ssid, strlen(ssid));
	m2m_memcpy((uint8 *)&_localip, (uint8 *)&strM2MAPConfig.au8DHCPServerIP[0], 4);
	_submask = 0x00FFFFFF;
	_gateway = _localip;

	// WiFi led ON (rev A then rev B).
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 0);
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);

	return _status;
}

uint32_t WIFI__provisioned(void)
{
	m2m_wifi_handle_events(NULL);

	if (_mode == WL_STA_MODE) {
		return 1;
	}
	else {
		return 0;
	}
}

// void WIFI__config(IPAddress local_ip)
// {
// 	config(local_ip, (uint32_t)0);
// }

// void WIFI__config(IPAddress local_ip, IPAddress dns_server)
// {
// 	config(local_ip, dns_server, (uint32_t)0);
// }

void WIFI__config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
	WIFI__config_subnet(local_ip, dns_server, gateway, INADDR_NONE);
}

void WIFI__config_subnet(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
 {
 	tstrM2MIPConfig conf;

 	if (!_init) {
 		WIFI__init();
 	}

 	conf.u32DNS = (uint32_t)dns_server._address.dword;
 	conf.u32Gateway = (uint32_t)gateway._address.dword;
 	conf.u32StaticIP = (uint32_t)local_ip._address.dword;
 	conf.u32SubnetMask = (uint32_t)subnet._address.dword;
 	_dhcp = 0;
 	m2m_wifi_enable_dhcp(0); // disable DHCP
 	m2m_wifi_set_static_ip(&conf);
 	_localip = conf.u32StaticIP;
 	_submask = conf.u32SubnetMask;
 	_gateway = conf.u32Gateway;
 }

void WIFI__disconnect(void)
{
	m2m_wifi_disconnect();

	// WiFi led OFF (rev A then rev B).
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 1);
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1);
}

void WIFI__end(void)
{
	if (_mode == WL_AP_MODE) {
		m2m_wifi_disable_ap();
	} else {
		if (_mode == WL_PROV_MODE) {
			m2m_wifi_stop_provision_mode();
		}

		m2m_wifi_disconnect();
	}

	// WiFi led OFF (rev A then rev B).
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 1);
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1);

	socketDeinit();

	m2m_wifi_deinit(NULL);

	nm_bsp_deinit();

	_mode = WL_RESET_MODE;
	_status = WL_NO_SHIELD;
	_init = 0;
}

uint8_t *WIFI__macAddress(uint8_t *mac)
{
	m2m_wifi_get_mac_address(mac);
	byte tmpMac[6], i;

	m2m_wifi_get_mac_address(tmpMac);

	for(i = 0; i < 6; i++)
		mac[i] = tmpMac[5-i];

	return mac;
}

uint32_t WIFI__localIP(void)
{
	return _localip;
}

uint32_t WIFI__subnetMask(void)
{
	return _submask;
}

uint32_t WIFI__gatewayIP(void)
{
	return _gateway;
}

char* WIFI__SSID(void)
{
	if (_status == WL_CONNECTED || _status == WL_AP_LISTENING || _status == WL_AP_CONNECTED) {
		return _ap_info.ssid;
	}
	else {
		return 0;
	}
}

char* WIFI__PASS(void)
{
	if (_status == WL_CONNECTED || _status == WL_AP_LISTENING || _status == WL_AP_CONNECTED) {
		return _ap_info.pass;
	}
	else {
		return 0;
	}
}

uint8_t* WIFI__BSSID(uint8_t* bssid)
{
	if (_mode == WL_AP_MODE) {
		return WIFI__macAddress(bssid);
	} else {
		return WIFI__remoteMacAddress(bssid);
	}
}

uint8_t* WIFI__APClientMacAddress(uint8_t* mac)
{
	if (_mode == WL_AP_MODE) {
		return WIFI__remoteMacAddress(mac);
	} else {
		memset(mac, 0, 6);
		return mac;
	}
}

uint8_t* WIFI__remoteMacAddress(uint8_t* remoteMacAddress)
{
	_remoteMacAddress = remoteMacAddress;
	memset(remoteMacAddress, 0, 6);

	m2m_wifi_get_connection_info();

	// Wait for connection or timeout:
	unsigned long start = millis();
	while (_remoteMacAddress != 0 && millis() - start < 1000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	_remoteMacAddress = 0;
	return remoteMacAddress;
}

int32_t WIFI__RSSI(void)
{
	// Clear pending events:
	m2m_wifi_handle_events(NULL);

	// Send RSSI request:
	_resolve = 0;
	if (m2m_wifi_req_curr_rssi() < 0) {
		return 0;
	}

	// Wait for connection or timeout:
	unsigned long start = millis();
	while (_resolve == 0 && millis() - start < 1000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	int32_t rssi = _resolve;

	_resolve = 0;

	return rssi;
}

int8_t WIFI__scanNetworks(void)
{
	wl_status_t tmp = _status;

	if (!_init) {
		WIFI__init();
	}

	// Start scan:
	if (m2m_wifi_request_scan(M2M_WIFI_CH_ALL) < 0) {
		return 0;
	}

	// Wait for scan result or timeout:
	_status = WL_IDLE_STATUS;
	unsigned long start = millis();
	while (!(_status & WL_SCAN_COMPLETED) && millis() - start < 5000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}
	_status = tmp;
	return m2m_wifi_get_num_ap_found();
}

char* WIFI__SSID_pos(uint8_t pos)
{
	wl_status_t tmp = _status;

	// Get scan SSID result:
	memset(_scan_ssid, 0, M2M_MAX_SSID_LEN);
	if (m2m_wifi_req_scan_result(pos) < 0) {
		return 0;
	}

	// Wait for connection or timeout:
	_status = WL_IDLE_STATUS;
	unsigned long start = millis();
	while (!(_status & WL_SCAN_COMPLETED) && millis() - start < 2000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	_status = tmp;
	_resolve = 0;

	return _scan_ssid;
}

int32_t WIFI__RSSI_pos(uint8_t pos)
{
	wl_status_t tmp = _status;

	// Get scan RSSI result:
	if (m2m_wifi_req_scan_result(pos) < 0) {
		return 0;
	}

	// Wait for connection or timeout:
	_status = WL_IDLE_STATUS;
	unsigned long start = millis();
	while (!(_status & WL_SCAN_COMPLETED) && millis() - start < 2000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	_status = tmp;

	int32_t rssi = _resolve;

	_resolve = 0;

	return rssi;
}

uint8_t WIFI__encryptionType(void)
{
	int8_t net = WIFI__scanNetworks();

	for (uint8_t i = 0; i < net; ++i) {
		WIFI__SSID_pos(i);
		if (strcmp(_scan_ssid, _ap_info.ssid) == 0) {
			break;
		}
	}

	return _scan_auth;
}

uint8_t WIFI__encryptionType_pos(uint8_t pos)
{
	wl_status_t tmp = _status;

	// Get scan auth result:
	if (m2m_wifi_req_scan_result(pos) < 0) {
		return 0;
	}

	// Wait for connection or timeout:
	_status = WL_IDLE_STATUS;
	unsigned long start = millis();
	while (!(_status & WL_SCAN_COMPLETED) && millis() - start < 2000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	_status = tmp;
	_resolve = 0;

	return _scan_auth;
}

uint8_t WIFI__status(void)
{
	if (!_init) {
		WIFI__init();
	}

	m2m_wifi_handle_events(NULL);
	return _status;
}

int WIFI__hostByName(const char* aHostname, IPAddress* aResult)
{

	// check if aHostname is already an ipaddress
	if (IPAddress__fromString(aHostname, aResult)) {
		// if fromString returns true we have an IP address ready
		return 1;

	} else {
		// Network led ON (rev A then rev B).
		//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 0);
		//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 0);

		// Send DNS request:
		_resolve = 0;
		if (gethostbyname((uint8 *)aHostname) < 0) {
			// Network led OFF (rev A then rev B).
			//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
			//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);
			return 0;
		}

		// Wait for connection or timeout:
		unsigned long start = millis();
		while (_resolve == 0 && millis() - start < 40000) {
            nm_wait_ms(2);
			m2m_wifi_handle_events(NULL);
		}

		// Network led OFF (rev A then rev B).
		//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
		//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);

		if (_resolve == 0) {
			return 0;
		}

		aResult->_address.dword = _resolve;
		_resolve = 0;
		return 1;
	}
}

void WIFI__refresh(void)
{
	// Update state machine:
	m2m_wifi_handle_events(NULL);
}

void WIFI__lowPowerMode(void)
{
	m2m_wifi_set_sleep_mode(M2M_PS_H_AUTOMATIC, true);
}

void WIFI__maxLowPowerMode(void)
{
	m2m_wifi_set_sleep_mode(M2M_PS_DEEP_AUTOMATIC, true);
}

void WIFI__noLowPowerMode(void)
{
	m2m_wifi_set_sleep_mode(M2M_NO_PS, false);
}

int WIFI__ping_host(const char* hostname, uint8_t ttl)
{
	IPAddress ip;

	if (WIFI__hostByName(hostname, &ip) > 0) {
		return WIFI__ping_ip(ip, ttl);
	} else {
		return WL_PING_UNKNOWN_HOST;
	}
}

//int WIFI__ping(const String &hostname, uint8_t ttl)
//{
//	return ping(hostname.c_str(), ttl);
//}

int WIFI__ping_ip(IPAddress host, uint8_t ttl)
{
	// Network led ON (rev A then rev B).
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 0);
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 0);

	uint32_t dstHost = (uint32_t)host._address.dword;
	_resolve = dstHost;

	if (m2m_ping_req((uint32_t)dstHost, ttl, &ping_cb) < 0) {
		// Network led OFF (rev A then rev B).
		//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
		//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);
		//  Error sending ping request
		return WL_PING_ERROR;
	}

	// Wait for success or timeout:
	unsigned long start = millis();
	while (_resolve == dstHost && millis() - start < 5000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	// Network led OFF (rev A then rev B).
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
	//m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);

	if (_resolve == dstHost) {
		_resolve = 0;
		return WL_PING_TIMEOUT;
	} else {
		int rtt = (int)_resolve;
		_resolve = 0;
		return rtt;
	}
}

uint32_t WIFI__getTime(void)
{
#ifdef WIFI_101_NO_TIME_H
	#warning "No system <time.h> header included, WIFI__getTime() will always return 0"
	return 0;
#else
	tstrSystemTime systemTime;

	_resolve = (uint32_t)&systemTime;

	m2m_wifi_get_sytem_time();

	unsigned long start = millis();
	while (_resolve != 0 && millis() - start < 5000) {
        nm_wait_ms(2);
		m2m_wifi_handle_events(NULL);
	}

	time_t t = 0;

	if (_resolve == 0 && systemTime.u16Year > 0) {
		struct tm tm;

		tm.tm_year = systemTime.u16Year - 1900;
		tm.tm_mon = systemTime.u8Month - 1;
		tm.tm_mday = systemTime.u8Day + 1;
		tm.tm_hour = systemTime.u8Hour;
		tm.tm_min = systemTime.u8Minute;
		tm.tm_sec = systemTime.u8Second;
		tm.tm_isdst = -1;

		t = mktime(&tm);
	}

	_resolve = 0;

	return t;
#endif
}

//WiFiClass WiFi;
