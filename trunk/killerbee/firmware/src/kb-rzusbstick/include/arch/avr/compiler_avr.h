// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief This file implements some macros that makes the IAR C-compiler and
 *        avr-gcc work with the same code base for the AVR architecture
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
 * $Id: compiler_avr.h 41567 2008-05-13 12:52:19Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef COMPILER_AVR_H
#define COMPILER_AVR_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/** This macro will protect the following code from interrupts.*/
#define ENTER_CRITICAL_REGION() uint8_t volatile saved_sreg = SREG; \
                                     cli();

/** This macro must always be used in conjunction with ENTER_CRITICAL_REGION
    so that interrupts are enabled again.*/
#define LEAVE_CRITICAL_REGION() SREG = saved_sreg;

#if defined(__ICCAVR__)

#include <inavr.h>
#include <ioavr.h>
#include <intrinsics.h>
#include <pgmspace.h>

#ifndef __HAS_ELPM__
#define _MEMATTR  __flash
#else /* __HAS_ELPM__ */
#define _MEMATTR  __farflash
#endif /* __HAS_ELPM__ */


#define delay_us( us )   __delay_cycles((F_CPU / 1000000UL) * (us))

/**
 * Some preprocessor magic to allow for a header file abstraction of
 * interrupt service routine declarations for the IAR compiler.  This
 * requires the use of the C99 _Pragma() directive (rather than the
 * old #pragma one that could not be used as a macro replacement), as
 * well as two different levels of preprocessor concetanations in
 * order to do both, assign the correct interrupt vector name, as well
 * as construct a unique function name for the ISR.
 *
 * Do *NOT* try to reorder the macros below, or you'll suddenly find
 * out about all kinds of IAR bugs...
 */
#define PRAGMA(x) _Pragma(#x)
#define ISR(vec) PRAGMA(vector=vec) __interrupt void handler_##vec(void)
#define sei() (__enable_interrupt( ))
#define cli() (__disable_interrupt( ))

#define nop() (__no_operation())

#define INLINE PRAGMA(inline=forced) static

#define PROGMEM_LOCATION(var, loc) const _MEMATTR var @ loc
#define PROGMEM_DECLARE(x) _MEMATTR x
#define PROGMEM_STRING(x) ((_MEMATTR const char *)(x))
#define PROGMEM_STRING_T  char const _MEMATTR *
#define PROGMEM_T const _MEMATTR
#define PROGMEM_PTR_T const _MEMATTR *
#define PROGMEM_BYTE_ARRAY_T uint8_t const _MEMATTR *
#define PROGMEM_WORD_ARRAY_T uint16_t const _MEMATTR *
#define PROGMEM_READ_BYTE(x) *(x)
#define PROGMEM_READ_WORD(x) *(x)

#define EEGET(var, adr) __EEGET(var, adr)
#define EEPUT(adr, val) __EEPUT(adr, val)

#define SHORTENUM /**/

#elif defined(__GNUC__)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define delay_us(us)   (_delay_us( us ))

#define INLINE static inline

#define nop()   do {__asm__ __volatile__ ("nop");} while (0)

#define PROGMEM_LOCATION(var, loc) var __attribute__((section (#loc)))
#define PROGMEM_DECLARE(x) x __attribute__((__progmem__))
#define PROGMEM_STRING(x) PSTR(x)
#define PROGMEM_STRING_T  PGM_P
#define PROGMEM_T
#define PROGMEM_PTR_T *
#define PROGMEM_BYTE_ARRAY_T uint8_t*
#define PROGMEM_WORD_ARRAY_T uint16_t*
#define PROGMEM_READ_BYTE(x) pgm_read_byte(x)
#define PROGMEM_READ_WORD(x) pgm_read_word(x)

#define EEGET(var, addr) (var) = eeprom_read_byte ((uint8_t *)(addr))
#define EEPUT(addr, var) eeprom_write_byte ((uint8_t *)(addr), var)

#define SHORTENUM __attribute__ ((packed))
#else
#error Compiler not supported.
#endif
#endif
/* EOF */
