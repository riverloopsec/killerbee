// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the commando interface handler.
 *
 *      The command interface consists of handlers for any request that the
 *      ATmega1284p can receive from the connected ATmega3290p. Also the
 *      commando interface is responsible for forwarding any relevant indications
 *      from the ZigBee NWK layer.
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
 * $Id: cmd_if_1284p.c 41573 2008-05-13 13:39:00Z hmyklebust $
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
#include "vrt_mem.h"
#include "vrt_kernel.h"

#include "cmd_types.h"
#include "cmd_if_1284p.h"
#include "sipc.h"
#include "rvn_loc.h"
#include "bl.h"
#include "eep.h"
#include "wdt_avr.h"
#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_msg_types.h"
#include "ieee802_15_4_pib.h"
#include "self_programming.h"

#include "zigbee.h"
#include "zigbee_const.h"
#include "zigbee_nib.h"

//! \addtogroup applCmdAVRRAVEN1284p
//! @{
/*================================= MACROS           =========================*/
#define CMD_IF_SCAN_DURATION (7) //!< Scan duration used in NLME_NETWORK_DISCOVERY.request.
#define CMD_IF_SIPC_BUFFER_LENGTH (192) //!< Length of the SIPC RX buffer.
/*================================= TYEPDEFS         =========================*/
/*! \brief Container definining pointers to the different callbacks that is defined
 *         for the Command interface.
 */
typedef union CB_PTR_UNION_TAG {
    nlde_data_conf_t *ndc;
    nlme_network_discovery_conf_t *nndc;
    nlme_join_conf_t *njc;
} cb_ptr_t;

/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*! \brief This union stores the parameters used in the different modes of operation
 *         defined for the commando interface.
 */
static union {
    struct {
        nlde_data_req_t *ndr; //!< Pointer to NLDE_DATA.request message.
    } data;
    
    struct {
        uint8_t channel; //!< Channel to scan
        uint16_t pan_id; //!< PAN ID to later issue the NLME_JOIN.request on.
        nlme_network_discovery_req_t *ndr; //!< Pointer to NLME_NETWORK_DISCOVERY.request message.
    } discovery;
    
    struct {
        nlme_join_req_t *njr; //!< Pointer to NLME_JOIN.request message.
    } join;
    
    struct {
        nlme_leave_req_t *nlr; //!< Pointer to NLME_LEAVE.request message.
    } leave;
} params;

static bool initialized = false; //!< Variable indicating if the CMD_IF module is initialized.
static uint64_t ieee_address; //!< Stores the device's IEEE 802.15.4 MAC address.
static uint8_t sipc_rx_buffer[CMD_IF_SIPC_BUFFER_LENGTH]; //!< Data buffer for the sipc


/*================================= PROTOTYPES       =========================*/

/*! \brief  Reboot
 *
 *          The watchdog timer is used to generate a HW reset.
 */
static void reboot(void);


/*! \brief  Handler for new SIPC data packets
 *
 *  \param[in]  packet     Received packet
 */
static void cmd_sipc_new_packet_handler(void *packet);

/* ZigBee NWK layer callbacks. */
static void cmd_if_nwk_data_confirm_callback(nlde_data_conf_t *ndc);
static void cmd_if_nwk_data_indication(nlde_data_ind_t *ndi);
static void cmd_if_nwk_join_confirm_callback(nlme_join_conf_t *njc);
static void cmd_if_nwk_leave_confirm_callback(nlme_leave_conf_t *nlc);
static void cmd_if_nwk_leave_indication(nlme_leave_ind_t *nli);
static void cmd_if_nwk_discovery_confirm_callback(nlme_network_discovery_conf_t *ndc);
static void cmd_if_nwk_data_indication(nlde_data_ind_t *ndi);
static void cmd_if_nwk_join_indication(nlme_join_ind_t *nji);
static void cmd_if_nwk_leave_indication(nlme_leave_ind_t *nli);

bool cmd_if_1284p_init(void) {
    
    /* Get the device's 64-bit IEEE 802.15.4 address. */
    ieee_address = eep_read_ieee_address();
    
    /* Initialize the IEEE 802.15.4 MAC to ensure that the radio transceiver will
     * be at sleep, and not drawing any power from the batteries.
     */
    if (true != ieee802_15_4_init(ieee_address)) {
    } else if (true != sipc_init(CMD_IF_SIPC_BUFFER_LENGTH, sipc_rx_buffer, \
                          cmd_sipc_new_packet_handler)) {
	} else {
        initialized = true;
    }
    
	(uint8_t)ieee802_15_4_rx_disable();

    return initialized;
}


void cmd_if_1284p_deinit(void) {
    sipc_deinit();
}


static void rvn_loc_cmd_sleep(void *cmd) {
    
    /* Configure the system to go to sleep:
     * - Leave network, with store address set.
     * - Deinit ZigBee NWK.
     * - Set SIPC in sleep mode.
     * - Set CPU sleep.
     */
    
    /* Send dummy response for now. */
    rvn_loc_rsp_std_t rsp = { .id = RVN_LOC_RSP_OK };
    sipc_send_frame(sizeof(rvn_loc_rsp_std_t), (uint8_t *)&rsp);
}


static void rvn_loc_cmd_restart(void *cmd) {
    // Send response
    send_loc_simple_rsp(RVN_LOC_RSP_OK);
    reboot();
}


static void rvn_loc_cmd_enter_boot(void *cmd) {
    // Send response
    send_loc_simple_rsp(RVN_LOC_RSP_OK);
    reboot();
}

/*! \brief This function is used to handle a RVN_LOC_CMD_RADIO_CONNECT commando.
 *
 *  \note This function is not to be used directly. It is only available for the
 *        vrt_kernel's dispatcher.
 * 
 *  \param[in] cmd Pointer to the vrt_packet_t where the commando is stored.
 *
 */
static void rvn_loc_cmd_radio_connect(void *cmd)
{
    /* Get hold of the channel and PAN ID parameters sent with this commando. */
    rvn_loc_cmd_radio_connect_t *crc = (rvn_loc_cmd_radio_connect_t *)cmd;
    
    params.discovery.channel = crc->channel;
    params.discovery.pan_id  = crc->pan_id;

    /* Try to allocate memort to build the NLME_NETWORK_DISCOVERY.request. */
    params.discovery.ndr = (nlme_network_discovery_req_t *)MEM_ALLOC(nlme_network_discovery_req_t);
    
    /* Build NLME_NETWORK_DISCOVERY.request. */
    params.discovery.ndr->ChannelToScan = params.discovery.channel;
    params.discovery.ndr->ScanDuration = CMD_IF_SCAN_DURATION;
    params.discovery.ndr->nlme_callback_discovery_confirm = cmd_if_nwk_discovery_confirm_callback; 
        
    /* Verify that memory was successfully allocated. If it was, enable the 
     * ZigBee NWK layer and issue the NLME_NETWORK_DISCOVERY.request.
     */
    bool connect_status = false;
    if (NULL == params.discovery.ndr) {
    } else if (true != zigbee_init(ieee_address)) {
    } else if (true != zigbee_network_discovery_request(params.discovery.ndr)) {
    } else {
        connect_status = true;
    } // END: if (NULL == params.discovery.ndr) ...
    
    /* Give feedback to the ATmega3290p if the attempt to connect was not successful. */
    if (false == connect_status) {
        MEM_FREE(params.discovery.ndr);
        zigbee_deinit();
        rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
    } // END: if (false == connect_status) ...
}


static void rvn_loc_cmd_radio_disconnect(void *cmd) {
 
    /* Leave the network with the store NWK address parameter equal to false. */
    params.leave.nlr = (nlme_leave_req_t *)MEM_ALLOC(nlme_leave_req_t);
    
    /* Verify that memory was allocated. */
    if (NULL == (params.leave.nlr)) {
        /* Send response. */
        rvn_loc_cmd_std_t rsp;
        rsp.id = RVN_LOC_RSP_OK;
    
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
    } // END: if (NULL == (params.leave.nlr)) ...

    /* Build NLME_LEAVE.request message. */
    params.leave.nlr->DeviceAddress = IEEE802_15_4_GET_EXTENDED_ADDRESS();
    params.leave.nlr->RemoveChildren = false;
    params.leave.nlr->Rejoin = false;
    params.leave.nlr->ReuseAddress = true;
    params.leave.nlr->Silent = false;
    params.leave.nlr->nlme_callback_leave_confirm = cmd_if_nwk_leave_confirm_callback;
    if (true != zigbee_leave_request(params.leave.nlr)) {
        /* Free allocated memory to prevent memory leaks. */
        MEM_FREE(params.leave.nlr);
        
        /* Send response. */
        rvn_loc_cmd_std_t rsp;
        rsp.id = RVN_LOC_RSP_OK;
    
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
    }
}


static void rvn_loc_rx_on(void *cmd) {

    /* Call MLME_RX_ENABLE. */
    rvn_loc_cmd_std_t rsp;
    if (MAC_SUCCESS != ieee802_15_4_rx_enable()) {
        rsp.id = RVN_LOC_RSP_ERROR;
    } else {
        /* Set callbacks from ZigBee NWK layer. */
        zigbee_set_nlde_data_indication(cmd_if_nwk_data_indication);
        zigbee_set_nlme_join_indication(cmd_if_nwk_join_indication);
        zigbee_set_nlme_leave_indication(cmd_if_nwk_leave_indication);
        rsp.id = RVN_LOC_RSP_OK;
    } // END: if (MAC_SUCCESS != ieee802_15_4_rx_enable()) ...
    
    sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
}


static void rvn_loc_rx_off(void *cmd) {
    
    /* Call MLME_RX_ENABLE. */
    rvn_loc_cmd_std_t rsp;
    if (MAC_SUCCESS != ieee802_15_4_rx_disable()) {
        rsp.id = RVN_LOC_RSP_ERROR;
    } else {
        rsp.id = RVN_LOC_RSP_OK;
    } // END: if (MAC_SUCCESS != ieee802_15_4_rx_enable()) ...
    
    sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
}


static void rvn_loc_cmd_packet(void *cmd) {
    /* Cast the raw command to the correct type. */
    rvn_loc_cmd_ota_packet_t *cop = (rvn_loc_cmd_ota_packet_t *)cmd;
    
    /* First check if the frame length is valid. */
    if ((aMaxMACFrameSize - NWK_MIN_HEADER_OVERHEAD) < (cop->size)) {
        /* Send response to the ATmega3290p. */
        rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
        
        return;
    }
    
    /* Try to allocate necessary memory to build the NLDE_DATA.request message. A
     * variable with file scope must be used, since the final freeing of memory
     * is done when the NLDE_DATA.confirm is received.
     */
    params.data.ndr = (nlde_data_req_t *)MEM_ALLOC(nlde_data_req_t);
    
    if (NULL == (params.data.ndr)) {
        /* Send response to the ATmega3290p. */
        rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
        
        return;
    }
    
    /* Build NLDE_DATA.request message. */
    params.data.ndr->dst_addr_mode = 0;
    params.data.ndr->dst_addr = cop->adr;
    params.data.ndr->radius = 3;
    params.data.ndr->non_member_radius = 3;
    params.data.ndr->nsdu_length = cop->size;
    params.data.ndr->nsdu = cop->data;
    params.data.ndr->discovery_route = 0x00;
    params.data.ndr->nlde_callback_data_confirm = cmd_if_nwk_data_confirm_callback;
    
    /* Try to execute the primitive. */
    if (true != zigbee_data_request(params.data.ndr)) {
        /* Send response to the ATmega3290p. */
        rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
        
        /* Free any allocated memory. */
        MEM_FREE(params.data.ndr);
    }
}


static void rvn_loc_cmd_start_router(void *cmd) {
    
    rvn_loc_cmd_std_t rsp;
    
    /* Try to start router and send response to ATmega3290p. */    
    if (true != zigbee_start_router_request()) {
        rsp.id = RVN_LOC_RSP_ERROR;
    } else {
        rsp.id = RVN_LOC_RSP_OK;
    } // END: if (true != bool zigbee_start_router_request()) ...
    
    sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
}


static void rvn_loc_cmd_get_param(void *cmd) {
    /* Extract parameter type from command. */
    rvn_loc_cmd_get_param_t *cgp = (rvn_loc_cmd_get_param_t *)cmd;
    rvn_param_t param = cgp->param;
    
    /* Determine what kind of command received, and if it is supported. */
    if (NWK_ADDRESS == param) {
        uint8_t rsp_buffer[4];
        rvn_loc_rsp_get_param_t *rsp = (rvn_loc_rsp_get_param_t *)rsp_buffer;
        
        rsp->id = RVN_LOC_RSP_GET_PARAM;
        rsp->size = 2;
        rsp->data[0] = (NWK_NIB_GET_NWK_ADDRESS() >> 8 * 0) & 0xFF;
        rsp->data[1] = (NWK_NIB_GET_NWK_ADDRESS() >> 8 * 1) & 0xFF;
            
        sipc_send_frame(sizeof(rsp_buffer), (uint8_t *)rsp);
    } else if (IEEEE_ADDRESS == param) {
        uint8_t rsp_buffer[10];
        rvn_loc_rsp_get_param_t *rsp = (rvn_loc_rsp_get_param_t *)rsp_buffer;
        
        rsp->id = RVN_LOC_RSP_GET_PARAM;
        rsp->size = 8;
        rsp->data[0] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 0) & 0xFF;
        rsp->data[1] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 1) & 0xFF;
        rsp->data[2] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 2) & 0xFF;
        rsp->data[3] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 3) & 0xFF;
        rsp->data[4] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 4) & 0xFF;
        rsp->data[5] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 5) & 0xFF;
        rsp->data[6] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 6) & 0xFF;
        rsp->data[7] = (IEEE802_15_4_GET_EXTENDED_ADDRESS() >> 8 * 7) & 0xFF;
            
        sipc_send_frame(sizeof(rsp_buffer), (uint8_t *)rsp);
    } else if (FW_REV == param) {
        uint8_t rsp_buffer[6];
        rvn_loc_rsp_get_param_t *rsp = (rvn_loc_rsp_get_param_t *)rsp_buffer;
        
        
        // Build response
        rsp->id = RVN_LOC_RSP_GET_PARAM;
        rsp->size = 4;
        rsp->data[0] = sp_read_flash(FW_REV_ADR_M1284P_APP); // Read current application- and bootloader revision stored in progmem
        rsp->data[1] = sp_read_flash(FW_REV_ADR_M1284P_APP+1);
        rsp->data[2] = sp_read_flash(FW_REV_ADR_M1284P_BL);
        rsp->data[3] = sp_read_flash(FW_REV_ADR_M1284P_BL+1);
        
        sipc_send_frame(sizeof(rsp_buffer), (uint8_t *)rsp);
    } else {
        uint8_t rsp_buffer[2];
        rvn_loc_rsp_get_param_t *rsp = (rvn_loc_rsp_get_param_t *)rsp_buffer;
        
        rsp->id = RVN_LOC_RSP_GET_PARAM;
        rsp->size = 0;
            
        sipc_send_frame(sizeof(rsp_buffer), (uint8_t *)rsp);
    } // END: if (NWK_ADDRESS == param) ...
}


static void reboot(void) {
    wdt_enable(WDTO_15MS);
    while(1){
        ;
    }
}


static void cmd_sipc_new_packet_handler(void *raw_cmd) {
    /* Cast void packet to correct type and extract command type. */
    uint8_t cmd = *(uint8_t *)raw_cmd;
    
    switch (cmd) {
    case RVN_LOC_CMD_SLEEP:
        rvn_loc_cmd_sleep(raw_cmd);
        break;
    case RVN_LOC_CMD_RESTART:
        rvn_loc_cmd_restart(raw_cmd);
        break;
    case RVN_LOC_CMD_ENTER_BOOT:
        rvn_loc_cmd_enter_boot(raw_cmd);
        break;
    case RVN_LOC_CMD_RADIO_CONNECT:
        rvn_loc_cmd_radio_connect(raw_cmd);
        break;
    case RVN_LOC_CMD_RADIO_DISCONNECT:
        rvn_loc_cmd_radio_disconnect(raw_cmd);
        break;
    case RVN_LOC_CMD_RX_ON:
        rvn_loc_rx_on(raw_cmd);
        break;
    case RVN_LOC_CMD_RX_OFF:
        rvn_loc_rx_off(raw_cmd);
        break;
    case RVN_LOC_CMD_PACKET:
        rvn_loc_cmd_packet(raw_cmd);
        break;
    case RVN_LOC_CMD_START_ROUTER:
        rvn_loc_cmd_start_router(raw_cmd);
        break;
    case RVN_LOC_CMD_GET_PARAM:
        rvn_loc_cmd_get_param(raw_cmd);
        break;
    default:
        // Reply NOT SUPPORTED on all other requests
        send_loc_simple_rsp(RVN_LOC_RSP_NOT_SUPPORTED);
        break;
    }
    
    /* ACK packet. */
    SIPC_ACK_PACKET();
}



/*               Callbacks from NWK                                           */



static void cmd_if_nwk_data_confirm_callback(nlde_data_conf_t *ndc) {
    rvn_loc_cmd_std_t rsp;
    if (NWK_SUCCESS != (ndc->Status)) {
        rsp.id = RVN_LOC_RSP_ERROR;
    } else {
        rsp.id = RVN_LOC_RSP_OK;
    } // END: if (NWK_SUCCESS != (ndc->Status)) ...
    
    /* Report status of the node's attempt to join the network. */
    sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
    
    /* Free allocated memory. */
    MEM_FREE(params.data.ndr);
}

static void cmd_if_nwk_data_indication(nlde_data_ind_t *ndi) {
    uint8_t *buffer = MEM_ALLOC_ARRAY(uint8_t, (ndi->NsduLength) + 5);
    
    if (NULL == buffer) { return; }
    
    /* Build rvn_loc_evt_ota_packet_t. */
    rvn_loc_evt_ota_packet_t *rsp = (rvn_loc_evt_ota_packet_t *)buffer;
    rsp->id = RVN_LOC_EVT_OTA_PACKET;
    rsp->lqi = ndi->LinkQuality;
    memcpy((void *)(&(rsp->adr)), (void *)(&(ndi->SrcAddr)), sizeof(uint16_t));
    rsp->size = ndi->NsduLength;
    memcpy((void *)(rsp->data),(void *)(ndi->Nsdu), (ndi->NsduLength));
    
    /* Report status of the node's attempt to join the network. */
    sipc_send_frame((ndi->NsduLength) + 5, (uint8_t *)rsp);
    MEM_FREE(buffer);
}


/*! \brief This callback is executed when the ZigBee NWK layer is reporting back
 *         the status of a NLME_NETWORK_DISCOVERY.request.
 *
 *  \param[in] ndc Pointer to the nlme_network_discovery_conf_t message.
 */
static void cmd_if_nwk_discovery_confirm_callback(nlme_network_discovery_conf_t *ndc) {
    /* This is a part of the connect procedure. If the discovery was successful it
     * is time to issue the NLME_JOIN.request commando.
     */
    if (NWK_SUCCESS != (ndc->Status)) {
        /* The NLME_NETWORK_DISCOVERY.request was not successful, so allocated
         * memory must be freed up and an error response sent to the ATmega3290p.
         */
        MEM_FREE(params.discovery.ndr);
        zigbee_deinit();
        rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
        sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
    } else {
        /* Free up unused memory. */
        MEM_FREE(params.discovery.ndr);
        
        uint16_t pan_id = params.discovery.pan_id;
        
        /* Build the NLME_JOIN.request and issue it */
        params.join.njr = (nlme_join_req_t *)MEM_ALLOC(nlme_join_req_t);
        
        if (NULL == params.join.njr) {
            zigbee_deinit();
            rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
            sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
        } else {
            params.join.njr->PANId = pan_id;
            params.join.njr->RejoinNetwork = NWK_JOIN_THROUGH_ASSOCIATION;
            params.join.njr->JoinAsRouter = false;
            params.join.njr->PowerSource = 0;
            params.join.njr->RxOnWhenIdle = true;
            params.join.njr->nlme_callback_join_confirm = cmd_if_nwk_join_confirm_callback;
                
            if (true != zigbee_join_request(params.join.njr)) {
                MEM_FREE(params.join.njr);
                zigbee_deinit();
                rvn_loc_cmd_std_t rsp = { .id = RVN_LOC_RSP_ERROR };
                sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
            } // END: if (true != zigbee_join_request(params.join.njr)) ...
        } // END: if (NULL == params.join.njr) ...
    } // END: if (NWK_SUCCESS != (ndc->Status)) ...
}


/*! \brief This callback is executed when the ZigBee NWK layer is reporting back
 *         the status of a NLME_JOIN.request.
 *
 *  \param[in] njc Pointer to the nlme_join_conf_t message.
 */
static void cmd_if_nwk_join_confirm_callback(nlme_join_conf_t *njc) {
    rvn_loc_cmd_std_t rsp;
    if (NWK_SUCCESS != (njc->Status)) {
        zigbee_deinit();
        rsp.id = RVN_LOC_RSP_ERROR;
    } else {
        IEEE802_15_4_SET_RX_ON_WHEN_IDLE(true);
        rsp.id = RVN_LOC_RSP_OK;
    } // END: if (NWK_SUCCESS != (njc->Status)) ...
    
    /* Report status of the node's attempt to join the network. */
    sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
    
    /* Free allocated memory. */
    MEM_FREE(params.join.njr);
}


static void cmd_if_nwk_join_indication(nlme_join_ind_t *nji) {
    
}


static void cmd_if_nwk_leave_indication(nlme_leave_ind_t *nli) {
    
}


static void cmd_if_nwk_leave_confirm_callback(nlme_leave_conf_t *nlc) {
    /* Return the previously allocated memory. */
    MEM_FREE(params.leave.nlr);
    
    /* Send response. */
    rvn_loc_cmd_std_t rsp;
    rsp.id = RVN_LOC_RSP_OK;
    
    sipc_send_frame(sizeof(rsp), (uint8_t *)&rsp);
}
//! @}
/*EOF*/
