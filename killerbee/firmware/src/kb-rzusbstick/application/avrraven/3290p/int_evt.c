// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Handlers for all internal(3290p only) events
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
 * $Id: numedit.c 41136 2008-04-29 11:43:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/


/*========================= INCLUDES                 =========================*/
#include "int_evt.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "menu_funcs.h"

#include "sipc.h"
#include "macros.h"
#include "bl.h"
#include "mmap.h"
#include "timndate.h"
#include "df.h"
#include "txtedit.h"
#include "numedit.h"
#include "audio.h"
#include "battery.h"

#include "led.h"
#include "lcd.h"
#include "sfs.h"

#include "tick.h"

#include "key.h"
#include "menu.h"
#include "temp.h"
#include "rvn_loc.h"
#include "rvn_ota.h"
#include "mbox.h"
#include "menu_definitions.h"
#include "avrraven_3290p.h"

//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/
//! Pointer to mail buffer. Used for reading mail
mbox_mail_t* current_mail;

//! Pointer to mail buffer. Used for writing mail
uint8_t* new_mail_buffer;

//! Generic numeric editor buffer
uint16_t numedit_buffer;

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
void int_evt_startup(void* evt)
{
    rvn_loc_cmd_std_t cmd;
    uint32_t blcc;
    
    switch (avrraven.status.state) {
    case STATE_STARTUP:
        /* Restart ATmega1284 application program. If in bootloader mode, start 
         * Application program. If command disappears (during ATmega1284 startup etc.)
         * this application will react on the application startup event sent by
         * ATmega1284.
         */
        cmd.id = RVN_LOC_CMD_ENTER_BOOT;
        sipc_send_frame(sizeof(rvn_loc_cmd_std_t), (uint8_t *)&cmd);
        
        // Put init symbols
        lcd_symbol_set(LCD_SYMBOL_RAVEN);
        lcd_symbol_clr(LCD_SYMBOL_RX);
        lcd_symbol_clr(LCD_SYMBOL_TX);
        lcd_symbol_antenna_signal(LCD_SYMBOL_ANTENNA_DIS);
                
        avrraven.status.ntwk_address = NTWK_ADDRESS_INVALID;
        
        // 
        led_status_set(LED_OFF);
        
        // 
        lcd_puts_P("WAIT FOR M1284 BOOT LOADER");
        
        // If m1284 factory default fw should be loaded:
        BLCC_READ(blcc);
        if (blcc == BLCC_LOAD_FACTORY_DEFAULT) {
            avrraven.status.state = STATE_M1284P_ENTER_BOOT_WAIT_BOOT_EVENT; 
        } else {
            avrraven.status.state = STATE_M1284P_RESTART_WAIT_BOOT_EVENT;
        }
        
        break;
    default:
        // Startupevent in any other state is ignored
        break;
    }
}

void int_evt_key(void* evt)
{
    key_state_t key_state = *(key_state_t*)evt;
    FMENU_item_flash_t* current_menu_entry = FMENU_GetCurrentItem(&MENU_menuHandle);
    FMENU_item_flash_t* new_menu_entry = current_menu_entry;
    menu_action_t menu_action = MENU_ACTION_ENTER;
    int int_temp;
    static uint8_t file_name[SFS_MAX_FILENAME_BUFFER_SIZE];
    unsigned char search_string[SFS_MAX_FILENAME_BUFFER_SIZE];
    lcd_config_t config;
    static key_state_t auto_key_state;
    static bool auto_key_wait = true;
    static bool auto_key_registered = false;
    static asy_tmr_t auto_key_timer = ASY_TMR_NO_TIMER;
    
    // Navigate to next avrraven.status.state
    switch (key_state){
    case KEY_UP:
        switch (avrraven.status.state) {
        case STATE_IDLE:
            new_menu_entry = FMENU_NavigateUp(&MENU_menuHandle);
            break;
        case STATE_CONFIG_DEBUG_ENABLE:
            lcd_puts_P("ENABLE");
            avrraven.user_config.debug_enable = true;
            break;
        case STATE_CONFIG_TXTPREV_ENABLE:
            lcd_puts_P("ENABLE");
            avrraven.user_config.txtmsg_preview = true;
            break;
        case STATE_CONFIG_FW_UPGRADE:
            lcd_puts_P("AUTO");
            avrraven.user_config.fw_upgr_auto = true;
            break;
        case STATE_CONFIG_CONNECT:
            lcd_puts_P("AUTO");
            avrraven.user_config.join_auto = true;
            break;
        case STATE_CONFIG_IMG_RCV:
            lcd_puts_P("AUTO");
            avrraven.user_config.fw_rcv_auto = true;
            break;
        case STATE_CONFIG_NAME:
        case STATE_MAIL_COMPOSE:
            txtedit_event(TXTEDIT_KEY_UP);
            break;
        case STATE_CONFIG_LCD_CONTRAST:
            lcd_num_putdec((int)numedit_event(NUMEDIT_KEY_UP), LCD_NUM_PADDING_SPACE);
            config = lcd_config_get();
            config.contrast = numedit_buffer;
            lcd_config_set(config);
            break;
        case STATE_CONFIG_CLOCK_MIN:
            {
                timendate_clk_t set_time;
                timndate_clock_get(&set_time, avrraven.user_config.unit.clock); // hours from current clock
                int clk = set_time.hour*100 + numedit_event(NUMEDIT_KEY_UP); // minutes from current editing
                lcd_num_putdec(clk, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_CONFIG_CLOCK_HOUR:
            {
                timendate_clk_t set_time;
                timndate_clock_get(&set_time, avrraven.user_config.unit.clock); // minutes from current clock
                int clk = numedit_event(NUMEDIT_KEY_UP)*100 + set_time.min; // hours from current editing
                lcd_num_putdec(clk, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_CONFIG_LCD_SCROLLING:
            lcd_num_putdec((int)numedit_event(NUMEDIT_KEY_UP), LCD_NUM_PADDING_SPACE);
            
            // Update LCD scrolling settings
            config = lcd_config_get();
            config.scrolling = (lcd_scrolling_t)numedit_value_get();
            lcd_config_set(config);
            break;
        case STATE_CONFIG_RADIO_CHANNEL:
            lcd_num_putdec((int)numedit_event(NUMEDIT_KEY_UP), LCD_NUM_PADDING_SPACE);
            break;
        case STATE_CONFIG_RADIO_PANID:
        case STATE_MAIL_COMPOSE_ADDRESS_SET:
            lcd_num_puthex(numedit_event(NUMEDIT_KEY_UP), LCD_NUM_PADDING_ZERO);
            break;
        case STATE_CONFIG_UNIT_TEMP:
            avrraven.user_config.unit.temp = TEMP_UNIT_CELCIUS;
            lcd_symbol_clr(LCD_SYMBOL_F);
            lcd_symbol_set(LCD_SYMBOL_C);
            break;
        case STATE_CONFIG_UNIT_CLOCK:
            avrraven.user_config.unit.clock = TIME_CLOCK_UNIT_24;
            if (avrraven.user_config.unit.clock == TIME_CLOCK_UNIT_24) {
                lcd_symbol_clr(LCD_SYMBOL_AM);
                lcd_symbol_clr(LCD_SYMBOL_PM);
                lcd_num_puthex(0x2400, LCD_NUM_PADDING_ZERO);
            } else {
                lcd_symbol_set(LCD_SYMBOL_AM);
                lcd_symbol_set(LCD_SYMBOL_PM);
                lcd_num_puthex(0x1200, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_MAIL_READ:
            mbox_mail_get(&current_mail, MBOX_GET_NEXT);
            if (current_mail != NULL) {
                int_temp = current_mail->size;
                lcd_puta(current_mail->data, int_temp);
                lcd_num_puthex(current_mail->address, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_FW_WAIT_USER_CONFIRMATION:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_AUDIO_FILE_SELECT:
            strncpy_P((char*)search_string, AUDIO_FILE_TYPE, AUDIO_FILE_TYPE_SIZE);
            if (avrraven.status.batt_low == false) {
                sfs_fget(search_string, file_name, SFS_FGET_REV);
                lcd_puts((const char*)file_name);
            } else {
                lcd_puts_P("ERROR");
            }
            break;
        case STATE_STORAGE_FILE_LIST:
            strncpy_P((char*)search_string, "*.*", 4);
            if (avrraven.status.batt_low == false) {
                sfs_fget(search_string, file_name, SFS_FGET_REV);
                lcd_puts((const char*)file_name);
            } else {
                lcd_puts_P("ERROR");
            }
            break;
        default:
            // Key events in unknown state is ignored
            break;
        }
        break;
    case KEY_DOWN:
        switch (avrraven.status.state) {
        case STATE_IDLE:
            new_menu_entry = FMENU_NavigateDown(&MENU_menuHandle);
            break;
        case STATE_CONFIG_DEBUG_ENABLE:
            lcd_puts_P("DISABLE");
            avrraven.user_config.debug_enable = false;
            break;
        case STATE_CONFIG_TXTPREV_ENABLE:
            lcd_puts_P("DISABLE");
            avrraven.user_config.txtmsg_preview = false;
            break;
        case STATE_CONFIG_FW_UPGRADE:
            lcd_puts_P("USR ACK");
            avrraven.user_config.fw_rcv_auto = false;
            break;
        case STATE_CONFIG_CONNECT:
            lcd_puts_P("MANUAL");
            avrraven.user_config.join_auto = false;
            break;
        case STATE_CONFIG_IMG_RCV:
            lcd_puts_P("USR ACK");
            avrraven.user_config.fw_rcv_auto = false;
            break;
        case STATE_MAIL_COMPOSE:
        case STATE_CONFIG_NAME:
            txtedit_event(TXTEDIT_KEY_DOWN);
            break;
        case STATE_CONFIG_LCD_CONTRAST:
            lcd_num_putdec((int)numedit_event(NUMEDIT_KEY_DOWN), LCD_NUM_PADDING_SPACE);
            config = lcd_config_get();
            config.contrast = numedit_buffer;
            lcd_config_set(config);
            break;
        case STATE_CONFIG_CLOCK_MIN:
            {
                timendate_clk_t set_time;
                timndate_clock_get(&set_time, avrraven.user_config.unit.clock); // hours from current clock
                int clk = set_time.hour*100 + numedit_event(NUMEDIT_KEY_DOWN); // minutes from current editing
                lcd_num_putdec(clk, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_CONFIG_CLOCK_HOUR:
            {
                timendate_clk_t set_time;
                timndate_clock_get(&set_time, avrraven.user_config.unit.clock); // minutes from current clock
                int clk = numedit_event(NUMEDIT_KEY_DOWN)*100 + set_time.min; // hours from current editing
                lcd_num_putdec(clk, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_CONFIG_LCD_SCROLLING:
            lcd_num_putdec((int)numedit_event(NUMEDIT_KEY_DOWN), LCD_NUM_PADDING_SPACE);
            
            // Update LCD scrolling settings
            config = lcd_config_get();
            config.scrolling = (lcd_scrolling_t)numedit_value_get();
            lcd_config_set(config);
            break;
        case STATE_CONFIG_RADIO_CHANNEL:
            lcd_num_putdec((int)numedit_event(NUMEDIT_KEY_DOWN), LCD_NUM_PADDING_SPACE);
            break;
        case STATE_CONFIG_RADIO_PANID:
        case STATE_MAIL_COMPOSE_ADDRESS_SET:
            lcd_num_puthex(numedit_event(NUMEDIT_KEY_DOWN), LCD_NUM_PADDING_ZERO);
            break;
        case STATE_CONFIG_UNIT_TEMP:
            avrraven.user_config.unit.temp = TEMP_UNIT_FAHRENHEIT;
            lcd_symbol_clr(LCD_SYMBOL_C);
            lcd_symbol_set(LCD_SYMBOL_F);
            break;
        case STATE_CONFIG_UNIT_CLOCK:
            avrraven.user_config.unit.clock = TIME_CLOCK_UNIT_12;
            if (avrraven.user_config.unit.clock == TIME_CLOCK_UNIT_24) {
                lcd_symbol_clr(LCD_SYMBOL_AM);
                lcd_symbol_clr(LCD_SYMBOL_PM);
                lcd_num_puthex(0x2400, LCD_NUM_PADDING_ZERO);
            } else {
                lcd_symbol_set(LCD_SYMBOL_AM);
                lcd_symbol_set(LCD_SYMBOL_PM);
                lcd_num_puthex(0x1200, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_MAIL_READ:
            mbox_mail_get(&current_mail, MBOX_GET_PREV);
            if (current_mail != NULL) {
                int_temp = current_mail->size;
                lcd_puta(current_mail->data, int_temp);
                lcd_num_puthex(current_mail->address, LCD_NUM_PADDING_ZERO);
            }
            break;
        case STATE_FW_WAIT_USER_CONFIRMATION:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_AUDIO_FILE_SELECT:
            strncpy_P((char*)search_string, AUDIO_FILE_TYPE, AUDIO_FILE_TYPE_SIZE);
            if (avrraven.status.batt_low == false) {
                sfs_fget(search_string, file_name, SFS_FGET_FWD);
                lcd_puts((const char*)file_name);
            } else {
                lcd_puts_P("ERROR");
            }
            break;
        case STATE_STORAGE_FILE_LIST:
            strncpy_P((char*)search_string, "*.*", 4);
            if (avrraven.status.batt_low == false) {
                sfs_fget(search_string, file_name, SFS_FGET_FWD);
                lcd_puts((const char*)file_name);
            } else {
                lcd_puts_P("ERROR");
            }
            break;
        default:
            // Key events in unknown state is ignored
            break;
        }
        break;
    case KEY_LEFT:
        switch (avrraven.status.state) {
        case STATE_IDLE:
            new_menu_entry = FMENU_NavigateLeft(&MENU_menuHandle);
            break;
        case STATE_CONFIG_CLOCK_MIN:
            {    // Set min
                timendate_clk_t new_time;
                timndate_clock_get(&new_time, avrraven.user_config.unit.clock);
                new_time.min = numedit_buffer;
                new_time.sec = 0;
                timndate_clock_set(new_time, avrraven.user_config.unit.clock);
            }
            
            // Done numeric editing on hours
            numedit_done();
            
            // Start numeric editor on hours
            {
                timendate_clk_t current_time;
                timndate_clock_get(&current_time, avrraven.user_config.unit.clock);
                numedit_buffer = current_time.hour;
                numedit_config_t conf = {
                    .number = &numedit_buffer,
                    .min_value = 0,
                    .max_value = 23,
                    .radix = NUMEDIT_RADIX_DEC,
                    .size = 2,
                    .offset = 2
                };
                numedit_init(conf);
            }
            
            avrraven.status.state = STATE_CONFIG_CLOCK_HOUR;
            break;
        case STATE_AUDIO_FILE_SELECT:
            if (audio_playback_active() == true) {
                audio_playback_stop();
            }
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_NAME:
        case STATE_MAIL_COMPOSE:
            txtedit_event(TXTEDIT_KEY_LEFT);
            break;
        case STATE_CONFIG_RADIO_PANID:
        case STATE_MAIL_COMPOSE_ADDRESS_SET:
            lcd_num_puthex(numedit_event(NUMEDIT_KEY_LEFT), LCD_NUM_PADDING_ZERO);
            break;
        case STATE_CONFIG_UNIT_TEMP:
            break;
        case STATE_CONFIG_UNIT_CLOCK:
            break;
/*        case STATE_MAIL_READ:
            mbox_mail_getnext(&txtmsg);
            if (txtmsg != NULL) {
                int_temp = ((txtmsg->size) < LCD_SEGMENT_COUNT) ? txtmsg->size : LCD_SEGMENT_COUNT;
                mail_read_offset = ((mail_read_offset + LCD_SEGMENT_COUNT) >= txtmsg->size) ? mail_read_offset : (mail_read_offset + LCD_SEGMENT_COUNT);
                lcd_puta(&txtmsg->data[mail_read_offset], int_temp);
            }
            break;*/
        case STATE_FW_WAIT_USER_CONFIRMATION:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        default:
            // Key events in unknown state is ignored
            break;
        }
        break;
    case KEY_RIGHT:
        switch (avrraven.status.state) {
        case STATE_IDLE:
            new_menu_entry = FMENU_NavigateRight(&MENU_menuHandle);
            break;
        case STATE_CONFIG_CLOCK_HOUR:
            {    // Set hour
                timendate_clk_t new_time;
                timndate_clock_get(&new_time, avrraven.user_config.unit.clock);
                new_time.hour = numedit_buffer;
                new_time.sec = 0;
                timndate_clock_set(new_time, avrraven.user_config.unit.clock);
            }
            
            // Done numeric editing on hours
            numedit_done();
            
            // Start numeric editor on minutes
            {
                timendate_clk_t current_time;
                timndate_clock_get(&current_time, avrraven.user_config.unit.clock);
                numedit_buffer = current_time.min;
                numedit_config_t conf = {
                    .number = &numedit_buffer,
                    .min_value = 0,
                    .max_value = 59,
                    .radix = NUMEDIT_RADIX_DEC,
                    .size = 2,
                    .offset = 0
                };
                numedit_init(conf);
            }
            
            avrraven.status.state = STATE_CONFIG_CLOCK_MIN;
            break;
        case STATE_MAIL_COMPOSE:
        case STATE_CONFIG_NAME:
            txtedit_event(TXTEDIT_KEY_RIGHT);
            break;
        case STATE_CONFIG_RADIO_PANID:
        case STATE_MAIL_COMPOSE_ADDRESS_SET:
            lcd_num_puthex(numedit_event(NUMEDIT_KEY_RIGHT), LCD_NUM_PADDING_ZERO);
            break;
        case STATE_CONFIG_UNIT_TEMP:
            break;
        case STATE_CONFIG_UNIT_CLOCK:
            break;
        case STATE_FW_WAIT_USER_CONFIRMATION:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        default:
            // Key events in unknown state is ignored
            break;
        }
        break;
    case KEY_ENTER:
        switch (avrraven.status.state) {
        case STATE_DISPLAY_MESSAGE:
            // Default display avrraven.status.state
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            led_status_set(LED_OFF);
            lcd_num_clr_all();
            
            // Refresh menu function (in case it affects the display)
            menu_action = MENU_ACTION_REFRESH;
            if (current_menu_entry->action != NULL) {
                current_menu_entry->action((void*)&menu_action);
            }
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_LCD_CONTRAST:
            // Done numeric editing, write value to user configuration
            numedit_done();
            avrraven.user_config.lcd.contrast = (lcd_contrast_t)numedit_buffer;
            
            // Update LCD HW 
            config = lcd_config_get();
            config.contrast = avrraven.user_config.lcd.contrast;
            lcd_config_set(config);
            
            // Write new user configuration to EEPROM
            USER_CONFIG_STORE();
            
            // Refresh menu title
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_LCD_SCROLLING:
            // Done numeric editing, write value to user configuration
            avrraven.user_config.lcd.scrolling = (lcd_scrolling_t)numedit_value_get();
            numedit_done();
            
            // Update LCD scrolling settings
            config = lcd_config_get();
            config.scrolling = avrraven.user_config.lcd.scrolling;
            lcd_config_set(config);
            
            // Write new user configuration to EEPROM
            USER_CONFIG_STORE();
            
            // Refresh menu title
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_RADIO_CHANNEL:
            // Done numeric editing, write value to user configuration
            avrraven.user_config.channel = numedit_value_get();
            numedit_done();
            
            // Write new user configuration to EEPROM
            USER_CONFIG_STORE();
            
            // Refresh menu title
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_CLOCK_HOUR:
            {    // Set hour
                timendate_clk_t new_time;
                timndate_clock_get(&new_time, avrraven.user_config.unit.clock);
                new_time.hour = numedit_value_get();
                new_time.sec = 0;
                timndate_clock_set(new_time, avrraven.user_config.unit.clock);
            }
            
            // Done numeric editing on hours
            numedit_done();
            
            // Start show clock event
            menu_action = MENU_ACTION_REFRESH;
            if (current_menu_entry->action != NULL) {
                current_menu_entry->action((void*)&menu_action);
            }
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_CLOCK_MIN:
            {    // Set min
                timendate_clk_t new_time;
                timndate_clock_get(&new_time, avrraven.user_config.unit.clock);
                new_time.min = numedit_value_get();
                new_time.sec = 0;
                timndate_clock_set(new_time, avrraven.user_config.unit.clock);
            }
            
            // Done numeric editing on minutes
            numedit_done();
            
            // Start show clock event
            menu_action = MENU_ACTION_REFRESH;
            if (current_menu_entry->action != NULL) {
                current_menu_entry->action((void*)&menu_action);
            }
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_DEBUG_ENABLE:
            // Write new user configuration to EEPROM
            USER_CONFIG_STORE();
            
            // Refresh menu title
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_RADIO_PANID:
            // Done numeric editing on hours
            numedit_done();
            
            // Write new user configuration to EEPROM
            USER_CONFIG_STORE();
            
            // Refresh menu title
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_UNIT_TEMP:
        case STATE_CONFIG_UNIT_CLOCK:
        case STATE_CONFIG_TXTPREV_ENABLE:
        case STATE_CONFIG_FW_UPGRADE:
        case STATE_CONFIG_CONNECT:
        case STATE_CONFIG_IMG_RCV:
            // Write new user configuration to EEPROM
            USER_CONFIG_STORE();
            
            // Refresh menu title
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_MAIL_PREVIEW:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            menu_action = MENU_ACTION_REFRESH;
            if (current_menu_entry->action != NULL) {
                current_menu_entry->action((void*)&menu_action);
            }
            led_status_set(LED_OFF);
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_AUDIO_FILE_SELECT:
            if (audio_playback_active() == false) {
                audio_playback_start(file_name, false);
            } else {
                audio_playback_stop();
            }
            break;
        case STATE_STORAGE_FILE_LIST:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_FW_WAIT_USER_CONFIRMATION:
            avrraven.status.state = STATE_FW_WRITE_INIT;
            break;
        case STATE_IDLE:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            if (current_menu_entry->action != NULL) {
                current_menu_entry->action((void*)&menu_action);
            }
            break;
        case STATE_AUDIO_RECORD:
            audio_record_stop();
            lcd_symbol_clr(LCD_SYMBOL_MIC);
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_CONFIG_NAME:
            txtedit_event(TXTEDIT_KEY_ENTER);
            txtedit_done();
            USER_CONFIG_STORE();
            // Print the menu title to the display
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_MAIL_READ:
            lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(current_menu_entry));
            // If any unread mail, show closed envelope
            if (mbox_unread() == true) {
                lcd_symbol_set(LCD_SYMBOL_ENV_MAIN);
                lcd_symbol_set(LCD_SYMBOL_ENV_CL);
                lcd_symbol_clr(LCD_SYMBOL_ENV_OP);
            // If all mail read, show open envelope
            } else {
                lcd_symbol_set(LCD_SYMBOL_ENV_MAIN);
                lcd_symbol_set(LCD_SYMBOL_ENV_CL);
                lcd_symbol_set(LCD_SYMBOL_ENV_OP);
            }
            // show number of mails on display
            int_temp = mbox_mail_count_get();
            lcd_num_putdec(int_temp, LCD_NUM_PADDING_SPACE);
            mbox_close();
            avrraven.status.state = STATE_IDLE;
            break;
        case STATE_MAIL_COMPOSE:
            txtedit_event(TXTEDIT_KEY_ENTER);
            txtedit_done();
            mbox_mail_buffer_put(new_mail_buffer);
            {
                lcd_puts_P("ADDRESS");
                    numedit_config_t conf = {
                        .number = &numedit_buffer,
                        .min_value = 0,
                        .max_value = UINT16_MAX,
                        .radix = NUMEDIT_RADIX_DEC,
                        .size = 1,
                        .offset = 0
                    };
                numedit_init(conf);
            }
            numedit_buffer = 0x0000; // default address
            lcd_num_puthex(numedit_buffer, LCD_NUM_PADDING_ZERO);
            avrraven.status.state = STATE_MAIL_COMPOSE_ADDRESS_SET;
            break;
        case STATE_MAIL_COMPOSE_ADDRESS_SET:
            avrraven.status.trans_seq_nmbr = mbox_mail_send(numedit_value_get(), txtedit_size_get());
            avrraven.status.state = STATE_MAIL_SEND_INIT;
            numedit_done();
            mbox_close();
            break;
        default:
            lcd_puts_P("CANCELED");
            led_status_set(LED_FAST_BLINK);
            lcd_num_clr_all();
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
            break;
        }
        break;
    case KEY_NO_KEY:
        // Do nothing?
        break;
    default:
            // Key events in unknown state is ignored
        break;
    }
    
    // Navigate menu if new menu entry is different from last
    if (new_menu_entry != current_menu_entry) {
        navigate_menu(current_menu_entry, new_menu_entry, menu_action);
    }
    
    // After processing the key event it is safe to enable auto key
    auto_key_state = key_state;
    if (key_state != KEY_NO_KEY) {
        if (auto_key_registered == false) {
            long delay;
            if (auto_key_wait == true) {
                delay = 300;
                auto_key_wait = false;
            } else {
                asy_tmr_put(auto_key_timer);
                delay = 80;
                auto_key_registered = true;
            }
            auto_key_timer = asy_tmr_get(int_evt_key, (void*)&auto_key_state, delay);
        }
    } else {
        asy_tmr_put(auto_key_timer);
        auto_key_wait = true;
        auto_key_registered = false;
    }
}

void int_evt_check_key(void* evt)
{
    static bool first_time = true;          // Static variable used as flag to indicate first execution of function
    static key_state_t last_key_state;      // Variable hodling last value of the joystick. Used to detect changes in joystick state
    
    /* Check joystick state. Post event if any change since last
     * First time function is executed no event is posted. This is
     * done to not respond if joystick activated while booting
     */
    key_state_t new_key_state;
    if (first_time) {
        last_key_state = key_state_get();
        new_key_state = last_key_state;
        first_time = false;
    } else if((new_key_state = key_state_get()) != last_key_state){
        last_key_state = new_key_state;
        vrt_post_event(int_evt_key, (void*)&last_key_state);
    }
}

void int_evt_lcd_shift_left(void* evt)
{
    lcd_text_shift_left();
}

void int_evt_lcd_cursor_toggle(void* evt)
{
    lcd_cursor_toggle();
}

void int_evt_lcd_num_refresh(void* evt)
{
    lcd_num_refresh();
}

void int_evt_update_batt(void* evt)
{
    int16_t vcc_batt = battery_voltage_read();
    
    lcd_symbol_battery_empty();
    if (vcc_batt>2000) {
        lcd_symbol_set(LCD_SYMBOL_BAT_CAP1);
    }
    if (vcc_batt>2500) {
        lcd_symbol_set(LCD_SYMBOL_BAT_CAP2);
    }
    if (vcc_batt>2800) {
        lcd_symbol_set(LCD_SYMBOL_BAT_CAP3);
    }
    
    // Indicate low battery if voltage less than 2.6 V
    if (avrraven.status.batt_low == false) {
        if (vcc_batt<2600) {
            avrraven.status.batt_low = true;
            lcd_symbol_set(LCD_SYMBOL_ATT);
            mbox_deinit();
            sfs_deinit();
        }
    }
}

void int_evt_show_clock(void* evt)
{
    static bool col = false;
    timendate_clk_t clk;
    timndate_clock_get(&clk, avrraven.user_config.unit.clock);
    int16_t display_clock = clk.hour*100 + clk.min;
    lcd_num_putdec(display_clock, LCD_NUM_PADDING_ZERO);
    if ((col = !col) == true){
        lcd_symbol_set(LCD_SYMBOL_COL);
    } else {
        lcd_symbol_clr(LCD_SYMBOL_COL);
    }
}

void int_evt_toggle_num_digit(void* evt)
{
    int8_t digit_mask = *(int8_t*)evt;
    lcd_num_enable(digit_mask);
}

void int_evt_audio(void* evt)
{
    audio_event_handler();
}
//! @}
/*EOF*/
