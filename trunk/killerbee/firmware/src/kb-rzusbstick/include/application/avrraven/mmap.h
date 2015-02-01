// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Memory map of on board program memory and eeprom, and extern dataflash
 *
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: mmap.h 41567 2008-05-13 12:52:19Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __MMAP_H__
#define __MMAP_H__

/*========================= INCLUDES                 =========================*/


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(AVRRAVEN_3290P)
#include "df.h"
#endif

/*========================= CONSTANT DEFINES         =========================*/

///{ ATmega3290P on board flash defines
#define M3290P_FLASH_PAGE_SIZE           128UL

#define M3290P_FLASH_PAGE_COUNT          256UL
#define M3290P_FLASH_SIZE                ((uint32_t)M3290P_FLASH_PAGE_SIZE*(uint32_t)M3290P_FLASH_PAGE_COUNT)

#define M3290P_BOOT_START_PAGE           M3290P_APP_FLASH_PAGE_COUNT
#define M3290P_BOOT_FLASH_PAGE_COUNT     32UL
#define M3290P_BOOT_FLASH_SIZE           ((uint32_t)M3290P_FLASH_PAGE_SIZE*(uint32_t)M3290P_BOOT_FLASH_PAGE_COUNT)

#define M3290P_APP_START_PAGE            0UL
#define M3290P_APP_FLASH_PAGE_COUNT      ((uint32_t)M3290P_FLASH_PAGE_COUNT-(uint32_t)M3290P_BOOT_FLASH_PAGE_COUNT)
#define M3290P_APP_FLASH_SIZE            ((uint32_t)M3290P_FLASH_PAGE_SIZE*(uint32_t)M3290P_APP_FLASH_PAGE_COUNT)

#define M3290P_EEPROM_PAGE_SIZE          4UL
#define M3290P_EEPROM_PAGE_COUNT         256UL
#define M3290P_EEPROM_SIZE               ((uint32_t)M3290P_EEPROM_PAGE_SIZE*(uint32_t)M3290P_EEPROM_PAGE_COUNT)
///}



///{ ATmega1284P on board flash defines
#define M1284P_FLASH_PAGE_SIZE           256UL

#define M1284P_FLASH_PAGE_COUNT          512UL
#define M1284P_FLASH_SIZE                ((uint32_t)M1284P_FLASH_PAGE_SIZE*(uint32_t)M1284P_FLASH_PAGE_COUNT)

#define M1284P_BOOT_START_PAGE           M1284P_APP_FLASH_PAGE_COUNT
#define M1284P_BOOT_FLASH_PAGE_COUNT     32UL
#define M1284P_BOOT_FLASH_SIZE           ((uint32_t)M1284P_FLASH_PAGE_SIZE*(uint32_t)M1284P_BOOT_FLASH_PAGE_COUNT)

#define M1284P_APP_START_PAGE            0UL
#define M1284P_APP_FLASH_PAGE_COUNT      ((uint32_t)M1284P_FLASH_PAGE_COUNT-(uint32_t)M1284P_BOOT_FLASH_PAGE_COUNT)
#define M1284P_APP_FLASH_SIZE            ((uint32_t)M1284P_FLASH_PAGE_SIZE*(uint32_t)M1284P_APP_FLASH_PAGE_COUNT)

#define M1284P_EEPROM_PAGE_SIZE          4UL
#define M1284P_EEPROM_PAGE_COUNT         1024UL
#define M1284P_EEPROM_SIZE               ((uint32_t)M1284P_EEPROM_PAGE_SIZE*(uint32_t)M1284P_EEPROM_PAGE_COUNT)
///}



///{ Firmware images on data flash
#if defined(AVRRAVEN_3290P)
#define M3290P_FLASH_IMG_SIZE            (M3290P_FLASH_SIZE)
#define M1284P_FLASH_IMG_SIZE            (M1284P_FLASH_SIZE)
#define M3290P_EEPROM_IMG_SIZE           (M3290P_EEPROM_SIZE)
#define M1284P_EEPROM_IMG_SIZE           (M1284P_EEPROM_SIZE)


#define M1284P_FLASH_FD_IMG_ADR          (DF_SECTOR_11_ADR)
#define M3290P_FLASH_FD_IMG_ADR          (DF_SECTOR_12_ADR)
#define M1284P_FLASH_USR_IMG_ADR         (DF_SECTOR_13_ADR)
#define M3290P_FLASH_USR_IMG_ADR         (DF_SECTOR_14_ADR)
#define M3290P_EEPROM_USR_IMG_ADR        (DF_SECTOR_15_ADR + (uint32_t)0 )
#define M1284P_EEPROM_USR_IMG_ADR        ((uint32_t)DF_SECTOR_15_ADR + (uint32_t)M3290P_EEPROM_IMG_SIZE)
#endif
///}

///{ EEPROM addresses
#if defined(AVRRAVEN_3290P)
#define BLCC_SIZE               4
#define BLCC_EEADR              0x24

#define USER_CONFIG_EEADR       (BLCC_EEADR + BLCC_SIZE)
#define USER_CONFIG_SIZE        (sizeof(avrraven_t))
#endif
///}


///{ Revision number addresses
#if defined(AVRRAVEN_3290P)
#define FW_REV_ADR_M3290P_APP   (((M3290P_APP_START_PAGE  + M3290P_APP_FLASH_PAGE_COUNT)  * M3290P_FLASH_PAGE_SIZE) - (uint32_t)sizeof(uint16_t))
#define FW_REV_ADR_M3290P_BL    (((M3290P_BOOT_START_PAGE + M3290P_BOOT_FLASH_PAGE_COUNT) * M3290P_FLASH_PAGE_SIZE) - (uint32_t)sizeof(uint16_t))
#endif

#define FW_REV_ADR_M1284P_APP   (((M1284P_APP_START_PAGE  + M1284P_APP_FLASH_PAGE_COUNT)  * M1284P_FLASH_PAGE_SIZE) - (uint32_t)sizeof(uint16_t))
#define FW_REV_ADR_M1284P_BL    (((M1284P_BOOT_START_PAGE + M1284P_BOOT_FLASH_PAGE_COUNT) * M1284P_FLASH_PAGE_SIZE) - (uint32_t)sizeof(uint16_t))
///}



/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

#endif // __MMAP_H__

/*EOF*/
