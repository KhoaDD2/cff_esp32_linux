#include <stdint.h>
#include "inc\ble.h"
#include "inc\VBUS_Inc.h"
#include <intrinsics.h>
#include "wifi.h"


#if !defined(enumbool)
typedef uint8_t enumbool;
#define eFALSE 0
#define eTRUE 1
#endif

/* TO DISABLE TI BLE on Pro34 */
void PortDisableTIBLE(void);

enumbool isBLEConnected = eFALSE;
//uint8_t ble_name[18] = "CarScanTool";

void ProcessCmd(void* cmd);
void Port_Connected(uint8_t connectedStatus);
void ProcessTcpCmd(void* cmd);

#define BLE_SendMsg(msgid,len) BLE_SendMsgBuffer(msgid,len,BLE_GetMsgBuffer())

uint8_t testDataBuf[128];

static uint8_t* BLE_GetMsgBuffer(void)
{
    return testDataBuf;
}

static uint8_t dummy[] = "This is a dummy message payload 0123456789. It contains more than 20 characters.";
static uint16_t dummyUsbPid = 0x312; /* 3211a Dongle */
static uint8_t isConnected = 0;

void SoftApInitialization(void);

/* PRO14/34 BLE Init */
void BlueToothModule_Initialization(void);
void BlueToothModule_Initialization(void)
{
  PortDisableTIBLE();
  BLE_RegisterConnectedCallback(Port_Connected);
  BLE_RegisterCommandCallBack(ProcessCmd);
}

void Port_Connected(uint8_t connectedStatus)
{
  isBLEConnected = (enumbool) connectedStatus;
  if (isBLEConnected == eTRUE) {
      System_DelayMs(500);
      BLE_EnableSPPMode();
  }
}

void ProcessCmd(void* cmd)
{
    VBUS_MSG* msgIn = (VBUS_MSG*) (cmd);

    if (msgIn->MsgID == 0x99) /* Dummy MsgID */
    {
        uint8_t* msgBuf = BLE_GetMsgBuffer();
        memcpy(msgBuf, dummy, sizeof dummy & 0xffff);
        if (eBLE_SUCCESS == BLE_SendMsg(msgIn->MsgID, sizeof dummy & 0xffff))
        {
            ;
        }
        else
        {
            __no_operation();
        }
    }
    else if (msgIn->MsgID == 0xe7)
    {
        uint8_t* msgBuf = BLE_GetMsgBuffer();
        memcpy(msgBuf, &dummyUsbPid, sizeof dummyUsbPid);
        if (eBLE_SUCCESS == BLE_SendMsg(msgIn->MsgID, sizeof dummyUsbPid))
        {
            ; /* Succeed */
        }
        else
        {
            __no_operation(); /* Debug purpose */
        }
    }
    else
    {
        uint8_t* msgBuf = BLE_GetMsgBuffer();

        /*
        0xC1 ACK
        0xC2 NACK
        0xC3 Busy
        0xC4 Wait for next packet
        */
        msgBuf[0] = 0xC1;
        msgBuf[1] = 0x00;

        if (eBLE_SUCCESS == BLE_SendMsg(msgIn->MsgID, 2))
        {
            ;
        }
        else
        {
            __no_operation();
        }
    }
}


#if BLE_TI
void PortDisableTIBLE(void)
{
    /* BLE control pin initialization */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Disable BLE */
    GPIO_ResetBits(BLE_PORT_nRST, BLE_PIN_nRST);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = BLE_PIN_nRST;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(BLE_PORT_nRST, &GPIO_InitStructure);
    GPIO_ResetBits(BLE_PORT_nRST, BLE_PIN_nRST);
}
#else
void PortDisableTIBLE(void)
{
}
#endif

#if ESP32_WIFI_SOFTAP_ENABLE
void ProcessTcpCmd(void* cmd)
{
    VBUS_MSG* msgIn = (VBUS_MSG*) (cmd);

    if (msgIn->MsgID == 0x99) /* Dummy MsgID */
    {
        uint8_t* msgBuf = BLE_GetMsgBuffer();
        memcpy(msgBuf, dummy, sizeof dummy & 0xffff);
        if (eBLE_SUCCESS == Wifi_SoftApTcpBleSend(msgIn->MsgID, sizeof dummy & 0xffff, msgBuf))
        {
            ;
        }
        else
        {
            __no_operation();
        }
    }
    else if (msgIn->MsgID == 0xe7)
    {
        uint8_t* msgBuf = BLE_GetMsgBuffer();
        memcpy(msgBuf, &dummyUsbPid, sizeof dummyUsbPid);
        if (eBLE_SUCCESS == Wifi_SoftApTcpBleSend(msgIn->MsgID, sizeof dummyUsbPid, msgBuf))
        {
            ; /* Succeed */
        }
        else
        {
            __no_operation(); /* Debug purpose */
        }
    }
    else
    {
        uint8_t* msgBuf = BLE_GetMsgBuffer();

        /*
        0xC1 ACK
        0xC2 NACK
        0xC3 Busy
        0xC4 Wait for next packet
        */
        msgBuf[0] = 0xC1;
        msgBuf[1] = 0x00;

        if (eBLE_SUCCESS == Wifi_SoftApTcpBleSend(msgIn->MsgID, 2, msgBuf))
        {
            ;
        }
        else
        {
            __no_operation();
        }
    }
}

void SoftApInitialization(void)
{
    Wifi_SoftApRegisterCallback((void*) ProcessTcpCmd);
}
#endif
