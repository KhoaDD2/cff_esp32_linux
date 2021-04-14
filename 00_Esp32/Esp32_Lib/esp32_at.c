#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if ESP32
#include "esp32.h"
#else
#include "3_Wifi/esp32/esp32/esp32.h"
#endif
#include "esp32_at.h"
//#include <intrinsics.h>

#include "3rd/Ring-Buffer/ringbuffer.h"
#include "timer_drv.h"
#include "uart_drv.h"

#include "BLE/ble_at_support.h"
#include "wifi.h"

#if ESP32_BT_ENABLE
#include "./bt/bt_at.h"
extern BT_ConnectedCbFn bt_conn_cb;
#endif

#if ESP32_WIFI_SOFTAP_ENABLE
#include "inc/VBUS_Inc.h"
#endif

#include "logfile.h"

extern void HTTPClient__init(void);
extern void NM__connection_check(void);

/*=============================================================================
*
*   @section Macros
*
/=============================================================================*/
#define BT_RAW_DATA_MAX_SZ                  (32)
#define ESP32_AT_TIMEOUT_MS                 (10000)
#define ESP32_AT_TIMEOUT_ASYNC_MS           (20000)
#define CMD_QUEUE_DEPTH                     (2) // #20200301 idx 0 for BLE, idx 1 for softap

typedef struct {
        uint8_t state;
        atcbfn cbFn;
        void*  cbParam;
} asyncProcessState_t;
asyncProcessState_t asyncState;

/*=============================================================================
*
*   @section Private data
*
/=============================================================================*/
static AsyncCbFn asynccb = NULL;
static uint8_t gotIp = 0;

static ring_buffer_t tcp_rx_buf;
static const uint8_t esp32_lib_version[] = ESP32_LIB_VERSION;
static uint8_t esp32_mac_addr[18] = {0x00};
atCmdQueue_t at_cmd_q[CMD_QUEUE_DEPTH];
static uint8_t tcpSendEnable = 0;
static uint8_t esp32_version[33] = {0x00};
static uint8_t statusBreak = 0;
uint8_t cipmux = 0;
static uint8_t bt_raw_data[BT_RAW_DATA_MAX_SZ] = {0x00};
static uint32_t bt_raw_len = 0;
static uint8_t bt_has_data = 0;


/* TODO Breakdown features like BLE Central, Periph, TCP */

static const struct rx_status_tok rx_st_l[] = {
        TOK_AND_MSG("ready", MSG_READY),
        TOK_AND_MSG("busy p...", MSG_BUSY),
        TOK_AND_MSG("ERROR", MSG_ERROR),
        TOK_AND_MSG("OK", MSG_OK),
        TOK_AND_MSG("+IPD", MSG_IPD),
        TOK_AND_MSG("WIFI CONNECTED", MSG_WIFI_CONN),
        TOK_AND_MSG("WIFI GOT IP", MSG_WIFI_GOTIP),
        TOK_AND_MSG("SEND OK", MSG_SEND_OK),
        TOK_AND_MSG("WIFI DISCONNECT", MSG_WIFI_DISCONN),
        TOK_AND_MSG(">", MSG_SEND_INDICATE),
        TOK_AND_MSG("+WRITE", MSG_WRITE),
        TOK_AND_MSG("+BLECONN:", MSG_BLE_CONN), // 20200111 dattl workaround, to distinguish with connparam
        TOK_AND_MSG("+BLECONNPARAM", MSG_BLE_PARAM),
        TOK_AND_MSG("+BLEDISCONN", MSG_BLE_DISCONN),
        TOK_AND_MSG("CLOSED", MSG_CLOSED),
        TOK_AND_MSG("0,CLOSED", MSG_CLOSED),
        TOK_AND_MSG("\r\n", MSG_LINE_BREAK),
        #if ESP32_BT_ENABLE
        TOK_AND_MSG("+BTDATA", MSG_BTDATA),
        TOK_AND_MSG("+BTSPPDISCONN", MSG_BTSPPDISCONN),
        TOK_AND_MSG("+BTSPPCONN", MSG_BTSPPCONN),
        TOK_AND_MSG("at_bt_gap_cb", MSG_AT_BT_GAP_CB),
        TOK_AND_MSG("at_bt_spp_cb", MSG_AT_BT_SPP_CB),
        #endif
};
#define RX_TOK_LIST_SIZE (sizeof(rx_st_l) / sizeof(*(rx_st_l)))

static const uint8_t* txt_at_cmd_at = "AT\r\n";

#if ESP32_WIFI_ENABLE
static const uint8_t* txt_at_cmd_scan = "AT+CWLAP\r\n";
static const uint8_t* txt_at_cmd_check_conn = "AT+CWJAP?\r\n";
static const uint8_t* txt_at_cmd_conn_tcp = "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n";
static const uint8_t* txt_at_cmd_conn_tcp_mux = "AT+CIPSTART=0,\"TCP\",\"%s\",%s\r\n";
static const uint8_t* txt_at_cmd_dns = "AT+CIPDOMAIN=\"%s\"\r\n";
static const uint8_t* txt_at_cmd_close_tcp = "AT+CIPCLOSE\r\n";
static const uint8_t* txt_at_cmd_close_tcp_mux = "AT+CIPCLOSE=0\r\n";
static const uint8_t* txt_at_cmd_tcp_send_init = "AT+CIPSEND=%lu\r\n";
static const uint8_t* txt_at_cmd_connect = "AT+CWJAP=\"%s\",\"%s\"\r\n";
static const uint8_t* txt_at_cmd_cwmode0 = "AT+CWMODE=0\r\n";
#endif

static const uint8_t* txt_at_cmd_cwmode3 = "AT+CWMODE=3\r\n";
static const uint8_t* txt_at_cmd_auto_wifi = "AT+CWAUTOCONN=%u\r\n";

#if ESP32_WIFI_SOFTAP_ENABLE
static const uint8_t* txt_at_cmd_tcp_send_mux_init = "AT+CIPSEND=0,%lu\r\n";
static const uint8_t* txt_at_cmd_cipmux = "AT+CIPMUX=1\r\n";
static const uint8_t* txt_at_cmd_cipserver = "AT+CIPSERVER=1,8082\r\n";
static const uint8_t* txt_at_cmd_cipserver_stop = "AT+CIPSERVER=0\r\n";
#endif

static const uint8_t* txt_at_cmd_ble_init = "AT+BLEINIT=%s\r\n"; //* Note text input here*/
static const uint8_t* txt_at_cmd_ble_gatt_cre = "AT+BLEGATTSSRVCRE\r\n";
static const uint8_t* txt_at_cmd_ble_gatt_start = "AT+BLEGATTSSRVSTART\r\n";
static const uint8_t* txt_at_cmd_ble_gatt_stop = "AT+BLEGATTSSRVSTOP\r\n";
static const uint8_t* txt_at_cmd_ble_adv_data = "AT+BLEADVDATA=\"%s\"\r\n"; // Data in hex text
static const uint8_t* txt_at_cmd_ble_adv_start = "AT+BLEADVSTART\r\n";
static const uint8_t* txt_at_cmd_ble_adv_stop = "AT+BLEADVSTOP\r\r";
static const uint8_t* txt_at_cmd_ble_name = "AT+BLENAME=\"%s\"\r\n";
static const uint8_t* txt_at_cmd_ble_addr = "AT+BLEADDR=1,\"%s\"\r\n"; /* Note address hex in text xx:xx:xx:xx:xx:xx */
static const uint8_t* txt_at_cmd_ble_notify_init = "AT+BLEGATTSNTFY=0,1,1,%lu\r\n";
static const uint8_t* txt_at_cmd_ble_notify_fff0_init = "AT+BLEGATTSNTFY=0,2,1,%lu\r\n";
static const uint8_t* txt_at_cmd_get_sta_mac = "AT+CIPSTAMAC?\r\n";
static const uint8_t* txt_at_cmd_get_module_addr = "AT+BLEADDR?\r\n";
static const uint8_t* txt_at_cmd_get_version = "AT+GMR\r\n";
static const uint8_t* txt_at_cmd_deep_sleep = "AT+GSLP=%lu\r\n";
static const uint8_t* txt_at_cmd_ble_scan_rsp_data = "AT+BLESCANRSPDATA=\"%s\"\r\n";
static const uint8_t* txt_at_cmd_ble_spp_cfg = "AT+BLESPPCFG=1,0,0,0,2\r\n";
static const uint8_t* txt_at_cmd_ble_spp_en = "AT+BLESPP\r\n";
static const uint8_t* txt_at_cmd_ble_check_conn = "AT+BLECONN?\r\n";
static const uint8_t* txt_at_cmd_ble_check_init = "AT+BLEINIT?\r\n";

#if ESP32_BT_CENTRAL_ENABLE
static const uint8_t* txt_at_cmd_ble_scan = "AT+BLESCAN=%u,%u\r\n";
static const uint8_t* txt_at_cmd_ble_connect = "AT+BLECONN=0,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n";
static const uint8_t* txt_at_cmd_ble_disco_prime = "AT+BLEGATTCPRIMSRV=0\r\n";
static const uint8_t* txt_at_cmd_ble_disco_char = "AT+BLEGATTCCHAR=0,%u\r\n";
static const uint8_t* txt_at_cmd_ble_disconnect = "AT+BLEDISCONN=0\r\n";
static const uint8_t* txt_at_cmd_ble_writechar_prep = "AT+BLEGATTCWR=0,%u,%u,,%u\r\n"; // svcIdx, charIdx, len
static const uint8_t* txt_at_cmd_ble_scan_param = "AT+BLESCANPARAM=1,0,0,320,48\r\n"; // Set active scan param
#endif

#if ESP32_BT_ENABLE
static const uint8_t* txt_at_cmd_bt_init = "AT+BTINIT=%s\r\n";
static const uint8_t* txt_at_cmd_bt_name = "AT+BTNAME=\"%s\"\r\n";
static const uint8_t* txt_at_cmd_bt_scan_mode = "AT+BTSCANMODE=%s\r\n";
static const uint8_t* txt_at_cmd_bt_sec_param = "AT+BTSECPARAM=%u,%u,\"%s\"\r\n";
static const uint8_t* txt_at_cmd_bt_spp_init = "AT+BTSPPINIT=%s\r\n";
static const uint8_t* txt_at_cmd_bt_spp_start = "AT+BTSPPSTART\r\n";
static const uint8_t* txt_at_cmd_bt_spp_send_init = "AT+BTSPPSEND=0,%u\r\n";
#endif

#if ESP32_WIFI_SOFTAP_ENABLE
#define TX_BUFFER_SIZE                    (TX_MAX_LEN + 20)
#define BLE_FRAME_LEN                     (20)
#define TX_MAX_LEN                        (BLE_FRAME_LEN * 5)

static uint8_t txBuffer[TX_BUFFER_SIZE];

static void TcpReceiveMsg(VBUS_MSG* p_strtMIMsg, enumMsgRecState* p_eMsgRecState);
static void TcpMsgProcess(void);
static VBUS_MSG g_strtMsgInTcp;
static BleCmdCbFn tcpCmdCbFn = NULL;
void SoftApMain(void);
static uint32_t MI_SendTcpMsg(msgID msgId, uint8_t* data, uint16_t len, uint8_t isStatus);
#endif

static const uint8_t* txt_at_cmd_uart_cur = "AT+UART_CUR=%lu,8,1,0,1\r\n";

static uint32_t esp32_at_send(uint8_t* cmdData, uint32_t dataSize);
static void ap_process(eApStatus action, void* data);
static void tcp_rx_proces(void* data, uint32_t cnt);
static void ESP32_MacHandle(void* data, void* param);
static uint8_t* getCmdTextByCmd(eAtCmd cmd, uint8_t* input, uint32_t input_size);
static void esp32_at_asyncProcess(void);
static void _at_get_version_cb(void* data, void* param);
static enum esp32_message_t rx_tok_matching(uint8_t *buf, uint32_t bs);

#define RX_PROCESS_BUF_SIZE 256
#define STATIC_BUF 1
#if STATIC_BUF
static uint8_t esp32_buf[RX_PROCESS_BUF_SIZE];
static uint8_t cmdBuf[128];
#endif

extern void NM_status_cb(uint8_t newSt);
extern void esp32_bootmode(uint8_t mode);

extern uint8_t __socket;
extern uint8_t bleNotifyEnabled;

static enum esp32_message_t rx_tok_matching(uint8_t *buf, uint32_t bs)
{
        if (bs == 0) return MSG_NONE;

        enum esp32_message_t msg = MSG_NONE;
        bool stop = false;
        uint32_t bi = 0;
        uint32_t li = 0;
        while (!stop && bi < bs && li < RX_TOK_LIST_SIZE) {
                if (buf[bi] != rx_st_l[li].tok[bi]) {
                       li++;
                }
                else {
                        bi++;
                        if (bi == rx_st_l[li].tok_len) {
                                msg = rx_st_l[li].msg;
                                stop = true;
                        }
                }
                if (li >= RX_TOK_LIST_SIZE) {
                        stop = true;
                }
        }

        return msg;
}

static uint8_t* getCmdTextByCmd(eAtCmd cmd, uint8_t* input, uint32_t input_size)
{
        uint8_t* buf = NULL;
        uint8_t* text = NULL;
        switch (cmd) {
        case AT_CMD_AT:
                text = (uint8_t*) txt_at_cmd_at;
                break;
        case AT_CMD_GET_STA_MAC:
                text = (uint8_t*) txt_at_cmd_get_sta_mac;
                break;

        #if ESP32_WIFI_ENABLE
        case AT_CMD_SCAN:
                text = (uint8_t*) txt_at_cmd_scan;
                break;
        case AT_CMD_CONNECT:
             {   ssid_conn_info_t* info = (ssid_conn_info_t*) input;
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_connect, info->ap_name, info->pass);
                        text = buf;
                }
                break;}

        case AT_CMD_CHECK_CONN:
                text = (uint8_t*) txt_at_cmd_check_conn;
                break;

        case AT_CMD_CONN_TCP:
            {    at_tcp_host_info_t* host = (at_tcp_host_info_t*) input;
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_conn_tcp, host->ip, host->port);
                        text = buf;
                }
                break;}

        case AT_CMD_DOMAIN:
              {  uint8_t* domain = input;
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_dns, domain);
                        text = buf;
                }
                break;}

        case AT_CMD_CONN_CLOSE:
                {text = (uint8_t*) txt_at_cmd_close_tcp;
                break;}

        case AT_CMD_CONN_CLOSE_MUX:
                {text = (uint8_t*) txt_at_cmd_close_tcp_mux;
                break;}

        case AT_CMD_TCP_SEND_INIT:
               { buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 64, txt_at_cmd_tcp_send_init, input_size);
                        text = buf;
                }
                break;}

        case AT_CMD_CW_MODE3:
               { text = (uint8_t*) txt_at_cmd_cwmode3;
                break;
}
        case AT_CMD_CW_MODE0:
                {text = (uint8_t*) txt_at_cmd_cwmode0;
                break;}

        case AT_CMD_AUTO_CONNECT:
              {  buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        uint32_t sleep_time_ms = *((uint32_t*) input);
                        snprintf(buf, 64, txt_at_cmd_auto_wifi, *input);
                        text = buf;
                }
                break;}
        case AT_CMD_CONN_TCP_MUX:
            {    at_tcp_host_info_t* host = (at_tcp_host_info_t*) input;
                    host = (at_tcp_host_info_t*) input;
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_conn_tcp_mux, host->ip, host->port);
                        text = buf;
                }
                break;}
        #endif /* ESP32_WIFI_ENABLE */



        case AT_CMD_TCP_SEND_DATA:
                text = input;
                break;

        case AT_CMD_BLE_CHECK_CONN:
                text = (uint8_t *) txt_at_cmd_ble_check_conn;
                break;

        case AT_CMD_BLE_CHECK_INIT:
                text = (uint8_t *) txt_at_cmd_ble_check_init;
                break;

        case AT_CMD_BLE_INIT:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_ble_init, input);
                        text = buf;
                }
                break;

        case AT_CMD_BLE_NAME:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_ble_name, input);
                        text = buf;
                }
                break;

        case AT_CMD_BLE_ADDR:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_ble_addr, input);
                        text = buf;
                }
                break;

        case AT_CMD_BLE_GET_ADDR:
                text = (uint8_t*) txt_at_cmd_get_module_addr;
                break;

        case AT_CMD_BLE_GATT_CRE:
                text = (uint8_t*) txt_at_cmd_ble_gatt_cre;
                break;

        case AT_CMD_BLE_GATT_START:
                text = (uint8_t*) txt_at_cmd_ble_gatt_start;
                break;

        case AT_CMD_BLE_GATT_STOP:
                text = (uint8_t*) txt_at_cmd_ble_gatt_stop;
                break;

        case AT_CMD_BLE_ADV_DATA:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_ble_adv_data, input);
                        text = buf;
                }
                break;


        case AT_CMD_BLE_ADV_START:
                text = (uint8_t*) txt_at_cmd_ble_adv_start;
                break;

        case AT_CMD_BLE_NOTIFY_INIT:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 64, txt_at_cmd_ble_notify_init, input_size);
                        text = buf;
                }
                break;

        case AT_CMD_BLE_NOTIFY_INIT_FFF0:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 64,
                                        txt_at_cmd_ble_notify_fff0_init,
                                        input_size);
                        text = buf;
                }
                break;

        case AT_CMD_BLE_SPP_CFG:
                text = (uint8_t*) txt_at_cmd_ble_spp_cfg;
                break;

        case AT_CMD_BLE_SPP_ENABLE:
                text = (uint8_t*) txt_at_cmd_ble_spp_en;
                break;

        case AT_CMD_GET_VERSION:
                text = (uint8_t*) txt_at_cmd_get_version;
                break;

        case AT_CMD_BLE_NOTIFY_SEND_DATA:
                text = input;
                break;

        case AT_CMD_DEEP_SLEEP:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_deep_sleep, *input);
                }
                text = buf;
                break;

        #if ESP32_BT_CENTRAL_ENABLE
        case AT_CMD_BLE_SCAN:
                {ble_scan_info_t* spec = (ble_scan_info_t*) (input);
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_ble_scan, spec->enable, spec->timeSecond);
                }
                text = buf;
                break;}

        case AT_CMD_BLE_SCAN_PARAM:
                {text = (uint8_t*) txt_at_cmd_ble_scan_param;}
            break;

        case AT_CMD_BLE_CONNECT:
            {    bt_deviceInfo_t* btDev = (bt_deviceInfo_t*) (input);
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_ble_connect, btDev->bt_addr[0], btDev->bt_addr[1], btDev->bt_addr[2],
                                                         btDev->bt_addr[3], btDev->bt_addr[4], btDev->bt_addr[5]);
                }
                text = buf;
                break;}

        case AT_CMD_BLE_DISCPRIM:
               { text = (uint8_t*) txt_at_cmd_ble_disco_prime;
                break;}

        case AT_CMD_BLE_DISCCHAR:
        {
                ble_disco_svc_t* svc = (ble_disco_svc_t*) (input);
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_ble_disco_char, svc->bleDiscoSvc);
                }
                text = buf;
                break;
        }

        case AT_CMD_BLE_DISCONNECT:
              {  text = (uint8_t*) txt_at_cmd_ble_disconnect;
                break;}

        case AT_CMD_BLE_WRITE_CHAR_PREP:
              {  buf = cmdBuf;
                if (buf) {
                        ble_write_prop_t* writeProp = (ble_write_prop_t*) (input);
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_ble_writechar_prep, writeProp->devProp->writeServiceIdx, writeProp->devProp->writeCharIdx, writeProp->dataLen);
                }
                text = buf;
                break;}

        case AT_CMD_BLE_WRITE_CHAR:
              {  buf = cmdBuf;
                if (buf) {
                        //ble_write_prop_t* writeProp = (ble_write_prop_t*) (input);
                        // memcpy(buf, writeProp->data, writeProp->dataLen);
                        memcpy(buf, input, input_size);
                }
                text = buf;
                break;}
        #endif

        case AT_CMD_BLE_SCAN_RSP_DATA:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 128);
                        snprintf(buf, 127, txt_at_cmd_ble_scan_rsp_data, input);
                        text = buf;
                }
                break;

        case AT_CMD_UART_CUR:
                buf = cmdBuf;
                if (buf) {
                        uint32_t baud = *((uint32_t*) input);
                        memset(buf, 0x00, 64);
                        snprintf((char *)buf, 63, (const char *)txt_at_cmd_uart_cur, baud);
                }
                text = buf;
                break;
        #if ESP32_WIFI_SOFTAP_ENABLE
        case AT_CMD_AP_CIPMUX:
                text = (uint8_t*) txt_at_cmd_cipmux;
                break;

        case AT_CMD_AP_CIPSERVER:
                text = (uint8_t*) txt_at_cmd_cipserver;
                break;

        case AT_CMD_AP_CIPSERVER_STOP:
                text = (uint8_t*) txt_at_cmd_cipserver_stop;
                break;

        case AT_CMD_TCP_SEND_MUX_INIT:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 64, txt_at_cmd_tcp_send_mux_init, input_size);
                        text = buf;
                }
                break;
        #endif

        #if ESP32_BT_ENABLE
        case AT_CMD_BT_INIT:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 32);
                        snprintf(buf, 31, txt_at_cmd_bt_init, input);
                }
                text = buf;
                break;

        case AT_CMD_BT_NAME:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 64);
                        snprintf(buf, 63, txt_at_cmd_bt_name, input);
                }
                text = buf;
                break;

        case AT_CMD_BT_SCAN_MODE:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 32);
                        snprintf(buf, 31, txt_at_cmd_bt_scan_mode, input);
                }
                text = buf;
                break;

        case AT_CMD_BT_SEC_PARAM:
                buf = cmdBuf;
                if (buf) {
                        bt_sec_param_t * sec = (bt_sec_param_t *) input;
                        memset(buf, 0x00, 32);
                        snprintf(buf, 31, txt_at_cmd_bt_sec_param, sec->input, sec->output, sec->pin);
                }
                text = buf;
                break;

        case AT_CMD_BT_SPP_INIT:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 32);
                        snprintf(buf, 31, txt_at_cmd_bt_spp_init, input);
                }
                text = buf;
                break;

        case AT_CMD_BT_SPP_START:
                text = (uint8_t *) txt_at_cmd_bt_spp_start;
                break;

        case AT_CMD_BT_SPP_SEND_INIT:
                buf = cmdBuf;
                if (buf) {
                        memset(buf, 0x00, 32);
                        snprintf(buf, 31, txt_at_cmd_bt_spp_send_init, input_size);
                }
                text = buf;
                break;

        case AT_CMD_BT_SPP_SEND_DATA:
                text = input;
                break;
        #endif

        default:
                break;
        }

        return text;
}

/*
* @date Feb. 27, 2019
* @author Dat Le
* @brief RX/TX ESP32 command
*/
uint32_t esp32_at_cmdSync(eAtCmd cmd, uint8_t* input, uint32_t input_size, atcbfn cbfn, void* atcbpr, uint32_t timeout)
{
        uint32_t status = 0;
        uint32_t cmdTime = ESP32_AT_TIMEOUT_MS;
        uint8_t* text = getCmdTextByCmd(cmd, input, input_size);
        uint8_t done = 0;
        if (timeout) {
                cmdTime = timeout;
        }

        /* This made makes RX fails to receive LD stop streaming command
        uart_drv_buffer_clear();
        */
        if (text)
        {
                timer_drv_t cmdTimer;
                Timer_Create(&cmdTimer, cmdTime);
                if (cmd == AT_CMD_TCP_SEND_DATA
                        || cmd == AT_CMD_BLE_NOTIFY_SEND_DATA
                        || cmd == AT_CMD_BLE_WRITE_CHAR
                        #if ESP32_BT_ENABLE
                        || cmd == AT_CMD_BT_SPP_SEND_DATA
                        #endif
                        ) {
                        esp32_at_send(text, input_size);
                }
                else {
                        esp32_at_send(text, strlen((char *)text));
                }

        statusBreak = 0; // Reset command break state
        // esp32_at_rxProcess(NULL, NULL, &done); // #20200308 flush buffer
        // done = 0;
        while (!Timer_Timeout(&cmdTimer) && !statusBreak && (!done || cmd == AT_CMD_BLE_SCAN || cmd == AT_CMD_BLE_CONNECT)) {
                esp32_at_rxProcess(cbfn, atcbpr, &done); /* This loops to get result */
                if (statusBreak) {
                        done = 1;
                        //statusBreak = 0;
                        }
                }
        }

        return (uint32_t) done;
}

static uint32_t esp32_at_send(uint8_t* cmdData, uint32_t dataSize)
{
        //LOGI("Tx[%02i] %s",dataSize,cmdData);
        uart_put_bytes(cmdData, dataSize);
        LOGI("%s",cmdData);
        return 0;
}

/*
* @author Dat Le
* @brief RX Handle
*/
void esp32_at_rxProcess(atcbfn fn, void* atcbpr, uint8_t* done)
{
        static uint8_t overflow = 0;
        uint8_t* buf = esp32_buf;

        memset(buf, 0x00, RX_PROCESS_BUF_SIZE);
        uint8_t lend[2] = {0x00};
        uint8_t c = 0;
        uint32_t cnt = 0;
        uint32_t ret;
        do {
                c = 0;
                ret = uart_get_char(&c);
                if (ret) {
                        buf[cnt++] = c;
                        if (c == '\r' || c =='>') {
                                lend[0] = c;
                        }
                        else if (c == '\n' && lend[0] == '\r') {
                                lend[1] = '\n'; // Completed line ending
                                //LOGI("Rx[%i]", cnt);
                        }
                        else { // Reset
                                lend[0] = 0;
                                lend[1] = 0;
                        }
                }

                if (lend[1] == '\n') { // Will break;
                        // But in case of "+IPD" or "+WRITE"
                        enum esp32_message_t m = rx_tok_matching(buf, sizeof("+WRITE"));
                        if (m == MSG_IPD || m == MSG_WRITE) {
                                // Check if there still any thing
                                uint8_t t;
                                if (uart_peek_char(&t)) {
                                        if (t == '+') {
                                                // Next notify, break
                                                ret = 0;
                                        }
                                        else {
                                                // Continue to get
                                                lend[0] = 0;
                                                lend[1] = 0;
                                        }
                                }
                        } else {
                          ret = 0;
                        }
                }
                else if (!ret && cnt && lend[0] != '>') {
                        // Best effort to read more data
                        timer_drv_t rxto;
                        Timer_Create(&rxto, 50); // -> what the hell
                        uint8_t t;
                        while (!Timer_Timeout(&rxto)) {
                                // Continue to get if there is still data
                                if (uart_peek_char(&t)) {
                                        ret = 1;
                                }
                        }
                }
        }
        while (ret != 0 && cnt < RX_PROCESS_BUF_SIZE);

        //LOGI("cnt=%i", cnt);

        switch (rx_tok_matching(buf, cnt)) {
        case MSG_READY:
                break;
        case MSG_BUSY:
        case MSG_LINE_BREAK:
                break;
        case MSG_ERROR:
                if (done) {
                        *done = 1;
                }
                break;
        case MSG_WIFI_CONN:
                break;
        case MSG_WIFI_GOTIP:
                if (asyncState.state && at_cmd_q[0].cmd == AT_CMD_CONNECT) {
                        gotIp = 1;
                }
                ap_process(eApConnected, NULL);
                break;
        case MSG_IPD:
                tcp_rx_proces(buf, cnt);
                break;
        case MSG_SEND_OK:
                if (done)
                {
                        *done = 1; // TCP send okay
                }
                break;
        case MSG_OK:
                if (done) {
                        *done = 1;
                }
                if (!asyncState.state) {
                        ;
                }
                else if (at_cmd_q[0].cmd == AT_CMD_CONNECT) {
                        if (gotIp) {
                                asyncState.state = 0;
                        }
                }
                else {
                        asyncState.state = 0;
                }
                break;
        case MSG_SEND_INDICATE:
                if (done) {
                        *done = 1;
                }
                tcpSendEnable = 1;
                break;
        case MSG_WRITE:
                // BLE
                // FIXME: #Apr 9 2019, hacks for BLE write
                // +WRITE:0,1,1,,5,hahah
                //+WRITE:0,1,1,1,2,[01][00] // Notification
                /*+WRITE:<conn_index>,<srv_index>,<char_index>,[<desc_index>],<len>,<value>*/
                /*+NOTIFY:<conn_index>,<srv_index>,<char_index>,<len>,<value>*/
                /*
                Service index 2
                +WRITE:0,2,1,1,2,[01][00]
                +WRITE:0,2,2,,4,haha
                */
                /*
                * @note In case buf[13] is ',' it is a characteristic write
                * otherwise it is a descriptor write
                */
                if (buf[13] == ',') {
                        if (buf[9] == '1') {
                                /* Service index 1 */
                                uint32_t rxLen = strtol((const char *)buf+14, NULL, 10);
                                uint8_t* data = buf +15 +(rxLen < 10?1:2);
                                for (uint32_t i = 0; i < rxLen; i++) {
                                        ble_rxCallback(data[i]);
                                }
                        }
                        else if (buf[9] == '2') {
                                /* Service index 2, for ELM327 */
                                uint32_t rx_len = strtol((const char *)buf+14, NULL, 10);
                                uint8_t* data = buf +15 +(rx_len < 10?1:2);
                                memcpy(bt_raw_data, data, rx_len);
                                bt_raw_len = rx_len;
                                bt_has_data = 1;
                        }
                }
                else if (buf[13] == '1') {
                        /* Write Description */
                        if (buf[17] == 1) {
                                bleNotifyEnabled = 1;
                        }
                        else {
                                bleNotifyEnabled = 0;
                        }
                }
                break;
        case MSG_BLE_PARAM:
                break;
        case MSG_BLE_CONN:
                #if ESP32_BT_CENTRAL_ENABLE
                if (isBleModeCentral()) {
                        *done = 1;
                        if (fn) {
                                fn(buf, atcbpr);
                        }
                }
                else {
                        BLE_InternalConnectionCb(1);
                }
                #else
                    BLE_InternalConnectionCb(1);
                #endif
                break;
        case MSG_BLE_DISCONN:
                #if ESP32_BT_CENTRAL_ENABLE
                if (isBleModeCentral()) {
                        *done = 1;
                        BLE_InternalConnectionCb(0);
                }
                else {
                        BLE_InternalConnectionCb(0);
                }
                #else
                    BLE_InternalConnectionCb(0);
                #endif
                break;

        #if ESP32_BT_ENABLE
        case MSG_BTSPPCONN:
        if (bt_conn_cb) {
            bt_conn_cb(1);
        }
        break;

        case MSG_BTSPPDISCONN:
        if (bt_conn_cb) {
            bt_conn_cb(0);
        }
        break;

        case MSG_BTDATA:
        {
                /* +BTDATA:14,hello classic  */
                uint8_t* text = strstr(buf, ",");
                uint32_t len = 0;
                len = strtol((char*) (buf + 8), NULL, 10);
                if (text && len)
                {
                        if (bt_data_cb != NULL) {
                                bt_data_cb(text+1, len);
                                /* FIXME IMPORTANT consider recursive bug */
                        }
                }
        }
        break;
        #endif

        case MSG_CLOSED:
                __socket = 0;
                break;
        case MSG_NONE:
        default:
                if (!cnt) {
                        break;
                }
                if (fn) {
                        fn(buf, atcbpr);
                }
                if (asyncState.state && asyncState.cbFn) {
                        asyncState.cbFn(buf, asyncState.cbParam);
                }
                break;
        }
        #if !(STATIC_BUF)
        free(buf);
        #endif
}

static void ap_process(eApStatus action, void* data)
{
        if (action == eApDisconnected)
        {
                // Disconnect
                NM_status_cb(0);
        }
        else if (action == eApConnected)
        {
                // Connected
                NM_status_cb(1);
        }
        else
        {
                ;
        }
}

static void tcp_rx_proces(void* data, uint32_t cnt)
{
        // +IPD,<len>[,<remoteIP>,<remote port>]:<data>
        //+IPD,<link ID>,<len>[,<remoteIP>,<remote port>]:<data> //(+CIPMUX=1)
        // if cipmux = 1
        // +IPD,0,6:hell
        uint8_t* txt = (uint8_t*) data;
        uint8_t* tok = NULL;
        uint32_t rxLen = 0;

        if (cipmux) {
                rxLen = strtol((const char*) data+(uint32_t)strlen("+IPD,0,"),(char **)&tok,10);
        }
        else {
                rxLen = strtol((const char*) data+(uint32_t)strlen("+IPD,"),(char **)&tok,10);
        }
        uint32_t headerLen = (tok - (uint8_t*) data + 1 + ((txt[cnt-1] == '\n' & txt[cnt-2]=='\r')?2:0)); // 5 + 1 + 2 (\r\n)

        //LOGI("cnt=%i headerLen=%i ", cnt, headerLen);
        //LOGI("data=%.8p  tok=%.8p ", data, tok);

        timer_drv_t tcp_rx_timeout;
        if (tok)
        {
                int32_t remain = cnt - headerLen;
                tok += 1;
                for (uint32_t i = 0; i < remain; i++)
                {
                        ring_buffer_queue(&tcp_rx_buf, *(tok+i));
                }
                remain = rxLen - remain;
                if(remain<0)
                    LOGI("remain=%i rxLen=%i", remain, rxLen);
                if(cnt>rxLen)
                {
                  LOGI("I say no");
                }

                Timer_Create(&tcp_rx_timeout, 16000);
                while (!Timer_Timeout(&tcp_rx_timeout) && remain)
                {
                        //LOGI("Need to get more %i",remain);
                        uint8_t c = 0;
                        if (uart_get_char(&c))
                        {
                                ring_buffer_queue(&tcp_rx_buf, c);
                                remain --;
                                // if (remain > 10000)
                                // {
                                //         while (1);
                                // }
                        }
                }
                if(remain>0)
                {
                    LOGI("Timeout, never let it happen, remain=%i",remain);
                    //while(1);
                }
                // printf("Received %lu of %lu bytes, tx buf size %lu \r\n", rxLen - remain, rxLen, ring_buffer_num_items(&tcp_rx_buf));
                // for (uint32_t i = 0; i < rxLen; i++)
                // {
                //         uint8_t c = 0;
                //         ring_buffer_peek(&tcp_rx_buf, &c, i);
                //         printf("%c", c);
                // }
                // printf("\r\n");
        }
        else
        {
                while (1);
        }
}

void esp32_at_init(void)
{
        uint8_t noauto = 0;
        ring_buffer_init(&tcp_rx_buf);
        uart_drv_buffer_clear();
        System_DelayMs(500); // Make sure module is stable
        memset(&at_cmd_q, 0x00, sizeof at_cmd_q);
        memset(&asyncState, 0x00, sizeof asyncState);
        esp32_at_cmdSync(AT_CMD_AT, NULL, 0, NULL, NULL, 0);
        uart_drv_buffer_clear();
        esp32_at_cmdSync(AT_CMD_GET_VERSION, NULL, 0, _at_get_version_cb, NULL, 0);
        esp32_at_cmdSync(AT_CMD_CW_MODE3, NULL, 0, NULL, NULL, 0);
        if (esp32_at_cmdSync(AT_CMD_AP_CIPMUX, NULL, 0, NULL, NULL, 0)) {
                cipmux = 1;
        }
        uart_drv_buffer_clear();
        esp32_at_cmdSync(AT_CMD_GET_STA_MAC, NULL, 0, ESP32_MacHandle, NULL, 0);
        esp32_at_cmdSync(AT_CMD_AUTO_CONNECT, &noauto, 1, NULL, NULL, 0);
        uart_drv_buffer_clear();
}

//1.1.3_IEC_00001_(WROOM-32) - <MFR version><IEC tag><IEC build number><MFR added tag>
static void _at_get_version_cb(void* data, void* param)
{
        uint8_t* txt = (uint8_t*) data;

        if (strstr((const char *)data, "Bin version:"))
        {
                strcpy((char *)esp32_version, (const char *)txt + strlen("Bin version:"));
        }
}

uint32_t esp32_at_tcp_rx(uint8_t* c)
{
        static uint32_t bufSize = 0;
        bufSize =  ring_buffer_num_items(&tcp_rx_buf);
        return ring_buffer_dequeue(&tcp_rx_buf, (char *)c);
}

void ESP32_Main(void)
{
        esp32_at_rxProcess(NULL, NULL, NULL);
#if ASYNC_API
        esp32_at_asyncProcess();
#endif
        BLE_main();
        if (bt_has_data > 0 && ble_raw_cb != NULL) {
                bt_has_data = 0;
                ble_raw_cb(bt_raw_data, bt_raw_len);
        }
#if ESP32_WIFI_SOFTAP_ENABLE
        if (cipmux) {
                SoftApMain();
        }
#endif
}

static void ESP32_MacHandle(void* data, void* param)
{
        if (strstr((const char*) data, "+CIPSTAMAC:\""))
        {
                memcpy(esp32_mac_addr, (uint8_t*) data+strlen("+CIPSTAMAC:\""), 17);
        }
}

void ESP32_Init(void)
{
        // #20191011 dattl not enable change baud by default
        // const uint32_t baud = 2000000;
        // TODO : KHoaDD2
        // SystemCoreClockUpdate();
        Timer_Init();
        uart_hw_init();
        // if (ESP32_ChangeBaudRate(baud))
        // {
        //         uart_hw_reinit_baud(baud);
        // }
        esp32_at_init();
        #if ESP32_ENABLE_HTTP
        HTTPClient__init();
        NM__connection_check();
        #endif

        #if ESP32_BT_ENABLE
        BT_Startup();
        #endif
}

#if ASYNC_API
uint32_t esp32_at_cmd_asyncQueue(eAtCmd cmd, void* cmdData, atcbfn cbfn, void* param, void* resultData)
{
        for (uint32_t i = 0; i < CMD_QUEUE_DEPTH; i++)
        {
                if (at_cmd_q[i].cmd == 0
                        || at_cmd_q[i].cmdStatus == CMD_STATUS_DONE
                        || at_cmd_q[i].cmdStatus == CMD_STATUS_TIMEOUT) {
                        at_cmd_q[i].cmd = cmd;
                        at_cmd_q[i].param = param;
                        at_cmd_q[i].cmdStatus = 0;
                        at_cmd_q[i].cbFn = cbfn;
                        at_cmd_q[i].cmdData = cmdData;
                        at_cmd_q[i].completeCbData = resultData;
                }
        }
}

void esp32_at_asyncProcess(void)
{
        // FIXME Queue depth = 1
        atCmdQueue_t* cmd = &at_cmd_q[0];
        static timer_drv_t asyncTimeout;
        switch (cmd->cmdStatus)
        {
                case CMD_STATUS_INIT:
                {
                        Timer_Create(&asyncTimeout, ESP32_AT_TIMEOUT_ASYNC_MS);
                        uint8_t* cmdText = getCmdTextByCmd(cmd->cmd, (uint8_t*)cmd->cmdData, NULL);
                        if (cmdText)
                        {
                                if (cmd->cmd == AT_CMD_CONNECT)
                                {
                                        gotIp = 0;
                                }
                                esp32_at_send(cmdText, strlen((const char*)cmdText));
                                cmd->cmdStatus = CMD_STATUS_WAIT_RESULT;
                                asyncState.state = 1; // In processing
                                asyncState.cbFn = cmd->cbFn;
                                asyncState.cbParam = cmd->param;
                                if (cmd->cmdData)
                                {
                                        free(cmd->cmdData);
                                }
                        }
                        break;
                }

                case CMD_STATUS_WAIT_RESULT:
                {
                        if (asyncState.state == 0) // DONE
                        {
                                cmd->cmdStatus = CMD_STATUS_DONE;
                                //memset(&at_cmd_q[0], 0x00, sizeof(atCmdQueue_t)); // DONE
                                if (asynccb)
                                {
                                        asynccb(cmd->completeCbData);
                                }
                        }
                        else if (Timer_Timeout(&asyncTimeout))
                        {
                                cmd->cmdStatus = CMD_STATUS_TIMEOUT;
                                if (asynccb)
                                {
                                        asynccb(cmd->completeCbData);
                                }
                        }
                        break;
                }

                case CMD_STATUS_DONE:
                case CMD_STATUS_TIMEOUT:
                        {
                                //memset(&at_cmd_q[0], 0x00, sizeof(atCmdQueue_t)); // DONE
                                break;
                        }
                default:
                {
                        break;
                }
        }
}

#endif

void ClearSendTcpIndication(void)
{
        tcpSendEnable = 0;
}

uint8_t GetTcpSendIndication(void)
{
        return tcpSendEnable;
}

uint8_t* ESP32_GetModuleFwVersion(void)
{
        return esp32_version;
}

// void ESP32_Bootmode(uint8_t mode)
// {
//         esp32_bootmode(mode);
// }

void Wifi_RegisterAsyncCompleteCallback(AsyncCbFn callbackfn)
{
        asynccb = callbackfn;
}

void Wifi_GetMacAddress(uint8_t* macAddr)
{
        if (macAddr)
        {
                strncpy((char*)macAddr, (const char *)esp32_mac_addr, 17);
        }
}

void ESP32_Sleep(uint32_t sleepTime)
{
        esp32_at_cmdSync(AT_CMD_DEEP_SLEEP, (uint8_t*) &sleepTime, sizeof sleepTime, NULL, NULL, 500);
}

void ESP32_IsBreakCmd(uint8_t isBreak)
{
  statusBreak = isBreak;
}

/*
* @author Dat Le
* @date 2019-08-26
* @brief Change ESP32 baudrate
* @param newbaud newbaudrate
* @retval true false
*/

uint8_t ESP32_ChangeBaudRate(uint32_t newbaud)
{
  if (esp32_at_cmdSync(AT_CMD_UART_CUR, (uint8_t*) &newbaud, sizeof newbaud, NULL, NULL, 0))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


void Wifi_SoftApControl(uint8_t enable)
{
        if (enable) {
                do {

                        if (!esp32_at_cmdSync(AT_CMD_AP_CIPSERVER, NULL, 0, NULL, NULL, 0)) {
                                break;
                        }
                } while (0);
        }
        else {
                do {
                        if (!esp32_at_cmdSync(AT_CMD_AP_CIPSERVER_STOP, NULL, 0, NULL, NULL, 0)) {
                                break;
                        }
                }
                while (0);
        }
}

#if ESP32_WIFI_SOFTAP_ENABLE


static void TcpReceiveMsg(VBUS_MSG* p_strtMIMsg, enumMsgRecState* p_eMsgRecState)
{
        uint8_t bValue;
        static uint16_t bMsgRecLen;
        static uint8_t bMsgCS;

        timer_drv_t t;

        Timer_Create(&t, 1000);

        while (*p_eMsgRecState != MSG_WAIT_PROCESSING)
        {
                if (esp32_at_tcp_rx(&bValue))
                {
                        // iTickrx = jiffies + MILISECOND_TO_TICK(MI_INTERBYTE_TIMEOUT);
                        switch (*p_eMsgRecState)
                        {
                        case MSG_WAIT_FOR_CMD:
                        case MSG_REC_SIGNATURE_1:
                        case MSG_WAIT_FOR_EXCEPTION:
                                if (bValue == MSG_SIGNATURE_1)
                                {
                                        bMsgCS = bValue;
                                        *p_eMsgRecState = MSG_REC_SIGNATURE_2;
                                }
                                else
                                {
                                         *p_eMsgRecState = MSG_WAIT_FOR_EXCEPTION;
                                }
                                break;

                        case MSG_REC_SIGNATURE_2:
                                p_strtMIMsg->MsgID = bValue;
                                bMsgCS += bValue;
                                *p_eMsgRecState = MSG_REC_LOW_BYTE_LEN;
                                break;

                        case MSG_REC_LOW_BYTE_LEN:
                                bMsgRecLen = 0;
                                p_strtMIMsg->MsgLen = bValue;
                                bMsgCS += bValue;

                                // #if 1
                                if (p_strtMIMsg->MsgLen > 0)
                                {
                                        *p_eMsgRecState = MSG_REC_DATA;
                                }
                                else
                                {
                                        *p_eMsgRecState = MSG_REC_CHECKSUM;
                                }
                                // #else
                                // *p_eMsgRecState = MSG_REC_HIGH_BYTE_LEN;
                                // #endif

                                break;

                        case MSG_REC_HIGH_BYTE_LEN:
                        {
                          p_strtMIMsg->MsgLen |= bValue << 8;
                          bMsgCS += bValue;
                          if (p_strtMIMsg->MsgLen > 0)
                          {
                                  *p_eMsgRecState = MSG_REC_DATA;
                          }
                          else
                          {
                                  *p_eMsgRecState = MSG_REC_CHECKSUM;
                          }
                          break;
                        }

                        case MSG_REC_DATA:
                                p_strtMIMsg->MsgParamBuf[bMsgRecLen] = bValue;
                                bMsgCS += bValue;
                                bMsgRecLen++;

                                if (bMsgRecLen < p_strtMIMsg->MsgLen)
                                {
                                        /*Continue receive until get full msg data*/
                                }
                                else
                                {
                                        *p_eMsgRecState = MSG_REC_CHECKSUM;
                                }
                                break;

                        case MSG_REC_CHECKSUM:
                                if (bMsgCS != bValue) /*Wrong check sum*/
                                {
                                        *p_eMsgRecState = MSG_WAIT_FOR_CMD;
                                }
                                else
                                {
                                        *p_eMsgRecState = MSG_WAIT_PROCESSING;
                                }
                                break;

                        case MSG_WAIT_PROCESSING:
                        default:
                                break;
                        }
                }
                else
                {
                        if (*p_eMsgRecState != MSG_WAIT_FOR_CMD)
                        {
                          if (Timer_Timeout(&t))
                          {
                                *p_eMsgRecState = MSG_WAIT_FOR_CMD;
                                break;
                          }
                        }
                        else
                        {
                                /* Not receive any thing */
                                break;
                        }
                        esp32_at_rxProcess(NULL, NULL, NULL); // Fix bug multi frame dattl #May072019
                }

        }
}

static void TcpMsgProcess(void)
{
        static enumMsgRecState eMsgRecState = MSG_WAIT_FOR_CMD;

        TcpReceiveMsg(&g_strtMsgInTcp, &eMsgRecState);
        if (eMsgRecState == MSG_WAIT_PROCESSING)
        {
                if (tcpCmdCbFn) {
                tcpCmdCbFn(&g_strtMsgInTcp);
        }
                eMsgRecState = MSG_WAIT_FOR_CMD;
        }
}

void SoftApMain(void)
{
        atCmdQueue_t* cmd = &at_cmd_q[1];
        if (cmd->cmdStatus != CMD_STATUS_WAIT_RESULT)
        {
                TcpMsgProcess();
        }
}

void Wifi_SoftApRegisterCallback(void* fn)
{
        tcpCmdCbFn = (BleCmdCbFn) fn;
}

eBLE_STATUS Wifi_SoftApTcpBleSend(uint8_t msgId, uint16_t msgLen, uint8_t* buffer)
{
  eBLE_STATUS result = eBLE_SUCCESS;

  MI_SendTcpMsg(msgId+1, buffer, msgLen, 2); // 2: legacy logic compatible
  return result;
}

static uint32_t MI_SendTcpMsg(msgID msgId, uint8_t* data, uint16_t len, uint8_t isStatus)
{
  uint32_t result = 0;
  uint32_t sizeIdx = 0;
  uint8_t done = 0;
  uint32_t sent = 0;
  uint8_t chkSum = 0;
  uint8_t hdr;

  if (isStatus == 1) { // #20191011 Fix issue where data is 0xC2xx, hdr will be wrong
    hdr = DEVICE_APP_STATUS_HEADER;
  }
  else {
    hdr = DEVICE_APP_DATA_HEADER;
  }

  /* #20151126 dattql Workaround changing header for status/notification */
  if (isStatus == 1 || isStatus == 0) {
    ;
  }
  else if (len == 0)
  {
    hdr = DEVICE_APP_STATUS_HEADER;
  }
  else if (len == 2) //  || len == 4) // #Jun142019 Only len 2 as DD
  {
    if (msgId == MSG_ID_BUSY
        || data[0] == MSG_ID_ACK
        || data[0] == MSG_ID_ERR
        || data[0] == MSG_ID_WAIT_MULTI)
    {
      hdr = DEVICE_APP_STATUS_HEADER;
    }
  }

  txBuffer[sizeIdx++] = hdr;
  txBuffer[sizeIdx++] = (uint8_t) msgId;
  txBuffer[sizeIdx++] = len & 0xff;
  txBuffer[sizeIdx++] = (len >> 8) & 0xff;

  chkSum = chkSumCalc(txBuffer, sizeIdx, 1); /* Calculate header CheckSum */

  while (len-- > 0)
  {
    txBuffer[sizeIdx] = data[sent++];
    chkSum += txBuffer[sizeIdx];
    sizeIdx ++;
    if (sizeIdx == TX_BUFFER_SIZE)
    {
      WiFiClient__send_all((const char*)txBuffer, sizeIdx);
      sizeIdx = 0;
    }
  }

  /* Last frame (with checksum) */
//  txBuffer[sizeIdx] = data[sent++];
//  chkSum += txBuffer[sizeIdx];
//  sizeIdx ++;
  txBuffer[sizeIdx++] = chkSum;
  WiFiClient__send_all((const char*)txBuffer, sizeIdx);
  return result;
}

#endif

void ESP32_Sync(void)
{
    BLE_UpdateConnection();
    cipmux = 1;
}
