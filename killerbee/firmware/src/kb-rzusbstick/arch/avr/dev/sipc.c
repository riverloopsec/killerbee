// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  SIPC driver for USART1 on AVRRaven
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
 * $Id: sipc.c 41437 2008-05-09 00:04:54Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "vrt_kernel.h"

#include "compiler.h"
#include "board.h"
#include "sipc_conf.h"
#include "sipc.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
bool volatile sipc_buffer_overflow;      //<! This variable is per definition global, but should only be read through its access function.
/*================================= LOCAL VARIABLES  =========================*/
static bool sipc_next_stuffed;           //!< Flag indicating if the next byte is stuffed.
static bool sipc_frame_valid;            //!< Flag indicating valid frame.

static uint8_t *rx_buffer; //!< Pointer to storage where the packet is to be stored.
static uint8_t rx_buffer_length;
static uint8_t rx_buffer_size;
static spic_packet_handler_t handler;
/*================================= PROTOTYPES       =========================*/
bool sipc_init(uint8_t buffer_length, uint8_t *rx_buffer_ptr,\
               spic_packet_handler_t event_handler) {
    /* Perform sanity check on function parameters. */
    if (0 == buffer_length) {
        return false;
    }
    
    if (NULL == rx_buffer_ptr) {
        return false;
    }
    
    if (NULL == event_handler) {
        return false;
    }
    
    SIPC_DISABLE_RX_INTERRUPT();
    
    /* Initialize local variables. */
    sipc_buffer_overflow = false;
    sipc_next_stuffed    = false;
    rx_buffer            = rx_buffer_ptr;
    rx_buffer_size       = buffer_length;
    handler              = event_handler;
        
    /* Enable and configure the USART module used by SIPC. */
    // Power on the USART Module.
    SIPC_PRR &= ~(1 << SIPC_PRUSART);
    
    // Enable double speed.
    SIPC_CTRLA = (1 << SIPC_TXEN) | (1 << SIPC_RXEN);
    
    // Enable Receiver and Transmitter.
    SIPC_CTRLB = (1 << SIPC_TXEN) | (1 << SIPC_RXEN);
    
    // Set Data bits and parity mode: 8-N-1.
    SIPC_CTRLC = (1 << SIPC_UCSZ1) | (1 << SIPC_UCSZ0);
    
    // Set double speed mode. The SIPC is asynchronous.
    
    
    // Set baudrate.
    SIPC_UBRR = SIPC_UBRR_VALUE;
    
    SIPC_ENABLE_RX_INTERRUPT();
    
    return true;
}


void sipc_deinit(void){
    SIPC_DISABLE_RX_INTERRUPT();
    // Power off the USART Module
    SIPC_PRR |= (1 << SIPC_PRUSART);
}


bool sipc_send_frame(uint8_t length, uint8_t *data) {
    
    sipc_timeout_t time_out = SIPC_TIME_OUT;
    
    // First try to send SOP.
    while ((( SIPC_CTRLA & (1 << SIPC_UDRE)) != (1 << SIPC_UDRE)) &&
           (time_out != 0)) { time_out--; }
    if (time_out == 0) { return false; }
    
    SIPC_DATA = SIPC_SOF;
    
    // Send data.
    do {
        time_out = SIPC_TIME_OUT;
        
        // Check if the next byte must be stuffed.
        if ((*data == SIPC_SOF) || (*data == SIPC_ESC) || (*data == SIPC_EOF)) {
            
            // We now have to send the ESC sequence first and then the byte it self
            // OR'ed with ESC_MASK.
            while ((( SIPC_CTRLA & (1 << SIPC_UDRE)) != (1 << SIPC_UDRE)) &&
                    (time_out != 0)) { time_out--; }
            if (time_out == 0) { return false; }
            
            SIPC_DATA = SIPC_ESC;
            time_out = SIPC_TIME_OUT;
            
            while ((( SIPC_CTRLA & (1 << SIPC_UDRE)) != (1 << SIPC_UDRE)) &&
                    (time_out != 0)) { time_out--; }
            if (time_out == 0) { return false; }
            
            SIPC_DATA = (*data) | SIPC_ESC_MASK;
        } else {
            
            while ((( SIPC_CTRLA & (1 << SIPC_UDRE)) != (1 << SIPC_UDRE)) &&
                    (time_out != 0)) { time_out--; }
            if (time_out == 0) { return false; }
            
            SIPC_DATA = *data;
        } // END: if ((*data == SIPC_SOF) || (*data == SIPC_ESC) || ...
        
        length--;
        data++;
    } while (length != 0);
    
    // Send EOF.
    while ((( SIPC_CTRLA & (1 << SIPC_UDRE)) != (1 << SIPC_UDRE)) &&
           (time_out != 0)) { time_out--; }
    if (time_out == 0) { return false; }
    
    SIPC_DATA = SIPC_EOF;
    
    return true;
}


void sipc_reset_rx(void){
    sipc_buffer_overflow = false;
    sipc_next_stuffed    = false;
    sipc_frame_valid     = false;
    rx_buffer_length               = 0;
    
    //Following loop is used to ensure that the rx FIFO is flushed.
	//Sometimes it gets cloged up with old data.
    uint8_t volatile dummy;
	while ((SIPC_CTRLA & (1 << SIPC_RXC)) == (1 << SIPC_RXC)){
		dummy = SIPC_DATA;
    }
    
    SIPC_ENABLE_RX_INTERRUPT();
}


ISR(SIPC_RX_vector){
    // Unvalidate frame if data overrun detected. All data till next SOF will be discarded
    if (SIPC_CTRLA & (1<<SIPC_DOR)) {
        VRT_EVENT_MISSED();
        sipc_frame_valid = false;
    }
    
    // Read data
    uint8_t data = SIPC_DATA;
        
    // Return and discard data until a SIPC_SOF is recevied
    if ((sipc_frame_valid == false) && (data != SIPC_SOF)) {
        return;
    } // END: if ((sipc_frame_valid == false) && (data != SIPC_SOF)) ...
    
    if (rx_buffer_size > rx_buffer_length) {
        if (data == SIPC_SOF) {
            sipc_next_stuffed = false;
            sipc_frame_valid = true;
            rx_buffer_length = 0;
        } else if (data == SIPC_EOF) {
            if (0 == VRT_GET_ITEMS_FREE()) {
                VRT_EVENT_MISSED();
                rx_buffer_length = 0;
            } else {
                vrt_quick_post_event(handler, (void *)rx_buffer);
                SIPC_DISABLE_RX_INTERRUPT();
            }
        } else if (data == SIPC_ESC) {
            sipc_next_stuffed = true;
        } else {
            if (true != sipc_next_stuffed) {
                *(rx_buffer + rx_buffer_length) = data;
            } else {
                sipc_next_stuffed = false;
                *(rx_buffer + rx_buffer_length) = data & ~SIPC_ESC_MASK;
        } // END: if (sipc_next_stuffed == true) ...

        rx_buffer_length++;
        }
    }
}
/*EOF*/
