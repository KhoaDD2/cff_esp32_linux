#ifndef __ESP32_AT_H
#define __ESP32_AT_H
#include <stdint.h>
#include "BLE/ble_at_support.h"

#define printf
#define TOK_AND_MSG(x, msg) {x, sizeof(x) - 1, msg}

extern uint8_t cipmux;

typedef void (*atcbfn) (void*, void*);

typedef struct
{
  uint8_t hostname[30];
  uint8_t ip[20];
  uint8_t port[7];
} at_tcp_host_info_t;

typedef struct {
  uint8_t ap_name[33];
  uint8_t pass[65];

} ssid_conn_info_t;

typedef struct {
  uint8_t enable;
  uint8_t timeSecond;
} ble_scan_info_t;

typedef struct {
  uint8_t bleDiscoSvc;
} ble_disco_svc_t;

typedef struct {
  bt_deviceProp_t* devProp;
  uint8_t* data;
  uint32_t dataLen;
} ble_write_prop_t;

typedef struct {
  uint8_t input;
  uint8_t output;
  uint8_t* pin[7];
} bt_sec_param_t;

typedef enum
{
  AT_CMD_NONE = 0,
  AT_CMD_AT,
  AT_CMD_SCAN,
  AT_CMD_CHECK_CONN,
  AT_CMD_CONN_TCP,
  AT_CMD_CONN_TCP_MUX,
  AT_CMD_DOMAIN,
  AT_CMD_CONN_CLOSE,
  AT_CMD_CONN_CLOSE_MUX,
  AT_CMD_TCP_SEND_INIT,
  AT_CMD_TCP_SEND_MUX_INIT,
  AT_CMD_TCP_SEND_DATA,
  AT_CMD_CW_MODE3,
  AT_CMD_CONNECT,
  AT_CMD_GET_STA_MAC,
  AT_CMD_AUTO_CONNECT,
  AT_CMD_CW_MODE0,

  // BLE
  AT_CMD_BLE_INIT,
  AT_CMD_BLE_NAME,
  AT_CMD_BLE_ADDR,
  AT_CMD_BLE_GET_ADDR,
  AT_CMD_BLE_CHECK_CONN,
  AT_CMD_BLE_CHECK_INIT,

  AT_CMD_BLE_GATT_CRE,
  AT_CMD_BLE_GATT_START,
  AT_CMD_BLE_GATT_STOP,

  AT_CMD_BLE_ADV_DATA,
  AT_CMD_BLE_ADV_START,
  AT_CMD_BLE_ADV_STOP,
  AT_CMD_BLE_SCAN_RSP_DATA,

  AT_CMD_BLE_NOTIFY_INIT,
  AT_CMD_BLE_NOTIFY_INIT_FFF0,
  AT_CMD_BLE_NOTIFY_SEND_DATA,

  AT_CMD_BLE_SPP_CFG,
  AT_CMD_BLE_SPP_ENABLE,


  // Central BLE
  AT_CMD_BLE_SCAN,
  AT_CMD_BLE_SCAN_PARAM,
  AT_CMD_BLE_CONNECT,
  AT_CMD_BLE_DISCPRIM,
  AT_CMD_BLE_DISCCHAR,
  AT_CMD_BLE_WRITE_CHAR_PREP,
  AT_CMD_BLE_DISCONNECT,
  AT_CMD_BLE_WRITE_CHAR,

  // BT SPP
  AT_CMD_BT_INIT,
  AT_CMD_BT_SPP_INIT,
  AT_CMD_BT_NAME,
  AT_CMD_BT_SEC_PARAM,
  // SoftAP
  AT_CMD_AP_CIPMUX,
  AT_CMD_AP_CIPSERVER,
  AT_CMD_AP_CIPSERVER_STOP,

  AT_CMD_BT_SCAN_MODE,
  AT_CMD_BT_SPP_START,
  AT_CMD_BT_SPP_SEND_INIT,
  AT_CMD_BT_SPP_SEND_DATA,


  // General
  AT_CMD_GET_VERSION,
  AT_CMD_DEEP_SLEEP,
  AT_CMD_UART_CUR,
} eAtCmd;

typedef enum
{
  eApDisconnected = 0,
  eApConnected,
} eApStatus;

typedef struct {
  uint8_t queueId;
  eAtCmd cmd;
  uint8_t cmdStatus;
  atcbfn cbFn;
  void* param;
  void* cmdData;
  void* completeCbData;
} atCmdQueue_t;

typedef enum {
  CMD_STATUS_INIT = 0,
  CMD_STATUS_WAIT_RESULT,
  CMD_STATUS_DONE,
  CMD_STATUS_TIMEOUT,
} eCmdStatus;

enum esp32_message_t {
        MSG_NONE = 0,
        MSG_READY,
        MSG_BUSY,
        MSG_OK,
        MSG_ERROR,
        MSG_IPD,
        MSG_WIFI_CONN,
        MSG_WIFI_DISCONN,
        MSG_SEND_INDICATE,
        MSG_WRITE,
        MSG_BLE_CONN,
        MSG_BLE_PARAM,
        MSG_BLE_DISCONN,
        MSG_CLOSED,
        MSG_SEND_OK,
        MSG_WIFI_GOTIP,
        MSG_LINE_BREAK,
        #if ESP32_BT_ENABLE
        MSG_BTDATA,
        MSG_BTSPPDISCONN,
        MSG_BTSPPCONN,
        MSG_AT_BT_GAP_CB,
        MSG_AT_BT_SPP_CB,
        #endif
};

#pragma pack(1)
struct rx_status_tok {
        uint8_t *tok;
        uint32_t tok_len;
        enum esp32_message_t msg;
};
#pragma pack()


/*
* @author Dat Le
* @brief AT CMD Sync mode
*/
uint32_t esp32_at_cmdSync(eAtCmd cmd, uint8_t* input, uint32_t input_size, atcbfn cbfn, void* atcbpr, uint32_t timeout);

/*
* @author Dat Le
* @brief RX Handle
*/
void esp32_at_rxProcess(atcbfn fn, void* atcbpr, uint8_t* done);

/*
* @author Dat Le
* @brief Init ESP32 AT
*/
void esp32_at_init(void);

/*
* @author Dat Le
* @brief Return a byte from TCP Rx buffer
*/
uint32_t esp32_at_tcp_rx(uint8_t* c);

/*
* @author Dat Le
*/
void ClearSendTcpIndication(void);

/*
* @author Dat Le
*/
uint8_t GetTcpSendIndication(void);

/*
* @author Dat Le
*/
uint32_t esp32_at_cmd_asyncQueue(eAtCmd cmd, void* cmdData, atcbfn cbfn, void* param, void* resultData);

/*
* @author Dat Le
* @brief This allows caller to break current CMD wait in esp32_at
*/
void ESP32_IsBreakCmd(uint8_t isBreak);

#endif /*__TCP_ESP32_AT_H*/
