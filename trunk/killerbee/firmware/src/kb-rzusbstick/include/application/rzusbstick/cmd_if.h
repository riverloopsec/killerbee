// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This is the header file for the RZUSBSTICK's command interface.
 *
 * \defgroup applCmdRzUsbStick The RZUSBSTICK Command Interface
 * \ingroup applRzUsbStick
 *
 *         The command interface is implemented as a parser that takes a formatted
 *         string from the USB interface, extracts parameters, and executes the
 *         associated internal command handler. Two types of commands are defined:
 *         -# Synchronous: Most of the supported commands are synchronous. That
 *                         implies that all commands return with a response to the
 *                         caller. The timeout for each command will vary.
 *         -# Asynchronous: These commands are actually non periodic events that
 *                          are defined for the command interface to support. 
 *                          These event-commands are stored in queue that must 
 *                          be polled by the user application to avoid overrun
 *                          situations. A typical asynchronous command is an indication
 *                          from the communication stack.
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
 * $Id: cmd_if.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef CMD_IF_H
#define CMD_IF_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

//! \addtogroup applCmdRzUsbStick
//! @{
/*================================= MACROS           =========================*/
#define NWK_EVENT_FIFO_SIZE (5) //!< Number of NWK_EVENTs that can be stored.
#define CMD_EVENT_SIZE (127 + 20) //!< Size of a CMD_EVENT in bytes.
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*! \brief This function is called to initialize the command interface.
 *
 *  \ingroup applCmdRzUsbStick
 *
 *  \retval true The command interface was successfully initialized.
 *  \retval false The command interface could not be started. This is a serious
 *                error, and correct operation cannot be guaranteed.
 */
bool cmd_if_init(void);

/*! \brief This function is called to disable the command interface.
 *  \ingroup applCmdRzUsbStick
 */
void cmd_if_deinit(void);

/*! \brief The command interface will store asynchronous commands in a queue that
 *         must be polled by the user (PC's USB driver). The task function ensures
 *         that data is pushed to the DPRAM of the AVR's USB macro.
 *
 *  \ingroup applCmdRzUsbStick
 */
void cmd_if_task(void);
//! @}
#endif
/*EOF*/
