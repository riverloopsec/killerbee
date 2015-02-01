// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Hardware access layer for ATmega3290p on the AVR Raven
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
 * $Id: hal_m3290.c 41500 2008-05-12 21:27:34Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#include "avrraven_3290p.h"

#include "compiler.h"
#include "df.h"
#include "sfs.h"
#include "macros.h"

int sfs_hal_init(void)
{
    df_init();
    return 0;
}

int df_hal_init(void)
{
    spi_init(SPI_MASTER, SPI_MODE_3, SPI_DORD_MSB_FIRST, SPI_SPEED_DIV_16);

    // Configure RESET_n pin
    IO_OUTPUT(DF,1,DF_RST);

    df_rst_release();

    return 0;
}

int df_hal_rw(uint8_t *out_data, uint8_t *in_data, int count)
{
    // If low batt, do not try to write data flash
    if (avrraven.status.batt_low == true) {
        return EOF;
    }
    return spi_rw(out_data, in_data, count);
}
/*EOF*/
