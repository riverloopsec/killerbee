// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the API for the AT24CXX serial EEPROM driver.
 *
 * \defgroup grAt24CxxLowLevel The AT24Cxx Serial EEPROM Low Level Driver
 * \ingroup grLowLevelDrivers
 *
 * The AT24Cxx is a family of serial EEPROMs from Atmel. These EEPROMs support
 * operational voltages down to 1.8Volts, and are therfore well suited for battery
 * applications. The serial interface for the AT24Cxx family is TWI (Two Wire Interface).
 * This particular driver is polled (Non interrupt driven)
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
 * $Id: at24cxx.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef AT24CXX_H
#define AT24CXX_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup grAt24CxxLowLevel
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*! \brief This function will initialize the AT24CXX serial EEPROM.
 *  \note Must be called before any of the access functions.
 *
 *  \retval true AT24CXX device ready to use.
 *  \retval false Not able to initialize the AT24CXX device.
 */
bool at24cxx_init(void);


/*! \brief This function will disable the TWI interface. */
void at24cxx_deinit(void);


/*! \brief Write single byte to the serial EEPROM. 
 *  
 *  \param[in] byte_address Address of byte to write.
 *  \param[in] byte_value Value that will be written to the specified address.
 *
 *  \retval true Byte written successfully.
 *  \retval false Byte could not be written.
 */
bool at24cxx_write_byte(uint8_t byte_address, uint8_t byte_value);


/*! \brief Write bytes continously to the serial EEPROM. 
 *  
 *  \param[in] start_address Address of first byte in transaction.
 *  \param[in] length Number of bytes to write.
 *  \param[in] wr_buffer Pointer to array where the bytes to be written are stored.
 *
 *  \retval true Bytes written successfully.
 *  \retval false Bytes could not be written.
 */
bool at24cxx_write_continuous(uint8_t start_address, uint16_t length, uint8_t const *wr_buffer);


/*! \brief Read single byte from serial EEPROM. 
 *  
 *  \param[in] byte_address Address of byte to read.
 *  \param[out] read_byte Pointer to memory where the read byte will be stored.
 *
 *  \retval true Byte read successfully.
 *  \retval false Byte could not be read.
 */
bool at24cxx_read_byte(uint8_t byte_address, uint8_t *read_byte);


/*! \brief Read bytes continously from the serial EEPROM. 
 *  
 *  \param[in] start_address Address of first byte to read.
 *  \param[in] length Number of bytes to read.
 *  \param[out] rd_buffer Pointer to memory where the read bytes will be stored.
 *
 *  \retval true Bytes read successfully.
 *  \retval false Bytes could not be read.
 */
bool at24cxx_read_continuous(uint8_t start_address, uint16_t length, uint8_t *rd_buffer);
//! @}
#endif
/* EOF */
