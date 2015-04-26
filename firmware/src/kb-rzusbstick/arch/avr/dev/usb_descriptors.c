// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the usb parameters that uniquely identify the
 *  application through descriptor tables
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
 * $Id: usb_descriptors.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

//_____ I N C L U D E S ____________________________________________________
#include "conf_usb.h"

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"


//_____ M A C R O S ________________________________________________________




//_____ D E F I N I T I O N ________________________________________________

// usb_user_device_descriptor
S_usb_device_descriptor usb_dev_desc =
{
  sizeof(usb_dev_desc)
, DEVICE_DESCRIPTOR
, Usb_write_word_enum_struc(USB_SPECIFICATION)
, DEVICE_CLASS
, DEVICE_SUB_CLASS
, DEVICE_PROTOCOL
, EP_CONTROL_LENGTH
, Usb_write_word_enum_struc(VENDOR_ID)
, Usb_write_word_enum_struc(PRODUCT_ID)
, Usb_write_word_enum_struc(RELEASE_NUMBER)
, MAN_INDEX
, PROD_INDEX
, SN_INDEX
, NB_CONFIGURATION
};

// usb_user_configuration_descriptor FS
S_usb_user_configuration_descriptor usb_conf_desc = {
 { sizeof(S_usb_configuration_descriptor)
 , CONFIGURATION_DESCRIPTOR
 , Usb_write_word_enum_struc(sizeof(usb_conf_desc))
 , NB_INTERFACE
 , CONF_NB
 , CONF_INDEX
 , CONF_ATTRIBUTES
 , MAX_POWER
 }
 ,
 { sizeof(S_usb_interface_descriptor)
 , INTERFACE_DESCRIPTOR
 , INTERFACE_NB
 , ALTERNATE
 , NB_ENDPOINT
 , INTERFACE_CLASS
 , INTERFACE_SUB_CLASS
 , INTERFACE_PROTOCOL
 , INTERFACE_INDEX
 }
 ,
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_1
 , EP_ATTRIBUTES_1
 , Usb_write_word_enum_struc(EP_SIZE_1)
 , EP_INTERVAL_1
 }
 ,
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_2
 , EP_ATTRIBUTES_2
 , Usb_write_word_enum_struc(EP_SIZE_2)
 , EP_INTERVAL_2
 }
 ,
 /*
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_3
 , EP_ATTRIBUTES_3
 , Usb_write_word_enum_struc(EP_SIZE_3)
 , EP_INTERVAL_3
 }
 ,
 */
 { sizeof(S_usb_endpoint_descriptor)
 , ENDPOINT_DESCRIPTOR
 , ENDPOINT_NB_4
 , EP_ATTRIBUTES_4
 , Usb_write_word_enum_struc(EP_SIZE_4)
 , EP_INTERVAL_4
 }
};



                                      // usb_user_manufacturer_string_descriptor
S_usb_manufacturer_string_descriptor usb_user_manufacturer_string_descriptor = {
  sizeof(usb_user_manufacturer_string_descriptor)
, STRING_DESCRIPTOR
, USB_MANUFACTURER_NAME
};


                                      // usb_user_product_string_descriptor

S_usb_product_string_descriptor usb_user_product_string_descriptor = {
  sizeof(usb_user_product_string_descriptor)
, STRING_DESCRIPTOR
, USB_PRODUCT_NAME
};


                                      // usb_user_serial_number

S_usb_serial_number usb_user_serial_number = {
  sizeof(usb_user_serial_number)
, STRING_DESCRIPTOR
, USB_SERIAL_NUMBER
};


                                      // usb_user_language_id

S_usb_language_id usb_user_language_id = {
  sizeof(usb_user_language_id)
, STRING_DESCRIPTOR
, Usb_write_word_enum_struc(LANGUAGE_ID)
};



