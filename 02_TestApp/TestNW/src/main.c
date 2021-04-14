#include <time.h>

#include <logfile.h>

#include "esp32/esp32.h"
#include "esp32/inc/NM.h"
#include "esp32/inc/HTTPClient.h"
#include "esp32/inc/RESTClient.h"

#define countof(a)                      (sizeof(a)/sizeof(*(a)))
#define WIFI_PROFILE_SSID_LEN           64
#define WIFI_PROFILE_PASS_LEN           64
#define WIFI_PROFILE_CHOICE             6


typedef struct {
  char ssid[WIFI_PROFILE_SSID_LEN];
  char pass[WIFI_PROFILE_PASS_LEN];
} wifi_profile_t;

wifi_profile_t wifi[] = {
  {
    .ssid = "TestWifi",
    .pass = "12345678"
  },
  {
    .ssid = "VINFAST-STAFF01",
    .pass = "Vinfast@9999"
  },
  {
    .ssid = "Khoadd2",
    .pass = "12345678"
  },
  {
    .ssid = ".GUTA CAFE",
    .pass = "gutacafe"
  },
  {
    .ssid = "KhoaDD2_PC",
    .pass = "12345678"
  },
  {
    .ssid = "Dao",
    .pass = "56781234"
  },
  {
    .ssid = "Khoadd2_phone",
    .pass = "12345678"
  }
};

// KhoaDD2 create a package manager to assign task

const char strVehicleRawData[] = "{\"Vin\":\"1GNALAEK1FZ105363\",\
            \"Mileage\": 25,\
                \"DongleId\":\"085a6489-d605-4b2c-903d-3618d57805db\",\
                    \"UsbProductId\": 960,\
                        \"VinProfileRaw\":\"qjFHTkFMQUVLMUZaMTA1MzYzAAQA\
                          IwAHAD0A/////50AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                          AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=\",\
                            \"MonitorStatusEcmRaw\":\"qt+/gACFB2Vl\",\
                            \"FreezeFrameEcmRaw\":\"qg8AAAIAAASqAQIEAAQAAgIIAA\
                              KqAwIKAAKqBAIMAAGqBQINAAGqBgIOAAKqBwIQAAKqCAISAA\
                              KqCQIUAAKqCgIWAAGqCwIXAAGqDAIYAAKqDQIaAAGqDgIbAA\
                              GqAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//wAAA\
                              AAAAAEUgAAAACABoAHgAiACQoKAAAAlgAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAA==\",\
                            \"DtcsEcmRaw\":\"AAACAAEdAOgHAAAcBAIEUgQCIAQhIgQBB\
                              wUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAqgAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAqg==\",\
                            \"VehicleInfoEcmRaw\":\"VQAAAAAxR05BTEFFSzFGWjEwNT\
                              M2MwAxMjY0NDQ0NwAAAAAAAAAAMTI2NjMyMzIAAAAAAAAAAD\
                              EyNjYzMjMwAAAAAAAAAAAxMjYzODUzNQAAAAAAAAAAMTI2NT\
                              QyOTQAAAAAAAAAADEyNjU0Mjk5AAAAAAAAAAAxMjY1NDMwNQ\
                              AAAAAAAAAAMTI2NTQzMDAAAAAAAAAAAAAAANaxAAAHpQAA+l\
                              IAAJiZAADiigAAPW4AAHL1AAAzAhQO5y7pFtsOJhbaDiYM8A\
                              4mDPEOJhmsDucAAAAAAmUCsg3PDiYN6w4mAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAA==\",\
                            \"VehicleInfoTcmRaw\":\"AAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
                              AAAAAAAAAAAAAAAAAAAA==\"}";

int main(void)
{
    int r=0, trytime=3;
    int8_t rssi;
    bool connected,r_b;
    uint32_t nbr_ap;
    wl_ap_info_t ap_list[20];
    HTTPResult status;

    LOGI("Start test NW app");

    if(r==0)
    {
        LOGI("ESP32_Init NM__init");
        ESP32_Init();
        NM__init();
    }
    if(r==0)
    {
        LOGI("NM__ScanNetworks");
        nbr_ap = NM__ScanNetworks(ap_list, countof(ap_list));
        if(nbr_ap<=0)
        {
          LOGE("No network found");
          r=-1;
        }
        else
        {
            LOGI("KhoaDD2 nbr_ap : %i",nbr_ap);
            for(int i=0;i<nbr_ap;i++)
            {
                LOGI("ssid [%i]: %s", i, ap_list[i].ssid);
                usleep(10000);
            }
        }
    }

    if(r==0)
    {
        LOGI("NM__connect");
        connected = NM__connect(wifi[WIFI_PROFILE_CHOICE].ssid,
                                wifi[WIFI_PROFILE_CHOICE].pass);
        LOGI("NW_connect %s",connected?"Connected":"Connect failed");
        if(!connected)
        {
            LOGE("Network not found");
            r=-1; // If failed here, add error code for net not found
        }

    }

    if(r==0)
    {
        LOGI("NM__GetConnectionRssi");
        rssi = NM__GetConnectionRssi();
        if(rssi<-120) // Marco this
        {
            r=-1;
            LOGE("RSSI is too low %i",(int)rssi);
        }
        // Check rssi here, if rssi is too low, show error
        // If failed here, we can add error code for low rssi here
    }
    if(r==0)
    {
        // This function void return
        LOGI("HTTPClient__init");
        HTTPClient__init();
    }
    if(r==0 && !NM__is_connected())
    {
        r=-1; // We can add error code for not connected here
    }
    if(r==0)
    {
        LOGI("RESTClient__set_host");
        RESTClient__set_host("dev-tablet-api.innova.com",80);
    }
    if(r==0)
    {
        LOGI("RESTClient__set_header");
        r_b = RESTClient__set_header("api-auth-ticket","4jjHCnMV3jy9g6W/9XLNQI"
                                     "m3H9THI/isXtvteqgUChQqOoq/0uF7yQZ079ITXu"
                                       "W+308btOuUKGqKwiqClAeKmUcih9SvC0f7K9mF"
                                         "UK6Dv5PjjBzt3M3cZz3IJ9369qLqNsLdDFt9"
                                           "zZpAk4bvJfkctg==");
        if(r_b==false)
        {
            LOGE("RESTClient__set_header failed");
            r=-1;
        }
    }

    if(r==0 && false)
    {
        LOGI("RESTClientDiag__get_sche_maintenance");
        status = RESTClientDiag__get_sche_maintenance(
                              "4b92a9c3-fac8-4a89-b82c-60cff6ca0924");
        if(HTTP_OK!=status)
        {
            r=-1;
            LOGE("RESTClientDiag__get_sche_maintenance failed");
        }

    }

    if(r==0 && false)
    {
        LOGI("RESTClientDiag__create_report");
        status = RESTClientDiag__create_report(strVehicleRawData);
        if(HTTP_OK!=status)
        {
            r=-1;
            LOGE("RESTClientDiag__create_report failed");
        }

    }

    if( r==0 )
        while(trytime-->0)
        {
            LOGI("RESTClientDiag__get_tsbs_recalls");
            status = RESTClientDiag__get_tsbs_recalls(
                                  "4b92a9c3-fac8-4a89-b82c-60cff6ca0924");
            if(HTTP_OK!=status)
            {
                r=-1;
                LOGE("RESTClientDiag__get_tsbs_recalls failed");
            }
            else
            {
                r=0;
                LOGI("RESTClientDiag__get_tsbs_recalls success");
                break;
            }

        }

    while(1)
    {
        
    }
    return 0;
}