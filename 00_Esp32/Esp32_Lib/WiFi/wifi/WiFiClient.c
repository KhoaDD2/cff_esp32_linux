/*
  cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
// #include "mbed.h"
//#include "mbed_toolchain.h"
//EXTERN_C
//	#include "socket/include/socket.h"
//	#include "driver/include/m2m_periph.h"
//EXTERN_C_END
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "esp32_at.h"
#include "timer_drv.h"
//#include <intrinsics.h>

//include "WiFi101.h"
#include "WifiClient.h"

#define INTERNAL_RX_TIMEOUT  4000

#define printf

#define IS_CONNECTED	(_flag & SOCKET_BUFFER_FLAG_CONNECTED)

static uint32_t internal_timeout = 0;
static at_tcp_host_info_t host_inf;
static void _wifi_at_dns(void* data, void* param);
static void _wifi_at_connect_tcp(void* data, void* param);
static void tcp_send_init_cb(void* data, void* param);

uint8_t __socket = 0;

// WiFiClient WIFIClient;

int WiFiClient__send_all(const char* data, int length)
{
  int result = 0;
  if (esp32_at_cmdSync(AT_CMD_AT, NULL, 0, NULL, NULL, 0))
  {
    uint8_t ret = 0;
    uint8_t retry = 3;
    ClearSendTcpIndication();
    while (retry--)
    {
      #if ESP32_WIFI_SOFTAP_ENABLE
      if (cipmux) {
        if (esp32_at_cmdSync(AT_CMD_TCP_SEND_MUX_INIT, (uint8_t*) data, length, tcp_send_init_cb, &ret, 5000))
        {
          break;
        }
      }
      else
      #endif
      if (esp32_at_cmdSync(AT_CMD_TCP_SEND_INIT, (uint8_t*) data, length, tcp_send_init_cb, &ret, 5000))
      {
        break;
      }
    }
    timer_drv_t sendTmr;
    Timer_Create(&sendTmr, 100);
    while (!GetTcpSendIndication() && !Timer_Timeout(&sendTmr))
    {
      esp32_at_rxProcess(NULL, NULL, NULL); // Receive data
    }

    if (GetTcpSendIndication())
    {
      // printf("Sending %lu bytes\r\n", length);
      result = (int) esp32_at_cmdSync(AT_CMD_TCP_SEND_DATA, (uint8_t*) data, length, NULL, NULL, 0);
    }
  }

  return result;
}

static void tcp_send_init_cb(void* data, void* param)
{
  uint8_t* result = (uint8_t*) param;
  uint8_t* text = (uint8_t*) data;

  if (strstr(text, ">"))
  {
    *result = 2;
  }
  else if (strstr(text, "busy p...\r\n"))
  {
    *result = 1;
  }

}

int WiFiClient__receive(char* data, int length)
{
  internal_timeout = 1000;
  return WiFiClient__read_buff(data, length);
}

void WiFiClient__set_blocking(bool blocking, unsigned int timeout)
{

}

void WiFiClient__init(void)
{

}


int WiFiClient__connectSSL_host(const char* host, uint16_t port)
{
	// return WiFiClient__connect_host(host, port, SOCKET_FLAGS_SSL);
}

int WiFiClient__connectSSL(IPAddress ip, uint16_t port)
{
	// return WiFiClient__connect_raw(ip, port, SOCKET_FLAGS_SSL, 0);
}

// int WiFiClient__connect_host(const char* host, uint16_t port)
// {
// 	return WiFiClient__connect(host, port, 0);
// }

int WiFiClient__connect_ip(IPAddress ip, uint16_t port)
{
	// return WiFiClient__connect_raw(ip, port, 0, 0);
}

int WiFiClient__connect_host(const char* host, uint16_t port, uint8_t opt)
{
  int32_t status = 0;
  if (NM__is_connected())
  {
    esp32_at_cmdSync(AT_CMD_DOMAIN, (uint8_t*) host, strlen(host), _wifi_at_dns, host_inf.ip, 0);
    snprintf(host_inf.port, 6, "%lu", port);
    printf("Connecting... ip: %s, port: %s\n", host_inf.ip, host_inf.port);

    if (cipmux) {
      esp32_at_cmdSync(AT_CMD_CONN_TCP_MUX, (void*) &host_inf, sizeof host_inf, _wifi_at_connect_tcp, &status, 0);
    }
    else {
      esp32_at_cmdSync(AT_CMD_CONN_TCP, (void*) &host_inf, sizeof host_inf, _wifi_at_connect_tcp, &status, 0);
    }
  }
  else
  {
    status = -1;
  }
}

/*
* @author Dat Le
* @brief at dns callback
*/
static void _wifi_at_dns(void* data, void* param)
{
  if (!param) return;
  if (strstr((char*) data, "+CIPDOMAIN") != NULL)
  {
    uint8_t buf[20] = {0x00};
    uint8_t idx = 0;
    uint8_t* txt = (uint8_t*) data + 11; // +CIPDOMAIN:
    do
    {
      buf[idx++] = *txt++;
    }
    while (*txt != '\r' && *txt != '\0');
    strncpy(param, buf, strlen(buf));
  }
}


static void _wifi_at_connect_tcp(void* data, void* param)
{
  if (!param) return;
  int32_t* status = param;
  if (strstr((char*) data, "CONNECT") != NULL)
  {
    __socket = 1;
    *status = 1;
  }
  else
  {
    *status = -1;
  }
}

int WiFiClient__connect_raw(IPAddress ip, uint16_t port, uint8_t opt, const uint8_t *hostname)
{
	// if ((WIFI__status() != WL_CONNECTED))
	// {
	// 	tr_err("Network is down");
	// 	return -1;
	// }
	// struct sockaddr_in addr;

	// // Initialize socket address structure:
	// addr.sin_family = AF_INET;
	// addr.sin_port = _htons(port);
	// addr.sin_addr.s_addr = ip._address.dword;

	// // Create TCP socket:
	// _flag = 0;
	// _head = 0;
	// _tail = 0;
	// if ((_socket = socket(AF_INET, SOCK_STREAM, opt)) < 0) {
	// 	return 0;
	// }

	// if (opt & SOCKET_FLAGS_SSL && hostname) {
	// 	setsockopt(_socket, SOL_SSL_SOCKET, SO_SSL_SNI, hostname, m2m_strlen((uint8_t *)hostname));
	// }

	// // Add socket buffer handler:
	// socketBufferRegister(_socket, &_flag, &_head, &_tail, (uint8 *)_buffer);

	// // Connect to remote host:
	// if (connectSocket(_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
 //        close(_socket); /* socket.h */
	// 	_socket = -1;
	// 	return 0;
	// }

	// // Wait for connection or timeout:
	// unsigned long start = millis();
	// while (!IS_CONNECTED && millis() - start < 40000) {
	// 	m2m_wifi_handle_events(NULL);
 //        nm_wait_ms(2);
	// }
	// if (!IS_CONNECTED) {
 //      close(_socket);
	// 	_socket = -1;
	// 	return 0;
	// }

	// return 1;
}


size_t WiFiClient__write_byte(uint8_t b)
{
	// return WiFiClient__write_buff(&b, 1);
}

size_t WiFiClient__write_buff(const uint8_t *buf, size_t size)
{
// 	sint16 err;

// 	if (_socket < 0 || size == 0) {
// //		setWriteError(); /*TODO */
// 		return 0;
// 	}

// 	// Network led ON (rev A then rev B).
// 	// m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 0);
// 	// m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 0);

// 	m2m_wifi_handle_events(NULL);

// 	while ((err = send(_socket, (void *)buf, size, 0)) < 0) {
// 		// Exit on fatal error, retry if buffer not ready.
// 		if (err != SOCK_ERR_BUFFER_FULL) {
// //			setWriteError(); /*TODO */
// //			m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
// //			m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);
// 			return 0;
// 		}
// 		m2m_wifi_handle_events(NULL);
// 	}

// 	// Network led OFF (rev A then rev B).
// 	// m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
// 	// m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);

// 	return size;
}

int WiFiClient__available(void)
{
	// m2m_wifi_handle_events(NULL);

	// if (_socket != -1) {
	// 	return _head - _tail;
	// }
	// return 0;
}

int WiFiClient__read_byte(void)
{
	// uint8_t b;

	// if (WiFiClient__read_buff(&b, sizeof(b)) == -1) {
	// 	return -1;
	// }

	// return b;
}

int WiFiClient__read_buff(uint8_t* buf, size_t size)
{
	// // sizeof(size_t) is architecture dependent
	// // but we need a 16 bit data type here
	// uint16_t size_tmp = WiFiClient__available();

	// if (size_tmp == 0) {
	// 	return -1;
	// }

	// if (size < size_tmp) {
	// 	size_tmp = size;
	// }

	// for (uint32_t i = 0; i < size_tmp; ++i) {
	// 	buf[i] = _buffer[_tail++];
	// }

	// if (_tail == _head) {
	// 	_tail = _head = 0;
	// 	_flag &= ~SOCKET_BUFFER_FLAG_FULL;
	// 	recv(_socket, _buffer, SOCKET_BUFFER_MTU, 0);
	// 	m2m_wifi_handle_events(NULL);
	// }

	// return size_tmp;
  // printf("Input read size: %lu\r\n", size);
  timer_drv_t timeout;
  uint16_t cnt = 0;
  Timer_Create(&timeout, internal_timeout);
  do
  {
    uint8_t c;
    uint8_t res;

    c = 0;
    res = esp32_at_tcp_rx(&c);
    if (res)
    {
      // #20191113 dattl update time if has data
      Timer_Create(&timeout, internal_timeout);
      buf[cnt++] = c;
    }
    else
    {
      esp32_at_rxProcess(NULL, NULL, NULL); // Receive data
    }
  }
  while (!Timer_Timeout(&timeout) && cnt < size);
  //while (cnt < size);
  // printf("Output read size: %lu\r\n", cnt);

  return cnt;
}

int WiFiClient__receive_all(char* data, int length)
{
  // NOTE dat le Blocking
    // uint32_t total_read = 0;

    // // Wait for connection or timeout:
    // unsigned long start = millis();
    // while (IS_CONNECTED && length > 0 && millis() - start < _timeout)
    // {
    //     int nbr_read = WiFiClient__read_buff((uint8_t*)data, length);
    //     if (nbr_read > 0)
    //     {
    //         data += nbr_read;
    //         total_read +=nbr_read;
    //         length -= nbr_read;
    //     }
    // }
    // return total_read;
  internal_timeout = INTERNAL_RX_TIMEOUT;
  return WiFiClient__read_buff(data, length);
}

// int WiFiClient__peek()
// {
// 	if (!available())
// 		return -1;

// 	return _buffer[_tail];
// }

// void WiFiClient__flush()
// {
// 	while (available())
// 		read();
// }

void WiFiClient__stop(void)
{
	// if (_socket < 0)
	// 	return;

	// socketBufferUnregister(_socket);
	// close(_socket);
	// _socket = -1;
	// _flag = 0;
}

void WiFiClient__close(void)
{
  uint32_t status = 0;
  /* FIXME This does have handle message "CLOSED" */
  // System_DelayMs(50);
  if (cipmux) {
    esp32_at_cmdSync(AT_CMD_CONN_CLOSE_MUX, NULL, 0, NULL, &status, 0);
  }
  else {
    esp32_at_cmdSync(AT_CMD_CONN_CLOSE, NULL, 0, NULL, &status, 0);
  }
  __socket = 0;
  // FIXME WRONG CALL
  if (status == 1)
  {
    printf("Connection closed..\r\n");
  }
}
uint8_t WiFiClient__connected(void)
{
	// m2m_wifi_handle_events(NULL);
	// if (WiFiClient__available())
	// 	return 1;
	// return IS_CONNECTED;
}

uint8_t WiFiClient__is_connected(void)
{
    return 1; // FIXME real status here
    // return connected();
}

bool WiFiClient__isNotClosed(void)
{
	// return _socket != -1;
  return __socket == 1;
}

// uint8_t WiFiClient__status()
// {
// 	// Deprecated.
// 	return 0;
// }

//WiFiClient__operator bool()
//{
//	return _socket != -1;
//}
