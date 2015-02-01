// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the NLME-START-ROUTER.request.
 *
 *         The implementation of this primitive has been simplified by reducing
 *         the primitive's parameter set, and by not implementing the associated
 *         confirm primitive. Since duty-cycled networks are not supported, this
 *         primitive is of a synchronous nature (Direct return)
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
 * $Id: zigbee_start_router.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "ieee802_15_4.h"
#include "ieee802_15_4_pib.h"

#include "zigbee.h"
#include "zigbee_internal_api.h"
#include "zigbee_neighbor_table.h"
#include "zigbee_nib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

bool zigbee_start_router_request(void) {
    /* The device must be joined before it can be started as a router. */
    if (NWK_JOINED != ZIGBEE_NWK_GET_STATE()) { return false; }
    
    /* The device is joined to a coordinator or another router. */
    bool start_router_status = false;
    if (MAC_SUCCESS != ieee802_15_4_start_request(IEEE802_15_4_GET_PAN_ID(), IEEE802_15_4_GET_CHANNEL(), true)) {    
    } else if (MAC_SUCCESS != ieee802_15_4_rx_enable()) {
    } else {
        
        /* Calculate the router's depth in the network and set up the Cskip algorithm. */
        uint8_t depth = zigbee_neighbor_table_find_depth(IEEE802_15_4_GET_COORD_SHORT_ADDRESS(), \
                                                         IEEE802_15_4_GET_SHORT_ADDRESS());
        
        uint16_t c_skip = zigbee_nib_c_skip(depth);
        NWK_NIB_SET_ADDRESS_INCREMENT(c_skip);
         
        ZIGBEE_NWK_SET_STATE(NWK_STARTED);
        NWK_NIB_SET_NODE_ROLE(ZIGBEE_TYPE_COORD);
        start_router_status = true;
    }
    
    return start_router_status; 
}
/*EOF*/
