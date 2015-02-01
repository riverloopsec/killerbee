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
 * $Id: sipc.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef SIPC_H
#define SIPC_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "sipc_conf.h"
#include "board.h"
/*================================= MACROS           =========================*/
#define SIPC_ENABLE_RX_INTERRUPT()	SIPC_CTRLB |= (1 << SIPC_RXCIE)   //!< Enable RXC interrupt.
#define SIPC_DISABLE_RX_INTERRUPT() SIPC_CTRLB &= ~(1 << SIPC_RXCIE)  //!< Disable RX interrupt.

#define SIPC_ACK_PACKET() SIPC_ENABLE_RX_INTERRUPT() //!< Restart the RX interrupt after the packet has been read.
/*================================= TYEPDEFS         =========================*/
typedef void(*spic_packet_handler_t)(void*); //!< New packet received handler type.
/*================================= GLOBAL VARIABLES =========================*/
extern bool volatile sipc_buffer_overflow; //!< Do not access this variable directly. Use access function.
extern bool volatile sipc_data_available; //!< Do not access this variable directly. Use access function.
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function initializes the Serial Inter Processor Communication.
 */
bool sipc_init(uint8_t buffer_length, uint8_t *rx_buffer_ptr,\
               spic_packet_handler_t event_handler);

/*! \brief This function is used to turn the USART module associated with the SIPC off.
 *
 *         Calling this function uses one of the power reduction registers in the AVR
 *         to turn the USART module associated with the SIPC completly off.
 */
void sipc_deinit(void);

/*! \brief This function returns the status of the buffer overflow flag.
 *
 *  \retval true Buffer overflow.
 *  \retval false No buffer overflow.
 */
#define sipc_get_rx_overflow_flag() (sipc_buffer_overflow)

/*! \brief This function is used to transmit frames.
 *  
 *  \param length Length of frame to send.
 *  \param data Pointer to the frame.
 *
 *  \retval true The frame was successfully transmitted.
 *  \retval false An error occured during transmission.
 */
bool sipc_send_frame(uint8_t length, uint8_t* data);

/*! \brief This function must be called to restart the receiver.
 *
 *         The receiver is disabled when a buffer overflow occurs or when a EOF is seen.
 *         To start the received again this function must be called.
 */
void sipc_reset_rx(void);
#endif
/*EOF*/
