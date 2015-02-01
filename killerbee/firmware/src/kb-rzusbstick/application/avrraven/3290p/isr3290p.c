// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  ISR's for the ATmega3290p on AVR Raven
*
*
*
* \par Application note:
*      AVR2017: RZRAVEN FW
*
* \par Documentation
*
* \author
*      Atmel Corporation: http://www.atmel.com \n
*      Support email: avr@atmel.com
*
* $Id: avrraven_3290p.h 41156 2008-04-29 16:11:31Z hmyklebust $
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "tick.h"

/*================================= MACROS           =========================*/

/*================================= TYEPDEFS         =========================*/

/*================================= GLOBAL VARIABLES =========================*/

/*================================= LOCAL VARIABLES  =========================*/

/*================================= PROTOTYPES       =========================*/

ISR(TIMER2_OVF_vect)
{
    tick_handler();
}
/*EOF*/
