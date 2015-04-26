// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  ......
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
 * $Id: cmd_types.h 41180 2008-04-30 08:19:04Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef CMD_TYPES_H
#define CMD_TYPES_H
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
typedef enum CMD_IF_TYPES_ENUM_TAG {
    CMD_MAC_INIT = 0x40, //!< Send command to the ATmega1284p to init the IEEE 802.15.4 MAC.
    CMD_MAC_START,
    CMD_MAC_ASSOCIATE,
    CMD_MAC_ASSOCIATE_INDICATION,
    CMD_MAC_SEND_DATA,
    CMD_MAC_DATA_RECEIVED,
} cmd_if_cmd_t;

typedef enum CMD_IF_RESPONSE_ENUM_TAG {
    RESP_SUCCESS = 0x00,
    RESP_MAC_INIT_FAILED,
    RESP_MAC_START_FAILED,
    RESP_MAC_ASSOCIATE_FAILED,
    RESP_MAC_SEND_FAILED,
} cmd_if_resp_t;



typedef struct CMD_IF_MAC_INIT_STRUCT_TAG {
    uint8_t type;
} cmd_if_mac_init_t;

typedef struct CMD_IF_MAC_START_STRUCT_TAG {
    uint8_t type;
    uint8_t channel;
    uint16_t pan_id;
    bool start_as_coordinator;
} cmd_if_mac_start_t;

typedef struct CMD_IF_MAC_ASSOCIATE_STRUCT_TAG {
    uint8_t type;
    uint8_t channel;
    uint16_t pan_id;
} cmd_if_mac_associate_t;

typedef struct CMD_IF_MAC_ASSOCIATE_INDICATION_STRUCT_TAG {
    uint8_t type;
    uint64_t device_address;
} cmd_if_mac_associate_indication_t;

typedef struct CMD_IF_MAC_SEND_STRUCT_TAG {
    uint8_t type;
    uint16_t dst_address;
    uint8_t length;
    uint8_t data[];
} cmd_if_mac_send_t;

typedef struct CMD_IF_MAC_DATA_RECEIVED_STRUCT_TAG {
    uint8_t type;
    uint16_t src_address;
    uint8_t length;
    uint8_t data[];
} cmd_if_mac_data_received_t;

/* RESPONSES. */
typedef struct CMD_IF_RESP_SUCCESS_STRUCT_TAG {
    uint8_t type;
} cmd_if_resp_success_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#endif
/*EOF*/
