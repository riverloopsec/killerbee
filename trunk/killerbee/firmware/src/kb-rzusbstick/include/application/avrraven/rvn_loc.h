// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Commands, responses and events for local AVR Raven transactions
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
 * $Id: rvn_loc.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __RVN_LOC_H__
#define __RVN_LOC_H__

/*========================= INCLUDES                 =========================*/
/*#include "rvn_ota.h"*/

/*========================= CONSTANT DEFINES         =========================*/
#define RVN_FW_PACKET_SIZE 32

/*========================= MACROS DEFINES           =========================*/
#define send_loc_evt(evt)           send_loc_simple_rsp(evt)
#define send_loc_simple_rsp(rsp) do {\
    rvn_loc_rsp_std_t response = {.id = rsp};\
    sipc_send_frame(sizeof(rvn_loc_rsp_std_t), (uint8_t *)&response);\
} while(0)

/*========================= TYPEDEFS                 =========================*/
/*! \brief Definition of supported set of parameters that can be retrived from the
 *         node.
 */
typedef enum {
    NWK_ADDRESS = 0,
    IEEEE_ADDRESS = 1,
    FW_REV = 2,
} rvn_param_t;

/// Command ID's
typedef enum {
    RVN_LOC_CMD_NO_COMMAND              = 0x00,     //< Important so enums don't over lap with LOC's
    RVN_LOC_CMD_SLEEP                   = 0x01,
    RVN_LOC_CMD_RESTART                 = 0x02,

    RVN_LOC_CMD_ENTER_BOOT              = 0x03,
    RVN_LOC_CMD_APP_START               = 0x04,
    RVN_LOC_CMD_FLASH_PAGE_SIZE_GET     = 0x05,
    RVN_LOC_CMD_FW_PACKET               = 0x06,

    RVN_LOC_CMD_TERMINAL_ENABLE         = 0x07,
    RVN_LOC_CMD_TERMINAL_DISABLE        = 0x08,

    RVN_LOC_CMD_RADIO_CONNECT           = 0x09,
    RVN_LOC_CMD_RADIO_DISCONNECT        = 0x0A,
    
    RVN_LOC_CMD_RX_ON                   = 0x0B,
    RVN_LOC_CMD_RX_OFF                  = 0x0C,
    
    RVN_LOC_CMD_PACKET                  = 0x0D,
    RVN_LOC_CMD_START_ROUTER            = 0x0E, //!< This command is used to start the router.
    
    RVN_LOC_CMD_GET_PARAM               = 0x0F, //!< This command is used to retrive the node's parameters.
    
    RVN_LOC_CMD_GET_REV                 = 0x10,

    RVN_LOC_CMD_COUNT                   = 0x11,                          //<  Important so enums don't overlap
} rvn_loc_cmd_t;

/// Response ID's
typedef enum {
    RVN_LOC_RSP_OK                      = RVN_LOC_CMD_COUNT,         //< Important so enums don't overlap
    RVN_LOC_RSP_ERROR                   = 0x12,
    RVN_LOC_RSP_NOT_SUPPORTED           = 0x13,
    RVN_LOC_RSP_BUSY                    = 0x14,
    RVN_LOC_RSP_GET_PARAM               = 0x15, //!< Response used to return NWK parameters such as address.

    RVN_LOC_CMD_RSP_COUNT               = 0x16,                      //<  Important so enums don't overlap
} rvn_loc_rsp_t;

/// Event ID's
typedef enum {
    RVN_LOC_EVT_OTA_PACKET              = RVN_LOC_CMD_RSP_COUNT,     //< Important so enums don't over lap
    RVN_LOC_EVT_BL_ENTERED              = 0x17,
    RVN_LOC_EVT_APP_STARTED             = 0x18,

    RVN_LOC_EVT_PRODTEST                = 0x19,

    RVN_LOC_CMD_RSP_EVT_COUNT           = 0x1A,                  //< Important for correct operation of sipc
} rvn_loc_evt_t;

/// Command types
typedef struct {
    uint8_t     id;
} rvn_loc_cmd_std_t;

typedef struct {
    uint8_t     id;
    uint32_t    adr;
    uint16_t    size;
    uint16_t    data[];
} rvn_loc_cmd_fw_packet_t;

typedef struct {
    uint8_t     id;
    uint8_t     mode;
} rvn_loc_cmd_sleep_t;

typedef struct {
//SEQ_NMBR    uint8_t     seq_nmbr;
    uint8_t     id;
    uint16_t    adr;
    uint8_t     size;
    uint8_t     data[];
} rvn_loc_cmd_ota_packet_t;


typedef struct {
    uint8_t     id;
    uint16_t    test_id;
    uint8_t     test_stage;
} rvn_loc_cmd_prodtest_t;

typedef struct RVN_LOC_CMD_RADIO_CONNECT_STRUCT_TAG {
    uint8_t  id;
    uint8_t  channel;
    uint16_t pan_id;
} rvn_loc_cmd_radio_connect_t;

typedef struct RVN_LOC_CMD_RX_ON_STRUCT_TAG {
    uint8_t id;
} rvn_loc_cmd_rx_on_t;

typedef struct RVN_LOC_CMD_RX_OFF_STRUCT_TAG {
    uint8_t id;
} rvn_loc_cmd_rx_off_t;

typedef struct RVN_LOC_CMD_START_ROUTER_STRUCT_TAG {
    uint8_t id;
} rvn_loc_cmd_start_router_t;

typedef struct RVN_LOC_CMD_GET_NWK_ADDRESS_STRUCT_TAG {
    uint8_t id;
    rvn_param_t param;
} rvn_loc_cmd_get_param_t;


/// Response types
typedef struct {
    uint8_t id;
} rvn_loc_rsp_std_t;

typedef struct RVN_LOC_RSP_GET_PARAM_STRUCT_TAG {
    uint8_t id;
    uint8_t size;
    uint8_t data[];
} rvn_loc_rsp_get_param_t;

typedef struct {
    uint8_t     id;
    uint16_t    test_id;
    uint8_t     test_stage;
    uint8_t     data[5];
} rvn_loc_rsp_prodtest_t;

/// Event types
typedef struct {
    uint8_t id;
} rvn_loc_evt_std_t;

typedef struct {
    uint8_t     id;
    uint8_t     lqi;
    uint16_t    adr;
    uint8_t     size;
    uint8_t     data[];
} rvn_loc_evt_ota_packet_t;


/// Transport layer header
typedef struct {
    uint8_t seq_nmbr;
    uint8_t data[];
} rvn_ota_transport_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/


#endif // __RVN_LOC_H__

/*EOF*/
