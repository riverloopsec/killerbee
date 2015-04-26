// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  ..............
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
 * $Id: usb_standard_request.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/


#ifndef _USB_ENUM_H_
#define _USB_ENUM_H_

//_____ I N C L U D E S ____________________________________________________
//#include "config.h"
#include "usb_descriptors.h"

//_____ M A C R O S ________________________________________________________

//_____ S T A N D A R D    D E F I N I T I O N S ___________________________
                    //! Standard Requests

#define GET_STATUS                     0x00
#define GET_DEVICE                     0x01
#define CLEAR_FEATURE                  0x01           //!< see FEATURES below
#define GET_STRING                     0x03
#define SET_FEATURE                    0x03           //!< see FEATURES below
#define SET_ADDRESS                    0x05
#define GET_DESCRIPTOR                 0x06
#define SET_DESCRIPTOR                 0x07
#define GET_CONFIGURATION              0x08
#define SET_CONFIGURATION              0x09
#define GET_INTERFACE                  0x0A
#define SET_INTERFACE                  0x0B
#define SYNCH_FRAME                    0x0C

#define GET_DEVICE_DESCRIPTOR             1
#define GET_CONFIGURATION_DESCRIPTOR      4

#define REQUEST_DEVICE_STATUS          0x80
#define REQUEST_INTERFACE_STATUS       0x81
#define REQUEST_ENDPOINT_STATUS        0x82
#define ZERO_TYPE                      0x00
#define INTERFACE_TYPE                 0x01
#define ENDPOINT_TYPE                  0x02

                     //! Descriptor Types

#define DEVICE_DESCRIPTOR                     0x01
#define CONFIGURATION_DESCRIPTOR              0x02
#define STRING_DESCRIPTOR                     0x03
#define INTERFACE_DESCRIPTOR                  0x04
#define ENDPOINT_DESCRIPTOR                   0x05
#define DEVICE_QUALIFIER_DESCRIPTOR           0x06
#define OTHER_SPEED_CONFIGURATION_DESCRIPTOR  0x07



                    //! Standard Features

#define FEATURE_DEVICE_REMOTE_WAKEUP   0x01
#define FEATURE_ENDPOINT_HALT          0x00

#define TEST_J                         0x01
#define TEST_K                         0x02
#define TEST_SEO_NAK                   0x03
#define TEST_PACKET                    0x04
#define TEST_FORCE_ENABLE              0x05


                        //! Device Status
#define BUS_POWERED                       0
#define SELF_POWERED                      1

                         //! Device State

#define ATTACHED                          0
#define POWERED                           1
#define DEFAULT                           2
#define ADDRESSED                         3
#define CONFIGURED                        4
#define SUSPENDED                         5

#define USB_CONFIG_ATTRIBUTES_RESERVED    0x80
#define USB_CONFIG_BUSPOWERED            (USB_CONFIG_ATTRIBUTES_RESERVED | 0x00)
#define USB_CONFIG_SELFPOWERED           (USB_CONFIG_ATTRIBUTES_RESERVED | 0x40)
#define USB_CONFIG_REMOTEWAKEUP          (USB_CONFIG_ATTRIBUTES_RESERVED | 0x20)

//_____ U S B   S T R U C T U R E S _________________________________________



//_____ D E C L A R A T I O N ______________________________________________

void    usb_var_init(        void);
void    usb_process_request( void);

#endif  // _USB_ENUM_H_

