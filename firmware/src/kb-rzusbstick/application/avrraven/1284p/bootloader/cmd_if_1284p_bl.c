// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Command interface for the AVR Raven ATmega1284p bootloader
 *
 * \par Application note:
 *     
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: cmd_if_1284p_bl.c 41819 2008-05-16 08:39:03Z ihanssen $
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
#include "wdt_avr.h"

#include "avrraven_1284p_bl.h"
#include "cmd_if_1284p_bl.h"
#include "self_programming.h"
#include "self_programming_conf.h"
#include "sipc.h"
#include "bl.h"
#include "rvn_ota.h"
#include "rvn_loc.h"
#include "macros.h"

/*========================= MACROS                   =========================*/
//! Length of the SIPC RX buffer.
#define CMD_IF_SIPC_BUFFER_LENGTH (255)

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Flag to indicate 
static bool initialized = false;

//! Application program boot vector
static void(* const jmp_app_section)(void) = (void(*)(void))0x0000;

//! Buffer for FW data
static uint8_t page_buffer[SF_PAGE_BYTE_SIZE];

//! Data buffer for use with sipc
static uint8_t sipc_rx_buffer[CMD_IF_SIPC_BUFFER_LENGTH];

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*! \brief  Handler for new SIPC data packets
 *
 *  \param[in]  raw_cmd     Received command
 */
static void cmd_sipc_new_packet_handler(void *raw_cmd);


/*! \brief  Start application program
 *
 *          Interrupts are moved back to app section and a jump to address
 *          0x0000 is executed.
 */
static void app_start(void);


/*! \brief  Reboot
 *
 *          The watchdog timer is used to generate a HW reset.
 */
static void reboot(void);


/*! \brief  Handler for \ref RVN_LOC_CMD_APP_START
 *
 *  \param[in]  cmd     Pointer to entire command
 */
static void rvn_loc_cmd_app_start(uint8_t *cmd);


/*! \brief  Handler for \ref RVN_LOC_CMD_FLASH_PAGE_SIZE_GET
 *
 *  \param[in]  cmd     Pointer to entire command
 */
static void rvn_loc_cmd_page_size_get(uint8_t *cmd);


/*! \brief  Handler for \ref RVN_LOC_CMD_RESTART
 *
 *          The watchdog timer is used to generate a HW reset.
 *
 *  \param[in]  cmd     Pointer to entire command
 */
static void rvn_loc_cmd_restart(uint8_t *cmd);


/*! \brief  Handler for \ref RVN_LOC_CMD_ENTER_BOOT
 *
 *          Enter bootloader (i.e. restart, and enter bootloader. BOOTRST fuse is set)
 *
 *  \param[in]  cmd     Pointer to entire command
 */
static void rvn_loc_cmd_enter_boot(uint8_t *cmd);


/*! \brief  Handler for \ref RVN_LOC_CMD_FW_PACKET
 *
 *          Command contains a FW packet. It will be written to progmem
 *
 *  \param[in]  cmd     Pointer to entire command
 */
static void rvn_loc_cmd_fw_packet(uint8_t *cmd);

/*========================= IMPLEMENTATION           =========================*/


bool cmd_if_init(void) {
    /* Initialize global variables. */
    if (true != sipc_init(CMD_IF_SIPC_BUFFER_LENGTH, sipc_rx_buffer, \
                          cmd_sipc_new_packet_handler)) {
    } else {
        initialized = true;
    }
    
    return initialized;
}


void cmd_if_deinit(void) {
    sipc_deinit();
}


static void cmd_sipc_new_packet_handler(void *raw_cmd) {
    /* Cast void packet to correct type and extract command type. */
    uint8_t cmd = *(uint8_t *)raw_cmd;
    
    switch (cmd) {
    case RVN_LOC_CMD_APP_START:
        rvn_loc_cmd_app_start((uint8_t *)raw_cmd);
        break;
    case RVN_LOC_CMD_FLASH_PAGE_SIZE_GET:
        rvn_loc_cmd_page_size_get((uint8_t *)raw_cmd);
        break;
    case RVN_LOC_CMD_RESTART:
        rvn_loc_cmd_restart((uint8_t *)raw_cmd);
        break;
    case RVN_LOC_CMD_ENTER_BOOT:
        rvn_loc_cmd_enter_boot((uint8_t *)raw_cmd);
        break;
    case RVN_LOC_CMD_FW_PACKET:
        rvn_loc_cmd_fw_packet((uint8_t *)raw_cmd);
        break;
    default:
        /* ACK packet. */
        SIPC_ACK_PACKET();
    
        // Send response
        send_loc_simple_rsp(RVN_LOC_RSP_NOT_SUPPORTED);
        break;
    }
}


static void rvn_loc_cmd_fw_packet(uint8_t *cmd)
{
    rvn_loc_cmd_fw_packet_t* fw_packet = (rvn_loc_cmd_fw_packet_t*)cmd;
    sp_adr_t adr = fw_packet->adr;
    uint16_t size = fw_packet->size;
    sp_adr_t wr_adr = adr - SF_PAGE_BYTE_SIZE;

    // Now its OK to ACK the packet
    SIPC_ACK_PACKET();
   
    // Write page if received data packet is start of next page
    if ((adr%SF_PAGE_BYTE_SIZE == 0) && (adr != 0)) {
        sp_write_flash_page(wr_adr, page_buffer);
    }
    
    // Copy received FW data into page buffer
    memcpy((void*)&page_buffer[adr%SF_PAGE_BYTE_SIZE], (const void*)fw_packet->data, size);
    
    // Send response
    send_loc_simple_rsp(RVN_LOC_RSP_OK);
}


static void rvn_loc_cmd_page_size_get(uint8_t* cmd)
{
    // Now its OK to ACK the packet
    SIPC_ACK_PACKET();
    
    // ### not implemented ###
    send_loc_simple_rsp(RVN_LOC_RSP_NOT_SUPPORTED);
}


static void rvn_loc_cmd_restart(uint8_t *cmd)
{
    // Now its OK to ACK the packet
    SIPC_ACK_PACKET();
    
    // Send response
    send_loc_simple_rsp(RVN_LOC_RSP_OK);
    
    // move interrutp vectors to app section, and start application program
    intvecs_to_app();
    app_start();
}


static void rvn_loc_cmd_app_start(uint8_t *cmd)
{
    // Now its OK to ACK the packet
    SIPC_ACK_PACKET();
    
    // Send response
    send_loc_simple_rsp(RVN_LOC_RSP_OK);
 
    // Move interrutp vectors to app section, and start application program
    uint8_t mcucr = MCUCR & ~(1<<IVSEL) & ~(1<<IVCE);
    MCUCR |= (1<<IVCE);
    MCUCR = mcucr;
    
    app_start();
}


static void rvn_loc_cmd_enter_boot(uint8_t *cmd)
{
    // Now its OK to ACK the packet
    SIPC_ACK_PACKET();
    
    // Send response
    send_loc_simple_rsp(RVN_LOC_RSP_OK);
 
    // restart boot loader
    reboot();
}


static void reboot(void) {
    
    watchdog_timeout_set(WDT_TIMEOUT_2K);
    watchdog_enable();
    while(1){
        ;
    }
}


static void app_start(void)
{
    cli();
    intvecs_to_app();
    jmp_app_section();
    /*program execution continues in application progam*/
}
/*EOF*/
