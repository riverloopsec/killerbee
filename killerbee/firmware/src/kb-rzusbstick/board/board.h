// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file includes the correct board definitions (HW configuration)
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
 * $Id: board.h 41219 2008-05-01 10:51:43Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef BOARD_H
#define BOARD_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#if defined(RZUSBSTICK)
#include    "board_rzusbstick.h"



#elif defined(AVRRAVEN_1284P)
#include    "board_avrraven_1284p.h"



#elif defined(AVRRAVEN_3290P)
#include    "board_avrraven_3290p.h"



#elif defined(XMEGA_TEST)
#include    "board_xmega_test.h"



#elif defined(RZ600_USB)
#include    "board_rz600_usb.h"



#elif defined(SAM7_TEST)
#include    "board_sam7_test.h"



#else
#error  'This files should only be compiled with the BOARD switch set.'
#endif
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
#endif
/* EOF */
