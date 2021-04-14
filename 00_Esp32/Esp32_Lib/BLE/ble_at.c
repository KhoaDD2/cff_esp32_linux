#include "inc/ble.h"
#include "inc/VBUS_Inc.h"
#include "stdbool.h"

#include "esp32_at.h"
#include "ble_at_support.h"

#include "timer_drv.h"

#define BLE_NAME "CarScanTool"

uint8_t ble_name[18] = BLE_NAME;
extern atCmdQueue_t at_cmd_q[];

static void _ble_disco_prime_cb(void* data, void* param);
static void _ble_disco_char_cb(void* data, void* param);
static void _ble_connect_cb(void* data, void* param);
static void _ble_scan_cb(void* buf, void* param);
static void getModAddrHandle(void* data, void* param);
static int32_t AddDeviceToList(uint8_t* bdaddr, uint8_t* name, uint8_t nameLen, uint8_t type, int8_t rssi);
static uint8_t GetAdvDataByType(uint8_t adv_type, uint8_t adv_data_len, uint8_t* adv_data, uint8_t* out_buf, uint8_t* out_len);
static void ResetDeviceList(uint32_t maxDev);

//extern uint8_t ble_name[];
static VBUS_MSG g_strtMsgIn;
#define TX_BUFFER_SIZE                    (TX_MAX_LEN + 20)
#define BLE_FRAME_LEN                     (20)
#define TX_MAX_LEN                        (BLE_FRAME_LEN * 5)
static uint8_t txBuffer[TX_BUFFER_SIZE];
static uint8_t ble_module_mac[13] = {0x00};
static uint8_t bleInit = 0;

static bt_deviceInfo_t* listPtr = NULL;
static uint8_t maxDev = 0;
static uint8_t numOfDev = 0;
static uint8_t bleConnected = 0;
static uint8_t bleCentralConnected = 0;
static int BLE_SendMessage(uint8_t* data, uint32_t length, uint8_t service);
static uint32_t ble_get_char(uint8_t * bChar);
static void receiveMsg(VBUS_MSG* p_strtMIMsg, enumMsgRecState* p_eMsgRecState);
static void MsgProcess(void);
static uint32_t BLE_ESP32_PeriphStart(void);
static uint32_t MI_SendMsg(msgID msgId, uint8_t* data, uint16_t len, uint8_t isStatus);
static uint32_t MI_SendMsgVCI(msgID msgId, uint8_t* data, uint16_t len, uint8_t* vciBuf, uint8_t vciLen);
static uint32_t BLE_UpdateLocalName(void);
static uint32_t BLE_UpdateScanResp(void);
static void bleInitCheck(void * data, void * param);

ble_raw_cb_t ble_raw_cb;
BLE_ConnectedCbFn connectCbFn = NULL;
BleCmdCbFn cmdCbFn = NULL;
uint8_t p_bBleRxRingBuffer[256];
static volatile uint8_t* p_bBleRxHeadPtr = p_bBleRxRingBuffer;
static volatile uint8_t* p_bBleRxTailPtr = p_bBleRxRingBuffer;
static volatile uint32_t overrun = 0;
static ble_mode_t bleMode = BLE_MODE_PERIPHERAL;

static bleBroadCastID_t currentCustomerId = eId5XXX;
uint8_t ble_adv_data[31*2+1] = "02010A020A040302F0181209426C7565546F6F7468205072696E746572";
#define BLE_ADV_DATA_PREFIX "02010A020A0403020D18"
uint8_t ble_scan_rsp_data[31*2+1] = "05FF20170000";
#define BLE_SCAN_RESP_DATA_HOLDER    "05FF2017%02x%02x"

uint8_t bleNotifyEnabled = 0;

/*
* @brief Initialize the BLE module
* @author Dat Le
* @date 20150617
* @note !!! Call this function first !!!
*/
eBLE_STATUS BLE_Startup(void)
{
  bleMode = BLE_MODE_PERIPHERAL;
  if (BLE_ESP32_PeriphStart())
  {
    return eBLE_SUCCESS;
  }
  else
  {
    return eBLE_INIT_FAIL;
  }
}

static uint32_t BLE_UpdateScanResp(void)
{
  uint32_t result = 0;
  memset(ble_scan_rsp_data, 0x00, sizeof ble_scan_rsp_data);
  snprintf((char *)ble_scan_rsp_data, 32*2, BLE_SCAN_RESP_DATA_HOLDER, currentCustomerId & 0xff, (currentCustomerId>>8) & 0xff);

  return result;
}

static uint32_t BLE_UpdateLocalName(void)
{
  uint32_t result = 0;

  uint8_t nameLen = strlen((const char *)ble_name); // Len placeholder
  uint8_t adv_len = 0;
  memset(ble_adv_data, 0x00, sizeof ble_adv_data);
  snprintf((char *)ble_adv_data, 32*2, "%s%02x%02x", BLE_ADV_DATA_PREFIX, nameLen+1, 9); // w NameLen & Local name type follows
  // snprintf(&ble_adv_data[strlen(ble_adv_data)], 31*2, "%02x", nameLen+1);
  // snprintf(&ble_adv_data[strlen(ble_adv_data)], 31*2, "%02x", 9); // LOCAL NAME
  for (uint8_t i = 0; i < nameLen; i++)
  {
    snprintf((char *)&ble_adv_data[strlen((const char *)ble_adv_data)], 31*2, "%02x", ble_name[i]);
  }

  return result;
}

static uint32_t BLE_ESP32_PeriphStart(void)
{
  uint32_t result = 0;
  do
  {
    if (!esp32_at_cmdSync(AT_CMD_BLE_INIT, (uint8_t *)"2", sizeof("2"), NULL, NULL, 0))
    {
      break;
    }

    if (!esp32_at_cmdSync(AT_CMD_BLE_INIT, (uint8_t *)"0", sizeof("0"), NULL, NULL, 0))
    {
      break;
    }

    if (!esp32_at_cmdSync(AT_CMD_BLE_INIT, (uint8_t *)"2", sizeof("2"), NULL, NULL, 0))
    {
      break;
    }
    else
    {
      InitGetBLEAddr();
    }

    /* Gatt services */
    if (!esp32_at_cmdSync(AT_CMD_BLE_GATT_CRE, NULL, 0, NULL, NULL, 0))
    {
      break;
    }

    if (!esp32_at_cmdSync(AT_CMD_BLE_GATT_START, NULL, 0, NULL, NULL, 0))
    {
      break;
    }

    BLE_UpdateLocalName();
    BLE_UpdateScanResp();
    if (!esp32_at_cmdSync(AT_CMD_BLE_SCAN_RSP_DATA, (uint8_t*) ble_scan_rsp_data, strlen((const char *)ble_scan_rsp_data), NULL, NULL, 0))
    {
      break;
    }

    if (!esp32_at_cmdSync(AT_CMD_BLE_ADV_DATA, (uint8_t*) ble_adv_data, strlen((const char *)ble_adv_data), NULL, NULL, 0))
    {
      break;
    }

    if (!esp32_at_cmdSync(AT_CMD_BLE_ADV_START, NULL, 0, NULL, NULL, 0))
    {
      break;
    }

    if (!esp32_at_cmdSync(AT_CMD_BLE_SPP_CFG, NULL, 0, NULL, NULL, 0)) {
      break;
    }

    result = 1;
  }
  while (0);

  return result;
}

/*
* @func BLE_SendTransVCIMsgBuffer
* @brief Send VCI message via BLE with buffer provided
* @retval
*     eBLE_SUCCESS: Message sent
*     fail otherwise
*/
eBLE_STATUS BLE_SendTransVCIMsgBuffer(uint8_t msgId, uint16_t msgLen, uint8_t* buffer, uint8_t statusVci)
{
  eBLE_STATUS result = eBLE_SUCCESS;
  if (bleNotifyEnabled)
  {
    uint8_t vciHdr[5] = {0x00};
    vciHdr[0] = msgLen;
    vciHdr[1] = (uint8_t) (msgLen >> 8);
    vciHdr[2] = (uint8_t) (msgLen >> 16);
    vciHdr[3] = (uint8_t) (msgLen >> 24);
    vciHdr[4] =  statusVci; //User Input

    MI_SendMsgVCI(msgId+1, buffer, msgLen, vciHdr, 5);
  }
  return result;
}

/*
* @func BLE_SendMsgBuffer
* @brief Send message via BLE with buffer provided
* @retval
*     eBLE_SUCCESS: Message sent
*     fail otherwise
*/
eBLE_STATUS BLE_SendMsgBuffer(uint8_t msgId, uint16_t msgLen, uint8_t* buffer)
{
  eBLE_STATUS result = eBLE_SUCCESS;
  if (bleNotifyEnabled)
  {
    MI_SendMsg(msgId+1, buffer, msgLen, 2); // 2: legacy logic compatible
  }
  else
  {
    result = eBLE_FAIL;
  }
  return result;
}

eBLE_STATUS BLE_SendMsgBufferV2(uint8_t msgId, uint16_t msgLen, uint8_t* buffer, uint8_t isStatus)
{
  eBLE_STATUS result = eBLE_SUCCESS;
  if (bleNotifyEnabled)
  {
    MI_SendMsg(msgId+1, buffer, msgLen, isStatus);
  }
  else
  {
    result = eBLE_FAIL;
  }
  return result;
}

static uint32_t MI_SendMsgVCI(msgID msgId, uint8_t* data, uint16_t len, uint8_t* vciBuf, uint8_t vciLen)
{
  uint32_t result = 0;
  uint32_t sizeIdx = 0;
  uint8_t done = 0;
  uint32_t sent = 0;
  uint8_t chkSum = 0;

  uint8_t hdr = DEVICE_APP_DATA_HEADER;

  txBuffer[sizeIdx++] = hdr;
  txBuffer[sizeIdx++] = (uint8_t) msgId;
  txBuffer[sizeIdx++] = (len+vciLen) & 0xff;
  txBuffer[sizeIdx++] = ((len+vciLen) >> 8) & 0xff;
  for (uint32_t i = 0; i < vciLen; i++)
  {
    txBuffer[sizeIdx++] = vciBuf[i];
  }
  /* Checksum calc #1 */
  chkSum = chkSumCalc(txBuffer, sizeIdx, 1); /* Calculate header CheckSum */

  while (len-- > 1)
  {
    if (sizeIdx == BLE_FRAME_LEN)
    {
      BLE_SendMessage(txBuffer, sizeIdx, 1);
      sizeIdx = 0;
    }

    txBuffer[sizeIdx] = data[sent++];
    chkSum += txBuffer[sizeIdx];
    sizeIdx ++;
  }

  /* Last frame (with checksum) */
  txBuffer[sizeIdx] = data[sent++];
  chkSum += txBuffer[sizeIdx];
  sizeIdx ++;
  txBuffer[sizeIdx++] = chkSum;
  BLE_SendMessage(txBuffer, sizeIdx, 1);
  return result;
}

static uint32_t MI_SendMsg(msgID msgId, uint8_t* data, uint16_t len, uint8_t isStatus)
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
    if (sizeIdx == BLE_FRAME_LEN)
    {
      BLE_SendMessage(txBuffer, sizeIdx, 1);
      sizeIdx = 0;
    }
  }

  /* Last frame (with checksum) */
//  txBuffer[sizeIdx] = data[sent++];
//  chkSum += txBuffer[sizeIdx];
//  sizeIdx ++;
  txBuffer[sizeIdx++] = chkSum;
  BLE_SendMessage(txBuffer, sizeIdx, 1);
  return result;
}

uint8_t chkSumCalc(uint8_t* data, uint32_t len, uint8_t reset)
{
  static uint8_t result = 0;
  if (reset)
  {
    result = 0;
  }
  for (uint32_t i = 0; i < len; i++)
  {
    result += data[i];
  }
  return result;
}

/*
* @brief Main loop, to run BLE Request processing. Always run this routine when using BLE module.
* @author Dat Le
*/
int BLE_main(void)
{
  atCmdQueue_t* cmd = &at_cmd_q[0];
  if (cmd->cmdStatus != CMD_STATUS_WAIT_RESULT)
  {
    MsgProcess();
  }
  return 0;
}

/*
* @brief Change BLE module name
* @param [in] Name
* @reval eBLE_SUCCESS / eBLE_FAIL
*/
eBLE_STATUS BLE_ChangeModuleName(uint8_t* newName)
{
  eBLE_STATUS status = eBLE_SUCCESS;

  memset(ble_name, 0x00, sizeof ble_name);
  strncpy((char *)ble_name, (char *)newName, 17);

  return status;
}

static int BLE_SendMessage(uint8_t* data, uint32_t length, uint8_t service)
{
  int result = 0;
  eAtCmd cmd = AT_CMD_BLE_NOTIFY_INIT;
  if (service == 2) {
    /* ELM327 FFF0 service */
    cmd = AT_CMD_BLE_NOTIFY_INIT_FFF0;
  }
  if (esp32_at_cmdSync(cmd, (uint8_t*) data, length, NULL, NULL, 0))
  {
    printf("Sending %lu bytes\r\n", length);
    result = (int) esp32_at_cmdSync(AT_CMD_BLE_NOTIFY_SEND_DATA, (uint8_t*) data, length, NULL, NULL, 0);
  }

  return result;
}

void BLE_InternalConnectionCb(uint8_t connected)
{
  bleConnected = connected;
  if (!connected)
  {
    BLE_UpdateBleAdvert();
    /* FIXME IMPORTANT RECURSIVE */
  }

  if (connectCbFn)
  {
    connectCbFn(bleConnected);
  }
}

void BLE_RegisterConnectedCallback(BLE_ConnectedCbFn fn)
{
  connectCbFn = fn;
}

void BLE_RegisterCommandCallBack(BleCmdCbFn fn)
{
  cmdCbFn = fn;
}

static void MsgProcess(void)
{
  static enumMsgRecState eMsgRecState = MSG_WAIT_FOR_CMD;

  receiveMsg(&g_strtMsgIn, &eMsgRecState);
  if (eMsgRecState == MSG_WAIT_PROCESSING)
  {
    if (cmdCbFn)
    {
      cmdCbFn(&g_strtMsgIn);
    }
    eMsgRecState = MSG_WAIT_FOR_CMD;
  }
}


static void receiveMsg(VBUS_MSG* p_strtMIMsg, enumMsgRecState* p_eMsgRecState)
{
//    uint32 iTickrx;
    uint8_t bValue;
    static uint16_t bMsgRecLen;
    static uint8_t bMsgCS;

    timer_drv_t t;

    Timer_Create(&t, 1000);

    // iTickrx = jiffies; /* khabv 20160205 fix stuck issue when jiffies > 0x7FFFFFFF */
    while (*p_eMsgRecState != MSG_WAIT_PROCESSING)
    {
        if (ble_get_char(&bValue))
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
                    // /*Call received notification*/
                    // if (notificationOps.pMI_RecvMsg)
                    // {
                    //     notificationOps.pMI_RecvMsg(p_strtMIMsg);
                    // }
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
                // /* Timeout in middle of receiving sequence */
                // if (TIME_PASS(iTickrx))
                // {
                //     *p_eMsgRecState = MSG_WAIT_FOR_CMD;
                //     break;
                // }
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


/*
* @brief Get a character in uart rx buf
*/
static uint32_t ble_get_char(uint8_t * bChar)
{
    if (p_bBleRxHeadPtr != p_bBleRxTailPtr)
    {
        *bChar = *p_bBleRxTailPtr++;
        if (p_bBleRxTailPtr >= p_bBleRxRingBuffer + sizeof(p_bBleRxRingBuffer))
        {
            p_bBleRxTailPtr = (volatile uint8_t*) p_bBleRxRingBuffer;
        }
        return 1;
    }
    return 0;
}

/*
* @author Dat Le
* @date 2019-08-10
* @brief peek a char in ble rx buf
* @param
* @retval true or false
*/
static uint32_t ble_peek_char(uint8_t* c)
{
  if (p_bBleRxHeadPtr != p_bBleRxTailPtr)
  {
    *c = *p_bBleRxTailPtr;
    return 1;
}
  return 0;
}

/*
* @author Dat Le
* @date 2019-08-10
* @brief Check if BLE RX buffer empty or nor
* @param none
* @retval 1 empty, 0 not empty
*/
uint8_t BLE_IsRxBufferEmpty(void)
{
  uint8_t c;
  return (ble_peek_char(&c) == 0);
}


void ble_rxCallback(uint8_t rxChar)
{
    /* Read one byte from the receive data register */
    if (p_bBleRxHeadPtr != p_bBleRxTailPtr - 1)
    {
        if ((p_bBleRxHeadPtr != p_bBleRxRingBuffer + sizeof(p_bBleRxRingBuffer) - 1)
            || (p_bBleRxTailPtr != p_bBleRxRingBuffer))
        {
            *p_bBleRxHeadPtr++ = rxChar;
            if (p_bBleRxHeadPtr >= p_bBleRxRingBuffer + sizeof(p_bBleRxRingBuffer))
            {
              p_bBleRxHeadPtr = (volatile uint8_t*) p_bBleRxRingBuffer;
            }
        }
        else
        {
            overrun++;
        }
    }
    else
    {
        overrun++;
    }
}

uint8_t* BLE_GetCurrentCmd(void)
{
  return (uint8_t*) &g_strtMsgIn;
}

/*
* @brief Get module MAC
* @param [in] buffer to store MAC. 12 char address.
* @note Only call this API after module startup.
* @retval eBLE_SUCCESS / eBLE_FAIL
*/
eBLE_STATUS BLE_GetModuleMac(uint8_t* buf)
{
    eBLE_STATUS status = eBLE_FAIL;

    if (ble_module_mac[0])
    {
        memcpy(buf, ble_module_mac, sizeof ble_module_mac);
        status = eBLE_SUCCESS;
    }
    else
    {
       ;
    }

    return status;
}

void InitGetBLEAddr(void)
{
  memset(ble_module_mac, 0x00, sizeof ble_module_mac);
  esp32_at_cmdSync(AT_CMD_BLE_GET_ADDR, NULL, 0, getModAddrHandle, NULL, 0);
}

static void getModAddrHandle(void* data, void* param)
{
  uint8_t* text = (uint8_t*) data;
  if (strstr((char *)text, "+BLEADDR:"))
  {
    text = text + strlen("+BLE_ADDR");
    for (uint32_t i = 0; i < 6; i++)
    {
      memcpy(ble_module_mac+i*2, text, 2);
      text = text + 3;
    }
  }
}

/*
* @brief Set customer broadcast ID
* @note Call this API before BLE_Startup.
* @retval eBLE_SUCCESS / eBLE_FAIL
*/
eBLE_STATUS BLE_SetCustomBroadcastID(bleBroadCastID_t newId)
{
  eBLE_STATUS result = eBLE_SUCCESS;

  currentCustomerId = newId;

  return result;
}

/*
* @brief Update BLE Adv params and start broadcast again
* @author Dat Le
*/
eBLE_STATUS BLE_UpdateBleAdvert(void)
{
  eBLE_STATUS result = eBLE_SUCCESS;
  BLE_UpdateLocalName();
  BLE_UpdateScanResp();
  if (bleConnected)
  {
    // Do nothing
    ;
  }
  else
  {
    esp32_at_cmdSync(AT_CMD_BLE_ADV_STOP, NULL, 0, NULL, NULL, 0);
    do
    {
      if (!esp32_at_cmdSync(AT_CMD_BLE_SCAN_RSP_DATA, (uint8_t*) ble_scan_rsp_data, strlen((char *)ble_scan_rsp_data), NULL, NULL, 0))
      {
        break;
      }
      if (!esp32_at_cmdSync(AT_CMD_BLE_ADV_DATA, (uint8_t*) ble_adv_data, strlen((char *)ble_adv_data), NULL, NULL, 0))
      {
        break;
      }
      if (!esp32_at_cmdSync(AT_CMD_BLE_ADV_START, NULL, 0, NULL, NULL, 0))
      {
        break;
      }
      result = eBLE_SUCCESS;
    }
    while (0);
  }

  return result;
}

#if ESP32_BT_CENTRAL_ENABLE
/* Printer mode */

// OLD ESP32 FW
//+BLESCAN246f28d46b76,-59,02010a020a0403020d180f09446f6e676c6530335f6461743236,,0
//0f09446f6e676c6530335f6461743236
//-59 rssi

// NEWESP32 FW
// +BLESCAN:32:e0:cb:70:24:d3,-71,1eff060001092002d1b54ca33c60e
uint32_t BLE_Central_StartScan(bt_deviceInfo_t* list, uint8_t maxDevice)
{
  uint32_t result = 0;
  listPtr = list;
  maxDev = maxDevice;
  ResetDeviceList(maxDev);

  if (isBleModeCentral())
  {
    ble_scan_info_t scanSpec;
    scanSpec.enable = 1;
    scanSpec.timeSecond = 5;
    if (esp32_at_cmdSync(AT_CMD_BLE_SCAN, (uint8_t*) &scanSpec, sizeof scanSpec, _ble_scan_cb, NULL, scanSpec.timeSecond*1000))
    {
      result = 1;
    }
  }

  return result;
}

// NEW ESP32 FW
// +BLESCAN:32:e0:cb:70:24:d3,-71,1eff060001092002d1b54ca33c60e
static void _ble_scan_cb(void* buf, void* param)
{
  if (strncmp("+BLESCAN", (char *)buf, strlen("+BLESCAN")) == 0)
  {
    uint8_t addr[6] = {0x00};
    uint8_t advHex[64] = {0x00};
    uint8_t idx = 0;
    uint8_t txtIdx = 0;
    uint8_t loop_time = 0;
    uint8_t btName[30] = {0x00};
    uint8_t nameLen = 0;
    char* endptr = NULL;
    uint8_t* text = (uint8_t* )buf;
    int8_t rssi = 0;
    rssi = strtol((char *)&text[27], NULL, 10);
    uint8_t* addrText = &text[9];
    for (uint32_t i = 0; i < 6; i++)
    {
      addr[i] = strtol((char *)addrText + (i*3), NULL, 16);
    }

    // Parsing ADV
    text = &text[31];

    do
    {
      uint8_t hexChar[3] = {0x00};
      memcpy(hexChar, &text[txtIdx*2], 2);
      uint8_t c = strtol((char *)hexChar, &endptr, 16);
      if ((uint8_t*)endptr == hexChar)
      {
        loop_time += 1;
        text = (uint8_t*) buf + 31 + txtIdx * 2 + 1; // #Apr. 15, 2020 By pass ',' to continue parsing hex
        txtIdx = 0;
        // endptr = NULL;
        //break;
      }
      else
      {
        advHex[idx++] = c;
        txtIdx += 1;
      }
    } while (idx < 64 && endptr != NULL && loop_time < 2);

    GetAdvDataByType(9, idx, advHex, btName, &nameLen);
    if (nameLen) // dattl workaround esp32 issue https://github.com/espressif/esp32-at/issues/225
    {
      AddDeviceToList(addr, btName, nameLen, 0, rssi);
    }
  }
}

bool isBleModeCentral(void)
{
  return (bleMode == BLE_MODE_CENTRAL);
}


int32_t BLE_SetWorkingMode(ble_mode_t newMode)
{
  int32_t status = 0;

  if (bleMode == newMode)
  {
    ;
  }
  else
  {
    do
    {
      // Disable RF
      if (!esp32_at_cmdSync(AT_CMD_BLE_INIT, (uint8_t *)"0", sizeof("0"), NULL, NULL, 0)) {
        break;
      }
      if (newMode == BLE_MODE_CENTRAL)
      {
        if (!esp32_at_cmdSync(AT_CMD_BLE_INIT, (uint8_t *)"1", sizeof("1"), NULL, NULL, 0)) {
          break;
        }
        if (!esp32_at_cmdSync(AT_CMD_BLE_SCAN_PARAM, 0, 0, NULL, NULL, 0)) {
          break;
        }
      }
      else // BLE_MODE_PERIPHERAL
      {
        if (!esp32_at_cmdSync(AT_CMD_BLE_INIT, (uint8_t *)"2", sizeof("2"), NULL, NULL, 0)) {
          break;
        }
      }

      status = 1;
      bleMode = newMode;
    }
    while (0);
  }

  return status;
}

static int32_t AddDeviceToList(uint8_t* bdaddr, uint8_t* name, uint8_t nameLen, uint8_t type, int8_t rssi)
{
  uint32_t result = 0;
  if (listPtr == NULL) result = -1;
  for (uint32_t i = 0; i < maxDev; i++)
  {
    if (memcmp(listPtr[i].bt_addr, bdaddr, 6) == 0)
    {
      result = -1; /* Already have */
      break;
    }
  }
  if (result == 0 && numOfDev < maxDev)
  {
    listPtr[numOfDev].bt_addr_type = type;
    listPtr[numOfDev].rssi = rssi;
    memcpy(listPtr[numOfDev].bt_name, name, nameLen);
    memcpy(listPtr[numOfDev].bt_addr, bdaddr, 6);
    numOfDev += 1;
  }
  else
  {
    result = -1;
  }
  return result;
}

static void ResetDeviceList(uint32_t maxDev)
{
  if (listPtr)
  {
    memset(listPtr, 0x00, maxDev * sizeof(bt_deviceInfo_t));
  }
  numOfDev = 0;
}

static uint8_t GetAdvDataByType(uint8_t adv_type, uint8_t adv_data_len, uint8_t* adv_data, uint8_t* out_buf, uint8_t* out_len)
{
  uint8_t index = 0;

  while (index < adv_data_len)
  {
    /* Advertising data fields: len, type, values */
    /* Check if field is complete local name and the lenght is the expected one for BlueNRG Chat MS */
    if (adv_data[index+1] == adv_type)
    {
      memcpy(out_buf, &adv_data[index+2], adv_data[index]-1);
      *out_len = adv_data[index];
      break;
    }
    else
    {
      /* move to next advertising field */
      index += (adv_data[index] +1);
    }
  }
  return index;

}

static uint8_t* printerSvcUuid = NULL;
static uint8_t* printerWriteUuid = NULL;
static uint8_t printerUuidLen = 2;
static bt_deviceProp_t printerDevProp;

static const uint8_t zkc5804_svc_uuid[] = {0xf0, 0x18};
static const uint8_t zkc5804_write_uuid[] = {0xf1, 0x2a};

// static const uint8_t munbyn_svc_uuid[] = {0xf0, 0x18};
// static const uint8_t munbyn_write_uuid[] = {0xf1, 0x2a};

/*
* @brief Connect to a printer
* @param device printer bt info
*        printer_type ZKC-5804 or SM-L200
*/
extern int32_t App_CentralConnectBT(bt_deviceInfo_t* device);
extern void uart_drv_buffer_clear(void);

int32_t BLE_Central_ConnectPrinter(bt_deviceInfo_t device, bt_printer_type printer_type)
{
  int32_t result = 0;
  if (printer_type == BT_PRINTER_ZKC_5804)
  {
    printerSvcUuid = (uint8_t*) zkc5804_svc_uuid;
    printerWriteUuid = (uint8_t*) zkc5804_write_uuid;
  }
  else if (printer_type == BT_PRINTER_MUNBYN)
  {
    printerSvcUuid = (uint8_t*) zkc5804_svc_uuid;
    printerWriteUuid = (uint8_t*) zkc5804_write_uuid;
  }
  else
  {
    result = -1;
  }

  result = App_CentralConnectBT((bt_deviceInfo_t *)&device);
  if (result)
  {
    // __no_operation(); /* Fail to connect to printer */
  }
  else
  {
    BLE_Central_Disconnect();
  }

  return result;
}

int32_t App_CentralConnectBT(bt_deviceInfo_t* device)
{
  int32_t result = 0;

  bleCentralConnected = 0;
  memset(&printerDevProp, 0x00, sizeof printerDevProp);
  if (esp32_at_cmdSync(AT_CMD_BLE_CONNECT, (uint8_t*) device, sizeof(bt_deviceInfo_t), _ble_connect_cb, NULL, 0))
  {
    timer_drv_t timeout;
    Timer_Create(&timeout, 5000);
    while (!Timer_Timeout(&timeout)) // Wait connected
    {
      if (bleCentralConnected) break;
    }

    if (bleCentralConnected) // Connected
    {
      // Filter and find needed characteristic
      // Discover primary services
      uart_drv_buffer_clear();
      esp32_at_cmdSync(AT_CMD_BLE_DISCPRIM, NULL, 0, _ble_disco_prime_cb, NULL, 3000);


      if (printerDevProp.valid)
      {
        ble_disco_svc_t discoSvc;
        discoSvc.bleDiscoSvc = printerDevProp.writeServiceIdx;
        printerDevProp.valid = 0;
        esp32_at_cmdSync(AT_CMD_BLE_DISCCHAR, (uint8_t*) &discoSvc, sizeof discoSvc, _ble_disco_char_cb, NULL, 3000);
      }

      if (printerDevProp.valid)
      {
        // Finish connecting printer
        result = 1;
      }
      else
      {
        // Disconnect current connection
        esp32_at_cmdSync(AT_CMD_BLE_DISCONNECT, NULL, 0, NULL, NULL, 3000);
      }
    }
  }

  return result;
}

/*
* @author Dat Le
* @date 2019-08-07
* @brief Central disconnect api
* @param none
* @retval boolean
*/
int32_t BLE_Central_Disconnect(void)
{
  int32_t result = 0;
  if (esp32_at_cmdSync(AT_CMD_BLE_DISCONNECT, NULL, 0, NULL, NULL, 0))
  {
    result = 1;
  }

  return result;
}

/*
* @author Dat Le
* @date 2019-08-07
* @brief Central print API
* @param data and len
* @retval boolean
*/
int32_t BLE_Central_Print(uint8_t* data, uint32_t data_len)
{
  int32_t result = 0;

  uint8_t len = 0;

  for (uint8_t i = 0; i < data_len;) // #Sep222019 Fix issue missing char
  {
    if ((data_len - i) <= 20)
    {
      len = data_len - i;
    }
    else
    {
      len = 20;
    }

    ble_write_prop_t writeProp;
    writeProp.devProp = &printerDevProp;
    writeProp.data = data+i;
    writeProp.dataLen = len;
    if (!esp32_at_cmdSync(AT_CMD_BLE_WRITE_CHAR_PREP, (uint8_t*) &writeProp, sizeof writeProp, NULL,  NULL, 0))
    {
      break;
    }
    else
    {
      esp32_at_cmdSync(AT_CMD_BLE_WRITE_CHAR, writeProp.data, writeProp.dataLen, NULL, NULL, 0);
    }

    i += len;
  }

  return result;
}

// AT+BLEGATTCPRIMSRV=0
// +BLEGATTCPRIMSRV:0,1,0x1800,1
// +BLEGATTCPRIMSRV:0,2,0x1801,1
// +BLEGATTCPRIMSRV:0,3,0x180A,1
// +BLEGATTCPRIMSRV:0,4,0x180F,1
// +BLEGATTCPRIMSRV:0,5,0x1803,1
// +BLEGATTCPRIMSRV:0,6,0x1802,1
// +BLEGATTCPRIMSRV:0,7,0x1804,1
// +BLEGATTCPRIMSRV:0,8,0x18F0,1
// +BLEGATTCPRIMSRV:0,9,0xE7810A7173AE499D8C15FAA9AEF0C3F2,1
// strlen("+BLEGATTCPRIMSRV:") // 17
static void _ble_disco_prime_cb(void* data, void* param)
{
  // Search for printer service
  uint8_t* text = (uint8_t*) data;
  if (printerSvcUuid == NULL) return;
  uint8_t prtSvcText[6] = {0x00};
  snprintf((char *)prtSvcText, 5, "%02X%02X,", printerSvcUuid[1], printerSvcUuid[0]);
  if (strncmp((const char *)text, "+BLEGATTCPRIMSRV:", 17) == 0)
  {
    //bt_deviceProp_t
    if (strstr((const char *)text+17, (const char *)prtSvcText) != NULL)
    {
      printerDevProp.valid = 1;
      printerDevProp.writeServiceIdx = 0xff & strtol((const char *)text+19, NULL, 10);
    }
  }
}

// AT+BLEGATTCCHAR=0,8
// +BLEGATTCCHAR:"char",0,8,1,0x2AF1,0x04
// +BLEGATTCCHAR:"desc",0,8,1,1,0x2902
// +BLEGATTCCHAR:"char",0,8,2,0x2AF0,0x30
// +BLEGATTCCHAR:"desc",0,8,2,1,0x2902
/*
* @author Dat Le
* @date 2019-08-07
* @brief Discover ble characteristic call back
* @param none
* @retval none
*/
static void _ble_disco_char_cb(void* data, void* param)
{
  if (printerWriteUuid == NULL) return;
  // FIXME This one just handle 2 bytes UUID
  uint8_t* text = (uint8_t*) data;
  uint8_t writeUuidText[6] = {0x00};
  snprintf((char *)writeUuidText, 5, "%02X%02X,", printerWriteUuid[1], printerWriteUuid[0]);
  //printerDevProp.valid = 0;
  if (strncmp((const char *)text, "+BLEGATTCCHAR:", 14) == 0)
  {
    if (strstr((const char *)text+14, (const char *)writeUuidText) != NULL)
    {
      printerDevProp.writeCharIdx = strtol((const char *)(text+25), NULL, 10);
      printerDevProp.valid = 1;
    }
  }
}


static void _ble_connect_cb(void* data, void* param)
{
  uint8_t* text = (uint8_t*) data;
  if (strncmp((const char *)text, "+BLECONN:", strlen("+BLECONN:")) == 0)
  {
    bleCentralConnected = 1;
    ESP32_IsBreakCmd(1);
  }
}
#endif

eBLE_STATUS BLE_EnableSPPMode(void)
{
  eBLE_STATUS ret = eBLE_FAIL;

  if (esp32_at_cmdSync(AT_CMD_BLE_SPP_ENABLE, NULL, 0, NULL, NULL, 0))
  {
    ret = eBLE_SUCCESS;
  }

  return ret;
}

eBLE_STATUS BLE_UpdateConnection(void)
{
  eBLE_STATUS ret = eBLE_SUCCESS;
  esp32_at_cmdSync(AT_CMD_BLE_CHECK_CONN, NULL, 0, NULL, NULL, 0);
  if (bleConnected) {
    bleNotifyEnabled = 1; // #20200331 assume enabled
  }

  return ret;
}

uint32_t BLE_IsInit(void)
{
  uint32_t ret = 0;
  esp32_at_cmdSync(AT_CMD_BLE_CHECK_INIT, NULL, 0, bleInitCheck, NULL, 0);
  if (bleInit) ret = 1;
  return ret;
}

static void bleInitCheck(void * data, void * param)
{
  uint8_t* text = (uint8_t*) data;
  if (strncmp((const char *)text, "+BLEINIT:0", strlen("+BLECONN:0")) == 0) {
    bleInit = 0;
  }
  else if (strncmp((const char *)text, "+BLEINIT:1", strlen("+BLECONN:1")) == 0
            || strncmp((const char *)text, "+BLEINIT:2", strlen("+BLECONN:2")) == 0) {
    bleInit = 1;
  }
  else {
    bleInit = 0;
  }
}

/*
* @date Oct 24, 2020
* @brief Send BLE message for FFE0 service
* @note For ELM327 support
* @author Dat le
* @param
* @retval
*/
eBLE_STATUS BLE_SendDataELM327(uint8_t* data, uint32_t length)
{
  BLE_SendMessage(data, length, 2);
}

/*
* @date Oct 24, 2020
* @brief Data callback for ELM327 commands
* @note For ELM327 support
* @author Dat le
* @param
* @retval
*/
void BLE_RegisterElm327DataCallBack(ble_raw_cb_t cb)
{
  ble_raw_cb = cb;
}
