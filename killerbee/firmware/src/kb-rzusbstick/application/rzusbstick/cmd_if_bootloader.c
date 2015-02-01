// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the command interface that the PC uses to control
 *         the RZUSBSTICK's bootloader
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
 * $Id: cmd_if_bootloader.c 41438 2008-05-09 00:08:19Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "vrt_kernel.h"
#include "wdt_avr.h"

#include "led.h"
#include "cmd_if_bootloader.h"
#include "rzusbstickcommon.hh"
#include "self_programming.h"

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_task.h"
#include "eep.h"

//! \addtogroup grRzUsbStickBootloader
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static uint64_t ieee_address = 0x12345678;
static usb_trans_descriptor_t desc;
/*================================= PROTOTYPES       =========================*/
/*! \brief This function is executed each time an USB transaction is completed.
 *
 *  When an USB transaction is completed, a new command is received. There is
 *  a command handler defined in the vrt_kernel for each event that implements
 *  the command specific behavior.
 *
 *  \param[in] length Length of received data.
 *  \param[in] data Pointer to the received data from last USB transaction.
 */
static void usb_transaction_done(uint16_t length, uint8_t *data);

/*! \brief Reeboot the AVR. */
static void reboot(void);

/*! \brief Handler for the CMD_SIGN_OFF command.
 *
 *  \param[in] cmd_sign_off Pointer to raw command received on the USB interface.
 */
static void cmd_if_sign_off(void *cmd_sign_off);

/*! \brief Handler for the CMD_SIGN_ON command.
 *
 *  \param[in] cmd_sign_on Pointer to raw command received on the USB interface.
 */
static void cmd_if_sign_on(void *cmd_sign_on);

/*! \brief Handler for the CMD_BOOT_READ_SIGNATURE command.
 *
 *  \param[in] cmd_read_signature Pointer to raw command received on the USB interface.
 */
static void cmd_if_read_signature(void *cmd_read_signature);

/*! \brief Handler for the CMD_BOOT_GET_VERSION command.
 *
 *  \param[in] cmd_get_version Pointer to raw command received on the USB interface.
 */
static void cmd_if_get_version(void *cmd_get_version);

/*! \brief Handler for the CMD_BOOT_START_APPLICATION command.
 *
 *  \param[in] cmd_start_application Pointer to raw command received on the USB 
 *                                   interface.
 */
static void cmd_if_start_application(void *cmd_start_application);

/*! \brief Handler for the CMD_BOOT_ERASE_FLASH command.
 *
 *  \param[in] cmd_erase_flash Pointer to raw command received on the USB interface.
 */
static void cmd_if_erase_flash(void *cmd_erase_flash);

/*! \brief Handler for the CMD_BOOT_ERASE_EEPROM command.
 *
 *  \param[in] cmd_erase_eeprom Pointer to raw command received on the USB interface.
 */
static void cmd_if_erase_eeprom(void *cmd_erase_eeprom);

/*! \brief Handler for the CMD_BOOT_PROG_FLASH command.
 *
 *  \param[in] cmd_program_flash Pointer to raw command received on the USB interface.
 */
static void cmd_if_program_flash(void *cmd_program_flash);

/*! \brief Handler for the CMD_BOOT_PROG_EEPROM command.
 *
 *  \param[in] cmd_program_eeprom Pointer to raw command received on the USB interface.
 */
static void cmd_if_program_eeprom(void *cmd_program_eeprom);

/*! \brief Handler for the CMD_BOOT_READ_FLASH command.
 *
 *  \param[in] cmd_read_flash Pointer to raw command received on the USB interface.
 */
static void cmd_if_read_flash(void *cmd_read_flash);

/*! \brief Handler for the CMD_BOOT_READ_EEPROM command.
 *
 *  \param[in] cmd_read_eeprom Pointer to raw command received on the USB interface.
 */
static void cmd_if_read_eeprom(void *cmd_read_eeprom);

/*! \brief Handler for the CMD_BOOT_READ_LOCKBITS command.
 *
 *  \param[in] cmd_read_lockbits Pointer to raw command received on the USB interface.
 */
static void cmd_if_read_lockbits(void *cmd_read_lockbits);

/*! \brief Handler for the CMD_BOOT_WRITE_LOCKBITS command.
 *
 *  \param[in] cmd_write_lockbits Pointer to raw command received on the USB interface.
 */
static void cmd_if_write_lockbits(void *cmd_write_lockbits);

/*! \brief Handler for the CMD_BOOT_READ_FUSES command.
 *
 *  \param[in] cmd_read_fuses Pointer to raw command received on the USB interface.
 */
static void cmd_if_read_fuses(void *cmd_read_fuses);


bool cmd_if_init(void) {
    /* Initialize local variables. */
    ieee_address = eep_read_ieee_address();
    
    /* Initialize USB devic driver. */
    usb_task_init(ieee_address);
    
    /* Set up the transaction descriptor for the OUT end point where commands will
     * be received.
     */
    desc.ep = EP_OUT;
    desc.done_callback = usb_transaction_done;
    
    bool init_status = false;
    if (USB_SUCCESS != usb_ep_open(&desc)) {
    } else {
        init_status = true;
    } // END: if (USB_SUCCESS != usb_ep_open(&desc)) ...
    
    return init_status;
}


void cmd_if_deinit(void) {
    /* Close EP and turn the USB macro off. */
    usb_ep_close();
    usb_task_deinit();
}

static void reboot(void) {
    LED_GREEN_OFF();
    LED_ORANGE_OFF();

    wdt_enable(WDTO_15MS);
    
    while (true) {
        ;
    }
}

/*                          COMMAND HANDLERS                                  */



static void cmd_if_sign_off(void *cmd_sign_off) {
    /* Send response to the PC. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    Usb_write_byte(RESP_SUCCESS);
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
    
    /* Reset the AVR. */
    reboot();
}


static void cmd_if_sign_on(void *cmd_sign_on) {
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SIGN_ON);
    Usb_write_byte(21);
	Usb_write_byte('R');
	Usb_write_byte('Z');
	Usb_write_byte('U');
	Usb_write_byte('S');
	Usb_write_byte('B');
	Usb_write_byte('S');
    Usb_write_byte('T');
    Usb_write_byte('I');
    Usb_write_byte('C');
    Usb_write_byte('K');
    Usb_write_byte(' ');
    Usb_write_byte('B');
    Usb_write_byte('O');
    Usb_write_byte('O');
    Usb_write_byte('T');
    Usb_write_byte('L');
    Usb_write_byte('O');
    Usb_write_byte('A');
    Usb_write_byte('D');
    Usb_write_byte('E');
    Usb_write_byte('R');
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_read_signature(void *cmd_read_signature) {
    /* Read signature bytes. */
    uint8_t signature_low = sp_read_signature_low();
    uint8_t signature_mid = sp_read_signature_mid();
    uint8_t signature_high = sp_read_signature_high();

    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_BOOT_PARAM);
    Usb_write_byte(signature_low);
    Usb_write_byte(signature_mid);
    Usb_write_byte(signature_high);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}

static void cmd_if_get_version(void *cmd_get_version) {
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_BOOT_PARAM);
    Usb_write_byte(BOOT_VER_MAJ);
    Usb_write_byte(BOOT_VER_MIN);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_start_application(void *cmd_start_application) {
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SUCCESS);
    
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
    
    /* Delay so that the response is sent before application section is entered. */
    delay_us(50000);
    
    /* Set the interrupt vectors to the application area and start the application. */
    ENTER_CRITICAL_REGION();
    uint8_t mcucr = MCUCR & ~(1<<IVSEL) & ~(1<<IVCE);
    MCUCR |= (1<<IVCE);
    MCUCR = mcucr;
	LEAVE_CRITICAL_REGION();
    
    EEPUT(EE_BOOT_MAGIC_ADR, 0xFF);
    reboot();
}


static void cmd_if_erase_flash(void *cmd_erase_flash) {
    sp_chip_erase();
    
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SUCCESS);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_erase_eeprom(void *cmd_erase_eeprom) {
    sp_erase_eeprom();
    
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SUCCESS);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_program_flash(void *cmd_program_flash) {
    /* Decode the program flash command. */
    cmd_boot_program_flash_t *cmd = (cmd_boot_program_flash_t *)cmd_program_flash;
    
    if (true != sp_write_flash_page(cmd->address, cmd->flash_page)) {
        /* Send response. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_SYNTACTICAL_ERROR);
        
        Usb_send_in();
    } else {
        LED_GREEN_TOGGLE();
        
        /* Send response. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
    
        Usb_write_byte(RESP_SUCCESS);
        
        Usb_send_in();
    }
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_program_eeprom(void *cmd_program_eeprom) {
    /* Decode the program eeprom command. */
    cmd_boot_program_eeprom_t *cmd = (cmd_boot_program_eeprom_t *)cmd_program_eeprom;
    
    /* Write Eeprom cell. */
    sp_write_eeprom(cmd->address, cmd->value);
    
    /* Send response. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SUCCESS);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_read_flash(void *cmd_read_flash) {
    cmd_boot_read_flash_t *cmd = (cmd_boot_read_flash_t *)cmd_read_flash;
    
    uint8_t flash_value = sp_read_flash(cmd->address);
    
    /* Respond to the CMD_READ_FLASH. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_BOOT_PARAM);
    Usb_write_byte(flash_value);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_read_eeprom(void *cmd_read_eeprom) {
    cmd_boot_read_eeprom_t *cmd = (cmd_boot_read_eeprom_t *)cmd_read_eeprom;
    
    uint8_t eeprom_value = sp_read_eeprom(cmd->address);
    
    /* Respond to the CMD_READ_EEPROM. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_BOOT_PARAM);
    Usb_write_byte(eeprom_value);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_read_lockbits(void *cmd_read_lockbits) {
    uint8_t lock_bits = sp_read_lockbits();
    
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_BOOT_PARAM);
    Usb_write_byte(lock_bits);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_write_lockbits(void *cmd_write_lockbits) {
    /* Decode the write lock bits command and write. */
    cmd_boot_write_lockbits_t *cmd = (cmd_boot_write_lockbits_t *)cmd_write_lockbits;
    sp_write_lockbits(cmd->lockbits);
    
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_SUCCESS);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}


static void cmd_if_read_fuses(void *cmd_read_fuses) {
    /* Read fuses. */
    uint8_t low_fuses = sp_read_fuse_low();
    uint8_t high_fuses = sp_read_fuse_high();
    uint8_t extended_fuses = sp_read_fuse_extended();
    
    /* Respond to the CMD_SIGN_ON. */
    Usb_select_endpoint(EP_IN);
    Usb_send_control_in();
    
    Usb_write_byte(RESP_BOOT_PARAM);
    Usb_write_byte(low_fuses);
    Usb_write_byte(high_fuses);
    Usb_write_byte(extended_fuses);
        
    Usb_send_in();
    
    /* Release the transaction buffer. */
    usb_ep_ack_transaction();
}
/*================================= CALLBACKS ================================*/
                           /* USB callbacks */
static void usb_transaction_done(uint16_t length, uint8_t *data) {
    /* Check that the minimum command length of one byte is received. */
    if (0 == length) { usb_ep_ack_transaction(); return; }
    
    /* First byte in all defined commands equals the command type. This byte is
     * used to execute the associated command handler through the event system.
     */
    uint8_t event = *data;
    vrt_event_handler_t event_handler = NULL;
    
    /* Verify that this is a valid command. */
    if (CMD_SIGN_ON == event) {
        event_handler = cmd_if_sign_on;
    } else if (CMD_SIGN_OFF == event) {
        event_handler = cmd_if_sign_off;
    } else if (CMD_BOOT_START_APPLICATION == event) {
        event_handler = cmd_if_start_application;
    } else if (CMD_BOOT_READ_SIGNATURE == event) {
        event_handler = cmd_if_read_signature;
    } else if (CMD_BOOT_GET_VERSION == event) {
        event_handler = cmd_if_get_version;
    } else if (CMD_BOOT_ERASE_FLASH == event) {
        event_handler = cmd_if_erase_flash;
    } else if (CMD_BOOT_ERASE_EEPROM == event) {
        event_handler = cmd_if_erase_eeprom;
    } else if (CMD_BOOT_PROG_FLASH == event) {
        event_handler = cmd_if_program_flash;
    } else if (CMD_BOOT_PROG_EEPROM == event) {
        event_handler = cmd_if_program_eeprom;
    } else if (CMD_BOOT_READ_LOCKBITS == event) {
        event_handler = cmd_if_read_lockbits;
    } else if (CMD_BOOT_WRITE_LOCKBITS == event) {
        event_handler = cmd_if_write_lockbits;
    }  else if (CMD_BOOT_READ_FUSES == event) {
        event_handler = cmd_if_read_fuses;
    }  else if (CMD_BOOT_READ_FLASH == event) {
        event_handler = cmd_if_read_flash;
    } else if (CMD_BOOT_READ_EEPROM == event) {
        event_handler = cmd_if_read_eeprom;
    } else {
        /* Unsupported command. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_COMMAND_UNKNOWN);
        Usb_send_in();
        
        usb_ep_ack_transaction();
        return;
    }
    
    if (false == vrt_post_event(event_handler, (void *)data)) {
        /* Command could not be posted into the event system, notify the user about this. */
        Usb_select_endpoint(EP_IN);
        Usb_send_control_in();
        Usb_write_byte(RESP_VRT_KERNEL_ERROR);
        Usb_send_in();
        
        usb_ep_ack_transaction();
    } // END: if (false == vrt_post_event(event, data)) ...
}
//! @}
/*EOF*/
