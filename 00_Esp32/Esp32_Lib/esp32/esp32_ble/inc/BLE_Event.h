#pragma once
#ifndef _EVENTS_H
#define _EVENTS_H

typedef enum {
  EVT_NO_EVENT = 0,
  EVT_HARD_BRAKE,
  EVT_HARD_ACCEL,
  EVT_OBD_LINK_STATUS = 0x03,
  EVT_IGNITION = 0x04,

} Event_t;

/*
* @brief Send event to app
* @param [in] evt Event ID
* @param [in] evt_data pointer to event data buffer
* @param [in] size Event buffer size
* @reval eBLE_SUCCESS/eBLE_FAIL
*/
eBLE_STATUS BLE_EVT_NotifyEvent(Event_t evt, uint8_t* evt_data, uint8_t size);

#endif /*_EVENTS_H*/