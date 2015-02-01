// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven joystick driver
 *
 * \defgroup grJoystick Interface for the AVRRAVEN's joystick
 * \ingroup grHardwareInterfaceLayer
 *
 * The AVRRAVEN's joystick is used to collect user input, mainly when navigating
 * the menu.
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
 * $Id: key.h 41581 2008-05-13 23:13:00Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __KEY_H__
#define __KEY_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

//! \addtogroup grJoystick
//! @{
/*========================= CONSTANT DEFINES         =========================*/

/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/
typedef enum {
    KEY_UP      = 0x01,
    KEY_DOWN    = 0x02,
    KEY_LEFT    = 0x04,
    KEY_RIGHT   = 0x08,
    KEY_ENTER   = 0x10,
    KEY_NO_KEY  = 0x00
}key_state_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialize the joystick
 *
 * \return                              EOF on error
 */
int key_init(void);


/*! \brief          De-initialize the joystick
 *
 */
void key_deinit(void);


/*! \brief          Get joystick state
 *
 * \return                              Key state (\ref key_state_t)
 */
key_state_t key_state_get(void);

//! @}
#endif // __KEY_H__
/*EOF*/
