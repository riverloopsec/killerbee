// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the handling and dispatch of IEEE 802.15.4 
 *         command frames
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
 * $Id: ieee802_15_4_cmd.c 41144 2008-04-29 12:42:28Z ihanssen $
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
#include "ieee802_15_4_internal_api.h"
#include "ieee802_15_4_pib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*! \brief Function that returns the radio transceiver to RX_AACK_ON when the
 *         received frame has been ACK'ed.
 */
static void goto_rx_aack_on(void);


void ieee802_15_4_mlme_cmd_indication(void *cmd_frame_info) {
    ieee802_15_4_meta_data_t *frame_info = (ieee802_15_4_meta_data_t *)cmd_frame_info;
    
    /* Read the actual command. */
    uint8_t cmd_type = frame_info->cmd_type;
    /* Handle the different commands given the state the device is operating in. */
    if ((ASSOCIATIONREQUEST == cmd_type) && (IEEE_MAC_STARTED == IEEE802_15_4_GET_STATE())) {
        /* CMD: ASSOCIATIONREQUEST received. */
        
        /* 7.5.3.1: "If a coordinator with macAssociationPermit set to false 
         * receives an association request command from a device, the command 
         * shall be ignored."
         */
        if (true != IEEE802_15_4_GET_ASSOCIATION_PERMITTED()) {
            goto_rx_aack_on();
            return; 
        }
        
        /* Check if the MLME_ASSOCIATE.indication callback is enabled. */
        ieee802_15_4_associate_indication_t mai_callback = ieee802_15_4_get_mlme_associate_indication();
        if (NULL == mai_callback) { 
            goto_rx_aack_on();
            return;
        }
            
        /* Try to allocate some memory to build the MLME_ASSOCIATE.indication message. Use the internal
         * IEEE 802.15.4 MAC indication buffer.
         */
        mlme_associate_ind_t *mai = (mlme_associate_ind_t *)IEEE802_15_4_GET_INDICATION_BUFFER();
        if (NULL == mai) {
            goto_rx_aack_on();
            return;
        }
            
        /* Build indication message. */
        memcpy((void *)(&(mai->DeviceAddress)), (void *)(&(frame_info->src_addr)), sizeof(uint64_t));
        mai->CapabilityInformation = *(frame_info->msdu);
                
        /* Execute the callback. */
        mai_callback(mai);
        
        goto_rx_aack_on();
    } else if (((DISASSOCIATIONNOTIFICATION == cmd_type) && (IEEE_MAC_STARTED == IEEE802_15_4_GET_STATE())) ||
               ((DISASSOCIATIONNOTIFICATION == cmd_type) && (IEEE_MAC_ASSOCIATED == IEEE802_15_4_GET_STATE()))) {
        /* CMD: DISASSOCIATIONNOTIFICATION received. */
        
        /* Check if the MLME_ASSOCIATE.indication callback is enabled. */
        ieee802_15_4_disassociate_indication_t dai_callback = ieee802_15_4_get_mlme_disassociate_indication();
        if (NULL == dai_callback) {
            goto_rx_aack_on();
            return;
        }
                   
        /* Get hold of some memory to build the Disassociate indication. */
        mlme_disassociate_ind_t *mdi = (mlme_disassociate_ind_t *)IEEE802_15_4_GET_INDICATION_BUFFER();
        
        /* Set the DeviceAddress, aka. the address of the device requesting the disassociaton which 
         * is always contained in the source address. 
         */
        memcpy((void *)(&(mdi->DeviceAddress)), (void *)(&(frame_info->src_addr)), sizeof(uint64_t));
        
        /* Extract disassociate reason. */
        mdi->DisassociateReason = *(frame_info->msdu);
        
        /* Execute the associated callback. */
        dai_callback(mdi);
        
        if (IEEE_MAC_ASSOCIATED == IEEE802_15_4_GET_STATE()) {
            tat_go_to_sleep();
            
            /* Clear coordinator addres. */
            IEEE802_15_4_SET_COORD_EXTENDED_ADDRESS(0);
            IEEE802_15_4_SET_COORD_SHORT_ADDRESS(BROADCAST);
            
            IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
        }
    } else if ((DATAREQUEST == cmd_type) && (IEEE_MAC_WAITING_FOR_DATA_REQUEST == IEEE802_15_4_GET_STATE())) {
        /* CMD: DATAREQUEST received. */
        /* Send Association response frame. */
        vrt_timer_stop_high_pri_delay();
        ieee802_15_4_send_association_response();
    } else if ((ORPHANNOTIFICATION == cmd_type) && (IEEE_MAC_STARTED == IEEE802_15_4_GET_STATE())) {
        /* CMD: ORPHANNOTIFICATION received. */
        /* Check if the MLME_ASSOCIATE.indication callback is enabled. */
        ieee802_15_4_orphan_indication_t oi_callback = ieee802_15_4_get_mlme_orphan_indication();
        if (NULL == oi_callback) {
            goto_rx_aack_on();
            return;
        }
                   
        /* Get hold of some memory to build the Orphan indication. */
        mlme_orphan_ind_t *moi = (mlme_orphan_ind_t *)IEEE802_15_4_GET_INDICATION_BUFFER();
        
        memcpy((void *)(&moi->OrphanAddress), (void *)(&(frame_info->src_addr)), sizeof(uint64_t));
        
        /* Execute callback. */
        oi_callback(moi);
    } else if ((BEACONREQUEST == cmd_type) && (IEEE_MAC_STARTED == IEEE802_15_4_GET_STATE())) {
        /* CMD: BEACONREQUEST received. */
        ieee802_15_4_send_beacon();
    } else if ((COORDINATORREALIGNMENT == cmd_type) && (IEEE_MAC_BUSY_SCANNING == IEEE802_15_4_GET_STATE())) {
        /* CMD: COORDINATORREALIGNMENT received. */
        
        /* Stop timer, clear callback and force radio transceiver to the TRX_OFF 
         * state. The final transition to sleep will be done by the code that 
         * executes the confirm callback.
         */
        vrt_timer_stop_high_pri_delay();
        rf230_subregister_write(SR_TRX_CMD, CMD_TRX_OFF);
        
        uint8_t status = MAC_INVALID_PARAMETER;
        if (7 != (frame_info->msdu_length)) { 
        } else {
            status = MAC_SUCCESS;
        }
        
        uint8_t *cr = frame_info->msdu;
        
        /* Extract PAN ID. */
        uint16_t conv = 0;
        conv = cr[1] << 8;
        conv |= cr[0];
        (uint8_t)ieee802_15_4_set_pan_id(conv);
        
        /* Extract Coordinator short address. */
        conv = 0;
        conv = cr[3] << 8;
        conv |= cr[2];
        IEEE802_15_4_SET_COORD_SHORT_ADDRESS(conv);
        
        /* Extract Short address */
        conv = cr[6] << 8;
        conv |= cr[5];
        if (BROADCAST != conv) { (uint8_t)ieee802_15_4_set_pan_id(conv); }
        
        /* Extract channel. */
        uint8_t channel = cr[4];
        (uint8_t)ieee802_15_4_set_channel(channel); // Set the appropriate PIB entries
        
        mlme_scan_conf_t *msc = (mlme_scan_conf_t *)mac_param.scan.msc;
        msc->status = status;
        
        if (true != vrt_post_event(ieee802_15_4_scan_do_confirm, (void *)msc)) {
            goto_rx_aack_on();
        }
    } else if ((ASSOCIATIONRESPONSE == cmd_type) && (IEEE_MAC_WAITING_ASSOCIATION_RESPONSE == IEEE802_15_4_GET_STATE())) {
        /* Stop symbol timer running. */
        vrt_timer_stop_high_pri_delay();
        
        /* Extract association status. Position xx.*/
        uint8_t *association_response = frame_info->msdu;
        
        /* Extract and set short address. Positions 22 and 23.*/
        uint16_t short_addr = association_response[1] << 8;
        short_addr         |= association_response[0];
        
        /* Extract the association status. Positions 24.*/
        uint8_t status = association_response[2];
        
        /* The return from the set function is not used, since it will only report
         * back false if the radio transceiver could not be taken out of sleep.
         * However the system is guaranteed not to sleep in this mode.
         */
        (uint8_t)ieee802_15_4_set_short_address(short_addr);
    
        /* Set coordinator extended address. */
        memcpy((void *)(&IEEE802_15_4_GET_COORD_EXTENDED_ADDRESS()), \
               (void *)(&(frame_info->src_addr)), sizeof(uint64_t));
        
        /* Issue the confirm message back to the user. */
        issue_associate_confirm(status, short_addr);
    } else {
        goto_rx_aack_on();
    }
}


static void goto_rx_aack_on(void) {
    /* Wait until the PLL_ON state is entered. */
    do {
        delay_us(20);
    } while (CMD_PLL_ON != tat_get_state());
    
    /* Go to RX_AACK_ON. */
    (bool)tat_set_state(RX_AACK_ON);
}
/*EOF*/
