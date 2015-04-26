// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the MLME-RESET.request primitive.
 *
 *      The MLME-RESET.request primitive allows the next higher layer to request
 *      that a reset operation is performed. This primitive does not have an 
 *      associated confirm primitive due to its synchronous nature
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
 * $Id: ieee802_15_4_reset.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

bool ieee802_15_4_reset(bool set_default_pib) {
    
    /* Reset the MAC's internal state. */
    IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
    
    uint8_t reset_status = false;
    if (true != tat_init()) {
        tat_deinit();
    } else{
        
        /* Reset the PIBs if requested. */
        if (true == set_default_pib) {
            ieee802_15_4_pib_init();
        }
        
        /* Reset callback TAT callback handlers. */
        tat_set_tx_callback_handler(ieee802_15_4_send_ack_event_handler);
        tat_set_rx_callback_handler(ieee802_15_4_pd_data_indication);
        
        /* Configure the radio transceiver; CMSA and frame filer. */
        rf230_subregister_write(SR_MIN_BE, macMinBE_def);
        rf230_subregister_write(SR_MAX_CSMA_RETRIES, macMaxCSMABackoffs_def);
        rf230_subregister_write(SR_MAX_FRAME_RETRIES, aMaxFrameRetries);
        
        tat_set_ieee_address((uint8_t *)(&IEEE802_15_4_GET_EXTENDED_ADDRESS()));
        tat_set_short_address(IEEE802_15_4_GET_SHORT_ADDRESS());
        tat_set_pan_id(IEEE802_15_4_GET_PAN_ID());
        
        /* Go to sleep if configured to do so. Otherwise the radio transceiver will
         * stay in TRX_OFF state.
         */
        if (false == IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
            tat_go_to_sleep();
        }
        
        reset_status = true;
    }
    
    return reset_status;
}
/*EOF*/
