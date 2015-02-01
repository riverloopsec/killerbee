// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the AVR ADC.
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
 * $Id: adc.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "adc.h"

//! \addtogroup grADCLowLevel
//! @{
/*========================= CONSTANT DEFINES         =========================*/

/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Flag indication ADC initialized
static bool adc_initialized;

//! Flag indication adc conversion ongoing
static bool adc_conversion_started;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int adc_init(adc_chan_t chan, adc_trig_t trig, adc_ref_t ref, adc_ps_t prescale)
{
    // Enable ADC module
	PRR &= ~(1 << PRADC);

    // Configure
    ADCSRA = (1<<ADEN)|prescale;
    ADMUX = (uint8_t)ref|(uint8_t)chan;
    ADCSRB = trig;

    adc_initialized = true;
    adc_conversion_started = false;

    return 0;
}

void adc_deinit(void)
{
    // Disable ADC
    ADCSRA &= ~(1<<ADEN);
	PRR |= (1 << PRADC);

    adc_initialized = false;
    adc_conversion_started = false;
}

int adc_conversion_start(void)
{
    if (adc_initialized == false) {
        return EOF;
    } else {
        adc_conversion_started = true;
        ADCSRA |= (1<<ADSC);
        return 0;
    }
}

int16_t adc_result_get(adc_adj_t adjust)
{
    if (adc_conversion_started == false) {
        return EOF;
    } else if (ADCSRA & (1<<ADSC)){
        return EOF;
    } else {
        adc_conversion_started = false;
        ADMUX |= (adjust<<ADLAR);
        return (int16_t)ADC;
    }
}
//! @}
/*EOF*/
