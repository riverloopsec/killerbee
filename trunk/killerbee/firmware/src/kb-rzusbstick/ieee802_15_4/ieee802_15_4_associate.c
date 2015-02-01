// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the IEEE 802.15.4 Association process.
 *
 *      The association process in IEEE 802.15.4 is basically the way a device
 *      gets it's short address. A scan will typically be run prior to issuing
 *      an associate request. This scan is used to select the PAN identifier
 *      of the network that the device is to associate
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
 * $Id: ieee802_15_4_associate.c 41144 2008-04-29 12:42:28Z ihanssen $
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
#include "vrt_timer.h"

#include "tat.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_internal_api.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_msg_types.h"
#include "ieee802_15_4_pib.h"
/*================================= MACROS           =========================*/
/*! \brief Worst case association request command size in bytes */
#define ASSOCIATION_REQUEST_CMD_SIZE  (29)
/*! \brief Worst case data request command size in bytes. */
#define DATA_REQUEST_CMD_SIZE         (20)
/*! \brief Length of Association Response command in bytes. */
#define ASSOCIATION_RESPONSE_CMD_SIZE (27)
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*! \brief Pointer used to store the location of the associate confirm storage. */
static mlme_associate_conf_t *confirm_ptr;
/*! \brief Pointer to the user selected callback for the associate confirm.  */
static void (*mlme_associate_confirm)(mlme_associate_conf_t *mar) = NULL;
/*! \brief Buffer used as storage for the Association Response command. */
static uint8_t association_response[ASSOCIATION_RESPONSE_CMD_SIZE];
/*! \brief Storage used to build MLME_COMM_STATUS.indication messages in.*/
static mlme_comm_status_ind_t mcsi;
/*================================= PROTOTYPES       =========================*/

/*! \brief This function will build and send a Data Request Command. */
static void send_data_request(void);

/*! \brief This is an internal function that will issue a MLME_COMM_STATUS.indication
 *         after a MLME_ASSOCIATE.response. 
 * 
 *  The MLME_COMM_STATUS.indication message will be pre-built when the Association
 *  Response Command is sent. However, the status field is not set. When the callback
 *  has been executed the radio transceiver will be taken back into receive mode.
 *
 *  \param[in] status Status that the MLME_COMM_STATUS.indication will be sent with.
 */
static void send_comm_status_indication(uint8_t status);


bool ieee802_15_4_associate_request(mlme_associate_req_t *mar) {
    /* Check that the device is in the correct state. */
    if (IEEE_MAC_IDLE != IEEE802_15_4_GET_STATE()) { return false; }
    
    /* Perform sanity check on all function parameters. */
    if (NULL == mar) { return false; }
    if (NULL == (mar->mlme_associate_confirm)) { return false; }    
    
    /* Store confirm callback and storage for confirm parameters. */
    confirm_ptr = &(mar->mac);
    mlme_associate_confirm = mar->mlme_associate_confirm;
    
    uint8_t channel = mar->LogicalChannel;
    if ((channel < MIN_CHANNEL) || (channel > MAX_CHANNEL)) { return false; }
    
    if (BROADCAST == (mar->CoordPANId))  { return false; }
    
    uint8_t addr_mode = mar->CoordAddrMode;
    
    if ((FCF_SHORT_ADDR != addr_mode) && (FCF_LONG_ADDR != addr_mode)) { return false; }
    
    /* Allocate buffer space to build the associate request. Use the internal 
     * IEEE 802.15.4 MAC Command buffer.
     */
    uint8_t *association_request  = IEEE802_15_4_GET_TX_BUFFER();
    
    /* 1. Build FCF. */
    uint8_t index = 0;
    uint16_t fcf;
    
    if (FCF_SHORT_ADDR == addr_mode) {
        IEEE802_15_4_SET_COORD_SHORT_ADDRESS((mar->CoordAddress));
        fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) | FCF_SET_DEST_ADDR_MODE(FCF_SHORT_ADDR) |
              FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR) | FCF_ACK_REQUEST;
    } else {
        memcpy((void *)(&(IEEE802_15_4_GET_COORD_EXTENDED_ADDRESS())), (void *)(&(mar->CoordAddress)), sizeof(uint64_t));
        
        fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) | FCF_SET_DEST_ADDR_MODE(FCF_LONG_ADDR) |
              FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR) | FCF_ACK_REQUEST;
    }
    
    // Add FCF fields.
    association_request[index++] = ((fcf >> 8*0) & 0xFF);
    association_request[index++] = ((fcf >> 8*1) & 0xFF);

    /* 2. Add farme sequence number. */
    association_request[index++] = IEEE802_15_4_GET_DSN();

    /* 3. Build Address Fields. */
    memcpy((void *)(&association_request[index]), (void *)(&(mar->CoordPANId)),sizeof(uint16_t));
    index += sizeof(uint16_t);
    
    // Build the destination address.
    if (FCF_SHORT_ADDR == addr_mode) {
        memcpy((void *)(&association_request[index]), (void *)(&(mar->CoordAddress)), sizeof(uint16_t));
        index += sizeof(uint16_t);
    } else {
        memcpy((void *)(&association_request[index]), (void *)(&(mar->CoordAddress)), sizeof(uint64_t));
        index += sizeof(uint64_t);
    }

    // Source PAN ID equals broadcast (0xFFFF) PAN ID for this message.
    association_request[index++] = 0xFF;
    association_request[index++] = 0xFF;

    // Build the source address.
    memcpy((void *)(&association_request[index]), (void *)(&(IEEE802_15_4_GET_EXTENDED_ADDRESS())), sizeof(uint64_t));
    index += sizeof(uint64_t);

    /* 4. Build the command frame id. */
    association_request[index++] = ASSOCIATIONREQUEST;

    /* 5. Build the capability info. */
    association_request[index++] = mar->CapabilityInformation;

    /* 7. Add dummy bytes in crc field. */
    index += 2;
    
    /* 8. Set necessary parameters and finally send the frame. */
    if ((MAC_SUCCESS != ieee802_15_4_set_pan_id(mar->CoordPANId))) {
        goto association_request_error;
    } else if (true != tat_set_channel(mar->LogicalChannel)) {
        goto association_request_error;
    } else if (true != ieee802_15_4_send(index, association_request)) {
        goto association_request_error;
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_ASSOCIATION_REQUEST_ACK);
        return true;
    }
    
    
    /* Do error handling for the association request: Go to SLEEP. */
    association_request_error:
    
    
    tat_go_to_sleep();
    
    return false;
}


bool ieee802_15_4_associate_response(mlme_associate_resp_t* mar) {
    /* Verify that the IEEE 802.15.4 MAC state is correct. */
    if (IEEE_MAC_STARTED != IEEE802_15_4_GET_STATE()) { return false; }
    
    /* Perform sanity check on the function parameters. */
    if (NULL == mar) { return false; }
    
    /* 1. Build the Association Response command frame. Start with the FCF. */
    uint8_t index = 0;
    uint16_t fcf;
    
    fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) | FCF_SET_DEST_ADDR_MODE(FCF_LONG_ADDR) |
          FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR)  | FCF_ACK_REQUEST | FCF_INTRA_PAN;
    
    association_response[index++] = ((fcf >> 8*0) & 0xFF);
    association_response[index++] = ((fcf >> 8*1) & 0xFF);
    
    /* 3. Add sequence number. */
    association_response[index++] = IEEE802_15_4_GET_DSN();
    
    /* 4. Add Address Fields. */
    memcpy((void *)(&association_response[index]), (void *)(&(IEEE802_15_4_GET_PAN_ID())), sizeof(uint16_t));
    index += sizeof(uint16_t);
    
    memcpy((void *)(&association_response[index]), (void *)(&(mar->DeviceAddress)), sizeof(uint64_t));
    index += sizeof(uint64_t);
    
    memcpy((void *)(&association_response[index]), (void *)(&(IEEE802_15_4_GET_EXTENDED_ADDRESS())), sizeof(uint64_t));
    index += sizeof(uint64_t);

    /* 5. Add the command frame id. */
    association_response[index++] = ASSOCIATIONRESPONSE;

    /* 6. Add short address assigned to the associating device. */
    memcpy((void *)(&association_response[index]), (void *)(&(mar->AssocShortAddress)), sizeof(uint16_t));
    index += sizeof(uint16_t);

    /* 7. Set the association status. */
    association_response[index++] = mar->status;

    // Add dummy bytes in crc field.
    index += 2;
   
    /* Prebuild MLME_COMM_STATUS.indication. */
    mcsi.PANId = IEEE802_15_4_GET_PAN_ID();
    mcsi.SrcAddrMode = FCF_LONG_ADDR;
    memcpy((void *)(&mcsi.SrcAddr), (void *)(&(IEEE802_15_4_GET_EXTENDED_ADDRESS())),\
           sizeof(uint64_t));    
    mcsi.DstAddrMode = FCF_LONG_ADDR;
    memcpy((void *)(&mcsi.DstAddr), (void *)(&(mar->DeviceAddress)), sizeof(uint64_t));
    
    bool response_status = false;
    /* 8. Set a timeout for the Data Request command to arrive. */
    IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_FOR_DATA_REQUEST);
    uint32_t ticks = IEEE802_15_4_SYMBOL_TO_TICKS(2 * aResponseWaitTime);
        
    if (true != vrt_timer_start_high_pri_delay(ticks, ieee802_15_4_associate_process_timeout,\
                                               (void*)NULL)) {
                                                   
        ieee802_15_4_associate_process_timeout(NULL);                                         
    } else {
        
        response_status = true;
    }
    
    return response_status;
}


void ieee802_15_4_associate_process_ack(void *ack_status) {
    /* Extract the transmission status from the ack_status pointer. */
    uint8_t tx_status = *((uint8_t *)ack_status);
    
    if ((TRAC_SUCCESS == tx_status) || (TRAC_SUCCESS_DATA_PENDING == tx_status)) {
        /* If this ACK is a response to the data request, the transceiver must be set 
         * in RX_AACK_ON mode before the response timer is started.
         */
        if (IEEE_MAC_WAITING_DATA_REQUEST_ACK == IEEE802_15_4_GET_STATE()) {
            
            if(true != tat_set_state(RX_AACK_ON)) {
                tx_status = MAC_CHANNEL_ACCESS_FAILURE;
                goto associate_ack_error;
            } else {
                IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_ASSOCIATION_RESPONSE);
            }
        } else {
            IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_TO_SEND_DATA_REQUEST);
        }
        
        /* Start response wait symbol timer. */
        uint32_t ticks = IEEE802_15_4_SYMBOL_TO_TICKS(aResponseWaitTime);
        if (true != vrt_timer_start_high_pri_delay(ticks, ieee802_15_4_associate_process_timeout,\
                                                   (void *)NULL)) {
                                                             
            /* In lack of a better error status in the IEEE 802.15.4 standard this one is
             * reported back to the next higher layer.
             */
            tx_status = MAC_CHANNEL_ACCESS_FAILURE;
            goto associate_ack_error;
        } else {
            /* Timer started. Wait for it to expire. */
            return;
        }
    } else {
        if (TRAC_NO_ACK == tx_status) {
            tx_status = MAC_NO_ACK;
        } else if (TRAC_CHANNEL_ACCESS_FAILURE == tx_status) {
            tx_status = MAC_CHANNEL_ACCESS_FAILURE;
        } else {
            tx_status = MAC_NO_DATA;
        }
    }
    
    
    /* Execute the associated callback, with the short address set to 0xFFFF.
     * This together with the status indicates that the association was not
     * successful.
     */
    associate_ack_error:
    
    
    vrt_timer_stop_high_pri_delay();
    issue_associate_confirm(tx_status, 0xFFFF);
}


void ieee802_15_4_associate_process_timeout(void *null) {
    if (IEEE_MAC_WAITING_TO_SEND_DATA_REQUEST == IEEE802_15_4_GET_STATE()) {
        send_data_request();
    } else if (IEEE_MAC_WAITING_ASSOCIATION_RESPONSE == IEEE802_15_4_GET_STATE()) {
        /* An Association response was not received. */
        issue_associate_confirm(MAC_NO_ACK, 0xFFFF);
    } else if (IEEE_MAC_WAITING_FOR_DATA_REQUEST == IEEE802_15_4_GET_STATE()) {
        /* Send MLME_COMM_STATUS.indication. */
        send_comm_status_indication(MAC_TRANSACTION_EXPIRED);
    } else {
        /* Error: Turn off the radio transceiver and set the IEEE 802.15.4 MAC's
         * state back to Idle.
         */
        tat_go_to_sleep();
        IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
    }
}


/*! \brief This function is used to build the associate confirm message and 
 *         execute the associated callback to the user.
 *
 *  \param[in] status Status of the association process.
 *  \param[in] short_addr Short address the node shall operate with. 0xFFFF
 *                        indicates an error during the association process.
 */
void issue_associate_confirm(uint8_t status, uint16_t short_addr) {
    /* Set internal state. */
    if (MAC_SUCCESS != status) {
        IEEE802_15_4_SET_STATE(IEEE_MAC_IDLE);
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_ASSOCIATED);
    }
    
    /* Now, put the radio transceiver to SLEEP. */
    tat_go_to_sleep();
    
    /* Build the confirm message and issue the associated callback. */
    confirm_ptr->status = status;
    confirm_ptr->AssocShortAddress = short_addr;
    
    mlme_associate_confirm(confirm_ptr);
}


static void send_data_request(void) {
    /* 1. Request some memory to build the association request frame. Use the
     *    internal IEEE 802.15.4 MAC Command buffer.
     */
    uint8_t* data_request  = IEEE802_15_4_GET_TX_BUFFER();
    
    /* 1. Build FCF. */
    uint8_t index = 0;
    uint16_t fcf;
    
    // Long address need to be used if we do not have a short address
    // of if we are forced to use the long address
    // (e.g. in case the coordinator indicates pending data for us using our long address)  
    fcf = FCF_SET_FRAMETYPE(FCF_FRAMETYPE_MAC_CMD) | FCF_SET_SOURCE_ADDR_MODE(FCF_LONG_ADDR) | FCF_ACK_REQUEST;
    
    // Add the Frame Control Field
    data_request[index++] = ((fcf >> 8*0) & 0xFF);
    data_request[index++] = ((fcf >> 8*1) & 0xFF);

    /* 3. Add the sequence number. */
    data_request[index++] = IEEE802_15_4_GET_DSN();

    /* 4. Add the Source PAN ID */
    data_request[index++] = ((IEEE802_15_4_GET_PAN_ID() >> 8*0) & 0xFF);
    data_request[index++] = ((IEEE802_15_4_GET_PAN_ID() >> 8*1) & 0xFF);
    
    /* 5. Add Source Address.*/
    memcpy((void *)(&data_request[index]), (void *)(&(IEEE802_15_4_GET_EXTENDED_ADDRESS())), sizeof(uint64_t));
    index += sizeof(uint64_t);

    /* 6. Add command id. */
    data_request[index++] = DATAREQUEST;
        
    index += CRC_SIZE; // Two bytes are added for CRC. The radio transceiver will calculate it.
    
    if (true != ieee802_15_4_send(index, data_request)) {
        issue_associate_confirm(MAC_TRANSACTION_OVERFLOW, 0xFFFF);
    } else {
        IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_DATA_REQUEST_ACK);
    }
}


void ieee802_15_4_send_association_response(void) {
    /* The radio transceiver will be in RX_ACK_ON or BUSY_RX_AACK_ON when entering 
     * this function. Change state to TX_ARET_ON for frame transmission.
     */
    uint8_t time_out = 0xFF;
    do {
        delay_us(20);
        time_out--;
    } while ((CMD_PLL_ON != tat_get_state()) && (0 != time_out));
    
    /* Check if the wait loop above timed out, or if the RX_AACK_ON state was entered. */
    if (0 == time_out) {
        send_comm_status_indication(MAC_CHANNEL_ACCESS_FAILURE);
    } else if (true != ieee802_15_4_send(ASSOCIATION_RESPONSE_CMD_SIZE, association_response)) {
        send_comm_status_indication(MAC_CHANNEL_ACCESS_FAILURE);
    } else {
        /* Set new IEEE 802.15.4 MAC state. */
        IEEE802_15_4_SET_STATE(IEEE_MAC_WAITING_ASSOCIATE_RESPONSE_ACK);
        /* Transmission of the associated Association Response was successful. Wait for the ACK to arrive. */
    }
}


void ieee802_15_4_associate_response_process_ack(void *tx_status) {
    /* Set status of the MLME_COMM_STATUS.indication, and then execute the 
     * callback.
     */
    uint8_t mcsi_status = *((uint8_t *)tx_status);
    
    if (TRAC_SUCCESS == mcsi_status) {
        mcsi_status = MAC_SUCCESS;
    } else if (TRAC_NO_ACK == mcsi_status) {
        mcsi_status = MAC_NO_ACK;
    } else if (TRAC_CHANNEL_ACCESS_FAILURE == mcsi_status) {
        mcsi_status = MAC_CHANNEL_ACCESS_FAILURE;
    } else {
        mcsi_status = MAC_NO_DATA;
    }
    
    send_comm_status_indication(mcsi_status);  
}


static void send_comm_status_indication(uint8_t status) {
    ieee802_15_4_comm_status_indication_t msci_callback = ieee802_15_4_get_mlme_comm_status_indication();
    
    if (NULL == msci_callback) { return; }
    
    mcsi.status = status;
    
    /* Execute callback. */
    msci_callback(&mcsi);
    
    /* Ensure that the Coordinator is taken back to RX_AACK_ON state. */
    tat_reset();
    (bool)tat_set_state(RX_AACK_ON);
    IEEE802_15_4_SET_STATE(IEEE_MAC_STARTED);
}
/*EOF*/
