// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Functions for 
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
 * $Id: menu_funcs.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __MENU_FUNCS_H__
#define __MENU_FUNCS_H__

/*========================= INCLUDES                 =========================*/
#include "menu.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/
//! Type for defining menu action when calling menu functions
typedef enum {
    MENU_ACTION_ENTRY = 0,
    MENU_ACTION_EXIT,
    MENU_ACTION_ENTER,
    MENU_ACTION_REFRESH,
}menu_action_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
void navigate_menu(FMENU_item_flash_t* current_menu_entry, FMENU_item_flash_t* new_menu_entry, menu_action_t menu_action);
//! @}
#endif // __MENU_FUNCS_H__

