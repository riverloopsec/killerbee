// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Configuration file for spi.c
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
 * $Id: spi_conf.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __SPI_CONF_H__
#define __SPI_CONF_H__

/*========================= INCLUDES                 =========================*/

/*========================= CONSTANT DEFINES         =========================*/


#ifdef AVRRAVEN_3290P

#define PORTSPI         PORTB
#define DDRSPI          DDRB

#define SS              PORTB0
#define SCK             PORTB1
#define MOSI            PORTB2
#define MISO            PORTB3

//#define SPI_BUFFER_SIZE 32
//#define USE_SPI_BUFFER

#else
#error "SPI configuration not supported."
#endif

/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/


#endif // __SPI_CONF_H__
