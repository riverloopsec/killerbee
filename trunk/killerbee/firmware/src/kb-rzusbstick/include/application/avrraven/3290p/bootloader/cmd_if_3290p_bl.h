// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  ..............
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
 * $Id: cmd_if_3290p_bl.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef CMD_IF_CONFIG_H
#define CMD_IF_CONFIG_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
/*================================= MACROS           =========================*/
#define CMD_IF_SIPC_BUFFER_SIZE (128)
/*================================= TYEPDEFS         =========================*/

/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
extern bool cmd_if_init(void);
extern void cmd_if_deinit(void);

extern void cmd_if_loc_rsp_ok(uint8_t *data);
extern void cmd_if_loc_rsp_fail(uint8_t *data);
extern void cmd_if_loc_evt_bl_entered(uint8_t *data);
extern void cmd_if_loc_evt_ota_packet(uint8_t *data);
extern void cmd_if_int_evt_startup(uint8_t *data);

#endif
/*EOF*/
