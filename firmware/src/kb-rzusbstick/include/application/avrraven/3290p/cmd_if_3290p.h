// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Command Interface defined for the ATmega3290p microcontroller.
 *
 * \defgroup applCmdAVRRAVEN3290p The AVRRAVEN Command Interface for the ATmega3290p
 * \ingroup applAVRRAVEN
 *
 *      The command interface consists of handlers for any request that the
 *      ATmega3290p can receive from the connected ATmega1284p (SIPC). And also
 *      handlers for the commands received on the air interface, joystick input 
 *      etc.
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
 * $Id: cmd_if_3290p.h 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef CMD_IF_3290P_H
#define CMD_IF_3290P_H

/*============================     INCLUDES       ============================*/
#include <stdbool.h>

//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*============================     MACROS         ============================*/

/*============================     TYPEDEFS       ============================*/

/*============================     VARIABLES      ============================*/

/*============================     FUNCTIONS      ============================*/

/*! \brief  Initialization of the command interface.
 *
 *          The initalization function uses a static data buffer and 
 *          initializes the sipc communction interface with a pointer to this
 *          buffer.
 *
 *  \retval true Command interface initialized successfully.
 *  \retval false Command interface failed to initialize.
 */
extern bool cmd_if_init(void);


/*! \brief  De-initialization of the command interface.
 *
 *          The de-initalization function also de-initializes the sipc
 *          communction interface.
 *
 */
extern void cmd_if_deinit(void);
//! @}
#endif
/*EOF*/
