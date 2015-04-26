// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This implements the main loop for the RZUSBSTICK.
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
 * $Id: rzusbstick.c 41458 2008-05-09 08:15:27Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "vrt_kernel_conf.h"
#include "vrt_kernel.h"
#include "vrt_timer.h"
#include "vrt_mem.h"

#include "led.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_task.h"
#include "wdt_avr.h"
#include "eep.h"
#include "cmd_if.h"
#include "air_capture.h"
/*================================= MACROS           =========================*/
#define BLOCK_SIZE_1 (32)
#define BLOCK_SIZE_2 (CMD_EVENT_SIZE)

#define BLOCK_COUNT_1 (5)
#define BLOCK_COUNT_2 (AC_ACDU_FIFO_SIZE + NWK_EVENT_FIFO_SIZE)
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
static vrt_mem_partition_desc_t partition1;
static uint8_t partition_buffer1[BLOCK_SIZE_1 * BLOCK_COUNT_1];

static vrt_mem_partition_desc_t partition2;
static uint8_t partition_buffer2[BLOCK_SIZE_2 * BLOCK_COUNT_2];
/*================================= PROTOTYPES       =========================*/
bool avr_init(void);
static void error_handler(void);

bool avr_init(void) {
    
    
    /* Enable power reduction on all peripheral modules. */
    PRR0 = (1 << PRTWI)  |   // Disable TWI.
	       (1 << PRTIM2) |   // Disable TIMER2.
	       (1 << PRTIM0) |   // Disable TIMER0.
           (1 << PRTIM1) |   // Disable TIMER1.
           (1 << PRSPI)  |   // Disable SPI.
	       (1 << PRADC);     // Disable ADC.
    
    PRR1 = (1 << PRUSB)  |   // Disable USB.
	       (1 << PRTIM3) |   // Disable TIMER3.
	       (1 << PRUSART1);  // Disable USART1.
	
	ACSR |= (1 << ACD);      // Disable Analog Comparator.
	
	DIDR0 = (1 << ADC7D) |   // Disable digital input buffer for analog input pins.
	        (1 << ADC6D) |   // Disable digital input buffer for analog input pins.
	        (1 << ADC5D) |   // Disable digital input buffer for analog input pins.
	        (1 << ADC4D);    // Disable digital input buffer for analog input pins.
    
	/* Initialize the VRT Runtime Framework. */
	if (true != vrt_init()) {
    } else if (true != vrt_mem_init()) {
    } else if (true != vrt_mem_add_partition(partition_buffer1, BLOCK_SIZE_1, BLOCK_COUNT_1, &partition1)) {
    } else if (true != vrt_mem_add_partition(partition_buffer2, BLOCK_SIZE_2, BLOCK_COUNT_2, &partition2)) {
    } else if (true != vrt_timer_init()) {
    } else {
        /* AVR successfully initialized. */
        LED_INIT();
        return true;
    }
    
    return false;
}


static void error_handler(void) {
    LED_RED_ON();
    cli();
    while (true) {
        ;
    }
}

#if defined(__ICCAVR__)
#pragma type_attribute = __task
void main(void) {
#else
int main(void) {
#endif
    /* Ensure that the Watchdog is not running. */
    wdt_disable();
    
	/* Initialize system. */
	if (true != avr_init()) {
		error_handler();
	} else if (true != eep_init()) {
        eep_deinit();
        error_handler();
    } else if (true != cmd_if_init()) {
        cmd_if_deinit();
        error_handler();
    }
	
    /* Disable modules that are not needed any more. */
    eep_deinit();
    
    LED_ORANGE_ON();
        
    /* Enable interrupts. */
    sei();
    
	/* Endless application loop. */
	for(;;) {
        /* Dispatch events from the event queue. */
		vrt_dispatch_event();
        
        /* Poll modules that require this. */
        vrt_timer_task();
        usb_task();
        air_capture_task();
        cmd_if_task();
	}
}
/* EOF */
