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
 * $Id: usb_task.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

#ifndef _USB_TASK_H_
#define _USB_TASK_H_

//_____ I N C L U D E S ____________________________________________________
#include <stdint.h>
#include <stdbool.h>
//_____ M A C R O S ________________________________________________________

#define EVT_USB                        0x60               // USB Event
#define EVT_USB_POWERED               (EVT_USB+1)         // USB plugged
#define EVT_USB_UNPOWERED             (EVT_USB+2)         // USB un-plugged
#define EVT_USB_DEVICE_FUNCTION       (EVT_USB+3)         // USB in device
#define EVT_USB_HOST_FUNCTION         (EVT_USB+4)         // USB in host
#define EVT_USB_SUSPEND               (EVT_USB+5)         // USB suspend
#define EVT_USB_WAKE_UP               (EVT_USB+6)         // USB wake up
#define EVT_USB_RESUME                (EVT_USB+7)         // USB resume
#define EVT_USB_RESET                 (EVT_USB+8)         // USB hight speed

#define USB_TRANSACTION_BUFFER_SIZE (288) //!< Size of the transaction buffer in bytes. Must be less than or equal to 0xFF.

typedef enum USB_ENUM_STATUS_TAG {
    USB_SUCCESS = 0,
    USB_EP_HANDLE_ALREADY_OPEN,
    USB_INVALID_PARAMETER,
    USB_NO_DATA,
    USB_HW_TIME_OUT
} usb_status_t;



/*! \brief Transaction done flag type. Can be polled by the user to find out 
 *         when an USB transaction is done.
 */
typedef bool usb_trans_done_t;

//! Callback type that is executed when an USB transaction is finished.
typedef void (* usb_trans_done_callback_t)(uint16_t, uint8_t*);

/*! \brief Available event types for reporting that an USB transaction is done. 
 *
 *  \note Do not use directly. Initialization functions are provided.
 */
typedef enum USB_TRANS_DONE_EVENT_TYPE_ENUM_TAG {
    USB_TRANS_DONE_UNINITIALIZED = 0,
	USB_TRANS_DONE_FLAG_EVENT,
	USB_TRANS_DONE_CALLBACK_EVENT
} usb_trans_done_event_type_t;

//! Timing event control block. Do not use directly. Initialization functions are provided.
typedef struct USB_TRANS_DESCRIPTOR_STRUCT {
	uint8_t ep; //!< Number of endpoint to be opened.
	usb_trans_done_callback_t done_callback;  //!< Callback function that will be executed when an USB transaction is done.
} usb_trans_descriptor_t;
//_____ D E C L A R A T I O N S ____________________________________________

extern bool ep_error; //!< Flag indicating if an error has occured during an USB transaction. Do not use directly, use access functions instead.

#define USB_EP_TRANS_GET_ERROR_FLAG() (ep_error)

void usb_task_init(uint64_t serial_nmbr);
void usb_task_deinit   (void);
void usb_start_device  (void);
void usb_start_host    (void);
void usb_task          (void);

uint8_t usb_ep_open(usb_trans_descriptor_t *trans_desc);
void usb_ep_close(void);
void usb_ep_ack_transaction(void);
uint8_t usb_ep_write(uint8_t ep, uint8_t length, uint8_t *packet);

uint8_t usb_ep_write_block(uint8_t ep, uint8_t length, uint8_t *packet);
#endif /* _USB_TASK_H_ */
