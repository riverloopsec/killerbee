// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Configuration file for the Transceiver Access Toolbox
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
 * $Id: tat_conf.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef TAT_CONF_H
#define TAT_CONF_H

//! \addtogroup grTAT
//! @{
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
/*================================= MACROS           =========================*/
#define TAT_MIN_CHANNEL (11) //!< Lowest valid channel.
#define TAT_MAX_CHANNEL (26) //!< Highest valid channel.

#define TAT_MIN_FRAME_LENGTH (0x08) //!< Shortest frame length in bytes that the TAT will signal as valid.
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/* EOF */
