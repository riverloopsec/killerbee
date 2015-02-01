// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the IEEE 802.15.4 MLME-START.request primitive.
 *
 *      The implementation of this particular primitive has been greatly simplified
 *      due to the fact that beaconing and GTS is not supported at the moment.
 *      The MLME-START.request primitive is used to start a node as coordinator.
 *      The associated confirm primitive has been removed due to the synchronous
 *      nature of this primitive
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
 * $Id: ieee802_15_4_start.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_pib.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_msg_types.h"
#include "ieee802_15_4_internal_api.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

uint8_t ieee802_15_4_start_request(uint16_t pan_id, uint8_t channel, bool pan_coordinator) {
    /* Check that the internal state is correct. */
    if ((IEEE_MAC_ASSOCIATED != IEEE802_15_4_GET_STATE()) && 
        (IEEE_MAC_IDLE != IEEE802_15_4_GET_STATE())) {
        
        /* Jump to the local error handler. */
        goto ieee_start_error;
    }
    
    /* Perform sanity checks on all function parameters. */
    if (BROADCAST == pan_id) {
        
        /* Jump to the local error handler. */
        goto ieee_start_error;
    }
    
    
    if ((channel < MIN_CHANNEL) || (channel > MAX_CHANNEL)) {
        
        /* Jump to the local error handler. */
        goto ieee_start_error;
    }
    
    if(MAC_SUCCESS != ieee802_15_4_set_pan_id(pan_id)) {
                
        /* Jump to the local error handler. */
        goto ieee_start_error;
    } else if (true != tat_set_channel(channel)) {
                
        /* Jump to the local error handler. */
        goto ieee_start_error;
    } else {
        IEEE802_15_4_SET_I_AM_COORDINATOR(pan_coordinator);
        tat_set_pan_coordinator(pan_coordinator);
        
        /* For nonbeacon-enabled networks the macRxOnWhenIdle is set true
         * since the coordinator is always awake.
         */
        IEEE802_15_4_SET_RX_ON_WHEN_IDLE(true);
        
        /* Update the internal state to reflect that the node has successfully
         * been started as a cooridinator.
         */
        IEEE802_15_4_SET_STATE(IEEE_MAC_STARTED);
        return MAC_SUCCESS;
    }
    
    
    /* Handle any error that occured during the primitive execution. */
    ieee_start_error:
    

    tat_go_to_sleep();
    
    return MAC_INVALID_PARAMETER;
}
/*EOF*/
