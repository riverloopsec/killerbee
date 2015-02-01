// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the IEEE 802.15.4 MLME-RX-ENABLE.request primitive.
 *
 *      This implementation of the MLME-RX-ENABLE.request primitive is somewhat
 *      simplified with regards to the IEEE 802.15.4 standard. There is one 
 *      primitive that is used to enable the receiver and another one to disable
 *      it. Also there is no confirm primitive associated with these requests, the
 *      transition status is returned immidiately
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
 * $Id: ieee802_15_4_rx_enable.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_pib.h"
#include "ieee802_15_4_internal_api.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/


uint8_t ieee802_15_4_rx_enable(void) {
    /* Check first that the MAC is in a state where it is possible to send
     * data. A coordinator must be started and a router or device must be associated.
     */
    if ((IEEE_MAC_STARTED != IEEE802_15_4_GET_STATE()) && 
        (IEEE_MAC_ASSOCIATED != IEEE802_15_4_GET_STATE())) {
        return MAC_INVALID_PARAMETER;
    }
    
    uint8_t rx_enable_status = MAC_INVALID_PARAMETER;
    
    if (true != tat_wake_up()) {
    } else if (true != tat_set_state(RX_AACK_ON)) {
        if (false == IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
            tat_go_to_sleep();
        }
    } else {
        rx_enable_status = MAC_SUCCESS;
    }
    
    return rx_enable_status;
}


uint8_t ieee802_15_4_rx_disable(void) {
    /* Disable the receiver. The new state of the radio transceiver will be
     * TRX_OFF or SLEEP depending on the RX_ON_WHEN_IDLE flag.
     */
    if (false == IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
        tat_go_to_sleep();
    } else {
		tat_reset();
	}
    
    return MAC_SUCCESS;
}
/*EOF*/
