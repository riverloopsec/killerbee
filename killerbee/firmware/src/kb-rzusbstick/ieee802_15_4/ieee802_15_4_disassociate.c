// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the MLME-DISASSOCIATE.request primitive.
 *
 *         The MLME-DISASSOCIATE.request primitive is used by an associated 
 *         device to notify the coordinator of its intent to leave the PAN. It is 
 *         also used by the coordinator to instruct an associated device to leave
 *         the PAN
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
 * $Id: ieee802_15_4_disassociate.c 41144 2008-04-29 12:42:28Z ihanssen $
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
#include "ieee802_15_4_pib.h"
#include "ieee802_15_4_internal_api.h"
#include "ieee802_15_4_msg_types.h"
#include "ieee802_15_4_const.h"
/*================================= MACROS           =========================*/
#define DISASSOCIATIONNOTIFICATION_CMD_SIZE (27) //!< Length of Disassociate Notification command in bytes.
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

bool ieee802_15_4_disassociate_request(mlme_disassociate_req_t* mdr) {
    /* Check if it is possible to execute this primitive from the given internal 
     * state. A device must be associated before it can disassociate itself, and 
     * a  PAN coordinator must be started before it can force one of its children
     * to disassociate.
     */
    if ((IEEE_MAC_STARTED != IEEE802_15_4_GET_STATE()) && 
        (IEEE_MAC_ASSOCIATED != IEEE802_15_4_GET_STATE())) {
        
        return false;
    }
    
    /* Perform sanity check on function parameters. */
    if (NULL == (mdr->mlme_disassociate_confirm)) {
        return false;
    }
    
    // 1. Request some memory to build the disassociation request frame.
    uint8_t* disassociation_request  = IEEE802_15_4_GET_TX_BUFFER();
    
    /* Store some internal variables. */
    mac_param.disassociate.mlme_disassociate_confirm = mdr->mlme_disassociate_confirm;
    mac_param.disassociate.mdc = &(mdr->mdc);
    
    // 2. Build FCF.
    uint8_t index = 0;
    uint16_t fcf;

    fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) | FCF_SET_DEST_ADDR_MODE(FCF_LONG_ADDR) |
          FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR) | FCF_ACK_REQUEST;

    // Add the Frame Control Field
    disassociation_request[index++] = ((fcf >> 8*0) & 0xFF);
    disassociation_request[index++] = ((fcf >> 8*1) & 0xFF);

    // 3. Add the DSN.
    disassociation_request[index++] = IEEE802_15_4_GET_DSN();

    // 4. Add destination PAN ID.
    disassociation_request[index++] = ((IEEE802_15_4_GET_PAN_ID()) >> 8*0) & 0xFF;
    disassociation_request[index++] = ((IEEE802_15_4_GET_PAN_ID()) >> 8*1) & 0xFF;

    // 5. If a coordinator, then destination address is the 64 bit address of 
    //    the device. If a device, then the destination address is 
    //     "macCoordExtendedAddress".
    if ((mdr->DeviceAddress) != IEEE802_15_4_GET_COORD_EXTENDED_ADDRESS()) {
        memcpy((void *)(&disassociation_request[index]), (void *)(&(mdr->DeviceAddress)), sizeof(uint64_t));
        index += sizeof(uint64_t);
    } else {
        memcpy((void *)(&disassociation_request[index]), (void *)(&(IEEE802_15_4_GET_COORD_EXTENDED_ADDRESS())), sizeof(uint64_t));
        index += sizeof(uint64_t);
        IEEE802_15_4_SET_COORD_EXTENDED_ADDRESS(0);
        (uint8_t)ieee802_15_4_set_short_address(BROADCAST);
    }

    // 6. Add the source pan id.
    disassociation_request[index++] = ((IEEE802_15_4_GET_PAN_ID() >> 8*0) & 0xFF);
    disassociation_request[index++] = ((IEEE802_15_4_GET_PAN_ID() >> 8*1) & 0xFF);

    // 7. Add the source address, the source address of the device sending the command.
    memcpy((void *)(&disassociation_request[index]), (void *)(&(IEEE802_15_4_GET_EXTENDED_ADDRESS())), sizeof(uint64_t));
    index += sizeof(uint64_t);
    
    // 8. Add the command frame identifier.
    disassociation_request[index++] = DISASSOCIATIONNOTIFICATION;

    // 9. Add the disassociation reason code.
    disassociation_request[index++] = mdr->DisassociateReason;
    
    // 10. Add two dummy bytes as CRC.
    index += CRC_SIZE;
    
    bool mdr_status = false;
    if (true != ieee802_15_4_send(index, disassociation_request)) {
        if (true != IEEE802_15_4_GET_RX_ON_WHEN_IDLE()) {
            tat_go_to_sleep();
        } else {
            (bool)tat_set_state(RX_AACK_ON);
        }
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_DISASSOCIATE_WAITING_ACK);
        mdr_status = true;
    } 
    
    return mdr_status;
}


void ieee802_15_4_disassociate_process_ack(void *tx_status) {
    /* Extract status of transmission, for then to set the correct confirm status. */
    uint8_t status = *((uint8_t*)tx_status);
    
    if ((TRAC_SUCCESS == status) || (TRAC_SUCCESS_DATA_PENDING == status)) {
        mac_param.disassociate.mdc->status = MAC_SUCCESS;
    } else if (TRAC_NO_ACK == status) {
        mac_param.disassociate.mdc->status = MAC_NO_ACK;
    } else {
        mac_param.disassociate.mdc->status = MAC_CHANNEL_ACCESS_FAILURE;
    }
        
    /* The last transitted frame has been acknowledged. The user is notified 
     * through the callback. 
     */
    mac_param.disassociate.mlme_disassociate_confirm(mac_param.disassociate.mdc);
    
    if (true != IEEE802_15_4_GET_I_AM_COORDINATOR()) {
        tat_go_to_sleep();
        IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_STARTED);
        (bool)tat_set_state(RX_AACK_ON);
    }
}
/*EOF*/
