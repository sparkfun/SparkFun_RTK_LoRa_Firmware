/*
 * drv_flash.h
 *
 *  Created on: May 27, 2024
 *      Author: ming.chen
 */

#ifndef APP_DRV_FLASH_H_
#define APP_DRV_FLASH_H_

#include "platform.h"
#include "hc_type.h"

//#include "sys_app.h"
//#include "app_common.h"

#define STM_SECTOR_SIZE	2048
#define STM32_FLASH_SIZE 256
#define STM32_FLASH_BASE 0x08000000
#define STM32_FLASH_APPCFG_BASE  (STM32_FLASH_BASE+250*1024) // from 250k span 2k
#define STM32_FLASH_APPCFG_SIZE  (2*1024)
#define STM32_FLASH_BOOTCFG_BASE  (STM32_FLASH_BASE+252*1024) // from 252k span 2k
#define STM32_FLASH_BOOTCFG_SIZE  (2*1024)

void STMFLASH_Read(u32 ReadAddr,u64 *pBuffer,u16 Num64bitToRead);
void STMFLASH_Write(u32 WriteAddr,u64 *pBuffer,u16 Num64bitToWrite);

#endif /* APPLICATION_USER_SUBGHZ_PHY_APP_DRV_FLASH_H_ */
