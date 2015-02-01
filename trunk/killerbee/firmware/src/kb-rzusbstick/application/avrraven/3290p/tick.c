// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  System tick implementation
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

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

#include "tick.h"


/*========================= MACROS                   =========================*/
//! Macro to get protected access of current tick count
#define TICK_COUNT_GET(var) {\
    ENTER_CRITICAL_REGION();\
    var = tick_count;\
    LEAVE_CRITICAL_REGION();\
}

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/
//! variable holding current time (secs)
tick_count_t tick_count;

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int tick_init(void)
{
    // Disable asyncronous timer
    tick_timer_disable();
    
    // Reset tick counter
    tick_count = 0;
    
    // Reset and start asynchronous timer with correst prescale to get selecte number of ticks per secound
    tick_timer_reset();
    tick_timer_enable();

    return 0;
}

tick_count_t  tick_count_get(void)
{
    tick_count_t current_tick_count;
    TICK_COUNT_GET(current_tick_count);
    return current_tick_count;
}
/*EOF*/
