// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Interface for AVRRAVEN's NTC.
 *
 * \defgroup grNTC Interface for the AVRRAVEN's NTC resistor
 * \ingroup grHardwareInterfaceLayer
 *
 * The AVRRAVEN has an onboard NTC resistor that can be used to measure 
 * temperature.
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
 * $Id: temp.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __TEMP_H__
#define __TEMP_H__
/*========================= INCLUDES                 =========================*/


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "compiler.h"

#include "adc.h"

//! \addtogroup grNTC
//! @{
/*========================= MACROS                   =========================*/


/*========================= TYEPDEFS                 =========================*/
//! Type used with \ref temp_get() to select temperature unit
typedef enum {
    TEMP_UNIT_CELCIUS,
    TEMP_UNIT_FAHRENHEIT
} temp_unit_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
/*! \brief          Initialize the temperature sensor
 *
 * \return                              EOF on error
 */
int temp_init(void);


/*! \brief          De-initialize the temperature sensor
 *
 */
void temp_deinit(void);

/*! \brief          Read current temperature
 *
 * \param[in]		unit			Selected temperature unit (\ref temp_unit_t)
 *
 *
 * \return                              EOF on error
 */
int16_t temp_get(temp_unit_t unit);
//! @}
#endif // __TEMP_H__
/*EOF*/
