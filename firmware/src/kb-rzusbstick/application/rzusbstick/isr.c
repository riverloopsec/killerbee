// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file holds all the interrupt service routines for the RZUSBSTICK
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
 * $Id: isr.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"
#include "vrt_timer.h"
#include "rf230.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
ISR(TIMER1_CAPT_vect) {   
    RF230_QUICK_ISR_HANDLER();
}


ISR(VRT_TIME_OVF_VECTOR) {
    VRT_TIMER_TICK_OVF_HANDLER();
}

ISR (VRT_TIME_COMPA_vect) {
    VRT_TIMER_HIGH_PRI_DELAY_HANDLER(); 
}


ISR(TIMER1_COMPB_vect) {
    VRT_TIMER_SLOW_TICK_HANDLER();
}
/*EOF*/
