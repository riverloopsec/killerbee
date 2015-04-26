// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven text editor
 *
 *           The text editor makes the user able to use the joystick while
 *           editing the value in the alphanumeric section of the AVR Raven.
 *           The text is continously updated in the display.
 *
 * \defgroup applAVRRAVEN3290pUtilities Utilities used by the ATmega3290p
 * \ingroup applAVRRAVEN
 *
 *      Utilities are a set of code snippets that perform application specific
 *      tasks on application level; playing audio, clock, menu navigation, text
 *      editor etc.
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
 * $Id: txtedit.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __TXTEDIT_H__
#define __TXTEDIT_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= CONSTANT DEFINES         =========================*/

/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/

/// Type used to identify events for \ref txtedit_event
typedef enum {
    TXTEDIT_KEY_UP,
    TXTEDIT_KEY_DOWN,
    TXTEDIT_KEY_LEFT,
    TXTEDIT_KEY_RIGHT,
    TXTEDIT_KEY_ENTER,
} txtedit_evt_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
/*! \brief  Initialize the text editor
 *
 *              Must be executed to start each text edit session. 
 *
 *  \param[in]  txtbuffer       Pointer to text edit buffer
 *  \param[in]  txtbuffer_size  Size of text edit buffer
 *
 *  \return     EOF on fail
 */
int txtedit_init(uint8_t* txtbuffer, uint8_t txtbuffer_size);


/*! \brief  End current edit session
 *
 *          Displays edited text and resets internal variables.
 *          Must be executed to start each text edit session. 
 *
 *  \return     Nothing
 */
void txtedit_done(void);


/*! \brief  Execute a \ref txtedit_evt_t event
 *
 *  \param[in]  event   Event of type \ref txtedit_evt_t
 *
 *  \return     Nothing
 */
void txtedit_event(txtedit_evt_t event);


/*! \brief  Get current size of text being edited 
 *
 *  \return     Text size
 */
int8_t txtedit_size_get(void);
//! @}
#endif // __TXTEDIT_H__
/*EOF*/
