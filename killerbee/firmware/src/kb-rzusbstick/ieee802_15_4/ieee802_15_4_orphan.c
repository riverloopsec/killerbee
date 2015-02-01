// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the MLME-ORPHAN.response primitive.
 *
 *         An MLME-ORPHAN.response is sent after an MLME-ORPHAN.indication has
 *         been issued
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
 * $Id: ieee802_15_4_orphan.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_internal_api.h"
#include "ieee802_15_4_pib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static mlme_comm_status_ind_t mcsi;
/*================================= PROTOTYPES       =========================*/

bool ieee802_15_4_orphan_response(mlme_orphan_resp_t *mor) {
    
    /* Start by verifying the internal state of the IEEE 802.15.4 MAC. */
    if (IEEE_MAC_STARTED != IEEE802_15_4_GET_STATE()) {
        return false;
    }
    
    /* 1. Start with allocating the internal transmission buffer. */
    uint8_t *coord_realign = IEEE802_15_4_GET_TX_BUFFER();
    
    /* 2. This is a Coordinator Realignment Command sent in response to an
     *    orphan indication, so 64-bit address mode is used.
     */
    uint16_t fcf = FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR)  |
                   FCF_SET_DEST_ADDR_MODE(FCF_LONG_ADDR)    |
                   FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) |
                   FCF_ACK_REQUEST;
        
    uint8_t index = 0;
    coord_realign[index++] = ((fcf >> 8*0) & 0xFF);
    coord_realign[index++] = ((fcf >> 8*1) & 0xFF);

    /* 3. Add DSN */
    coord_realign[index++] = IEEE802_15_4_GET_DSN();

    /* 4. Broadcast PAN identifier is used (0xFFFF) as destination PAN ID. */
    coord_realign[index++] = 0xFF;
    coord_realign[index++] = 0xFF;
    
    /* 5. Add destination address. */
    memcpy((void *)(&coord_realign[index]), (void *)(&(mor->OrphanAddress)), sizeof(uint64_t));
    index += sizeof(uint64_t);
        
    /* 6. Insert Source PAN ID. */
    coord_realign[index++] = mac_pib_macPANId;
    coord_realign[index++] = (mac_pib_macPANId >> 8);

    /* 7. Add this device's IEEE address as source address. */
    memcpy((void *)(&coord_realign[index]), (void *)(&IEEE802_15_4_GET_EXTENDED_ADDRESS()), sizeof(uint64_t));
    index += sizeof(uint64_t);

    /* 8. Set the command type. */
    coord_realign[index++] = COORDINATORREALIGNMENT;

    /* 9. Set the Coordinator Realignment commands payload fields. */
    coord_realign[index++] = (IEEE802_15_4_GET_PAN_ID() >> 8*0);
    coord_realign[index++] = (IEEE802_15_4_GET_PAN_ID() >> 8*1);

    coord_realign[index++] = (IEEE802_15_4_GET_SHORT_ADDRESS() >> 8*0);
    coord_realign[index++] = (IEEE802_15_4_GET_SHORT_ADDRESS() >> 8*1);

    coord_realign[index++] = IEEE802_15_4_GET_CHANNEL();

    coord_realign[index++] = ((mor->ShortAddress) >> 8*0);
    coord_realign[index++] = ((mor->ShortAddress) >> 8*1);
    
    /* 10. Add two dummy bytes for CRC fields. Will be computed by the radio 
     * transceiver.
     */
    index += CRC_SIZE;
    
    bool mor_status = false;
    if (true != ieee802_15_4_send(index, coord_realign)) {
        /* Frame could not be sent. Go back to RX_AACK_ON. */
        tat_reset();
        (bool)tat_set_state(RX_AACK_ON);
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_FOR_COORD_REALIGN_ACK);
        mor_status = true;
    }
    
    return mor_status;
}


void ieee802_15_4_coord_realign_process_ack(void *tx_status) {
    /* Issue MLME_COMM_STATUS.indication. */
    ieee802_15_4_comm_status_indication_t msci_callback = ieee802_15_4_get_mlme_comm_status_indication();
    
    if (NULL == msci_callback) { return; }
    
    uint8_t mcsi_status = *((uint8_t*)tx_status);
    
    if (TRAC_SUCCESS == mcsi_status) {
        mcsi_status = MAC_SUCCESS;
    } else if (TRAC_NO_ACK == mcsi_status) {
        mcsi_status = MAC_NO_ACK;
    } else if (TRAC_CHANNEL_ACCESS_FAILURE == mcsi_status) {
        mcsi_status = MAC_CHANNEL_ACCESS_FAILURE;
    } else {
        mcsi_status = MAC_NO_DATA;
    }
    
    mcsi.status = mcsi_status;
    
    /* Execute callback. */
    msci_callback(&mcsi);
    
    /* Set state from TRX_OFF to RX_AACK_ON. */
    IEEE802_15_4_SET_STATE(IEEE_MAC_STARTED);
    (bool)tat_set_state(RX_AACK_ON);
}
/*EOF*/
