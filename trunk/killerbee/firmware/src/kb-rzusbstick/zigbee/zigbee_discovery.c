// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the NLME-DISCOVERY.request primitive
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
 * $Id: zigbee_discovery.c 41144 2008-04-29 12:42:28Z ihanssen $
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

#include "zigbee.h"
#include "zigbee_conf.h"
#include "zigbee_internal_api.h"
#include "zigbee_msg_types.h"
#include "zigbee_neighbor_table.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief Event handler executed when the IEEE 802.15.4 MAC is reporting back 
 *         the status of the associated MLME-SCAN.request primitive.
 *
 *  \param[in] msc Pointer to the MLME-SCAN.confirm message.
 */
static void mac_scan_confirm_callback(mlme_scan_conf_t *msc);

/*! \brief This event handler is executed when the IEEE 802.15.4 MAC is 
 *         reporting back that a Beacon Frame has been received.
 *
 *  \param[in] mnbi Pointer to MLME-BEACON-NOTIFY.indication message.
 *
 */
static void mac_beacon_notify_callback(mlme_beacon_notify_ind_t *mnbi);


bool zigbee_network_discovery_request(nlme_network_discovery_req_t *ndr) {
    /* Verify that the device is in IDLE state. */
    if (NWK_IDLE != ZIGBEE_NWK_GET_STATE()) {
        return false;
    }
    
    /* Perform sanity check on function parameters. */
    if (NULL == ndr) {
        return false;
    }
    
    if (NULL == (ndr->nlme_callback_discovery_confirm)) {
        return false;
    }
    
    /* Store NWK parameters. */
    nwk_param.discovery.nlme_callback_discovery_confirm = ndr->nlme_callback_discovery_confirm;
    nwk_param.discovery.ndc = &(ndr->ndc);
    
    /* Build and send MLME_SCAN.request. */
    mlme_scan_req_t *msr = &(ndr->msr);
    msr->ScanType = MLME_SCAN_TYPE_ACTIVE;
    msr->ScanChannel = ndr->ChannelToScan;
    msr->ScanDuration = ndr->ScanDuration; 
    msr->mlme_scan_confirm = mac_scan_confirm_callback;
    
    /* Set the MLME_BEACON_NOTIFY.indication callback. */
    ieee802_15_4_set_mlme_beacon_notify_indication(mac_beacon_notify_callback);
    
    /* Issue MLME_SCAN.request primitive. */
    bool discovery_status = false;
    if (true != ieee802_15_4_scan_request(msr)) {
    } else {
        ZIGBEE_NWK_SET_STATE(NWK_BUSY_DISCOVERING);
        discovery_status = true;
    }
    
    return discovery_status;
}


void zigbee_network_discovery_confirm_do_callback(void *ndc) {
    /* Cast the ndc to a NLME-NETWORK-DISCOVERY.confirm message. */
    
    ZIGBEE_NWK_SET_STATE(NWK_IDLE);
    
    /* Execute the event handler. */
    nwk_param.discovery.nlme_callback_discovery_confirm((nlme_network_discovery_conf_t *)ndc);
}


/*                     IEEE802.15.4 MAC Callbacks                             */


static void mac_scan_confirm_callback(mlme_scan_conf_t *msc) {
    /* Check result of the scan. If any PANs were found, there must be copied into
     * the Neighbor table.
     */
    
    /* Clear the MLME_BEACON_NOTIFY.indication callback. */
    ieee802_15_4_clear_mlme_beacon_notify_indication();
    
    /* Build the NLME_NETWORK_DISCOVERY.confirm messsage. This is done by simply
     * copying the Network Descriptors from the Neighbor Table.
     */
    
    nlme_network_discovery_conf_t *ndc = nwk_param.discovery.ndc;
    ndc->Status = msc->status;
    ndc->NetworkCount = NEIGHBOR_TABLE_GET_SIZE();
        
    network_desc_t * nd_list = (network_desc_t *)(&(ndc->NetworkDescriptor[0]));
    
    for(zigbee_neighbor_table_item_t *nb_item = NEIGHBOR_TABLE_GET_FIRST(); \
        true != NEIGHBOR_TABLE_IS_LAST_ELEMENT(nb_item); nb_item = NEIGHBOR_TABLE_GET_NEXT(nb_item)) {
        memcpy((void *)(nd_list), (void *)(&(nb_item->ndesc)), sizeof(network_desc_t));
        nd_list++;
    }
    
    /* Post the confirm event. */
    (bool)vrt_post_event(zigbee_network_discovery_confirm_do_callback, (uint8_t *)(ndc));
}


static void mac_beacon_notify_callback(mlme_beacon_notify_ind_t *mnbi) {
    /* Check that short address is used, and if the node with this beacon is 
     * already known.
     */
    if (WPAN_ADDRMODE_SHORT != (mnbi->PANDescriptor.CoordAddrMode)) {
        return;
    }
    
    if (NULL != zigbee_neighbor_table_find(mnbi->PANDescriptor.CoordAddress)) {
        return;
    }
    
    /* Add PANDescriptor into the neighbor table. */
    zigbee_neighbor_table_item_t *nbi = zigbee_neighbor_table_get();
        
    /* Verify that an item was available. */
    if (NULL == nbi) {
        zigbee_neighbor_table_put(nbi);
        return;
    }
    
    /* Transfer all items to the Network table. */
    ieee802_15_4_pan_descriptor_t *pd = &(mnbi->PANDescriptor);
    
    nbi->ndesc.PanID = pd->CoordPANId;
    nbi->ndesc.ExtendedPanID = 0;
    nbi->ndesc.LogicalChannel = pd->LogicalChannel;;
    nbi->ndesc.StackProfile = 0;
    nbi->ndesc.ZigBeeVersion = 0;
    
    uint16_t superframe_spec = pd->SuperframeSpec;
    
    nbi->ndesc.BeaconOrder = ((uint8_t)superframe_spec) & 0x0F;
    nbi->ndesc.SuperframeOrder = (((uint8_t)superframe_spec) >> 4) & 0x0F;;
    
    if (((superframe_spec & (1U << 15)) != (1U << 15))) {
        nbi->ndesc.PermitJoining = false;
    } else {
        nbi->ndesc.PermitJoining = true;
    }
    
    nbi->ExtendedAddress = 0;
    nbi->NetworkAddress = pd->CoordAddress;
    nbi->DeviceType = ZIGBEE_TYPE_DEVICE;
    nbi->RxOnWhenIdle = false;
    nbi->Relationship = ZIGBEE_REL_UNKNOWN;
    nbi->Depth = NWK_MAX_DEPTH;
    nbi->TransmitFailure = 0;
    nbi->PotentialParent = true;
    nbi->LQI = pd->LinkQuality;
    
    /* Add Neighbor Table item. */
    zigbee_neighbor_table_append(nbi);
}
/*EOF*/
