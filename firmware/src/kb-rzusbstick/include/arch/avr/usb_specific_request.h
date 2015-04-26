// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the user call-back functions corresponding to the
 *         application
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
 * $Id: usb_specific_request.h 41220 2008-05-01 13:43:18Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef _USB_USER_ENUM_H_
#define _USB_USER_ENUM_H_

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdint.h>
#include <stdbool.h>

/*_____ M A C R O S ________________________________________________________*/

extern S_usb_device_descriptor usb_dev_desc;
extern S_usb_user_configuration_descriptor usb_conf_desc;
extern S_usb_user_configuration_descriptor usb_other_conf_desc;
extern S_usb_device_qualifier_descriptor usb_qual_desc;
extern S_usb_manufacturer_string_descriptor usb_user_manufacturer_string_descriptor;
extern S_usb_product_string_descriptor usb_user_product_string_descriptor;
extern S_usb_serial_number usb_user_serial_number;
extern S_usb_language_id usb_user_language_id;



/*_____ D E F I N I T I O N ________________________________________________*/
bool  usb_user_read_request(uint8_t, uint8_t);
bool  usb_user_get_descriptor(uint8_t , uint8_t);
void  usb_user_endpoint_init(void);
#endif

