// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Interface to internal and external EEPROM used for production parameters
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
 * $Id: eep.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

#include "eep.h"
#include "eep_conf.h"
#include "at24cxx.h"

//! \addtogroup grEEP
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static eeprom_source_t eep_source = EEP_SOURCE_UNSUPPORTED;
/*================================= PROTOTYPES       =========================*/


bool eep_init(void) {
    /* Initialize the external TWI EEPROM, and see if it is possible to read-back
     * the magic.
     */    
    if(true != at24cxx_init()) {
        at24cxx_deinit();
        return false;    
    }
    
    /* Read internal and external EEPROM magic. */
    uint8_t int_magic = 0xFF;
    EEGET(int_magic, EE_MAGIC_ADR);
    
    uint8_t ext_magic = 0xFF;
    (bool)at24cxx_read_byte(EE_MAGIC_ADR, &ext_magic);
    
    bool init_status = false;
    if (EE_MAGIC == ext_magic) {
        eep_source = EEP_SOURCE_EXTERNAL;
        init_status = true;
    } else if (EE_MAGIC == int_magic) {
        /* Save power by turning the TWI module off. */
        at24cxx_deinit();
        eep_source = EEP_SOURCE_INTERNAL;
        init_status = true;
    } else {
        /* Save power by turning the TWI module off. */
        at24cxx_deinit();
        /* No magic found. The init failed. */
        eep_source = EEP_SOURCE_UNSUPPORTED;
    }
    
    return init_status;
}


void eep_deinit(void) {
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        /* Save power by turning the TWI module off. */
        at24cxx_deinit(); 
    }
    
    eep_source = EEP_SOURCE_UNSUPPORTED;
}


uint8_t eep_read_production_day(void) {
    uint8_t prod_day = 0xFF;
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_PRODDAY_ADR, &prod_day);
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET(prod_day, EE_PRODDAY_ADR);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return prod_day;
}


uint8_t eep_read_production_month(void) {
    uint8_t prod_month = 0xFF;
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_PRODMONTH_ADR, &prod_month);
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET(prod_month, EE_PRODMONTH_ADR);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return prod_month;
}


uint8_t eep_read_production_year(void) {
    uint8_t prod_year = 0xFF;
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_PRODYEAR_ADR, &prod_year);
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET(prod_year, EE_PRODYEAR_ADR);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return prod_year;
}


uint64_t eep_read_ieee_address(void) {
    /* Union used to convert from byte read EEPROM address to number. */
    union {
        uint64_t nmbr;
        uint8_t array[sizeof(uint64_t)];
    } address_conv;
    
    address_conv.nmbr = 0;
    
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_MAC_ADR + 0, &(address_conv.array[7]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 1, &(address_conv.array[6]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 2, &(address_conv.array[5]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 3, &(address_conv.array[4]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 4, &(address_conv.array[3]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 5, &(address_conv.array[2]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 6, &(address_conv.array[1]));
        (bool)at24cxx_read_byte(EE_MAC_ADR + 7, &(address_conv.array[0]));
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET((address_conv.array[7]), EE_MAC_ADR + 0);
        EEGET((address_conv.array[6]), EE_MAC_ADR + 1);
        EEGET((address_conv.array[5]), EE_MAC_ADR + 2);
        EEGET((address_conv.array[4]), EE_MAC_ADR + 3);
        EEGET((address_conv.array[3]), EE_MAC_ADR + 4);
        EEGET((address_conv.array[2]), EE_MAC_ADR + 5);
        EEGET((address_conv.array[1]), EE_MAC_ADR + 6);
        EEGET((address_conv.array[0]), EE_MAC_ADR + 7);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return address_conv.nmbr;
}


uint8_t eep_read_cap_array(void) {
    uint8_t cap_array_setting = 0xFF;
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_CAPARRAY_ADR, &cap_array_setting);
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET(cap_array_setting, EE_CAPARRAY_ADR);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return cap_array_setting;
}


uint16_t eep_read_1v1(void) {
    /* Union used to convert from byte read EEPROM address to number. */
    union {
        uint16_t nmbr;
        uint8_t array[sizeof(uint16_t)];
    } v_conv;
    
    v_conv.nmbr = 0;
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_1V1_CAL_VALUE_ADR + 1, &(v_conv.array[0]));
        (bool)at24cxx_read_byte(EE_1V1_CAL_VALUE_ADR + 0, &(v_conv.array[1]));
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET((v_conv.array[0]), EE_1V1_CAL_VALUE_ADR + 1);
        EEGET((v_conv.array[1]), EE_1V1_CAL_VALUE_ADR + 0);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return v_conv.nmbr;
}


uint8_t eep_read_hw_revision(void) {
    uint8_t hw_rev = 0xFF;
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        (bool)at24cxx_read_byte(EE_HWREV_ADR, &hw_rev);
    } else if (EEP_SOURCE_INTERNAL == eep_source) {
        EEGET(hw_rev, EE_HWREV_ADR);
    } else {
        /* Function has been called without one of the supported EEPROM locations 
         * being valid.
         */
    }
    
    return hw_rev;
}


bool eep_read(eep_address_t eeprom_address, uint8_t *data_buffer_ptr, eep_length_t length) {
    /* Perform sanity checks. */
    if (EEP_SOURCE_UNSUPPORTED == eep_source) { return false; }
    if (NULL == data_buffer_ptr) { return false; }
    if (0 == length) { return false; }
    
    if (EEP_SOURCE_EXTERNAL == eep_source) {
        return at24cxx_read_continuous(eeprom_address, length, data_buffer_ptr);
    } else {
        do {
            EEGET(*data_buffer_ptr, eeprom_address);
            data_buffer_ptr++;
            eeprom_address++;
            length--;
        } while (0 != length);
        
        return true;
    }
}


bool eep_write(eep_address_t eeprom_address, const uint8_t *data_buffer_ptr, eep_length_t length) {
    /* Perform sanity checks. */
    if (EEP_SOURCE_UNSUPPORTED == eep_source) { return false; }
    if (NULL == data_buffer_ptr) { return false; }
    if (0 == length) { return false; }

    if (EEP_SOURCE_EXTERNAL == eep_source) {
        return at24cxx_write_continuous(eeprom_address, length, data_buffer_ptr);
    } else {
        do {
            EEPUT(eeprom_address, *data_buffer_ptr);
            data_buffer_ptr++;
            eeprom_address++;
            length--;
        } while (0 != length);
    
        return true;
    }
}
//! @}
/*EOF*/
