// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven asynchronous timer
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
 * $Id: lcd.h 41236 2008-05-02 16:41:57Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __ASY_TMR_H__
#define __ASY_TMR_H__

/*========================= INCLUDES                 =========================*/
#include "tick.h"

/*========================= MACROS                   =========================*/
//! Max number of timers
#define ASY_TMR_COUNT        8

//! Max number of timers
#define ASY_TMR_NO_TIMER    (-1)

/*========================= TYPEDEFS                 =========================*/
//! Timer type
typedef int8_t asy_tmr_t;

//! Typedef for the function types associated with the timeouts
typedef void (*asy_tmr_handler_t)(void*);

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Initialize the asynchronous timer
 *
 */
int  asy_tmr_init(void);


/*! \brief  De-initialize the asynchronous timer
 *
 */
void asy_tmr_deinit(void);


/*! \brief  Get a timer
 *
 *  \param[in]  event_handler   Pointer to timeout callback function
 *  \param[in]  attribute       Pointer to event data
 *  \param[in]  timeout         Selected timeout
 *
 *  \returns                    EOF on no free timers
 */
asy_tmr_t asy_tmr_get(vrt_event_handler_t event_handler, void* attribute, long timeout);


/*! \brief  Put a timer
 *
 *  \param[in]  tmr             Timer to put
 *
 *  \returns                    EOF if timer not in use or out of range
 */
int8_t asy_tmr_put(asy_tmr_t tmr);


/*! \brief  Asynchronous timer handler
 *
 *  \param[in]  current_tick_count  Current tick count
 */
void asy_tmr_task(tick_count_t current_tick_count);


#endif // __ASY_TMR_H__
/*EOF*/
