// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  API for the BCD algorithms
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
* $Id: bcd.h 41436 2008-05-09 00:02:43Z hmyklebust $
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __BCD_H__
#define __BCD_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

/*========================= CONSTANT DEFINES         =========================*/

/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Convert integer(unsigned 16 bit) to Binary Coded Decimal number
 *
 *  \param[in]  integer     Integer to convert
 *
 *  \returns    BCD coded integer
 */
uint16_t itobcd(uint16_t integer);

/*! \brief  Convert Binary Coded Decimal number to integer(unsigned 16 bit)
 *
 *  \param[in]  bcd_number     Binary Coded Decimal number to convert
 *
 *  \returns    Integer
 */
uint16_t bcdtoi(uint16_t bcd_number);

#endif // __BCD_H__

/*EOF*/
