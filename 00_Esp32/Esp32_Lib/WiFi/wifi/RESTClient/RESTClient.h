#ifndef __RESTCLIENT_H__
#define __RESTCLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include "BlobType.h"
#include "../HTTPClient.h"

#if 1
#define IEC_SERVER_HOST     "52.76.233.94"
#define IEC_SERVER_PORT     9000            /* Server US */
#else
#define IEC_SERVER_HOST     "52.76.233.94"
#define IEC_SERVER_PORT     9002            /* Server INT */
/* Web US 8080 INT 8082 */
#endif



#define LOCAL_SERVER_HOST    "10.182.152.109"
#define LOCAL_SERVER_PORT    8080

#define LAN_SERVER_HOST   "10.182.152.111"
#define LAN_SERVER_PORT   9000

// #define REST_HOST         TEST_SERVER_HOST
// #define REST_PORT         TEST_SERVER_PORT

#define REST_HOST         IEC_SERVER_HOST
#define REST_PORT         IEC_SERVER_PORT

// #define REST_HOST            LOCAL_SERVER_HOST
// #define REST_PORT            LOCAL_SERVER_PORT




#ifdef __cplusplus
extern "C" {
#endif

void RESTClient__init(char* host, uint32_t port);

void RESTClient__set_host(char* host, uint32_t port);

char* RESTClient__get_token(const char* deviceid);

uint32_t RESTClient__get_oe_item_name(const char* vin, uint16_t* p_itemList);

HTTPResult RESTClient__get_profile_crc(const char* vin, uint32_t* p_crc);

HTTPResult RESTClient__download_profile(const char* vin, IHTTPDataIn* writer);

HTTPResult RESTClient__download(const char* url, IHTTPDataIn* writer);

HTTPResult RESTClient__track(const char* url, uint8_t* content, size_t content_size, const char* tracking_host);

bool RESTClient__set_token(const char* token);

bool RESTClient__set_header(const char* key, const char* value);

HTTPResult RESTClient__get_ota_upgrade_url(const char* platform, BLOBType_t type, const char* cur_version, char** download_url);
//03Mar2021 DaoNg added to test diagnostic server on ESP32S2
HTTPResult RESTClientDiag__get_years(const char* make);
HTTPResult RESTClientDiag__get_tsbs_recalls(const char* id);
HTTPResult RESTClientDiag__change_pass(const char* oldPass, const char* pass);
HTTPResult RESTClientDiag__create_report(const char* strVehicleRawData);
HTTPResult RESTClientDiag__login(const char* userName, const char* pass);
void RESTClient__deinit(void);

HTTPResult RESTClient__get(const char* path, char** recv_content, size_t* len);
HTTPResult RESTClient__post(const char* path, char* content, size_t content_size,
                        char** recv_content, size_t* recv_len);
HTTPResult RESTClient__put(const char* path, char* content, size_t content_size,
                        char** recv_content, size_t* recv_len);


#ifdef __cplusplus
}
#endif

#endif
