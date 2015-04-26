// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the macros to handle the LEDs on the RZUSBSTICK.
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
 * $Id: led.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef RZUSBSTICK_LED_H
#define RZUSBSTICK_LED_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "board.h"
/*================================= MACROS           =========================*/

/*! \brief This is a function-like macro that enables the LEDs and turn them all off.  
 */
#define LED_INIT() do {                  \
    LED_BLUE_DDR   |= (1 << LED_BLUE);   \
    LED_GREEN_DDR  |= (1 << LED_GREEN);  \
    LED_ORANGE_DDR |= (1 << LED_ORANGE); \
    LED_RED_DDR    |= (1 << LED_RED);    \
                                         \
    LED_BLUE_OFF();                      \
    LED_GREEN_OFF();                     \
    LED_ORANGE_OFF();                    \
    LED_RED_OFF();                       \
} while (0)

#define LED_BLUE_ON()    (LED_BLUE_PORT |= (1 << LED_BLUE))      //!< Macro used to turn BLUE LED on.
#define LED_BLUE_OFF()   (LED_BLUE_PORT &= ~(1 << LED_BLUE))     //!< Macro used to turn BLUE LED off.
#define LED_BLUE_TOGGLE() (LED_BLUE_PORT ^= (1 << LED_BLUE))

#define LED_GREEN_ON()   (LED_GREEN_PORT &= ~(1 << LED_GREEN))   //!< Macro used to turn GREEN LED on.
#define LED_GREEN_OFF()  (LED_GREEN_PORT |= (1 << LED_GREEN))    //!< Macro used to turn GREEN LED off.
#define LED_GREEN_TOGGLE() (LED_GREEN_PORT ^= (1 << LED_GREEN))

#define LED_ORANGE_ON()  (LED_ORANGE_PORT &= ~(1 << LED_ORANGE)) //!< Macro used to turn ORANGE LED on.
#define LED_ORANGE_OFF() (LED_ORANGE_PORT |= (1 << LED_ORANGE))  //!< Macro used to turn ORANGE LED off.
#define LED_ORANGE_TOGGLE() (LED_ORANGE_PORT ^= (1 << LED_ORANGE))

#define LED_RED_ON()     (LED_RED_PORT &= ~(1 << LED_RED))       //!< Macro used to turn RED LED on.
#define LED_RED_OFF()    (LED_RED_PORT |= (1 << LED_RED))        //!< Macro used to turn RED LED off.
#define LED_RED_TOGGLE() (LED_RED_PORT ^= (1 << LED_RED))
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#endif
/*EOF*/
