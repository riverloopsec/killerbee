// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the on board temperature sensor
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
 * $Id: temp.c 41528 2008-05-13 09:45:28Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "temp.h"

#include "board.h"
//! \addtogroup grNTC
//! @{
/*========================= MACROS                   =========================*/
//! Type used together with \ref find_temp() to find temperature
typedef enum {
    TEMP_ZERO_OFFSET_CELCIUS    = -15,
    TEMP_ZERO_OFFSET_FAHRENHEIT =   0
} temp_zero_offset_t;


/*========================= TYEPDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Celcius temperatures (ADC-value) from -15 to 60 degrees
PROGMEM_DECLARE(static uint16_t temp_table_celcius[]) = {
    923,917,911,904,898,891,883,876,868,860,851,843,834,825,815,
    806,796,786,775,765,754,743,732,720,709,697,685,673,661,649,
    636,624,611,599,586,574,562,549,537,524,512,500,488,476,464,
    452,440,429,418,406,396,385,374,364,354,344,334,324,315,306,
    297,288,279,271,263,255,247,240,233,225,219,212,205,199,193,
    187,
};

//! Fahrenheit temperatures (ADC-value)   from 0 to 140 degrees
PROGMEM_DECLARE(static uint16_t temp_table_fahrenheit[]) = {
    938, 935, 932, 929, 926, 923, 920, 916, 913, 909, 906, 902, 898,
    894, 891, 887, 882, 878, 874, 870, 865, 861, 856, 851, 847, 842,
    837, 832, 827, 822, 816, 811, 806, 800, 795, 789, 783, 778, 772,
    766, 760, 754, 748, 742, 735, 729, 723, 716, 710, 703, 697, 690,
    684, 677, 670, 663, 657, 650, 643, 636, 629, 622, 616, 609, 602,
    595, 588, 581, 574, 567, 560, 553, 546, 539, 533, 526, 519, 512,
    505, 498, 492, 485, 478, 472, 465, 459, 452, 446, 439, 433, 426,
    420, 414, 408, 402, 396, 390, 384, 378, 372, 366, 360, 355, 349,
    344, 338, 333, 327, 322, 317, 312, 307, 302, 297, 292, 287, 282,
    277, 273, 268, 264, 259, 255, 251, 246, 242, 238, 234, 230, 226,
    222, 219, 215, 211, 207, 204, 200, 197, 194, 190, 187,
};

//! Flag indicating initialized or not
bool temp_initialized = false;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

//
/*! \brief          Find array index corresponding to input ADC value
 *
 *                  Returned array index is actual temperature + zero offset. To
 *                  get actual temperature, the zero offset (\ref temp_zero_offset_t)
 *                  has to be subtracted.
 *
 * \param[in]		value			Value to seracah for in table
 * \param[in]		array			Pointer to array in which to look for ADC value
 * \param[in]		count			Size of array
 *
 *
 * \return                              EOF on error
 */
static int find_temp(int16_t value, PROGMEM_WORD_ARRAY_T array, int count);

/*========================= IMPLEMENTATION           =========================*/
int temp_init(void)
{
    // Temp sens power pin as output
    TMPPWR_DDR |= (1<<TMPPWR_PIN);

    // Power off temp sensor
    TMPPWR_PORT &= ~(1<<TMPPWR_PIN);

    // Temp sens input, no pullup
    TEMP_DDR &= ~(1<<TEMP_PIN);
    TEMP_PUR &= ~(1<<TEMP_PIN);
    
    temp_initialized = true;
        
    return 0;
}

void temp_deinit(void)
{
    temp_initialized = false;
}

int16_t temp_get(temp_unit_t unit)
{
    int16_t res;
    int16_t temp;

    // Return if temp sensor driver not initialized
    if (temp_initialized == false) {
        return EOF;
    }
    
    // Power up sensor
    TMPPWR_PORT |= (1<<TMPPWR_PIN);

    // Init ADC and measure
	adc_init(ADC_CHAN_ADC4, ADC_TRIG_FREE_RUN, ADC_REF_AVCC, ADC_PS_128);
    adc_conversion_start();
    while ((res = adc_result_get(ADC_ADJ_RIGHT)) == EOF ){
        ;
    }
    adc_deinit();

    // Power down sensor
    TMPPWR_PORT &= ~(1<<TMPPWR_PIN);

    // Get corresponding temperature from table
    if (unit == TEMP_UNIT_CELCIUS) {
        temp = find_temp(res, temp_table_celcius, sizeof(temp_table_celcius)/sizeof(int)) + TEMP_ZERO_OFFSET_CELCIUS;
    } else /*unit == TEMP_UNIT_FAHRENHEIT*/{
        temp = find_temp(res, temp_table_fahrenheit, sizeof(temp_table_fahrenheit)/sizeof(int)) + TEMP_ZERO_OFFSET_FAHRENHEIT;
    }

    return temp;
}

static int find_temp(int16_t value, PROGMEM_WORD_ARRAY_T array, int count)
{
    int i = 0;
    do{
        if (array[i] < value) {
            return i;
        }
    } while(++i<count);
    return EOF;
}

//! @}
/*EOF*/
