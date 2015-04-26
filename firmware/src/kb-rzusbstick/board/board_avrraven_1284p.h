// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the different board dependent configurations for
 *         the ATmega1284p on the AVRRAVEN hardware platform
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
 * $Id: board_avrraven_1284p.h 41436 2008-05-09 00:02:43Z hmyklebust $
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



/*                                  VRT_TIMER Section.                        */



#define VRT_TIME_CONF_REG_A   (TCCR1A)
#define           VT_COMA1    (COM1A1)
#define           VT_COMA0    (COM1A0)
#define           VT_COMB1    (COM1B1)
#define           VT_COMB0    (COM1B0)
#define           VT_COMC1    (COM1C1)
#define           VT_COMC0    (COM1C0)
#define           VT_WGM1     (WGM11)
#define           VT_WGM0     (WGM10)

#define VRT_TIME_CONF_REG_B   (TCCR1B)
#define           VT_ICNC     (ICNC1)
#define           VT_ICES     (ICES1)
#define           VT_WGM3     (WGM13)
#define           VT_WGM2     (WGM12)
#define           VT_CS2      (CS12)
#define           VT_CS1      (CS11)
#define           VT_CS0      (CS10)

#define VRT_TIME_CONF_REG_C   (TCCR1C)
#define           VT_FOC_A    (FOC1A)
#define           VT_FOC_B    (FOC1B)

#define VRT_TIME_TC_VALUE     (TCNT1)

#define VRT_TIME_OUT_COMP_A   (OCR1A)
#define VRT_TIME_OUT_COMP_B   (OCR1B)

#define VRT_TIME_INPUT_CAPT   (ICR1)

#define VRT_TIME_FLAG_REG     (TIFR1)
#define       VT_ICF          (ICF1)
#define       VT_OCF_A        (OCF1A)
#define       VT_OCF_B        (OCF1B)
#define       VT_TOV          (TOV1)


#define VRT_TIME_ISR_MASK_REG (TIMSK1)
#define           VT_ICIE     (ICIE1)
#define           VT_OCIE_A   (OCIE1A)
#define           VT_OCIE_B   (OCIE1B)
#define           VT_TOIE     (TOIE1)

#define VRT_TIME_OVF_VECTOR   (TIMER1_OVF_vect)
#define VRT_TIME_COMPA_vect   (TIMER1_COMPA_vect)
#define VRT_TIME_COMPB_vect   (TIMER1_COMPB_vect)


/*! \brief Macros defined for RF230_ST.
 *
 *  These macros are used to define the correct setupt of the AVR's Timerx, and
 *  to ensure that the mt_get_system_time function returns the system time in 
 *  symbols (16 us ticks).
 */
#if (F_CPU == 16000000UL)
    #define VRT_TIME_TCCRB_CONFIG ((1 << VT_ICES) | (1 << VT_CS2))
#elif (F_CPU == 8000000UL)
    #define VRT_TIME_TCCRB_CONFIG ((1 << VT_ICES) | (1 << VT_CS1) | (1 << VT_CS0))
    #define VRT_TIME_TICK_PER_SYMBOL (2) 
#elif (F_CPU == 4000000UL)
    #define VRT_TIME_TCCRB_CONFIG ((1 << VT_ICES) | (1 << VT_CS1) | (1 << VT_CS0))
    #define VRT_TIME_TICK_PER_SYMBOL (1)
#elif (F_CPU == 1000000UL)
    #define VRT_TIME_TCCRB_CONFIG ((1 << ST_ICES) | (1 << ST_CS1))
#else
    #error "Selected F_CPU not supported."
#endif

#define VRT_TIME_ENABLE_MODULE()  (PRR0 &= ~(1 << PRTIM1))
#define VRT_TIME_DISABLE_MODULE() (PRR0 |= (1 << PRTIM1))

#define VRT_TIME_ENABLE() (VRT_TIME_ISR_MASK_REG =  (1 << VT_TOIE))
#define VRT_TIME_DISABLE() (VRT_TIME_ISR_MASK_REG =  ~(1 << VT_TOIE))

#define VRT_TIMER_DISABLE_HIGH_PRI_DELAY_ISR() VRT_TIME_ISR_MASK_REG &= ~(1 << VT_OCIE_A)

#define VRT_TIMER_SLOW_TICK_RELOAD_VALUE (3125) //!< This gives a 20ms slow tick.

#define VRT_TIMER_RESTART_SLOW_TICK() VRT_TIME_OUT_COMP_B = VRT_TIME_TC_VALUE + \
                                                            VRT_TIMER_SLOW_TICK_RELOAD_VALUE


#define VRT_TIMER_RELOAD_HIGH_PRI_COMP(reload_value) VRT_TIME_OUT_COMP_A = VRT_TIME_TC_VALUE + reload_value;


/*                                  RF230 Section.                            */



#define SLP_TR            (0x03)  //!< Pin number that corresponds to the SLP_TR line.
#define DDR_SLP_TR        (DDRB)  //!< Data Direction Register associated with the SLP_TR pin.
#define PORT_SLP_TR       (PORTB) //!< Port Register associated with the SLP_TR pin.
#define PIN_SLP_TR        (PINB)  //!< Pin Register associated with the SLP_TR pin.

#define RST               (0x01)  //!< Pin number that corresponds to the RST line.
#define DDR_RST           (DDRB)  //!< Data Direction Register associated with the RST pin.
#define PORT_RST          (PORTB) //!< Port Register associated with the RST pin.
#define PIN_RST           (PINB)  //!< Pin Register associated with the RST pin.

#define TST               (0x00)  //!< Pin number that corresponds to the TST line.
#define DDR_TST           (DDRB)  //!< Data Direction Register associated with the TST pin.
#define PORT_TST          (PORTB) //!< Port Register associated with the TST pin.
#define PIN_TST           (PINB)  //!< Pin Register associated with the TST pin.

#define RF230_PORT_SPI    (PORTB) //!< Port Register associated with the SPI module.
#define RF230_DDR_SPI     (DDRB)  //!< Data Direction Register associated with the SPI module.

#define RF230_DD_SS       (0x04)
#define RF230_DD_SCK      (0x07)
#define RF230_DD_MOSI     (0x05)
#define RF230_DD_MISO     (0x06)

#define RF230_SPI_PRR     (PRR0)  //!< Power reduction register associated with the RF230 radio transceiver.
#define RF230_SPI_PRR_BIT (PRSPI) //!< Bit in the PRR associated with the RF230 radio transceiver.



#if defined(__ICCAVR__)



#define RF230_SPI_DATA_REG   (SPDR) //!< SPI Data Register.
#define RF230_SPI_CTRL_REG   (SPCR) //!< SPI Control Register.
#define     RF230_SPI_SPE    (SPE0) //!< SPI Enable.
#define     RF230_SPI_MSTR   (MSTR0) //!< SPI Enable Master Mode.
#define     RF230_SPI_2X     (SPI2X0) //!< SPI Double Speed.

#define RF230_SPI_STATUS_REG (SPSR) //!< SPI Status Register.
#define RF230_SPI_DONE_FLAG  (SPIF0) //!< Bit in SPSR that indicates that the SPI transmission is done.



#elif defined(__GNUC__)



#define RF230_SPI_DATA_REG   (SPDR) //!< SPI Data Register.
#define RF230_SPI_CTRL_REG   (SPCR) //!< SPI Control Register.
#define     RF230_SPI_SPE    (SPE) //!< SPI Enable.
#define     RF230_SPI_MSTR   (MSTR) //!< SPI Enable Master Mode.
#define     RF230_SPI_2X     (SPI2X) //!< SPI Double Speed.

#define RF230_SPI_STATUS_REG (SPSR) //!< SPI Status Register.
#define RF230_SPI_DONE_FLAG  (SPIF) //!< Bit in SPSR that indicates that the SPI transmission is done.



#else
#error 'Unsupported compiler'
#endif



#define RF230_INTERRUPT_ENABLE()  do { \
    TIFR1  |= (1 << ICF1);             \
    TIMSK1 |= (1 << ICIE1);            \
} while (0)
#define RF230_INTERRUPT_DISABLE() (TIMSK1 &= ~(1 << ICIE1)) //!< Macro that disables the interrupt in the module where the IRQ pin from the radio transceiver is connected.

#define RF230_ACK_INTERRUPT_FLAG() (TIFR1 |= (ICIE1)) //!< Macro that is used to clear the interrupt flag when fired.



/*                                  AT24CXX Section                           */



#define AT24CXX_BRR (TWBR) //!< Baudrate register.

#define AT24CXX_STATUS_REG (TWSR)                                                 //!< TWI status register.
#define AT24CXX_GET_STATUS() (AT24CXX_STATUS_REG & 0xF8)                          //!< Get TWI status.
#define AT24CXX_SET_PRESCALER(prescaler) AT24CXX_STATUS_REG |= (prescaler & 0x03) //!< Set TWI prescaler.
#define AT24CXX_GET_PRESCALER() ((AT24CXX_STATUS_REG & 0x03))                     //!< Get TWI prescaler.

#define AT24CXX_CONTROL_REG (TWCR)  //!< TWI control register.
#define AT24CXX_TWINT       (TWINT) //!< TWI interrupt flag.
#define AT24CXX_TWEA        (TWEA)  //!< TWI ACK enable.
#define AT24CXX_TWSTA       (TWSTA) //!< TWI start condition bit.
#define AT24CXX_TWSTO       (TWSTO) //!< TWI stop condition bit.
#define AT24CXX_TWWC        (TWWC)  //!< TWI Write Collison flag.
#define AT24CXX_TWEN        (TWEN)  //!< TWI enable.
#define AT24CXX_TWIE        (TWIE)  //!< TWI interrupt enable.

#define AT24CXX_DATA_REG (TWDR) //!< TWI data register.



/*                                  BOOTLOADER Section                        */

/* Following macros are used to configure the SIPC for the ATmega1284p. */

#define SIPC_DATA        UDR0            //!< USART data register associated with the SIPC.
#define SIPC_CTRLA       UCSR0A          //!< USART control A register associated with the SIPC.
#define SIPC_CTRLB       UCSR0B          //!< USART control B register associated with the SIPC.
#define SIPC_CTRLC       UCSR0C          //!< USART control C associated with the SIPC.
#define SIPC_UBRR        UBRR0           //!< USART baud rate register.
#define SIPC_PRR         PRR0            //!< USART power reduction register.
#define SIPC_RX_vector   USART0_RX_vect //!< USART receive complete interrupt vector.

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
#define SIPC_UCSZ1    (UCSZ11)   //!< Character Size bit 1.
#define SIPC_UCSZ0    (UCSZ10)   //!< Character Size bit 0.
#define SIPC_PRUSART  (PRUSART0) //!< Enable/Disable bit for the USART in Power Reduction Register.
#define SIPC_U2X      (U2X0)     //!< Enable double speed in asynchronous mode.
#define SIPC_DOR      (DOR0)     //!< Data overrun flag

#define SIPC_UBRR_VALUE (1) //!< Value that is loaded into the Usart's Baude Rate Register.
#define SIPC_TIME_OUT (0xFFFF) //!< Number of maximum iterations in the busy wait loop polling the UDRE flag.




/*                                  BOOTLOADER Section                        */
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*@}*/
#endif
/* EOF */
