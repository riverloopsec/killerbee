// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Implementation of all Over The Air communcicaion
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
 * $Id: ota.c 41666 2008-05-14 22:55:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "ota.h"
#include "avrraven_3290p.h"

#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"
#include "vrt_kernel.h"
#include "vrt_mem.h"

#include "sipc.h"
#include "macros.h"
#include "mmap.h"
#include "mbox.h"

#include "txtedit.h"
#include "numedit.h"

#include "battery.h"
#include "audio.h"

#include "lcd.h"
#include "led.h"

//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Pointer to current file stream may be opened over the air (only one at the time)
static sfs_fstream_t* ota_fstream;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
void ota_cmd_fw_write_initiated(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    // Check state before taking action
    switch (avrraven.status.state) {
    case STATE_IDLE:
    case STATE_DISPLAY_MESSAGE:
        // Do not start FW upgrade if battery voltage is less than 2.6 V.
        if (battery_voltage_read() < 2600) {
            ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
            avrraven.status.state = STATE_IDLE;
        } else {
            if (avrraven.user_config.fw_rcv_auto == true) {
                avrraven.status.state = STATE_FW_WRITE_INIT;
            } else {
                lcd_puts_P("ENTER TO RECEIVE FW IMAGE");
                avrraven.status.state = STATE_FW_WAIT_USER_CONFIRMATION;
            }
            ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
        }
        break;
    default:
        // Respond busy if in unknown state
        ota_simple_response(RVN_OTA_RSP_BUSY, adr, seq_nmbr);
        break;
    }
}

void ota_cmd_fw_write_image(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    // Do not start FW image writing if battery voltage is less than 2.6 V.
    if (battery_voltage_read() < 2600) {
        // Send application response
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        avrraven.status.state = STATE_IDLE;
    } else {
        // Send application response
        ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
        
        // Set FW image offset to read from user location on data flash
        avrraven.status.m1284p_img_offset = M1284P_FLASH_USR_IMG_ADR;
        avrraven.status.state = STATE_FW_WRITE_IMAGE;
    }
}

void ota_cmd_fw_write_packet(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    rvn_ota_cmd_fw_packet_t* fw_packet = (rvn_ota_cmd_fw_packet_t*)&ota_transport->data;
    df_adr_t adr_offset = fw_packet->adr;
    df_size_t size = fw_packet->size;
    uint8_t type = fw_packet->type;
    
    
    // Allcoate memory for FW data
    uint8_t* data = vrt_mem_alloc((uint16_t)size);
    
    // If out of mem, ack data, respond error to sender and silently return
    if (data == NULL) {
        SIPC_ACK_PACKET();
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    // Else, copy and ack data 
    } else {
        memcpy((void*)data, (const void*)fw_packet->data, (size_t)size);
        SIPC_ACK_PACKET();
    }
    
    
    // Locals
    static int current_mcu = -1;
    df_adr_t base_adr;
    
    switch (avrraven.status.state) {
    case STATE_FW_WRITE:
    case STATE_DISPLAY_MESSAGE:
        // Find correct FW image base address
        if (type == RVN_MCU_M3290P_USR) {
            base_adr = M3290P_FLASH_USR_IMG_ADR;
            if (current_mcu != type) {
                lcd_puts_P("M3290");
            }
        } else if (type == RVN_MCU_M1284P_USR) {
            base_adr = M1284P_FLASH_USR_IMG_ADR;
            if (current_mcu != type) {
                lcd_puts_P("M1284");
            }
        } else if (type == RVN_MCU_M3290P_FD) {
            base_adr = M3290P_FLASH_FD_IMG_ADR;
            if (current_mcu != type) {
                lcd_puts_P("M3290FD");
            }
        } else if (type == RVN_MCU_M1284P_FD) {
            base_adr = M1284P_FLASH_FD_IMG_ADR;
            if (current_mcu != type) {
                lcd_puts_P("M1284FD");
            }
        } else {
        	// Send response
        	ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
			break;
		}
        lcd_num_putdec((int)(adr_offset/1024), LCD_NUM_PADDING_SPACE);
        current_mcu = type;
        
        // Write received FW block to data flash
        df_write(base_adr + adr_offset, size, data);
        
        // Send response
        ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
        
        //avrraven.status.state = STATE_FW_WRITE_WAIT_TX_DONE;
        break;
    case STATE_FW_WRITE_WAIT_TX_DONE:
    case STATE_FW_WRITE_WAIT_RX_ENABLE_RESPONSE:
    case STATE_FW_WAIT_USER_CONFIRMATION:
        // Send response
        ota_simple_response(RVN_OTA_RSP_BUSY, adr, seq_nmbr);
        break;
    default:
        // Send response
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        break;
    }
    
    // Free memory block
    vrt_mem_free((void*)data);
}

void ota_cmd_fw_write_completed(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    switch (avrraven.status.state) {
    case STATE_FW_WRITE:
        // Send response
        ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
        avrraven.status.state = STATE_FW_WRITE_COMPLETE_WAIT_TX_DONE;
        break;
    default:
        // Send response
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        break;
    }
}

void ota_cmd_txtmsg(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    rvn_ota_cmd_str_t* str = (rvn_ota_cmd_str_t*)&ota_transport->data;
    uint8_t size = str->size;
    
    // Allcoate memory for text msg data
    uint8_t* data = vrt_mem_alloc((uint16_t)size);
    
    // If out of mem, ack data, respond error to sender and silently return
    if (data == NULL) {
        SIPC_ACK_PACKET();
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    // Else, copy and ack data and respond OK to sender 
    } else {
        memcpy((void*)data, (const void*)str->data, (size_t)size);
        SIPC_ACK_PACKET();
        ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
    }
    
    // Open mailbox and store incomming mail
    if (mbox_open() != EOF) {
        if (mbox_mail_put(adr, data, size) != EOF) {
            lcd_symbol_clr(LCD_SYMBOL_ENV_OP);
            lcd_symbol_set(LCD_SYMBOL_ENV_MAIN);
            lcd_symbol_set(LCD_SYMBOL_ENV_CL);
        } else {
            lcd_symbol_clr(LCD_SYMBOL_ENV_OP);
            lcd_symbol_clr(LCD_SYMBOL_ENV_MAIN);
            lcd_symbol_clr(LCD_SYMBOL_ENV_CL);
        }
        mbox_close();
    } else {
        lcd_symbol_clr(LCD_SYMBOL_ENV_OP);
        lcd_symbol_clr(LCD_SYMBOL_ENV_MAIN);
        lcd_symbol_clr(LCD_SYMBOL_ENV_CL);
    }
    
    // Play sound
    unsigned char file_name[MBOX_NEW_MAIL_SOUND_FILE_NAME_BUFFER_SIZE];
    strncpy_P((char*)file_name, MBOX_NEW_MAIL_SOUND_FILE_NAME, MBOX_NEW_MAIL_SOUND_FILE_NAME_BUFFER_SIZE);
            
    audio_playback_start(file_name, false);
        
    // Show last retrieved message in display if user configuration says so, and system is idle
    switch (avrraven.status.state) {
    case STATE_IDLE:
        if (avrraven.user_config.txtmsg_preview == true) {
            lcd_puta(data, (int)size);
            led_status_set(LED_SOFT_BLINK);
            avrraven.status.state = STATE_MAIL_PREVIEW;
        }
        break;
    default:
        // In all other states, the message will no be showed directly
        break;
    }
    vrt_mem_free((void*)data);
}

void ota_cmd_sign_on(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    rvn_ota_rsp_sign_on_t* ota_rsp_sign_on;
    void* memory_block;
    
    uint16_t m3290p_fd_fw_rev;
    uint16_t m1284p_fd_fw_rev;
    uint16_t m3290p_bl_fd_fw_rev;
    uint16_t m1284p_bl_fd_fw_rev;
    
    // Resond ERROR if low batt
    if (avrraven.status.batt_low == true) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Allocate memory for application packet
    if ((memory_block = vrt_mem_alloc(sizeof(rvn_loc_cmd_ota_packet_t) + sizeof(rvn_ota_transport_t) + sizeof(rvn_ota_rsp_data_t) + sizeof(int16_t))) == NULL) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    ota_rsp_sign_on = memory_block;
    
    // Read factory default FW revisions from image in data flash
    df_read(M3290P_FLASH_FD_IMG_ADR + FW_REV_ADR_M3290P_APP, sizeof(uint16_t), (uint8_t*)&m3290p_fd_fw_rev);
    df_read(M1284P_FLASH_FD_IMG_ADR + FW_REV_ADR_M1284P_APP, sizeof(uint16_t), (uint8_t*)&m1284p_fd_fw_rev);
    df_read(M3290P_FLASH_FD_IMG_ADR + FW_REV_ADR_M3290P_BL, sizeof(uint16_t), (uint8_t*)&m3290p_bl_fd_fw_rev);
    df_read(M1284P_FLASH_FD_IMG_ADR + FW_REV_ADR_M1284P_BL, sizeof(uint16_t), (uint8_t*)&m1284p_bl_fd_fw_rev);
        
    // Build response
    ota_rsp_sign_on->id = RVN_OTA_RSP_SIGN_ON;
    ota_rsp_sign_on->type = AVRRAVEN_TYPE;
    ota_rsp_sign_on->rev_size = AVRRAVEN_REV_SIZE;
    ota_rsp_sign_on->rev[0] = AVRRAVEN_HW_REV;      // HW
    ota_rsp_sign_on->rev[1] = avrraven.status.m3290p_app_rev;   // 3290P
    ota_rsp_sign_on->rev[2] = avrraven.status.m1284p_app_rev;    // 1284P
    ota_rsp_sign_on->rev[3] = avrraven.status.m3290p_bl_rev;     // 3290P_BL
    ota_rsp_sign_on->rev[4] = avrraven.status.m1284p_bl_rev;      // 1284P_BL
    ota_rsp_sign_on->rev[5] = m3290p_fd_fw_rev;     // 3290P_FD
    ota_rsp_sign_on->rev[6] = m1284p_fd_fw_rev;     // 1284P_FD
    ota_rsp_sign_on->rev[7] = m3290p_bl_fd_fw_rev;  // 3290P_FD_BL
    ota_rsp_sign_on->rev[8] = m1284p_bl_fd_fw_rev;  // 1284P_FD_BL
    
    // Send frame
    if (ota_transaction((void*)ota_rsp_sign_on, adr, seq_nmbr, sizeof(rvn_ota_rsp_sign_on_t) + sizeof(int16_t)*AVRRAVEN_REV_SIZE) == EOF) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
    }
    
    // Free memory block
    vrt_mem_free((void*)memory_block);
}


void ota_cmd_fopen(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    rvn_ota_cmd_fopen_t* cmd_fopen = (rvn_ota_cmd_fopen_t*)&ota_transport->data;
    uint8_t open_mode = cmd_fopen->open_mode;
    int8_t file_name_size = cmd_fopen->file_name_size;
    uint8_t file_name[SFS_MAX_FILENAME_BUFFER_SIZE];
    memcpy((void*)file_name, (const void*)cmd_fopen->file_name, (size_t)file_name_size);

    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    rvn_ota_rsp_uint8_t* ota_rsp_fopen;
    
    // Resond ERROR if low batt
    if (avrraven.status.batt_low == true) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Try to open file. Respond ERROR and on fail
    if((ota_fstream = sfs_fopen(file_name, open_mode)) == NULL) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Allocate memory for application packet. Respond ERROR and close file if out of mem
    if ((ota_rsp_fopen = vrt_mem_alloc(sizeof(rvn_loc_cmd_ota_packet_t) + sizeof(rvn_ota_transport_t) + sizeof(rvn_ota_rsp_uint8_t))) == NULL) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        sfs_fclose(ota_fstream);
        return;
    }
    
    // Build response 
    ota_rsp_fopen->id = RVN_OTA_RSP_UINT8;
    ota_rsp_fopen->value = ota_fstream->handle;
    
    // Send frame
    if ((ota_transaction((void*)ota_rsp_fopen, adr, seq_nmbr, sizeof(rvn_ota_rsp_uint8_t))) == EOF) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
    }

    // Free memory block
    vrt_mem_free((void*)ota_rsp_fopen);
}

void ota_cmd_fclose(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    rvn_ota_cmd_fclose_t* cmd_fclose = (rvn_ota_cmd_fclose_t*)&ota_transport->data;
    uint8_t handle = cmd_fclose->handle;
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    // Resond ERROR if low batt
    if (avrraven.status.batt_low == true) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Respond ERROR if wrong handle is used. 
    if (ota_fstream->handle != handle) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Send OK response to sender
    ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
    
    // Close file
    sfs_fclose(ota_fstream);
}

void ota_cmd_fread(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    rvn_ota_cmd_fread_t* cmd_fread = (rvn_ota_cmd_fread_t*)&ota_transport->data; 
    uint8_t handle = cmd_fread->handle;
    uint8_t count = cmd_fread->count;
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    rvn_ota_rsp_data_t* ota_rsp_fread;
    sfs_size_t bytes_read;
    
    // Respond ERROR if low batt
    if (avrraven.status.batt_low == true) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Respond ERROR if asking for too many bytes
    if (count > RVN_OTA_CMD_FREAD_MAX_SIZE) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Respond ERROR if wrong handle is used
    if (ota_fstream->handle != handle) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Allocate memory for application packet. Respond ERROR if out of mem
    if ((ota_rsp_fread = vrt_mem_alloc(sizeof(rvn_loc_cmd_ota_packet_t) + sizeof(rvn_ota_transport_t) + sizeof(rvn_ota_rsp_data_t) + count)) == NULL) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Read data from file and store number of bytes read. If "bytes_read" is ls than "size" indicates 
    // EOF to the requester
    sfs_size_t read_count = count;
    bytes_read = sfs_fread((void*)ota_rsp_fread->data, read_count, ota_fstream);
    
    // Build response
    ota_rsp_fread->id = RVN_OTA_RSP_DATA;
    ota_rsp_fread->size = bytes_read;
    
    // ACK sipc packet
    SIPC_ACK_PACKET();
        
    // Send frame
    int ota_packet_size = sizeof(rvn_ota_rsp_data_t) + bytes_read;
    if (ota_transaction((void*)ota_rsp_fread, adr, seq_nmbr, ota_packet_size) == EOF) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Free memory block
    vrt_mem_free((void*)ota_rsp_fread);
}

void ota_cmd_fwrite(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    rvn_ota_cmd_fwrite_t* cmd_fwrite = (rvn_ota_cmd_fwrite_t*)&ota_transport->data;
    uint8_t handle = cmd_fwrite->handle;
    uint8_t size = cmd_fwrite->size;
    
    // Allcoate memory for FW data
    uint8_t* data = vrt_mem_alloc((uint16_t)size);
    
    // If out of mem, ack data, respond error to sender and silently return
    if (data == NULL) {
        SIPC_ACK_PACKET();
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    // Else, copy and ack data 
    } else {
        memcpy((void*)data, (const void*)cmd_fwrite->data, (size_t)size);
        SIPC_ACK_PACKET();
    }
    
    
    
    // Resond ERROR if low batt
    if (avrraven.status.batt_low == true) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Resond ERROR if too command tries to write to many bytes
    if (size > RVN_OTA_CMD_FWRITE_MAX_SIZE) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }
    
    // Respond ERROR if wrong handle is used
    if (ota_fstream->handle != handle) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
        return;
    }

    // Write data to file. Respond ERROR on fail
    sfs_size_t write_count = size;
    if (sfs_fwrite((void*)data, write_count, ota_fstream) != size) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
    } else {
        ota_simple_response(RVN_OTA_RSP_OK, adr, seq_nmbr);
    }
    
    // Free memory block
    vrt_mem_free((void*)data);
}

void ota_cmd_shared_rd(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
//  uint8_t handle = ((rvn_ota_cmd_shared_rd_t*)&ota_transport->data)->handle;   // Get application data packet (shared resource read)
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();

    //####### ONLY TEMPERATURE READING IMPLEMENTED ########
    
    // Buffer capable of holding temperature only
    unsigned char app_pack[sizeof(rvn_ota_rsp_data_t) + sizeof(int16_t)];
    rvn_ota_rsp_data_t* ota_rsp_data = (rvn_ota_rsp_data_t*)app_pack;
        /*
    switch (handle) {
    case HANDLE_TEMP:*/
        int16_t temp = temp_get(avrraven.user_config.unit.temp);        // read temperature
   /*     break;
    default:
        break;
    }
    */
    
    // Build response
    ota_rsp_data->id = RVN_OTA_RSP_DATA;
    ota_rsp_data->size = sizeof(int16_t);
    ota_rsp_data->data[0] = (temp & 0x00FF) >> 0;
    ota_rsp_data->data[1] = (temp & 0xFF00) >> 8;
    
    // ACK sipc packet
    SIPC_ACK_PACKET();
    
    // Send frame
    if (ota_transaction((void*)ota_rsp_data, adr, seq_nmbr, sizeof(rvn_ota_rsp_data_t) + sizeof(int16_t)) == EOF) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
    }
}

void ota_cmd_getname(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    // Buffer capable of holding a max size node name
    unsigned char app_pack[sizeof(rvn_ota_rsp_str_t) + AVRRAVEN_NAME_SIZE_MAX];
    rvn_ota_rsp_str_t* ota_rsp_str = (rvn_ota_rsp_str_t*)app_pack;
    
    // Build response
    ota_rsp_str->id = RVN_OTA_RSP_STRING;
    ota_rsp_str->size = strlen((const char*)avrraven.user_config.name);
    strcpy((char*)ota_rsp_str->data, (const char*)avrraven.user_config.name);
    
    // Send frame
    int app_pack_size = sizeof(rvn_ota_rsp_str_t) + ota_rsp_str->size;
    if (ota_transaction((void*)ota_rsp_str, adr, seq_nmbr, app_pack_size) == EOF) {
        ota_simple_response(RVN_OTA_RSP_ERROR, adr, seq_nmbr);
    }
}

void ota_rsp_error(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    // No data...
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    switch (avrraven.status.state) {
    case RVN_OTA_RSP_ERROR:
    case RVN_OTA_RSP_BUSY:
    case RVN_OTA_RSP_NOT_SUPPORTED:
        switch (avrraven.status.state) {
        case STATE_IDLE:
            lcd_puts_P("OTA ERROR");
            led_status_set(LED_FAST_BLINK);
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
            break;
        case STATE_MAIL_SENDING:
            lcd_puts_P("SENT WITH ERRORS");
            led_status_set(LED_FAST_BLINK);
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
            break;
        default:
            // ERROR response in an unknown state is ignored
            break;
        }
        break;
    default:
        // Ignored
        break;
    }
}

void ota_rsp_ok(rvn_loc_evt_ota_packet_t* ota_packet)
{
    // Get transport layer data
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;
    uint16_t adr = ota_packet->adr;
    uint8_t seq_nmbr = ota_transport->seq_nmbr;
    
    // Get application layer data
    // No data...
    
    // ACK sipc packet when all data read
    SIPC_ACK_PACKET();
    
    switch (avrraven.status.state) {
    case STATE_MAIL_SENDING:
        if (seq_nmbr != avrraven.status.trans_seq_nmbr) {
            lcd_puts_P("NOT RESPONDING");
            lcd_num_puthex(adr, LCD_NUM_PADDING_ZERO);
            led_status_set(LED_FAST_BLINK);
        } else {
            lcd_puts_P("SENT");
            lcd_num_puthex(adr, LCD_NUM_PADDING_ZERO);
            led_status_set(LED_SOFT_BLINK);
        }
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    default:
        // OK response in an unknown state is ignored
        break;
    }
}

void ota_simple_transaction(uint8_t response, uint16_t address, uint8_t seq_nmbr)
{
    // OTA packet
    uint8_t memory_block[sizeof(rvn_loc_cmd_ota_packet_t) + sizeof(rvn_ota_transport_t) + 1];
    
    // OTA packet header        (Network layer)
    rvn_loc_cmd_ota_packet_t* ota_packet;
    
    // OTA transport header     (Transport layer)
    rvn_ota_transport_t* ota_transport;
    
    // OTA application data     (Application layer)
    rvn_ota_rsp_std_t* ota_application_data;
    
    // Tie packages together
    ota_packet = (rvn_loc_cmd_ota_packet_t*)memory_block;       // OTA packet header
    ota_transport = (rvn_ota_transport_t*)&ota_packet->data;    // OTA transport header
    ota_application_data = (rvn_ota_rsp_std_t*)&ota_transport->data;       // OTA command
        
    // Build network layer header
    ota_packet->id = RVN_LOC_CMD_PACKET;
    ota_packet->adr = address;
    ota_packet->size = sizeof(rvn_ota_transport_t) + 1;
    
    // Build transport layer header
    ota_transport->seq_nmbr = seq_nmbr;
    
    // Build application packet
    ota_application_data->id = response;
    
    // Send entire frame
    sipc_send_frame(sizeof(rvn_loc_cmd_ota_packet_t) + sizeof(rvn_ota_transport_t) + 1, (uint8_t *)ota_packet);
}
        
int ota_transaction(void* data, uint16_t adr, uint8_t seq_nmbr, int data_size)
{
    // OTA packet
    void* memory_block;
    uint8_t ota_packet_size = sizeof(rvn_loc_cmd_ota_packet_t) + sizeof(rvn_ota_transport_t) + data_size;
    
    // OTA packet header        (Network layer)
    rvn_loc_cmd_ota_packet_t* ota_packet;
    int8_t ota_nwk_layer_size = sizeof(rvn_ota_transport_t) + data_size;
    
    // OTA transport header     (Transport layer)
    rvn_ota_transport_t* ota_transport;
    
    // OTA application data     (Application layer)
    uint8_t* ota_application_data;
    size_t ota_application_size = data_size;
    
    
    // Allocate memory for entire frame
    if ((memory_block = vrt_mem_alloc((uint16_t)ota_packet_size)) == NULL) {
        return EOF;
    }
    
    // Tie packages together
    ota_packet = (rvn_loc_cmd_ota_packet_t*)memory_block;   // OTA packet header
    ota_transport = (rvn_ota_transport_t*)&ota_packet->data; // OTA transport header
    ota_application_data = (uint8_t*)&ota_transport->data;      // OTA command
        
    // Build network layer header
    ota_packet->id = RVN_LOC_CMD_PACKET;
    ota_packet->adr = adr;
    ota_packet->size = ota_nwk_layer_size;
    
    // Build transport layer header
    ota_transport->seq_nmbr = seq_nmbr;
    
    // Build response
    memcpy((void*)ota_application_data, (const void*)data, ota_application_size); // copy application data
    
    // Send entire frame
    sipc_send_frame(ota_packet_size, (uint8_t *)ota_packet);
    
    // Free memory block
    vrt_mem_free((void*)memory_block);
    
    // Return success
    return 0;
}
//! @}
/*EOF*/
