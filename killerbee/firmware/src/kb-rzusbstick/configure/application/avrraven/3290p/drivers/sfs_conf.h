// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Configuration file for fs.c
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
 * $Id: sfs_conf.h 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __SFS_CONF_H__
#define __SFS_CONF_H__

#ifdef AVRRAVEN_3290P

#include "df.h"

//! Max open file streams at a time
#define SFS_MAX_OPEN_STREAMS             (8)

//! Size of a physical block the data flash
#define SFS_BLOCK_SIZE                   DF_PAGE_SIZE

//!
#define SFS_MAX_FILES                   (100UL)

//!
#define SFS_BLOCK_COUNT                 (500UL)


/// Max size of error messages
#define SFS_ERR_MSG_MAX_SIZE             (25)

/// Type big enough to hold the size of the entrie data flash
typedef df_size_t sfs_size_t;

///@{ Macros for faster HW access
#define sfs_hal_write(adr,count,data) df_write(adr,count,data)

#define sfs_hal_fill(adr,count,data) {\
    uint8_t val = data;\
    df_fill(adr,count,&val);\
}

#define sfs_hal_read(adr,count,data) df_read((df_adr_t)adr,count,data)
///@}


#define sfs_fopen_P(name, open_mode) {\
    char ram_name[13];\
    strncpy_P((char*)ram_name, name, sizeof(ram_name)+1);\
    sfs_fopen((const unsigned char*)ram_name, open_mode);\
    while(0);\
}

#else
#error "FS configuration not supported."
#endif

#endif // __SFS_CONF_H__
/*EOF*/

