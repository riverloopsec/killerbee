// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  Declarations for all internal(3290p only) event handlers
*
*      
*
* \par Application note:
*
* \par Documentation
*
* \author
*      Atmel Corporation: http://www.atmel.com \n
*      Support email: avr@atmel.com
*
* $Id$
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __INT_EVT_H__
#define __INT_EVT_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>



//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

//! Pointer to mail buffer. Used for writing mail
extern uint8_t* new_mail_buffer;

//! Generic numeric editor buffer
extern uint16_t numedit_buffer;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Polling of the joystikk
 *
 *  \param[in]  evt  Pointer to event data
 */
void int_evt_check_key(void* evt);


/*! \brief Read battery voltage
 *
 *  \param[in]  evt  Pointer to event data
 */
void int_evt_update_batt(void* evt);


/*! \brief  Show current temperature in the numeric display
 *
 *  \param[in]  evt  Pointer to event data
 */
void int_evt_show_temp(void* evt);


/*! \brief  Show clock in the numeric display
 *
 *  \param[in]  evt  Pointer to event data
 */
void int_evt_show_clock(void* evt);


/*! \brief  Show date in the numeric display
 *
 *  \param[in]  evt  Pointer to event data
 */
void int_evt_show_date(void* evt);


/*! \brief  Start up AVR Raven
 *
 *  \param[in]  evt  Pointer to event data
 */
void int_evt_startup(void* evt);


/*! \brief  
 *
 *  \param[in]  evt  Pointer to event data
 */
extern void int_evt_lcd_cursor_toggle(void* evt);


/*! \brief  Refresh numeric display
 *
 *  \param[in]  evt  Pointer to event data
 */
extern void int_evt_lcd_num_refresh(void* evt);


/*! \brief  Shift alphanumeric text left
 *
 *  \param[in]  evt  Pointer to event data
 */
extern void int_evt_lcd_shift_left(void* evt);


/*! \brief  If a change in joystick position is detected this event is posted
 *
 *  \param[in]  evt  Pointer to event data
 */
extern void int_evt_key(void* evt);

//! @}
#endif // __INT_EVT_H__
/*EOF*/
