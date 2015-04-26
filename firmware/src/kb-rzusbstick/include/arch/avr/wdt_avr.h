// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file conatains low level access functions for the AVR watchdog module
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
 * $Id: wdt_avr.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef WDT_AVR_H
#define WDT_AVR_H

#if defined( __ICCAVR__ )
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*================================= MACROS           =========================*/
#define wdt_reset() (__watchdog_reset())

#define wdt_disable() do {              \
    uint8_t volatile sreg_temp = SREG;  \
    cli();                              \
    __watchdog_reset();                 \
    MCUSR &= ~(1 << WDRF);              \
    WDTCSR |= (1 << WDCE) | (1 << WDE); \
    WDTCSR = 0x00;                      \
    SREG = sreg_temp;                   \
} while (0)

#define wdt_enable(timeout) do {        \
    uint8_t volatile sreg_temp = SREG;  \
    cli();                              \
    __watchdog_reset();                 \
    WDTCSR |= (1 << WDCE) | (1 << WDE); \
    WDTCSR = (1 << WDE) | timeout;      \
    SREG = sreg_temp;                   \
} while (0)

/*================================= TYEPDEFS         =========================*/
typedef enum WDT_AVR_TIMEOUT_ENUM {
    WDTO_15MS  = 0,
    WDTO_30MS  = 1,
    WDTO_60MS  = 2,
    WDTO_120MS = 3,
    WDTO_250MS = 4,
    WDTO_500MS = 5,
    WDTO_1S    = 6,
    WDTO_2S    = 7,
    WDTO_4S    = 8,
    WDTO_8S    = 9,
} wdt_avr_timeout_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#elif defined(__GNUC__)
#include <avr/wdt.h>
#else
#error 'Compiler not supported.'
#endif
#endif
/*EOF*/
