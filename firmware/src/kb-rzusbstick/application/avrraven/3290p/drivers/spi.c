// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Simple driver for the AVR SPI module
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
 * $Id: spi.c 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "spi.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "macros.h"

/*========================= MACROS                   =========================*/
//! Disable SPI interrupt
#define SPI_INTERRUPT_DISABLE   SPCR &= ~(1<<SPIE)

//! Enable SPI interrupt
#define SPI_INTERRUPT_ENABLE    SPCR |= (1<<SPIE)


/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Spi configuration
static spi_t sgSpi;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int spi_init(SpiMaster_t SpiMasterEnable, SpiMode_t SpiMode, SpiDataOrder_t DataOrder, SpiSpeed_t SpiSpeed)
{
    // enable SPI module
    PRR &= ~(1 << PRSPI);

    // I/O interface
    IO_INPUT_NO_PULLUP(SPI, 1, MISO);
    IO_OUTPUT(SPI, 3, MOSI, SCK, SS);

    // set default value SS pin
    IO_SET(SPI,1,SS);

    // Clear interrupt flag (and write SPI2X)
    IO_PUT(SPSR,(1<<SPIF) | (SpiSpeed & (1<<(SPI2X+MAGIC_OFFSET)))>>MAGIC_OFFSET);

    // init buffer
    sgSpi.buffer.in_ptr = NULL;
    sgSpi.buffer.out_ptr = NULL;
    sgSpi.buffer.end = NULL;
    sgSpi.buffer.size = 0;
    sgSpi.busy = false;

    // Configure SPI and enable the SPI interface
    SPCR = (uint8_t)SpiMasterEnable
        | (uint8_t)SpiMode
        | (uint8_t)DataOrder
        | ((uint8_t)(SpiSpeed & ((1<<SPR1) | (1<<SPR0))))
        | (1<<SPE);
    
    return 0;
}


void spi_deinit(void)
{
    // disable SPI module
    PRR |= (1 << PRSPI);
}

#ifdef USE_SPI_BUFFER
int spi_rw_buf(uint8_t *Data, int Size)
{
    uint8_t *temp_ptr;

    // Transaction on-going?
    if (sgSpi.busy) {
        return -1;
    }

    // Try to send too many bytes?
    if (Size > SPI_BUFFER_SIZE) {
        return -1;
    }

    // Initialize data pointers
    temp_ptr = sgSpi.buffer.out_ptr = sgSpi.buffer.in_ptr = sgSpi.buffer.data;
    sgSpi.buffer.end = sgSpi.buffer.data + Size;

    // Copy data
    while (Size--) {
        *temp_ptr++ = *Data++;
    }

    // Initiate transaction
    SPSR |= (1<<SPIF);
    SPDR = *sgSpi.buffer.out_ptr++;
    sgSpi.busy = true;
    SPI_INTERRUPT_ENABLE;

    return 0;
}
#endif

int spi_rw(uint8_t *OutData, uint8_t *InData, int Size)
{
    register uint8_t temp;

    // Transaction on-going?
    if (sgSpi.busy) {
        return -1;
    }

    // Initiate transaction
    sgSpi.busy = true;
    SPSR |= (1<<SPIF);

    // Do transaction
    SPDR = *OutData++;
    while (--Size) {
        while (!(SPSR & (1<<SPIF))){;}
        temp = SPDR;
        SPDR = *OutData++;
        *InData++ = temp;
    }
    while (!(SPSR & (1<<SPIF))){;}
    *InData = SPDR;

    // Close transaction
    sgSpi.busy = false;

    return 0;
}

bool spi_busy(void)
{
    return sgSpi.busy;
}

#ifdef USE_SPI_BUFFER
ISR(SPI_STC_vect)
{
    register uint8_t temp;

    if (sgSpi.buffer.out_ptr == sgSpi.buffer.end) { // if buffer empty
        *sgSpi.buffer.in_ptr = SPDR;                // read last reply
        SPI_INTERRUPT_DISABLE;                      // disable SPI interrupt
        sgSpi.busy = false;
    }
    else {
        temp = SPDR;
        SPDR = *sgSpi.buffer.out_ptr++;
        *sgSpi.buffer.in_ptr++ = temp;
    }
}
#endif
/*EOF*/
