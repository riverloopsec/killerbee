// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Common settings for the AVR Raven boot loader
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
 * $Id: bl.h 41763 2008-05-15 18:08:50Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __BL_H__
#define __BL_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"
#include "mmap.h"
#include "self_programming.h"

/*========================= MACROS                   =========================*/
#define BLCC_FW_UPGRADE_START_REQUEST_FROM_APP  (0xBAADF00D)
#define BLCC_RESTART_REQUEST_FROM_APP           (0xBAADBABE)

#define BLCC_FW_UPGRADE_COMPLETE                (0xCAFEBABE)
#define BLCC_NORMAL_APP_START                   (0xCAFEF00D)
#define BLCC_LOAD_FACTORY_DEFAULT               (0xBABEF00D)
#define BLCC_UNKNOWN

#define BLCC_WRITE(val) {\
    uint32_t tmp = val;\
    sp_write_eeprom((BLCC_EEADR + 0), (uint8_t)((tmp & 0x000000FF) >>  0));\
    sp_write_eeprom((BLCC_EEADR + 1), (uint8_t)((tmp & 0x0000FF00) >>  8));\
    sp_write_eeprom((BLCC_EEADR + 2), (uint8_t)((tmp & 0x00FF0000) >> 16));\
    sp_write_eeprom((BLCC_EEADR + 3), (uint8_t)((tmp & 0xFF000000) >> 24));\
}

#define BLCC_READ(var) {\
    uint32_t tmp;\
    tmp  = sp_read_eeprom(BLCC_EEADR + 3);\
    tmp <<=  8;\
    tmp |= sp_read_eeprom(BLCC_EEADR + 2);\
    tmp <<=  8;\
    tmp |= sp_read_eeprom(BLCC_EEADR + 1);\
    tmp <<=  8;\
    tmp |= sp_read_eeprom(BLCC_EEADR + 0);\
    var = tmp;\
}
        

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/


#endif // __BL_H__

