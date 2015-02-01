// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file manages the USB controller.
 *
 * The USB task checks the income of new requests from the USB Host.
 * When a Setup request occurs, this task will launch the processing
 * of this setup contained in the usb_enum.c file.
 * Other class specific requests are also processed in this file.
 *
 * This file manages all the USB events:
 * Suspend / Resume / Reset / Start Of Frame / Wake Up / Vbus events
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
 * $Id: usb_task.c 41220 2008-05-01 13:43:18Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

//_____  I N C L U D E S ___________________________________________________
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

#include "conf_usb.h"
#include "usb_task.h"

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"

#include "pll_drv.h"


#include "led.h"
//_____ M A C R O S ________________________________________________________
//_____ D E F I N I T I O N S ______________________________________________
static bool usb_connected;
static bool reset_detected;
static uint8_t usb_configuration_nb;

static uint8_t transaction_data[USB_TRANSACTION_BUFFER_SIZE];
static uint16_t trans_data_i;

static bool ep_open;
bool ep_error;
static uint8_t open_ep;

static usb_trans_done_callback_t done_callback;
//_____ D E C L A R A T I O N S ____________________________________________
static uint8_t bin2hex(uint8_t  nibble);

//!
//! @brief This function initializes the USB the associated variables.
//!
//! This function enables the USB controller and init the USB interrupts.
//! The aim is to allow the USB connection detection in order to send
//! the appropriate USB event to the operating mode manager.
//!
//! @param serial_nmbr Serial number for this device.
void usb_task_init(uint64_t serial_nmbr) {
    /* Initialze local variables. */
    usb_connected = false;
    reset_detected = false;
    usb_configuration_nb = 0;
    
    trans_data_i = 0;
    ep_open = false;
    ep_error = false;
    open_ep = 0;

    done_callback = NULL;
    
    
    PRR1 &= ~(1 << PRUSB); // Enable USB macro.
    
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t temp = (serial_nmbr >> 8*(i)) & 0xFF;
		usb_user_serial_number.wstring[i * 2]   = bin2hex(temp >> 4);		// High nibble
		usb_user_serial_number.wstring[i * 2 + 1] = bin2hex(temp & 0x0F);	// Low nibble
    }
    
	Usb_force_device_mode();
	Usb_enable();
	Usb_select_device();
	Usb_enable_vbus_interrupt();
}

void usb_task_deinit(void) {
    Usb_disable();
    PRR1 |= (1 << PRUSB); // Disable USB macro.
}

//!
//! @brief This function initializes the USB device controller
//!
//! This function enables the USB controller and init the USB interrupts.
//! The aim is to allow the USB connection detection in order to send
//! the appropriate USB event to the operating mode manager.
void usb_start_device (void)
{
   Usb_enable_regulator();
   Pll_start_auto();
   Wait_pll_ready();
   Usb_unfreeze_clock();
   Usb_enable_suspend_interrupt();
   Usb_enable_reset_interrupt();
   usb_init_device();         // configure the USB controller EP0
   Usb_attach();
   usb_connected = false;
}

//! @brief Entry point of the USB mamnagement
//!
//! This function is the entry point of the USB management. Each USB
//! event is checked here in order to launch the appropriate action.
//! If a Setup request occurs on the Default Control Endpoint,
//! the usb_process_request() function is call in the usb_enum.c file
//! If a new USB mass storage Command Block Wrapper (CBW) occurs,
//! this one will be decoded and the SCSI command will be taken in charge
//! by the scsi decoder.
void usb_task(void) {
   if (Is_usb_vbus_high()&& usb_connected==false) {
        usb_connected = true;
     
    	Usb_enable_reset_interrupt();
		Usb_attach();
   }

   if (reset_detected==true) {
      Usb_reset_endpoint(0);
      usb_configuration_nb=0;
      reset_detected=false;
   }

    // USB MANAGEMENT
    Usb_select_endpoint(EP_CONTROL);
    if (Is_usb_receive_setup()) { usb_process_request(); }
    
    /* Check for new transactions. */
    if (true == ep_open) {
        Usb_select_endpoint(open_ep); // Select open EP.
        
        /* Check if there is data available. */
        while ((UEINTX & (1 << RXOUTI)) == (1 << RXOUTI)) { 
            /* Data available. */
            UEINTX &= ~(1 << RXOUTI); // ACK RX_OUT interrupt flag.
        
            /* Ready to upload data. First read number of bytes received. */
            uint8_t bytes_in_bank = Usb_byte_counter_8();
        
            if ((bytes_in_bank + trans_data_i) > USB_TRANSACTION_BUFFER_SIZE) {
                ep_open = false;
                ep_error = true;
                
                return;
            }
            
            /* Upload data. */
            for (uint8_t i = bytes_in_bank; i != 0; i--) {
                *(transaction_data + trans_data_i) = Usb_read_byte();
                trans_data_i++;
            }
            
            UEINTX &= ~(1 << FIFOCON); // ACK that fifo is read.
            
            /* Check if this was the last packet in the transaction. */
            if (bytes_in_bank < 64) {
                ep_open = false;
                
                /* End of transmission is indicated, so set the flag value to true
                 * or execute the correct callback.
                 */
                done_callback(trans_data_i, transaction_data);

                break;
            }
        }
    }
}

uint8_t usb_ep_open(usb_trans_descriptor_t *trans_desc) {
    if (NULL == trans_desc) { return USB_INVALID_PARAMETER; }
    if (true == ep_open) { return USB_EP_HANDLE_ALREADY_OPEN; } // Already open. usb_ep_close must be issued first.
    if (EP_CONTROL == (trans_desc->ep)) { return USB_INVALID_PARAMETER; }
    
    ENTER_CRITICAL_REGION();
    open_ep = (trans_desc->ep);
    
    done_callback = trans_desc->done_callback;
    
    trans_data_i = 0;
    ep_open = true;
    ep_error = false;
    LEAVE_CRITICAL_REGION();
    
    return USB_SUCCESS;
}

void usb_ep_close(void) {
    ENTER_CRITICAL_REGION();
    ep_open = false;
    LEAVE_CRITICAL_REGION();
}

void usb_ep_ack_transaction(void) {
    ENTER_CRITICAL_REGION();
    ep_open = true;
    ep_error = false;
    trans_data_i = 0;
    LEAVE_CRITICAL_REGION();
}

uint8_t usb_ep_write(uint8_t ep, uint8_t length, uint8_t *packet) {
    UENUM = ep;
    if ((UEINTX & (1 << TXINI)) != (1 << TXINI)) { return USB_HW_TIME_OUT; }
    
    uint8_t* data_ptr = packet;
    uint8_t bytes_to_write;
    uint8_t packets_to_write = (uint8_t)(((uint16_t)(length + 64 - 1)) >> 6);
    
    if ((length % 64) == 0) {
        bytes_to_write = 64;
        packets_to_write++;
    } else if (length > 64) {
        bytes_to_write = 64;
    } else {
        bytes_to_write = length;
    }    
   
    // Do the USB transaction.
    do {
    
        // Upload EP_SIZE data, less or zero packet.
        // Idea...upload as long as the EP is not full -> RWAL not set.
        
        UEINTX &= ~(1 << TXINI);
        
        while (bytes_to_write != 0) {
                
            UEDATX = *data_ptr;
            
            // Decrement number of bytes in transaction.
            data_ptr++;
            bytes_to_write--;
        }
        
        length -= 64;
        
        // Clear FICOCON. -> switch Bank and send data.
        UEINTX &= ~(1 << FIFOCON);
        
        // End of transfer reached. Check if zero length packet must be sent.
                  
        if (length >= 64) {
            bytes_to_write = 64;
        } else {
            bytes_to_write = length;
        }
        
        packets_to_write--;
        uint16_t timeout = 0xFFFF;
        
        while ((UEINTX & (1 << TXINI)) != (1 << TXINI)) {
            if (timeout == 0) {
                LED_ORANGE_ON();
                return 1;
            } else {
                timeout--;
            }
        }            // bank is not ready..    
    } while (packets_to_write != 0);
    return USB_SUCCESS;
}

uint8_t usb_ep_write_block(uint8_t ep, uint8_t length, uint8_t *packet) {
    if (0 == length) { return 1; }
    
    bool send_zero_packet = false;
        
    if (length == 64) { send_zero_packet = true; }
    
    UENUM = ep;
    
    uint8_t timeout = 0xFF;
        while ((UEINTX & (1 << TXINI)) != (1 << TXINI)) {
            if (timeout == 0) {
                return USB_HW_TIME_OUT;
            } else {
                timeout--;
            }
        }            // bank is not ready..
    
    UEINTX &= ~(1 << TXINI);
    
    do {
        UEDATX = *packet;
            
        // Decrement number of bytes in transaction.
        packet++;
        length--;
    } while (length != 0);
    
    UEINTX &= ~(1 << FIFOCON);
    
    if (true == send_zero_packet) {
        timeout = 0xFF;
        while ((UEINTX & (1 << TXINI)) != (1 << TXINI)) {
            if (timeout == 0) {
                return USB_HW_TIME_OUT;
            } else {
                timeout--;
            }
        }            // bank is not ready..
    
        UEINTX &= ~(1 << TXINI);
        UEINTX &= ~(1 << FIFOCON);
    }
    
    return 0;
}

//! @brief USB interrupt process
//!
//! This function is called each time a USB interrupt occurs.
//! The following USB events are taken in charge:
//! - VBus On / Off
//! - Start Of Frame
//! - Suspend
//! - Wake-Up
//! - Resume
//! - Reset
//! For each event, the user can launch an action by completing
//! the associate define


//!
//! @brief General interrupt subroutine
//! Check for VBUS and ID pin transitions
//! @return
//!

#if defined(__ICCAVR__)
ISR(USB_General_vect) {
#elif defined(__GNUC__)
ISR(USB_GEN_vect) {
#else
#error 'Unsupported compiler.'
#endif
   if (Is_usb_vbus_transition())
   {
      Usb_ack_vbus_transition();
      if (Is_usb_vbus_high())
      {
         usb_connected = true;
         //Usb_send_event(EVT_USB_POWERED);
			Usb_enable_reset_interrupt();
         usb_start_device();
			Usb_attach();
      }
      else
      {
         usb_connected = false;
         //Usb_send_event(EVT_USB_UNPOWERED);
      }
   }

   if(Is_usb_id_transition())
   {
      Usb_ack_id_transition();
      if(Is_usb_id_device())
      {
         //Usb_send_event(EVT_USB_DEVICE_FUNCTION);
      }
      else
      {
        // Usb_send_event(EVT_USB_HOST_FUNCTION);
      }
   }

   if (Is_usb_sof())
   {
      Usb_ack_sof();
//      Usb_sof_action();
   }

   if (Is_usb_suspend())
   {
      Usb_ack_suspend();
      Usb_enable_wake_up_interrupt();
      Usb_ack_wake_up();                 // clear wake up to detect next event
      Usb_freeze_clock();
      //Usb_send_event(EVT_USB_SUSPEND);
   }

   if (Is_usb_wake_up())
   {
      Usb_unfreeze_clock();
      Usb_ack_wake_up();
      Usb_disable_wake_up_interrupt();
      //Usb_send_event(EVT_USB_WAKE_UP);
   }

   if (Is_usb_resume())
   {
      Usb_disable_wake_up_interrupt();
      Usb_ack_resume();
      Usb_disable_resume_interrupt();
      //Usb_send_event(EVT_USB_RESUME);
   }

   if (Is_usb_reset()) {
      Usb_ack_reset();
      usb_init_device();
     // Usb_send_event(EVT_USB_RESET);
      reset_detected=true;
   }
}

static uint8_t bin2hex(uint8_t  nibble) {
    nibble &= 0x0F;
	if (nibble < 10)
	    nibble += '0';
	else
	    nibble += 'A' - 10;
	
	return nibble;
}
