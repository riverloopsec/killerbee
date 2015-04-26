// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file holds the configuration parameters for the SIPC driver used
 *         in the AVRRaven project
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
 * $Id: sipc_conf.h 41241 2008-05-03 17:07:52Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef SIPC_CONF_H
#define SIPC_CONF_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*================================= MACROS           =========================*/
#define SIPC_SOF      (0x02) //!< Unique start of frame delimiter.
#define SIPC_EOF      (0x03) //!< Unique end of frame delimiter.
#define SIPC_ESC      (0x17) //!< Unique byte used to indicate a stuffed byte.
#define SIPC_ESC_MASK (0x40) //!< Value used to OR together with the stuffed byte.

#define SIPC_PACKET_SIZE (256) //!< Maximum packet size that SIPC can handle.
/*================================= TYEPDEFS         =========================*/
typedef uint8_t sipc_size_t; //!< Size variable type for the buffer used for the SIPC receiver.
typedef uint16_t sipc_timeout_t; //!< Timeout variable type.
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#endif
/*EOF*/
