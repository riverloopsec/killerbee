// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the AVR Raven speaker
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
 * $Id: speaker.c 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "speaker.h"

#include "board.h"
/*========================= MACROS                   =========================*/

/*========================= TYEPDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*========================= IMPLEMENTATION           =========================*/

int speaker_init(void)
{
    // Set speaker pin as output, default low
    SPEAKER_DDR |= (1<<SPEAKER_PIN);
    SPEAKER_PORT &= ~(1<<SPEAKER_PIN);

    // Power up audio filter op-amp
    AUDIO_POWER_DDR |= (1<<AUDIO_POWER_PIN);
    AUDIO_POWER_PORT &= ~(1<<AUDIO_POWER_PIN);

    return 0;
}

void speaker_deinit(void)
{
    // Do not power down audio filter op-amp. It may be used by the mic driver
}
/*EOF*/
