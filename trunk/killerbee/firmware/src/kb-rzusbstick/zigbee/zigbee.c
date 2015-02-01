// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the initialization and management routines for
 *        the NWK layer
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
 * $Id: zigbee.c 41144 2008-04-29 12:42:28Z ihanssen $
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
#include "zigbee_internal_api.h"
#include "zigbee_nib.h"
#include "zigbee_msg_types.h"
#include "zigbee_neighbor_table.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
zigbee_nwk_state_t nwk_state = NWK_UNINITIALIZED; //!< Variable containing the internal NWK layer state.
zigbee_nwk_parameters_t nwk_param; //!< Storage used to hold different NWK layer parameters.
/*================================= LOCAL VARIABLES  =========================*/
/* Callbacks for the stack's indications. */
static zigbee_data_indication_t ndi; //!< Pointer function that will be executed upon a NLDE_DATA.indication.
static zigbee_join_indication_t nji; //!< Pointer function that will be executed upon a NLME_JOIN.indication.
static zigbee_leave_indication_t nli; //!< Pointer function that will be executed upon a NLME_LEAVE.indication.
/*================================= PROTOTYPES       =========================*/

bool zigbee_init(uint64_t ieee_address) {
    /* Set local variables to initial value. */
    ENTER_CRITICAL_REGION();
    bool init_status = false;
    
    ndi = NULL;
    nji = NULL;
    nli = NULL;
    LEAVE_CRITICAL_REGION();
    
    /* Reset internal variables. */
    zigbee_nib_init();
    zigbee_neighbor_table_init();
    
    if(true != ieee802_15_4_init(ieee_address)) {
    } else {
        /* Initialize all necessary callbacks from the IEEE 802.15.4 MAC. */
        ieee802_15_4_set_mcps_data_indication(mac_data_indication_callback);
        ieee802_15_4_set_mlme_associate_indication(mac_associate_indication_callback);
        ieee802_15_4_set_mlme_disassociate_indication(mac_disassociate_indication_callback);
        ieee802_15_4_set_mlme_orphan_indication(mac_orphan_indication_callback);
        ieee802_15_4_set_mlme_comm_status_indication(mac_comm_status_indication_callback);

        ZIGBEE_NWK_SET_STATE(NWK_IDLE);
        init_status = true;
    } // END: if(ieee802_15_4_init(ieee_address)) ...
    
    return init_status;
}

void zigbee_deinit(void) {
    if (NWK_UNINITIALIZED == nwk_state) { return; }
    
    ieee802_15_4_deinit();
    
    ENTER_CRITICAL_REGION();
    
    ndi = NULL;
    nji = NULL;
    nli = NULL;
    
    LEAVE_CRITICAL_REGION();
    
    nwk_state = NWK_UNINITIALIZED;
}


zigbee_data_indication_t zigbee_get_nlde_data_indication(void) {    
    return ndi;
}


void zigbee_clear_nlde_data_indication(void) {
    ENTER_CRITICAL_REGION();
    ndi = NULL;
    LEAVE_CRITICAL_REGION();
}


void zigbee_set_nlde_data_indication(zigbee_data_indication_t indication) {
    ENTER_CRITICAL_REGION();
    ndi = indication;
    LEAVE_CRITICAL_REGION();
}


zigbee_join_indication_t zigbee_get_nlme_join_indication(void) {    
    return nji;
}


void zigbee_clear_nlme_join_indication(void) {
    ENTER_CRITICAL_REGION();
    nji = NULL;
    LEAVE_CRITICAL_REGION();
}


void zigbee_set_nlme_join_indication(zigbee_join_indication_t indication) {
    ENTER_CRITICAL_REGION();
    nji = indication;
    LEAVE_CRITICAL_REGION();
}


zigbee_leave_indication_t zigbee_get_nlme_leave_indication(void) {    
    return nli;
}


void zigbee_clear_nlme_leave_indication(void) {
    ENTER_CRITICAL_REGION();
    nli = NULL;
    LEAVE_CRITICAL_REGION();
}


void zigbee_set_nlme_leave_indication(zigbee_leave_indication_t indication) {
    ENTER_CRITICAL_REGION();
    nli = indication;
    LEAVE_CRITICAL_REGION();
}


/*                       IEEE 802.15.4 MAC Callbacks.                         */


void mac_orphan_indication_callback(mlme_orphan_ind_t *moi) {

}
/*EOF*/
