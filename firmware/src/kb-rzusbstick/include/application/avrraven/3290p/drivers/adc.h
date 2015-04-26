// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR ADC driver
 *
 * \defgroup grADCLowLevel Analog-to-Digital Low Level Driver
 * \ingroup grLowLevelDrivers
 *
 * Low level driver for the AVR's Analog-to-Digital converter.
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
 * $Id: adc.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __ADC_H__
#define __ADC_H__

/*========================= INCLUDES                 =========================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

//! \addtogroup grADCLowLevel
//! @{
/*========================= CONSTANT DEFINES         =========================*/

/*========================= MACROS DEFINES           =========================*/
//! Macro for determin if an ADC conversion is ongoing
#define adc_conversion_ongoing (ADCSRA |= (1<<ADSC))

//! Macro for determin if an ADC conversion is done
#define adc_conversion_done (!adc_conversion_ongoing())

/*========================= TYPEDEFS                 =========================*/
//! Type for selecting ADC auto trigging
typedef enum {
    ADC_TRIG_FREE_RUN   = ((0<<ADTS2)|(0<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_ANACOMP    = ((0<<ADTS2)|(0<<ADTS1)|(1<<ADTS0)),
    ADC_TRIG_EXTINT0    = ((0<<ADTS2)|(1<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_TIM0_COMPA = ((0<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)),
    ADC_TRIG_TIM0_OVF   = ((1<<ADTS2)|(0<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_TIM1_COMPB = ((1<<ADTS2)|(0<<ADTS1)|(1<<ADTS0)),
    ADC_TRIG_TIM1_OVF   = ((1<<ADTS2)|(1<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_TIM1_CAPT  = ((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)),
} adc_trig_t;

//! Type for selecting ADC clock prescaler
typedef enum {
  //ADC_PS_2   = ((0<<ADPS2)|(0<<ADPS1)|(0<<ADPS0)),
    ADC_PS_2   = ((0<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)),
    ADC_PS_4   = ((0<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)),
    ADC_PS_8   = ((0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)),
    ADC_PS_16  = ((1<<ADPS2)|(0<<ADPS1)|(0<<ADPS0)),
    ADC_PS_32  = ((1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)),
    ADC_PS_64  = ((1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)),
    ADC_PS_128 = ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)),
} adc_ps_t;

//! Type for selecting ADC analog reference
typedef enum {
    ADC_REF_AREF = ((0<<REFS1)|(0<<REFS0)),
    ADC_REF_AVCC = ((0<<REFS1)|(1<<REFS0)),
    ADC_REF_INT  = ((1<<REFS1)|(1<<REFS0)),
} adc_ref_t;

//! Type for selecting ADC channel
typedef enum {
    ADC_CHAN_ADC0   = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC1   = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0)),
    ADC_CHAN_ADC2   = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC3   = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0)),
    ADC_CHAN_ADC4   = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC5   = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0)),
    ADC_CHAN_ADC6   = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC7   = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)),
    ADC_CHAN_VBG    = ((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0)),
    ADC_CHAN_GND    = ((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)),
} adc_chan_t;

//! Type for selecting right- /left adjustment of result
typedef enum {
    ADC_ADJ_RIGHT = 0,
    ADC_ADJ_LEFT = 1
} adc_adj_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
/*! \brief  Initialize the ADC module
 *
 * \param[in]      chan                 
 * \param[in]      trig                 
 * \param[in]      ref                  
 * \param[in]      prescale             
 *
 * \return							    Non-zero on error
 */
int adc_init(adc_chan_t chan, adc_trig_t trig, adc_ref_t ref, adc_ps_t prescale);


/*! \brief  De-initialize the ADC module
 *
 */
void adc_deinit(void);


/*! \brief  Start an ADC conversion
 *
 * \return							    EOF on error
 */
int adc_conversion_start(void);


/*! \brief  Initialize the ADC module
 *               
 * \param[in]      adjust             
 *
 * \return							    ADC value, EOF when conversion not finished
 */
int16_t adc_result_get(adc_adj_t adjust);
//! @}
#endif // __ADC_H__
/*EOF*/
