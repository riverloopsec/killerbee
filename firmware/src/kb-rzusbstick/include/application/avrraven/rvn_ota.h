// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Commands, responses and events for all Over The Air transactions to/from the AVR Raven
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
 * $Id: rvn_ota.h 41666 2008-05-14 22:55:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __RVN_OTA_H__
#define __RVN_OTA_H__

/*========================= INCLUDES                 =========================*/

/*========================= MACROS                   =========================*/
/*RVN_OTA_CMD_FOPEN*/
#define RVN_OTA_CMD_FOPEN_MODE_RD       0x01
#define RVN_OTA_CMD_FOPEN_MODE_WR       0x02
#define RVN_OTA_CMD_FOPEN_MODE_RDWR     0x03
#define RVN_OTA_CMD_FOPEN_MODE_APPEND   0x04
#define RVN_OTA_CMD_FOPEN_MODE_TRUNC    0x08
#define RVN_OTA_CMD_FOPEN_MODE_CREATE   0x10
#define RVN_OTA_CMD_FOPEN_MODE_BINARY   0x20

/*RVN_OTA_CMD_FREAD*/
#define RVN_OTA_CMD_FREAD_MAX_SIZE      64

/*RVN_OTA_CMD_FWRITE*/
#define RVN_OTA_CMD_FWRITE_MAX_SIZE     64

/*========================= TYPEDEFS                 =========================*/

/// Command ID's
typedef enum {
    RVN_OTA_CMD_NO_COMMAND          = 0x00,

    // Firmware related functions:
    RVN_OTA_CMD_FW_REV_GET          = 0x01,     // Obsolete
    RVN_OTA_CMD_FW_WRITE_PACKET     = 0x02,
    RVN_OTA_CMD_FW_WRITE_INIT       = 0x03,
    RVN_OTA_CMD_FW_WRITE_COMPLETED  = 0x04,
    RVN_OTA_CMD_FW_WRITE_IMAGE      = 0x05,

    // Message functions:
    RVN_OTA_CMD_TXTMSG              = 0x06,
    RVN_OTA_CMD_AUDMSG              = 0x07,

    // Node control/configuration:
    RVN_OTA_CMD_STORAGE_GET         = 0x08,
    RVN_OTA_CMD_RESTART             = 0x09,
    RVN_OTA_CMD_CLOCK_SET           = 0x0A,

    // Shared resource operations:
    RVN_OTA_CMD_SHARELIST_GET       = 0x0B,
    RVN_OTA_CMD_SHARE               = 0x0C,
    RVN_OTA_CMD_SHARED_GET          = 0x0D,
    RVN_OTA_CMD_SHARED_FREE         = 0x0E,
    RVN_OTA_CMD_SHARED_RD           = 0x0F,
    RVN_OTA_CMD_SHARED_WR           = 0x10,

    // File operations:
    RVN_OTA_CMD_FREAD               = 0x11,
    RVN_OTA_CMD_FWRITE              = 0x12,
    RVN_OTA_CMD_FOPEN               = 0x13,
    RVN_OTA_CMD_FCLOSE              = 0x14,
    RVN_OTA_CMD_FCREATE             = 0x15,
    RVN_OTA_CMD_FREMOVE             = 0x16,

    // Misc:
    RVN_OTA_CMD_CRC32               = 0x17,
    RVN_OTA_CMD_CHAT                = 0x18,
    RVN_OTA_CMD_GETNAME             = 0x19,

    RVN_OTA_CMD_COUNT               = 0x1A,                          // Important so enums don't overlap
    
    // Sign on
    RVN_OTA_CMD_SIGN_ON             = 0xF0, // Do not change!
    
    // Voting
    RVN_OTA_CMD_VOTE_START          = 0x2B,
    RVN_OTA_CMD_VOTE_GET            = 0x2C,
    RVN_OTA_CMD_VOTE_STOP           = 0x2E,
        
} rvn_ota_cmd_t;

/// Response ID's
typedef enum {
    // Generic:
    RVN_OTA_RSP_OK                  = RVN_OTA_CMD_COUNT,         // Important so enums don't overlap
    RVN_OTA_RSP_ERROR               = 0x1B,
    RVN_OTA_RSP_NOT_SUPPORTED       = 0x1C,
    RVN_OTA_RSP_BUSY                = 0x1D,

    // Special:
    RVN_OTA_RSP_DATA                = 0x1E,
    RVN_OTA_RSP_INT8                = 0x1F,
    RVN_OTA_RSP_INT16               = 0x20,
    RVN_OTA_RSP_INT32               = 0x21,
    RVN_OTA_RSP_UINT8               = 0x22,
    RVN_OTA_RSP_UINT16              = 0x23,
    RVN_OTA_RSP_UINT32              = 0x24,
    RVN_OTA_RSP_STRING              = 0x25,
    RVN_OTA_RSP_FILE_INFO           = 0x26,
    RVN_OTA_RSP_FREAD               = 0x27,     // Obsolete

    RVN_OTA_CMD_RSP_COUNT           = 0x28,                      // Important so enums don't overlap
    
    // Vote
    RVN_OTA_RSP_VOTE                = 0x2D,
    
    // Sign on
    RVN_OTA_RSP_SIGN_ON             = 0xF1, // Do not change!
    
} rvn_ota_rsp_t;

/// Event ID's
typedef enum {
    RVN_OTA_EVT_FW_WRITE_INITATED   = RVN_OTA_CMD_RSP_COUNT,  // Important so enums don't overlap
    RVN_OTA_EVT_FW_WRITE_DONE       = 0x29,

    RVN_OTA_CMD_RSP_EVT_COUNT       = 0x2A                 // Important for correct operation of sipc
} rvn_ota_evt_t;

/// Command types
typedef struct {
    uint8_t     id;
    uint8_t     type;
    uint32_t    adr;
    uint8_t     size;
    uint8_t     data[];
} rvn_ota_cmd_fw_packet_t;

typedef struct {
    uint8_t     id;
    uint8_t     open_mode;
    int8_t      file_name_size;
    uint8_t     file_name[];
} rvn_ota_cmd_fopen_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
} rvn_ota_cmd_fclose_t;

typedef struct {     // Obsolete
    uint8_t     id;
    uint8_t     handle;
    uint8_t     count;
} rvn_ota_cmd_fread_t;

typedef struct {
    uint8_t     id;
    uint8_t     attrib;
    int8_t      file_name_size;
    uint8_t     file_name[];
} rvn_ota_cmd_fcreate_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
} rvn_ota_cmd_fremove_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
    uint8_t     size;
    uint8_t     data[];
} rvn_ota_cmd_fwrite_t;

typedef struct {
    uint8_t     id;
    uint8_t     size;
    uint8_t     data[];
} rvn_ota_cmd_str_t;

typedef struct {
    uint8_t     id;
} rvn_ota_cmd_std_t;

typedef struct {
    uint8_t     id;
    uint8_t     hour;
    uint8_t     min;
    uint8_t     sec;
} rvn_ota_cmd_clk_set_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
} rvn_ota_cmd_shared_rd_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
    uint32_t    value;
} rvn_ota_cmd_shared_wr_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
} rvn_ota_cmd_share_t;

typedef struct {
    uint8_t     id;
    uint8_t     handle;
} rvn_ota_cmd_shared_get_t;

typedef struct {
    uint8_t     id;
    uint8_t     mode;
} rvn_ota_cmd_sleep_t;

typedef struct {
    uint8_t     id;
    uint8_t     type;
    uint8_t     mere;
} rvn_ota_cmd_crc32_t;

typedef struct {
    uint8_t     id;
    int16_t     min_value;
    int16_t     max_value;
    int16_t     start_value;
    int8_t      text_size;
    uint8_t     text[];
} rvn_ota_cmd_vote_start_t;

typedef struct {
    uint8_t     id;
    int8_t      text_size;
    uint8_t     text[];
} rvn_ota_cmd_vote_get_t;

typedef struct {
    uint8_t     id;
    int8_t      text_size;
    uint8_t     text[];
} rvn_ota_cmd_vote_stop_t;

/// Response types
typedef struct {
    uint8_t id;
} rvn_ota_rsp_std_t;

typedef struct {
    uint8_t id;
    uint8_t size;
    uint8_t data[];
} rvn_ota_rsp_data_t;

typedef struct {
    uint8_t     id;
    uint8_t     size;
    uint8_t     data[];
} rvn_ota_rsp_str_t;

typedef struct {
    uint8_t id;
    uint8_t value;
} rvn_ota_rsp_uint8_t;

typedef struct {
    uint8_t id;
    uint16_t value;
} rvn_ota_rsp_uint16_t;

typedef struct {
    uint8_t id;
    uint32_t value;
} rvn_ota_rsp_uint32_t;

typedef struct {
    uint8_t id;
    int8_t value;
} rvn_ota_rsp_int8_t;

typedef struct {
    uint8_t id;
    int16_t value;
} rvn_ota_rsp_int16_t;

typedef struct {
    uint8_t id;
    int32_t value;
} rvn_ota_rsp_int32_t;

typedef struct {
    uint8_t id;
    uint8_t type;
    int8_t rev_size;
    uint16_t rev[];
} rvn_ota_rsp_sign_on_t;

typedef struct {
    uint8_t id;
    uint8_t valid;
    int16_t value;
} rvn_ota_rsp_vote_t;

/// Event types
typedef struct {
    uint8_t id;
} rvn_ota_evt_std_t;

/// Other types
typedef enum {
    RVN_MCU_M3290P_USR  = 0,
    RVN_MCU_M1284P_USR  = 1,
    RVN_MCU_M3290P_FD   = 2,
    RVN_MCU_M1284P_FD   = 3,
} rvn_ota_cmd_fw_write_image_type_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/


#endif // __RVN_OTA_H__

