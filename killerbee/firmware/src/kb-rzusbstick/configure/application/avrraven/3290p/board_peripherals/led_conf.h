// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven LED driver
 *
 * \defgroup grLCDLowLevel AVRRAVEN LCD Low Level Driver
 * \ingroup grLowLevelDrivers
 *
 * Low level driver for the AVRRAVEN LCD.
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
 * $Id: lcd.h 41236 2008-05-02 16:41:57Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __LCD_H__
#define __LCD_H__

/*========================= INCLUDES                 =========================*/
//! \addtogroup grLEDLowLevel
//! @{
/*========================= MACROS                   =========================*/

#if defined(AVRRAVEN_3290P)
#define LED_TIMER_FRQ                32768UL
#define LED_TIMER_PRESCALE           1

#define LED_TIMER_MAX_VALUE          256
#define LED_TIMER_SECOUND_PRESCALE   (LED_TIMER_FRQ/LED_TIMER_PRESCALE/LED_TIMER_MAX_VALUE)

#else
#error "LED configuration not supported."
#endif

//! @}
#endif // __LCD_H__
/*EOF*/
