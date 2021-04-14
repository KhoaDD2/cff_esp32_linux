#ifndef __NM_H__
#define __NM_H__
//#include "../Wifi101.h"

#include <stdint.h>

#define M2M_MAX_SSID_LEN 								33
/*!< Maximum size for the Wi-Fi SSID including the NULL termination.
 */


#define M2M_MAX_PSK_LEN           						65
/*!< Maximum size for the WPA PSK including the NULL termination.
 */


#define M2M_DEVICE_NAME_MAX								48
/*!< Maximum Size for the device name including the NULL termination.
 */

typedef struct
{
    char ssid[M2M_MAX_SSID_LEN];
    char pass[M2M_MAX_PSK_LEN];
    int32_t rssi;
    uint8_t sec_type;
} wl_ap_info_t;


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

#include <stdbool.h>
#include <stdint.h>

#define NTP_EN          0


#define NW_RECONNECT_RETRY          0


#define NUMBER_SAVED_SSID   5
typedef struct
{
    uint8_t nbr_ap;
    // FIXME
    //wl_ap_info_t ap[NUMBER_SAVED_SSID];
} WIFI_SAVED_AP_T;

#ifdef __cplusplus
extern "C" {
#endif

void NM__init(void);
void NM__teardown(void);
void NM__restart(void);
bool NM__connect(char* ssid, char* pass);
bool NM__is_connected(void);
void NM__connection_check(void);
bool NM__connect_noAuto(char* ssid, char* pass, bool connectDefault);
bool NM__connectDefault(void);
int8_t NM__GetConnectionRssi(void);

int8_t NM__ScanNetworks(wl_ap_info_t* ap_list, uint32_t max_nbr_ap);

#ifdef __cplusplus
}
#endif

#endif
