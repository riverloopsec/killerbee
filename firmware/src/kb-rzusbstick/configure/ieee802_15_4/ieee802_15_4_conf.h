// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the user's compile time configuration for the 
 *         IEEE 802.15.4 MAC
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
 * $Id: ieee802_15_4_conf.h 41219 2008-05-01 10:51:43Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef IEEE802_15_4_CONF_H
#define IEEE802_15_4_CONF_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "vrt_timer.h"

//! \addtogroup grIEEE802_15_4
//! @{
/*================================= MACROS           =========================*/
/*! \brief Defines the maximum number of Network Descriptors that will be returned
 *         from a NMLE_NETWORK_DISCOVERY.confirm.
 *
 *  \note This number should be kept the same as that used for the MLME_SCAN.confirm.
 */
#define IEEE802_15_4_MAX_NMBR_OF_PANDESC (4)

/*! \brief Defines the number of packets that can be held by the IEEE 802.15.4 MAC
 *         simultaneously. The number must be different from 0.
 */
#define IEEE_802_15_4_POOL_SIZE (3)


#define IEEE802_15_4_SYMBOL_TO_TICKS(symbols) (VRT_TIME_TICK_PER_SYMBOL * ((uint32_t)symbols))
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/*EOF*/
