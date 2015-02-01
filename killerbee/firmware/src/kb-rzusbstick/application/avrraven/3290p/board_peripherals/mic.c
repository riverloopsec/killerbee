// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Interface for the AVRRAVEN's microphone.
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
 * $Id: mic.c 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "mic.h"

#include "board.h"
/*========================= MACROS                   =========================*/

/*========================= TYEPDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/
int mic_init(void)
{
    // Set port pin as input, no pullup
    MIC_DDR &= ~(1<<MIC_PIN);
    MIC_PUR &= ~(1<<MIC_PIN);

    // Power up audio filter op-amp
    AUDIO_POWER_DDR |= (1<<AUDIO_POWER_PIN);
    AUDIO_POWER_PORT &= ~(1<<AUDIO_POWER_PIN);
    
    return 0;
}

void mic_deinit(void)
{
    // Do not power down audio filter op-amp. It may be used by the speaker driver
}
/*EOF*/
