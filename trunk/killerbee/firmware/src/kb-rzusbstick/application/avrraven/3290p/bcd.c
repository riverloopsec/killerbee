// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Convertion algorithm for number/BCD
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
 * $Id: bcd.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "bcd.h"

/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
uint16_t itobcd(uint16_t integer)
{
    uint16_t bcd = 0;

    // Count thousends
    while (integer >= 1000) {
        bcd++;
        integer -= 1000;
    }
    bcd <<= 4;

    // Count hundreds
    while (integer >= 100) {
        bcd++;
        integer -= 100;
    }
    bcd <<= 4;

    // Count tens
    while (integer >= 10) {
        bcd++;
        integer -= 10;
    }

    return  (bcd << 4) | integer;        // Add ones and return answer
}


uint16_t bcdtoi(uint16_t bcd_number)
{
    uint16_t integer;
    
    integer  = ((bcd_number&0x000F) >>  0) *    1;
    integer += ((bcd_number&0x00F0) >>  4) *   10;
    integer += ((bcd_number&0x0F00) >>  8) *  100;
    integer += ((bcd_number&0xF000) >> 12) * 1000;
    
    return integer;
}
/*EOF*/
