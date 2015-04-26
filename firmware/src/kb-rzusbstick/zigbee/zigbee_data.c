// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the NLDE-DATA primitives
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
 * $Id: zigbee_data.c 41759 2008-05-15 15:09:45Z vkbakken $
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
#include "vrt_mem.h"

#include "ieee802_15_4_const.h"
#include "ieee802_15_4_pib.h"
#include "zigbee.h"
#include "zigbee_conf.h"
#include "zigbee_internal_api.h"
#include "zigbee_const.h"
#include "zigbee_msg_types.h"
#include "zigbee_nib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static void (*nlde_callback_data_confirm)(nlde_data_conf_t* ndc) = NULL;
static nlde_data_conf_t *confirm;

static mcps_data_req_t *relay;
/*================================= PROTOTYPES       =========================*/

/*! \brief Callback executed when the IEEE 802.15.4 MAC is reporting back the 
 *         status of the associated MCPS_DATA.request primitive.
 *
 *  \param[in] mdc Pointer to the MCPS-DATA.confirm message.
 */
static void mac_data_confirm_callback(mcps_data_conf_t *mdc);

/*! \brief Event handler executed when the MCPS-DATA.confirm message is sent
 *         from the IEEE 802.15.4 during a relay procedure.
 *
 *  \param[in] mdc Pointer to MCPS-DATA.confirm message.
 */
static void relay_confirm(mcps_data_conf_t *mdc);

/*! \brief This function is used to calculate the next hop address when doing 
 *         tree routing.
 *
 *  \param[in] destination Short address of the destination for this particular frame.
 *  \returns A number between 0 and 0xFFFF, where 0xFFFF indicates an error.
 */
static uint16_t find_next_address(uint16_t destination);


bool zigbee_data_request(nlde_data_req_t *ndr) {
    /* Check that the NWK status is either: Formed, started or joined. */
    if ((NWK_FORMED  != ZIGBEE_NWK_GET_STATE()) &&
        (NWK_STARTED != ZIGBEE_NWK_GET_STATE()) && 
        (NWK_JOINED  != ZIGBEE_NWK_GET_STATE())) {
        return false;
    }
    
    /* Perform sanity check on function parameters. */
    if (NULL == ndr) {
        return false;
    }
    
    if (NULL == (ndr->nsdu)) {
        return false;
    }
    
    if (NULL == (ndr->nlde_callback_data_confirm)) {
        return false;
    }
    
    if (IEEE802_15_4_GET_SHORT_ADDRESS() == (ndr->dst_addr)) {
        return false;
    }
    
    /* Verify that the payload to be transmitted is within the defined bounds and 
     * not zero.
     */
    if (0 == (ndr->nsdu_length)) {
        return false;
    }
    
    if ((aMaxMACFrameSize - NWK_MIN_HEADER_OVERHEAD) < (ndr->nsdu_length)) {
        return false;
    }
    
    /* Try to allocate some memory to build the frame on. */
    uint8_t *nwk_frame = (uint8_t *)MEM_ALLOC_ARRAY(uint8_t, ((ndr->nsdu_length) + NWK_MIN_HEADER_OVERHEAD));
    
    if (NULL == nwk_frame) {
        return false;
    }
    
    /* 1. Build the NWK layer FCF and add to the frame. */
    uint16_t fcf = NWK_FCF_SET_FRAME_TYPE(NWK_FCF_FRAMETYPE_DATA) | NWK_FCF_SET_PROTOCOL_VERSION(1);
    
    uint8_t index = 0;
    
    nwk_frame[index++] = ((fcf >> 8*0) & 0xFF);
    nwk_frame[index++] = ((fcf >> 8*1) & 0xFF);
    
    /* 2. Add destination address. */
    memcpy((void *)(&nwk_frame[index]), (void *)(&(ndr->dst_addr)), sizeof(uint16_t));
    index += sizeof(uint16_t);
    
    /* 3. Add source address. */
    memcpy((void *)(&nwk_frame[index]), (void *)(&IEEE802_15_4_GET_SHORT_ADDRESS()), sizeof(uint16_t));
    index += sizeof(uint16_t);
    
    /* 4. Next is the radius field. If the radius fiels of the request message
     *    unequal from zero it will be used. Otherwise, the specification says that
     *    it should be set to twice the value of the nwkMaxDepth attribute.
     */
    if (0 == (ndr->radius)) {
        nwk_frame[index++] = (2 * ZIGBEE_MAX_NWK_DEPTH);
    } else {
        nwk_frame[index++] = ndr->radius;
    }
    
    /* 5. Add NWK layer sequence number. */
    nwk_frame[index++] = NWK_NIB_GET_SEQUENCE_NMBR();
    
    /* 6. Copy the user defined data. */
    memcpy((void *)(&nwk_frame[index]), (void *)(ndr->nsdu), ndr->nsdu_length);
    index += (ndr->nsdu_length);
    
    /* TODO: Use tree routing algorithm to find the first hop destination address. */
    uint16_t next_hop_address = find_next_address(ndr->dst_addr);

    /* Could not find a valid address for the next hop. */
    if (0xFFFF == next_hop_address) {
        /* Free allocated memory. */
        MEM_FREE(nwk_frame);
        return false;
    }
    
    /* 7. Build MCPS_DATA.request. */
    ndr->mcps.SrcAddrMode       = WPAN_ADDRMODE_SHORT;
    ndr->mcps.SrcAddr           = IEEE802_15_4_GET_SHORT_ADDRESS();
    ndr->mcps.DstAddrMode       = WPAN_ADDRMODE_SHORT;
    ndr->mcps.DstPANId          = IEEE802_15_4_GET_PAN_ID();
    ndr->mcps.DstAddr           = next_hop_address;
    ndr->mcps.TxOptions         = WPAN_TXOPT_ACK;
    ndr->mcps.msduLength        = index;
    ndr->mcps.data              = nwk_frame;
    ndr->mcps.mcps_data_confirm = mac_data_confirm_callback;
        
    /* Set NWK layer callbacks. */
    nlde_callback_data_confirm = ndr->nlde_callback_data_confirm;
    confirm = &(ndr->confirm);

    /* 8. Issue primitive. */
    bool data_request_status = false;
    if (true != ieee802_15_4_data_request(&(ndr->mcps))) {
    } else {
        data_request_status = true;
    }
    
    /* Free allocated memory. */
    MEM_FREE(nwk_frame);
    
    return data_request_status;
}


void zigbee_data_indication_do_callback(void *ndi) {
    /* Verify that the */
    if (NULL == ndi) {
        return;
    }
    
    /* Check if a callback is registered. */
    zigbee_data_indication_t ndi_callback = zigbee_get_nlde_data_indication();
    if (NULL == ndi_callback) {
        MEM_FREE(ndi);
        return;
    }
    
    /* Execute the callback. */
    ndi_callback((nlde_data_ind_t *)ndi);
    MEM_FREE(ndi);
}


static void mac_data_confirm_callback(mcps_data_conf_t *mdc) {
    /* Post NLDE_DATA.confirm event. */
    if (NULL == mdc) {
        return;
    }
    
    if (NULL == nlde_callback_data_confirm) {
        return;
    }
    
    if (NULL == confirm) {
        return;
    }
    
    confirm->Status = mdc->status;
    nlde_callback_data_confirm(confirm);
}


static void relay_confirm(mcps_data_conf_t *mdc) {
    /* Free previously allocated memory. */
    MEM_FREE(relay);
    
    /* Go back to RX_AACK_ON. */
    ieee802_15_4_rx_enable();
}


/*                         IEEE 802.15.4 MAC Callback.                        */


void mac_data_indication_callback(mcps_data_ind_t *mcdi) {
    /* Must at least be enough data available to extract the NWK layer header. */
    if (NWK_MIN_HEADER_OVERHEAD > (mcdi->msduLength)) { return; }
    
    uint8_t *nwk_header = mcdi->msdu;
    
    uint16_t fcf = (nwk_header[0] << 8 * 0); // Read FCF LSB.
    fcf         |= (nwk_header[1] << 8 * 1); // Read FCF MSB.
    
    uint16_t nwk_dest_addr = (nwk_header[2] << 8 * 0); // Read Destination Address LSB.
    nwk_dest_addr         |= (nwk_header[3] << 8 * 1); // Read Destination Address MSB.;
    
    if (nwk_dest_addr != IEEE802_15_4_GET_SHORT_ADDRESS()) {
        /* Route to another device. */
        /* Route along the three. */
        /* Update destination MAC destination address and send. */
        
        relay = (mcps_data_req_t *)MEM_ALLOC(mcps_data_req_t);
        /* Verify that memory was allocated. */
        if (NULL == relay) {
            /* Go back to RX_AACK_ON. */
            ieee802_15_4_rx_enable();
            return;
        }
        
        /* TODO: Use tree routing algorithm to find the first hop destination address. */
        /* Send top parent or to one of the childs if this is a coordinator or router. */
        /* For now always send to parent. */
        uint16_t next_hop_address = find_next_address(nwk_dest_addr);
    
        /* Could not find a valid address for the next hop. */
        if (0xFFFF == next_hop_address) {
            /* Free allocated memory. */
            MEM_FREE(relay);
            
            /* Go back to RX_AACK_ON. */
            ieee802_15_4_rx_enable();
            return;
        }
    
        /* 7. Build MCPS_DATA.request. */
        relay->SrcAddrMode       = WPAN_ADDRMODE_SHORT;
        relay->SrcAddr           = IEEE802_15_4_GET_SHORT_ADDRESS();
        relay->DstAddrMode       = WPAN_ADDRMODE_SHORT;
        relay->DstPANId          = IEEE802_15_4_GET_PAN_ID();
        relay->DstAddr           = next_hop_address;
        relay->TxOptions         = WPAN_TXOPT_ACK;
        relay->msduLength        = mcdi->msduLength;
        relay->data              = nwk_header;
        relay->mcps_data_confirm = relay_confirm;

        /* 8. Issue primitive. */
        if (true != ieee802_15_4_data_request(relay)) {
            MEM_FREE(relay);
        }
    } else {
        /* Message is for this device. */
        
        /* Allocate memory to build and NLDE_DATA.indication message. */
        /* Try to allocate some memory to build the NLDE_DATA.indication message. */
        nlde_data_ind_t *ndi = (nlde_data_ind_t *)MEM_ALLOC_ARRAY(uint8_t, sizeof(nlde_data_ind_t) + (mcdi->msduLength) - NWK_MIN_HEADER_OVERHEAD);
        if (NULL == ndi) { return; }
        
        /* Build indication. */
        ndi->DstAddrMode = 0x01;
        
        memcpy((void *)(&(ndi->DstAddr)), (void *)(&nwk_dest_addr), sizeof(uint16_t));
        
        uint16_t nwk_src_addr = (nwk_header[4] << 8 * 0); // Read Source Address LSB.
        nwk_src_addr         |= (nwk_header[5] << 8 * 1); // Read Source Address MSB.;
        
        memcpy((void *)(&(ndi->SrcAddr)), (void *)(&nwk_src_addr), sizeof(uint16_t));
        
        ndi->LinkQuality = mcdi->mpduLinkQuality;
        ndi->NsduLength = (mcdi->msduLength) - NWK_MIN_HEADER_OVERHEAD;
        memcpy((void *)(ndi->Nsdu), (void *)(&nwk_header[8]), ((mcdi->msduLength) - NWK_MIN_HEADER_OVERHEAD));
        
        /* Post this event. */
        if (true != vrt_post_event(zigbee_data_indication_do_callback, (uint8_t *)ndi)) {
            MEM_FREE(ndi);
        }
    }
    
    /* Go back to RX_AACK_ON. */
    ieee802_15_4_rx_enable();
}


static uint16_t find_next_address(uint16_t destination) {
    uint16_t next_hop_address = 0xFFFF;
    
    /* Check if the destination is a descendant of this node. */
    if (0x0000 == IEEE802_15_4_GET_SHORT_ADDRESS()) {
        /* Coordinator: All devices are descendants. */
        /* Destination address is a descendant. */
        if (destination <= ((ZIGBEE_MAX_NMBR_OF_ROUTERS * NWK_NIB_GET_ADDRESS_INCREMENT()))) {
            /* As found in the ZigBee specification section 3.7.3.3. */
            next_hop_address = 1 + (((destination - 1) / NWK_NIB_GET_ADDRESS_INCREMENT()) * NWK_NIB_GET_ADDRESS_INCREMENT());
        } else {
            next_hop_address = destination;
        }
    } else if ((destination > IEEE802_15_4_GET_SHORT_ADDRESS()) && 
               (destination < (IEEE802_15_4_GET_SHORT_ADDRESS() + NWK_NIB_GET_PARENT_ADDRESS_INCREMENT()))) {
        /* Destination address is a descendant. */
        if (destination <= (IEEE802_15_4_GET_SHORT_ADDRESS() + (ZIGBEE_MAX_NMBR_OF_ROUTERS * NWK_NIB_GET_ADDRESS_INCREMENT()))) {
            /* As found in the ZigBee specification section 3.7.3.3. */
            next_hop_address = (IEEE802_15_4_GET_SHORT_ADDRESS() + 1 + 
            ((uint16_t)(destination - (IEEE802_15_4_GET_SHORT_ADDRESS() + 1)) / NWK_NIB_GET_ADDRESS_INCREMENT()) * NWK_NIB_GET_ADDRESS_INCREMENT());
        } else {
            next_hop_address = destination;
        }
    } else {
        /* Send frame to parent node. */
        next_hop_address = IEEE802_15_4_GET_COORD_SHORT_ADDRESS();
    }
        
    return next_hop_address;
}
/*EOF*/
