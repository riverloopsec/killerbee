// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the IEEE 802.15.4 MAC initialization procedure,
 *         and the low level frame parser
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
 * $Id: ieee802_15_4.c 41742 2008-05-15 12:49:32Z vkbakken $
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

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_pib.h"
#include "ieee802_15_4_internal_api.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
ieee802_15_4_state_t ieee802_15_4_state = IEEE_MAC_UNINITIALIZED;
uint8_t indication_buffer[30];                     //!< Internal buffer to build indications in.
uint8_t ieee802_15_4_tx_buffer[aMaxPHYPacketSize]; //!< Buffer to build IEEE 802.15.4 MAC frames in.
ieee802_15_4_mac_parameters_t mac_param;           //! Storage used by the request primitives.

/*================================= LOCAL VARIABLES  =========================*/


/* Pointers to the IEEE 802.15.4 MACs indication callback functions. */


/*! \brief Pointer function that will be executed upon a MCPS_DATA.indication. */
static ieee802_15_4_data_indication_t mcdi;
/*! \brief Pointer function that will be executed upon a MLME_ASSOCIATE.indication. */
static ieee802_15_4_associate_indication_t mai;
/*! \brief Function pointer that will be executed upon a MLME_DISASSOCIATE.indication. */
static ieee802_15_4_disassociate_indication_t mdi;
/*! \brief Function pointer that will be executed upon a MLME_ORPHAN.indication. */
static ieee802_15_4_orphan_indication_t moi;
/*! \brief Function pointer that will be executed upon a MLME_COMM_STATUS.indication. */
static ieee802_15_4_comm_status_indication_t mcsi;
/*! \brief Function pointer that will be executed upon a MLME_BEACON_NOTIFY.indication. */
static ieee802_15_4_beacon_notify_indication_t mbni;
/*! \brief Local frame buffer frame length and LQI inclusive. */
static uint8_t pd_frame_buffer[aMaxPHYPacketSize + 1 + 1];
/*! \brief Storage for information extracted by the frame parser. */
static ieee802_15_4_meta_data_t frame_info;
/*================================= PROTOTYPES       =========================*/
/*! \brief Do a state transition to receive mode. */
static void go_to_rx(void);

bool ieee802_15_4_init(uint64_t ieee_address) {
    IEEE802_15_4_SET_STATE(IEEE_MAC_UNINITIALIZED);
    
    /* The macro must come before ieee802_15_4_reset is called. The macro
     * shall only be called here and never from "user space".
     */
    IEEE802_15_4_SET_EXTENDED_ADDRESS(ieee_address);
    
    /* Set all indication callbacks to NULL. */
    mcdi = NULL;
    mai  = NULL;
    mdi  = NULL;
    moi  = NULL;
    mcsi = NULL;
    mbni = NULL;
    
    bool init_status = false;
    if (true != ieee802_15_4_reset(true)) {
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
        init_status = true;
    }
    
    return init_status;
}


void ieee802_15_4_deinit(void) {
    /* Turn off the radio transceiver. */
    tat_deinit();
    
    mcdi = NULL;
    mai  = NULL;
    mdi  = NULL;
    moi  = NULL;
    mcsi = NULL;
        
    /* Set the IEEE 802.15.4 MAC's internal state. */
    ieee802_15_4_state = IEEE_MAC_UNINITIALIZED;
}


ieee802_15_4_data_indication_t ieee802_15_4_get_mcps_data_indication(void) {    
    return mcdi;
}


void ieee802_15_4_clear_mcps_data_indication(void) {
    ENTER_CRITICAL_REGION();
    mcdi = NULL;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_set_mcps_data_indication(ieee802_15_4_data_indication_t indication) {
    ENTER_CRITICAL_REGION();
    mcdi = indication;
    LEAVE_CRITICAL_REGION();
}


ieee802_15_4_associate_indication_t ieee802_15_4_get_mlme_associate_indication(void) {    
    return mai;
}


void ieee802_15_4_clear_mlme_associate_indication(void) {
    ENTER_CRITICAL_REGION();
    mai = NULL;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_set_mlme_associate_indication(ieee802_15_4_associate_indication_t indication) {
    ENTER_CRITICAL_REGION();
    mai = indication;
    LEAVE_CRITICAL_REGION();
}


ieee802_15_4_disassociate_indication_t ieee802_15_4_get_mlme_disassociate_indication(void) {    
    return mdi;
}


void ieee802_15_4_clear_mlme_disassociate_indication(void) {
    ENTER_CRITICAL_REGION();
    mdi = NULL;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_set_mlme_disassociate_indication(ieee802_15_4_disassociate_indication_t indication) {
    ENTER_CRITICAL_REGION();
    mdi = indication;
    LEAVE_CRITICAL_REGION();
}


ieee802_15_4_orphan_indication_t ieee802_15_4_get_mlme_orphan_indication(void) {    
    return moi;
}


void ieee802_15_4_clear_mlme_orphan_indication(void) {
    ENTER_CRITICAL_REGION();
    moi = NULL;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_set_mlme_orphan_indication(ieee802_15_4_orphan_indication_t indication) {
    ENTER_CRITICAL_REGION();
    moi = indication;
    LEAVE_CRITICAL_REGION();
}


ieee802_15_4_comm_status_indication_t ieee802_15_4_get_mlme_comm_status_indication(void) {    
    return mcsi;
}


void ieee802_15_4_clear_mlme_comm_status_indication(void) {
    ENTER_CRITICAL_REGION();
    mcsi = NULL;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_set_mlme_comm_status_indication(ieee802_15_4_comm_status_indication_t indication) {
    ENTER_CRITICAL_REGION();
    mcsi = indication;
    LEAVE_CRITICAL_REGION();
}


ieee802_15_4_beacon_notify_indication_t ieee802_15_4_get_mlme_beacon_notify_indication(void) {    
    return mbni;
}


void ieee802_15_4_clear_mlme_beacon_notify_indication(void) {
    ENTER_CRITICAL_REGION();
    mbni = NULL;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_set_mlme_beacon_notify_indication(ieee802_15_4_beacon_notify_indication_t indication) {
    ENTER_CRITICAL_REGION();
    mbni = indication;
    LEAVE_CRITICAL_REGION();
}


void ieee802_15_4_pd_data_indication(void *frame_length) {
    /* When this function is entered, the radio transceiver will be doing the
     * ACK associated with the frame reception, or have entered the PLL_ON state
     * that is used to protect the frame buffer from being overwritten by another
     * frame. The frame is also know to be at least as long as the minimum frame
     * length accespted by the TAT.
     */


    /* Extract frame length from the event parameter. */
    uint8_t length = *((uint8_t*)frame_length);
    
    /* Upload frame. Take the LQI field into count, so add two (frame length + LQI).*/
    if (true != tat_read_frame(length + 2, pd_frame_buffer)) {
        go_to_rx();
        return;
    }
    
    /* Store LQI from the frame. */
    frame_info.lqi = pd_frame_buffer[length + 1];
    
    // 2. Extract the FCF field. */
    uint16_t fcf = (pd_frame_buffer[1] << 8 * 0); // Read FCF LSB.
    fcf         |= (pd_frame_buffer[2] << 8 * 1); // Read FCF MSB.
    
    // 4. Check if this frame has security enabled. Not supported on MAC level.
    if (FCF_SECURITY_ENABLED == (fcf & FCF_SECURITY_ENABLED)) {
        go_to_rx();
        return;
    }
    
    // 5. Get frame type and verify that it is supported.
    uint8_t frame_type = FCF_GET_FRAMETYPE(fcf);
    if ((FCF_FRAMETYPE_DATA != frame_type) && 
        (FCF_FRAMETYPE_MAC_CMD != frame_type) &&
        (FCF_FRAMETYPE_BEACON != frame_type)) {
        go_to_rx();
        return;
    }
    
    frame_info.frame_type = frame_type;
    
    // 6. Read the addressing mode used and check that it is valid.
    frame_info.dst_addr_mode = FCF_GET_DEST_ADDR_MODE(fcf);
    frame_info.src_addr_mode = FCF_GET_SOURCE_ADDR_MODE(fcf);
    if ((FCF_NO_ADDR == (frame_info.dst_addr_mode)) && 
        (FCF_NO_ADDR == (frame_info.src_addr_mode))) {
        go_to_rx();
        return;
    }
    
    /* Extract sequence number. */
    uint8_t i = 3;
    frame_info.seq_nmbr = pd_frame_buffer[i];
    i++;
    
    // 7. Extract Destination addressing fields.
    frame_info.dst_addr = 0;
    if (FCF_NO_ADDR == (frame_info.dst_addr_mode)) {
        // No destination address is only possible if this is a PAN Coordinator and
        // the received frame is a Data Request command. Or that the frame is a Beacon.
    } else if (FCF_SHORT_ADDR == (frame_info.dst_addr_mode)) {
        memcpy((void *)(&(frame_info.dst_pan_id)), (void *)(&pd_frame_buffer[i]), sizeof(uint16_t));
        i += sizeof(uint16_t);
        memcpy((void *)(&(frame_info.dst_addr)), (void *)(&pd_frame_buffer[i]), sizeof(uint16_t));
        i += sizeof(uint16_t);
    } else if (FCF_LONG_ADDR == (frame_info.dst_addr_mode)) {
        memcpy((void *)(&(frame_info.dst_pan_id)), (void *)(&pd_frame_buffer[i]), sizeof(uint16_t));
        i += sizeof(uint16_t);
        memcpy((void *)(&(frame_info.dst_addr)), (void *)(&pd_frame_buffer[i]), sizeof(uint64_t));
        i += sizeof(uint64_t);
    } else { 
        go_to_rx();
        return;
    }
    
    // 8. Extract Source addressing fields.
    frame_info.src_addr = 0;
    if (FCF_NO_ADDR == (frame_info.src_addr_mode)) {
    } else {
        // Either a short or long address is available. Check for PAN_ID compression.
        if (FCF_INTRA_PAN != (fcf & FCF_INTRA_PAN)) {
            memcpy((void *)(&(frame_info.src_pan_id)), (void *)(&pd_frame_buffer[i]), sizeof(uint16_t));
            i += sizeof(uint16_t);
        } // END: if (FCF_INTRA_PAN == (fcf & FCF_INTRA_PAN)) ...
        
        if (FCF_SHORT_ADDR == (frame_info.src_addr_mode)) {    
            memcpy((void *)(&(frame_info.src_addr)), (void *)(&pd_frame_buffer[i]), sizeof(uint16_t));
            i += sizeof(uint16_t);
        } else {
            memcpy((void *)(&(frame_info.src_addr)), (void *)(&pd_frame_buffer[i]), sizeof(uint64_t));
            i += sizeof(uint64_t);
        }
    }
    
    // 9. Set the command type if applicable.
    if (FCF_FRAMETYPE_MAC_CMD == (frame_info.frame_type)) {
        frame_info.cmd_type = pd_frame_buffer[i];
        i++;
    } else {
        frame_info.cmd_type = 0;
    }
    
    // 10. Set pointer to msdu and calculate msdu length.
    frame_info.msdu = &pd_frame_buffer[i];
    
    /* Decrement frame index so that msdu length calculation is correct. The decrement
     * by one is done since 'i' is always pointing to the start of the next field.
     */
    i--;
    frame_info.msdu_length = length - i - CRC_SIZE;
    
    vrt_event_handler_t event_handler = (vrt_event_handler_t)NULL;
    
    switch(frame_info.frame_type) {
        case FCF_FRAMETYPE_DATA:
            event_handler = ieee802_15_4_process_data_indication;
        break;
        
        case FCF_FRAMETYPE_MAC_CMD:
            event_handler = ieee802_15_4_mlme_cmd_indication;
        break;
        
        default:
            event_handler = ieee802_15_4_mlme_beacon_indication;
        break;
    }
    
    /* Post the appropriate indication event, or free up the packet. */
    if (true != vrt_post_event(event_handler, ((void *)(&frame_info)))) {
        go_to_rx();
    }
}


bool ieee802_15_4_send(uint8_t length, uint8_t *data) {
    /* Do state transition to TX_ARET_ON. */
    if (true != tat_wake_up()) {
    } else if (true != tat_set_state(TX_ARET_ON)) {
    } else {
        
        /* Send frame. */
        tat_send_frame(length, data);
        return true;
    }
    
    return false;
}

void ieee802_15_4_send_ack_event_handler(void *tx_status) {
    /* Frame has been sent and then ACK'ed. The radio transeiver will now have 
     * been taken to TRX_OFF. */
   
    uint8_t mac_state = IEEE802_15_4_GET_STATE();
    
    switch (mac_state) {
        case IEEE_MAC_TX_WAITING_ACK:
            ieee802_15_4_data_process_ack(tx_status);
        break;
        
        case IEEE_MAC_WAITING_ASSOCIATION_REQUEST_ACK:
        case IEEE_MAC_WAITING_DATA_REQUEST_ACK:
            ieee802_15_4_associate_process_ack(tx_status);
        break;
        
        case IEEE_MAC_WAITING_ASSOCIATE_RESPONSE_ACK:
            ieee802_15_4_associate_response_process_ack(tx_status);
        break;
        
        case IEEE_MAC_DISASSOCIATE_WAITING_ACK:
            ieee802_15_4_disassociate_process_ack(tx_status);
        break;
        
        case IEEE_MAC_WAITING_FOR_COORD_REALIGN_ACK:
            ieee802_15_4_coord_realign_process_ack(tx_status);
        break;
        
        case IEEE_MAC_BUSY_SCANNING:
            ieee802_15_4_scan_process_ack(tx_status);
        break;
        
        case IEEE_MAC_WAITING_BEACON_ACK:
            ieee802_15_4_process_beacon_ack(tx_status);
        break;
        
        default:
            /* Return to RX mode if the device is set to do so. */
            if (true == IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
                go_to_rx();
            } else {
                tat_go_to_sleep();
            }
        break;
    }
}


static void go_to_rx(void) {
    tat_reset();
    (bool)tat_set_state(RX_AACK_ON);
}
/*EOF*/
