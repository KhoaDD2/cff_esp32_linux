/*
  IPAddress.h - Base class that provides IPAddress
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

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

#ifndef IPAddress_h
#define IPAddress_h

#include <stdint.h>
#include <stddef.h>
//#include "Printable.h"
//#include "WString.h"

// A class to make it easier to handle and pass around IP addresses


typedef struct _IPAddress
{
    union {
	uint8_t bytes[4];  // IPv4 address
	uint32_t dword;
    } _address;

    // Access the raw byte array containing the address.  Because this returns a pointer
    // to the internal structure rather than a copy of the address this function should only
    // be used when you know that the usage of the returned uint8_t* will be transient and not
    // stored.
//    uint8_t* (*raw_address)(void);
//
//    void (*IPAddress__Constructor)();
//    void (*IPAddress__Constructor)(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
//    void (*IPAddress__Constructor)(uint32_t address);
//    void (*IPAddress__Constructor)(const uint8_t *address);
//
//    bool (*fromString)(const char *address);
} IPAddress;

int IPAddress__fromString(const char* address, IPAddress* ip);

#ifdef __IPADDRESS_C__ 
const IPAddress INADDR_NONE = {0,0,0,0};
#else
extern const IPAddress INADDR_NONE;
#endif

#endif
