// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the AVR Raven battery monitor
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
 * $Id: battery.c 41500 2008-05-12 21:27:34Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/


/*========================= INCLUDES                 =========================*/
#include "battery.h"

#include "adc.h"
#include "lcd.h"
#include "mbox.h"
#include "sfs.h"
#include "avrraven_3290p.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int battery_init(void)
{
    lcd_symbol_battery_init();
    
    return 0;
}

void battery_deinit(void)
{
    lcd_symbol_battery_remove();
}

int16_t battery_voltage_read(void)
{
    int16_t adc_res;
    // Read band gap voltage with Avcc as ref. Use result to calulate Vcc
	adc_init(ADC_CHAN_VBG, ADC_TRIG_FREE_RUN, ADC_REF_AVCC, ADC_PS_128);
    
    // wait for band gap voltage to stabilize
    delay_us(150);
    
    // read ADC
    adc_conversion_start();
    while ((adc_res = adc_result_get(ADC_ADJ_RIGHT)) == EOF ){;}
    adc_deinit();
    
    // Return battery voltage in mV
    return (int16_t)((1100L*1024L)/(long)adc_res);
}
//! @}
/*EOF*/
