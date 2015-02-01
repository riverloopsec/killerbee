// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven speaker driver
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
 * $Id: speaker.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __SPEAKER_H__
#define __SPEAKER_H__
/*========================= INCLUDES                 =========================*/
#include "avrraven_3290p.h"

#include "compiler.h"

/*========================= MACROS                   =========================*/

/*========================= TYEPDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialize the speaker
 *
 * \return                              EOF on error
 */
int speaker_init(void);


/*! \brief          De-initialize the speaker
 *
 */
void speaker_deinit(void);


#endif // __SPEAKER_H__
/*EOF*/
