// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This is the header file defined for the ATmega1284p's command interface.
 *
 * \defgroup applCmdAVRRAVEN1284p The AVRRAVEN Command Interface for the ATmega1284p
 * \ingroup applAVRRAVEN
 *
 *      The command interface consists of handlers for any request that the
 *      ATmega1284p can receive from the connected ATmega3290p. Also the
 *      command interface is responsible for forwarding any relevant indications
 *      from the ZigBee NWK layer.
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
 * $Id: cmd_if_1284p.h 41236 2008-05-02 16:41:57Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef CMD_IF_1284P_H
#define CMD_IF_1284P_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup applCmdAVRRAVEN1284p
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*! \brief This function will initialize the ATmega1284p's command interface.
 *
 *  \ingroup applCmdAVRRAVEN1284p
 *
 *  \retval true The command interface was successfully started.
 *  \retval false The command interface could not be started. Either due to an error
 *                with the SIPC or IEEE 802.15.4 MAC.
 */
bool cmd_if_1284p_init(void);

/*! \brief This function will disable the command interface and put the utilized
 *         HW modules in low power states (SPIC and IEEE 802.15.4 MAC).
 *
 *  \ingroup applCmdAVRRAVEN1284p 
 */
void cmd_if_1284p_deinit(void);
//! @}
#endif
/*EOF*/
