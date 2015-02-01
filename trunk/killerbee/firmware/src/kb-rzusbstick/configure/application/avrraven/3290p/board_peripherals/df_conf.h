// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Configuration file for df.c
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
 * $Id: df_conf.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __DF_CONF_H__
#define __DF_CONF_H__

/*========================= INCLUDES                 =========================*/
/*========================= CONSTANT DEFINES         =========================*/

#if defined(AVRRAVEN_3290P)

#include "spi.h"

//! \addtogroup grAt45DLowLevel
//! @{

#define DDRDF       DDRSPI

#define PORTDF      PORTSPI

#define PORTDF_CS   PORTSPI
#define PORTDF_RST  PORTSPI
#define PORTDF_WR   PORTSPI

#define DF_CS    SS
#define DF_RST   PINB4
//#define DF_WR

#define DF_PAGE_SIZE        528U
#define DF_BUFFER_SIZE      528U
#define DF_PAGES            4096U
#define DF_PAGES_PER_SECTOR 256U

#define DF_SECTOR_0A_ADR    (0UL)
#define DF_SECTOR_0B_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 0UL)
#define DF_SECTOR_1_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 1UL)
#define DF_SECTOR_2_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 2UL)
#define DF_SECTOR_3_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 3UL)
#define DF_SECTOR_4_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 4UL)
#define DF_SECTOR_5_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 5UL)
#define DF_SECTOR_6_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 6UL)
#define DF_SECTOR_7_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 7UL)
#define DF_SECTOR_8_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 8UL)
#define DF_SECTOR_9_ADR     ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR* 9UL)
#define DF_SECTOR_10_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR*10UL)
#define DF_SECTOR_11_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR*11UL)
#define DF_SECTOR_12_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR*12UL)
#define DF_SECTOR_13_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR*13UL)
#define DF_SECTOR_14_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR*14UL)
#define DF_SECTOR_15_ADR    ((unsigned long)DF_PAGE_SIZE*(unsigned long)DF_PAGES_PER_SECTOR*15UL)

#else
#error "DF configuration not supported."
#endif

/*========================= MACROS DEFINES           =========================*/

#if defined(AVRRAVEN_3290P)

#include "spi.h"

#define df_cs_assert()      PORTDF_CS &= ~(1<<DF_CS)
#define df_cs_release()     PORTDF_CS |= (1<<DF_CS)

#define df_rst_assert()     PORTDF_RST &= ~(1<<DF_RST)
#define df_rst_release()    PORTDF_RST |= (1<<DF_RST)

//#define df_wr_assert()      PORTDF_WR &= ~(1<<DF_WR)
//#define df_wr_release()     PORTDF_WR |= (1<<DF_WR)

#else
#error "DF configuration not supported."
#endif
/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
//! @}
#endif // __DF_CONF_H__
/*EOF*/
