// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Hardware access layer
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
 * $Id: hal_m3290_bl.c 41506 2008-05-13 07:03:28Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#include "avrraven_3290p_bl.h"

#include "df.h"

#include "macros.h"

int fs_hal_init(void)
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
    return spi_rw(out_data, in_data, count);
}
