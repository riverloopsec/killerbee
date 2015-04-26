// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header file for the command interface for the RZUSBSTICK bootloader.
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
 * $Id: cmd_if_bootloader.h 41438 2008-05-09 00:08:19Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef CMD_IF_BOOTLOADER_H
#define CMD_IF_BOOTLOADER_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "self_programming_conf.h"

//! \addtogroup grRzUsbStickBootloader
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*! \brief CMD_SIGN_OFF struct. */
typedef struct CMD_SIGN_OFF_STRUCT_TAG {
    uint8_t type;
} cmd_sign_off_t;

/*! \brief CMD_SIGN_ON struct. */
typedef struct CMD_SIGN_ON_STRUCT_TAG {
    uint8_t type;
} cmd_sign_on_t;

/*! \brief CMD_BOOT_WRITE_LOCKBITS struct. */
typedef struct CMD_BOOT_WRITE_LOCKBITS_STRUCT_TAG {
    uint8_t type;
    uint8_t lockbits;
} cmd_boot_write_lockbits_t;

/*! \brief CMD_BOOT_PROG_FLASH struct. */
typedef struct CMD_BOOT_PROG_FLASH_STRUCT_TAG {
    uint8_t type;
    uint32_t address;
    uint8_t flash_page[SF_PAGE_BYTE_SIZE];
} cmd_boot_program_flash_t;

/*! \brief CMD_BOOT_PROG_EEPROM struct. */
typedef struct CMD_BOOT_PROG_EEPROM_STRUCT_TAG {
    uint8_t type;
    uint16_t address;
    uint8_t value;
} cmd_boot_program_eeprom_t;

/*! \brief CMD_BOOT_READ_FLASH struct. */
typedef struct CMD_BOOT_READ_FLASH_STRUCT_TAG {
    uint8_t type;
    uint32_t address;
} cmd_boot_read_flash_t;

/*! \brief CMD_BOOT_READ_EEPROM struct. */
typedef struct CMD_BOOT_READ_EEPROM_STRUCT_TAG {
    uint8_t type;
    uint16_t address;
} cmd_boot_read_eeprom_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function must be called to initialize the bootloader's command 
 *         interface.
 *
 *  \retval true The command interface was successfully initialized.
 *  \retval false Unsuccessful initialization. The USB command interface is not 
 *                working.
 */
bool cmd_if_init(void);


/*! \brief This function is used to disable the command interface. */
void cmd_if_deinit(void);
//! @}
#endif
/*EOF*/
