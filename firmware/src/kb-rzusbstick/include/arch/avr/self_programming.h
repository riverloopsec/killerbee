// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for the AVR's self programming interface
 *
 *         The self programming interface is implemented as an API to the different
 *         features available in the AVR's bootloader.
 *
 * \defgroup apiSelfProgramming API for the Self Programming Module
 *          These functions are associated with the AVR's bootloader, and the 
 *          microcontroller's self programming features. The self programming 
 *          module contains features like:
 *             - Read, write and erase Flash memory.
 *             - Read, write and erase EEPROM.
 *             - Read signature.
 *             - Read fuses.
 *             - Read and write lockbits.
 *             - Read OSCCAL byte.
 *
 *          The self programming module will protect the bootloader it is used
 *          in from programming the bootloader region. Hence possibly loosing the
 *          possibility to do self progarmming
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
 * $Id: self_programming.h 41457 2008-05-09 08:15:04Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef SELF_PROGRAMMING_H
#define SELF_PROGRAMMING_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "self_programming_conf.h"

/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/


/*! \brief Read byte from address in Flash memory.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \param[in] byte_address Address in Flash memory to read.
 *  \returns Value of byte.
 */
uint8_t sp_read_flash(sp_adr_t byte_address);


/*! \brief Write Flash page.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \param[in] start_of_page_address     Address in Flash of first byte in page.
 *  \param[in] write_buffer              Pointer to memory where the page to be written is stored.
 *                                       The buffer must equal one bootloader page.
 *
 *  \retval true Flash page written.
 *  \retval false Flash page could not be written.
 */
bool sp_write_flash_page(sp_adr_t start_of_page_address, uint8_t *write_buffer);


/*! \brief Do chip erase. Bootloader will not be erased.
 *
 *  \ingroup apiSelfProgramming
 */
void sp_chip_erase(void);


/*! \brief Read byte from internal EEPROM.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \param[in] address Address of byte to read.
 *
 *  \returns Value of the addressed EEPROM location.
 */
uint8_t sp_read_eeprom(uint16_t address);


/*! \brief Write byte to internal EEPROM.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \param[in] address Address of byte to write.
 *  \param[in] eeprom_value New value for the addressed EEPROM location.
 */
void sp_write_eeprom(uint16_t address, uint8_t eeprom_value);


/*! \brief Write 0xFF to the internal EEPROM.
 *  \ingroup apiSelfProgramming
 */
void sp_erase_eeprom(void);


/*! \brief Read fuse's low byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns Low fuse value.
 */
uint8_t sp_read_fuse_low(void);


/*! \brief Read fuse's high byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns High fuse value.
 */
uint8_t sp_read_fuse_high(void);


/*! \brief Read fuse's extended byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns Extended fuse value.
 */
uint8_t sp_read_fuse_extended(void);


/*! \brief Read lockbits.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns Lockbits value.
 */
uint8_t sp_read_lockbits(void);


/*! \brief Write the AVR's lockbits. See the datasheet for lock options.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \param[in] lock_bits Updated value for the lock bit storage.
 */
void sp_write_lockbits(uint8_t lock_bits);

#ifdef SIGRD
// These functions will be included only if the SIGRD bit is defined

/*! \brief Read device's low signature byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns Low signature value.
 */
uint8_t sp_read_signature_low(void);


/*! \brief Read device's mid signature byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns Mid signature value.
 */
uint8_t sp_read_signature_mid(void);


/*! \brief Read device's high signature byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns High signature value.
 */
uint8_t sp_read_signature_high(void);
#endif


/*! \brief Read device's OSCCAL byte.
 *
 *  \ingroup apiSelfProgramming
 *
 *  \returns OSCCAL value.
 */
uint8_t sp_read_osccal(void);
/*\@}*/
#endif
/*EOF*/
