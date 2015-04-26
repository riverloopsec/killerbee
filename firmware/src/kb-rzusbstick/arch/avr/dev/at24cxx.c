// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements a low level TWI driver for the ATMEL AT24Cxx
 *         serial EEPROM devices. The driver is polled
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
 * $Id: at24cxx.c 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "at24cxx.h"
#include "at24cxx_conf.h"
#include "board.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
bool at24cxx_init(void) {
    /* Enable the TWI module. */
    AT24CXX_PRR &= ~(1 << AT24CXX_PRR_BIT);
    
    /* Set TWI speed. */
    AT24CXX_BRR = AT24CXX_BRR_CONF;
    AT24CXX_SET_PRESCALER(AT24CXX_PRESCALER_CONF);
    
    return true;
}


void at24cxx_deinit(void) {
    /* Disable the TWI module. */
    AT24CXX_PRR |= (1 << AT24CXX_PRR_BIT);
}


bool at24cxx_write_byte(uint8_t byte_address, uint8_t byte_value) {
    /* Send start condition and wait for reply. */
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTA) | (1 << AT24CXX_TWEN);
    uint8_t time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
     
    /* Check that the start condition was successfully transmitted. */
    if (AT24CXX_START != AT24CXX_GET_STATUS()) {
        AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTO) | (1 << AT24CXX_TWEN);
        return false;
    }
    
    /* Send SLA + W. */
    AT24CXX_DATA_REG = AT24CXX_SLA | AT24CXX_W;
    AT24CXX_CONTROL_REG |= (1 << AT24CXX_TWINT);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the SLA + W was transmitted. */
    if (AT24CXX_MT_SLA_ACK != AT24CXX_GET_STATUS()) {
        AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTO) | (1 << AT24CXX_TWEN);
        return false;
    }
    
    /* Send address. */
    AT24CXX_DATA_REG = byte_address;
    AT24CXX_CONTROL_REG |= (1 << AT24CXX_TWINT);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the address was acknowledged. */
    if (AT24CXX_MT_DATA_ACK != AT24CXX_GET_STATUS()) {
        AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTO) | (1 << AT24CXX_TWEN);
        return false;
    }
    
    /* Send data. */
    AT24CXX_DATA_REG = byte_value;
    AT24CXX_CONTROL_REG |= (1 << AT24CXX_TWINT);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the address was acknowledged. */
    bool tx_status = false;
    if (AT24CXX_MT_DATA_ACK == AT24CXX_GET_STATUS()) {
        tx_status = true;
    }
    
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTO) | (1 << AT24CXX_TWEN);
    
    return tx_status;
}


bool at24cxx_write_continuous(uint8_t start_address, uint16_t length, uint8_t const *wr_buffer) {
    return false;
}


bool at24cxx_read_byte(uint8_t byte_address, uint8_t *read_byte) {
    /* Perform sanity check on function parameters. */
    if (NULL == read_byte) { return false; }
    
    /* Initialize local variable. */
    bool rx_status = false;
    
    /* Send start condition. */
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTA) | (1 << AT24CXX_TWEN);
    uint8_t time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the start condition was successfully transmitted. */
    if ((AT24CXX_START != AT24CXX_GET_STATUS()) || (0 == time_out)) {
        goto quit;
    }
    
    /* Send SLA + W. */
    AT24CXX_DATA_REG = AT24CXX_SLA | AT24CXX_W;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the SLA + W was ACK'ed. */
    if (AT24CXX_MT_SLA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Send address. */
    AT24CXX_DATA_REG = byte_address;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the address was ACK'ed. */
    if (AT24CXX_MT_DATA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Send Rep. start. Must change to receive mode to fetch data from the AT24CXX 
     * serial EEPROM.
     */
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTA) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    if (AT24CXX_REPSTART != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Send SLA + R. */
    AT24CXX_DATA_REG = AT24CXX_SLA | AT24CXX_R;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the SLA + R was transmitted. */
    if (AT24CXX_MR_SLA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Poll for data. */
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the NACK was received. If NACK was received it is time to
     * read data from the TWI data register.
     */
    if (AT24CXX_MR_DATA_NACK == AT24CXX_GET_STATUS() || (0 == time_out)) {
        *read_byte = AT24CXX_DATA_REG;
        rx_status = true;
    }
    
    /* This label is used to terminate the TWI transaction with a stop condition. */
    quit:
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTO) | (1 << AT24CXX_TWEN);
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT);
    
    return rx_status;
}


bool at24cxx_read_continuous(uint8_t start_address, uint16_t length, uint8_t *rd_buffer) {
    /* Perform sanity check on function parameters. */
    if (0 == length) { return false;}
    if (NULL == rd_buffer) { return false;}
    
    /* Initialize local variable. */
    bool rx_status = false;
    
    /* Send start condition. */
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTA) | (1 << AT24CXX_TWEN);
    uint8_t time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the start condition was successfully transmitted. */
    if ((AT24CXX_START != AT24CXX_GET_STATUS()) || (0 == time_out)) {
        goto quit;
    }
    
    /* Send SLA + W. */
    AT24CXX_DATA_REG = AT24CXX_SLA | AT24CXX_W;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the SLA + W was ACK'ed. */
    if (AT24CXX_MT_SLA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Send start address. */
    AT24CXX_DATA_REG = start_address;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the address was ACK'ed. */
    if (AT24CXX_MT_DATA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Send Rep. start. Must change to receive mode to fetch data from the AT24CXX 
     * serial EEPROM.
     */
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTA) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    if (AT24CXX_REPSTART != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    /* Send SLA + R. */
    AT24CXX_DATA_REG = AT24CXX_SLA | AT24CXX_R;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEN);
    
    time_out = 0xFF;
    
    /* Busy wait for status. */
    do {
        time_out--;
    } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
             (0 != time_out));
    
    /* Check that the SLA + R was transmitted. */
    if (AT24CXX_MR_SLA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
        goto quit;
    }
    
    do {
        /* Send ACK. */
        AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWEA) | (1 << AT24CXX_TWEN);
        time_out = 0xFF;
    
        /* Busy wait for status. */
        do {
            time_out--;
        } while (((AT24CXX_CONTROL_REG & (1 << AT24CXX_TWINT)) != (1 << AT24CXX_TWINT)) &&
                 (0 != time_out));
    
        /* Check that the NACK was received. If NACK was received it is time to
         * read data from the TWI data register.
         */
        if (AT24CXX_MR_DATA_ACK != AT24CXX_GET_STATUS() || (0 == time_out)) {
            goto quit;
        } else {
            *rd_buffer = AT24CXX_DATA_REG;
        }
        
        length--;
        rd_buffer++;
    } while (0 != length);    
    
    /* This label is used to terminate the TWI transaction with a stop condition. */
    quit:
    rd_buffer = NULL;
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT) | (1 << AT24CXX_TWSTO) | (1 << AT24CXX_TWEN);
    AT24CXX_CONTROL_REG = (1 << AT24CXX_TWINT);
    
    return rx_status;
}
/* EOF */
