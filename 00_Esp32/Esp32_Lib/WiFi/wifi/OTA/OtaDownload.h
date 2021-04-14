/******************************************************************************=
 ================================================================================
 INNOVA ELECTRONICS VIETNAM
 Filename: OtaDownload.h
 Description:
 Layer:
 Accessibility:
 ================================================================================
 *******************************************************************************/
#ifndef __OTA_DOWNLOAD_H__
#define __OTA_DOWNLOAD_H__
#include "BLOBType.h"

bool OTA_Download(BLOBType_t type, const char* current_version);

#endif
