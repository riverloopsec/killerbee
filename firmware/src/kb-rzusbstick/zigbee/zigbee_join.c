// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the NLME-JOIN primitives.
 *
 *         It is currently only possible to join the network through an 
 *         association procedure. Joining through orphan scan and rejoining 
 *         mechanism might be implemented at a later date
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
 * $Id: zigbee_join.c 41144 2008-04-29 12:42:28Z ihanssen $
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
#include "vrt_kernel.h"
#include "vrt_kernel_conf.h"
#include "vrt_mem.h"

#include "ieee802_15_4_const.h"
#include "ieee802_15_4_pib.h"

#include "zigbee.h"
#include "zigbee_internal_api.h"
#include "zigbee_const.h"
#include "zigbee_msg_types.h"
#include "zigbee_neighbor_table.h"
#include "zigbee_nib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief Evnet handler that is executedd when the IEEE 802.15.4 MAC reports
 *         the result after a MLME-ASSOCIATE.request.
 *
 *  \param[in] mac Pointer to MLME-ASSOCIATE.confirm message.
 */
static void mac_associate_confim_callback(mlme_associate_conf_t *mac);


bool zigbee_join_request(nlme_join_req_t *njr) {
    /* Perform sanity check on parameter set. */
    if (NULL == njr) {
        return false;
    }
    
    if (NULL == (njr->nlme_callback_join_confirm)) {
        return false;
    }
    
    if (NWK_JOIN_THROUGH_ASSOCIATION == (njr->RejoinNetwork)) {
        /* Association. */
        
        /* Verify that the device is in idle mode. */
        if (NWK_IDLE != ZIGBEE_NWK_GET_STATE()) {
            return false;
        }
        
        /* Serach the Neighbor Table for a potential item with the suggested
         * PAN ID.
         */
        bool join_candidate_found = false;
        
        zigbee_neighbor_table_item_t *nb_item = NEIGHBOR_TABLE_GET_FIRST();
        
        while ((true != NEIGHBOR_TABLE_IS_LAST_ELEMENT(nb_item)) && (true != join_candidate_found)) {
            
            if (((nb_item->ndesc.PanID) == (njr->PANId)) && (true == nb_item->ndesc.PermitJoining)) {
                join_candidate_found = true;
                break;
            } else {
                nb_item = NEIGHBOR_TABLE_GET_NEXT(nb_item);
            }
        }
        
        /* Verify that a candidate with the correct PAN ID and that does permit
         * joining was found.
         */
        if (true != join_candidate_found) {
            return false;
        }
        
        /* Build MLME_ASSOCIATE.request. */
        njr->mar.LogicalChannel = nb_item->ndesc.LogicalChannel;
        njr->mar.CoordAddrMode = WPAN_ADDRMODE_SHORT;
        njr->mar.CoordPANId = njr->PANId;
        njr->mar.CoordAddress = 0;
        njr->mar.CoordAddress = nb_item->NetworkAddress;
        
        uint8_t capabilities = 0;                  
        if (true == (njr->JoinAsRouter)) { capabilities |= (1 << 1); }
        if (0x01 == (njr->PowerSource))  { capabilities |= (1 << 2); }
        if (true == (njr->RxOnWhenIdle)) { capabilities |= (1 << 3); }
        
        njr->mar.CapabilityInformation = capabilities;
        
        njr->mar.mlme_associate_confirm = mac_associate_confim_callback;
        
        /* Set some of the internal storage pointers. */
        nwk_param.join.nlme_callback_join_confirm = njr->nlme_callback_join_confirm;
        nwk_param.join.njc = &(njr->njc);
        nwk_param.join.parent_address = njr->mar.CoordAddress;
        
        /* Issue request. */
        if (true != ieee802_15_4_associate_request(&(njr->mar))) {
            return false;
        } else {
            /* Set new NWK state. */
            return true;
        }
    } else if (NWK_JOIN_THROUGH_ORPHANINING == (njr->RejoinNetwork)) {
        /* Orphan Scan. */
        /* Not implemented yet. */
        return false;
    } else if (NWK_JOIN_THROUGH_REJOIN == (njr->RejoinNetwork)) {
        /* NWK Rejoin procedure. */
        /* Not implemented yet. */
        return false;
    } else {
        /* Unknown option. */
        return false;
    }
}


void zigbee_join_indication_do_callback(void *nji) {
    /* Verify that the */
    if (NULL == nji) {
        return;
    }
    
    /* Check if a callback is registered. */
    zigbee_join_indication_t nji_callback = zigbee_get_nlme_join_indication();
    if (NULL == nji_callback) {
        MEM_FREE(nji);
        return;
    }
    
    /* Execute the callback. */
    nji_callback((nlme_join_ind_t *)nji);
    MEM_FREE(nji);
}


/*                            IEEE 802.15.4 MAC Callbacks                     */


void mac_associate_indication_callback(mlme_associate_ind_t *mai) {
    /* Allcoate some memory to build the MLME_ASSOCIATE.response on. */
    mlme_associate_resp_t *response = (mlme_associate_resp_t *)MEM_ALLOC(mlme_associate_resp_t);
    
    if (NULL == response) {
        return;
    }
    
    memcpy((void *)(&(response->DeviceAddress)), (void *)(&(mai->DeviceAddress)), sizeof(uint64_t));
    nwk_param.join_ind.capability_information = mai->CapabilityInformation;
    
    /* Check if the device with this long address has been given a short address
     * already. That is this device is the parent.
     */
    zigbee_neighbor_table_item_t *child = zigbee_neighbor_table_find_long(mai->DeviceAddress);
    if (NULL != child) {
        response->AssocShortAddress = child->NetworkAddress;
        nwk_param.join_ind.allocted_address = child->NetworkAddress;
    
        response->status = ASSOCIATION_SUCCESSFUL;
    } else {
        /* Check if the joining device is an End device or Router. */
        if (((mai->CapabilityInformation) & (0x02)) != (0x02)) {
            /* Check if it is possible to join the end device. */
            child = zigbee_neighbor_table_add_device();
        } else {
            /* Check if it is possible to join the router. */
            child = zigbee_neighbor_table_add_router();
        }
        
        if (NULL == child) {
            response->AssocShortAddress = 0xFFFF;
            response->status = PAN_AT_CAPACITY;
            /* Also upate the permit joining PIB in the IEEE 802.15.4 MAC to
             * reflect that the system is now not to associate more devices.
             */
            IEEE802_15_4_SET_ASSOCIATION_PERMITTED(false);
        } else {
            response->AssocShortAddress = child->NetworkAddress;
            nwk_param.join_ind.allocted_address = child->NetworkAddress;
            response->status = ASSOCIATION_SUCCESSFUL;
        }
    }
    
    (bool)ieee802_15_4_associate_response(response);
    MEM_FREE(response);
}


void mac_comm_status_indication_callback(mlme_comm_status_ind_t *mcsi) {
    /* Only generate NLME_JOIN.indication if the association was successful. */
    if (MAC_SUCCESS != (mcsi->status)) {
        return;
    }
    
    /* Try to allocate some memory to build the indication on. */
    nlme_join_ind_t *nji = (nlme_join_ind_t *)MEM_ALLOC(nlme_join_ind_t);
    
    /* Verify that memory was allocated. */
    if (NULL == nji) {
        return;
    }
    
    /* Build the NLME_JOIN.indication. */
    nji->ShortAddress = nwk_param.join_ind.allocted_address;
    memcpy((void *)(&(nji->ExtendedAddress)), (void *)(&(mcsi->DstAddr)),sizeof(uint64_t));
    nji->CapabilityInformation = nwk_param.join_ind.capability_information;
    
    /* Add information about the newly added node in the Neighbor Table. */
    zigbee_neighbor_table_item_t *child = zigbee_neighbor_table_find(nwk_param.join_ind.allocted_address);
    if (NULL != child) { 
        memcpy((void *)(&(child->ExtendedAddress)), (void *)(&(mcsi->DstAddr)), sizeof(uint64_t));
        memcpy((void *)(&(child->NetworkAddress)), (void *)(&(nwk_param.join_ind.allocted_address)), sizeof(uint16_t));
    }
    
    /* Post event. */
    if (true != vrt_post_event(zigbee_join_indication_do_callback, (uint8_t *)nji)) {
        MEM_FREE(nji);
    }
}


static void mac_associate_confim_callback(mlme_associate_conf_t *mac) {
    /* Build NLME_JOIN.confirm. */
    nlme_join_conf_t *njc = nwk_param.join.njc;
    memcpy((void *)(&(njc->ShortAddress)), (void *)(&(mac->AssocShortAddress)), sizeof(uint16_t));
    memcpy((void *)(&(njc->PANId)), (void *)(&IEEE802_15_4_GET_PAN_ID()), sizeof(uint16_t));
    njc->Status = mac->status;
    
    /* Set state to NWK_JOINED if the association process was successful. */
    if (ASSOCIATION_SUCCESSFUL == (mac->status)) {
        NWK_NIB_SET_NWK_ADDRESS(mac->AssocShortAddress);
        NWK_NIB_SET_ADDRESS_INCREMENT(0);
        
        uint8_t depth = zigbee_neighbor_table_find_depth(IEEE802_15_4_GET_COORD_SHORT_ADDRESS(), \
                                                         IEEE802_15_4_GET_SHORT_ADDRESS());
        
        uint16_t c_skip = zigbee_nib_c_skip(depth - 1);
        NWK_NIB_SET_PARENT_ADDRESS_INCREMENT(c_skip); 
        
        ZIGBEE_NWK_SET_STATE(NWK_JOINED);
        NWK_NIB_SET_NODE_ROLE(ZIGBEE_TYPE_DEVICE);
    } else {
        /* Remove this node from the Neighbor Table. */
        zigbee_neighbor_table_item_t *nb = zigbee_neighbor_table_find(nwk_param.join.parent_address);
        if (NULL != nb) { zigbee_neighbor_table_delete(nb); }
    }
    
    /* Execute callback. */
    nwk_param.join.nlme_callback_join_confirm(njc);
}
/*EOF*/
