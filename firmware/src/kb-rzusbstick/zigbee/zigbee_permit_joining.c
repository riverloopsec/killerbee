// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the NLME-PERMIT-JOINING.request primitive.
 *
 *         The chosen implementation does not allow joining for a certain period:
 *         either devices can join or not. It is for the user to control when
 *         and for how long devices might join
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
 * $Id: zigbee_permit_joining.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "ieee802_15_4_pib.h"

#include "zigbee.h"
#include "zigbee_internal_api.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

bool zigbee_permit_joining(bool join_permitted) {
    /* Check that the device is operating either as router or coordinator. */
    if ((NWK_FORMED != ZIGBEE_NWK_GET_STATE()) && 
        (NWK_STARTED != ZIGBEE_NWK_GET_STATE())) {
        return false;
    }
    
    /* Set the Associate Permitted flag in the IEEE 802.15.4 MAC. */
    IEEE802_15_4_SET_ASSOCIATION_PERMITTED(join_permitted);
    return true;
}
/*EOF*/
