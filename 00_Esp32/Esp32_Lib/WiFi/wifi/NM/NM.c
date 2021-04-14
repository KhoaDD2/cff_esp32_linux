/******************************************************************************=
================================================================================
INNOVA ELECTRONICS VIETNAM
Filename:       NM.cpp
Description:    Network manager
Layer:
Accessibility:
================================================================================
*******************************************************************************/
#include <stdio.h>
#include "NM.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "esp32_at.h"

#if 1
extern void System_DelayMs(unsigned int ms);
extern unsigned char WIFI__begin_default(void);
#endif

void NM_status_cb(uint8_t newSt);

static wl_status_t stat = WL_IDLE_STATUS;
static int8_t rssi;

static void _nm_at_scan(void* data, void* param);
static void _nm_at_conn_check(void* data, void* param);
static void _nm_connect_cb(void* data, void* param);

typedef struct {
  wl_ap_info_t* list;
  uint32_t numAp;
  uint32_t listMaxAp;
} ap_list_t;
static ap_list_t nm_list;

void NM__init(void)
{
  rssi = -100;
}

void NM__teardown(void)
{

}

bool NM__connect(char* ssid, char* pass)
{
  if (ssid) // Connect with SSID and PASSWD
  {
    ssid_conn_info_t info;
    memset(&info, 0x00, sizeof info);
    strcpy(info.ap_name, ssid);
    strcpy(info.pass, pass);
    esp32_at_cmdSync(AT_CMD_CONNECT, (uint8_t*) &info, sizeof info, NULL, NULL, 25000);
  }
  else
  {
    // FIXME Connect without saved SSID
  }

  if (esp32_at_cmdSync(AT_CMD_AT, NULL, 0, NULL, NULL, 0))
  {
    NM__connection_check();
  }

  return (stat == WL_CONNECTED);
}

bool NM__is_connected(void)
{
  // if (stat != WL_CONNECTED)
  // {
  //   NM__connection_check();
  // }
  return (stat == WL_CONNECTED);
}

void NM__connection_check(void)
{
  esp32_at_cmdSync(AT_CMD_CHECK_CONN, NULL, 0, _nm_at_conn_check, NULL, 0);
}

// bool NM__connectDefault(void)
// {
//     return (WL_CONNECTED != WIFI__begin_default());
// }

int8_t NM__ScanNetworks(wl_ap_info_t* ap_list, uint32_t max_nbr_ap)
{
  // ap_list_t list;
  nm_list.list = ap_list;
  nm_list.listMaxAp = max_nbr_ap;
  memset(ap_list, 0x00, max_nbr_ap * sizeof(wl_ap_info_t));
  nm_list.numAp = 0;
  if (esp32_at_cmdSync(AT_CMD_CW_MODE3, NULL, 0, NULL, NULL, 0))
  {
    esp32_at_cmdSync(AT_CMD_SCAN, NULL, 0, _nm_at_scan, (void*) &nm_list, 0);
  }
  return nm_list.numAp;
}

/*
* @author Dat Le
* @brief Scan for APs
*/
static void _nm_at_scan(void* data, void* param)
{
  if (!param) return;
  ap_list_t* listAp = param;
  if (strstr((char*) data, "+CWLAP:") != NULL)
  {
    uint8_t buf[128] =  {0x00};
    uint8_t* txt = (uint8_t*) data + 11;
    uint8_t idx = 0;
    int32_t rssi = 0;
    uint8_t ssidlen = 0;
    do
    {
      buf[idx++] = *txt++;
      ssidlen += 1;
    } while (*txt != 0  && *txt != '\r' && !(*txt == '\"' && *(txt+1) == ',' && *(txt+2) == '-'));
    if (*txt == '\"' && *(txt+1) == ',')
    {
      rssi = strtol((txt+2), NULL, 10);
      if (listAp->numAp < listAp->listMaxAp)
      {
        memcpy(listAp->list[listAp->numAp].ssid, buf, ssidlen);
        listAp->list[listAp->numAp].rssi = rssi;
        listAp->numAp += 1;
      }
    }
  }
}

/*
* @author Dat Le
* @brief Get connection status
*/
static void _nm_at_conn_check(void* data, void* param)
{
  uint8_t tmp[6] = {0x00};
  if (strstr((char*) data, "+CWJAP:") != NULL) {
    stat = WL_CONNECTED;
    memcpy(tmp, (char*) data + strlen((char*) data) - 6, 6);
    char *c = strstr(tmp, "-");
    if (c != NULL) {
      long t = -100;
      t = strtol(c, NULL, 10);
      rssi = (int8_t) t;
    }
  }
}

#if ASYNC_API
/*
* @author Dat Le
* @brief Async call for NM ScanNetworks
*/
int8_t NM__ScanNetworks_Async(wl_ap_info_t* ap_list, uint32_t max_nbr_ap)
{
  nm_list.list = ap_list;
  nm_list.listMaxAp = max_nbr_ap;
  memset(ap_list, 0x00, max_nbr_ap * sizeof(wl_ap_info_t));
  nm_list.numAp = 0;
  esp32_at_cmd_asyncQueue(AT_CMD_SCAN, NULL, _nm_at_scan, (void*) &nm_list, &nm_list.numAp);
  return nm_list.numAp;
}

/*
* @author Dat Le
* @brief Async call of NM Connect
*/
bool NM__connect_Async(char* ssid, char* pass)
{
  bool result = true;
  if (!ssid) return false;
  ssid_conn_info_t* apInfo = (ssid_conn_info_t*) malloc(sizeof(ssid_conn_info_t));
  if (apInfo)
  {
    memset(apInfo, 0x00, sizeof(ssid_conn_info_t));
    strncpy(apInfo->ap_name, ssid, 32);
    if (pass)
    {
      strncpy(apInfo->pass, pass, 64);
    }
    esp32_at_cmd_asyncQueue(AT_CMD_CONNECT, apInfo, NULL, NULL, NULL);

    // NOTE Free in asynce queue
  }
  return result;
}

static void _nm_connect_cb(void* data, void* param)
{
  // NM__connection_check();
}
#endif

void NM_status_cb(uint8_t newSt)
{
  if (newSt)
  {
    stat = WL_CONNECTED;
  }
  else
  {
    stat = WL_IDLE_STATUS;
  }
}

int8_t NM__GetConnectionRssi(void)
{
  NM__connection_check();
  return rssi;
}
