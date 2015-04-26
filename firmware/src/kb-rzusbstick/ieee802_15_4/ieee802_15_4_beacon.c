// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the IEEE 802.15.4 standard's handling of beacon
 *         frames
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
 * $Id: ieee802_15_4_beacon.c 41144 2008-04-29 12:42:28Z ihanssen $
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

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_internal_api.h"
#include "ieee802_15_4_pib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

void ieee802_15_4_mlme_beacon_indication(void *beacon_frame_info) {
    /* Handle the beacon: Issue a MLME_BEACON_NOTIFICATION.indication and 
     * add the pan descriptor to the list if the device is busy doing a
     * active or passive scan.*/
    
    /* Cast the beacon frame info. */
    ieee802_15_4_meta_data_t *frame_info = (ieee802_15_4_meta_data_t *)beacon_frame_info;
    
    /* Build PANDescriptor. This will be used both if the device is running an
     * active or passive scan. It is here safe to use the IEEE 802.15.4 MAC's
     * internal command buffer to build the PAN Descriptor.
     */
    ieee802_15_4_pan_descriptor_t *pan_desc = (ieee802_15_4_pan_descriptor_t *)IEEE802_15_4_GET_TX_BUFFER();
    
    pan_desc->CoordAddrMode = frame_info->src_addr_mode;
    pan_desc->CoordPANId = frame_info->src_pan_id;
    pan_desc->CoordAddress = 0;
    
    memcpy((void *)(&(pan_desc->CoordAddress)), (void *)(&(frame_info->src_addr)), sizeof(uint64_t));
    
    pan_desc->LogicalChannel = IEEE802_15_4_GET_CHANNEL();
    
    uint8_t *beacon_payload = frame_info->msdu;
    
    pan_desc->SuperframeSpec = 0;
    pan_desc->SuperframeSpec = beacon_payload[1] << 8;
    pan_desc->SuperframeSpec |= beacon_payload[0];
    
    pan_desc->LinkQuality = frame_info->lqi;
    
    /* Do scan stuff herein. */
    if (IEEE_MAC_BUSY_SCANNING == IEEE802_15_4_GET_STATE()) {
        /* First set a start pointer to the first element in the MLME_SCAN.confirm's
         * resultList.
         */
        mlme_scan_conf_t *msc = mac_param.scan.msc;
        
        ieee802_15_4_pan_descriptor_t *pan_desc_ptr = \
        (ieee802_15_4_pan_descriptor_t *)(&(msc->pan_desc[0]));
        
        /* We now can compare the beacon frame data with the contents of our
         * pan descriptor storage and determine if we save the pan descriptor data.
         * A PAN is considered the same as an existing one if all, the PAN Id, the
         * coordinator address mode, the coordinator address, and the Logical Channel
         * compare equal.
         */
        bool known_pan = false;
        for(uint8_t i = 0; i < (msc->ResultListSize); i++, pan_desc_ptr++) {
            if ((pan_desc->CoordPANId == pan_desc_ptr->CoordPANId)
                && ((pan_desc->CoordAddrMode) == (pan_desc_ptr->CoordAddrMode))
                && ((pan_desc->CoordAddress) == (pan_desc_ptr->CoordAddress))
                && ((pan_desc->LogicalChannel) == (pan_desc_ptr->LogicalChannel))) {
                /* Found a new beacon. */
                known_pan = true;
                break;
            }
        }
        
        /* Check if a new PAN was found and if there is room to store it. */
        if((true != known_pan) && ((msc->ResultListSize) < IEEE802_15_4_MAX_NMBR_OF_PANDESC)) {
            memcpy((void *)pan_desc_ptr, (void *)pan_desc, sizeof(ieee802_15_4_pan_descriptor_t));
            (msc->ResultListSize)++;
        }
    }
    
    /* In all cases (pan or device) if the payload is not equal to zero
     * or macAutoRequest is false, then we have to generate a 
     * MLME_BEACON_NOTIFY.indication.
     */
    if((frame_info->msdu_length > 0)/* || (mac_pib_macAutoRequest == false)*/) {
        /* Try to allocate some memory to build the MLME_BEACON_NOTIFICATION.indication on. */
        mlme_beacon_notify_ind_t *mbni = (mlme_beacon_notify_ind_t *)MEM_ALLOC(mlme_beacon_notify_ind_t);
        
        /* Verify that the memory was allocated before continuing execution. */
        if (NULL == mbni) {
        } else {
            /* Build MLME_BEACON_NOTIFIY.indication message and post the
             * associated event.
             */
            mbni->BSN = frame_info->seq_nmbr;
            memcpy((void *)(&(mbni->PANDescriptor)), (void *)pan_desc, sizeof(ieee802_15_4_pan_descriptor_t));
            
            if (true != vrt_post_event(ieee802_15_4_beacon_notify_do_callback, (void *)mbni)) {
                MEM_FREE(mbni);
            }
        }
    }
    
    if (true == IEEE802_15_4_GET_RX_ON_WHEN_IDLE() ||
        (IEEE_MAC_BUSY_SCANNING == IEEE802_15_4_GET_STATE())) {
        (bool)tat_set_state(RX_AACK_ON);
    }
}


void ieee802_15_4_send_beacon(void) {
    /* Devices are only allowed to send beacons if their short adddres is different
     * from the broadcast address (0xFFFF).*/
    if (BROADCAST == (IEEE802_15_4_GET_SHORT_ADDRESS())) { return; }
    
    /* Get hold of some memory to build the beacon on. */
    uint8_t *beacon  = IEEE802_15_4_GET_TX_BUFFER();
    
    /* 1. Build FCF. */
    uint8_t index = 0;
    uint16_t fcf;
    
    fcf = FCF_SET_SOURCE_ADDR_MODE(FCF_SHORT_ADDR) |
          FCF_SET_FRAMETYPE(FCF_FRAMETYPE_BEACON);

    // Add FCF fields.
    beacon[index++] = ((fcf >> 8*0) & 0xFF);
    beacon[index++] = ((fcf >> 8*1) & 0xFF);

    /* 2. Add farme sequence number. */
    beacon[index++] = IEEE802_15_4_GET_DSN();
    
    /* 3. Add addressing fields. */
    beacon[index++] = ((IEEE802_15_4_GET_PAN_ID() >> 8*0) & 0xFF);
    beacon[index++] = ((IEEE802_15_4_GET_PAN_ID() >> 8*1) & 0xFF);
    
    beacon[index++] = ((IEEE802_15_4_GET_SHORT_ADDRESS() >> 8*0) & 0xFF);
    beacon[index++] = ((IEEE802_15_4_GET_SHORT_ADDRESS() >> 8*1) & 0xFF);
    
    /* 4. Build and add super frame specification. */
    uint16_t superframe_spec = 0x0F;
    superframe_spec |= (0x0F << 4);
    superframe_spec |= (0x0F << 8);
    
    if (true) {
        superframe_spec |= (1U << 13);
    }
    
    if (true == IEEE802_15_4_GET_I_AM_COORDINATOR()) {
        superframe_spec |= (1U << 14);
    }
    
    if (true == IEEE802_15_4_GET_ASSOCIATION_PERMITTED()) {
        superframe_spec |= (1U << 15);
    }
    
    memcpy((void *)(&beacon[index]), (void *)(&superframe_spec), sizeof(uint16_t));
    index += sizeof(uint16_t);
    
    /* 5. No GTS fields. */
    beacon[index++] = 0;
    /* 6. No pending data or addresses. */
    beacon[index++] = 0;
    
    index += CRC_SIZE;
    
    /* Send beacon. */
    if (true != ieee802_15_4_send(index, beacon)) {
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_BEACON_ACK);
    }
}


void ieee802_15_4_process_beacon_ack(void *tx_status) {
    /* Do not care about the transmission status. Just take the radio transceiver 
     * back to RX_AACK_ON, set the callback handler and state back to started.
     */
    (bool)tat_set_state(RX_AACK_ON);
    IEEE802_15_4_SET_STATE(IEEE_MAC_STARTED);
}


void ieee802_15_4_beacon_notify_do_callback(void *mbni) {
    ieee802_15_4_beacon_notify_indication_t mbni_callback = ieee802_15_4_get_mlme_beacon_notify_indication();
    if (NULL == mbni_callback) {
        MEM_FREE(mbni);
        return;
    }
    
    /* Execute the callback and free up allocated memory. */
    mbni_callback((mlme_beacon_notify_ind_t *)mbni);
    MEM_FREE(mbni);
}
/*EOF*/
