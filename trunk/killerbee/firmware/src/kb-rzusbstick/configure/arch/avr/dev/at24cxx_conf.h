// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file is used to configure the AT24CXX serial EEPROM driver
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
 * $Id: at24cxx_conf.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef AT24CXX_CONF_H
#define AT24CXX_CONF_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup grAt24CxxLowLevel
//! @{
/*================================= MACROS           =========================*/
#define AT24CXX_PRR (PRR0)      //!< Power reduction register used by the AT24CXX device (TWI).
#define AT24CXX_PRR_BIT (PRTWI) //!< Bit in the PRR associated with the AT24CXX device.

/** TWI address for AT24Cxx EEPROM:
 *
 * 1 0 1 0 E2 E1 E0 R/~W	24C01/24C02
 * 1 0 1 0 E2 E1 A8 R/~W	24C04
 * 1 0 1 0 E2 A9 A8 R/~W	24C08
 * 1 0 1 0 A10 A9 A8 R/~W	24C16
 */
#define AT24CXX_SLA	(0xA0) //!< SLA, see details above.
#define AT24CXX_W (0x00)   //!< TWI write flag.
#define AT24CXX_R (0x01)   //!< TWI read flag.

#define AT24CXX_PAGE_SIZE (8)                          //!< Number of bytes that can be written in a row (Page).
#define AT24CXX_PRESCALER_CONF (AT24CXX_DIV1)          //!< Prescaler setting.
#define AT24CXX_BRR_CONF ((F_CPU / 100000UL - 16) / 2) //!< Baudrate register value.
/*================================= TYEPDEFS         =========================*/
typedef enum AT24CXX_PRESCALER_ENUM {
    AT24CXX_DIV1  = 0, //!< No prescaler.
    AT24CXX_DIV4  = 1, //!< Divide by 4 prescaler.
    AT24CXX_DIV16 = 2, //!< Divide by 16 prescaler.
    AT24CXX_DIV64 = 3, //!< Divide by 64 prescaler.
} at24cxx_prescaler_t;

typedef enum {
    AT24CXX_START        = 0x08, //!< A START condition has been transmitted.
    AT24CXX_REPSTART     = 0x10, //!< A repeated START condition has been transmitted.
    AT24CXX_MT_SLA_ACK   = 0x18, //!< SLA+W has been transmitted, ACK received.
    AT24CXX_MT_SLA_NACK  = 0x20, //!< SLA+W has been transmitted, NACK received.
    AT24CXX_MT_DATA_ACK  = 0x28, //!< Data byte has been transmitted, ACK has been received.
    AT24CXX_MT_DATA_NACK = 0x30, //!< Data byte has been transmitted, NACK has been received.
    AT24CXX_ARB_LOST     = 0x38, //!< Arbitration lost in SLA+W or SLA+Ror NACK.
    AT24CXX_MR_SLA_ACK   = 0x40, //!< SLA+R has been transmitted, ACK has been received.
    AT24CXX_MR_SLA_NACK  = 0x48, //!< SLA+R has been transmitted, NACK has been received.
    AT24CXX_MR_DATA_ACK  = 0x50, //!< Data byte has been received, ACK has been returned.
    AT24CXX_MR_DATA_NACK = 0x58, //!< Data byte has been received, NACK has been returned.
}at24cxx_state_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/* EOF */
