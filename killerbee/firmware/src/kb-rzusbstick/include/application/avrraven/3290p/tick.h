// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  API for the system tick
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
* $Id: avrraven_3290p.h 41156 2008-04-29 16:11:31Z hmyklebust $
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __TICK_H__
#define __TICK_H__

/*========================= INCLUDES                 =========================*/
#include "vrt_kernel.h"
#include "tick_conf.h"

/*========================= MACROS                   =========================*/
//! Ticks per secound
#define TICKS_PER_SEC               ((uint32_t)TICK_TIMER_FRQ/(uint32_t)TICK_TIMER_PRESCALE/(uint32_t)TICK_TIMER_MAX_VALUE)


//! Tick handler
#define tick_handler() {\
    /*Increase system tick count*/\
    tick_count++;\
    \
    /* Post system tick event */\
    tick_event_hook(); \
}

//! Unprotected read of tick count 
#define tick_count_get_isr() (tick_count)

/*========================= TYPEDEFS                 =========================*/
//! Type large enough to hold the system tick count
typedef uint32_t tick_count_t;

/*========================= PUBLIC VARIABLES         =========================*/
//! Not public, but must be extern declared for \ref tick_count_get_isr() to work
extern tick_count_t tick_count;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Initialize the system tick
 *
 */
int  tick_init(void);


/*! \brief  Get current tick count
 *
 *  \returns                    Current timer tick count
 */
tick_count_t  tick_count_get(void);


#endif // __TICK_H__
/*EOF*/
