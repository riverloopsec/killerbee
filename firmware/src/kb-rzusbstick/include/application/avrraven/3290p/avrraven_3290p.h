// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  Definitions for the m3290 MCU on AVR Raven
*
*
*
* \par Application note:
*      AVR2017: RZRAVEN FW
*
* \par Documentation
*
* \author
*      Atmel Corporation: http://www.atmel.com \n
*      Support email: avr@atmel.com
*
* $Id: avrraven_3290p.h 41763 2008-05-15 18:08:50Z hmyklebust $
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __AVRRAVEN_3290P_H__
#define __AVRRAVEN_3290P_H__
/*========================= INCLUDES                 =========================*/
#include "temp.h"
#include "timndate.h"
#include "mmap.h"
#include "df.h"
#include "vrt_kernel.h"
#include "asy_tmr.h"
#include "lcd.h"

/*========================= MACROS                   =========================*/
//!@{ Macros for the AVR Raven node name
#define AVRRAVEN_NAME_SIZE_MAX              (10)
#define AVRRAVEN_DEFAULT_NAME               "AVR RAVEN"
#define AVRRAVEN_DEFAULT_NAME_SIZE          (sizeof(AVRRAVEN_DEFAULT_NAME))
#define AVRRAVEN_DEFAULT_NAME_BUFFER_SIZE   (AVRRAVEN_DEFAULT_NAME_SIZE+1)
//!@}


//!@{ User configuration macros

#define USER_CONFIG_STORE() {\
    uint8_t* data_ptr = (uint8_t*)&avrraven.user_config;\
    uint16_t eeadr = USER_CONFIG_EEADR;\
    for (int16_t i=0; i<sizeof(avrraven.user_config); i++) {\
        sp_write_eeprom(eeadr++, *(data_ptr++));\
    };\
}

#define USER_CONFIG_LOAD() {\
    uint8_t* data_ptr = (uint8_t*)&avrraven.user_config;\
    uint16_t eeadr = USER_CONFIG_EEADR;\
    for (int16_t i=0; i<sizeof(avrraven.user_config); i++) {\
        *(data_ptr++) = sp_read_eeprom(eeadr++);\
    };\
}


#define USER_CONFIG_BASIC_VALID_ID      0xCAFEBABE
#define USER_CONFIG_EXTENDED_VALID_ID   0xCAFEBABE

#define USER_CONFIG_BASIC_VALID() (avrraven.user_config.basic_valid == USER_CONFIG_BASIC_VALID_ID)
#define USER_CONFIG_EXTENDED_VALID() (avrraven.user_config.extended_valid == USER_CONFIG_EXTENDED_VALID_ID)
//!@}

//! Number identifying the node as a AVR Raven
#define AVRRAVEN_TYPE               1

//! Number of revisions retured as respons to RVN_OTA_CMD_SIGN_ON
#define AVRRAVEN_REV_SIZE           9

//! Current hardware revision
#define AVRRAVEN_HW_REV             REVISION_TO_INT16(2,0)

#define NTWK_ADDRESS_INVALID 0xFFFF

/*========================= TYEPDEFS                 =========================*/
//! States
typedef enum {
    STATE_STARTUP = 0,
    
    STATE_RADIO_CONNECTING,
    STATE_RADIO_DISCONNECTING,
    STATE_RADIO_WAIT_FOR_RX_ENABLE_RESPONSE,
    STATE_RADIO_WAIT_FOR_RX_REENABLE_RESPONSE,
    STATE_RADIO_GET_ADDRESS_WAIT_RESPONSE,
    STATE_RADIO_RX_DISABLE,
    STATE_RADIO_GET_PARAM,
    
    STATE_OTA_TRANSACTION_WAITING_FOR_RESPONSE,
    STATE_OTA_TRANS_WAITING_FOR_RX_ENABLE_RESPONSE,
    
    STATE_IDLE,
    STATE_DISPLAY_MESSAGE,
    
    STATE_MAIL_READ,
    STATE_MAIL_COMPOSE,
    STATE_MAIL_COMPOSE_ADDRESS_SET,
    STATE_MAIL_SEND_INIT,
    STATE_MAIL_SENDING,
    STATE_MAIL_SENT,
    STATE_MAIL_PREVIEW,
    
    STATE_CONFIG_LCD_CONTRAST,
    STATE_CONFIG_LCD_SCROLLING,
    STATE_CONFIG_NAME,
    STATE_CONFIG_UNIT_TEMP,
    STATE_CONFIG_UNIT_CLOCK,
    STATE_CONFIG_RADIO_CHANNEL,
    STATE_CONFIG_RADIO_PANID,
    STATE_CONFIG_DEBUG_ENABLE,
    STATE_CONFIG_TXTPREV_ENABLE,
    STATE_CONFIG_FW_UPGRADE,
    STATE_CONFIG_CONNECT,
    STATE_CONFIG_IMG_RCV,
    STATE_CONFIG_CLOCK_HOUR,
    STATE_CONFIG_CLOCK_MIN,
    
    STATE_AUDIO_FILE_SELECT,
    STATE_AUDIO_PLAYBACK,
    STATE_AUDIO_RECORD,
    
    STATE_STORAGE_FILE_LIST,
        
    STATE_FW_WRITE_INIT,
    STATE_FW_WRITE_INIT_WAIT_RX_ENABLE_RESPONSE,
    
    STATE_FW_WRITE,
    STATE_FW_WRITE_WAIT_TX_DONE,
    STATE_FW_WRITE_WAIT_RX_ENABLE_RESPONSE,
    
    STATE_FW_WRITE_IMAGE,
    
    STATE_FW_WAIT_USER_CONFIRMATION,
    
    STATE_FW_WRITE_COMPLETE_TX_DONE,
    STATE_FW_WRITE_COMPLETE_WAIT_TX_DONE,
    STATE_FW_WRITE_COMPLETE_WAIT_RX_ENABLE_RESPONSE,
    STATE_FW_WRITE_COMPLETE_WAIT_RADIO,
    STATE_FW_WRITTEN_WAIT_RX_ENABLE_RESPONSE,
    STATE_FW_WRITE_COMPLETE_DISCONNECTING_FROM_NWK,
    
    STATE_M1284P_DISCONNECT_WAIT_RESPOND,
    STATE_M1284P_UPGRADE_INIT_WAIT_RESPOND,
    STATE_M1284P_UPGRADE_PACKET_WAIT_RESPOND,
    STATE_M1284P_UPGRADE_DONE_WAIT_RESPOND,
    
    STATE_M1284P_RESTART_GET_REV,
    STATE_M1284P_ENTER_BOOT,
    STATE_M1284P_FW_UPGRADE,
    STATE_M1284P_FW_UPGRADE_DONE,
    STATE_M1284P_START_NEW_FW,
    STATE_M1284P_RESTART_WAIT_BOOT_EVENT,
    STATE_M1284P_ENTER_BOOT_WAIT_BOOT_EVENT,
    STATE_M1284P_RESTART_WAIT_APP_EVENT,
    STATE_M1284P_NEW_FW_WAIT_EVENT,
}state_t;

//! Type containing user configuration
typedef struct {
    uint32_t basic_valid;
    uint16_t panid;
    int8_t channel;
    struct {
        temp_unit_t temp;
        timndate_clock_unit_t clock;
    } unit;
    bool txtmsg_preview;
    bool fw_upgr_auto;
    bool fw_rcv_auto;
    bool join_auto;
    bool show_hidden_files;
    bool show_system_files;
    bool debug_enable;
    bool data_flash_valid;
    struct {
        lcd_contrast_t contrast;
        lcd_scrolling_t scrolling;
    } lcd;
    uint8_t name[AVRRAVEN_NAME_SIZE_MAX];
    uint32_t extended_valid;
} avrraven_user_config_t;

//! Type holding avrraven status
typedef struct {
    bool batt_low;
    uint16_t ntwk_address;
    
    state_t state;

    uint32_t m1284p_address_offset;
    df_adr_t m1284p_img_offset;
    
    uint16_t m1284p_app_rev;
    uint16_t m1284p_bl_rev;

    uint16_t m3290p_app_rev;
    uint16_t m3290p_bl_rev;
    
    uint8_t trans_seq_nmbr;

} avrraven_status_t;


//! Type holding user configuration and status
typedef struct {
    avrraven_user_config_t user_config;
    avrraven_status_t status;
} avrraven_t;


/*========================= PUBLIC VARIABLES         =========================*/
//! Struct holding user configuration and status
extern avrraven_t avrraven;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  System reboot
 *
 *              The watchdog timer is used to give a hard reset
 */
void reboot(void);


#endif // __AVRRAVEN_3290P_H__

/*EOF*/

