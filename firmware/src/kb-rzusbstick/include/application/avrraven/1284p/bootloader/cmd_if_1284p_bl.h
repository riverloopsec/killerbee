// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven ATmega1284p bootloader command interface
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
 * $Id: cmd_if_1284p_bl.h 41519 2008-05-13 08:30:30Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef CMD_IF_CONFIG_H
#define CMD_IF_CONFIG_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

/*================================= MACROS           =========================*/

/*================================= TYEPDEFS         =========================*/

/*================================= GLOBAL VARIABLES =========================*/

/*================================= LOCAL VARIABLES  =========================*/

/*================================= PROTOTYPES       =========================*/
bool cmd_if_init(void);
void cmd_if_deinit(void);


#endif
/*EOF*/
