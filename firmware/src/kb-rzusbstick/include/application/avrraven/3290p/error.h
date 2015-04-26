// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Error handling routines for the AVR Raven
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

#ifndef __ERROR_H__
#define __ERROR_H__

/*========================= INCLUDES                 =========================*/

/*========================= MACROS                   =========================*/
//! Timeout for system reboot on error
#define ERROR_REBOOT_TIMEOUT    100000000

//! Timeout for user interraction on error (for contiuing execution (Dangerous!!))
#define ERROR_CONTINUE_TIMEOUT  200000

//@{ Error output. Prints current function and line
#ifdef AVRRAVEN_DEBUG
#define ERROR() error_handler(__FUNCTION__, __LINE__)
#else
#define ERROR() while(1){;}
#endif
//@}

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief  Error handler
 *
 *          User defined behauviour. May use the unput string and a number
 *          to display information surch as \ref ERROR()
 *
 *  \param[in]  str             String
 *  \param[in]  num             Number
 *
 */
extern void error_handler(const char* str, int num);

#endif // __ERROR_H__
/*EOF*/
