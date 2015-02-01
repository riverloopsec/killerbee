// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This implements the main loop for the AVRRAVEN's ATmega1284p.
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
 * $Id: avrraven_1284p.c 41696 2008-05-15 08:17:43Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

#include "vrt_kernel.h"
#include "vrt_mem.h"
#include "vrt_timer.h"
#include "cmd_if_1284p.h"
#include "eep.h"
#include "wdt_avr.h"
#include "sleep_avr.h"

#include "mmap.h"
#include "macros.h"

#include "rvn_loc.h"
#include "sipc.h"

#include "ieee802_15_4.h"
#include "zigbee.h"
#include "zigbee_const.h"
#include "zigbee_nib.h"
/*================================= MACROS           =========================*/
//! Application program revision
#define AVRRAVEN_1284P_APP_REV  REVISION_TO_INT16(AVRRAVEN_1284P_APP_REV_MAJ, AVRRAVEN_1284P_APP_REV_MIN)

//! Number of memory partitions
#define MEMORY_PARTITIONS   (3)

//@{! Memory block sizes
#define MEMORY_BLOCKSIZE0 (32)
#define MEMORY_BLOCKSIZE1 (192)
#define MEMORY_BLOCKSIZE2 (1024)
//@}


//@{! Memory block counts
#define MEMORY_BLOCKCOUNT0 (5)
#define MEMORY_BLOCKCOUNT1 (4)
#define MEMORY_BLOCKCOUNT2 (1)
//@}

/*! \brief Reference number of cycles that should be counted by the internal timer
 *         running from the RC oscillator during calibration.
 */
#if (F_CPU == (4000000UL))
#define    RC_OSC_REFEREMCE_COUNT (31250)
#elif (F_CPU == (8000000UL))
#define    RC_OSC_REFEREMCE_COUNT (62500)
#else
#error 'Oscillator frequency not supported'
#endif
/*================================= TYEPDEFS         =========================*/

/*================================= GLOBAL VARIABLES =========================*/

/*================================= LOCAL VARIABLES  =========================*/
//@{! Stauts flags
static bool volatile timed_out = false;
static bool volatile rf_on = true;
//@}

//@{! Memory partitions
static vrt_mem_partition_desc_t desc0;
static vrt_mem_partition_desc_t desc1;
static vrt_mem_partition_desc_t desc2;
//@}

//@{! Statically allocated memory blocks
static uint8_t partition0[MEMORY_BLOCKSIZE0 * MEMORY_BLOCKCOUNT0];
static uint8_t partition1[MEMORY_BLOCKSIZE1 * MEMORY_BLOCKCOUNT1];
static uint8_t partition2[MEMORY_BLOCKSIZE2 * MEMORY_BLOCKCOUNT2];
//@}

//! Progmem variable holding application program revision
PROGMEM_LOCATION(static uint16_t ATmega1284p_fw_rev, FW_REV_ADR_M1284P_APP) = AVRRAVEN_1284P_APP_REV;

/*================================= PROTOTYPES       =========================*/

/*! \brief  Basic MCU initialization
 *
 * \retval true     Initialization OK
 * \retval false    Initialization FAIL
 */
static bool avr_init(void);

/*! \brief  Initialize memory manager
 */
static void memory_init(void);

/*! \brief  Error handler for fatal errors
 *
 */
static void error_handler(void);

/*! \brief This function is used to initialize the memory blocks assigned to the
 *         memblock_lib.
 */
static void memory_init(void) {
    (bool)vrt_mem_add_partition(partition0, MEMORY_BLOCKSIZE0, MEMORY_BLOCKCOUNT0, &desc0);
    (bool)vrt_mem_add_partition(partition1, MEMORY_BLOCKSIZE1, MEMORY_BLOCKCOUNT1, &desc1);
    (bool)vrt_mem_add_partition(partition2, MEMORY_BLOCKSIZE2, MEMORY_BLOCKCOUNT2, &desc2);
}


static bool avr_init(void) { 
    /* The input buffers must be disabled before the ADC, because the disabling
     * is handled by the ADC module.
     */    
    DIDR0 = (1 << ADC7D) |   // Disable digital input buffer for analog input pins.
	        (1 << ADC6D) |   // Disable digital input buffer for analog input pins.
	        (1 << ADC5D) |   // Disable digital input buffer for analog input pins.
	        (1 << ADC4D);    // Disable digital input buffer for analog input pins.

    
    /* Turn off all modules to save power. Each driver is responsible for turning
     * on the peripheral modules utilized when their init is called.
     */    
	PRR0 = (1 << PRTWI)    | // Disable TWI.
	       (1 << PRTIM2)   | // Disable TIMER2.
	       (1 << PRTIM0)   | // Disable TIMER0.
           (1 << PRUSART1) | // Disable USART1.
           (1 << PRTIM1)   | // Disable TIMER1.
           (1 << PRSPI)    | // Disable SPI.
           (1 << PRUSART0) | // Disable USART0
	       (1 << PRADC);     // Disable ADC.
    
    PRR1 = (1 << PRTIM3);    // Disable TIMER3.
	
	ACSR |= (1 << ACD);      // Disable Analog Comparator.
	
    /* System is started with the CLKPR set to divide by eight. To run at 4 MHz
     * the value of the CLKPR must be updated. This is done by a special handshake procedure.
     */
    ENTER_CRITICAL_REGION();
    CLKPR = (1 << CLKPCE);
    CLKPR = (1 << CLKPS0); 
    LEAVE_CRITICAL_REGION();
    
    /* Enable asynchronous timer and use it for calibration of internal RC-
     * oscillator.
     */
	PRR0 &= ~(1 << PRTIM2); // Turn on Timer/Counter 2 module.
	PRR0 &= ~(1 << PRTIM1); // Turn on Timer/Counter 1 module.
    
    /* Calibrate RC Oscillator: The calibration routine is done by clocking TIMER2
     * from the external 32kHz crystal while running an internal timer simultaneously.
     * The internal timer will be clocked at the same speed as the internal RC 
     * oscillator, while TIMER2 is running at 32768 Hz. This way it is not necessary
     * to use a timed loop, and keep track cycles in timed loop vs. optimization 
     * and compiler.
     */
    uint8_t osccal_original = OSCCAL;
    
    TIMSK2 = 0x00; // Disable Timer/Counter 2 interrupts.
	TIMSK1 = 0x00; // Disable Timer/Counter 1 interrupts.
    
    /* Enable TIMER/COUNTER 2 to be clocked from the external 32kHz clock crystal. 
     * Then wait for the timer to become stable before doing any calibration.
     */
    ASSR |= (1 << AS2);     
    while (ASSR & 0x1B) { ; }
    
    uint8_t counter = 128;
    bool cal_ok = false;
    do {
        TCNT2 = 0;
        TIFR2 = 0xFF;
        
        TCNT1 = 0;
        
        TCCR2B = 1 << CS20; 
        TCCR1B = 1 << CS10;

        /* Wait for TIMER/COUNTER 2 to overflow. Stop TIMER/COUNTER 1 and 2, and
         * read the counter value of TIMER/COUNTER 1. It will now contain the 
         * number of cpu cycles elapsed within the period.
         */
        while ((TIFR2 & (1 << TOV2)) != (1 << TOV2)) { ; }
        
        TCCR2B = 0; 
        TCCR1B = 0;
        
        volatile uint16_t temp = TCNT1;
        
        /* Simple control loop. */
        if (temp < (uint16_t)(0.995 * RC_OSC_REFEREMCE_COUNT)) {
            /* Too slow. Put the hammer down. */
            OSCCAL++;
        } else if (temp > (uint16_t)(1.005 * RC_OSC_REFEREMCE_COUNT)) {
            /* Too fast, retard. */
            OSCCAL--;
        } else {
            /* The CPU clock frequency is now within +/- 0.5% of the target value. */
            cal_ok = true;
        }
        
        counter--;
    } while ((counter != 0) && (false == cal_ok));
    
    if (true != cal_ok) {
        /* We failed, therefore restore previous OSCCAL value. */
        OSCCAL = osccal_original;
    }
    
    /* Disable both timers again to save power. */
    PRR0 |= (1 << PRTIM1); // Turn off Timer/Counter 1 module.
    PRR0 |= (1 << PRTIM2); // Turn off Timer/Counter 2 module.

    
    /* Initialize memory and interrupt handlers. */
    if (true != vrt_init()) {
    } else if (true != vrt_mem_init()) {
    } else if (true != vrt_timer_init()) {
    } else {
        memory_init();
        
        return true;
    }
    
    return false;
}


static void error_handler(void) {
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
    
    // Prevent optimizing of bootloader revision in progmem
    volatile uint16_t tmp = ATmega1284p_fw_rev;

    /* Ensure that the Watchdog is not running. */
    wdt_disable();

    /* Initialize sub-systems for the AVR1284p microcontroller. */
    if (true != avr_init()) {
        error_handler();
    } else if (true != eep_init()) {
        error_handler();
    } else if (true != cmd_if_1284p_init()) {
        error_handler();
    } else {
        /* The EEPROM is not needed any more, and is therefore turned off to reduce 
         * power consumption.
         */
        eep_deinit();
    }
        
    sei();
    
    // Send startup event
    rvn_loc_rsp_std_t evt = { .id = RVN_LOC_EVT_APP_STARTED };
    sipc_send_frame(sizeof(rvn_loc_evt_std_t), (uint8_t *)&evt);
    
    for (;;) {
        /* Task the event system. */
        vrt_dispatch_event();
        vrt_timer_task();
    }
}
/*EOF*/
