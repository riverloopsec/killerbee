// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the configuration parameters for the ZigBee NWK
 *         layer that is user tunable.
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
 * $Id: zigbee_conf.h 41218 2008-04-30 17:41:47Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef ZIGBEE_CONF_H
#define ZIGBEE_CONF_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup grZigBeeNWK
//! @{
/*================================= MACROS           =========================*/

/*! \brief Defines the maximum number of Network Descriptors that will be returned
 *         from a NMLE-NETWORK-DISCOVERY.confirm.
 *
 *  \note This number should be kept the same as that used for the MLME_SCAN.confirm.
 */
#define ZIGBEE_MAX_NMBR_OF_NWKDESC (4)


/*! \brief This macro defines the maximum number of elements in a device's Neighbor
 *         table.
 */
#define ZIGBEE_MAX_NMBR_OF_CHILDREN (25)


/*! \brief This macro defines the maximum number of routers a Coordinator or 
 *         Router can have.
 *
 *  \note This constant must be a positive number.
 */
#define ZIGBEE_MAX_NMBR_OF_ROUTERS (1)


/*! \brief This macro defines the maximum network depth. */
#define ZIGBEE_MAX_NWK_DEPTH (1)


#if ((0 == ZIGBEE_MAX_NMBR_OF_ROUTERS) && (1 != ZIGBEE_MAX_NWK_DEPTH))
#error 'When the number of routers is zero, the network topology is limited to a star.'
#endif
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/*EOF*/
