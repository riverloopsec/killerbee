// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the NLME-NETWORK-FORMATION.request primitive.
 *
 * The primitive has been slightly modified from that described in the ZigBee
 * specification. The MLME-START.request of the IEEE 802.15.4 MAC is simply called
 * instructing the creation of a new PAN. It is up to the next higher layer to
 * find the suitable channel and PAN identifier for the new PAN
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
 * $Id: zigbee_formation.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"

#include "ieee802_15_4.h"
#include "zigbee.h"
#include "zigbee_const.h"
#include "zigbee_internal_api.h"
#include "zigbee_msg_types.h"
#include "zigbee_nib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

bool zigbee_formation_request(nlme_formation_req_t *nfr) {
    /* Add check of current state. */
    if (NWK_IDLE != ZIGBEE_NWK_GET_STATE()) {
        return false;
    }
    
    /* Peform sanity check on the function arguments. */
    if (NULL == nfr) {
        return false;
    }
    
    bool formation_status = false;
    
    /* Form the newtork by issuing the MLME_START.request primitive to the
     * the IEEE 802.15.4 MAC. If this primitive is successful, set the device's
     * short address to 0x0000. Finally enable the receiver.
     */
    if (MAC_SUCCESS != ieee802_15_4_start_request((nfr->pan_id), (nfr->channel), true)) {
    } else if (MAC_SUCCESS != ieee802_15_4_set_short_address(0x0000)) {
    } else if (MAC_SUCCESS != ieee802_15_4_rx_enable()) {
    } else {
        /* Network Formation is only used to start a Coordinator. Set network
         * depth to zero, calculate Cskip and set next nwkAddress.
         */
        
        NWK_NIB_SET_NWK_ADDRESS(0);
            
        uint16_t c_skip = zigbee_nib_c_skip(0);
        NWK_NIB_SET_ADDRESS_INCREMENT(c_skip);
        NWK_NIB_SET_PARENT_ADDRESS_INCREMENT(c_skip);
        
        ZIGBEE_NWK_SET_STATE(NWK_FORMED);
        NWK_NIB_SET_NODE_ROLE(ZIGBEE_TYPE_ROUTER);
        
        formation_status = true;
    }
    
    return formation_status;
}
/*EOF*/
