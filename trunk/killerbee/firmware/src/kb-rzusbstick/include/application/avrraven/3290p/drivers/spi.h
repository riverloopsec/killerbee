// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for spi.c
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
 * $Id: spi.h 41457 2008-05-09 08:15:04Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __spi_H__
#define __spi_H__

#include "compiler.h"
#include "spi_conf.h"

//! Type for selecting SPI mode
typedef enum {
    SPI_MODE_0 = (0<<CPHA) | (0<<CPOL),
    SPI_MODE_1 = (1<<CPHA) | (0<<CPOL),
    SPI_MODE_2 = (0<<CPHA) | (1<<CPOL),
    SPI_MODE_3 = (1<<CPHA) | (1<<CPOL)
} SpiMode_t;

//! Type for selecting SPI data order
typedef enum {
    SPI_DORD_MSB_FIRST = (0<<DORD),
    SPI_DORD_LSB_FIRST = (1<<DORD)
} SpiDataOrder_t;

//! Type for selecting maaster/slave
typedef enum {
    SPI_SLAVE  = (0<<MSTR),
    SPI_MASTER = (1<<MSTR)
} SpiMaster_t;

//! Offset used to merge both SPRn and SPI2X into same 8 bit variable
#define MAGIC_OFFSET 2

//! Type for selecting SPI speed
typedef enum {
    SPI_SPEED_DIV_2   = (0<<SPR1) | (0<<SPR0) | (1<<(SPI2X+MAGIC_OFFSET)),
    SPI_SPEED_DIV_4   = (0<<SPR1) | (0<<SPR0) | (0<<(SPI2X+MAGIC_OFFSET)),
    SPI_SPEED_DIV_8   = (0<<SPR1) | (1<<SPR0) | (1<<(SPI2X+MAGIC_OFFSET)),
    SPI_SPEED_DIV_16  = (0<<SPR1) | (1<<SPR0) | (0<<(SPI2X+MAGIC_OFFSET)),
    SPI_SPEED_DIV_32  = (1<<SPR1) | (0<<SPR0) | (1<<(SPI2X+MAGIC_OFFSET)),
    SPI_SPEED_DIV_64  = (1<<SPR1) | (0<<SPR0) | (0<<(SPI2X+MAGIC_OFFSET)),
    SPI_SPEED_DIV_128 = (1<<SPR1) | (1<<SPR0) | (0<<(SPI2X+MAGIC_OFFSET))
} SpiSpeed_t;

//! Type for selecting buffered or blocking transaction
typedef enum {
    SPI_TRANS_BUFFERED,
    SPI_TRANS_BLOCKING
} SpiTransMode_t;

//! SPI configuration type
typedef struct {
    struct{
        int size;
        uint8_t *out_ptr;
        uint8_t *in_ptr;
        uint8_t *end;
        #ifdef USE_SPI_BUFFER
        uint8_t data[SPI_BUFFER_SIZE];
        #endif
    } buffer;
    bool busy;
}spi_t;

//! SPI transaction type
typedef struct {
    uint8_t *Data;
}SpiTrans_t;


/*! \brief  Initialize the SPI interface
 *
 * \param[in]      SpiMasterEnable      Select master/slave operation
 * \param[in]      SpiMode              Select SPI mode (0..3)
 * \param[in]      DataOrder            LSB/MSB first
 * \param[in]      SpiSpeed             Prescale of SPI interface
 *
 * \return							    Non-zero on error
 */
int spi_init(SpiMaster_t SpiMasterEnable, SpiMode_t SpiMode, SpiDataOrder_t DataOrder, SpiSpeed_t SpiSpeed);


/*! \brief  De-initialize the SPI interface
 *
 */
void spi_deinit(void);


/*! \brief  Buffered SPI transaction
 *
 * \param[in,out]     Data               Pointer to buffer for data
 * \param[in]      Size                 Number of bytes to send/recieve
 *
 * \return							    Non-zero if SPI HW busy transeiving
 */
int spi_rw_buf(uint8_t *Data, int Size);


/*! \brief  Unbuffered SPI transaction
 *
 * \param[in]      OutData              Pointer to buffer for data to send
 * \param[out]     InData               Pointer to buffer for received data
 * \param[in]      Size                 Number of bytes to send/recieve
 *
 * \return							    Non-zero if SPI HW busy transeiving
 */
int spi_rw(uint8_t *OutData, uint8_t *InData, int Size);


/*! \brief  Check if SPI is available
 *
 *
 * \return								Non-zero if SPI HW busy transeiving
 */
bool spi_busy(void);

#endif // __spi_H__
/*EOF*/
