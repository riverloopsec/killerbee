// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven mail box
 *
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
 * \par Documentation
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: mbox.h 41666 2008-05-14 22:55:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __MBOX_H__
#define __MBOX_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "sfs.h"

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/
#define MBOX_GET_NEXT           SFS_FGET_FWD
#define MBOX_GET_PREV           SFS_FGET_REV
#define MBOX_GET_RESTART        SFS_FGET_RESTART

#define MBOX_NEW_MAIL_SOUND_FILE_NAME               "NEW_MAIL.PCM"
#define MBOX_NEW_MAIL_SOUND_FILE_NAME_SIZE          (sizeof(MBOX_NEW_MAIL_SOUND_FILE_NAME))
#define MBOX_NEW_MAIL_SOUND_FILE_NAME_BUFFER_SIZE   (MBOX_NEW_MAIL_SOUND_FILE_NAME_SIZE+1)
/*========================= TYPEDEFS                 =========================*/
typedef struct mbox_mail_tag{
    uint16_t address;
    uint8_t size;
    uint8_t data[];
} mbox_mail_t;

typedef struct mbox_mail_element_tag{
    bool opened;
    int16_t number;
    mbox_mail_t mail;
} mbox_mail_element_t;
/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/
/*! \brief  Initialize the mailbox
 *
 *  \returns    EOF on fail
 */
int          mbox_init(void);


/*! \brief  De-initialize the mail box
 *
 *  \returns    EOF on fail
 */
void         mbox_deinit(void);


/*! \brief  Open mailbox
 *
 *  \returns    EOF on fail
 */
int          mbox_open(void);


/*! \brief  Close mailbox
 *
 *  \returns    EOF on fail
 */
void         mbox_close(void);


/*! \brief  Check if mailbox is empty
 *
 *  \returns    true if mailbox empty
 */
bool         mbox_empty(void);


/*! \brief  Check for unread mail
 *
 *  \returns    true if any unread mail
 */
bool         mbox_unread(void);


/*! \brief  Put a new mail in the mailbox
 *
 *  \param[in]  address Address of sender
 *  \param[in]  data    Pointer to mail text
 *  \param[in]  size    Size of mail text
 *
 *  \returns    EOF on fail
 */
int          mbox_mail_put(uint16_t address, uint8_t* data, uint8_t size);


/*! \brief  Get next/previous mail in the mailbox
 *
 *  \param[in]  mail        Address of pointer to mail buffer
 *  \param[in]  direction   Search direction
 *
 *  \returns    EOF on fail
 */
int8_t mbox_mail_get(mbox_mail_t** mail, sfs_fget_t direction);


/*! \brief  Get number of mails on data flash
 *
 *  \returns    Number of mails found
 */
int8_t       mbox_mail_count_get(void);


/*! \brief  Get pointer to a new mail buffer
 *
 *  \param[in]  size    Required size of buffer
 *
 *  \returns    Pointer to buffer, NULL on error
 */
uint8_t*     mbox_mail_buffer_get(int8_t size);


/*! \brief  Return buffer retrieved by \ref mbox_mail_buffer_get().
 *
 *  \param[in]  buffer Pointer to buffer to free
 *
 *  \returns    EOF on fail
 */
int8_t       mbox_mail_buffer_put(uint8_t* buffer);


/*! \brief  Send a new mail
 *
 *  \param[in]  recipient_adr   Recipient address
 *  \param[in]  txtmsg_size     Size of text message (data is allready present in internal buffer)
 *
 *  \returns    EOF on fail
 */
uint8_t      mbox_mail_send(uint16_t recipient_adr, int8_t txtmsg_size);
//! @}
#endif // __MBOX_H__
/*EOF*/
