#pragma once

#define BLE_SETTING_NAME (1 << 0)

/**** LED */
/*
* @enum enumLEDState
* @brief State of BLE Status LED
*/
typedef enum _led_state
{
    eLED_STATE_ON,
    eLED_STATE_OFF,
    eLED_STATE_TOGGLE,                  /* User defined led toggle pattern */
    eLED_STATE_SIZE,                    /* Just the size of enum */
} enumLEDState;

typedef void (*BLE_LED_DriveFn) (const enumLEDState);


/**** IO */
typedef enum _ble_pin_t
{
    eBLE_Pin_EN,            /* Enable/Disable module */
    eBLE_Pin_nRST,          /* Reset, active low */
    eBLE_Pin_RTS,           /* Ready to send */
    eBLE_Pin_All            /* Just ignore this*/
} eBLE_Pin;

typedef enum _ble_pin_state
{
    eBLE_Pin_State_Reset,   /* LOW */
    eBLE_Pin_State_Set,     /* HIGH */
} eBLE_Pin_State;

/*
* @brief BLE pin control function
* @fn
* @param [in] pin BLE Pin to change state
* @param [in] state Next state of the pin
*/
typedef void (*BLE_PinOutFn) (eBLE_Pin pin, eBLE_Pin_State state);

/*
* @brief BLE connected call back
* \This function get called when a connection is established.
* \Since it may be called from an interrupt, the routine should be short (e.g. a flag trigger).
* @fn
*/
typedef void (*BLE_ConnectedCbFn) (uint8_t);

/*
* @brief BLE pin control function
* @fn
* @param [in] pin BLE Pin to read state
* @retval State of the Pin
*/
typedef eBLE_Pin_State (*BLE_PinReadFn) (eBLE_Pin pin);


/**** SYSTEM */
typedef void (*DelayUsFn) (uint32_t);

/**** COM */
typedef enum
{
    eBLE_FAIL,
    eBLE_SUCCESS,
    eBLE_BAUD_FAIL,
    eBLE_INIT_FAIL,
    eBLE_NOT_SUPPORT,
} eBLE_STATUS;

typedef void (*BLE_UART_SendByteFn) (uint8_t);

/*
* @param [in] baud 9600, 115200...
*/
typedef void (*BLE_UART_HostBaudFn) (uint32_t);

/*
* #20160831 Command call back handle type
*/
typedef void (*BleCmdCbFn) (void*);

/*
* @brief Setting update callback
*/
typedef void (*BLE_SettingCbFn) (uint32_t changeId, uint8_t* data, uint32_t len);

/*
* @brief Raw data callback
*/
typedef void (*ble_raw_cb_t) (void* data, uint32_t len);

/*
* @brief Discovery device information
*/
typedef struct {
    uint8_t bt_addr[6];
    uint8_t bt_name[20];
    uint8_t bt_addr_type;
    int8_t rssi;
} bt_deviceInfo_t;

/*
* @brief Supported printer type
*/
typedef enum {
    BT_PRINTER_SM_L200 = 0,
    BT_PRINTER_ZKC_5804,
    BT_PRINTER_MUNBYN,
} bt_printer_type;

/*
* @brief BLE working mode type
*/
typedef enum {
    BLE_MODE_PERIPHERAL = 0,
    BLE_MODE_CENTRAL,
} ble_mode_t;
