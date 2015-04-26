// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the AVR Raven LED, "The Red Eye"
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
 * $Id: led.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "led.h"

#include <stdbool.h>
#include <stdint.h>

#include "compiler.h"

#include "led_conf.h"

//! \addtogroup grREDEYELowLevel
//! @{
/*========================= MACROS                   =========================*/
//! Value used for slow blinking
#define LED_SLOW_BLINK_VALUE            LED_TIMER_SECOUND_PRESCALE/2

//! Value used for fast blinking
#define LED_FAST_BLINK_VALUE            LED_TIMER_SECOUND_PRESCALE/15

//! Value used for soft blinking
#define LED_SOFT_BLINK_PRESCALE_VALUE   LED_TIMER_SECOUND_PRESCALE/15

//! Max PWM value for LED status == ON. (to save power)
#define LED_ON_VALUE    0xA0

//! Value to load timer for LED status == OFF
#define LED_OFF_VALUE   0x00

/*========================= TYEPDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Flag for enable/disable
static bool blink_enable;

//! LED blink prescale buffer
static int  blink_prescale;

//! Value to load \ref blink_prescale
static int  blink_prescale_value;

#ifndef LED_DRIVER_MINIMAL
//! Flag for soft blink enable/disable
static bool softblink_enable;

//! Softblink prescale buffer
static int  softblink_prescale;

//! Value to load \ref softblink_prescale
static int  softblink_prescale_value ;
#endif

#ifndef LED_DRIVER_MINIMAL
//! Sinus value table to generate soft blink
PROGMEM_DECLARE(uint8_t) tmr[] = {
103	,
102	,
99	,
94	,
89	,
83	,
77	,
69	,
62	,
54	,
46	,
39	,
31	,
25	,
19	,
14	,
9	,
6	,
5	,
4	,
};
#endif


/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/


int led_init(void)
{
    // Setup LED for Raven eye
    PORTB |= (1<<PB7);  // LED off
    DDRB |= (1<<PB7);   // LED output
    PORTB |= (1<<PB7);  // LED off when not controlled by timer

    led_status_set(LED_OFF);

    return 0;
}


void led_deinit(void)
{
    
}


void led_status_set(led_status_t status)
{
    switch (status) {
    case LED_ON:
        TCCR2A |= (1<<COM2A1)|(1<<COM2A0)|(1<<WGM21)|(1<<WGM20);    // fast PWM mode
        TIMSK2 |= (1<<OCIE2A);      // enable interrupt
        OCR2A = LED_ON_VALUE;
        blink_enable = false;
#ifndef LED_DRIVER_MINIMAL
        softblink_enable = false;
#endif
        break;
#ifndef LED_DRIVER_MINIMAL
    case LED_OFF:
        TIMSK2 &= ~(1<<OCIE2A);     // disable interrupt
        TCCR2A &= ~((1<<COM2A1)|(1<<COM2A0)); // disable output compare to get port control
        PORTB |= (1<<PB7);          // LED off
        blink_enable = false;
        softblink_enable = false;
        break;
    case LED_SLOW_BLINK:
        TCCR2A |= (1<<COM2A1)|(1<<COM2A0)|(1<<WGM21)|(1<<WGM20);    // fast PWM mode
        TIMSK2 |= (1<<OCIE2A);      // enable interrupt
        OCR2A = LED_ON_VALUE;
        blink_prescale_value = LED_SLOW_BLINK_VALUE;
        blink_enable = true;
        softblink_enable = false;
        break;
#endif
    case LED_FAST_BLINK:
        TCCR2A |= (1<<COM2A1)|(1<<COM2A0)|(1<<WGM21)|(1<<WGM20);    // fast PWM mode
        TIMSK2 |= (1<<OCIE2A);      // enable interrupt
        OCR2A = LED_ON_VALUE;
        blink_prescale_value = LED_FAST_BLINK_VALUE;
        blink_enable = true;
#ifndef LED_DRIVER_MINIMAL
        softblink_enable = false;
#endif
        break;
#ifndef LED_DRIVER_MINIMAL
    case LED_SOFT_BLINK:
        TCCR2A |= (1<<COM2A1)|(1<<COM2A0)|(1<<WGM21)|(1<<WGM20);    // fast PWM mode
        TIMSK2 |= (1<<OCIE2A);      // enable interrupt
        OCR2A = 0x04;
        softblink_prescale_value = LED_SOFT_BLINK_PRESCALE_VALUE;
        blink_enable = false;
        softblink_enable = true;
        break;
#endif
    default:
        // Status not defined, do nothing
        break;
    }
}


ISR(TIMER2_COMP_vect)
{
    static bool enable = 1;
#ifndef LED_DRIVER_MINIMAL
    static bool direction = 1;
    static int i = 0;
#endif

    if (blink_enable) {
        if (!blink_prescale--) {
            if ((enable = !enable) == false) {
                TCCR2A &= ~((1<<COM2A1)|(1<<COM2A0));
            }
            else {
                TCCR2A |= (1<<COM2A1)|(1<<COM2A0);
            }
            blink_prescale = blink_prescale_value;
        }
    }
#ifndef LED_DRIVER_MINIMAL
    else if (softblink_enable) {
        if (!softblink_prescale--) {
            if (direction) {
                OCR2A = tmr[i++];
                if (i == sizeof(tmr)) {
                    direction = 0;
                }
            }
            else {
                OCR2A = tmr[--i];
                if (i == 0) {
                    direction = 1;
                }
            }
            softblink_prescale = softblink_prescale_value;
        }
    }
#endif
}
//! @}
/*EOF*/
