// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file contains the different board dependent configurations for
 *         the RZUSBSTICK hardware platform
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
 * $Id: board_rzusbstick.h 41426 2008-05-08 13:02:14Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef BOARD_RZUSBSTICK_H
#define BOARD_RZUSBSTICK_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*================================= MACROS           =========================*/
/**
 * \defgroup grBoardRzusbstick Board Definitions for the RZUSBSTICK HW
 */
/*@{*/



/*                                  LED Section.                              */



#define LED_BLUE        (PORTD7)   //!< Pin number associated with BLUE LED.
#define LED_BLUE_DDR    (DDRD)  //!< DDR associated with BLUE LED.
#define LED_BLUE_PORT   (PORTD) //!< PORT associated with BLUE LED.

#define LED_GREEN       (PORTE7) //!< Pin number associated with GREEN LED.
#define LED_GREEN_DDR   (DDRE)  //!< DDR associated with GREEN LED.
#define LED_GREEN_PORT  (PORTE) //!< PORT associated with GREEN LED.

#define LED_ORANGE      (PORTE6) //!< Pin number associated with ORANGE LED.
#define LED_ORANGE_DDR  (DDRE)  //!< DDR associated with ORANGE LED.
#define LED_ORANGE_PORT (PORTE) //!< PORT associated with ORANGE LED.

#define LED_RED         (PORTD5)   //!< Pin number associated with RED LED.
#define LED_RED_DDR     (DDRD)  //!< DDR associated with RED LED.
#define LED_RED_PORT    (PORTD) //!< PORT associated with RED LED.



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

#define VRT_TIMER_SLOW_TICK_RELOAD_VALUE (6250) //!< This gives a 20ms slow tick.

#define VRT_TIMER_RESTART_SLOW_TICK() VRT_TIME_OUT_COMP_B = VRT_TIME_TC_VALUE + \
                                                            VRT_TIMER_SLOW_TICK_RELOAD_VALUE


#define VRT_TIMER_RELOAD_HIGH_PRI_COMP(reload_value) VRT_TIME_OUT_COMP_A = VRT_TIME_TC_VALUE + reload_value;



/*                                  RF230 Section.                            */



#define SLP_TR            (0x04)  //!< Pin number that corresponds to the SLP_TR line.
#define DDR_SLP_TR        (DDRB)  //!< Data Direction Register associated with the SLP_TR pin.
#define PORT_SLP_TR       (PORTB) //!< Port Register associated with the SLP_TR pin.
#define PIN_SLP_TR        (PINB)  //!< Pin Register associated with the SLP_TR pin.

#define RST               (0x05)  //!< Pin number that corresponds to the RST line.
#define DDR_RST           (DDRB)  //!< Data Direction Register associated with the RST pin.
#define PORT_RST          (PORTB) //!< Port Register associated with the RST pin.
#define PIN_RST           (PINB)  //!< Pin Register associated with the RST pin.

#define TST               (0x06)  //!< Pin number that corresponds to the TST line.
#define DDR_TST           (DDRB)  //!< Data Direction Register associated with the TST pin.
#define PORT_TST          (PORTB) //!< Port Register associated with the TST pin.
#define PIN_TST           (PINB)  //!< Pin Register associated with the TST pin.

#define RF230_PORT_SPI    (PORTB) //!< Port Register associated with the SPI module.
#define RF230_DDR_SPI     (DDRB)  //!< Data Direction Register associated with the SPI module.

#define RF230_DD_SS       (0x00)
#define RF230_DD_SCK      (0x01)
#define RF230_DD_MOSI     (0x02)
#define RF230_DD_MISO     (0x03)

#define RF230_SPI_PRR     (PRR0)  //!< Power reduction register associated with the RF230 radio transceiver.
#define RF230_SPI_PRR_BIT (PRSPI) //!< Bit in the PRR associated with the RF230 radio transceiver.

#define RF230_SPI_DATA_REG (SPDR) //!< SPI Data Register.
#define RF230_SPI_CTRL_REG (SPCR) //!< SPI Control Register.
#define     RF230_SPI_SPE    (SPE) //!< SPI Enable.
#define     RF230_SPI_MSTR   (MSTR) //!< SPI Enable Master Mode.
#define     RF230_SPI_2X     (SPI2X) //!< SPI Double Speed.
#define RF230_SPI_STATUS_REG (SPSR) //!< SPI Status Register.
#define RF230_SPI_DONE_FLAG (SPIF) //!< Bit in SPSR that indicates that the SPI transmission is done.

#define RF230_INTERRUPT_ENABLE() (TIMSK1 |= (1 << ICIE1)) //!< Macro that enables the interrupt in the module where the IRQ pin from the radio transceiver is connected.
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



#define BOOT_DDR DDRD   //!< Data direction register for pins that will force the device into bootloader.
#define BOOT_PIN PIND   //!< Pin register for pins that will force the device into bootloader.
#define BOOT_PORT PORTD //!< Port register for pins that will force the device into bootloader.
#define BOOT_RX PD2     //!< USART RX pin. This pin must be shorted to the TX pin to force the device into bootloader. 
#define BOOT_TX PD3     //!< USART TX pin. This pin must be shorted to the RX pin to force the device into bootloader.
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*@}*/
#endif
/* EOF */
