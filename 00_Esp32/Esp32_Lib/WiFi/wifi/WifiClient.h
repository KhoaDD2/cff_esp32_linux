/*
  WiFiClient.cpp - Library for Arduino Wifi shield.
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

#ifndef WIFICLIENT_H
#define WIFICLIENT_H

//#include <Arduino.h>
// #include "Client.h"
#include <stdbool.h>
#include "IPAddress.h"
//#include "socket/include/socket_buffer.h"
//#include "WiFi101.h"

// typedef struct _WiFiClient
// {
//     // WiFiClient();
//     // WiFiClient(uint8_t sock, uint8_t parentsock = 0);
//     // WiFiClient(const WiFiClient& other);

//     uint8_t (*status)(void);

//     int (*connectSSL_ip)(IPAddress ip, uint16_t port);
//     int (*connectSSL_host)(const char* host, uint16_t port);
//     int (*connect_ip)(IPAddress ip, uint16_t port);
//     //int (*connect_host)(const char* host, uint16_t port);
//     size_t (*write_byte)(uint8_t);
//     size_t (*write_buf)(const uint8_t *buf, size_t size);
//     size_t (*write_str)(const char *str);
//     int (*send_all)(const char* data, int length);

//     int (*read_byte)(void);
//     int (*read_buf)(uint8_t *buf, size_t size);
//     int (*receive)(char* data, int length);
//     int (*receive_all)(char* data, int length);
//     void (*stop)(void);
//     void (*close)(void);

//     uint8_t (*connected)(void);
//     uint8_t (*is_connected)(void);

//     void (*set_blocking)(bool blocking, unsigned int timeout);

//     uint32_t _flag;

//     SOCKET _socket;
//     uint32_t _head;
//     uint32_t _tail;
//     uint8_t _buffer[SOCKET_BUFFER_TCP_SIZE];
//     bool _blocking;
//     uint32_t _timeout;
//     int (*available)(void);
//     int (*connect_host)(const char* host, uint16_t port, uint8_t opt);
//     int (*connect_raw)(IPAddress ip, uint16_t port, uint8_t opt, const uint8_t *hostname);
// } WiFiClient;
#ifdef __cplusplus
extern "C" {
#endif

void WiFiClient__init(void);
void WiFiClient__set_blocking(bool blocking, unsigned int timeout);
int WiFiClient__connectSSL_host(const char* host, uint16_t port);
int WiFiClient__connectSSL(IPAddress ip, uint16_t port);
int WiFiClient__connect_ip(IPAddress ip, uint16_t port);
int WiFiClient__connect_host(const char* host, uint16_t port, uint8_t opt);
int WiFiClient__connect_raw(IPAddress ip, uint16_t port, uint8_t opt, const uint8_t *hostname);
size_t WiFiClient__write_byte(uint8_t b);
size_t WiFiClient__write_buff(const uint8_t *buf, size_t size);
int WiFiClient__send_all(const char* data, int length);
int WiFiClient__available(void);
int WiFiClient__read_byte(void);
int WiFiClient__read_buff(uint8_t* buf, size_t size);
int WiFiClient__receive_all(char* data, int length);
int WiFiClient__receive(char* data, int length);
void WiFiClient__stop(void);
void WiFiClient__close(void);
uint8_t WiFiClient__connected(void);
uint8_t WiFiClient__is_connected(void);
bool WiFiClient__isNotClosed(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFICLIENT_H */
