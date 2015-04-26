// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  AVR architecture specific implementation of the AT86RF230 radio transceiver
 *         low level driver.
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
 * $Id: rf230_avr.c 41219 2008-05-01 10:51:43Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "board.h"
#include "rf230.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
rf230_cb_handler_t rf230_callback_handler = NULL;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/


bool rf230_init(void) {
    /* Initialize local varaibles. */
    rf230_callback_handler = NULL;
    
    /* Enable IO pins used to control the radio transceiver. */
    DDR_SLP_TR |= (1 << SLP_TR);
    DDR_RST    |= (1 << RST);
    DDR_RST    |= (1 << TST);
    
    /* Enable the SPI module. */
    RF230_ENABLE_SPI_MODULE();
    
    /* Configure the SPI module. */
    RF230_DDR_SPI       |= (1 << RF230_DD_SS) | (1 << RF230_DD_SCK) | (1 << RF230_DD_MOSI);
    RF230_PORT_SPI      |= (1 << RF230_DD_SS) | (1 << RF230_DD_SCK); //Set SS and CLK high
    RF230_SPI_CTRL_REG   = (1 << RF230_SPI_SPE) | (1 << RF230_SPI_MSTR); //Enable SPI module and master operation.
    RF230_SPI_STATUS_REG = (1 << RF230_SPI_2X); //Enable doubled SPI speed in master mode.
    
    return true;
}


void rf230_deinit(void) {
    /* Disable the SPI module. */
    RF230_DISABLE_SPI_MODULE();
}


uint8_t rf230_register_read(uint8_t address) {
   /*Add the register read command to the register address. */
    address |= RF230_TRX_CMD_RR;
    
    ENTER_CRITICAL_REGION();
    
    RF230_SS_LOW();
    
    /*Send Register address and read register content.*/
    RF230_SPI_DATA_REG = address;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    address = RF230_SPI_DATA_REG;
    
    uint8_t register_value = 0;
    
    RF230_SPI_DATA_REG = register_value;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    register_value = RF230_SPI_DATA_REG;

    RF230_SS_HIGH();  
    
    LEAVE_CRITICAL_REGION();
    
    return register_value;
}


void rf230_register_write(uint8_t address, uint8_t value) {
    /* Add the Register Write command to the address. */
    address |= RF230_TRX_CMD_RW;
    
    ENTER_CRITICAL_REGION();    
    
    RF230_SS_LOW();
    
    /*Send Register address and write register content.*/
    RF230_SPI_DATA_REG = address;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    address = RF230_SPI_DATA_REG;
    
    RF230_SPI_DATA_REG = value;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    value = RF230_SPI_DATA_REG;
    
    RF230_SS_HIGH();
    
    LEAVE_CRITICAL_REGION();
}


uint8_t rf230_subregister_read(uint8_t address, uint8_t mask, uint8_t position) {
    /* Read current register value and mask out subregister. */
    uint8_t register_value = rf230_register_read(address);
    register_value &= mask;
    register_value >>= position;
    
    return register_value;
}


void rf230_subregister_write(uint8_t address, uint8_t mask, uint8_t position, uint8_t value) {
    /* Read current register value and mask area outside the subregister. */
    uint8_t register_value = rf230_register_read(address);
    register_value &= ~mask;
    
    /* Start preparing the new subregister value. shift in place and mask. */
    value <<= position;
    value &= mask;
    
    value |= register_value;

    /* Write the modified register value. */
    rf230_register_write(address, value);
}


void rf230_frame_write(uint8_t length, const uint8_t *wr_buffer) {
    ENTER_CRITICAL_REGION();
 
    RF230_SS_LOW();
    
    /*SEND FRAME WRITE COMMAND AND FRAME LENGTH.*/
    RF230_SPI_DATA_REG = RF230_TRX_CMD_FW;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    uint8_t dummy_read = RF230_SPI_DATA_REG;
        
    RF230_SPI_DATA_REG = length;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    dummy_read = RF230_SPI_DATA_REG;
    
    /*Download to the Frame Buffer. */
    do {
        
        RF230_SPI_DATA_REG = *wr_buffer++;
        --length;
        
        RF230_WAIT_FOR_SPI_TX_COMPLETE();
        
        dummy_read = RF230_SPI_DATA_REG;
    } while (length != 0);
    
    RF230_SS_HIGH();
    
    LEAVE_CRITICAL_REGION();
}


void rf230_frame_write_P(uint8_t length, PROGMEM_BYTE_ARRAY_T wr_buffer) {
    ENTER_CRITICAL_REGION();
 
    RF230_SS_LOW();
    
    /*SEND FRAME WRITE COMMAND AND FRAME LENGTH.*/
    RF230_SPI_DATA_REG = RF230_TRX_CMD_FW;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    uint8_t dummy_read = RF230_SPI_DATA_REG;
        
    RF230_SPI_DATA_REG = length;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    dummy_read = RF230_SPI_DATA_REG;
    
    /*Download to the Frame Buffer. */
    do {
        
        RF230_SPI_DATA_REG = PROGMEM_READ_BYTE(wr_buffer);
        wr_buffer++;
        --length;
        
        RF230_WAIT_FOR_SPI_TX_COMPLETE();
        
        dummy_read = RF230_SPI_DATA_REG;
    } while (length != 0);
    
    RF230_SS_HIGH();
    
    LEAVE_CRITICAL_REGION();
}


void rf230_frame_read(uint8_t length, uint8_t *rd_buffer) {
    ENTER_CRITICAL_REGION();
    
    RF230_SS_LOW();
    
    /* Send frame read command. */
    RF230_SPI_DATA_REG = RF230_TRX_CMD_FR;
    RF230_WAIT_FOR_SPI_TX_COMPLETE();
    *rd_buffer = RF230_SPI_DATA_REG;
        
    /* Download to the Frame Buffer. */
    do {
        
        RF230_SPI_DATA_REG = 0x00;
        RF230_WAIT_FOR_SPI_TX_COMPLETE();
        *rd_buffer = RF230_SPI_DATA_REG;
        rd_buffer++;
        --length;
    } while (length != 0);
    
    RF230_SS_HIGH();
 
    LEAVE_CRITICAL_REGION();
}


void rf230_set_callback_handler(rf230_cb_handler_t handler) {
    ENTER_CRITICAL_REGION();
    rf230_callback_handler = handler;
    LEAVE_CRITICAL_REGION();
}


rf230_cb_handler_t rf230_get_callback_handler(void) {
    rf230_cb_handler_t handler = NULL;
    
    ENTER_CRITICAL_REGION();
    handler = rf230_callback_handler;
    LEAVE_CRITICAL_REGION();
    
    return handler;
}


void rf230_clear_callback_handler(void) {
    ENTER_CRITICAL_REGION();
    rf230_callback_handler = NULL;
    LEAVE_CRITICAL_REGION();
}
/* EOF */
