// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Numeric editor for generic use on the AVR Raven
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
 * $Id: numedit.c 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "numedit.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Pointer to data currently being edited
static uint16_t* numedit_data;

//! Mask for currently enabled numeric digits
static uint16_t numedit_mask;

//! Mask for current active (edited) numeric digits
static uint8_t numedit_blink_mask;

//! Current radix type (hex/decimal)
static numedit_radix_t numedit_radix;

//! Current maximum edit value
static int numedit_max_value;

//! Current minimum edit value
static int numedit_min_value;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int numedit_init(numedit_config_t config)
{
    uint8_t numedit_blink_start_value_mask;
    
    // Variable initialization
    numedit_max_value = config.max_value;
    numedit_min_value = config.min_value;
    numedit_mask = (config.radix == NUMEDIT_RADIX_HEX) ? 0x000F : 0x0009;
    numedit_data = config.number;
    numedit_radix = config.radix;
    
    // Build blink mask
    numedit_blink_mask = 0x01;
    for (int i=1;i<config.size; i++) {
        numedit_blink_mask <<= 1;
        numedit_blink_mask |= 0x1;
    }
    numedit_blink_mask <<= config.offset;
    numedit_blink_start_value_mask = ~numedit_blink_mask;
    
    // Configure LCD driver
    lcd_num_blink_set(numedit_blink_mask, numedit_blink_start_value_mask);
        
    return 0;
}

uint16_t numedit_event(numedit_evt_t event)
{
    uint8_t numedit_blink_start_value_mask;
    switch (numedit_radix) {
    case NUMEDIT_RADIX_HEX:
        switch (event) {
        case NUMEDIT_KEY_UP:
            if ((*numedit_data & numedit_mask) == numedit_mask) {
                *numedit_data &= ~numedit_mask;
            } else {
                *numedit_data += (numedit_mask & ~(numedit_mask-1));
            }
            
            if ((int)*numedit_data > numedit_max_value) {
                *numedit_data = numedit_min_value;
            }
            
            numedit_blink_start_value_mask = ~numedit_blink_mask;
            break;
        case NUMEDIT_KEY_DOWN:
            if ((*numedit_data & numedit_mask) == 0) {
                *numedit_data |= numedit_mask;
            } else {
                *numedit_data -= (numedit_mask & ~(numedit_mask-1));
            }
            
            if ((int)*numedit_data < numedit_min_value) {
                *numedit_data = numedit_max_value;
            }
            
            numedit_blink_start_value_mask = ~numedit_blink_mask;
            break;
        case NUMEDIT_KEY_LEFT:
            if ((numedit_mask <<= 4) == 0x0000) {
                numedit_mask = 0xF000;
            }
            if ((numedit_blink_mask <<= 1) == 0x10) {
                numedit_blink_mask = 0x80;
            }
            
            numedit_blink_start_value_mask = numedit_blink_mask;
            break;
        case NUMEDIT_KEY_RIGHT:
            if ((numedit_mask >>= 4) == 0x0000) {
                numedit_mask = 0x000F;
            }
            if ((numedit_blink_mask >>= 1) == 0x00) {
                numedit_blink_mask = 0x01;
            }
            
            numedit_blink_start_value_mask = numedit_blink_mask;
            break;
        case NUMEDIT_KEY_ENTER:
			numedit_blink_start_value_mask = numedit_blink_mask;
            break;
        default:
			numedit_blink_start_value_mask = numedit_blink_mask;
            break;
        }
        //lcd_num_puthex(*numedit_data, numedit_padding);
        break;
        
    case NUMEDIT_RADIX_DEC:
        
        switch (event) {
        case NUMEDIT_KEY_UP:
            if ((*numedit_data)++ >= numedit_max_value) {
                *numedit_data = numedit_min_value;
            }
            numedit_blink_start_value_mask = ~numedit_blink_mask;
            break;
        case NUMEDIT_KEY_DOWN:
            if ((*numedit_data)-- <= numedit_min_value) {
                *numedit_data = numedit_max_value;
            }
            numedit_blink_start_value_mask = ~numedit_blink_mask;
            break;

        default:
			numedit_blink_start_value_mask = numedit_blink_mask;
            break;
        }
        //lcd_num_putdec((int)*numedit_data, numedit_padding);
        break;
        
    default:
		numedit_blink_start_value_mask = numedit_blink_mask;
        break;
    }
    
    lcd_num_blink_set(numedit_blink_mask, numedit_blink_start_value_mask);
    
    return *numedit_data;
}

void numedit_done(void)
{
    // Update LCD driver
    numedit_blink_mask = 0x00;
    lcd_num_blink_set(numedit_blink_mask, numedit_blink_mask);
}

uint16_t numedit_value_get(void)
{
    return *numedit_data;   
}
//! @}
/*EOF*/
