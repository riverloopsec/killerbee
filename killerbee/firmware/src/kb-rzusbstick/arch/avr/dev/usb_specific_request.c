// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Implementation of USB specific requests.
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
 * $Id: usb_specific_request.c 41220 2008-05-01 13:43:18Z vkbakken $
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

//_____ P R I V A T E   D E C L A R A T I O N ______________________________

extern uint8_t   *pbuffer;
extern uint8_t   data_to_transfer;


//_____ D E C L A R A T I O N ______________________________________________

//! usb_user_read_request(type, request);
//!
//! This function is called by the standard usb read request function when
//! the Usb request is not supported. This function returns true when the
//! request is processed. This function returns false if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard usb read request function.
//!
//! @param type Request type.
//! @param request Request.
//!
//! @retval false No user requests supported.
bool usb_user_read_request(uint8_t type, uint8_t request)
{
  switch (request)
  {
    default:
         return false;
  }
	
}



//! usb_user_endpoint_init.
//!
//! This function configures the endpoints.
void usb_user_endpoint_init(void)
{
 usb_configure_endpoint(EP_IN,          
                         TYPE_BULK,     
                         DIRECTION_IN,  
                        #if EP_IN_LENGTH == 64
                            SIZE_64,
                        #elif EP_IN_LENGTH == 256
                            SIZE_256,
                        #elif EP_IN_LENGTH == 512
                            SIZE_512,
                        #else
                            #error Wrong EP_IN_LENGTH
                        #endif
                        
                         ONE_BANK,
                        
                         NYET_ENABLED);
 
 usb_configure_endpoint(EP_OUT,        
                         TYPE_BULK,     
                         DIRECTION_OUT, 
                        #if EP_OUT_LENGTH == 64
                            SIZE_64,
                        #elif EP_OUT_LENGTH == 256
                            SIZE_256,
                        #elif EP_OUT_LENGTH == 512
                            SIZE_512,
                        #else
                            #error Wrong EP_OUT_LENGTH
                        #endif
                        
                         ONE_BANK,
                        
                         NYET_ENABLED);

usb_configure_endpoint(EP_EVENT,        
                         TYPE_BULK,     
                         DIRECTION_IN,
                            SIZE_64,
                         TWO_BANKS,
                         NYET_ENABLED);
}


//! usb_user_get_descriptor.
//!
//! This function returns the size and the pointer on a user information
//! structure
//!
//! @warning Code:xx bytes (function code length)
//!
//! @param type Descriptor type.
//! @param string Specific string.
//!
//! @retval true The descriptor was found and put in the TX buffer. False is
//!              returned if the descriptor could not be found.

bool usb_user_get_descriptor(uint8_t type, uint8_t string)
{
    switch(type)
    {
        case STRING_DESCRIPTOR:
	        switch (string)
	        {
    	        case LANG_ID:
		            data_to_transfer = sizeof (usb_user_language_id);
		            pbuffer = &(usb_user_language_id.bLength);
		            return true;

	            case MAN_INDEX:
		            data_to_transfer = sizeof (usb_user_manufacturer_string_descriptor);
		            pbuffer = &(usb_user_manufacturer_string_descriptor.bLength);
		            return true;

	            case PROD_INDEX:
		            data_to_transfer = sizeof (usb_user_product_string_descriptor);
		            pbuffer = &(usb_user_product_string_descriptor.bLength);
		            return true;

	            case SN_INDEX:
		            data_to_transfer = sizeof (usb_user_serial_number);
		            pbuffer = &(usb_user_serial_number.bLength);
		            return true;

	            default:
		            return false;
	        }
	
	    default:
		    return false;
	}
}
