// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the possible external configuration of the USB
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
 * $Id: conf_usb.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef _CONF_USB_H_
#define _CONF_USB_H_

#include <stdint.h>
#include <stdbool.h>

#define Usb_unicode(a)			((uint16_t)(a))


//_____ U S B    D E S C R I P T O R    T A B L E S ________________________


#define NB_ENDPOINTS	3  // number of endpoints in the application
#define EP_OUT			2
#define EP_IN			4

//#define EP_EVENT	    6
#define EP_EVENT		1

#endif  //! _CONF_USB_H_
