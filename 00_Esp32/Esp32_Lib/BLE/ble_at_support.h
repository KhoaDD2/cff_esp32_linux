#ifndef _BLE_AT_SUPPORT_H
#define _BLE_AT_SUPPORT_H

#include <stdbool.h>
#include "inc/BLE_Type.h"

// typedef {
//   uint8_t is16BitUuid;
//   uint8_t writeService[16];
//   uint8_t writeChar[16];
// } bt_deviceProp_t;
typedef uint8_t msgID;

typedef struct {
  uint8_t valid;
  uint8_t bleIdx;
  uint8_t writeServiceIdx;
  uint8_t writeCharIdx;
} bt_deviceProp_t;

typedef enum _enumMsgRecState
{
  MSG_WAIT_FOR_CMD,
  MSG_REC_SIGNATURE_1,
  MSG_REC_SIGNATURE_2,
  MSG_REC_LOW_BYTE_LEN,
  MSG_REC_HIGH_BYTE_LEN,
  MSG_REC_DATA,
  MSG_REC_CHECKSUM,
  MSG_WAIT_PROCESSING,
  MSG_WAIT_FOR_EXCEPTION,
} enumMsgRecState;

#define MSG_SIGNATURE_1                 0xAD

#define DEVICE_APP_STATUS_HEADER        0xDD
#define DEVICE_APP_DATA_HEADER          0xDA
#define MSG_ID_BUSY                     0xC3
#define MSG_ID_ACK                      0xC1
#define MSG_ID_ERR                      0xC2
#define MSG_ID_WAIT_MULTI               0xC4

/*
* @author Dat Le.
*/
void BLE_InternalConnectionCb(uint8_t connected);

/*
* @author Dat Le
* @brief BLE rx callback
*/
void ble_rxCallback(uint8_t rxChar);

void InitGetBLEAddr(void);

/*
* @author Dat Le
* @date 2019-08-14
* @brief BLE is in central mode or note
* @param
* @retval
*/

bool isBleModeCentral(void);

uint8_t chkSumCalc(uint8_t* data, uint32_t len, uint8_t reset);

/*
* @brief RAW data callback
*/
extern ble_raw_cb_t ble_raw_cb;

#endif /*_BLE_AT_SUPPORT_H*/
