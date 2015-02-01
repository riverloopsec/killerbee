// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the command interface that controls the RZUSBSTICK
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
 * $Id: cmd_if.c 41247 2008-05-04 18:29:02Z vkbakken $
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
#include "vrt_mem.h"

#include "wdt_avr.h"
#include "rzusbstickcommon.hh"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_task.h"
#include "eep.h"

#include "air_capture.h"
#include "cmd_if.h"

#include "ieee802_15_4.h"
#include "ieee802_15_4_const.h"
#include "ieee802_15_4_msg_types.h"
#include "ieee802_15_4_pib.h"

#include "zigbee.h"
#include "zigbee_conf.h"
#include "zigbee_const.h"
#include "zigbee_msg_types.h"

//! \addtogroup applCmdRzUsbStick
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*! \brief CMD_SIGN_OFF struct. */
typedef struct CMD_SIGN_OFF_STRUCT_TAG {
    uint8_t type;
    uint8_t reset_flag;
} cmd_sign_off_t;


/*! \brief CMD_GET_PARAMETER struct. */
typedef struct CMD_GET_PARAMETER_STRUCT_TAG {
    uint8_t type;
    uint8_t parameter;
} cmd_get_parameter_t;


/*! \brief CMD_SET_MODE struct. */
typedef struct CMD_SET_MODE_STRUCT_TAG {
    uint8_t type;
    uint8_t mode;
} cmd_set_mode_t;


/*! \brief CMD_SET_CHANNEL struct. */
typedef struct CMD_SET_CHANNEL_STRUCT_TAG {
    uint8_t type;
    uint8_t channel;
} cmd_set_channel_t;


/*! \brief CMD_OPEN_STREAM struct. */
typedef struct CMD_OPEN_STREAM_STRUCT_TAG {
    uint8_t type;
} cmd_open_stream_t;


/*! \brief CMD_CLOSE_STREAM struct. */
typedef struct CMD_CLOSE_STREAM_STRUCT_TAG {
    uint8_t type;
} cmd_close_stream_t;


/*! \brief CMD_CHANNEL_SCAN struct. */
typedef struct CMD_CHANNEL_SCAN_STRUCT_TAG {
    uint8_t type;
    uint8_t channel;
    uint8_t scan_type;
    uint8_t scan_duration;
} cmd_channel_scan_t;


/*! \brief CMD_INJECT_FRAME struct. */
typedef struct CMD_INJECT_FRAME_STRUCT_TAG {
    uint8_t type;
    uint8_t length;
    uint8_t frame[];
} cmd_inject_frame_t;


/*! \brief CMD_JAMMER_ON struct. */
typedef struct CMD_JAMMER_ON_STRUCT_TAG {
    uint8_t type;
} cmd_jammer_on_t;


/*! \brief CMD_JAMMER_OFF struct. */
typedef struct CMD_JAMMER_OFF_STRUCT_TAG {
    uint8_t type;
} cmd_jammer_off_t;


/*! \brief CMD_NLDE_DATA_REQUEST struct. */
typedef struct CMD_NLDE_DATA_REQUEST_STRUCT_TAG {
    uint8_t type;
    uint16_t dest_addr;
    uint8_t radius;
    uint8_t non_member_radius;
    uint8_t discovery_route;
    uint8_t length;
    uint8_t data[];
} cmd_nlde_data_request_t;


/*! \brief CMD_NLME_FORMATION_REQUEST struct. */
typedef struct CMD_NLME_FORMATION_REQUEST_STRUCT_TAG {
    uint8_t type;
    uint8_t channel;
    uint16_t pan_id;
} cmd_nlme_formation_request_t;


/*! \brief CMD_NLME_PERMIT_JOINING_REQUEST struct. */
typedef struct CMD_NLME_PERMIT_JOINING_REQUEST_STRUCT_TAG {
    uint8_t type;
    bool join_permitted;
} cmd_nlme_permit_joining_request_t;


/*! \brief CMD_NLME_JOIN_REQUEST struct. */
typedef struct CMD_NLME_JOIN_REQUEST_STRUCT_TAG {
    uint8_t type;
    uint16_t PANId;
    uint8_t RejoinNetwork;
    bool JoinAsRouter;
    uint8_t PowerSource;
    bool RxOnWhenIdle;
} cmd_nlme_join_request_t;


/*! \brief CMD_NLME_LEAVE_REQUEST struct. */
typedef struct CMD_NLME_LEAVE_REQUEST_STRUCT_TAG {
    uint8_t type;
    uint64_t device_address;
    bool remove_children;
    bool rejoin;
    bool reuse_address;
    bool silent;
} cmd_nlme_leave_request_t;


/*! \brief CMD_NLME_DISCOVERY_REQUEST struct. */
typedef struct CMD_NLME_DISCOVERY_REQUEST_STRUCT_TAG {
    uint8_t type;
    uint8_t channel;
    uint8_t duration;
} cmd_nlme_discovery_request_t;


/*! \brief CMD_NLME_START_ROUTER_REQUEST struct. */
typedef struct CMD_NLME_START_ROUTER_REQUEST_STRUCT_TAG {
    uint8_t type;
} cmd_nlme_router_start_request_t;

/*! \brief Enumeration that defines the internal command handler states. */
typedef enum {
    CMD_IF_NOT_INITIALIZED = 0,
    CMD_IF_INITIALIZED,
    CMD_IF_AC_MODE,
    CMD_IF_MAC_MODE,
    CMD_IF_NWK_MODE,
} cmd_if_state_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static cmd_if_state_t cmd_if_state = CMD_IF_NOT_INITIALIZED;

static nlde_data_req_t *ndr = NULL; //!< Pointer to NLDE_DATA.request message.
static nlme_join_req_t *njr = NULL; //!< Pointer to NLME_JOIN.request message.
static nlme_leave_req_t *nlr = NULL; //!< Pointer to NLME_LEAVE.request message.
static nlme_network_discovery_req_t *nndr = NULL; //!< Pointer to NLME_NETWORK_DISCOVERY.request message.

/* Variables used for fifo handling and control. */
static uint8_t *nwk_event_fifo[NWK_EVENT_FIFO_SIZE]; //!< Queue of pointers to the allocated NWK_EVENTs.
static uint8_t fifo_head;
static uint8_t fifo_tail;
static uint8_t items_used;
static uint8_t items_free;

/* Variables used when doing USB transactions. */
static uint8_t bytes_left;
static uint8_t packets_left;
static uint8_t *data_ptr;

static uint16_t nwk_events_missed = 0; //!< Variable that will be incremented each time a NWK event was signalled but could not be handled.

static uint64_t ieee_address = 0x12345678; //!< Unique 64-bit address used as USB serial number and for the IEEE 80.15.4 MAC.
/*================================= PROTOTYPES       =========================*/
/*! \brief This function will set-up the AVR's watch dog timer to generate a reset. */
static void reboot(void);

/*! \brief This is an internal command handler that performs a sign off.
 *
 *  \param[in] cmd_sign_off Pointer to a CMD_SIGN_OFF command.
 */
static void cmd_if_sign_off(void *cmd_sign_off);

/*! \brief This is an internal command handler that performs a sign on.
 *
 *  \param[in] cmd_sign_on Pointer to a CMD_SIGN_ON command.
 */
static void cmd_if_sign_on(void *cmd_sign_on);

/*! \brief This is an internal command handler that makes it possible to fetch
 *         parameters from the command interface, IEEE 802.15.4 MAC and ZigBee NWK.
 *
 *  \param[in] cmd_get_parameter Pointer to a CMD_GET_PARAMETER command.
 */
static void cmd_get_parameter(void *cmd_get_parameter);

/*! \brief This is an internal command handler that writes a new value to one of the
 *         supported parameters.
 *
 *  \param[in] cmd_set_parameter Pointer to a CMD_SET_PARAMETER command.
 */
static void cmd_set_parameter(void *cmd_set_parameter);

/*! \brief This is an internal command handler that will perform a self test
 *         of the RZUSBSTICK.
 *
 *  \note This handler is not implemented yet.
 *
 *  \param[in] cmd_self_test Pointer to a CMD_SELF_TEST command.
 */
static void cmd_self_test(void *cmd_self_test);

/*! \brief This is an internal command handler that validates the stack usage at 
 *         the moment on the RZUSBSTICK.
 *
 *  \note This handler is not implemented yet.
 *
 *  \param[in] cmd_check_stack_usage Pointer to a CMD_CHECK_STACK_USAGE command.
 */
static void cmd_check_stack_usage(void *cmd_check_stack_usage);

/*! \brief This is an internal command handler that performs a memory test.
 *
 *  \note This handler is not implemented yet.
 *
 *  \param[in] cmd_memory_test Pointer to a CMD_MEMORY_TEST command.
 */
static void cmd_memory_test(void *cmd_memory_test);

/*! \brief This is an internal command handler selects the mode that the
 *         command interface will work in:
 *         - ZigBee Corrdinator
 *         - AirCapture
 *
 *  \param[in] cmd_set_mode Pointer to a CMD_SET_MODE command.
 */
static void cmd_set_mode(void *cmd_set_mode);

/*! \brief This internal command handler sets the channel that the AirCapture
 *         application is fetching frames on.
 *
 *  \param[in] cmd_set_channel Pointer to a CMD_SET_CHANNEL command.
 */
static void cmd_set_channel(void *cmd_set_channel);

/*! \brief This internal command handler enables captured frames to be
 *         uploaded from the RZUSBSTICK.
 *
 *  \param[in] cmd_open_stream Pointer to a CMD_OPEN_STREAM command.
 */
static void cmd_open_stream(void *cmd_open_stream);

/*! \brief This internal command handler stops the AirCapture application to capture
 *         frames and store them in the frame queue.
 *
 *  \param[in] cmd_close_stream Pointer to a CMD_CLOSE_STREAM command.
 */
static void cmd_close_stream(void *cmd_close_stream);

/*! \brief This is an internal command handler that starts a channel scan through
 *         the AirCapture application.
 *
 *  \note This handler is not implemented.
 *
 *  \param[in] cmd_channel_scan Pointer to a CMD_CHANNEL_SCAN command.
 */
static void cmd_channel_scan(void *cmd_channel_scan);

/*! \brief This is an internal command handler that will terminate a previously
 *         started channel scan.
 *
 *  \note This handler is not implemented.
 *
 *  \param[in] cmd_channel_scan_stop Pointer to a CMD_CHANNEL_SCAN_STOP command.
 */
static void cmd_channel_scan_stop(void *cmd_channel_scan_stop);

/*! \brief This is an internal command handler injects a new frame onto the 
 *         network through the AirCapture application.
 *
 *  \param[in] cmd_inject_frame Pointer to a CMD_INJECT_FRAME command.
 */
static void cmd_inject_frame(void *cmd_inject_frame);

/*! \brief This is an internal command handler that enables the jammer available
 *         from the AirCapture application.
 *
 *  \param[in] cmd_jammer_on Pointer to a CMD_JAMMER_ON command.
 */
static void cmd_jammer_on(void *cmd_jammer_on);

/*! \brief This is an internal command handler that stops the previously started
 *         jammer.
 *
 *  \param[in] cmd_jammer_off Pointer to a CMD_JAMMER_OFF command.
 */
static void cmd_jammer_off(void *cmd_jammer_off);

/*! \brief This is an internal command handler that reboots the system into the
 *         bootloader.
 *
 *  \param[in] cmd_enter_boot Pointer to a CMD_ENTER_BOOT command.
 */
static void cmd_enter_boot(void *cmd_enter_boot);

/*! \brief This is an internal command handler that issues the NLDE_DATA.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_nlde_data_request Pointer to a CMD_NLDE_DATA_REQUEST command.
 */
static void cmd_nlde_data_request(void *cmd_nlde_data_request);

/*! \brief This is an internal command handler that issues the NLME_FORMATION.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_nlme_formation_request Pointer to a CMD_NLME_FORMATION_REQUEST command.
 */
static void cmd_nlme_formation_request(void *cmd_nlme_formation_request);

/*! \brief This is an internal command handler that issues the NLME_PERMIT_JOIN.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_nlme_permit_join_request Pointer to a CMD_NLME_PERMIT_JOIN_REQUEST command.
 */
static void cmd_nlme_permit_join_request(void *cmd_nlme_permit_join_request);

/*! \brief This is an internal command handler that issues the NLME_JOIN.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_nlme_join_request Pointer to a CMD_NLME_JOIN_REQUEST command.
 */
static void cmd_nlme_join_request(void *cmd_nlme_join_request);

/*! \brief This is an internal command handler that issues the NLME_LEAVE.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_nlme_leave_request Pointer to a CMD_NLME_LEAVE_REQUEST command.
 */
static void cmd_nlme_leave_request(void *cmd_nlme_leave_request);

/*! \brief This is an internal command handler that issues the NLME_DISCOVERY.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_nlme_discovery_request Pointer to a CMD_NLME_DISCOVERY_REQUEST command.
 */
static void cmd_nlme_discovery_request(void *cmd_nlme_discovery_request);

/*! \brief This is an internal command handler that issues the NLME_START_ROUTER.request
 *         to the ZigBee NWK.
 *
 *  \param[in] cmd_start_router_request pointer to a CMD_NLME_START_ROUTER_REQUEST command.
 */
static void cmd_nlme_start_router_request(void *cmd_start_router_request);

/*! \brief This function will partially parse the received raw command, and select
 *         the associated internal handler for this particular command.
 *
 *  \param[in] raw_cmd Pointer to raw command as received on the USB interface.
 */
static void cmd_if_dispatch(void *raw_cmd);

/*! \brief This function is executed each time an USB transaction is completed.
 *
 *  When an USB transaction is completed, a new command is received. There is
 *  a command handler defined in the vrt_kernel for each event that implements
 *  the command specific behavior.
 *
 *  \param[in] length Length of received data.
 *  \param[in] data Pointer to the received data from last USB transaction.
 */
static void usb_transaction_done(uint16_t length, uint8_t *data);

/*! \brief This function is executed when a NLDE_DATA.confirm is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] ndc Pointer to nlde_data_conf_t struct.
 */
static void cmd_nwk_data_confirm_callback(nlde_data_conf_t *ndc);

/*! \brief This function is executed when a NLDE_DATA.indication is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] ndi Pointer to nlde_data_ind_t struct.
 */
static void cmd_nwk_data_indication(nlde_data_ind_t *ndi);

/*! \brief This function is executed when a NLME_JOIN.confirm is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] njc Pointer to nlde_data_conf_t struct.
 */
static void cmd_nwk_join_confirm_callback(nlme_join_conf_t *njc);

/*! \brief This function is executed when a NLME_JOIN.indication is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] nji Pointer to nlme_join_ind_t struct.
 */
static void cmd_nwk_join_indication(nlme_join_ind_t *nji);

/*! \brief This function is executed when a NLME_LEAVE.confirm is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] nlc Pointer to nlme_leave_conf_t struct.
 */
static void cmd_nwk_leave_confirm_callback(nlme_leave_conf_t *nlc);

/*! \brief This function is executed when a NLME_LEAVE.indication is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] nli Pointer to nlme_leave_ind_t struct.
 */
static void cmd_nwk_leave_indication(nlme_leave_ind_t *nli);

/*! \brief This function is executed when a NLME_DISCOVERY.confirm is issued by the
 *         ZigBee NWK layer.
 *
 *  \param[in] ndc Pointer to nlme_network_discovery_conf_t struct.
 */
static void cmd_nwk_discovery_confirm_callback(nlme_network_discovery_conf_t *ndc);


bool cmd_if_init(void) {
    /* Initialize local variables. */
    ndr = NULL;
    ieee_address = eep_read_ieee_address();
    nwk_events_missed = 0;    
    
    for (uint8_t i = 0; (i < NWK_EVENT_FIFO_SIZE); i++) {
        uint8_t *nwk_event = (uint8_t *)MEM_ALLOC_ARRAY(uint8_t, CMD_EVENT_SIZE);
        
        if (NULL == nwk_event) {
            goto init_failed;
        } else {
            nwk_event_fifo[i] = nwk_event;
        }
    }
    
    /* Initialize USB devic driver. */
    usb_task_init(ieee_address);
    
    /* Set up the transaction descriptor for the OUT end point where commands will
     * be received.
     */
    usb_trans_descriptor_t desc;
    desc.ep = EP_OUT;
    desc.done_callback = usb_transaction_done;
    
    if (USB_SUCCESS != usb_ep_open(&desc)) {
        goto init_failed;
    }
    
    cmd_if_state = CMD_IF_INITIALIZED;
    
    return true;
    
    
    
    /* Handle failed initialization. */
    
  
  
    init_failed:
    
    for (uint8_t i = 0; (i < NWK_EVENT_FIFO_SIZE); i++) {
        MEM_FREE(nwk_event_fifo[i]);
        nwk_event_fifo[i] = (uint8_t *)NULL;
    }
    
    /* Close EP and turn the USB macro off. */
    usb_ep_close();
    usb_task_deinit();  
      
    return false;
}


void cmd_if_deinit(void) {
    if (CMD_IF_NOT_INITIALIZED == cmd_if_state) { return ; } // No point in turning off if not even started.
    
    cmd_if_state = CMD_IF_NOT_INITIALIZED;
    
    /* Clear any memory that potentially can be active. */
    MEM_FREE(ndr);
    ndr = NULL;
        
    for (uint8_t i = 0; (i < NWK_EVENT_FIFO_SIZE); i++) {
        MEM_FREE(nwk_event_fifo[i]);
        nwk_event_fifo[i] = (uint8_t *)NULL;
    }
    
    /* Close EP and turn the USB macro off. */
    usb_ep_close();
    usb_task_deinit();
}


void cmd_if_task(void) {
    /* The cmd_if_task function has one main function to fullfill: Forward NWK_EVENTs
     * if any available.
     */
    if (CMD_IF_NWK_MODE != cmd_if_state) { return; }
    
    /* Check if there is data to be transmitted. */
    if (0 == items_used) { return; }
        
    /* Select the event EP. */
    UENUM = EP_EVENT;
        
    /* Check that it is possible to fill at least one bank (64 bytes) in the DPRAM. */
    if ((UEINTX & (1 << TXINI)) != (1 << TXINI)) { return; }
        
    /* ACK TX_IN. */
    UEINTX &= ~(1 << TXINI);
        
    /* If there is no acdu allocated, but data left in the AirCapture fifo.
     * One acdu must be allocated from the fifo's tail, however the tail's
     * position will not be updated until the complete acdu is written.
     */
    if (0 == bytes_left) {
        if (0 == packets_left) {
            /* Get new NWK_EVENT. */
            data_ptr = nwk_event_fifo[fifo_tail];
            bytes_left = *(data_ptr + 1);
            packets_left = (uint8_t)(((uint16_t)(bytes_left + 64 - 1)) >> 6);
            if (0 == (bytes_left % 64)) { packets_left++ ; }
        } else if (1 == packets_left) {
            /* Send Zero Length Packet and then update tail pointer. */
            UEINTX &= ~(1 << FIFOCON);
                
            ENTER_CRITICAL_REGION();
                
            if ((NWK_EVENT_FIFO_SIZE - 1) == fifo_tail) {
                fifo_tail = 0;
            } else {
                fifo_tail++;
            }
                
            items_used--;
            items_free++;
                
                
            LEAVE_CRITICAL_REGION();
                
            return;
        }
    }
    
    /* At least one byte to send. */
    do {
        UEDATX = *data_ptr;
        data_ptr++;
        bytes_left--;
    } while ((0 != bytes_left) && ((UEINTX & (1 << RWAL)) == (1 << RWAL)));
        
    /* Either 64 bytes or the last byte in a packet is written. */
    packets_left--;
        
    /* ACK writing to the bank. */
    UEINTX &= ~(1 << FIFOCON);
        
    /* Check if it is time ti move tail. */
    if ((0 == bytes_left) && (0 == packets_left)) {
        ENTER_CRITICAL_REGION();
                
        if ((NWK_EVENT_FIFO_SIZE - 1) == fifo_tail) {
            fifo_tail = 0;
        } else {
            fifo_tail++;
        }
                
        items_used--;
        items_free++;
                
                
        LEAVE_CRITICAL_REGION(); 
    }
}


static void reboot(void) {
    wdt_enable(WDTO_15MS);
    
    while (true) {
        ;
    }
}



/*                          COMMAND HANDLERS                                  */



static void cmd_if_sign_off(void *cmd_sign_off) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(RESP_SUCCESS);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_sign_on(void *cmd_sign_on) {
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SIGN_ON);
    Usb_write_byte(10);
	Usb_write_byte('K');
	Usb_write_byte('I');
	Usb_write_byte('L');
	Usb_write_byte('L');
	Usb_write_byte('E');
	Usb_write_byte('R');
    Usb_write_byte('B');
    Usb_write_byte('0');
    Usb_write_byte('0');
    Usb_write_byte('1');
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_get_parameter(void *cmd_get_parameter) {
    /* Cast the raw command into the correct type. */
    cmd_get_parameter_t *cgp = (cmd_get_parameter_t *)cmd_get_parameter;
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    if (PARAM_NWK_MAX_CHILDREN == (cgp->parameter)) {
        Usb_write_byte(RESP_GET_PARAMETER);
        Usb_write_byte(0x01);
        Usb_write_byte(ZIGBEE_MAX_NMBR_OF_CHILDREN);
    } else if (PARAM_NWK_MAX_ROUTRES == (cgp->parameter)) {
        Usb_write_byte(RESP_GET_PARAMETER);
        Usb_write_byte(0x01);
        Usb_write_byte(ZIGBEE_MAX_NMBR_OF_ROUTERS);
    } else if (PARAM_NWK_MAX_DEPTH == (cgp->parameter)) {
        Usb_write_byte(RESP_GET_PARAMETER);
        Usb_write_byte(0x01);
        Usb_write_byte(ZIGBEE_MAX_NWK_DEPTH);
    } else if (PARAM_SW_REV == (cgp->parameter)) {
        Usb_write_byte(RESP_GET_PARAMETER);
        Usb_write_byte(0x02);
        Usb_write_byte(RZRAVEN_FW_MAJ);
        Usb_write_byte(RZRAVEN_FW_MIN);
    } else if (PARAM_IEEE_ADDRESS == (cgp->parameter)) {
        Usb_write_byte(RESP_GET_PARAMETER);
        Usb_write_byte((ieee_address >> 8*0) & 0xFF);
        Usb_write_byte((ieee_address >> 8*1) & 0xFF);
        Usb_write_byte((ieee_address >> 8*2) & 0xFF);
        Usb_write_byte((ieee_address >> 8*3) & 0xFF);
        Usb_write_byte((ieee_address >> 8*4) & 0xFF);
        Usb_write_byte((ieee_address >> 8*5) & 0xFF);
        Usb_write_byte((ieee_address >> 8*6) & 0xFF);
        Usb_write_byte((ieee_address >> 8*7) & 0xFF);
    } else {
        /* Unknown parameter: Semantical error. */
        Usb_write_byte(RESP_SEMANTICAL_ERROR);
    }

    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_set_parameter(void *cmd_set_parameter) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_self_test(void *cmd_self_test) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_check_stack_usage(void *cmd_check_stack_usage) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_memory_test(void *cmd_memory_test) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_set_mode(void *cmd_set_mode) {
    /* Cast the raw command to the correct type. */
    cmd_set_mode_t *sm = (cmd_set_mode_t *)cmd_set_mode;
    
    /* Reset internal variables. */
    fifo_head  = 0;
    fifo_tail  = 0;
    items_used = 0;
    items_free = NWK_EVENT_FIFO_SIZE;
    bytes_left = 0;
    packets_left = 0;
    data_ptr = NULL;
    nwk_events_missed = 0;
    
    /* Tear down any old modes that could be running. */
    air_capture_deinit();
    zigbee_deinit();
        
    cmd_if_state = CMD_IF_INITIALIZED;
    uint8_t set_mode_status = RESP_SEMANTICAL_ERROR;
    
    if (CMD_MODE_AC == (sm->mode)) {
        if (true != air_capture_init()) {
            set_mode_status = RESP_HW_TIMEOUT;
        } else {
            cmd_if_state = CMD_IF_AC_MODE;
            set_mode_status = RESP_SUCCESS;
        }
    } else if (CMD_MODE_NWK == (sm->mode)) {
        if (true != zigbee_init(ieee_address)){
            set_mode_status = RESP_HW_TIMEOUT;
        } else {
            /* Set callbacks from ZigBee NWK layer. */
            zigbee_set_nlde_data_indication(cmd_nwk_data_indication);
            zigbee_set_nlme_join_indication(cmd_nwk_join_indication);
            zigbee_set_nlme_leave_indication(cmd_nwk_leave_indication);
                
            cmd_if_state = CMD_IF_NWK_MODE;
            set_mode_status = RESP_SUCCESS;
        }
    } else {
        set_mode_status = RESP_SEMANTICAL_ERROR;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(set_mode_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_set_channel(void *cmd_set_channel) {
    /* Cast the raw command to the correct type. */
    cmd_set_channel_t *sc = (cmd_set_channel_t *)cmd_set_channel;
    
    /* Try to set the requested channel. */
    uint8_t set_channel_status = RESP_SEMANTICAL_ERROR;
    if (true != air_capture_set_channel((sc->channel))) {
    } else {
        set_channel_status = RESP_SUCCESS;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(set_channel_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_open_stream(void *cmd_open_stream) {
    uint8_t open_status = RESP_SEMANTICAL_ERROR;
    
    /* Try to open the strean of captured frames. */
    if (true != air_capture_open_stream()) {
    } else {
        open_status = RESP_SUCCESS;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(open_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_close_stream(void *cmd_close_stream) {
    uint8_t close_status = RESP_SEMANTICAL_ERROR;
    
    /* Try to close the strean of captured frames. */
    if (true != air_capture_close_stream()) {
    } else {
        close_status = RESP_SUCCESS;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(close_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_channel_scan(void *cmd_channel_scan) {
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_channel_scan_stop(void *cmd_channel_scan_stop) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}

/* Original cmd_inject_frame() function not implemented */
/*
static void cmd_inject_frame(void *cmd_inject_frame) {
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_NOT_IMPLEMENTED);
        
    Usb_send_in();
    
    usb_ep_ack_transaction();
}
*/

static void cmd_inject_frame(void *cmd_inject_frame) {
    uint8_t *frame = (uint8_t *)cmd_inject_frame;
    uint8_t inject_frame_status = RESP_PRIMITIVE_FAILED;
    uint8_t len = frame[1];
    inject_frame_status = air_capture_inject_frame(len, frame+2);
	
	if (inject_frame_status == 0) {
        inject_frame_status = RESP_SUCCESS;
    }
	
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    //Usb_write_byte(inject_frame_status);
	Usb_write_byte(inject_frame_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_jammer_on(void *cmd_jammer_on) {
    /* Turn the AirCapture jammer on. */
    uint8_t jammer_on_status = RESP_HW_TIMEOUT;
    if (true != air_capture_jammer_on()) {
    } else {
        jammer_on_status = RESP_SUCCESS;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(jammer_on_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_jammer_off(void *cmd_jammer_off) {
    /* Turn the AirCapture jammer off. */
    uint8_t jammer_off_status = RESP_HW_TIMEOUT;
    if (true != air_capture_jammer_off()) {
    } else {
        jammer_off_status = RESP_SUCCESS;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(jammer_off_status);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_enter_boot(void *cmd_enter_boot) {
    /* Respond to the CMD_ENTER_BOOT. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SUCCESS);
    
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
    
    /* Delay so that the response is sent before bootloader section is entered. */
    delay_us(50000);
    
    /* Set EEPROM magic and reset the device. */
    EEPUT(EE_BOOT_MAGIC_ADR, EE_BOOT_MAGIC_VALUE);
    reboot();
}


static void cmd_nlde_data_request(void *cmd_nlde_data_request) {
    /* Cast raw command to correct type. */
    cmd_nlde_data_request_t *cndr = (cmd_nlde_data_request_t *)cmd_nlde_data_request;
    
    /* First check if the frame length is valid. */
    if ((aMaxMACFrameSize - NWK_MIN_HEADER_OVERHEAD) < (cndr->length)) {
        /* Send response to the PC. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_SEMANTICAL_ERROR);
        Usb_send_in();
        
        /* Release the transaction buffer. */
        usb_ep_ack_transaction();
        
        return;
    }
    
    /* Try to allocate necessary memory to build the NLDE_DATA.request message. A
     * variable with file scope must be used, since the final freeing of memory
     * is done when the NLDE_DATA.confirm is received.
     */
    ndr = (nlde_data_req_t *)MEM_ALLOC(nlde_data_req_t);
    
    if (NULL == ndr) {
        /* Send response to the PC. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_OUT_OF_MEMORY);
        Usb_send_in();
        
        /* Release the transaction buffer. */
        usb_ep_ack_transaction();
        
        return;
    }
    
    /* Build NLDE_DATA.request message. */
    ndr->dst_addr_mode = 0;
    ndr->dst_addr = cndr->dest_addr;
    ndr->radius = cndr->radius;
    ndr->non_member_radius = cndr->non_member_radius;
    ndr->nsdu_length = cndr->length;
    ndr->nsdu = cndr->data;
    ndr->discovery_route = cndr->discovery_route;
    ndr->nlde_callback_data_confirm = cmd_nwk_data_confirm_callback;
    
    /* Try to execute the primitive. */
    if (true != zigbee_data_request(ndr)) {
        /* Send response to the PC. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
        Usb_send_in();
            
        /* Free any allocated memory. */
        MEM_FREE(ndr);
    }
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_nlme_formation_request(void *cmd_nlme_formation_request) {
    /* Try to allocate memory for the NLME_FORMATION.request. */
    nlme_formation_req_t *nfr = (nlme_formation_req_t *)MEM_ALLOC(nlme_formation_req_t);
    
    uint8_t formation_status = RESP_HW_TIMEOUT;
    
    /* Verify if memory was allocated. */
    if (NULL == nfr) {
        formation_status = RESP_OUT_OF_MEMORY;
    } else {
        /* Extract parameters and issue the NLME_FORMATION.request primitive. */
        cmd_nlme_formation_request_t *cnfr = (cmd_nlme_formation_request_t *)cmd_nlme_formation_request;
        
        nfr->channel = cnfr->channel;
        nfr->pan_id  = cnfr->pan_id;
        
        if (true != zigbee_formation_request(nfr)) {
            formation_status = RESP_SEMANTICAL_ERROR;
        } else {
            formation_status = RESP_SUCCESS;
        }
        
        MEM_FREE(nfr);
    }

    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(formation_status);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_nlme_permit_join_request(void *cmd_nlme_permit_join_request) {
    /* Extract parameters and issue the NLME_PERMIT_JOINING.request primitive. */
    cmd_nlme_permit_joining_request_t *cnpjr = (cmd_nlme_permit_joining_request_t *)cmd_nlme_permit_join_request;
    
    uint8_t permit_status = RESP_SEMANTICAL_ERROR;
    if (true != zigbee_permit_joining(cnpjr->join_permitted)) {
    } else {
        permit_status = RESP_SUCCESS;
    }
    
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(permit_status);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_nlme_join_request(void *cmd_nlme_join_request) {
    /* Cast raw command to correct type. */
    cmd_nlme_join_request_t *cnjr = (cmd_nlme_join_request_t *)cmd_nlme_join_request;
    
    /* Request some memory to build the NLME_JOIN.request. */
    njr = (nlme_join_req_t *)MEM_ALLOC(nlme_join_req_t);
    
    if (NULL == njr) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_OUT_OF_MEMORY);
        
        Usb_send_in();
    }
    
    /* Build the NLME_JOIN.request. */
    memcpy((void *)(&(njr->PANId)), (void *)(&(cnjr->PANId)), sizeof(uint16_t));
    njr->RejoinNetwork = cnjr->RejoinNetwork;
    njr->JoinAsRouter = cnjr->JoinAsRouter;
    njr->PowerSource = cnjr->PowerSource;
    njr->RxOnWhenIdle = cnjr->RxOnWhenIdle;
    njr->nlme_callback_join_confirm = cmd_nwk_join_confirm_callback;
    
    /* Issue primitive. */
    if (true != zigbee_join_request(njr)) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
        
        Usb_send_in();
        
        MEM_FREE(njr);
    }
    
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_nlme_leave_request(void *cmd_nlme_leave_request) {
    /* Cast raw command to correct type. */
    cmd_nlme_leave_request_t *cnlr = (cmd_nlme_leave_request_t *)cmd_nlme_leave_request;
    
    /* Request some memory to build the NLME_JOIN.request. */
    nlr = (nlme_leave_req_t *)MEM_ALLOC(nlme_join_req_t);
    
    if (NULL == nlr) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_OUT_OF_MEMORY);
        
        Usb_send_in();
    }
    
    /* Build the NLME_JOIN.request. */
    memcpy((void *)(&(nlr->DeviceAddress)), (void *)(&(cnlr->device_address)), sizeof(uint64_t));
    nlr->RemoveChildren = cnlr->remove_children;
    nlr->Rejoin = cnlr->rejoin;
    nlr->ReuseAddress = cnlr->reuse_address;
    nlr->Silent = cnlr->silent;
    nlr->nlme_callback_leave_confirm = cmd_nwk_leave_confirm_callback;
    
    /* Issue primitive. */
    if (true != zigbee_leave_request(nlr)) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
        
        Usb_send_in();
        
        MEM_FREE(nlr);
    }
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_nlme_discovery_request(void *cmd_nlme_discovery_request) {
    /* Cast raw command to correct type. */
    cmd_nlme_discovery_request_t *cndr = (cmd_nlme_discovery_request_t *)cmd_nlme_discovery_request;
    
    /* Request some memory to build the NLME_JOIN.request. */
    nndr = (nlme_network_discovery_req_t *)MEM_ALLOC(nlme_network_discovery_req_t);
    
    if (NULL == nndr) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_OUT_OF_MEMORY);
        
        Usb_send_in();
    }
    
    /* Build the NLME_NETWORK_DISCOVERY.request. */
    nndr->ChannelToScan = cndr->channel;
    nndr->ScanDuration = cndr->duration;
    nndr->nlme_callback_discovery_confirm = cmd_nwk_discovery_confirm_callback;
    
    /* Issue primitive. */
    if (true != zigbee_network_discovery_request(nndr)) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
        
        Usb_send_in();
        
        MEM_FREE(nndr);
    }
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_nlme_start_router_request(void *cmd_nlme_start_router_request) { 
    /* Issue primitive. */
    if (NWK_SUCCESS != zigbee_start_router_request()) {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
        
        Usb_send_in();
    } else {
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_SUCCESS);
        
        Usb_send_in();    
    }
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_dispatch(void *raw_cmd) {
    
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    UEINTX |= (1 << RXOUTI);
    
    /* First byte in all defined commands equals the command type. This byte is
     * used to execute the associated command handler.
     */
    uint8_t cmd = *(uint8_t *)raw_cmd;
    
    switch (cmd) {
      case CMD_SIGN_OFF:
        cmd_if_sign_off(raw_cmd);
        break;
      case CMD_SIGN_ON:
        cmd_if_sign_on(raw_cmd);
        break;
      case CMD_GET_PARAMETER:
        cmd_get_parameter(raw_cmd);
        break;
      case CMD_SET_PARAMETER:
        cmd_set_parameter(raw_cmd);
        break;
      case CMD_SELF_TEST:
        cmd_self_test(raw_cmd);
        break;
      case CMD_CHECK_STACK_USAGE:
        cmd_check_stack_usage(raw_cmd);
        break;
      case CMD_MEMORY_TEST:
        cmd_memory_test(raw_cmd);
        break;
      case CMD_SET_MODE:
        cmd_set_mode(raw_cmd);
        break;
      case CMD_SET_CHANNEL:
        cmd_set_channel(raw_cmd);
        break;
      case CMD_OPEN_STREAM:
        cmd_open_stream(raw_cmd);
        break;
      case CMD_CLOSE_STREAM:
        cmd_close_stream(raw_cmd);
        break;
      case CMD_CHANNEL_SCAN:
        cmd_channel_scan(raw_cmd);
        break;
      case CMD_CHANNEL_SCAN_STOP:
        cmd_channel_scan_stop(raw_cmd);
        break;
      case CMD_INJECT_FRAME:
        cmd_inject_frame(raw_cmd);
        break;
      case CMD_JAMMER_ON:
        cmd_jammer_on(raw_cmd);
        break;
      case CMD_JAMMER_OFF:
        cmd_jammer_off(raw_cmd);
        break;
      case CMD_ENTER_BOOT:
        cmd_enter_boot(raw_cmd);
        break;
      case CMD_NLDE_DATA_REQUEST:
        cmd_nlde_data_request(raw_cmd);
        break;
      case CMD_NLME_FORMATION_REQUEST:
        cmd_nlme_formation_request(raw_cmd);
        break;
      case CMD_NLME_PERMIT_JOIN_REQUEST:
        cmd_nlme_permit_join_request(raw_cmd);
        break;
      case CMD_NLME_JOIN_REQUEST:
        cmd_nlme_join_request(raw_cmd);
        break;
      case CMD_NLME_LEAVE_REQUEST:
        cmd_nlme_leave_request(raw_cmd);
        break;
      case CMD_NLME_DISCOVERY_REQUEST:
        cmd_nlme_discovery_request(raw_cmd);
        break;
      case CMD_NLME_START_ROUTER_REQUEST:
        cmd_nlme_start_router_request(raw_cmd);
        break; 
      default:
        /* Unsupported command. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_COMMAND_UNKNOWN);
        Usb_send_in();
        
        usb_ep_ack_transaction();
        break;
    }
}

/*================================= CALLBACKS ================================*/


                           /* USB callbacks */


static void usb_transaction_done(uint16_t length, uint8_t *data) {
    /* Check that the minimum command length of one byte is received. */
    if (0 == length) {
        /* Wrong length of command. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_SYNTACTICAL_ERROR);
        Usb_send_in();
        
        usb_ep_ack_transaction();
    } else if (false == vrt_post_event(cmd_if_dispatch, (void *)data)) {
        /* Command could not be posted into the event system, notify the user about this. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_VRT_KERNEL_ERROR);
        Usb_send_in();
        
        usb_ep_ack_transaction();
    }
}


                           /* NWK Layer callbacks */


static void cmd_nwk_data_confirm_callback(nlde_data_conf_t *ndc) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    if (NWK_SUCCESS != (ndc->Status)) {
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
    } else {
        Usb_write_byte(RESP_SUCCESS);
    }
    
    Usb_send_in();
            
    /* Free any allocated memory. */
    MEM_FREE(ndr);
}


static void cmd_nwk_data_indication(nlde_data_ind_t *ndi) {
    /* Check if there is room to add this NWK_EVENT. */
    if (0 == items_free) { nwk_events_missed++; return; }
    
    uint8_t *this_event = nwk_event_fifo[fifo_head];
    
    ENTER_CRITICAL_REGION();
    
    if ((NWK_EVENT_FIFO_SIZE - 1) == fifo_head) {
        fifo_head = 0;
    } else {
        fifo_head++;
    }
        
    items_used++;
    items_free--;
    
    LEAVE_CRITICAL_REGION();
    
    uint8_t index = 0;
    
    /* Start building the New Data event. */
    this_event[index++] = EVENT_NWK_DATA;
    this_event[index++] = 8 + (ndi->NsduLength);
    memcpy((void *)(&this_event[index]), (void *)(&(ndi->DstAddr)), sizeof(uint16_t));
    index += sizeof(uint16_t);
    memcpy((void *)(&this_event[index]), (void *)(&(ndi->SrcAddr)), sizeof(uint16_t));
    index += sizeof(uint16_t);
    this_event[index++] = ndi->LinkQuality;
    this_event[index++] = ndi->NsduLength;
    memcpy((void *)(&this_event[index]),(void *)(ndi->Nsdu), (ndi->NsduLength));
}


static void cmd_nwk_join_confirm_callback(nlme_join_conf_t *njc) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    if (NWK_SUCCESS != (njc->Status)) {
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
    } else {
        IEEE802_15_4_SET_RX_ON_WHEN_IDLE(true);
        (uint8_t)ieee802_15_4_rx_enable();
        Usb_write_byte(RESP_SUCCESS);
    }
    
    Usb_send_in();
            
    /* Free any allocated memory. Allocated in: cmd_if_process_nlme_join_request. */
    MEM_FREE(njr);
}


static void cmd_nwk_join_indication(nlme_join_ind_t *nji) {
    /* Check if there is room to add this NWK_EVENT. */
    if (0 == items_free) { nwk_events_missed++; return; }
    
    uint8_t *this_event = nwk_event_fifo[fifo_head];
    
    ENTER_CRITICAL_REGION();
    
    if ((NWK_EVENT_FIFO_SIZE - 1) == fifo_head) {
        fifo_head = 0;
    } else {
        fifo_head++;
    }
        
    items_used++;
    items_free--;
    
    LEAVE_CRITICAL_REGION();
    
    uint8_t index = 0;

    /* Start building the New Join event. */
    this_event[index++] = EVENT_NWK_JOIN;
    this_event[index++] = 12;
    memcpy((void *)(&this_event[index]), (void *)(&(nji->ShortAddress)), sizeof(uint16_t));
    index += sizeof(uint16_t);
    memcpy((void *)(&this_event[index]), (void *)(&(nji->ExtendedAddress)), sizeof(uint64_t));
}


static void cmd_nwk_leave_confirm_callback(nlme_leave_conf_t *nlc) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    if (NWK_SUCCESS != (nlc->Status)) {
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
    } else {
        Usb_write_byte(RESP_SUCCESS);
    }
    
    Usb_send_in();
            
    /* Free any allocated memory. Allocated in: cmd_if_process_nlme_leave_request. */
    MEM_FREE(nlr);
}


static void cmd_nwk_leave_indication(nlme_leave_ind_t *nli) {
    /* Check if there is room to add this NWK_EVENT. */
    if (0 == items_free) { nwk_events_missed++; return; }
    
    uint8_t *this_event = nwk_event_fifo[fifo_head];
    
    ENTER_CRITICAL_REGION();
    
    if ((NWK_EVENT_FIFO_SIZE - 1) == fifo_head) {
        fifo_head = 0;
    } else {
        fifo_head++;
    }
        
    items_used++;
    items_free--;
    
    LEAVE_CRITICAL_REGION();
    
    uint8_t index = 0;
    
    /* Start building the New Leave event. */
    this_event[index++] = EVENT_NWK_LEAVE;
    this_event[index++] = 10;
    memcpy((void *)(&this_event[index]), (void *)(&(nli->DeviceAddress)), sizeof(uint64_t));
    index += sizeof(uint64_t);
}


static void cmd_nwk_discovery_confirm_callback(nlme_network_discovery_conf_t *ndc) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    if (NWK_SUCCESS != (ndc->Status)) {
        Usb_write_byte(RESP_PRITMITIVE_FAILED);
    } else {
        Usb_write_byte(RESP_SUCCESS);
    }
    
    Usb_send_in();
            
    /* Free any allocated memory. Allocated in: cmd_if_process_nlme_discovery_request. */
    MEM_FREE(nndr);
}
//! @}
/*EOF*/
