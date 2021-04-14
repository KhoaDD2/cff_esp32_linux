#pragma once
#include "BLE_Type.h"
#include <stdint.h>

/*
* @brief Each tick is 10 ms
*/
#define BLE_TIME_TICK_INTERVAL 10 /* ms */

/*
* @brief In case external counter is used. User must provide a tick counter that increase each time with
* interval is BLE_TIME_TICK_INTERVAL
*/
#define USE_EXTERNAL_COUNTER 1

/*
* @brief Register a user provided us-based delay function
* @fn
* @author Dat Le
* @date 20160407
* @param [in] fn Pointer to a us-based delay function
*/
void BLE_SystemRegisterDelayUs(DelayUsFn fn);

#if (USE_EXTERNAL_COUNTER)
/* TODO */
/*
* @brief Register a counter for BLE internal time keeping.
* @fn
* @author Dat Le
* @date 20160407
*/
void BLE_SystemRegisterTickCounter(uint32_t* tick_counter, uint32_t timebase);
#else
/*
* @brief Call back for BLE internal time keeping. Each call increase the tick by 1.
* @fn
* @author Dat Le
* @date 20160407
*/
void BLE_SystemTimeIntervalHandler(void);
#endif /* USE_EXTERNAL_COUNTER */

/**** IO */
/*
* @brief Register function for controlling BLE TX/RX
* @fn
* @author Dat Le
* @param [in] PinOutFn Pointer to BLE pin out control
* @param [in] PinReadFn Pointer to BLE pin read function
*/
void BLE_RegisterBLEPinCtl(BLE_PinOutFn PinOutFn, BLE_PinReadFn PinReadFn);

/**** LED */
/*
* @brief Register driver for BLE LED status
* @fn
* @author Dat Le
* @param [in] fn Pointer to driver function. Null to disable LED.
*/
void BLE_LED_RegisterLEDDriver(BLE_LED_DriveFn fn);


