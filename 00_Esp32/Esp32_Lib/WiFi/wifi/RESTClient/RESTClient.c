/*
  RESTClient.cpp
*/


#include "RESTClient.h"

#define tr_info



const char* GET_TOKEN = "v1/get_token?deviceid="; /* Device GUID */

#if 0
const char* GET_PROFILE_CRC = "v1/crc32?vin="; /* VIN */
const char* GET_PROFILE = "v1/profile?format=binary&vin=";
#else
const char* GET_PROFILE_CRC = "v1/crc32?featureSet=oemdtc&vin="; /* VIN */
const char* GET_PROFILE = "v1/profile?format=binary&featureSet=oemdtc&vin="; /* Profile with NWS */
#endif
const char* TRACK_DATA = "v1/tracking?vin=";
const char* GET_OE_ITEM_NAME = "v1/itemname_enums?vin=";

#define URL_PLATFORM     "nxp_wifi"

const char* GET_UPGRADE[] = {
                                            "v1/checkFirmwareUpdate?platform=" URL_PLATFORM "&type=bootloader&current_version=",
                                            "v1/checkFirmwareUpdate?platform=" URL_PLATFORM "&type=firmware&current_version=",
                                            "v1/checkFirmwareUpdate?platform=" URL_PLATFORM "&type=vci&current_version=",
                                            "v1/checkFirmwareUpdate?platform=" URL_PLATFORM "&type=db&current_version=",
                                        };
#define MAX_NBR_HEADER_KEY          4


static char* __root_path = NULL;
static uint32_t nbr_header_key = 0;
static char* __hdrs[MAX_NBR_HEADER_KEY*2];

#define HTTPClient__lock()        do {} while(0)
#define HTTPClient__unlock()      do {} while(0)

void RESTClient__init(char* host, uint32_t port)
{
    __root_path = NULL;
    nbr_header_key = 0;
    RESTClient__set_host(host, port);
}

char* RESTClient__url_builder(const char* path, uint32_t len)
{
    if (len == 0)
    {
        len = strlen(path);
    }
    uint32_t url_len = strlen(__root_path) + len + 1 + 1; /* slash + null terminal*/
    char* url = (char*)malloc(url_len);
    snprintf(url, url_len, "%s/%s", __root_path, path);
    return url;
}

void RESTClient__deinit(void)
{
    if (__root_path)
    {
        free(__root_path);
        __root_path = NULL;
    }
}

void RESTClient__set_host(char* host, uint32_t port)
{
    tr_info("RESTClient: set host to: %s:%d", host, port);
    if (__root_path)
    {
        free(__root_path);
    }
    uint32_t path_len = strlen(host) + sizeof("https://:12456") + 1;
    __root_path = (char*)malloc(path_len);
    if (port == 80)
    {
        sprintf(__root_path, "http://%s", host);
    }
    else
    {
        sprintf(__root_path, "http://%s:%d", host, port);
    }
}

bool RESTClient__set_header(const char* key, const  char* value)
{
    tr_info("RESTClient: set header {%s:%.15s...}", key, value);
    for (uint32_t i = 0; i < nbr_header_key*2; i += 2)
    {
        if (strcmp(__hdrs[i], key) == 0)
        {
            if (__hdrs[i + 1] != NULL)
            {
                free(__hdrs[i + 1]);
                if (value == NULL) /* Remove key */
                {
                    __hdrs[i + 1] = NULL;
                }
                else
                {
                    __hdrs[i + 1] = strdup(value);
                }
            }
            HTTPClient__customHeaders((const char **)__hdrs, nbr_header_key);
            return true;
        }
    }

    if (nbr_header_key < MAX_NBR_HEADER_KEY)
    {
        __hdrs[nbr_header_key*2] = strdup(key);
        __hdrs[nbr_header_key*2 + 1] = strdup(value);
        nbr_header_key++;
        HTTPClient__customHeaders((const char **)__hdrs, nbr_header_key);
        return true;
    }

    return false;
}

char* RESTClient__get_token(const char* deviceid)
{
    char* url = RESTClient__url_builder(GET_TOKEN, strlen(GET_TOKEN) + strlen(deviceid));
    strcat(url, deviceid);
    char* recv_content = NULL;
    size_t recv_len = 0;
    HTTPClient__get_raw(url, &recv_content, &recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    free(url);

    char* ret = NULL;
    if (recv_content)
    {
        char* start_token = strstr(recv_content, "\"token\":\"");
        if (start_token != NULL)
        {
            char* token = start_token + strlen("\"token\":\"");
            char* quotePtr = strstr(token,"\"");
            *quotePtr = '\0';
            ret = (char*)malloc(strlen("Bearer ") + strlen(token) + 1);
            strcpy(ret, "Bearer ");
            strcat(ret, token);
        }
        free(recv_content);
    }
    return ret;
}

bool RESTClient__set_token(const char* token)
{
    return true;
}

uint32_t RESTClient__get_oe_item_name(const char* vin, uint16_t* p_itemList)
{
    uint32_t nbr_item = 0;
    if (vin == NULL)
    {
        return nbr_item;
    }
    char* url = RESTClient__url_builder(GET_OE_ITEM_NAME, strlen(GET_OE_ITEM_NAME) + 17);
    strncat(url, vin, 17);
    char* recv_content = NULL;
    size_t recv_len;
    HTTPClient__get_raw(url, &recv_content, &recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    free(url);

    if (recv_content)
    {
        char* start_token = strstr(recv_content, "\"items\":[");
        if (start_token != NULL)
        {
            char* token = start_token + strlen("\"items\":[");
            char* closePtr = strstr(token,"]");
            *closePtr = '\0';

            char* pch = strtok (token,",");
            while (pch != NULL)
            {
                int item;
                if (sscanf(pch, "%d", &item) == 1)
                {
                    p_itemList[nbr_item++] = (uint16_t)item;
                }
                pch = strtok (NULL, ",");
            }
        }
        free(recv_content);
    }
    return nbr_item;
}

HTTPResult RESTClient__get_profile_crc(const char* vin, uint32_t* p_crc)
{
    uint32_t crc = 0;
    if (vin == NULL)
    {
        return HTTP_REFUSED;
    }

    char* url = RESTClient__url_builder(GET_PROFILE_CRC, strlen(GET_PROFILE_CRC) + 17);
    strncat(url, vin, 17);
    char* recv_content  = NULL;
    size_t recv_len;
    HTTPResult ret = HTTPClient__get_raw(url, &recv_content, &recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    free(url);

    if (recv_content)
    {
        /* {"crc32":1152310590} */
        char* start_key = strstr(recv_content, "\"crc32\":");
        if (start_key != NULL)
        {
            char* ptr = start_key + strlen("\"crc32\":");
            char* bracket = strstr(ptr,"}");
            *bracket = '\0';
            while (*ptr != 0)
            {
                crc = crc*10 + *(ptr++) - '0';
            }
        }
        else
        {
            //tr_warn("Server's response doesn't contain crc32");
            ret = HTTP_NOTFOUND;
        }
        free(recv_content);
    }
    *p_crc = crc;
    return ret;
}

HTTPResult RESTClient__download_profile(const char* vin, IHTTPDataIn* writer)
{
    if (vin == NULL || writer == NULL)
    {
        return HTTP_REFUSED;
    }
    char* url = RESTClient__url_builder(GET_PROFILE, strlen(GET_PROFILE) + 17);
    strncat(url, vin, 17);
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get(url, writer, HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    free(url);
    return ret;
}

HTTPResult RESTClient__download(const char* url, IHTTPDataIn* writer)
{
    if (url == NULL || writer == NULL)
    {
        return HTTP_REFUSED;
    }
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get(url, writer, HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    return ret;
}

HTTPResult RESTClient__track(const char* vin, uint8_t* content, size_t content_size)
{
    if (vin == NULL || content == NULL)
    {
        return HTTP_REFUSED;
    }
    char* url = RESTClient__url_builder(TRACK_DATA, strlen(TRACK_DATA) + 17);
    strncat(url, vin, 17);
    char* recv_content;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__post_raw(url, (char*)content, content_size, &recv_content, &recv_len);
    HTTPClient__unlock();
    free(url);
    if (recv_content)
    {
        free(recv_content);
    }
    return ret;
}

HTTPResult RESTClient__get(const char* path, char** p_recv_content, size_t* p_recv_len)
{
    char* url = RESTClient__url_builder(path, 0);
    *p_recv_content = NULL;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get_raw(url, p_recv_content, p_recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    free(url);
    return ret;
}

HTTPResult RESTClient__post(const char* path, char* content, size_t content_size,
                            char** p_recv_content, size_t* p_recv_len)
{
    *p_recv_content = NULL;
    char* url = RESTClient__url_builder(path, 0);
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__post_raw(url, content, content_size, p_recv_content, p_recv_len);
    HTTPClient__unlock();
    free(url);
    return ret;
}

HTTPResult RESTClient__put(const char* path, char* content, size_t content_size,
                            char** p_recv_content, size_t* p_recv_len)
{
    p_recv_content = NULL;
    char* url = RESTClient__url_builder(path, 0);
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__put_raw(url, content, content_size, p_recv_content, p_recv_len);
    HTTPClient__unlock();
    free(url);
    return ret;
}

HTTPResult RESTClient__get_ota_upgrade_url(BLOBType_t type, const char* cur_version, char** p_download_url)
{
#if 0 // FIXME Dat Le
    p_download_url = NULL;
    char* url = RESTClient__url_builder(GET_UPGRADE[type], strlen(GET_UPGRADE[type]) + strlen(cur_version));
    strcat(url, cur_version);
    char* recv_content = NULL;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get_raw(url, &recv_content, &recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    free(url);

    if (recv_content)
    {
        char* start_token = strstr(recv_content, "\"version\":\"");
        if (start_token != NULL)
        {
            char* token = start_token + strlen("\"version\":\"");
            char* closePtr = strstr(token,"\"");
            *closePtr = '\0';

            int cmp = strcmp(cur_version, token);
            if (cmp <= 0)
            {
                token = closePtr + 1;
                url = strstr(token, "\"url\":\"");
                if (url)
                {
                    url = url + strlen( "\"url\":\"");
                    closePtr = strstr(url,"\"");
                    *closePtr = '\0';
                    *p_download_url = strdup(url);
                    tr_info("Got newer FW at: %s", download_url);
                }
            }
            else
            {
                tr_info("Server version = %s <= current version = %s", token, cur_version);
            }
        }
        free(recv_content);
    }
    return ret;
#endif
}