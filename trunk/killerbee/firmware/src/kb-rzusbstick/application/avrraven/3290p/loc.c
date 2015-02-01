// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Handlers for all local(3290p <-> 1284p) transactions
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
 * $Id: mbox.c 41098 2008-04-28 20:25:13Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "loc.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "vrt_mem.h"

#include "sipc.h"

#include "rvn_ota.h"
#include "rvn_loc.h"

#include "avrraven_3290p.h"

#include "menu.h"
#include "bl.h"
#include "menu_definitions.h"
#include "menu_funcs.h"

#include "led.h"
#include "lcd.h"

#include "ota.h"

#include "compiler.h"

//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! A pointer to a struct containing firmware for the ATmega1284p
static rvn_loc_cmd_fw_packet_t* m1284p_fw_packet;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/
/*! \brief  Send a package of firware data to ATmeaga1284p
 *
 *  \param[in]  fw_img_offset   Data flash image offset
 *  \param[in]  offset          Firmware package offset
 *  \param[in]  fw_packet       Pointer to firmware data packet
 */
static void send_m1284p_fw_packet(df_adr_t fw_img_offset, uint32_t offset, rvn_loc_cmd_fw_packet_t* fw_packet);

/*========================= IMPLEMENTATION           =========================*/
void loc_rsp_ok(void* rsp)
{
    rvn_loc_cmd_get_param_t cmd_get_param;
    rvn_loc_cmd_std_t cmd;
    uint32_t blcc;
    FMENU_item_flash_t* current_menu_entry = FMENU_GetCurrentItem(&MENU_menuHandle);
    
    
    SIPC_ACK_PACKET();
        
    switch (avrraven.status.state) {
    /*======================== FW write               ========================*/
    case STATE_FW_WRITE_INIT:
        lcd_puts_P("RECEIVING FW IMAGE");
        avrraven.status.state = STATE_FW_WRITE;
        break;
    case STATE_FW_WRITE_INIT_WAIT_RX_ENABLE_RESPONSE:
        avrraven.status.state = STATE_FW_WRITE;
        break;
    case STATE_FW_WRITE_COMPLETE_WAIT_RX_ENABLE_RESPONSE:
        lcd_puts_P("DONE");
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    case STATE_OTA_TRANS_WAITING_FOR_RX_ENABLE_RESPONSE:
        avrraven.status.state = STATE_IDLE;
        break;
    case STATE_FW_WRITE_WAIT_RX_ENABLE_RESPONSE:
        avrraven.status.state = STATE_FW_WRITE;
        break;
    case STATE_FW_WRITE_WAIT_TX_DONE:
        avrraven.status.state = STATE_FW_WRITE;
        break;
    case STATE_FW_WRITE_COMPLETE_WAIT_TX_DONE:
        lcd_puts_P("DONE");
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    case STATE_FW_WRITE_IMAGE:
        // Set ATmega1284p in boot loader mode
        led_status_set(LED_FAST_BLINK);
        lcd_puts_P("WRITING M1284 IMAGE");
        cmd.id = RVN_LOC_CMD_ENTER_BOOT;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_M1284P_UPGRADE_INIT_WAIT_RESPOND;
        break;
    case STATE_FW_WRITE_COMPLETE_WAIT_RADIO:
        lcd_symbol_set(LCD_SYMBOL_ZLINK);
        lcd_symbol_set(LCD_SYMBOL_ZIGBEE);
        lcd_symbol_antenna_signal(LCD_SYMBOL_ANTENNA_SIG_3);
        cmd.id = RVN_LOC_CMD_RX_ON;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_FW_WRITTEN_WAIT_RX_ENABLE_RESPONSE;
        break;
    case STATE_FW_WRITTEN_WAIT_RX_ENABLE_RESPONSE:
        lcd_puts_P("FW WRITTEN SUCCESSFULLY");
        ota_event(RVN_OTA_EVT_FW_WRITE_DONE, 0x0000);
        BLCC_WRITE(BLCC_NORMAL_APP_START);
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    case STATE_FW_WRITE: 
        break;
    /*======================== Radio                  ========================*/
    case STATE_RADIO_CONNECTING:
        lcd_symbol_set(LCD_SYMBOL_ZLINK);
        lcd_symbol_set(LCD_SYMBOL_ZIGBEE);
        lcd_symbol_antenna_signal(LCD_SYMBOL_ANTENNA_SIG_3);
        cmd.id = RVN_LOC_CMD_RX_ON;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_RADIO_WAIT_FOR_RX_ENABLE_RESPONSE;
        break;
    case STATE_RADIO_DISCONNECTING:
        lcd_symbol_clr(LCD_SYMBOL_ZLINK);
        lcd_symbol_clr(LCD_SYMBOL_ZIGBEE);
        lcd_symbol_clr(LCD_SYMBOL_RX);
        lcd_symbol_clr(LCD_SYMBOL_TX);
        lcd_symbol_antenna_signal(LCD_SYMBOL_ANTENNA_DIS);
        lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
        lcd_num_clr_all();
        avrraven.status.ntwk_address = NTWK_ADDRESS_INVALID;
        avrraven.status.state = STATE_IDLE;
        break;
    case STATE_RADIO_WAIT_FOR_RX_ENABLE_RESPONSE:
    case STATE_RADIO_WAIT_FOR_RX_REENABLE_RESPONSE:
        
        // Get short address
        cmd_get_param.id = RVN_LOC_CMD_GET_PARAM;
        cmd_get_param.param = NWK_ADDRESS;
        sipc_send_frame(sizeof(rvn_loc_cmd_get_param_t), (uint8_t *)&cmd_get_param);

        avrraven.status.state = STATE_RADIO_GET_ADDRESS_WAIT_RESPONSE;
        break;
    /*======================== ATmega1284p            ========================*/
    case STATE_M1284P_UPGRADE_INIT_WAIT_RESPOND:
        /*Does nothing. Waiting for enter bootloader event*/
        break;
    case STATE_M1284P_UPGRADE_PACKET_WAIT_RESPOND:
        send_m1284p_fw_packet(avrraven.status.m1284p_img_offset, avrraven.status.m1284p_address_offset, m1284p_fw_packet);
        lcd_num_putdec((int)(avrraven.status.m1284p_address_offset/1024), LCD_NUM_PADDING_SPACE);
        if ((avrraven.status.m1284p_address_offset += RVN_FW_PACKET_SIZE) == M1284P_APP_FLASH_SIZE) {
            vrt_mem_free((void *)(m1284p_fw_packet));
            lcd_puts_P("DONE");
            avrraven.status.state = STATE_M1284P_UPGRADE_DONE_WAIT_RESPOND;
        } else {
            /* stay in same avrraven.status.state and wait for response on the FW packet*/
        }
        break;
    case STATE_M1284P_UPGRADE_DONE_WAIT_RESPOND:
        // Start ATmega1284's new FW
        cmd.id = RVN_LOC_CMD_APP_START;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        
        // If m1284 factory default fw loaded, m3290 fw allready upgraded, and 
        BLCC_READ(blcc);
        if (blcc == BLCC_LOAD_FACTORY_DEFAULT) {
            BLCC_WRITE(BLCC_FW_UPGRADE_COMPLETE);
            reboot();
        } else {
            // Start own bootloader without waitig for 1284 to reboot
            BLCC_WRITE(BLCC_FW_UPGRADE_START_REQUEST_FROM_APP);
            reboot();
        }
        break;
    case STATE_M1284P_NEW_FW_WAIT_EVENT:
        /*Does nothing. Waiting for app prog start event*/
        break;
    case STATE_M1284P_RESTART_WAIT_BOOT_EVENT:
    case STATE_M1284P_ENTER_BOOT_WAIT_BOOT_EVENT:
        /*Does nothing. Waiting for boot loader start event*/
        break;
    case STATE_M1284P_RESTART_WAIT_APP_EVENT:
        /*Does nothing. Waiting for app prog start event*/
        break;
    case STATE_M1284P_DISCONNECT_WAIT_RESPOND:
        // Set ATmega1284p in boot loader mode
        led_status_set(LED_FAST_BLINK);
        lcd_puts_P("WRITING 1284P FW");
        cmd.id = RVN_LOC_CMD_ENTER_BOOT;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_M1284P_UPGRADE_INIT_WAIT_RESPOND;
        break;
    /*======================== Misc                   ========================*/
    case STATE_OTA_TRANSACTION_WAITING_FOR_RESPONSE:
        lcd_symbol_clr(LCD_SYMBOL_RX); // TIMED_FUNCTION
        lcd_symbol_set(LCD_SYMBOL_TX); // TIMED_FUNCTION
        cmd.id = RVN_LOC_CMD_RX_ON;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_OTA_TRANS_WAITING_FOR_RX_ENABLE_RESPONSE;
        break;
    case STATE_MAIL_SEND_INIT:
        lcd_puts_P("SENDING");
        led_status_set(LED_SOFT_BLINK);
        avrraven.status.state = STATE_MAIL_SENDING;
        break;
    default:
        // OK responses in unknown state is ignored
        break;
    }
}

void loc_rsp_fail(void* rsp)
{
    rvn_loc_cmd_std_t cmd;
    
    SIPC_ACK_PACKET();
    
    switch (avrraven.status.state) {
    case STATE_OTA_TRANS_WAITING_FOR_RX_ENABLE_RESPONSE:
        lcd_symbol_clr(LCD_SYMBOL_RX); // TIMED_FUNCTION
        lcd_symbol_clr(LCD_SYMBOL_TX); // TIMED_FUNCTION
        avrraven.status.state = STATE_IDLE;
        break;
    case STATE_RADIO_WAIT_FOR_RX_REENABLE_RESPONSE:
        lcd_puts_P("RX ENABLE FAILED");
        led_status_set(LED_FAST_BLINK);
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    case STATE_RADIO_WAIT_FOR_RX_ENABLE_RESPONSE:
        cmd.id = RVN_LOC_CMD_RX_ON; // re-try enabling rx
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_RADIO_WAIT_FOR_RX_REENABLE_RESPONSE;
        break;
    case STATE_FW_WRITE:
        lcd_puts_P("PACKET ERROR");
        led_status_set(LED_FAST_BLINK);
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    case STATE_RADIO_CONNECTING:
        lcd_puts_P("NO NET");
        led_status_set(LED_FAST_BLINK);
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    default:
        lcd_puts_P("ERROR");
        led_status_set(LED_FAST_BLINK);
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    }
}

void loc_rsp_get_param(void* rsp)
{
    rvn_loc_cmd_radio_connect_t cmd;
    uint32_t blcc;
    rvn_loc_rsp_get_param_t* rsp_param = (rvn_loc_rsp_get_param_t*)rsp;
    FMENU_item_flash_t* current_menu_entry = FMENU_GetCurrentItem(&MENU_menuHandle);
    FMENU_item_flash_t* startup_menu_entry;
    menu_action_t menu_action = MENU_ACTION_REFRESH;
    
    switch (avrraven.status.state) {
    case STATE_RADIO_GET_PARAM:
        lcd_num_puthex(*(uint16_t*)&rsp_param->data[0], LCD_NUM_PADDING_ZERO);
        avrraven.status.state = STATE_IDLE;
    
        SIPC_ACK_PACKET();
        break;
    case STATE_RADIO_GET_ADDRESS_WAIT_RESPONSE:
        avrraven.status.ntwk_address = *(uint16_t*)&rsp_param->data[0];
        navigate_menu(current_menu_entry, current_menu_entry, menu_action);
        led_status_set(LED_OFF);
        avrraven.status.state = STATE_IDLE;
    
        SIPC_ACK_PACKET();
        break;
    case STATE_M1284P_RESTART_GET_REV:
        // Store M1284P FW revisions
        avrraven.status.m1284p_app_rev = rsp_param->data[0];
        avrraven.status.m1284p_app_rev |= rsp_param->data[1] << 8;
        avrraven.status.m1284p_bl_rev  = rsp_param->data[2];
        avrraven.status.m1284p_bl_rev  |= rsp_param->data[3] << 8;
    
        SIPC_ACK_PACKET();
        
        // Read bootloader communication channel in EEPROM
        BLCC_READ(blcc);
        if (blcc == BLCC_FW_UPGRADE_COMPLETE) {
            // Enable radio
            cmd.id = RVN_LOC_CMD_RADIO_CONNECT;
            cmd.channel = avrraven.user_config.channel;
            cmd.pan_id = avrraven.user_config.panid;
            sipc_send_frame(sizeof(rvn_loc_cmd_radio_connect_t), (uint8_t *)&cmd);
            lcd_puts_P("JOINING NETWORK");
            led_status_set(LED_SOFT_BLINK);
            avrraven.status.state = STATE_FW_WRITE_COMPLETE_WAIT_RADIO;
        } else {
            // Navigate to startup menu
            startup_menu_entry = FMENU_GoToRootItem(&MENU_menuHandle);
            navigate_menu(startup_menu_entry, startup_menu_entry, menu_action);
            
            // If auto join is enabled, try to join network
            if (avrraven.user_config.join_auto == true) {
                // Enable radio
                cmd.id = RVN_LOC_CMD_RADIO_CONNECT;
                cmd.channel = avrraven.user_config.channel;
                cmd.pan_id = avrraven.user_config.panid;
                sipc_send_frame(sizeof(rvn_loc_cmd_radio_connect_t), (uint8_t *)&cmd);
                lcd_puts_P("JOINING NETWORK");
                led_status_set(LED_SOFT_BLINK);
                lcd_num_clr_all();
                avrraven.status.state = STATE_RADIO_CONNECTING;
            } else {
                avrraven.status.state = STATE_IDLE;
            }
        }
        break;
    default:
        // Param responses in unknown state is ignored. Impossible to know what to do with the data
        break;
    }
}

void loc_evt_app_started(void *evt)
{
    rvn_loc_cmd_get_param_t cmd;
    
    SIPC_ACK_PACKET();
    
    switch (avrraven.status.state) {
    case STATE_M1284P_RESTART_WAIT_APP_EVENT:
        cmd.id = RVN_LOC_CMD_GET_PARAM;
        cmd.param = FW_REV;
        sipc_send_frame(sizeof(rvn_loc_cmd_radio_connect_t), (uint8_t *)&cmd);
        lcd_puts_P("WAIT FOR M1284 REV REQ");
        avrraven.status.state = STATE_M1284P_RESTART_GET_REV;
        break;
    case STATE_M1284P_NEW_FW_WAIT_EVENT:
        // Start own bootloader
        BLCC_WRITE(BLCC_FW_UPGRADE_START_REQUEST_FROM_APP);
        reboot();
        break;
    default:
        // This event is ignored in any other state
        break;
    }
}

void loc_evt_bl_entered(void *evt)
{
    rvn_loc_cmd_std_t cmd;
    
    SIPC_ACK_PACKET();
    
    switch (avrraven.status.state) {
    case STATE_M1284P_RESTART_WAIT_BOOT_EVENT:
        cmd.id = RVN_LOC_CMD_APP_START;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        lcd_puts_P("WAIT FOR M1284 APP PROG");
        avrraven.status.state = STATE_M1284P_RESTART_WAIT_APP_EVENT;
        break;
    case STATE_M1284P_ENTER_BOOT_WAIT_BOOT_EVENT:
        // ATmega1284 is in boot loader
        led_status_set(LED_FAST_BLINK);
        lcd_puts_P("WRITING M1284 FACTORY DEFAULT FW");
        cmd.id = RVN_LOC_CMD_ENTER_BOOT;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
       
        // Set FW image offset to read from factory default location on data flash
        avrraven.status.m1284p_img_offset = M1284P_FLASH_FD_IMG_ADR;
        
        // Set new avrraven.status.state
        avrraven.status.state = STATE_M1284P_UPGRADE_INIT_WAIT_RESPOND;
        break;
    case STATE_M1284P_UPGRADE_INIT_WAIT_RESPOND:
        // ATmega1284 has entered booloader mode
        lcd_symbol_clr(LCD_SYMBOL_ZLINK);
        lcd_symbol_clr(LCD_SYMBOL_ZIGBEE);
        lcd_symbol_clr(LCD_SYMBOL_RX);
        lcd_symbol_clr(LCD_SYMBOL_TX);
        lcd_symbol_antenna_signal(LCD_SYMBOL_ANTENNA_DIS);
        
        // Allocate memory for FW packets
        if ((m1284p_fw_packet = (rvn_loc_cmd_fw_packet_t*)vrt_mem_alloc(sizeof(rvn_loc_cmd_fw_packet_t)+RVN_FW_PACKET_SIZE)) == NULL){
            break;
        }
        
        // Initiate transfer
        avrraven.status.m1284p_address_offset = 0;
        send_m1284p_fw_packet(avrraven.status.m1284p_img_offset, avrraven.status.m1284p_address_offset, m1284p_fw_packet);
        lcd_num_putdec((int)(avrraven.status.m1284p_address_offset/1024), LCD_NUM_PADDING_SPACE);
        avrraven.status.state = STATE_M1284P_UPGRADE_PACKET_WAIT_RESPOND;
        break;
    default:
        // This event is ignored in any other state
        break;
    }
}

void loc_evt_ota_packet(void *evt)
{
    rvn_loc_evt_ota_packet_t* ota_packet = evt;                                     // Get OTA packet header    (Network layer)
    rvn_ota_transport_t* ota_transport = (rvn_ota_transport_t*)&ota_packet->data;   // Get transport header
    uint8_t* app_data = (uint8_t*)&ota_transport->data;                             // Get application data packet
    uint8_t id = app_data[0];                                                       // Get application packet id
    
    // Update radio symbols
    lcd_symbol_clr(LCD_SYMBOL_TX);  // TIMED_FUNCTION
    lcd_symbol_set(LCD_SYMBOL_RX);  // TIMED_FUNCTION
    lcd_symbol_antenna_signal(3);
    if (ota_packet->lqi<220) {
        lcd_symbol_clr(LCD_SYMBOL_ANT_SIG3);
    }
    if (ota_packet->lqi<200) {
        lcd_symbol_clr(LCD_SYMBOL_ANT_SIG2);
    }
    if (ota_packet->lqi<150) {
        lcd_symbol_clr(LCD_SYMBOL_ANT_SIG1);
    }
    
    // Execute OTA command handler
    switch (id) {
    case RVN_OTA_CMD_FW_WRITE_INIT:
        ota_cmd_fw_write_initiated(ota_packet);
        break;
    case RVN_OTA_CMD_FW_WRITE_PACKET:
        ota_cmd_fw_write_packet(ota_packet);
        break;
    case RVN_OTA_CMD_FW_WRITE_COMPLETED:
        ota_cmd_fw_write_completed(ota_packet);
        break;
    case RVN_OTA_CMD_FW_WRITE_IMAGE:
        ota_cmd_fw_write_image(ota_packet);
        break;
    case RVN_OTA_CMD_TXTMSG:
        ota_cmd_txtmsg(ota_packet);
        break;
    case RVN_OTA_CMD_SHARED_RD:
        ota_cmd_shared_rd(ota_packet);
        break;
    case RVN_OTA_CMD_GETNAME:
        ota_cmd_getname(ota_packet);
        break;
    case RVN_OTA_CMD_FOPEN:
        ota_cmd_fopen(ota_packet);
        break;
    case RVN_OTA_CMD_FCLOSE:
        ota_cmd_fclose(ota_packet);
        break;
    case RVN_OTA_CMD_FREAD:
        ota_cmd_fread(ota_packet);
        break;
    case RVN_OTA_CMD_FWRITE:
        ota_cmd_fwrite(ota_packet);
        break;
    case RVN_OTA_CMD_SIGN_ON:
        ota_cmd_sign_on(ota_packet);
        break;
    case RVN_OTA_RSP_OK:
        ota_rsp_ok(ota_packet);
        break;
    case RVN_OTA_RSP_ERROR:
    case RVN_OTA_RSP_BUSY:
    case RVN_OTA_RSP_NOT_SUPPORTED:
        ota_rsp_error(ota_packet);
        break;
    default:{
        // ACK unknown sipc packet
        SIPC_ACK_PACKET();
    
        // Respond not supported
        ota_simple_response(RVN_OTA_RSP_NOT_SUPPORTED, ota_packet->adr, ota_transport->seq_nmbr);}
        break;
    }
}

static void send_m1284p_fw_packet(df_adr_t fw_img_offset, uint32_t offset, rvn_loc_cmd_fw_packet_t* fw_packet)
{
    // read and send 'RVN_FW_PACKET_SIZE' bytes at time
    df_read(fw_img_offset + offset, RVN_FW_PACKET_SIZE, (uint8_t*)fw_packet->data);
    fw_packet->id = RVN_LOC_CMD_FW_PACKET;
    fw_packet->size = RVN_FW_PACKET_SIZE;
    fw_packet->adr = offset;
    sipc_send_frame(sizeof(rvn_loc_cmd_fw_packet_t)+RVN_FW_PACKET_SIZE, (uint8_t*)fw_packet);
}
//! @}
/*EOF*/
