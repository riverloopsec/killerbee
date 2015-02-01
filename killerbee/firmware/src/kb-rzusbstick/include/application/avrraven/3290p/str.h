// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Some string/integer functions
 *
 *              The functions provided here is specialized/smaller versions of
 *              the functions found in the ANSI C runtime library
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
 * $Id: vrt_timer_xmega_conf.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __STR_H__
#define __STR_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

/*========================= CONSTANT DEFINES         =========================*/

/*========================= MACROS DEFINES           =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Convert integer into string
 *
 *              The integer is converted into a fixed size string (hex)
 *
 *  \param[out] string      Character buffer to store the converted integer(hex)
 *  \param[in]  integer     Integer to convert and insert
 *  \param[in]  digit_count Size of integer
 *
 */
void itos(int integer, unsigned char* string, int8_t digit_count);


/*! \brief  Insert integer into string
 *
 *              The integer is converted into a fixed size string (hex) and written to
 *              the string at a given offset
 *
 *  \param[out] string      String to modify
 *  \param[in]  integer     Integer to convert and insert
 *  \param[in]  offset      Position to insert the integer
 *  \param[in]  digit_count Size of integer
 *
 */
void insi(unsigned char* string, uint16_t integer, int8_t offset, int8_t digit_count);


#endif // __STR_H__
