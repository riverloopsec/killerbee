/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief This file includes the correct compiler definitions for the different 
 *        architectures.
 *
 * \par Application note:
 *      AVR2017: RZRAVEN
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler 
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 * 
 * $Id: compiler.h 41160 2008-04-29 18:26:40Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 ******************************************************************************/

#ifndef COMPILER_H
#define COMPILER_H

    #if defined(AVR)
        #include "compiler_avr.h"
    #elif defined(XMEGA)
        #include "compiler_xmega.h"
    #elif defined(AVR32)
        #include "compiler_avr32.h"
    #elif defined(SAM7)
        #include "compiler_sam7.h"
    #else
        #error This files should only be compiled with the ARCHITECTURE set.
    #endif

    /* The following list of function like macros must be defined for each
     * architecture. There might be other macros defined for each architecture
     * but these can only be used in microcontroller dependant code.
     */



#if defined(DOXYGEN)



/*! \brief This macro will protect the proceeding source code from interrupts. */
void ENTER_CRITICAL_REGION(void);

/*! \brief This macro must always be used in conjunction with ENTER_CRITICAL_REGION()
 *         so that interrupts are enabled again.
 */
void LEAVE_CRITICAL_REGION(void);

/*! \brief Delay for \c us microseconds (Busy waiting).
 *
 * The macro F_CPU is supposed to be defined to a constant defining the CPU
 * clock frequency (in Hertz).
 *
 * \note For the IAR compiler, currently F_CPU must be a
 *       multiple of 1000000UL (1 MHz).
 *
 * \param[in] us Delay in microseconds.
 */
void delay_us(uint16_t us);

/*! \brief This macro defines a pointer to a string in program memory, if that
 *         is supported by the architecture. If not, this will be a pointer in
 *         data memory.
 */
#define PROGMEM_STRING_T



#endif



#endif
/*EOF*/
