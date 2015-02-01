// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the different board dependent configurations for
 *         the ATmega3290p on the AVRRAVEN hardware platform
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
 * $Id: board_avrraven_3290p.h 41581 2008-05-13 23:13:00Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef BOARD_AVRRAVEN_1284P_H
#define BOARD_AVRRAVEN_1284P_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*================================= MACROS           =========================*/
/**
 * \defgroup grBoardAvrraven1284 Board Definitions for the AVRRAVEN ATmega1284p.
 */
/*@{*/


/*                                  sipc Section                        */

/* Following macros are used to configure the SIPC for the ATmega3290p. */

#define SIPC_DATA        UDR0           //!< USART data register associated with the SIPC.
#define SIPC_CTRLA       UCSR0A         //!< USART control A register associated with the SIPC.
#define SIPC_CTRLB       UCSR0B         //!< USART control B register associated with the SIPC.
#define SIPC_CTRLC       UCSR0C         //!< USART control C associated with the SIPC.
#define SIPC_UBRR        UBRR0          //!< USART baud rate register.
#define SIPC_PRR         PRR            //!< USART power reduction register.
#define SIPC_RX_vector   USART_RX_vect  //!< USART receive complete interrupt vector.

/* Defines to ensure that bit names are defined - substitusions can be allowed
   since bits are located in the same addresses for all UARTs. If one is undefined, all are. */

#define SIPC_RXC      (RXC0)     //!< RX Complete Flag.
#define SIPC_TXC      (TXC0)     //!< TX Complete Flag.
#define SIPC_UDRE     (UDRE0)    //!< Data Register Empty Flag.
#define SIPC_RXCIE    (RXCIE0)   //!< RX interrupt enable.
#define SIPC_TXCIE    (TXCIE0)   //!< TX interrupt enable.
#define SIPC_UDRIE    (UDRIE0)   //!< Data register empty interrupt enable.
#define SIPC_RXEN     (RXEN0)    //!< RX enable bit.
#define SIPC_TXEN     (TXEN0)    //!< TX enable bit.
#define SIPC_UCSZ2    (UCSZ02)   //!< Character Size bit 2.
#define SIPC_UCSZ1    (UCSZ01)   //!< Character Size bit 1.
#define SIPC_UCSZ0    (UCSZ00)   //!< Character Size bit 0.
#define SIPC_PRUSART  (PRUSART0) //!< Enable/Disable bit for the USART in Power Reduction Register.
#define SIPC_U2X      (U2X0)     //!< Enable double speed in asynchronous mode.
#define SIPC_DOR      (DOR0)     //!< Data overrun flag

#define SIPC_UBRR_VALUE (1) //!< Value that is loaded into the Usart's Baude Rate Register.
#define SIPC_TIME_OUT (0xFFFF) //!< Number of maximum iterations in the busy wait loop polling the UDRE flag.


/*                                  BOOTLOADER Section                        */
#define BOOT_DDR    DDRE
#define BOOT_PIN    PINE
#define BOOT_PORT   PORTE
#define BOOT_RX     PE3
#define BOOT_TX     PE4


/*                                  BOARD Section                             */
//#define AUDIO_POWER_OUTPORT   PORTE
//#define AUDIO_POWER_INPORT    PINE
#define AUDIO_POWER_PORT    PORTE
#define AUDIO_POWER_PUR     PORTE
#define AUDIO_POWER_DDR     DDRE
#define AUDIO_POWER_PIN     PORTE7

/*                                  SPEAKER Section                           */
//#define SPEAKER_OUTPORT   PORTB
//#define SPEAKER_INPORT    PINB
#define SPEAKER_PORT      PORTB
#define SPEAKER_PUR       PORTB
#define SPEAKER_DDR       DDRB
#define SPEAKER_PIN       PORTB5



/*                                  TEMP SENSOR Section                       */
//#define TMPPWR_OUTPORT   PORTF
//#define TMPPWR_INPORT    PINF
#define TMPPWR_PORT      PORTF
#define TMPPWR_PUR       PORTF
#define TMPPWR_DDR       DDRF
#define TMPPWR_PIN       PINF6

//#define TEMP_OUTPORT   PORTF
//#define TEMP_INPORT    PINF
#define TEMP_PORT      PINF
#define TEMP_PUR       PORTF
#define TEMP_DDR       DDRF
#define TEMP_PIN       PINF4



/*                                  MICROPHONE Section                        */
//#define MIC_OUTPORT PORTF
//#define MIC_INPORT  PINF
#define MIC_PORT    PORTF
#define MIC_PUR     PORTF
#define MIC_DDR     DDRF
#define MIC_PIN     PINF0



/*                                  JOYSTIKK Section                          */

//#define KEY_OUTPORT   PORTF
//#define KEY_INPORT    PINF
#define KEY_PORT      PINF
#define KEY_PUR       PORTF
#define KEY_DDR       DDRF
#define KEY_PIN       PINF1

//#define ENTER_OUTPORT   PORTE
//#define ENTER_INPORT    PINE
#define ENTER_PORT      PINE
#define ENTER_PUR       PORTE
#define ENTER_DDR       DDRE
#define ENTER_PIN       PINE2

#define KEY_STATE_UP        0x01
#define KEY_STATE_DOWN      0x02
#define KEY_STATE_LEFT      0x04
#define KEY_STATE_RIGHT     0x08
#define KEY_STATE_ENTER     0x10
#define KEY_STATE_NO_KEY    0x00

/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

// Bug in IAR C compiler! PRUSART0 missspelled as PRUSART0
#if defined(__ICCAVR__)
#define PRUSART0 PSUSART0
#endif


/*@}*/
#endif
/* EOF */
