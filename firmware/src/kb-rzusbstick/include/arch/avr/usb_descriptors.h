// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  USB Identifers
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
 * $Id: usb_descriptors.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef _USB_USERCONFIG_H_
#define _USB_USERCONFIG_H_

//_____ I N C L U D E S ____________________________________________________
#include <stdint.h>
#include <stdbool.h>

#include "usb_standard_request.h"
#include "conf_usb.h"

//_____ M A C R O S ________________________________________________________

#define Usb_get_dev_desc_pointer()        (&(usb_dev_desc.bLength))
#define Usb_get_dev_desc_length()         (sizeof (usb_dev_desc))
#define Usb_get_conf_desc_pointer()       (&(usb_conf_desc.cfg.bLength))
#define Usb_get_conf_desc_length()        (sizeof (usb_conf_desc))

//_____ U S B    D E F I N E _______________________________________________

                  // USB Device descriptor
#define USB_SPECIFICATION     0x0200
#define DEVICE_CLASS          0x00      // CDC class
#define DEVICE_SUB_CLASS      0      	// each configuration has its own sub-class
#define DEVICE_PROTOCOL       0      	// each configuration has its own protocol
#define EP_CONTROL_LENGTH     64
#define VENDOR_ID             0x03EB	// Atmel vendor ID = 03EBh
#define PRODUCT_ID            0x210A
#define RELEASE_NUMBER        0x0200
#define MAN_INDEX             0x01
#define PROD_INDEX            0x02
#define SN_INDEX              0x03
#define NB_CONFIGURATION      1

               //  CONFIGURATION
#define NB_INTERFACE       1
#define CONF_NB            1
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_BUSPOWERED
#define MAX_POWER          250          // 100 mA

             // USB Interface descriptor
#define INTERFACE_NB        0
#define ALTERNATE           0
#define NB_ENDPOINT         3
#define INTERFACE_CLASS     0xFF			// Vendor specific
#define INTERFACE_SUB_CLASS 0x00
#define INTERFACE_PROTOCOL  0x00
#define INTERFACE_INDEX     0

             // USB Endpoint 1 descriptor FS
#define ENDPOINT_NB_1       (EP_IN | 0x80)
#define EP_ATTRIBUTES_1     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_IN_LENGTH        64
#define EP_SIZE_1           EP_IN_LENGTH
#define EP_INTERVAL_1       0x00 			


             // USB Endpoint 2 descriptor FS
#define ENDPOINT_NB_2       EP_OUT
#define EP_ATTRIBUTES_2     0x02          // BULK = 0x02, INTERUPT = 0x03
#define EP_OUT_LENGTH       64
#define EP_SIZE_2           EP_OUT_LENGTH
#define EP_INTERVAL_2       0x00 			

/*
            // USB Endpoint 3 descriptor FS
#define ENDPOINT_NB_3       (EP_EVENT | 0x80)
#define EP_ATTRIBUTES_3     TYPE_BULK          // BULK = 0x02, INTERUPT = 0x03
#define EP_EVENT_LENGTH       64
#define EP_SIZE_3           EP_EVENT_LENGTH
#define EP_INTERVAL_3       0x00
*/

// USB Endpoint 4 descriptor FS
#define ENDPOINT_NB_4       (EP_EVENT | 0x80)
#define EP_ATTRIBUTES_4     TYPE_BULK          // BULK = 0x02, INTERUPT = 0x03
#define EP_STREAM_LENGTH       64
#define EP_SIZE_4           EP_STREAM_LENGTH
#define EP_INTERVAL_4       0x00

#define EP_USE_DOUBLE_BUF   TRUE


#define DEVICE_STATUS         0x00 // TBD
#define INTERFACE_STATUS      0x00 // TBD

#define LANG_ID               0x00


#define USB_MN_LENGTH         5
#define USB_MANUFACTURER_NAME \
{ Usb_unicode('A') \
, Usb_unicode('T') \
, Usb_unicode('M') \
, Usb_unicode('E') \
, Usb_unicode('L') \
}

#define USB_PN_LENGTH         10
#define USB_PRODUCT_NAME \
{ Usb_unicode('K') \
 ,Usb_unicode('I') \
 ,Usb_unicode('L') \
 ,Usb_unicode('L') \
 ,Usb_unicode('E') \
 ,Usb_unicode('R') \
 ,Usb_unicode('B') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('1') \
}

#define USB_SN_LENGTH         0x0D
              // Serial Number should be at least 12 characters long
#define USB_SERIAL_NUMBER \
{ Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('0') \
 ,Usb_unicode('C') \
}


#define LANGUAGE_ID           0x0409


                  //! Usb Request
typedef struct
{
   uint8_t      bmRequestType;        //!< Characteristics of the request
   uint8_t      bRequest;             //!< Specific request
   uint16_t     wValue;               //!< field that varies according to request
   uint16_t     wIndex;               //!< field that varies according to request
   uint16_t     wLength;              //!< Number of bytes to transfer if Data
}  S_UsbRequest;

                //! Usb Device Descriptor
typedef struct {
   uint8_t      bLength;              //!< Size of this descriptor in bytes
   uint8_t      bDescriptorType;      //!< DEVICE descriptor type
   uint16_t     bscUSB;               //!< Binay Coded Decimal Spec. release
   uint8_t      bDeviceClass;         //!< Class code assigned by the USB
   uint8_t      bDeviceSubClass;      //!< Sub-class code assigned by the USB
   uint8_t      bDeviceProtocol;      //!< Protocol code assigned by the USB
   uint8_t      bMaxPacketSize0;      //!< Max packet size for EP0
   uint16_t     idVendor;             //!< Vendor ID. ATMEL = 0x03EB
   uint16_t     idProduct;            //!< Product ID assigned by the manufacturer
   uint16_t     bcdDevice;            //!< Device release number
   uint8_t      iManufacturer;        //!< Index of manu. string descriptor
   uint8_t      iProduct;             //!< Index of prod. string descriptor
   uint8_t      iSerialNumber;        //!< Index of S.N.  string descriptor
   uint8_t      bNumConfigurations;   //!< Number of possible configurations
}  S_usb_device_descriptor;


          //! Usb Configuration Descriptor
typedef struct {
   uint8_t      bLength;              //!< size of this descriptor in bytes
   uint8_t      bDescriptorType;      //!< CONFIGURATION descriptor type
   uint16_t     wTotalLength;         //!< total length of data returned
   uint8_t      bNumInterfaces;       //!< number of interfaces for this conf.
   uint8_t      bConfigurationValue;  //!< value for SetConfiguration resquest
   uint8_t      iConfiguration;       //!< index of string descriptor
   uint8_t      bmAttibutes;          //!< Configuration characteristics
   uint8_t      MaxPower;             //!< maximum power consumption
}  S_usb_configuration_descriptor;


              //! Usb Interface Descriptor
typedef struct {
   uint8_t      bLength;               //!< size of this descriptor in bytes
   uint8_t      bDescriptorType;       //!< INTERFACE descriptor type
   uint8_t      bInterfaceNumber;      //!< Number of interface
   uint8_t      bAlternateSetting;     //!< value to select alternate setting
   uint8_t      bNumEndpoints;         //!< Number of EP except EP 0
   uint8_t      bInterfaceClass;       //!< Class code assigned by the USB
   uint8_t      bInterfaceSubClass;    //!< Sub-class code assigned by the USB
   uint8_t      bInterfaceProtocol;    //!< Protocol code assigned by the USB
   uint8_t      iInterface;            //!< Index of string descriptor
}  S_usb_interface_descriptor;


               //! Usb Endpoint Descriptor
typedef struct {
   uint8_t      bLength;               //!< Size of this descriptor in bytes
   uint8_t      bDescriptorType;       //!< ENDPOINT descriptor type
   uint8_t      bEndpointAddress;      //!< Address of the endpoint
   uint8_t      bmAttributes;          //!< Endpoint's attributes
   uint16_t     wMaxPacketSize;        //!< Maximum packet size for this EP
   uint8_t      bInterval;             //!< Interval for polling EP in ms
} S_usb_endpoint_descriptor;


               //! Usb Device Qualifier Descriptor
typedef struct {
   uint8_t      bLength;               //!< Size of this descriptor in bytes
   uint8_t      bDescriptorType;       //!< Device Qualifier descriptor type
   uint16_t     bscUSB;                //!< Binay Coded Decimal Spec. release
   uint8_t      bDeviceClass;          //!< Class code assigned by the USB
   uint8_t      bDeviceSubClass;       //!< Sub-class code assigned by the USB
   uint8_t      bDeviceProtocol;       //!< Protocol code assigned by the USB
   uint8_t      bMaxPacketSize0;       //!< Max packet size for EP0
   uint8_t      bNumConfigurations;    //!< Number of possible configurations
   uint8_t      bReserved;             //!< Reserved for future use, must be zero
}  S_usb_device_qualifier_descriptor;


               //! Usb Language Descriptor
typedef struct {
   uint8_t      bLength;               //!< size of this descriptor in bytes
   uint8_t      bDescriptorType;       //!< STRING descriptor type
   uint16_t     wlangid;               //!< language id
}  S_usb_language_id;


//_____ U S B   M A N U F A C T U R E R   D E S C R I P T O R _______________


//struct usb_st_manufacturer
typedef struct {
   uint8_t  bLength;               // size of this descriptor in bytes
   uint8_t  bDescriptorType;       // STRING descriptor type
   uint16_t wstring[USB_MN_LENGTH];// unicode characters
} S_usb_manufacturer_string_descriptor;


//_____ U S B   P R O D U C T   D E S C R I P T O R _________________________


//struct usb_st_product
typedef struct {
   uint8_t  bLength;               // size of this descriptor in bytes
   uint8_t  bDescriptorType;       // STRING descriptor type
   uint16_t wstring[USB_PN_LENGTH];// unicode characters
} S_usb_product_string_descriptor;


//_____ U S B   S E R I A L   N U M B E R   D E S C R I P T O R _____________


//struct usb_st_serial_number
typedef struct {
   uint8_t  bLength;               // size of this descriptor in bytes
   uint8_t  bDescriptorType;       // STRING descriptor type
   uint16_t wstring[USB_SN_LENGTH];// unicode characters
} S_usb_serial_number;


/*_____ U S B   S T O R A G E   D E S C R I P T O R __________________________________*/

typedef struct
{
   S_usb_configuration_descriptor cfg;
   S_usb_interface_descriptor     ifc;
   S_usb_endpoint_descriptor      ep1;
   S_usb_endpoint_descriptor      ep2;
   //S_usb_endpoint_descriptor      ep3;
   S_usb_endpoint_descriptor      ep4;
} S_usb_user_configuration_descriptor;
#endif
