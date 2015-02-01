// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven menu  navigator
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
 * $Id: menu.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#include "menu.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
FMENU_item_flash_t* FMENU_GoToRootItem( FMENU_handle_t* menuHandle )
{
	menuHandle->currentItem = menuHandle->items;
	return menuHandle->currentItem;
}

FMENU_item_flash_t* FMENU_NavigateUp( FMENU_handle_t* menuHandle )
{
	menuHandle->currentItem = menuHandle->currentItem->up;
	return menuHandle->currentItem;
}

FMENU_item_flash_t* FMENU_NavigateDown( FMENU_handle_t* menuHandle )
{
	menuHandle->currentItem = menuHandle->currentItem->down;
	return menuHandle->currentItem;
}

FMENU_item_flash_t* FMENU_NavigateLeft( FMENU_handle_t* menuHandle )
{
	menuHandle->currentItem = menuHandle->currentItem->parent;
	return menuHandle->currentItem;
}

FMENU_item_flash_t* FMENU_NavigateRight( FMENU_handle_t* menuHandle )
{
	menuHandle->currentItem = menuHandle->currentItem->child;
	return menuHandle->currentItem;
}

FMENU_item_flash_t* FMENU_GetCurrentItem ( FMENU_handle_t* menuHandle )
{
	return menuHandle->currentItem;
}

uint8_t FMENU_GetNumberOfItemsAbove( FMENU_item_flash_t* menuItem)
{
	uint8_t itemCounter = 0;
	
	//Search through menu items above current until FIRST_IN_MENU flag is encountered.
	while( !( FMENU_IsFirstInMenu( menuItem ) ) )
	{
		itemCounter++;
		menuItem = menuItem->up; // Now point to item above.
	}

	return itemCounter;
}

uint8_t FMENU_GetNumberOfItemsBelow( FMENU_item_flash_t* menuItem )
{
	uint8_t itemCounter = 0;
	
	//Search through menu items below current until FIRST_IN_MENU flag is encountered.
	while( !( FMENU_IsFirstInMenu( menuItem->down ) ) )
	{
		itemCounter++;
		menuItem = menuItem->down; // Now point to item below.
	}

	return itemCounter;
}

FMENU_item_flash_t* FMENU_GetItemAbove ( FMENU_item_flash_t* menuItem )
{
	return menuItem->up;
}

FMENU_item_flash_t* FMENU_GetItemBelow ( FMENU_item_flash_t* menuItem )
{
	return menuItem->down;
}

PROGMEM_BYTE_ARRAY_T FMENU_GetTitle ( FMENU_item_flash_t * menuItem )
{
	return menuItem->title;
}

bool FMENU_IsFirstInMenu( FMENU_item_flash_t * menuItem )
{
	if (menuItem->flags.firstItemInMenu == 0){
        return false;
    } else {
        return true;
    }
}
//! @}
/*EOF*/
