#ifndef __IAP_UPGRADE_H__
#define __IAP_UPGRADE_H__
#include "BLOBType.h"
#include "FlashIAP.h"
#include "SPIFBlockDevice.h"

bool IAPUpgradeSourceValid(BLOBType_t type);
bool IAPUpgrade(SPIFBlockDevice* bd, uint32_t src_addr, FlashIAP* iap, uint32_t des_addr);

#endif
