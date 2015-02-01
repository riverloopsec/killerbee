// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Functions called on joystick activity
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
 * $Id: menu_funcs.c 41767 2008-05-15 18:53:24Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "menu_funcs.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"
#include "vrt_kernel.h"
#include "vrt_mem.h"

#include "rvn_ota.h"
#include "rvn_loc.h"

#include "loc.h"
#include "int_evt.h"
#include "sipc.h"
#include "lcd.h"
#include "macros.h"
#include "bl.h"
#include "mmap.h"
#include "timndate.h"
#include "df.h"
#include "mbox.h"
#include "txtedit.h"
#include "numedit.h"
#include "audio.h"
#include "battery.h"

#include "key.h"
#include "led.h"
#include "sfs.h"
#include "tick.h"

#include "avrraven_3290p.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/
#define lcd_unit_temp_clr() lcd_symbol_clr(LCD_SYMBOL_F); lcd_symbol_clr(LCD_SYMBOL_C)
#define lcd_unit_clock_clr() lcd_symbol_clr(LCD_SYMBOL_AM); lcd_symbol_clr(LCD_SYMBOL_PM); lcd_num_clr_all()

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Display the current user configured name on the display
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void avrraven_name(void* arg);

/*! \brief  Display the current ATmega3290p application firmware revision
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void system_fw_rev(void* arg);

/*! \brief  Start a full firmware upgrade on both MCU's
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void system_fw_upgrade(void* arg);

/*! \brief  Power down system. The Raven is waked by pushing joystick center
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void system_poweroff(void* arg);

/*! \brief  System soft-boot
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void system_reboot(void* arg);

/*! \brief  Opens the mailbox.
 *
 *              Count mails and number of un-read mails and displays the number
 *              on the numeric display. The envelope symbol is closed if any
 *              un-read mail is detected on the data flash
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void mail_inbox(void* arg);

/*! \brief  Start composing a new mail
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void mail_compose(void* arg);

/*! \brief  Set new contrast of the LCD display.
 *
 *              The new constrast is stored in the user configuratino struct in EEPROM
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_lcd_contrast(void* arg);

/*! \brief  
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_name(void* arg);

/*! \brief  Set new node name
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_unit_temp(void* arg);

/*! \brief  Set radio PAN-ID
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_radio_panid(void* arg);

/*! \brief  Set radio channel
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_radio_channel(void* arg);

/*! \brief  Enable/disable autoconnect at startup
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_connect(void* arg);

/*! \brief  Set new time (clock)
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_clock(void* arg);

/*! \brief  Enable/disable debug interface. Must be disabled to show correct temperature
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_debug(void* arg);

/*! \brief  Configure preview of texmessages
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void config_txtprev(void* arg);

/*! \brief  Show current time in the display
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void time_clock(void* arg);

/*! \brief  Join the network.
 *
 *              The current stored PAN-ID and channel is used.
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void network_join(void* arg);

/*! \brief  Leave the network
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void network_leave(void* arg);

/*! \brief  Read current temperature and show it on the display
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void read_temp(void* arg);

/*! \brief  Start an audio playback
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void audio_playback(void* arg);

/*! \brief  Start an audio recording
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void audio_record(void* arg);

/*! \brief  Format the entire application area of the data flash
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void storage_format(void* arg);

/*! \brief  Display the number of files on the data flash.
 *
 *              On joystick enter, a list of all file names is displayed.
 *
 *  \param[in]  arg     The action (type \ref menu_action_t) activating the function
 */
extern void storage_files(void* arg);


/*========================= PRIVATE FUNCTIONS        =========================*/
//! Display current temperature unit
static void lcd_unit_temp_put(void);

/*========================= IMPLEMENTATION           =========================*/
//! Pointer to mail reading buffer (returned by \ref mbox_mail_get())
extern mbox_mail_t* current_mail;

void avrraven_name(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        lcd_puts((const char*)avrraven.user_config.name);
        if (avrraven.status.ntwk_address != NTWK_ADDRESS_INVALID) {
            lcd_num_puthex(avrraven.status.ntwk_address, LCD_NUM_PADDING_ZERO);
        }
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
    case MENU_ACTION_REFRESH:
        lcd_puts((const char*)avrraven.user_config.name);
        if (avrraven.status.ntwk_address != NTWK_ADDRESS_INVALID) {
            lcd_num_puthex(avrraven.status.ntwk_address, LCD_NUM_PADDING_ZERO);
        }
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void system_fw_upgrade(void* arg)
{
    rvn_loc_cmd_std_t cmd;
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        // Set ATmega1284p in boot loader mode
        led_status_set(LED_FAST_BLINK);
        lcd_puts_P("WRITING M1284 IMAGE");
        cmd.id = RVN_LOC_CMD_RADIO_DISCONNECT;
        sipc_send_frame(sizeof(cmd), (uint8_t *)&cmd);
        avrraven.status.state = STATE_M1284P_DISCONNECT_WAIT_RESPOND;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void system_reboot(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        BLCC_WRITE(BLCC_RESTART_REQUEST_FROM_APP);
        reboot();
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void mail_inbox(void* arg)
{
    int mail_count;
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        if (mbox_open() != EOF) {
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
            mail_count = mbox_mail_count_get();
            lcd_num_putdec(mail_count, LCD_NUM_PADDING_SPACE);
            mbox_close();
        }
        break;
    case MENU_ACTION_EXIT:
        // Remove mail box symbols
        lcd_num_clr_all();
        lcd_symbol_clr(LCD_SYMBOL_ENV_MAIN);
        lcd_symbol_clr(LCD_SYMBOL_ENV_CL);
        lcd_symbol_clr(LCD_SYMBOL_ENV_OP);
        break;
    case MENU_ACTION_ENTER:
        if (mbox_open() != EOF) {
            lcd_symbol_set(LCD_SYMBOL_ENV_MAIN);
            lcd_symbol_set(LCD_SYMBOL_ENV_CL);
            lcd_symbol_set(LCD_SYMBOL_ENV_OP);
        
            mbox_mail_get(&current_mail, MBOX_GET_NEXT);
            if (current_mail != NULL) {
                int int_temp = current_mail->size;
                lcd_puta(current_mail->data, int_temp);
                lcd_num_puthex(current_mail->address, LCD_NUM_PADDING_ZERO);
            } else {
                lcd_puts_P("EMPTY");
                avrraven.status.state = STATE_DISPLAY_MESSAGE;
            }
            // set new avrraven.status.state
            avrraven.status.state = STATE_MAIL_READ;
        } else {
            lcd_puts_P("ERROR");
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
            break;
        }
        break;
    default:
        break;
    }
}

void mail_compose(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        if (mbox_open() == EOF) {
            lcd_puts_P("ERROR"); // TIMED_SEQUENCE
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
        } else if ((new_mail_buffer = mbox_mail_buffer_get(30)) != NULL) {
            new_mail_buffer[0] = '\0';
            if (txtedit_init(new_mail_buffer, 30) == EOF) {
                lcd_puts_P("ERROR"); // TIMED_SEQUENCE
                avrraven.status.state = STATE_DISPLAY_MESSAGE;
            } else {
                avrraven.status.state = STATE_MAIL_COMPOSE;
            }
        } else {
            lcd_puts_P("ERROR"); // TIMED_SEQUENCE
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
        }
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_lcd_contrast(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        lcd_num_putdec((int)avrraven.user_config.lcd.contrast, LCD_NUM_PADDING_SPACE);
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        numedit_buffer = avrraven.user_config.lcd.contrast;
        {
            numedit_config_t conf = {
                .number = &numedit_buffer,
                .min_value = 0,
                .max_value = 15,
                .radix = NUMEDIT_RADIX_DEC,
                .size = 2,
                .offset = 0
            };
            numedit_init(conf);
        }
        avrraven.status.state = STATE_CONFIG_LCD_CONTRAST;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_lcd_scroll(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        lcd_num_putdec((int)avrraven.user_config.lcd.scrolling, LCD_NUM_PADDING_SPACE);
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        numedit_buffer = avrraven.user_config.lcd.scrolling;
        {
            numedit_config_t conf = {
                .number = &numedit_buffer,
                .min_value = LCD_SCROLLING_FAST,
                .max_value = LCD_SCROLLING_SLOW,
                .radix = NUMEDIT_RADIX_DEC,
                .size = 1,
                .offset = 0
            };
            numedit_init(conf);
        }
        avrraven.status.state = STATE_CONFIG_LCD_SCROLLING;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_name(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        txtedit_init(avrraven.user_config.name, AVRRAVEN_NAME_SIZE_MAX);
        lcd_puts((const char*)avrraven.user_config.name);
        avrraven.status.state = STATE_CONFIG_NAME;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_unit_temp(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_REFRESH:
    case MENU_ACTION_ENTRY:
        lcd_unit_temp_put();
        break;
    case MENU_ACTION_EXIT:
        lcd_unit_temp_clr();
        break;
    case MENU_ACTION_ENTER:
        avrraven.status.state = STATE_CONFIG_UNIT_TEMP;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_radio_panid(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        lcd_symbol_set(LCD_SYMBOL_PAN);
        lcd_num_puthex(avrraven.user_config.panid, LCD_NUM_PADDING_ZERO);
        break;
    case MENU_ACTION_EXIT:
        lcd_symbol_clr(LCD_SYMBOL_PAN);
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        {
            numedit_config_t conf = {
                .number = &avrraven.user_config.panid,
                .min_value = INT16_MIN,
                .max_value = INT16_MAX,
                .radix = NUMEDIT_RADIX_HEX,
                .size = 1,
                .offset = 0
            };
            numedit_init(conf);
        }
        avrraven.status.state = STATE_CONFIG_RADIO_PANID;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_radio_channel(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        lcd_num_putdec((int)avrraven.user_config.channel, LCD_NUM_PADDING_SPACE);
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        numedit_buffer = avrraven.user_config.channel;
        {
            numedit_config_t conf = {
                .number = &numedit_buffer,
                .min_value = 0,
                .max_value = 26,
                .radix = NUMEDIT_RADIX_DEC,
                .size = 2,
                .offset = 0
            };
            numedit_init(conf);
        }
        avrraven.status.state = STATE_CONFIG_RADIO_CHANNEL;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_connect(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        if (avrraven.user_config.join_auto == true) {
            lcd_puts_P("AUTO");
        } else {
            lcd_puts_P("MANUAL");
        }
        avrraven.status.state = STATE_CONFIG_CONNECT;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_clock(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    timendate_clk_t current_time;
    int display_time;
    static asy_tmr_t clock_colon_timer = ASY_TMR_NO_TIMER;
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        int_evt_show_clock((void*)NULL);
        clock_colon_timer = asy_tmr_get(int_evt_show_clock, (void*)NULL, 500);
        break;
    case MENU_ACTION_EXIT:
        asy_tmr_put(clock_colon_timer);
        lcd_symbol_clr(LCD_SYMBOL_COL);
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        asy_tmr_put(clock_colon_timer);
        lcd_symbol_set(LCD_SYMBOL_COL);
        
        // read current time
        timndate_clock_get(&current_time, avrraven.user_config.unit.clock);
        numedit_buffer = current_time.hour;
        
        // Init numeric editor
        {
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
        
        // Display current time (for editing)
        display_time = current_time.hour*100 + current_time.min; // hours from current editing
        lcd_num_putdec(display_time, LCD_NUM_PADDING_ZERO);
        
        avrraven.status.state = STATE_CONFIG_CLOCK_HOUR;
        break;
    case MENU_ACTION_REFRESH:
        int_evt_show_clock((void*)NULL);
        asy_tmr_put(clock_colon_timer); // if allready registered
        clock_colon_timer = asy_tmr_get(int_evt_show_clock, (void*)NULL, 500);
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_debug(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        if (avrraven.user_config.debug_enable == true) {
            // Enable the JTAG interface
            MCUCR &= ~(1<<JTD);
            MCUCR &= ~(1<<JTD);
        } else {
            // Disable the JTAG interface
            MCUCR |= (1<<JTD);
            MCUCR |= (1<<JTD);
        }
        break;
    case MENU_ACTION_ENTER:
        if (avrraven.user_config.debug_enable == true) {
            lcd_puts_P("ENABLE");
        } else {
            lcd_puts_P("DISABLE");
        }
        avrraven.status.state = STATE_CONFIG_DEBUG_ENABLE;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void config_txtprev(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        if (avrraven.user_config.txtmsg_preview == true) {
            lcd_puts_P("ENABLE");
        } else {
            lcd_puts_P("DISABLE");
        }
        avrraven.status.state = STATE_CONFIG_TXTPREV_ENABLE;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void time_clock(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    static asy_tmr_t clock_colon_timer = ASY_TMR_NO_TIMER;
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        int_evt_show_clock((void*)NULL);
        clock_colon_timer = asy_tmr_get(int_evt_show_clock, (void*)NULL, 500);
        break;
    case MENU_ACTION_EXIT:
        asy_tmr_put(clock_colon_timer);
        lcd_symbol_clr(LCD_SYMBOL_COL);
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void network_join(void* arg)
{
    rvn_loc_cmd_radio_connect_t cmd;
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        if (avrraven.status.ntwk_address != NTWK_ADDRESS_INVALID) {
            lcd_num_puthex(avrraven.user_config.panid, LCD_NUM_PADDING_ZERO);
        }
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        // Enable radio
        cmd.id = RVN_LOC_CMD_RADIO_CONNECT;
        cmd.channel = avrraven.user_config.channel;
        cmd.pan_id = avrraven.user_config.panid;
        sipc_send_frame(sizeof(rvn_loc_cmd_radio_connect_t), (uint8_t *)&cmd);
        lcd_puts_P("JOINING NETWORK");
        led_status_set(LED_SOFT_BLINK);
        lcd_num_clr_all();
        avrraven.status.state = STATE_RADIO_CONNECTING;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void network_leave(void* arg)
{
    rvn_loc_cmd_std_t cmd;
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        if (avrraven.status.ntwk_address != NTWK_ADDRESS_INVALID) {
            lcd_num_puthex(avrraven.user_config.panid, LCD_NUM_PADDING_ZERO);
        }
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        // Disable radio
        cmd.id = RVN_LOC_CMD_RADIO_DISCONNECT;
        sipc_send_frame(sizeof(rvn_loc_cmd_std_t), (uint8_t *)&cmd);
        lcd_puts_P("LEAVING NETWORK");
        avrraven.status.state = STATE_RADIO_DISCONNECTING;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void read_temp(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        // TIMED_FUNCTION
        lcd_num_putdec((int)temp_get(avrraven.user_config.unit.temp), LCD_NUM_PADDING_SPACE);
        lcd_unit_temp_put();
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        lcd_symbol_clr(LCD_SYMBOL_MINUS);
        lcd_unit_temp_clr();
        break;
    case MENU_ACTION_ENTER:
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void audio_playback(void* arg)
{
    uint8_t filename[SFS_MAX_FILENAME_BUFFER_SIZE];
    int file_count;
    unsigned char search_string[AUDIO_FILE_TYPE_BUFFER_SIZE];
    menu_action_t menu_action = *(menu_action_t*)arg;
    strncpy_P((char*)search_string, AUDIO_FILE_TYPE, AUDIO_FILE_TYPE_SIZE);
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        if (avrraven.status.batt_low == false) {
            file_count = sfs_fcount(search_string);
        } else {
            file_count = 0;
        }
        lcd_num_putdec(file_count, LCD_NUM_PADDING_SPACE);
        break;
    case MENU_ACTION_EXIT:
        lcd_symbol_clr(LCD_SYMBOL_TONE);
        lcd_symbol_clr(LCD_SYMBOL_SPEAKER);
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        if (avrraven.status.batt_low == false) {
            if ((sfs_fget(search_string, filename, SFS_FGET_FWD)) == EOF) {
                lcd_puts_P("EMPTY");
                avrraven.status.state = STATE_DISPLAY_MESSAGE;
            } else {
                lcd_symbol_set(LCD_SYMBOL_SPEAKER);
                lcd_puts((const char*)filename);
                avrraven.status.state = STATE_AUDIO_FILE_SELECT;
            }
        } else {
            lcd_puts_P("LOW BATT");
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
        }
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void audio_record(void* arg)
{
    unsigned char filename[MBOX_NEW_MAIL_SOUND_FILE_NAME_BUFFER_SIZE];
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        strncpy_P((char*)filename, MBOX_NEW_MAIL_SOUND_FILE_NAME, MBOX_NEW_MAIL_SOUND_FILE_NAME_BUFFER_SIZE);
        audio_record_start(filename);
        lcd_symbol_set(LCD_SYMBOL_MIC);
        avrraven.status.state = STATE_AUDIO_RECORD;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void storage_format(void* arg)
{
    menu_action_t menu_action = *(menu_action_t*)arg;
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        break;
    case MENU_ACTION_EXIT:
        break;
    case MENU_ACTION_ENTER:
        if (avrraven.status.batt_low == false) {
            lcd_puts_P("WAIT");
            sfs_format_drive();
            lcd_puts_P("DONE");
        } else {
            lcd_puts_P("ERROR");
        }
        avrraven.status.state = STATE_DISPLAY_MESSAGE;
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void storage_files(void* arg)
{
    uint8_t filename[SFS_MAX_FILENAME_BUFFER_SIZE];
    int file_count;
    unsigned char search_string[4];
    menu_action_t menu_action = *(menu_action_t*)arg;
    strncpy_P((char*)search_string, "*.*", 4);
    
    switch (menu_action) {
    case MENU_ACTION_ENTRY:
        if (avrraven.status.batt_low == false) {
            file_count = sfs_fcount(search_string);
        } else {
            file_count = 0;
        }
        lcd_num_putdec(file_count, LCD_NUM_PADDING_SPACE);
        break;
    case MENU_ACTION_EXIT:
        lcd_num_clr_all();
        break;
    case MENU_ACTION_ENTER:
        if (avrraven.status.batt_low == false) {
            if ((sfs_fget(search_string, filename, SFS_FGET_FWD)) == EOF) {
                lcd_puts_P("EMPTY");
                avrraven.status.state = STATE_DISPLAY_MESSAGE;
            } else {
                lcd_puts((const char*)filename);
                avrraven.status.state = STATE_STORAGE_FILE_LIST;
            }
        } else {
            lcd_puts_P("ERROR");
            avrraven.status.state = STATE_DISPLAY_MESSAGE;
        }
        break;
    default:
        // Igore other menu actions
        break;
    }
}

void navigate_menu(FMENU_item_flash_t* current_menu_entry, FMENU_item_flash_t* new_menu_entry, menu_action_t menu_action)
{
    // run exit function for "current_menu_entry"
    menu_action = MENU_ACTION_EXIT;
    if (current_menu_entry->action != NULL) {
        current_menu_entry->action((void*)&menu_action);
    }
    
    // Print the title to the display
    lcd_puts_P((PROGMEM_DECLARE(const char)*)FMENU_GetTitle(new_menu_entry));
    
    // run enter function for "new_menu_entry" (after printing of the menu title). In this
    // way the menu action may override the default menu tite.
    menu_action = MENU_ACTION_ENTRY;
    if (new_menu_entry->action != NULL) {
        new_menu_entry->action((void*)&menu_action);
    }
    // Save new menu entry
    current_menu_entry = new_menu_entry;
}

static void lcd_unit_temp_put(void)
{
    temp_unit_t temp_unit = avrraven.user_config.unit.temp;
    if (temp_unit == TEMP_UNIT_CELCIUS) {
        lcd_symbol_clr(LCD_SYMBOL_F);
        lcd_symbol_set(LCD_SYMBOL_C);
    } else {
        lcd_symbol_clr(LCD_SYMBOL_C);
        lcd_symbol_set(LCD_SYMBOL_F);
    }
}
//! @}
/*EOF*/
