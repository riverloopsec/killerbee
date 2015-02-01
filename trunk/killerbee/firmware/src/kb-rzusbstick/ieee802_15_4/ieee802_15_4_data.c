// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the MCPS-DATA primitives
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
 * $Id: ieee802_15_4_data.c 41760 2008-05-15 15:44:44Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "compiler.h"
#include "vrt_kernel.h"
#include "vrt_kernel_conf.h"
#include "vrt_mem.h"

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
static ieee802_15_4_state_t original_mac_state = IEEE_MAC_IDLE;
static mcps_data_conf_t *confirm_ptr;
static void (*mcps_data_confirm)(mcps_data_conf_t *mdc) = NULL;
/*================================= PROTOTYPES       =========================*/

bool ieee802_15_4_data_request(mcps_data_req_t *mdr) {
    /* Check first that the MAC is in a state where it is allowed to send
     * data. A coordinator must be started and a router or device must be associated.
     */
    if ((IEEE_MAC_STARTED != IEEE802_15_4_GET_STATE()) && 
        (IEEE_MAC_ASSOCIATED != IEEE802_15_4_GET_STATE())) { return false; }
    
    /* Perform sanity check on function parameters. */
    if (NULL == mdr) { return false; }
    if (NULL == (mdr->data)) { return false; }
    if (NULL == (mdr->mcps_data_confirm)) { return false; }
    
    // Check that the source address mode is a valid one.
    if ((mdr->SrcAddrMode != WPAN_ADDRMODE_NONE)  && 
        (mdr->SrcAddrMode != WPAN_ADDRMODE_SHORT) &&
        (mdr->SrcAddrMode != WPAN_ADDRMODE_LONG)) { return false; }
    
    // Check that the destination address mode is a valid one.
    if ((mdr->DstAddrMode != WPAN_ADDRMODE_NONE)  && 
        (mdr->DstAddrMode != WPAN_ADDRMODE_SHORT) && 
        (mdr->DstAddrMode != WPAN_ADDRMODE_LONG)) { return false; }
    
    // At least one address must be available.
    if ((mdr->SrcAddrMode == WPAN_ADDRMODE_NONE) && 
        (mdr->DstAddrMode == WPAN_ADDRMODE_NONE)) { return false; }
    
    // Check whether somebody requests an ACK of broadcast frame. Not allowed.
    if (((mdr->TxOptions & ACK_TRANSMISSION) == ACK_TRANSMISSION) && 
        (mdr->DstAddrMode == FCF_SHORT_ADDR) && 
        (mdr->DstAddr == BROADCAST)) { return false; }
    
    // Check whether source or destination address is set to reserved values.
    if ((mdr->SrcAddrMode == FCF_RESERVED_ADDR) || 
        (mdr->DstAddrMode == FCF_RESERVED_ADDR)) { return false; }
    
    /* Set some internal variables that is important in the following execution. */
    mcps_data_confirm = mdr->mcps_data_confirm;
    confirm_ptr = &(mdr->mdc);
    original_mac_state = IEEE802_15_4_GET_STATE();
    
    /* Verify that the data length is valid. */
    if (0 == (mdr->msduLength)) { return false; }
    if (aMaxMACFrameSize < (mdr->msduLength)) { return false; }
    
    // Only acknowledged or unacknowledged transmissions supported.
    if (((mdr->TxOptions & ACK_TRANSMISSION) != ACK_TRANSMISSION) && 
        ((mdr->TxOptions & ACK_TRANSMISSION) != 0)) { return false; }
    
    /* Get internal TX buffer to build the frame in. */
    uint8_t *frame_buffer = IEEE802_15_4_GET_TX_BUFFER();
    
    /* 1. Build the FCF field and add to the buffer. */
    uint16_t fcf;
    fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_DATA);

    if (mdr->TxOptions & ACK_TRANSMISSION) { fcf |= FCF_ACK_REQUEST; }
    
    fcf |= FCF_INTRA_PAN;
    
    fcf |= FCF_SET_SOURCE_ADDR_MODE(mdr->SrcAddrMode);
    fcf |= (FCF_SET_DEST_ADDR_MODE(mdr->DstAddrMode));
    
    uint8_t index = 0;
    
    frame_buffer[index++] = ((fcf >> 8*0) & 0xFF);
    frame_buffer[index++] = ((fcf >> 8*1) & 0xFF);
    
    /* 2. Add DSN field. */
    frame_buffer[index++] = IEEE802_15_4_GET_DSN();
    
    /* 3. Add destination address. */
    if (FCF_NO_ADDR != (mdr->DstAddrMode)) {
        frame_buffer[index++] = (((mdr->DstPANId) >> 8*0) & 0xFF);
        frame_buffer[index++] = (((mdr->DstPANId) >> 8*1) & 0xFF);
        
        if(FCF_LONG_ADDR == (mdr->DstAddrMode)) {
            memcpy((void *)(&frame_buffer[index]), (void *)(&(mdr->DstAddr)), sizeof(uint64_t));
            index += sizeof(uint64_t);
        } else {
            memcpy((void *)(&frame_buffer[index]), (void *)(&(mdr->DstAddr)), sizeof(uint16_t));
            index += sizeof(uint16_t);
        }
    }
    
    /* 4. Add source address. */
    if (FCF_NO_ADDR  != (mdr->SrcAddrMode)) {
        if(FCF_LONG_ADDR == (mdr->SrcAddrMode)) {
            memcpy((void *)(&frame_buffer[index]), (void *)(&(mdr->SrcAddr)), sizeof(uint64_t));
            index += sizeof(uint64_t);
        } else {
            memcpy((void *)(&frame_buffer[index]), (void *)(&(mdr->SrcAddr)), sizeof(uint16_t));
            index += sizeof(uint16_t);
        }
    }
    
    /* 5. Copy user data into frame buffer. */
    memcpy((void *)(&frame_buffer[index]), (void *)(mdr->data), mdr->msduLength);
    index += mdr->msduLength;
    
    /* 6. Add dummy bytes to the frame length (Will be overwritten by the CRC 
     *    calculated by the radio transceiver). 
     */
    index += CRC_SIZE;
    
    /* 7. Send data. */
    bool mcdr_status = false;
    if (true != ieee802_15_4_send(index, frame_buffer)) {
        /* Frame could not be sent. Go back to RX_AACK_ON or to SLEEP. */
        tat_reset();
        if (true != IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
            tat_go_to_sleep();
        } else {
            (bool)tat_set_state(RX_AACK_ON);
        }
    } else {
        /* Change internal state to busy transmitting. */
        IEEE802_15_4_SET_STATE(IEEE_MAC_TX_WAITING_ACK);
        mcdr_status = true;
    }
    
    return mcdr_status;
}


void ieee802_15_4_data_process_ack(void *tx_status) {
    /* Set the correct confirm status. */
    uint8_t status = *((uint8_t *)tx_status);
    if ((TRAC_SUCCESS == status) || (TRAC_SUCCESS_DATA_PENDING == status)) {
        status = MAC_SUCCESS;
    } else if (TRAC_NO_ACK == status) {
        status = MAC_NO_ACK;
    } else {
        status = MAC_CHANNEL_ACCESS_FAILURE;
    }
    
    /* Set the status variable that will be used in the callback. */
    confirm_ptr->status = status;
        
    /* The last transitted frame has been acknowledged. The user is notified 
     * through the ieee802_15_4_data_confirm callback. 
     */
    mcps_data_confirm(confirm_ptr);
    
    /* Take the radio transceiver to sleep or RX_AACK_ON depending on the 
     * RX_ON_WHEN_IDLE flag. The device will be in TRX_OFF when coming here.
     */
    if (true != IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
        tat_go_to_sleep();
    } else {
        tat_reset();
        (bool)tat_set_state(RX_AACK_ON);
    }
    
    /* Set back the state to either MAC_ASSOCIATED or MAC_STATRTED. */
    IEEE802_15_4_SET_STATE(original_mac_state);
}


void ieee802_15_4_process_data_indication(void *mdi_frame_info) {
    /* Cast the received pointer back to a vrt_packet. */
    ieee802_15_4_meta_data_t *ind_information = (ieee802_15_4_meta_data_t *)mdi_frame_info;
    
    /* Build MCPS_DATA.indication: Allocate memory to build on. */
    mcps_data_ind_t *indication = (mcps_data_ind_t *)MEM_ALLOC_ARRAY(uint8_t, sizeof (mcps_data_ind_t) + (ind_information->msdu_length));
    
    if (NULL == indication) {
        
        /* Go back to RX_AACK_ON if it was not possible to allocate memory, but first
         * wait for the PLL_ON state to be entered.
         */
        do {
            delay_us(20);
        } while (CMD_PLL_ON != tat_get_state());
        
        (bool)tat_set_state(RX_AACK_ON);
        
        return;
    }
    
    /* Set source addressing items. */
    indication->SrcAddrMode = ind_information->src_addr_mode;
    indication->SrcAddr = 0;
    if (FCF_LONG_ADDR== (indication->SrcAddrMode)) {
        memcpy((void *)(&(indication->SrcAddr)), (void *)(&(ind_information->src_addr)), sizeof(uint64_t));
    } else {
        memcpy((void *)(&(indication->SrcAddr)), (void *)(&(ind_information->src_addr)), sizeof(uint16_t));
    }
    
    /* Set PAN ID. */
    memcpy((void *)(&(ind_information->dst_pan_id)), (void *)(&(indication->DstPANId)), sizeof(uint16_t));
    
    /* Set destination addressing items. */
    indication->DstAddrMode = ind_information->dst_addr_mode;
    indication->DstAddr = 0;
    if (FCF_LONG_ADDR== (indication->DstAddrMode)) {
        memcpy((void *)(&(indication->DstAddr)), (void *)(&(ind_information->dst_addr)), sizeof(uint64_t));
    } else {
        memcpy((void *)(&(indication->DstAddr)), (void *)(&(ind_information->dst_addr)), sizeof(uint16_t));
    }
    
    /* Set the msdu fields. */
    indication->mpduLinkQuality = ind_information->lqi;
    indication->msduLength      = ind_information->msdu_length;
    memcpy((void *)(indication->msdu), (void *)(ind_information->msdu), ind_information->msdu_length);
    
    /* Post indication event. */
    if (true != vrt_post_event(ieee802_15_4_data_indication_do_callback, (void *)indication)) {
        /* Free up allocated memory. */
        MEM_FREE(indication);
        
        /* Go back to RX_AACK_ON if it was not possible to post the event handler, but first
         * wait for the PLL_ON state to be entered.
         */
        do {
            delay_us(20);
        } while (CMD_PLL_ON != tat_get_state());
        
        (bool)tat_set_state(RX_AACK_ON);
    }
}


void ieee802_15_4_data_indication_do_callback(void *mdi) {
    ieee802_15_4_data_indication_t mdi_callback = ieee802_15_4_get_mcps_data_indication();
    
    do {
        delay_us(20);
    } while (CMD_PLL_ON != tat_get_state());
    
    if (NULL == mdi_callback) {
        (bool)tat_set_state(RX_AACK_ON);
        
        return;
    }
    
    /* Execute the callback. */
    mdi_callback((mcps_data_ind_t *)mdi);
    MEM_FREE(mdi);
}
/*EOF*/
