// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  Asynchronous timer implementation
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

#include <stdio.h>
#include "asy_tmr.h"

/*========================= MACROS                   =========================*/


/*========================= TYPEDEFS                 =========================*/
//! Timer element type
typedef struct {
    asy_tmr_handler_t handler;
    void* attribute;
    long delay;
    long timeout;
}asy_tmr_element_t;


/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Timer list
static asy_tmr_element_t asy_tmr_list[ASY_TMR_COUNT];

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/

int asy_tmr_init(void)
{
    return 0;
}

void asy_tmr_deinit(void)
{
    
}

asy_tmr_t asy_tmr_get(vrt_event_handler_t event_handler, void* attribute, long timeout)
{
    // Get current tick count
    tick_count_t current_tick_count;
    current_tick_count = tick_count_get();
    
    // calculate tick count for new timeout
    tick_count_t timeout_ticks = timeout*TICKS_PER_SEC/1000UL;
    
    // Try to get timer
    for (asy_tmr_t tmr = 0; tmr<ASY_TMR_COUNT; tmr++) {
        if (asy_tmr_list[tmr].handler == 0) {
            asy_tmr_list[tmr].handler = event_handler;
            asy_tmr_list[tmr].attribute = attribute;
            asy_tmr_list[tmr].delay = timeout_ticks;
            asy_tmr_list[tmr].timeout = current_tick_count + timeout_ticks;
            
            return tmr;
        }
    }
    
    return EOF;
}

int8_t asy_tmr_put(asy_tmr_t tmr)
{
    // Return if not in range (inclulding ASY_TMR_NO_TIMER)
    if ((tmr <= 0) || (tmr >= ASY_TMR_COUNT)) {
        return EOF;
    }
    
    // Return if timer not used
    if (asy_tmr_list[tmr].handler == NULL) {
        return EOF;
    }
    
    // Free timer
    if (asy_tmr_list[tmr].handler != NULL) {
        asy_tmr_list[tmr].handler = NULL;
    }
    
    return 0;
}

void asy_tmr_task(tick_count_t current_tick_count)
{
    // Read through tick event list and post all registered events that has timed out
    for (int i=0; i<ASY_TMR_COUNT; i++) {
        if (asy_tmr_list[i].handler != 0) {
            if (current_tick_count >= asy_tmr_list[i].timeout){
                vrt_post_event(asy_tmr_list[i].handler, asy_tmr_list[i].attribute);   //< post event
                asy_tmr_list[i].timeout += asy_tmr_list[i].delay;                     //< set next timeout
            }
        }
    }
}
/*EOF*/
