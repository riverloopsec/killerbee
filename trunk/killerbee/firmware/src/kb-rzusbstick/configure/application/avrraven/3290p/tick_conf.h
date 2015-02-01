// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Configuration of the tick timer
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

#ifndef __TICK_CONF_H__
#define __TICK_CONF_H__

/*========================= INCLUDES                 =========================*/
#include "timndate.h"

/*========================= MACROS                   =========================*/
#if defined(AVRRAVEN_3290P)

//! Frequency of tick timer
#define TICK_TIMER_FRQ                 (32768)

//! Tick timer max value
#define TICK_TIMER_MAX_VALUE           (256)

//! Tick timer prescale, must be a valid prescale and match the values in the macro \ref TICK_TIMER_ENABLE()
#define TICK_TIMER_PRESCALE            (1)

//! Macro to disable the tick timer
#define tick_timer_disable() {\
    TIMSK2 &= ~(1<<TOIE2);\
}

//! Macro to reset the tick timer
#define tick_timer_reset() {\
    TCNT2 = 0x00;\
}

//! Macro to enable the tick timer
#define tick_timer_enable() {\
    \
    /* Asynchronous operation */\
    ASSR |= (1<<AS2);\
    \
    /* Enable async timer interrupt */\
    TIMSK2 |= (1<<TOIE2);\
    \
    /* start timer */\
    TCCR2A &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));\
    TCCR2A |= (1<<CS20);\
}

#else
#error "Asynchronous timer configuration not supported."
#endif

//! This hook is executed under ISR context every timer tick
#define tick_event_hook() {\
    tick_count_t current_tick_count = tick_count_get_isr();\
    \
    /* Increase time by one secound every TICKS_PER_SEC tick */\
    if (current_tick_count%TICKS_PER_SEC == 0) {\
        timndate_increase_isr();\
    }\
}

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

#endif // __TICK_CONF_H__
/*EOF*/
