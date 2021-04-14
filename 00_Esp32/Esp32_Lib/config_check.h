#ifndef _CONFIG_CHECK_H
#define _CONFIG_CHECK_H


#warning "======= CONFIG CHECK ========"
/*
* Verify MCU
*/
#if defined(STM32F2XX)
	#warning "Building for F2 MCU"
#elif defined(STM32F4XX)
	#warning "Building for F4 MCU"
#else
	#error "MCU not supported."
#endif

/*
* Async API
*/
#if (ASYNC_API)
	#warning "Async API enabled"
#endif

/*
* HTTP Client
*/
#if (ESP32_ENABLE_HTTP)
	#warning "HTTP Client enabled"
#endif

/*
* WiFi Manager
*/
#if (ESP32_WIFI_ENABLE)
	#warning "WiFi enabled"
#endif

/*
* SoftAP Client
*/
#if (ESP32_WIFI_SOFTAP_ENABLE)
	#warning "SoftAP API enabled"
#endif

/*
* BT Central Client
*/
#if (ESP32_BT_CENTRAL_ENABLE)
	#warning "BLE Central enabled"
#endif

/*
* BT SPP
*/
#if (ESP32_BT_ENABLE)
	#warning "BT Classic SPP enabled"
#endif


#warning "======= END CONFIG CHECK ========"

#endif /*_CONFIG_CHECK_H*/