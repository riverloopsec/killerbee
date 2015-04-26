// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This header file implements a low level sleep driver for the AVR 
 *         microcontroller. Currently the driver supports the IAR C compiler and
 *         AVR-GCC
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
 * $Id: sleep_avr.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef SLEEP_AVR_H
#define SLEEP_AVR_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*================================= MACROS           =========================*/
#define AVR_SLEEP_CTRL_REG  (SMCR) //!< Sleep control register.
/*================================= TYEPDEFS         =========================*/

/*! \brief Selectable modes for the AVR's power save module. It is up to the user
 *         to ensure that the selected mode is available, since this might vary 
 *         from part to part.
 */
typedef enum AVR_SLEEP_MODE_ENUM_TAG {
    SLEEP_IDLE                = 0, //!< CPU and flash clock stopped.
    SLEEP_ADC_NOISE_REDUCTION = 1, //!< IO clock stopped.
    SLEEP_POWER_DOWN          = 2, //!< Main clock off, lowest power mode.
    SLEEP_POWER_SAVE          = 3, //!< Main clock off, 32KHz running.
    SLEEP_RESERVED_1          = 4,
    SLEEP_RESERVED_2          = 5,
    SLEEP_STANDBY             = 6, //!< All off but with main clock running.
    SLEEP_EXTENDED_STANDBY    = 7, //!< All off, but with main clock and 32 KHz running.
} avr_sleep_mode_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#if defined(DOXYGEN)
void avr_set_sleep_mode(uint8_t mode);
void avr_sleep_enable(void);
void avr_sleep_disable(void);
void avr_mcu_sleep(void);
#endif



#define avr_set_sleep_mode(mode) do {                                           \
        AVR_SLEEP_CTRL_REG = ((AVR_SLEEP_CTRL_REG & ~((1 << SM0) | (1 << SM1) | \
                             (1 << SM2))) | (mode));                            \
} while(0)



#define avr_sleep_enable() do {                   \
        AVR_SLEEP_CTRL_REG |= (uint8_t)(1 << SE); \
} while(0)



#define avr_sleep_disable() do {                     \
        AVR_SLEEP_CTRL_REG &= (uint8_t)(~(1 << SE)); \
} while(0)



#if defined(__ICCAVR__)



#define avr_mcu_sleep() do { __sleep(); } while(0)



#elif defined(__GNUC__)



#define avr_mcu_sleep() do {                        \
        __asm__ __volatile__ ( "sleep" "\n\t" :: ); \
} while(0)



#else
#error  'Compiler not supported.'
#endif
#endif
/*EOF*/
