// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This implements the NLME_LEAVE ZigBee primitive. The implementation 
 *         relies on the IEEE 802.15.4 MAC's MLME-DISASSOCIATE instead of 
 *         The ZigBee NWK layer implementation (2006 specification). This might 
 *         be changed future releases of this FW, since this is not compliant 
 *         according to the current ZigBee specification
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
 * $Id: zigbee_leave.c 41144 2008-04-29 12:42:28Z ihanssen $
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
#include "vrt_mem.h"

#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_pib.h"

#include "zigbee.h"
#include "zigbee_internal_api.h"
#include "zigbee_const.h"
#include "zigbee_msg_types.h"
#include "zigbee_nib.h"
#include "zigbee_neighbor_table.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This is function is executed when a MLME_DISASSOCIATE.confirm message
 *         is issued from the IEEE 802.15.4 MAC.
 *
 *  \param[in] mdc This is a pointer to the confirm message built by the next 
 *                 lower layer.
 */
static void mac_disassociate_confirm_callback(mlme_disassociate_conf_t *mdc); 


bool zigbee_leave_request(nlme_leave_req_t *nlr) {
    /* Perform sanity check on function parameters. */
    if (NULL == nlr) {
        return false;
    }
    
    if (NULL == (nlr->nlme_callback_leave_confirm)) {
        return false;
    }
    
    mlme_disassociate_req_t* mdr = &nlr->mdr;
    
    /* Validate the DeviceAddress parameter. */
    if (0 == (nlr->DeviceAddress)) {
        /* The node must be an end device to remove itself. In this implementation
         * it is not possible for a router or coordinator to leave.
         */
        if (ZIGBEE_TYPE_DEVICE != NWK_NIB_GET_NODE_ROLE()) { return false; }
        memcpy((void *)(&(mdr->DeviceAddress)), (void *)(&(IEEE802_15_4_GET_COORD_EXTENDED_ADDRESS())), sizeof(uint64_t));
        mdr->DisassociateReason = 0x02; // Device wishes to leave the pan.
    } else {
        /* Only a ZigBee Coordinator or Router can force other nodes to leave
         * the network.
         */
        if ((ZIGBEE_TYPE_COORD != NWK_NIB_GET_NODE_ROLE()) && 
            (ZIGBEE_TYPE_ROUTER != NWK_NIB_GET_NODE_ROLE())) {
            return false;
        }
        
        /* Check that the node to be relinquished from the network is child of
         * this node, and hence is known in the Neighbor Table.
         */
        zigbee_neighbor_table_item_t *nb_item = zigbee_neighbor_table_find_long(nlr->DeviceAddress);
        
        if (NULL == nb_item) {
            return false;
        } else {
            /* Delete node. */
            zigbee_neighbor_table_delete(nb_item);
            memcpy((void *)(&(mdr->DeviceAddress)), (void *)(&(nlr->DeviceAddress)), sizeof(uint64_t));
            mdr->DisassociateReason = 0x01; // Coordinator wishes that device leaves the pan.
        }
    }
    
    /* By this point in the code it has been verified that this is either a ZigBee
     * Coordinator or Router that wishes another node to leave, or a ZigBee
     * End-device that wishes to leave the network. It is time to build the 
     * MLME_DISASSOCIATE.request message, and post it.
     */
    
    mdr->mlme_disassociate_confirm = mac_disassociate_confirm_callback;
        
    /* Store some local variable that will be used during the confirm part of
     * this primitive.
     */
    nwk_param.leave.nlme_callback_leave_confirm = nlr->nlme_callback_leave_confirm;
    nwk_param.leave.confirm = &(nlr->nlc);
    memcpy((void *)(&(nwk_param.leave.node_to_leave)), (void *)(&(nlr->DeviceAddress)), sizeof(uint64_t));
    
    bool leave_status = false;
    if (true != ieee802_15_4_disassociate_request(mdr)) {
    } else {
        ZIGBEE_NWK_SET_STATE(NWK_LEAVING);       
        return true;
    }
    
    return leave_status;
}


/*                            IEEE 802.15.4 MAC Callbacks                     */


void mac_disassociate_indication_callback(mlme_disassociate_ind_t *mdi) {
    /* This message can be received by a ZigBee Coordinator or Router that a
     * a child ZigBee End-device wishes to leave the network. Or by a ZigBee
     * End-device that is asked to leave the newtork.
     */
    if ((ZIGBEE_TYPE_COORD == NWK_NIB_GET_NODE_ROLE()) ||
        (ZIGBEE_TYPE_ROUTER == NWK_NIB_GET_NODE_ROLE())) {
        /* The ZigBee Coordinator or Router must remove this node from its
         * Neighbor Table.
         */
        zigbee_neighbor_table_item_t *nb_item = zigbee_neighbor_table_find_long(mdi->DeviceAddress);
        if (NULL != nb_item) {
            /* Delete node, and allow joining again. */
            zigbee_neighbor_table_delete(nb_item);
            IEEE802_15_4_SET_ASSOCIATION_PERMITTED(true);
        }
    } else {
        /* The ZigBee End-device should reset itself and go to idle. */
    }
    
    /* Now build the NLME_LEAVE_indication and call the appropriate callback. */
    zigbee_leave_indication_t nli_ind = zigbee_get_nlme_leave_indication();
    
    if (NULL == nli_ind) {
        return;
    }
    
    /* Try to allocate some memory to build the indication on. */
    nlme_leave_ind_t *nli = (nlme_leave_ind_t *)MEM_ALLOC(nlme_leave_ind_t);
    
    /* Verify that memory was allocated. */
    if (NULL == nli) {
        return;
    }
    
    /* Build indication message. */
    
    memcpy((void *)(&(nli->DeviceAddress)), (void *)(&(mdi->DeviceAddress)), sizeof(uint64_t));
    nli->Rejoin = false;
    
    /* Execute the callback. */
    nli_ind(nli);
    
    /* Release allocated memory. */
    MEM_FREE(nli);
}


static void mac_disassociate_confirm_callback(mlme_disassociate_conf_t *mdc) {
    /* Check if the MLME_DISASSOCIATE.request was initiated by a ZigBee Coordinator,
     * Router or End-device.
     */
    if (ZIGBEE_TYPE_DEVICE == NWK_NIB_GET_NODE_ROLE()) {
        /* ZigBee End-device that asked to be relinquished from the network. */
        
        /* Do a full node reset. */
        zigbee_nib_init();
        zigbee_neighbor_table_init();
        ZIGBEE_NWK_SET_STATE(NWK_IDLE);
    } else if (ZIGBEE_TYPE_COORD == NWK_NIB_GET_NODE_ROLE()) {
        ZIGBEE_NWK_SET_STATE(NWK_FORMED);
    } else {
        ZIGBEE_NWK_SET_STATE(NWK_STARTED);
    }
    
    memcpy((void *)(&(nwk_param.leave.confirm->DeviceAddress)), (void *)(&(nwk_param.leave.node_to_leave)), sizeof(uint64_t));
    nwk_param.leave.confirm->Status = mdc->status;
    
    nwk_param.leave.nlme_callback_leave_confirm(nwk_param.leave.confirm);
}
/*EOF*/
