// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Implementation of the command interface for the ATmega3290p on AVRRAVEN
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
 * $Id: cmd_if_3290p.c 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include "cmd_if_3290p.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"
#include "vrt_kernel.h"
#include "vrt_mem.h"
#include "ota.h"
#include "loc.h"
#include "sipc.h"

//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/
//! Length of the SIPC RX buffer.
#define CMD_IF_SIPC_BUFFER_LENGTH (127)

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTION PROTOTYPES =======================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Flag holding module initialization status
static bool cmd_if_initialized = false;

//! Static buffer for use with "sipc" 
static uint8_t sipc_rx_buffer[CMD_IF_SIPC_BUFFER_LENGTH];

/*========================= PRIVATE FUNCTIONS            =====================*/

/*! \brief  Function for handling new sipc packets
 *
 *          When a new packet has been successfully trasfered over the sipc interface,
 *          this function is called with a pointer to the newly arrived package.
 *          The package is identified by the first byte, and if the identifier is known
 *          an event is posted with the correct handler.
 */
static void cmd_if_sipc_packet_handler(void *packet);

/*========================= IMPLEMENTATION           =========================*/
bool cmd_if_init(void)
{
    if (true != sipc_init(CMD_IF_SIPC_BUFFER_LENGTH, sipc_rx_buffer,
                          cmd_if_sipc_packet_handler)) {
    } else {
        cmd_if_initialized = true;
    }

    return cmd_if_initialized;
}

void cmd_if_deinit(void)
{
    cmd_if_initialized = false;
    sipc_deinit();
}

static void cmd_if_sipc_packet_handler(void *packet)
{
    // Cast void packet to correct type and extract the packet identifier
    uint8_t id = *(uint8_t *)packet;
    vrt_event_handler_t event_handler;
    
    // Try to recognize the identifier
    switch (id) {
        case RVN_LOC_RSP_OK:
            event_handler = loc_rsp_ok;
        break;
        case RVN_LOC_RSP_ERROR:
            event_handler = loc_rsp_fail;
        break;
        case RVN_LOC_RSP_NOT_SUPPORTED:
            event_handler = loc_rsp_fail;
        break;
        case RVN_LOC_RSP_BUSY:
            event_handler = loc_rsp_fail;
        break;
        case RVN_LOC_RSP_GET_PARAM:
            event_handler = loc_rsp_get_param;
        break;
        case RVN_LOC_EVT_OTA_PACKET:
            event_handler = loc_evt_ota_packet;
        break;
        case RVN_LOC_EVT_BL_ENTERED:
            event_handler = loc_evt_bl_entered;
        break;
        case RVN_LOC_EVT_APP_STARTED:
            event_handler = loc_evt_app_started;
        break;
        default:
            // If identifier not recognized, ACK packet to free buffer and return.
            SIPC_ACK_PACKET();
        return;
    }
    
    // Posting the event (the .id identifies the event) will call the correct
    // handler which ACKs the packet when done with it. If event posting failes,
    // the packet is ACK'ed emediately to free the buffer.
    if (true != vrt_post_event(event_handler, packet)) {
        SIPC_ACK_PACKET();
    }
}

//! @}
/*EOF*/
