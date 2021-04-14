/*
  RESTClient.cpp
*/


#include "RESTClient.h"

#define tr_info
#define tr_warn
#define tr_err
#define countof(a)                      (sizeof(a)/sizeof(*(a)))



const char* GET_TOKEN = "v1/get_token?deviceid="; /* Device GUID */

#if 0
const char* GET_PROFILE_CRC = "v1/crc32?vin="; /* VIN */
const char* GET_PROFILE = "v1/profile?format=binary&vin=";
#else
const char* GET_PROFILE_CRC = "v1/crc32?featureSet=oemdtc&vin="; /* VIN */
const char* GET_PROFILE = "v1/profile?format=binary&featureSet=oemdtc&vin="; /* Profile with NWS */
#endif
//03MarDaoNg added to test diag server on ESP32S2
const char*  GET_DECODE_VIN = "v1.0/api/vehicles/vin/";
const char*  GET_REPORT = "v1.0/api/reports/";
const char*  GET_OVERVIEW ="v1.0/api/reports/overview/" ;
const char*  GET_FAILS_MODULES ="v1.0/api/reports/failed-modules/";
const char*  GET_PASS_MODULES  ="v1.0/api/reports/passed-modules/";
const char*  SEARCH_REPORT ="v1.0/api/reports/search/%d/%s";
const char*  CREATE_REPORT ="v1.0/api/reports/";
const char*  UPDATE_REPORT = "v1.0/api/reports/";
const char*  ASSIGN_TO_CUS = "v1.0/api/reports/assign";
const char*  GET_TSBS_RECALLS ="v1.0/api/reports/tsbs-recalls/";
const char*  GET_SCHE_MAINTAENANCE ="v1.0/api/reports/maintenance/";
const char*  GET_PREDICTED_FAILURES ="v1.0/api/reports/predicted-failure/";
const char*  SEARCH_TSBS = "v1.0/api/tsbs/search/%s/%d/%d";
const char*  SEARCH_TSBS_BY_CODE = "v1.0/api/tsbs/search-by-code/%s/%d/%s";
const char*  SEARCH_RECALLS ="v1.0/api/recalls/search/%s/%d";
const char*  SEARCH_DTCS ="v1.0/api/dtcs/search";
const char*  DOWNLOAD_TSB = "v1.0/api/tsbs/download/%s/%s";
const char*  CREATE_ACCOUNT ="v1.0/api/users" ;
const char*  LOGIN ="v1.0/api/users/login";
const char*  RESCOVER_PASSWORD ="v1.0/api/users/forgot-password";
const char*  GET_USER_PROFILE ="v1.0/api/users/profile";
const char*  CHANGE_PASSWORD ="v1.0/api/users/change-password";
const char*  GET_YEARS ="v1.0/api/vehicles/makes/%s/years";
const char*  GET_MAKES ="v1.0/api/vehicles/makes";
const char*  GET_MODELS ="v1.0/api/vehicles/years/%d/makes/%s/models";
const char*  GET_DLC_BY_VIN = "v1.0/api/vehicles/vin/%s/dlc-locator";
const char*  GET_DLC_BY_YMM ="v1.0/api/vehicles/years/%d/makes/%s/models/%s/dlc-locator";
const char*  REG_DEVICE ="v1.0/api/devices";
const char*  GET_DEVICE_INF ="v1.0/api/devices/";
const char*  CONTACT_US ="v1.0/api/supports/contact-us";
const char*  SEND_ISSUE ="v1.0/api/supports/report-issue";
const char*  SEND_REPORT = "v1.0/api/reports/send-email";
const char*  SEND_FIX_INFO = "v1.0/api/vehicles/send-fix-info";
const char*  CHECK_UPGRADE_FW = "v1.0/api/devices/check-upgrade-firmware";
const char*  GET_RETAILERS ="v1.0/api/retailers/list";
const char*  GET_AAP_IMAGE ="v1.0/api/advance-auto-parts/decode-image";
const char*  GET_PRODUCTS = "v1.0/api/products/list";
const char*  PURCHASE_PACKAGE ="v1.0/api/orders";
const char*  GET_ACCESS_TOKEN = "v1.0/api/auth/token";

const char* TRACK_DATA = "v1/tracking?vin=";
const char* GET_OE_ITEM_NAME = "v1/itemname_enums?vin=";

#define URL_PLATFORM     "nxp_wifi"

const char* GET_UPGRADE[] = {
                                            "v1/checkFirmwareUpdate?type=bootloader&current_version=",
                                            "v1/checkFirmwareUpdate?type=firmware&current_version=",
                                            "v1/checkFirmwareUpdate?type=vci&current_version=",
                                            "v1/checkFirmwareUpdate?type=db&current_version=",
                                        };
const char* UPGRADE_PLATFORM_PARAM = "&platform=";

#define MAX_NBR_HEADER_KEY          4


static char* __root_path = NULL;
static uint32_t nbr_header_key = 0;
static char* __hdrs[MAX_NBR_HEADER_KEY*2];

#define HTTPClient__lock()        do {} while(0)
#define HTTPClient__unlock()      do {} while(0)

static char* RESTClient__url_builder(const char* path, uint32_t len, const char* host);

void RESTClient__init(char* host, uint32_t port)
{
    __root_path = NULL;
    nbr_header_key = 0;
    RESTClient__set_host(host, port);
}

char* RESTClient__url_builder(const char* path, uint32_t len, const char* host)
{
    if (!host)
    {
        host = __root_path;
    }

    if (len == 0)
    {
        len = strlen(path);
    }
    uint32_t url_len = strlen(host) + len + 1 + 1; /* slash + null terminal*/
    char* url = (char*)malloc(url_len + 64);
//    if(url==NULL)
//          while(1);
    snprintf(url, url_len, "%s/%s", host, path);
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
    __root_path = (char*)malloc(path_len + 64);
//    if(__root_path==NULL)
//          while(1);
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
    char* url = RESTClient__url_builder(GET_TOKEN, strlen(GET_TOKEN) + strlen(deviceid), NULL);
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
            ret = (char*)malloc(strlen("Bearer ") + strlen(token) + 1 + 64);
//            if(ret==NULL)
//              while(1);
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
    char* url = RESTClient__url_builder(GET_OE_ITEM_NAME, strlen(GET_OE_ITEM_NAME) + 17, NULL);
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

    char* url = RESTClient__url_builder(GET_PROFILE_CRC, strlen(GET_PROFILE_CRC) + 17, NULL);
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
            tr_warn("Server's response doesn't contain crc32");
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
    char* url = RESTClient__url_builder(GET_PROFILE, strlen(GET_PROFILE) + 17, NULL);
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

HTTPResult RESTClient__track(const char* vin, uint8_t* content, size_t content_size, const char* tracking_url)
{
    if (vin == NULL || content == NULL)
    {
        return HTTP_REFUSED;
    }

    char* url = NULL;
    if (tracking_url == NULL) /* Default tracking host innova server */
    {
        url = RESTClient__url_builder(TRACK_DATA, strlen(TRACK_DATA) + 17, NULL);
    }
    else
    {
        uint32_t url_len = strlen(tracking_url) + 17 + 1 + 1; /* slash + null terminal*/
        url = (char*)malloc(url_len + 64);
//        if(url==NULL)
//              while(1);
        memset(url, 0x00, url_len);
        strcpy(url, tracking_url);
    }

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
    char* url = RESTClient__url_builder(path, 0, NULL);
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
    char* url = RESTClient__url_builder(path, 0, NULL);
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
    char* url = RESTClient__url_builder(path, 0, NULL);
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__put_raw(url, content, content_size, p_recv_content, p_recv_len);
    HTTPClient__unlock();
    free(url);
    return ret;
}

HTTPResult RESTClient__get_ota_upgrade_url(const char* platform, BLOBType_t type, const char* cur_version, char** p_download_url)
{
    p_download_url = NULL;
    if (!platform)
        return HTTP_ERROR;

    char* url = RESTClient__url_builder(GET_UPGRADE[type], strlen(GET_UPGRADE[type]) + strlen(cur_version) + strlen(UPGRADE_PLATFORM_PARAM) + strlen(platform), NULL);
    strcat(url, cur_version);
    strcat(url, UPGRADE_PLATFORM_PARAM);
    strcat(url, platform);
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
                    tr_info("Got newer FW at: %s", *p_download_url);
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
}
//03Mar DaoNg added to test diagnostic server on ESP32S2
HTTPResult RESTClientDiag__get_years(const char* make){
    char tmpBuff [1024]={0x00};
    sprintf((char*)tmpBuff,(char*)GET_YEARS, make);
    char* url = RESTClient__url_builder(tmpBuff, strlen(tmpBuff),NULL);
    char* recv_content  = NULL;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get_raw(url, &recv_content, &recv_len,HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    free(url);
    if(recv_content !=NULL)
    {
        free(recv_content);
    }
    return ret;
}

HTTPResult RESTClientDiag__get_sche_maintenance(const char* id){
    if (id == NULL)
    {
        return HTTP_REFUSED;
    }
    char* url = RESTClient__url_builder(GET_SCHE_MAINTAENANCE, strlen(GET_SCHE_MAINTAENANCE)+strlen(id),NULL);
    strncat(url, id, strlen(id));
    char* recv_content  = NULL;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get_raw(url, &recv_content, &recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    free(url);
    if(ret == HTTP_OK)
    {
      //Diagnostic_Data_Manager::getTsbAndRecallBigDataParser(recv_content, p_strtTsbAndRecall, p_iDataLen);
    }
    free(recv_content);
    return ret;
}


HTTPResult RESTClientDiag__get_tsbs_recalls(const char* id){
    if (id == NULL)
    {
        return HTTP_REFUSED;
    }
    char* url = RESTClient__url_builder(GET_TSBS_RECALLS, strlen(GET_TSBS_RECALLS)+strlen(id),NULL);
    strncat(url, id, strlen(id));
    char* recv_content  = NULL;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__get_raw(url, &recv_content, &recv_len, HTTP_CLIENT_DEFAULT_TIMEOUT);
    HTTPClient__unlock();
    free(url);
    if(ret == HTTP_OK)
    {
      //Diagnostic_Data_Manager::getTsbAndRecallBigDataParser(recv_content, p_strtTsbAndRecall, p_iDataLen);
    }
    free(recv_content);
    return ret;
}

HTTPResult RESTClientDiag__login(const char* userName, const char* pass){
    char content [512]={0} ;
    snprintf(content, countof(content), "{\"UserName\":\"%s\",\"Password\":\"%s\"}",userName, pass);
    char* url = RESTClient__url_builder(LOGIN, strlen(LOGIN),NULL);
    char* recv_content  = NULL;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__post_raw(url, (char*)content, strlen(content), &recv_content, &recv_len);
    HTTPClient__unlock();
    free(url);
    if(ret == HTTP_OK)
    {
//      Diagnostic_Data_Manager::LoginParser(recv_content, p_strtAccountSetting);
//      Unicode::toUTF8((uint16_t*)p_strtAccountSetting->bApi_auth_ticket, (uint8_t*)p_Ticket, countof(p_Ticket));
//      set_header("api-auth-ticket",p_Ticket);
    }
    if(recv_content !=NULL)
    {
        free(recv_content);
    }
    return ret;
}
HTTPResult RESTClientDiag__change_pass(const char* oldPass, const char* pass){
    char content [512]={0};
    snprintf(content, countof(content), "{\"OldPassword\":\"%s\",\"Password\":\"%s\"}",oldPass, pass);
    char* url = RESTClient__url_builder(CHANGE_PASSWORD, strlen(CHANGE_PASSWORD),NULL);
    char* recv_content  = NULL;
    size_t recv_len;
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__put_raw(url, (char*)content, strlen(content), &recv_content, &recv_len);
    HTTPClient__unlock();
    free(url);
    if(ret == HTTP_OK)
    {
      //Diagnostic_Data_Manager::MessageSerResponseParser(recv_content, p_strtAccountSetting->p_strtServerResMsg);
    }
    if(recv_content !=NULL)
    {
        free(recv_content);
    }
    return ret;
}
HTTPResult RESTClientDiag__create_report(const char* strVehicleRawData){
    char* content = (char*)strVehicleRawData;
    size_t content_size = strlen(content);
    char* recv_content ;
    size_t recv_len  ;
    char* url = RESTClient__url_builder(CREATE_REPORT, strlen(CREATE_REPORT),NULL);
    HTTPClient__lock();
    HTTPResult ret = HTTPClient__post_raw(url, (char*)content, content_size, &recv_content, &recv_len);
    HTTPClient__unlock();
    if(ret == HTTP_OK){
        //Diagnostic_Data_Manager::createReportParser(recv_content, p_CreateReportResponse);
    }
    free(url);
    if(recv_content !=NULL)
    {
        free(recv_content);
    }
    return ret;
}