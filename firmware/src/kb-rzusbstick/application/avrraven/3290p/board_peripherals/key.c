// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief Driver for the AVR Raven joystick.
 *
 *      Uses vrt_kernel event system to communicate joystick activity.
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
 * $Id: key.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "key.h"

#include "avrraven_3290p.h"
#include "vrt_kernel.h"
#include "int_evt.h"
#include "adc.h"

#include "board.h"

//! \addtogroup grJoystick
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int key_init(void)
{
    // Enable digital input buffer for ENTER
    DIDR1 &= ~(1 << AIN0D);

    // Enter is input w/pullup
    ENTER_DDR &= ~(1<<ENTER_PIN);
    ENTER_PUR |= (1<<ENTER_PIN);

    // Joystick is input wo/pullup (all though normal port function is overrided by ADC module when reading)
    KEY_DDR &= ~(1<<KEY_PIN);
    KEY_PUR &= ~(1<<KEY_PIN);

    // Enable PC int on ENTER to be able to wake mcu from sleep
    EIMSK |= (1<<PCIE0);
    PCMSK0 |= (1<<PCINT2);
        
    return 0;
}

void key_deinit(void)
{
    // Disable digital input buffer for ENTER
    DIDR1 |= (1 << AIN0D);

    // Disable PC int on ENTER
    EIMSK &= ~(1<<PCIE0);
    PCMSK0 &= ~(1<<PCINT2);
}

key_state_t key_state_get(void)
{

    key_state_t ret = KEY_NO_KEY;
    int16_t res;

	adc_init(ADC_CHAN_ADC1, ADC_TRIG_FREE_RUN, ADC_REF_AVCC, ADC_PS_128);
    adc_conversion_start();
    while ((res = adc_result_get(ADC_ADJ_RIGHT)) == EOF ){;}
    adc_deinit();

    if (res>0x0370) {
        ret = KEY_NO_KEY;
    }
    else if (res>0x0280) {
        ret = KEY_DOWN;
    }
    else if (res>0x0180) {
        ret = KEY_LEFT;
    }
    else if (res>0x00C0) {
        ret = KEY_RIGHT;
    }
    else {
        ret = KEY_UP;
    }

    if (!(ENTER_PORT & (1<<ENTER_PIN))) {
        ret |= KEY_ENTER;
    }

    return ret;
}

ISR(PCINT0_vect)
{

}
//! @}
/*EOF*/
