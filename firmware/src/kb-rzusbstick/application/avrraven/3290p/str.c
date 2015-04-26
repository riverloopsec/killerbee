// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  String utility functions.
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
 * $Id: vrt_timer_xmega_conf.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#include "str.h"
#include "str_conf.h"

void itos(int integer, unsigned char* string, int8_t digit_count)
{
    unsigned char c;
    int sub = 1;
   
    // calculate "sub". I.e.: 5 digits ->10^(5-1) -> sub = 10000
    for (c=0; c<(digit_count-1); c++) {
        sub *= 10;
    }

    // Fill string with ascii '0' and zero terminate
    for (c=0; c<digit_count; c++) {
        string[c] = '0';
    }
    string[c] = 0x00;

    // Make string
    for (c=0; c<digit_count; c++) {
        while( integer >= sub ) {
            string[c]++;
            integer -= sub;
        }
        sub /= 10;
    }
}

void insi(unsigned char* string, uint16_t integer, int8_t offset, int8_t digit_count)
{
    int sub = 1;
    
    // Make pointer to selected offset in string
    unsigned char *str = &string[offset];
    
    // Calculate start value based on "digit_count"
    for( int i=1; i<digit_count; i++ ) {
        sub *= 10;
    }
    
    // Calculate value for each digit and insert into string
    for( int i=0; i<digit_count; i++ ) {
        // Calulate digit value
        unsigned char digit;
        for (digit = '0'; integer >= sub; integer -= sub) {
             digit++;
        }
        
        // Write digit to string and increment position
        *(str++) = digit;
        
        // Decrease 
        sub /= 10;
    }
}
/*EOF*/
