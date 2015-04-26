// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Driver for the AVR Raven microphone
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
 * $Id: mic.h 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef __MIC_H__
#define __MIC_H__

/*========================= INCLUDES                 =========================*/
#include <stdbool.h>
#include <stdint.h>

#include "avrraven_3290p.h"

/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialize the microphone
 *
 * \return                              EOF on error
 */
extern int mic_init(void);


/*! \brief          De-initialize the microphone
 *
 */
extern void mic_deinit(void);

#endif // __MIC_H__
/*EOF*/
