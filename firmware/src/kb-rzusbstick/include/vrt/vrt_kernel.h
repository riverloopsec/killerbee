// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the API for the VRT kernel.
 *
 * \defgroup grVRTKernel The VRT Kernel
 * \ingroup grVRT
 *
 * These functions are a part of the VRT kernel. The VRT kernel is a co-operative 
 * multitasking kernel. The kernel provides primitives to post and dispatch events.
 * Events are used to signal that an event handler must be executed. 
 * An attribute can be associated with an event. This attribute is used to pass event 
 * context (parameters, data etc.) to the executed event handler. Events are stored
 * in a queue and dispatched as a FIFO.
 *
 * References:
 *   - http://en.wikipedia.org/wiki/Computer_multitasking#Cooperative_multitasking.2Ftime-sharing
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
 * $Id: vrt_kernel.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef VRT_KERNEL_H
#define VRT_KERNEL_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "vrt_kernel_conf.h"

//! \addtogroup grVRTKernel
//! @{
/*================================= MACROS           =========================*/
/**************************************************************************
 * Frequenctly used functions implemented as macros for speed optimization.
 **************************************************************************/

/*! \brief  Macro for inserting events from within interrupt handlers.
 *
 *  Since this macro does not check whether the FIFO has room for data or not,
 *  the caller must enure this by checking first, or by design.
 *
 *  This macro modifies the FIFO state without disabling interrupts. This means
 *  that this macro should only be called from within an interrupt handler.
 *  If called from application code, make sure no other interrupts would access
 *  the FIFO at the same time, otherwise the FIFO could be corrupted.
 */
#define vrt_quick_post_event(handler_func, event_attributes)  do{     \
	                                                                  \
    vrt_event_fifo[vrt_event_fifo_head].handler = handler_func;       \
    vrt_event_fifo[vrt_event_fifo_head].attribute = event_attributes; \
	if (vrt_event_fifo_head == (VRT_EVENT_QUEUE_SIZE - 1)) {          \
		vrt_event_fifo_head = 0;                                      \
	} else {                                                          \
		vrt_event_fifo_head++;                                        \
	}                                                                 \
	                                                                  \
	vrt_event_fifo_items_free--;                                      \
	vrt_event_fifo_items_used++;                                      \
} while (0)

/*! \brief This is the access function for vrt_event_fifo_items_used.
 */
#define VRT_GET_ITEMS_USED() (vrt_event_fifo_items_used)

/*! \brief This is the access function for vrt_event_fifo_items_free.
 */
#define VRT_GET_ITEMS_FREE() (vrt_event_fifo_items_free)

/*! \brief This should be called when an event cannot be handled.
 */
#define VRT_EVENT_MISSED() (vrt_kernel_events_missed++)
/*================================= TYEPDEFS         =========================*/
typedef void (*vrt_event_handler_t)(void*); //!< Event handler type.

typedef struct EVENT_STRUCT_TAG {
    vrt_event_handler_t handler; //!< Function pointer to event handler.
    void *attribute; //!< Pointer to the storage where the event's attributes can be found. NULL if none available.
} vrt_event_t;
/*================================= GLOBAL VARIABLES =========================*/
/* These variables are not available for the user to access directly.
 * They are kept here to have a speedy way to post events in ISRs.
 */
extern vrt_event_queue_size_t vrt_event_fifo_head;
extern vrt_event_queue_size_t vrt_event_fifo_items_used;
extern vrt_event_queue_size_t vrt_event_fifo_items_free;
extern vrt_event_t vrt_event_fifo[VRT_EVENT_QUEUE_SIZE];

extern uint8_t vrt_kernel_events_missed;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function will set-up the VRT kernel event queue and dispatcher.
 *  \note This function must be called before any other function in the VRT kernel
 *        is called.
 *
 *  \retval true VRT kernel successfully initialized.
 *  \retval false Will never happen.
 */
bool vrt_init(void);

/*! \brief This function will flush the event queue. */
void vrt_flush_event_queue(void);

/*! \brief This function will post a new entry in the event queue.
 *
 *  \param handler Pointer to function that will handle this event.
 *  \param attribute Pointer to any event context (Parameters, data etc.).
 *
 *  \retval true Event added to the event queue.
 *  \retval false Event queue full.
 */
bool vrt_post_event(vrt_event_handler_t handler, void* attribute);

/*! \brief This function should be called as frequently as possible by the 
 *         application in order to provide a permanent execution of the event handlers.
 */
void vrt_dispatch_event(void);
//! @}
#endif
/*EOF*/
