// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Implementation of a simple mailbox for the AVR Raven
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
 * $Id: mbox.c 41666 2008-05-14 22:55:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "mbox.h"

#include "str.h"
#include "ota.h"

#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "rvn_loc.h"
#include "rvn_ota.h"
#include "sipc.h"
#include "vrt_mem.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

//! @{ Mail box file name and buffer definitions
#define MBOX_MAIL_NAME_NUMBER_FILE_NAME             "MBOX.INF"
#define MBOX_MAIL_NAME_NUMBER_FILE_NAME_SIZE        (sizeof(MBOX_MAIL_NAME_NUMBER_FILE_NAME))
#define MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE (MBOX_MAIL_NAME_NUMBER_FILE_NAME_SIZE + 1)

#define MBOX_MAIL_NAME_NUMBER_OFFSET                (2)
#define MBOX_MAIL_NAME_NUMBER_SIZE                  (4)
#define MBOX_MAIL_NAME_BUFFER_SIZE                  MBOX_MAIL_NAME_TEMPLATE_BUFFER_SIZE

#define MBOX_MAIL_NAME_TEMPLATE                     "MB0000.MBX"
#define MBOX_MAIL_NAME_TEMPLATE_SIZE                (sizeof(MBOX_MAIL_NAME_TEMPLATE))
#define MBOX_MAIL_NAME_TEMPLATE_BUFFER_SIZE         (MBOX_MAIL_NAME_TEMPLATE_SIZE + 1)

#define MBOX_MAIL_TYPE                              "*.MBX"
#define MBOX_MAIL_TYPE_SIZE                         (sizeof(MBOX_MAIL_TYPE))
#define MBOX_MAIL_TYPE_BUFFER_SIZE                  (MBOX_MAIL_TYPE_SIZE + 1)

#define MBOX_MAIL_NAME_NUMBER_HEX_SIZE              (sizeof(mbox_mail_name_number_t))
//!@}

//! Sequence number used when sending mail
#define MBOX_SEQ_NMBR                               (0x01)

/*========================= TYPEDEFS                 =========================*/
typedef int16_t mbox_mail_name_number_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Pointer to current mail buffer (pointer returned by \ref mbox_mail_buffer_get())
static uint8_t* mbox_mail_buffer;

//! Pointer to mail buffer (returned by \ref mbox_open())
static mbox_mail_element_t* mbox_mail_element;

//! Flag indication mail box module initialized
static bool mbox_initalized = false;

//! Flag indication mail box open
static bool mbox_is_open;

/// Pointer to OTA command "text message"
static rvn_ota_cmd_str_t* mbox_ota_cmd_txtmsg;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/
/*! \brief  Get current mail name number from file on data flash
 *
 *  \returns    EOF on fail
 */
static mbox_mail_name_number_t mbox_mail_numb_get(void);


/*! \brief  Update file name number in file
 *
 *  \param[in]  mail_name_number    New number for mail name generation
 *
 *  \returns    EOF on fail
 */
static int16_t mbox_mail_numb_put(mbox_mail_name_number_t mail_name_number);


/*! \brief  Create a new mail name number file
 *
 *  \returns    EOF on fail
 */
static int16_t mbox_mail_numb_create(void);


/*! \brief  Get next mail from dataflash
 *
 *  \param[in]  mail_element    Address of a pointer to mail element
 *  \param[in]  direction       Search direction
 *  \param[in]  for_reading     Set to "true" if open for reading
 *
 *  \returns    EOF on fail
 */
static int8_t  mbox_mail_element_get(mbox_mail_element_t** mail_element, sfs_fget_t direction, bool for_reading);


/*! \brief  Store a new mail on dataflash
 *
 *  \param[in]  mail_element    Pointer to mail element
 *
 *  \returns    EOF on fail
 */
static int8_t  mbox_mail_element_put(mbox_mail_element_t* mail_element);

/*========================= IMPLEMENTATION           =========================*/
int mbox_init(void)
{
    // Init error flag
    bool init_error = false;
    
    // If allready initialized, deinitialize first
    if (mbox_initalized == true) {
        mbox_deinit();
    }
    
    // Init mail box buffer pointer
    mbox_mail_buffer = NULL;
    
    // mbox_mail_numb_get() and mbox_mail_numb_create() needs interrupts enabled
    ENTER_CRITICAL_REGION();
    sei();
    
    // If "name number file" does not exist, create it
    if (mbox_mail_numb_get() == EOF) {
        if (mbox_mail_numb_create() == EOF) {
            init_error = true;
        }
    }
    
    // Restore interrupt state
    LEAVE_CRITICAL_REGION();
    
    // Indicate successful initialization
    mbox_initalized = init_error ? false : true;
    
    // Return EOF on error
    return init_error ? EOF : 0;
}

void mbox_deinit(void)
{
    // Remove any envelope symbols
    lcd_symbol_clr(LCD_SYMBOL_ENV_MAIN);
    lcd_symbol_clr(LCD_SYMBOL_ENV_CL);
    lcd_symbol_clr(LCD_SYMBOL_ENV_OP);
    
    // Close mailbox and indicate no longer initialized
    mbox_close();
    mbox_initalized = false;
}

int mbox_open(void)
{
    // Return if not initialized
    if (mbox_initalized == false) {
        return EOF;
    }
    
    // If allready open, close first
    if (mbox_is_open == true) {
        mbox_close();
    }
    
    // Start file rading at start of data flash
    sfs_fget_restart();
    
    // allocate memory for mail reading
    if ((mbox_mail_element = (mbox_mail_element_t*)vrt_mem_alloc(/*sizeof(mbox_mail_element_t)*/ + 64)) == NULL) {
        return EOF;
    } else {
        mbox_is_open = true;
        return 0;
    }
}

void mbox_close(void)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return;
    }
    
    // Indicate mailbox closed and free memory allocated by mbox_open()
    mbox_is_open = false;
    vrt_mem_free((void*)(mbox_mail_element));
}

uint8_t* mbox_mail_buffer_get(int8_t size)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return (uint8_t*)NULL;
    }
    
    // If buffer allready allocated, just free it
    if (mbox_ota_cmd_txtmsg == NULL) {
        mbox_mail_buffer_put((uint8_t*)mbox_ota_cmd_txtmsg);
    }
    
    // Allocate a memory block for entire frame, return NULL if allocation fails
    if ((mbox_ota_cmd_txtmsg = vrt_mem_alloc(sizeof(rvn_ota_cmd_str_t) + size)) == NULL) {
        return (uint8_t*)NULL;
    }
    
    // Store text message data buffer and return to calling function
    mbox_mail_buffer = mbox_ota_cmd_txtmsg->data;
    
    return mbox_mail_buffer;
}

int8_t mbox_mail_buffer_put(uint8_t* buffer)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return EOF;
    }
    
    // Return if buffer pointer invalid
    if (buffer != (uint8_t*)mbox_ota_cmd_txtmsg) {
        return EOF;
    }
    
    // Free memory block
    vrt_mem_free((void*)mbox_ota_cmd_txtmsg);
    
    // Set buffer pointer to NULL to indicate not valid
    mbox_ota_cmd_txtmsg = NULL;
    
    return 0;
}


uint8_t mbox_mail_send(uint16_t recipient_adr, int8_t txtmsg_size)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return 0;
    }
    
    // Bulid text message application command
    mbox_ota_cmd_txtmsg->id = RVN_OTA_CMD_TXTMSG;
    mbox_ota_cmd_txtmsg->size = txtmsg_size;
    
    // Send frame
    int ota_packet_size = sizeof(rvn_ota_cmd_str_t) + txtmsg_size;
    ota_transaction((void*)mbox_ota_cmd_txtmsg, recipient_adr, MBOX_SEQ_NMBR, ota_packet_size);
    
    // Free buffer
    mbox_mail_buffer_put((unsigned char*)mbox_ota_cmd_txtmsg);

    return MBOX_SEQ_NMBR;
}

bool mbox_empty(void)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return true;
    }
    
    // Search file system for mails, return "true" if none found
    unsigned char file_types[MBOX_MAIL_TYPE_BUFFER_SIZE];
    strncpy_P((char*)file_types, MBOX_MAIL_TYPE, MBOX_MAIL_TYPE_BUFFER_SIZE);
    if (sfs_fcount(file_types) == 0) {
        return true;
    } else {
        return false;
    }
}

bool mbox_unread(void)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return false;
    }
    
    // Get first mail element, return if failed
    mbox_mail_element_get(&mbox_mail_element, MBOX_GET_RESTART, false);
    if (mbox_mail_element_get(&mbox_mail_element, MBOX_GET_NEXT, false) == EOF) {
        return false;
    }
    mbox_mail_name_number_t first_mail_number = mbox_mail_element->number;
    
    // Search through all mail elements, stop at first un-read mail
    do {
        mbox_mail_element_get(&mbox_mail_element, MBOX_GET_NEXT, false);
        if (mbox_mail_element->opened == false) {
            return true;
        }
    } while(mbox_mail_element->number != first_mail_number);

    return false;
}

int8_t mbox_mail_count_get(void)
{
    uint8_t file_types[MBOX_MAIL_TYPE_BUFFER_SIZE];
    int8_t count;
    
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return 0;
    }
    
    // Count files of type "MBOX_MAIL_TYPE"
    strncpy_P((char*)file_types, MBOX_MAIL_TYPE, MBOX_MAIL_TYPE_BUFFER_SIZE);
    count = sfs_fcount(file_types);
    
    return count;
}

int mbox_mail_put(uint16_t address, uint8_t* data, uint8_t size)
{
    mbox_mail_name_number_t file_name_number;
    
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return EOF;
    }
    
    // get current mail name number
    if ((file_name_number = mbox_mail_numb_get()) == EOF){
        return EOF;
    }
    
    // Store mail as a new mail element
    mbox_mail_element->number = file_name_number;
    mbox_mail_element->opened = false;
    mbox_mail_element->mail.address = address;
    mbox_mail_element->mail.size = size;
    memcpy((void*)mbox_mail_element->mail.data, (const void*)data, (size_t)size);
    if (mbox_mail_element_put(mbox_mail_element) == EOF) {
        return EOF;
    }
    
    // Update mail name number
    if (file_name_number++ == -1) {
        file_name_number = 0;
    }
    mbox_mail_numb_put(file_name_number);
    
    return 0;
}

static int8_t mbox_mail_element_put(mbox_mail_element_t* mail_element)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return EOF;
    }
    
    // Generate filename based on running-number
    unsigned char file_name[MBOX_MAIL_NAME_BUFFER_SIZE];
    strncpy_P((char*)file_name, MBOX_MAIL_NAME_TEMPLATE, MBOX_MAIL_NAME_TEMPLATE_BUFFER_SIZE);
    insi(file_name, (uint16_t)mail_element->number, MBOX_MAIL_NAME_NUMBER_OFFSET, MBOX_MAIL_NAME_NUMBER_SIZE);
    
    // Write data to file
    sfs_fstream_t* fstream;
    sfs_fsize_t size = sizeof(mbox_mail_element_t) + mail_element->mail.size;
    if ((fstream = sfs_fopen(file_name, SFS_OPEN_MODE_WR|SFS_OPEN_MODE_CREATE)) == NULL) {
        return EOF;
    } else {
        sfs_fwrite((const void*)(mail_element), size, fstream);
        sfs_fclose(fstream);
    }
    return 0;
}

int8_t mbox_mail_get(mbox_mail_t** mail, sfs_fget_t direction)
{
    // Return if mailbox not open
    if (mbox_is_open == false) {
        return EOF;
    }
    
    // Get next/prev mail element, return EOF on fail
    if (mbox_mail_element_get(&mbox_mail_element, direction, true) == EOF) {
        return EOF;
    }
    
    // copy pointer to new mail and return OK
    *mail = &mbox_mail_element->mail;
    return 0;    
}

static int8_t mbox_mail_element_get(mbox_mail_element_t** mail_element, sfs_fget_t direction, bool for_reading)
{
    uint8_t opened = true;
    unsigned char file_types[MBOX_MAIL_TYPE_BUFFER_SIZE];
    unsigned char file_name[MBOX_MAIL_NAME_BUFFER_SIZE];
    sfs_fstream_t* mail_stream;
    
    // Return if no mail
    strncpy_P((char*)file_types, MBOX_MAIL_TYPE, MBOX_MAIL_NAME_BUFFER_SIZE);
    if (sfs_fget(file_types, (unsigned char*)file_name, direction) == EOF) {
        return EOF;
    }
    
    // Return if "MBOX_GET_RESTART"
    if (direction == MBOX_GET_RESTART) {
        return EOF;
    }
    
    // Return if unable to open file
    if ((mail_stream = sfs_fopen(file_name, SFS_OPEN_MODE_RD|SFS_OPEN_MODE_BINARY)) == NULL) {
        return EOF;
    }
    
    // Else read file into struct pointed to by argument "mail_element"
    // read mail header
    sfs_fread((void*)(*mail_element), sizeof(mbox_mail_element_t), mail_stream);
    
    // Read mail data
    sfs_fsize_t size = (*mail_element)->mail.size;
    sfs_fread((void*)(*mail_element)->mail.data, size, mail_stream);
    
    // If not opened before, update "opened" flag of file data to indicate mail opened 
    if (((*mail_element)->opened == false) && (for_reading == true)) {
        sfs_fwrite((const void*)&opened, 1, mail_stream);
    }
    
    // Close mail file stream
    sfs_fclose(mail_stream);
    
    // Return OK
    return 0;
}

static int16_t mbox_mail_numb_create(void)
{
    uint8_t file_name[MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE];
    const mbox_mail_name_number_t mail_name_number = 0;
    sfs_fstream_t* fstream;
    
    // Open/create mail name number file.
    strncpy_P((char*)file_name, MBOX_MAIL_NAME_NUMBER_FILE_NAME, MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE);
    if ((fstream = sfs_fopen(file_name, SFS_OPEN_MODE_RDWR|SFS_OPEN_MODE_CREATE)) == NULL) {
        return EOF;
    }
    // Write init mail name number (0000)
    int write_count = sfs_fwrite((const void*)&mail_name_number, MBOX_MAIL_NAME_NUMBER_HEX_SIZE, fstream);
    sfs_fclose(fstream);
    
    // Return EOF if write operation failed
    if (write_count != MBOX_MAIL_NAME_NUMBER_HEX_SIZE) {
        return EOF;
    } else {
        return 0;
    }
}

static mbox_mail_name_number_t mbox_mail_numb_get(void)
{
    uint8_t file_name[MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE];
    mbox_mail_name_number_t mail_name_number;
    sfs_fstream_t* fstream;
    
    // Open mail name number file.
    strncpy_P((char*)file_name, MBOX_MAIL_NAME_NUMBER_FILE_NAME, MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE);
    if ((fstream = sfs_fopen(file_name, SFS_OPEN_MODE_RD|SFS_OPEN_MODE_BINARY)) == NULL) {
        return EOF;
    }
    
    // read mail name number
    sfs_fread((void*)&mail_name_number, MBOX_MAIL_NAME_NUMBER_HEX_SIZE, fstream);
    sfs_fclose(fstream);
    
    return mail_name_number;
}

static int16_t mbox_mail_numb_put(mbox_mail_name_number_t mail_name_number)
{
    unsigned char file_name[MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE];
    sfs_fstream_t* fstream;
    
    // Open mail name number file.
    strncpy_P((char*)file_name, MBOX_MAIL_NAME_NUMBER_FILE_NAME, MBOX_MAIL_NAME_NUMBER_FILE_NAME_BUFFER_SIZE);
    if ((fstream = sfs_fopen(file_name, SFS_OPEN_MODE_RDWR|SFS_OPEN_MODE_BINARY)) == NULL) {
        return EOF;
    }
    
    // Write new mail name number
    sfs_fwrite((const void*)&mail_name_number, MBOX_MAIL_NAME_NUMBER_HEX_SIZE, fstream);
    sfs_fclose(fstream);
    
    return 0;
}
//! @}
/*EOF*/
