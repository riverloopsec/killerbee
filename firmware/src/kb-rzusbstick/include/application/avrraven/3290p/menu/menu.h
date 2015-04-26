// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains documentation shown on the main page
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
 * $Id: menu.h 41528 2008-05-13 09:45:28Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "menu_data.h"
#include "compiler.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*----------------------------------------------------------------------------*/
/* Defines
*/

#define FIRST_IN_MENU				(1)	// !< Bit mask for menuItem_t bitfield structure
#define SPECIAL_FUNC_MENU_ITEM	(1)	// !< Bit mask for menuItem_t bitfield structure

/*----------------------------------------------------------------------------*/
/* Type definitions
*/

/*! \brief Defines the type used for menu item status flags.*/
typedef struct FMENU_menuItemFlags{
	uint8_t firstItemInMenu : 1;
	uint8_t itemHasData : 1;
  } menuItemFlags_t;

/*! \brief Defines the data for the menu item. This can be used to determine the
functionality that the menu item should trigger.*/
typedef uint16_t FMENU_menuData_t;

/*! \brief Defines type used for size of menu (number of menu items in a menu)
- available for easy replacement of type.*/
typedef uint8_t FMENU_size_t;

/*! \brief Defines the type used for menu items.*/
typedef struct menuItem_TAG{
  struct menuItem_TAG PROGMEM_PTR_T up;
  struct menuItem_TAG PROGMEM_PTR_T down;
  struct menuItem_TAG PROGMEM_PTR_T parent;
  struct menuItem_TAG PROGMEM_PTR_T child;
  unsigned char PROGMEM_PTR_T title;
  void (*action)(void*);
  //FMENU_menuData_t data;
  menuItemFlags_t flags;
} FMENU_item_t;


typedef FMENU_item_t PROGMEM_T  FMENU_item_flash_t;

/*! \brief Defines the type used to define a menu handle, which holds control and
 status information about the menu.*/
typedef struct menu{
   FMENU_item_flash_t *items;
   FMENU_item_flash_t *currentItem;
   uint8_t itemsInMenu;
} FMENU_handle_t;


/*----------------------------------------------------------------------------*/
/* Function proto types for menu functions defined in menu.c */
//void FMENU_Init( FMENU_handle_t*, FMENU_item_flash_t*, FMENU_size_t);
//void FMENU_Recall( FMENU_handle_t* menuHandle );

/*----------------------------------------------------------------------------*/
/*! \brief Resets the active menu to use the root menu item as current item.
 *
 * This function can be used to get back to a well-defined place in the active
 * menu. Thia can e.g. be used when used if user invokes a certain command to
 * get back to the "point of origin" in the menu system.
 *
 * \param menuHandle Pointer to menu item.
 *
 * \returns The new "current" menu item.
 */
FMENU_item_flash_t* FMENU_GoToRootItem( FMENU_handle_t* menuHandle );


/*----------------------------------------------------------------------------*/
/*! \brief Navigation up in menu.
 *
 * Handles navigation in the up direction in a menu. Returns a pointer to the
 * new current item in the menu.
 *
 * \param menuHandle Pointer to menu item.
 *
 * \returns Pointer to the new current-menu-item.
 */
FMENU_item_flash_t* FMENU_NavigateUp( FMENU_handle_t* menuHandle );

/*----------------------------------------------------------------------------*/
/*! \brief Navigation down in menu.
 *
 * Handles navigation in the down direction in a menu. Returns a pointer to the
 * new current item in the menu.
 *
 * \param menuHandle Pointer to menu item.
 *
 * \returns Pointer to the new current-menu-item.
 */
FMENU_item_flash_t* FMENU_NavigateDown( FMENU_handle_t* menuHandle );

/*----------------------------------------------------------------------------*/
/*! \brief Navigation left in menu.
 *
 * Handles navigation in the left direction in a menu. Returns a pointer to the
 * new current item in the menu.
 *
 * \param menuHandle Pointer to menu item.
 *
 * \returns Pointer to the new current-menu-item.
 */
FMENU_item_flash_t* FMENU_NavigateLeft( FMENU_handle_t* menuHandle );

/*----------------------------------------------------------------------------*/
/*! \brief Navigation right in menu.
 *
 * Handles navigation in the right direction in a menu. Returns a pointer to the
 * new current item in the menu.
 *
 * \param menuHandle Current menu item.
 *
 * \returns Pointer to the new current-menu-item.
 */
FMENU_item_flash_t* FMENU_NavigateRight( FMENU_handle_t* menuHandle );

/*----------------------------------------------------------------------------*/
/*! \brief Provides a pointer to the current menu item.
 *
 * Can be used to determine the current menu item. Is intended used together with
 * the other functions that take the the current menu item pointer as input.
 *
 * \param menuHandle Current menu item.
 *
 * \returns Pointer to the new current-menu-item.
 */
FMENU_item_flash_t* FMENU_GetCurrentItem ( FMENU_handle_t* menuHandle );


/*----------------------------------------------------------------------------*/
/*! \brief Get the number of menu items above a menu item.
 *
 * Is intended used when drawing menus on a display: the function returns the number
 * of menu items above a menu items, typically below the current item. The functions
 * search for the "first in menu" menu item and uses this as the top of the menu.\n
 *
 * \param menuItem Pointer to menu item.
 *
 * \returns Number of menu items above the current menu item
 * (not including the item given as input parameterargument)
 */
uint8_t FMENU_GetNumberOfItemsAbove( FMENU_item_flash_t* );

/*----------------------------------------------------------------------------*/
/*! \brief Get the number of menu items below a menu item.
 *
 * Is intended used when drawing menus on a display: the function returns the number
 * of menu items below a menu items, typically below the current item. The functions
 * search for the "first in menu" menu item and uses this as the top of the menu.\n
 *
 * \param menuItem Menu item to find number of items below of.
 *
 * \returns Number of menu items below the current menu item
 * (not including the item given as input parameterargument, nor first_in_menu menu item)
 */
uint8_t FMENU_GetNumberOfItemsBelow( FMENU_item_flash_t* );


/*----------------------------------------------------------------------------*/
/*! \brief Get pointer to the menu item above the item given as input parameter.
 *
 * Can be used when drawing menus in a display, since this function can provide
 * pointers to any item in a menu, without modifying the current-menu-item (the
 * menu item "in focus").
 *
 * \param menuItem Pointer to a menu item.
 *
 * \returns Pointer to the menu-item above the item given as input parameter.
 */
FMENU_item_flash_t* FMENU_GetItemAbove ( FMENU_item_flash_t* );

/*----------------------------------------------------------------------------*/
/*! \brief Get pointer to the menu item above the item given as input parameter.
 *
 * Can be used when drawing menus in a display, since this function can provide
 * pointers to any item in a menu, without modifying the current-menu-item (the
 * menu item "in focus").
 *
 * \param menuItem is a pointer to a menu item.
 *
 * \returns FMENU_item_flash_t* is a pointer to the menu-item above the item given as input parameter.
 */
FMENU_item_flash_t* FMENU_GetItemBelow ( FMENU_item_flash_t* );

/*----------------------------------------------------------------------------*/
/*! \brief Get pointer to title of a menu item through calling parameter.
 *
 * Returns a pointer to the title of the menu item given as input parameter.
 *
 * \param menuItem is a pointer to a menu item.
 *
 * \returns Pointer to the menu title.
 */
PROGMEM_BYTE_ARRAY_T FMENU_GetTitle ( FMENU_item_flash_t* );


/*----------------------------------------------------------------------------*/
/*! \brief Investigates if the menu item given as input parameter is the first in the menu level.
 *
 * The first in menu state of an menu item indicates that it is the top menu item
 * in a menu level. This first in menu information can be used to determine how
 * the menu highlighting should handled (e.g. wrap menu items or set highlighting
 * to the buttom of the display.
 *
 * \param menuItem is a pointer to a menu item.
 *
 * \returns Boolean variable that is either true or false.
 */
bool FMENU_IsFirstInMenu( FMENU_item_flash_t* );
//! @}
#endif
