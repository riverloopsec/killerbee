// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  File that implements self programming functionality for the AVR
 *         microcontroller
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
 * $Id: self_programming.c 41438 2008-05-09 00:08:19Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "self_programming.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/


uint8_t sp_read_flash(sp_adr_t byte_address) {
    return SF_LOAD_PROGRAM_MEMORY(byte_address);
}


bool sp_write_flash_page(sp_adr_t start_of_page_address, uint8_t *write_buffer) {
    if (NULL == write_buffer) { return false; }
    if (start_of_page_address >= SF_BOOTLOADER_START) { return false; }
    
    /* Erase page. */
    SF_WAIT_FOR_SPM();
    SF_PAGE_ERASE(start_of_page_address);
    SF_WAIT_FOR_SPM();
    
    /* Load temporary page buffer. */
    for (uint16_t intra_page_address = 0; intra_page_address < SF_PAGE_BYTE_SIZE; intra_page_address += 2) {
        uint16_t temp_word = ((write_buffer[1] << 8) | write_buffer[0]);
        SF_FILL_TEMP_WORD(intra_page_address, temp_word);
        write_buffer += 2;
    }
    
    /* Write Page. */
    SF_WAIT_FOR_SPM();
    SF_PAGE_WRITE(start_of_page_address);
    SF_WAIT_FOR_SPM();
    SF_ENABLE_RWW_SECTION();
    SF_WAIT_FOR_SPM();
    
    return true;
}


void sp_chip_erase(void) {
    ENTER_CRITICAL_REGION();
    
    for (sp_adr_t address = 0; address < SF_APPLICATION_END; address += SF_PAGE_BYTE_SIZE) {
        SF_WAIT_FOR_SPM();
        SF_PAGE_ERASE(address);
    }
    
    LEAVE_CRITICAL_REGION();
}


uint8_t sp_read_eeprom(uint16_t address) {
    uint8_t eeprom_byte = 0xFF;
    
    ENTER_CRITICAL_REGION();
    
    EEGET(eeprom_byte, address);
        
    LEAVE_CRITICAL_REGION();
    
    return eeprom_byte;
}


void sp_write_eeprom(uint16_t address, uint8_t eeprom_value) {
    ENTER_CRITICAL_REGION();
    
    EEPUT(address, eeprom_value);  
    
    LEAVE_CRITICAL_REGION();
}


void sp_erase_eeprom(void) {
    ENTER_CRITICAL_REGION();
    
    for (uint16_t address = 0; address < SF_EEPROM_SIZE; address++) {
        EEPUT(address, 0xFF); 
    } 
    
    LEAVE_CRITICAL_REGION();
    
}


uint8_t sp_read_fuse_low(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t low_fuses = SF_GET_LOW_FUSES();
    LEAVE_CRITICAL_REGION();
    
    return low_fuses;
}


uint8_t sp_read_fuse_high(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t high_fuses = SF_GET_HIGH_FUSES();
    LEAVE_CRITICAL_REGION();
    
    return high_fuses;
}


uint8_t sp_read_fuse_extended(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t extended_fuses = SF_GET_EXTENDED_FUSES();
    LEAVE_CRITICAL_REGION();
    
    return extended_fuses;
}


uint8_t sp_read_lockbits(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t lock_bits = SF_GET_LOCK_BITS();
    LEAVE_CRITICAL_REGION();
    
    return lock_bits;
}


void sp_write_lockbits(uint8_t lock_bits) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    SF_SET_LOCK_BITS(lock_bits);
    LEAVE_CRITICAL_REGION();
}

#ifdef SIGRD
// These functions will be included only if the SIGRD bit is defined
uint8_t sp_read_signature_low(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t signature_low = SF_GET_SIGNATURE_LOW();
    LEAVE_CRITICAL_REGION();
    
    return signature_low;
}


uint8_t sp_read_signature_mid(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t signature_mid = SF_GET_SIGNATURE_MID();
    LEAVE_CRITICAL_REGION();
    
    return signature_mid;
}


uint8_t sp_read_signature_high(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t signature_high = SF_GET_SIGNATURE_HIGH();
    LEAVE_CRITICAL_REGION();
    
    return signature_high;
}


uint8_t sp_read_osccal(void) {
    ENTER_CRITICAL_REGION();
    SF_WAIT_FOR_SPM();
    uint8_t osccal = SF_GET_OSCCAL();
    LEAVE_CRITICAL_REGION();
    
    return osccal;
}
#endif
/*EOF*/
