
// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  menu_definitions.c Auto_generated file - do not edit!
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
 * $Id: menu_definitions.c 41737 2008-05-15 12:26:47Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#include <stdint.h>
#include "menu.h"
#include "menu_data.h"

/*----------------------------------------------------------------------------
 Defines (used for initialisation of a menu).
----------------------------------------------------------------------------*/
#define MENU_NUMBER_OF_MENU_ITEMS (35)		//!< Declaration of number of items in the DB101 menu.

/*----------------------------------------------------------------------------
 Menu item titles.
----------------------------------------------------------------------------*/
PROGMEM_DECLARE(unsigned char) MENU_title00[]=""; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title01[]="NETWORK"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title02[]="JOIN"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title03[]="LEAVE"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title04[]="MAIL"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title05[]="INBOX"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title06[]="COMPOSE"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title07[]="AUDIO"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title08[]="PLAY"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title09[]="RECORD"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title10[]="SENSORS"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title11[]="TEMP"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title12[]="TIME"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title13[]="CLOCK"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title14[]="SYSTEM"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title15[]="CONFIG"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title16[]="LCD"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title17[]="CONTRST"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title18[]="SCROLLING"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title19[]="NAME"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title20[]="UNITS"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title21[]="TEMP"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title22[]="RADIO"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title23[]="PAN ID"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title24[]="CHANNEL"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title25[]="CLOCK"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title26[]="CONNECT"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title27[]="DEBUG"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title28[]="TXTPREV"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title29[]="STORAGE"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title30[]="FILES"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title31[]="FORMAT"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title32[]="FW"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title33[]="UPGRADE"; //!< String used by menu.
PROGMEM_DECLARE(unsigned char) MENU_title34[]="REBOOT"; //!< String used by menu.

/*----------------------------------------------------------------------------
 !\brief MENU menu stored in flash
----------------------------------------------------------------------------*/
PROGMEM_DECLARE(FMENU_item_t) MENU_menuItems[MENU_NUMBER_OF_MENU_ITEMS] = {
	{ &MENU_menuItems[14], &MENU_menuItems[ 1], &MENU_menuItems[ 0], &MENU_menuItems[ 0], MENU_title00, avrraven_name, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 0], &MENU_menuItems[ 4], &MENU_menuItems[ 1], &MENU_menuItems[ 2], MENU_title01, NULL, {0,0}},
	{ &MENU_menuItems[ 3], &MENU_menuItems[ 3], &MENU_menuItems[ 1], &MENU_menuItems[ 2], MENU_title02, network_join, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 2], &MENU_menuItems[ 2], &MENU_menuItems[ 1], &MENU_menuItems[ 3], MENU_title03, network_leave, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 1], &MENU_menuItems[ 7], &MENU_menuItems[ 4], &MENU_menuItems[ 5], MENU_title04, NULL, {0,0}},
	{ &MENU_menuItems[ 6], &MENU_menuItems[ 6], &MENU_menuItems[ 4], &MENU_menuItems[ 5], MENU_title05, mail_inbox, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 5], &MENU_menuItems[ 5], &MENU_menuItems[ 4], &MENU_menuItems[ 6], MENU_title06, mail_compose, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 4], &MENU_menuItems[10], &MENU_menuItems[ 7], &MENU_menuItems[ 8], MENU_title07, NULL, {0,0}},
	{ &MENU_menuItems[ 9], &MENU_menuItems[ 9], &MENU_menuItems[ 7], &MENU_menuItems[ 8], MENU_title08, audio_playback, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 8], &MENU_menuItems[ 8], &MENU_menuItems[ 7], &MENU_menuItems[ 9], MENU_title09, audio_record, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[ 7], &MENU_menuItems[12], &MENU_menuItems[10], &MENU_menuItems[11], MENU_title10, NULL, {0,0}},
	{ &MENU_menuItems[11], &MENU_menuItems[11], &MENU_menuItems[10], &MENU_menuItems[11], MENU_title11, read_temp, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[10], &MENU_menuItems[14], &MENU_menuItems[12], &MENU_menuItems[13], MENU_title12, NULL, {0,0}},
	{ &MENU_menuItems[13], &MENU_menuItems[13], &MENU_menuItems[12], &MENU_menuItems[13], MENU_title13, time_clock, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[12], &MENU_menuItems[ 0], &MENU_menuItems[14], &MENU_menuItems[15], MENU_title14, NULL, {0,0}},
	{ &MENU_menuItems[34], &MENU_menuItems[29], &MENU_menuItems[14], &MENU_menuItems[16], MENU_title15, NULL, {FIRST_IN_MENU, 0}},
	{ &MENU_menuItems[28], &MENU_menuItems[19], &MENU_menuItems[15], &MENU_menuItems[17], MENU_title16, NULL, {FIRST_IN_MENU, 0}},
	{ &MENU_menuItems[18], &MENU_menuItems[18], &MENU_menuItems[16], &MENU_menuItems[17], MENU_title17, config_lcd_contrast, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[17], &MENU_menuItems[17], &MENU_menuItems[16], &MENU_menuItems[18], MENU_title18, config_lcd_scroll, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[16], &MENU_menuItems[20], &MENU_menuItems[15], &MENU_menuItems[19], MENU_title19, config_name, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[19], &MENU_menuItems[22], &MENU_menuItems[15], &MENU_menuItems[21], MENU_title20, NULL, {0,0}},
	{ &MENU_menuItems[21], &MENU_menuItems[21], &MENU_menuItems[20], &MENU_menuItems[21], MENU_title21, config_unit_temp, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[20], &MENU_menuItems[25], &MENU_menuItems[15], &MENU_menuItems[23], MENU_title22, NULL, {0,0}},
	{ &MENU_menuItems[24], &MENU_menuItems[24], &MENU_menuItems[22], &MENU_menuItems[23], MENU_title23, config_radio_panid, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[23], &MENU_menuItems[23], &MENU_menuItems[22], &MENU_menuItems[24], MENU_title24, config_radio_channel, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[22], &MENU_menuItems[26], &MENU_menuItems[15], &MENU_menuItems[25], MENU_title25, config_clock, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[25], &MENU_menuItems[27], &MENU_menuItems[15], &MENU_menuItems[26], MENU_title26, config_connect, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[26], &MENU_menuItems[28], &MENU_menuItems[15], &MENU_menuItems[27], MENU_title27, config_debug, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[27], &MENU_menuItems[16], &MENU_menuItems[15], &MENU_menuItems[28], MENU_title28, config_txtprev, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[15], &MENU_menuItems[32], &MENU_menuItems[14], &MENU_menuItems[30], MENU_title29, NULL, {0,0}},
	{ &MENU_menuItems[31], &MENU_menuItems[31], &MENU_menuItems[29], &MENU_menuItems[30], MENU_title30, storage_files, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[30], &MENU_menuItems[30], &MENU_menuItems[29], &MENU_menuItems[31], MENU_title31, storage_format, {0 , SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[29], &MENU_menuItems[34], &MENU_menuItems[14], &MENU_menuItems[33], MENU_title32, NULL, {0,0}},
	{ &MENU_menuItems[33], &MENU_menuItems[33], &MENU_menuItems[32], &MENU_menuItems[33], MENU_title33, system_fw_upgrade, {FIRST_IN_MENU, SPECIAL_FUNC_MENU_ITEM}},
	{ &MENU_menuItems[32], &MENU_menuItems[15], &MENU_menuItems[14], &MENU_menuItems[34], MENU_title34, system_reboot, {0 , SPECIAL_FUNC_MENU_ITEM}}
};

FMENU_handle_t MENU_menuHandle = {
	MENU_menuItems,
	MENU_menuItems,
	MENU_NUMBER_OF_MENU_ITEMS
	};
