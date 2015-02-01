// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the low level macros and definition for the PLL
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
 * $Id: pll_drv.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef PLL_DRV_H
#define PLL_DRV_H

//_____ I N C L U D E S ____________________________________________________

//_____ M A C R O S ________________________________________________________

   
#define PLLx24 			( (0<<PLLP2) | (0<<PLLP1) | (0<<PLLP0) )
#define PLLx12 			( (0<<PLLP2) | (0<<PLLP1) | (1<<PLLP0) )
#define PLLx08 			( (0<<PLLP2) | (1<<PLLP1) | (0<<PLLP0) )
#define PLLx06 			( (0<<PLLP2) | (1<<PLLP1) | (1<<PLLP0) )
#define PLLx04 			( (1<<PLLP2) | (0<<PLLP1) | (0<<PLLP0) )
#define PLLx03 			( (1<<PLLP2) | (0<<PLLP1) | (1<<PLLP0) )

//! Start the PLL at only 48 MHz, regarding CPU frequency


#define Start_pll(clockfactor)                                                  \
           (PLLCSR = ( clockfactor  | (1<<PLLE)  ))

#define Is_pll_ready()       (PLLCSR & (1<<PLOCK) )

#define Wait_pll_ready()     while (!(PLLCSR & (1<<PLOCK)))

#define Stop_pll()           (PLLCSR  &= (~(1<<PLOCK)) )            //!< Stop the PLL

#if   (F_CPU==2000000UL)
   #define Pll_start_auto()   Start_pll(PLLx24)
#elif (F_CPU==4000000UL)
   #define Pll_start_auto()   Start_pll(PLLx12)
#elif (F_CPU==6000000UL)
   #define Pll_start_auto()   Start_pll(PLLx08)
#elif (F_CPU==8000000UL)
   #define Pll_start_auto()   Start_pll(PLLx06)
#elif (F_CPU==12000000UL)
   #define Pll_start_auto()   Start_pll(PLLx04)
#elif (F_CPU==16000000UL)
   #define Pll_start_auto()   Start_pll(PLLx03)
#else
   #error   "FOSC should be defined in config.h"
#endif



//_____ D E F I N I T I O N S ______________________________________________

//_____ F U N C T I O N S __________________________________________________

#endif  // PLL_DRV_H


