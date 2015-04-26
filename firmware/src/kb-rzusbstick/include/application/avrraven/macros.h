// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains commonly used macros
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
 * $Id: macros.h 41763 2008-05-15 18:08:50Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __MACROS_H__
#define __MACROS_H__

#include "self_programming.h"

#define INC_PIN_7(pin,...) (1<<pin) | INC_PIN_6(__VA_ARGS__)
#define INC_PIN_6(pin,...) (1<<pin) | INC_PIN_5(__VA_ARGS__)
#define INC_PIN_5(pin,...) (1<<pin) | INC_PIN_4(__VA_ARGS__)
#define INC_PIN_4(pin,...) (1<<pin) | INC_PIN_3(__VA_ARGS__)
#define INC_PIN_3(pin,...) (1<<pin) | INC_PIN_2(__VA_ARGS__)
#define INC_PIN_2(pin,...) (1<<pin) | INC_PIN_1(__VA_ARGS__)
#define INC_PIN_1(pin)     (1<<pin)

#define PORT(letter) PORT##letter
#define DDR(letter) DDR##letter

#define IO_INPUT_NO_PULLUP(letter,nmb,...)      DDR(letter) &= ~(INC_PIN_##nmb(__VA_ARGS__)); IO_CLEAR(letter,nmb,__VA_ARGS__)
#define IO_INPUT_WITH_PULLUP(letter,nmb,...)    DDR(letter) &= ~(INC_PIN_##nmb(__VA_ARGS__)); IO_SET(letter,nmb,__VA_ARGS__)
#define IO_SET(letter,nmb,...)                  PORT(letter) |= INC_PIN_##nmb(__VA_ARGS__)
#define IO_CLEAR(letter,nmb,...)                PORT(letter) &= ~(INC_PIN_##nmb(__VA_ARGS__))
#define IO_OUTPUT(letter,nmb,...)               DDR(letter) |= INC_PIN_##nmb(__VA_ARGS__)
#define IO_PUT(reg,val)                         reg = val;
#define REG_CLEAR(reg)                          reg = 0x00;

#define intvecs_to_app()                {uint8_t mcucr = (MCUCR & ~(1<<IVSEL)) & ~(1<<IVCE); MCUCR |= (1<<IVCE); MCUCR = mcucr;}
#define intvecs_to_boot()               {uint8_t mcucr = (MCUCR |  (1<<IVSEL)) & ~(1<<IVCE); MCUCR |= (1<<IVCE); MCUCR = mcucr;}

#ifdef RZUSBSTICK
#define watchdog_disable()              wdt_reset(); {uint8_t wdtcr = WDTCR; wdtcr |= (1<<WDCE); wdtcr &= ~(1<<WDE); WDTCR = wdtcr;}
#define watchdog_enable()               wdt_reset(); {uint8_t wdtcr = WDTCR; wdtcr |= (1<<WDCE)|(1<<WDE); WDTCR = wdtcr;}
#define watchdog_timeout_set(timeout)   wdt_reset(); {uint8_t wdtcr = WDTCR; wdtcr &= ~(((1<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0))); wdtcr |= (1<<WDCE)|timeout; WDTCR = wdtcr;}
#elif defined(AVRRAVEN_1284P)
#define WDT_TIMEOUT_2K      (((0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_4K      (((0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_8K      (((0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_16K     (((0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_32K     (((0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_64K     (((0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_128K    (((0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_256K    (((0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_512K    (((1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_1024K   (((1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define watchdog_disable()              wdt_reset(); {MCUSR &= ~(1<<WDRF); WDTCSR |= (1<<WDCE)|(1<<WDE); WDTCSR = 0x00;}
#define watchdog_enable()               wdt_reset(); {uint8_t wdtcsr = WDTCSR; wdtcsr |= (1<<WDCE)|(1<<WDE); WDTCSR = wdtcsr;}
#define watchdog_timeout_set(timeout)   wdt_reset(); {uint8_t wdtcsr = WDTCSR; wdtcsr &= ~(((1<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0))); wdtcsr |= (1<<WDCE)|timeout; WDTCSR = wdtcsr;}
 #elif defined(AVRRAVEN_1284P_BL)
#define WDT_TIMEOUT_2K      (((0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_4K      (((0<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_8K      (((0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_16K     (((0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_32K     (((0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_64K     (((0<<WDP3)|(1<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_128K    (((0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_256K    (((0<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_512K    (((1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_1024K   (((1<<WDP3)|(0<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define watchdog_disable()              wdt_reset(); {MCUSR &= ~(1<<WDRF); WDTCSR |= (1<<WDCE)|(1<<WDE); WDTCSR = 0x00;}
#define watchdog_enable()               wdt_reset(); {uint8_t wdtcsr = WDTCSR; wdtcsr |= (1<<WDCE)|(1<<WDE); WDTCSR = wdtcsr;}
#define watchdog_timeout_set(timeout)   wdt_reset(); {uint8_t wdtcsr = WDTCSR; wdtcsr &= ~(((1<<WDP3)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0))); wdtcsr |= (1<<WDCE)|timeout; WDTCSR = wdtcsr;}
#elif defined (AVRRAVEN_3290P)
#define WDT_TIMEOUT_16K      (((0<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_32K      (((0<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_64K      (((0<<WDP2)|(1<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_128K     (((0<<WDP2)|(1<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_256K     (((1<<WDP2)|(0<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_512K     (((1<<WDP2)|(0<<WDP1)|(1<<WDP0)))
#define WDT_TIMEOUT_1024K    (((1<<WDP2)|(1<<WDP1)|(0<<WDP0)))
#define WDT_TIMEOUT_2048K    (((1<<WDP2)|(1<<WDP1)|(1<<WDP0)))
#define watchdog_disable()              wdt_reset(); {uint8_t wdtcr = WDTCR; wdtcr |= (1<<WDCE); wdtcr &= ~(1<<WDE); WDTCR = wdtcr; WDTCR = wdtcr;}
#define watchdog_enable()               wdt_reset(); {uint8_t wdtcr = WDTCR; wdtcr |= (1<<WDCE)|(1<<WDE); WDTCR = wdtcr;}
#define watchdog_timeout_set(timeout)   wdt_reset(); {uint8_t wdtcr = WDTCR; wdtcr &= ~(((1<<WDP2)|(1<<WDP1)|(1<<WDP0))); wdtcr |= (1<<WDCE)|timeout; WDTCR = wdtcr;}
#else
#error 'configuration not supported.'
#endif



//! Macro for more readable firmware revision
#define REVISION_TO_INT16(maj, min)     ((maj<<8)|min)


#endif // __MACROS_H__

/*EOF*/
