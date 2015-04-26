// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the VRT Kernel
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
 * $Id: vrt_kernel.c 41310 2008-05-05 17:49:35Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"
#include "vrt_kernel_conf.h"
#include "vrt_kernel.h"

//! \addtogroup grVRTKernel
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
vrt_event_queue_size_t vrt_event_fifo_head; //!< Insertion point in event queue for new events.
vrt_event_queue_size_t vrt_event_fifo_items_used; //!< Number of events stored.
vrt_event_queue_size_t vrt_event_fifo_items_free; //!< Number of event slots available in queue.
vrt_event_t vrt_event_fifo[VRT_EVENT_QUEUE_SIZE]; //!< The FIFO event queue itself.

/*!\brief Variable incremented each time it is impossible to post an event. The
 *        variable can be used for debug and monitor purposes.
 */
uint8_t vrt_kernel_events_missed;
/*================================= LOCAL VARIABLES  =========================*/
static vrt_event_queue_size_t vrt_event_fifo_tail; //!< Extraction point of events.
/*================================= PROTOTYPES       =========================*/
bool vrt_init(void) {
    vrt_event_fifo_head = 0;
    vrt_event_fifo_tail = VRT_EVENT_QUEUE_SIZE - 1;
    vrt_event_fifo_items_used = 0;
    vrt_event_fifo_items_free = VRT_EVENT_QUEUE_SIZE;
    
    vrt_kernel_events_missed = 0;
    
    return true;
}


void vrt_flush_event_queue(void) {
    ENTER_CRITICAL_REGION();
    vrt_event_fifo_head = 0;
    vrt_event_fifo_tail = VRT_EVENT_QUEUE_SIZE - 1;
    vrt_event_fifo_items_used = 0;
    vrt_event_fifo_items_free = VRT_EVENT_QUEUE_SIZE;
    LEAVE_CRITICAL_REGION();
}


bool vrt_post_event(vrt_event_handler_t handler, void* attribute) {
    //Check that there is enough room left to do add an event.
    if (vrt_event_fifo_items_free == 0) { return false;}
    
    vrt_event_t* this_event_ptr = &vrt_event_fifo[vrt_event_fifo_head];
    this_event_ptr->handler = handler;
    this_event_ptr->attribute = attribute;
    
    // Do event FIFO handling. Must be protected from interrupts.
    ENTER_CRITICAL_REGION();
    
    if (vrt_event_fifo_head == (VRT_EVENT_QUEUE_SIZE - 1)) {
        vrt_event_fifo_head = 0;
    } else {
        vrt_event_fifo_head++;
    } // END: if (event_fifo_head == (VRT_EVENT_QUEUE_SIZE - 1)) ...
    
    vrt_event_fifo_items_used++;
    vrt_event_fifo_items_free--;
    
    LEAVE_CRITICAL_REGION();
    
    return true;
}


void vrt_dispatch_event(void) {
    /* Get next event. */
    ENTER_CRITICAL_REGION();
	/* Check if there are events in the queue. */
    if (0 == vrt_event_fifo_items_used) { 
        LEAVE_CRITICAL_REGION();
        return;
    }
    
    if ((VRT_EVENT_QUEUE_SIZE - 1) == vrt_event_fifo_tail) {
	    vrt_event_fifo_tail = 0;
    } else {
	    vrt_event_fifo_tail++;
    }
    
    vrt_event_fifo_items_used--;
    vrt_event_fifo_items_free++;
    
    LEAVE_CRITICAL_REGION();
    
    /* Extract next event and call the event handler if it is known to the system. */
    vrt_event_t *this_event = &vrt_event_fifo[vrt_event_fifo_tail];
    
    // Dispatch event if possible.
    if (NULL == (this_event->handler)) { 
        return;
    }
    
    (this_event->handler)(this_event->attribute);
}
//! @}
/*EOF*/
