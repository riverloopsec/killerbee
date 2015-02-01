// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Functions to manage current time and date
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
 * $Id: timndate.c 41236 2008-05-02 16:41:57Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "timndate.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

//@{ Protected access of secound timer
#define TIMNDATE_SEC_SET(value)  ENTER_CRITICAL_REGION(); timndate_sec = value; LEAVE_CRITICAL_REGION();
#define TIMNDATE_SEC_GET(var)  ENTER_CRITICAL_REGION(); var = timndate_sec; LEAVE_CRITICAL_REGION();
//@}

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/
/*! \brief  Varible holding current secound.
 *  
 *              Must be updated by calling \ref timndate_increase() or
 *              \ref timndate_increase_isr() (ISR context) in user written code
 *
 */
long timndate_sec;

/*========================= IMPLEMENTATION           =========================*/
int timndate_init(void)
{    
    // Initialize time to 00:00
    TIMNDATE_SEC_SET(0)
    
    return 0;
}

void timndate_deinit(void)
{
    // Does nothing
}

int8_t timndate_clock_get(timendate_clk_t* clk, timndate_clock_unit_t unit)
{
    // Only 24 hour clock implemented in this version of the driver
    if (unit == TIME_CLOCK_UNIT_12) {
        return EOF;
    } else {
        // Get current secound count
        long tmp;
        TIMNDATE_SEC_GET(tmp);
        
        // Calculate time in hour:min:sec
        clk->sec = tmp%60;
        clk->min = (tmp/60)%60;
        clk->hour = (tmp/3600)%24;
    }
    
    return 0;
}

int8_t timndate_clock_set(timendate_clk_t clk, timndate_clock_unit_t unit)
{
    // Only 24 hour clock implemented in this version of the driver
    if (unit == TIME_CLOCK_UNIT_12) {
        return EOF;
    }
    
    // Convert new time from hour:min:sec to secounds
    long tmp;
    tmp = clk.sec;
    tmp += clk.min*60;
    tmp += (long)clk.hour*3600L;
    
    // Write new time
    TIMNDATE_SEC_SET(tmp);
    
    return 0;
}


int8_t timndate_date_get(timendate_date_t* date)
{
    // Not implemented in this version of the driver
    return EOF;
}

int8_t timndate_date_set(timendate_date_t* date)
{
    // Not implemented in this version of the driver
    return EOF;
}

void timndate_increase(void)
{
    timndate_sec++;
}
//! @}
/*EOF*/
