// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the USB driver routines
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
 * $Id: usb_drv.c 41220 2008-05-01 13:43:18Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

//_____ I N C L U D E S ____________________________________________________
#include "compiler.h"

#include "usb_drv.h"

//_____ M A C R O S ________________________________________________________

//_____ D E C L A R A T I O N ______________________________________________

//! usb_configure_endpoint.
//!
//!  This function configures an endpoint with the selected type.
//!
//! @warning Code:xx bytes (function code length)
//!
//! @param config0   EP configuration 0.
//! @param config1   EP configuration 1.
//!
//! @return Is_endpoint_configured.
//!
//!           The possible types are:
//!                CONTROL
//!                BULK_IN
//!                BULK_OUT
//!                INTERRUPT_IN
//!                INTERRUPT_OUT
//!                ISOCHRONOUS_IN
//!                ISOCHRONOUS_OUT
//!
//! NOTE:
//! The endpoint 0 shall always be configured in CONTROL type.
//!
uint8_t usb_config_ep(uint8_t config0, uint8_t config1)
{
    Usb_enable_endpoint();
    UECFG0X = config0;
    UECFG1X = (UECFG1X & (1<<ALLOC)) | config1;
    Usb_allocate_memory();
    return (Is_endpoint_configured());
}

//! usb_select_endpoint_interrupt.
//!
//! This function select the endpoint where an event occurs and returns the
//! number of this endpoint. If no event occurs on the endpoints, this
//! function returns 0.
//!
//! @return endpoint number.
uint8_t usb_select_enpoint_interrupt(void)
{
uint8_t interrupt_flags;
uint8_t ep_num;

   ep_num = 0;
   interrupt_flags = Usb_interrupt_flags();

   while(ep_num < 9)
   {
      if (interrupt_flags & 1)
      {
         return (ep_num);
      }
      else
      {
         ep_num++;
         interrupt_flags = interrupt_flags >> 1;
      }
   }
   return 0;
}

//! usb_send_packet.
//!
//! This function moves the data pointed by tbuf to the selected endpoint fifo
//! and sends it through the USB.
//!
//! @warning Code:xx bytes (function code length)
//!
//! @param ep_num       number of the addressed endpoint
//! @param *tbuf        address of the first data to send
//! @param data_length  number of bytes to send
//!
//! @return address of the next uint8_t to send.
//!
//! Example:
//! usb_send_packet(3,&first_data,0x20);    // send packet on the endpoint #3
//! while(!(Usb_tx_complete));              // wait packet ACK'ed by the Host
//! Usb_clear_tx_complete();                     // acknowledge the transmit
//!
//! Note:
//! tbuf is incremented of 'data_length'.
//!
uint8_t usb_send_packet(uint8_t ep_num, uint8_t* tbuf, uint8_t data_length)
{
uint8_t remaining_length;

   remaining_length = data_length;
   Usb_select_endpoint(ep_num);
   while(Is_usb_write_enabled() && (0 != remaining_length))
   {
      Usb_write_byte(*tbuf);
      remaining_length--;
      tbuf++;
   }
   return remaining_length;
}

//! usb_read_packet.
//!
//! This function moves the data stored in the selected endpoint fifo to
//! the address specified by *rbuf.
//!
//! @warning Code:xx bytes (function code length)
//!
//! @param ep_num       number of the addressed endpoint
//! @param *rbuf        aaddress of the first data to write with the USB data
//! @param data_length  number of bytes to read
//!
//! @return address of the next uint8_t to send.
//!
//! Example:
//! while(!(Usb_rx_complete));                      // wait new packet received
//! usb_read_packet(4,&first_data,usb_get_nb_byte); // read packet from ep 4
//! Usb_clear_rx();                                 // acknowledge the transmit
//!
//! Note:
//! rbuf is incremented of 'data_length'.
//!
uint8_t usb_read_packet(uint8_t ep_num, uint8_t* rbuf, uint8_t  data_length)
{
uint8_t remaining_length;

   remaining_length = data_length;
   Usb_select_endpoint(ep_num);

   while(Is_usb_read_enabled() && (0 != remaining_length))
   {
      *rbuf = Usb_read_byte();
      remaining_length--;
      rbuf++;
   }
   return remaining_length;
}

//! usb_halt_endpoint.
//!
//! This function sends a STALL handshake for the next Host request. A STALL
//! handshake will be send for each next request untill a SETUP or a Clear Halt
//! Feature occurs for this endpoint.
//!
//! @warning Code:xx bytes (function code length)
//!
//! @param ep_num Addressed endpoint
//!
void usb_halt_endpoint (uint8_t ep_num)
{
   Usb_select_endpoint(ep_num);
   Usb_enable_stall_handshake();
}

//! usb_init_device.
//!
//! This function initializes the USB device controller and
//! configures the Default Control Endpoint.
//!
//! @returns Zero is returned if the device could not be initialized. A non-zero,
//!          positive number is returned if the initialization was successful.
uint8_t usb_init_device (void)
{
   Usb_select_device();
   if(Is_usb_id_device())
   {
      Usb_select_endpoint(EP_CONTROL);
      if(!Is_usb_endpoint_enabled())
      {
         return usb_configure_endpoint(EP_CONTROL,    \
                                TYPE_CONTROL,  \
                                DIRECTION_OUT, \
                                SIZE_64,       \
                                ONE_BANK,      \
                                NYET_DISABLED);
      }
   }
   return false;

}
