/******************************************************************************=
================================================================================
INNOVA ELECTRONICS VIETNAM
Filename: MI.h
Description: Message Interface prototype
Layer:
Accessibility:
================================================================================
*******************************************************************************/

#ifndef __VBUS_INC_H__
#define __VBUS_INC_H__
#include <stdint.h>


#define MSG_IN_DATA_MAX_LEN    512
#define MSG_OFFSET             0

#pragma pack(1)
typedef struct _VBUS_MSG
{
    uint8_t MsgID;
    uint8_t MsgLen;
    uint8_t MsgParamBuf[MSG_IN_DATA_MAX_LEN];
} VBUS_MSG;

typedef struct _VBUS_OUT_MSG
{
    uint8_t MsgID;
    uint16_t MsgLen;
    uint8_t* MsgParamBuf;
} VBUS_OUT_MSG;
#pragma pack()
#endif
