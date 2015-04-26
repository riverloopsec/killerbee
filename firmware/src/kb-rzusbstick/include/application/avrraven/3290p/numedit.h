// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven generic numeric editor
 *
 *           The numeric editor makes the user use the joystick while
 *           editing the value in the numeric section of the AVR Raven.
 *           The number may be both decimal and hexadecimal, and the value is
 *           continously updated in the display.
 *
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
 *
 * \par Documentation
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: numedit.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __NUMEDIT_H__
#define __NUMEDIT_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

#include "lcd.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

///@{ Macros for setlecting padding type
#define NUMEDIT_PADDING_SPACE LCD_NUM_PADDING_SPACE
#define NUMEDIT_PADDING_ZERO LCD_NUM_PADDING_ZERO
///@}

/*========================= TYPEDEFS                 =========================*/

/// Type used to identify events for \ref numedit_evt_t
typedef enum {
    NUMEDIT_KEY_UP,
    NUMEDIT_KEY_DOWN,
    NUMEDIT_KEY_LEFT,
    NUMEDIT_KEY_RIGHT,
    NUMEDIT_KEY_ENTER,
} numedit_evt_t;

/// Type used to set neric radix
typedef enum {
    NUMEDIT_RADIX_HEX,
    NUMEDIT_RADIX_DEC,
} numedit_radix_t;

/// Type used to select padding (space/zero)
typedef lcd_padding_t numedit_padding_t;

/// Type used to initialize the numeric editor
typedef struct {
    uint16_t* number;
    int16_t min_value;
    int16_t max_value;
    numedit_radix_t radix;
//    numedit_padding_t padding;
    uint16_t size;
    uint16_t offset;
}numedit_config_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
/*! \brief  Initialize the numeric editor
 *
 *              Must be executed to start each numeric edit session. 
 *
 *  \param[in]  config    Configuration
 *
 *  \returns    EOF on fail
 */
int numedit_init(numedit_config_t config);


/*! \brief  End current edit session
 *
 *          Displays edited value and resets internal variables.
 *          Must be executed to start each numeric edit session. 
 */
void numedit_done(void);


/*! \brief  Execute a \ref numedit_evt_t event
 *
 *  \param[in]  event   Event of type \ref numedit_evt_t
 *
 */
uint16_t numedit_event(numedit_evt_t event);


/*! \brief  Get current value of the edited number
 *
 *  \returns    Current value
 */
uint16_t numedit_value_get(void);
//! @}
#endif // __NUMEDIT_H__

