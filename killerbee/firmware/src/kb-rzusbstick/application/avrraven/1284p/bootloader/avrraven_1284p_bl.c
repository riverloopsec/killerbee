// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  Bootloader for the m1284p on AVR Raven
*
* \par Application note:
*
* \par Documentation
*
* \author
*      Atmel Corporation: http://www.atmel.com \n
*      Support email: avr@atmel.com
*
 * $Id: avrraven_1284p_bl.c 41557 2008-05-13 12:09:48Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

#include "vrt_kernel.h"
#include "wdt_avr.h"
#include "avrraven_1284p_bl.h"
#include "cmd_if_1284p_bl.h"
#include "rvn_loc.h"

#include "macros.h"

#include "mmap.h"

#include "sipc.h"
#include "bl.h"


/*========================= MACROS                   =========================*/
/*! \brief Reference number of cycles that should be counted by the internal timer
 *         running from the RC oscillator during calibration.
 */
#define RC_OSC_REFERENCE_COUNT (F_CPU*31250UL/4000000UL)
#define RC_OSC_REFERENCE_COUNT_MAX  (1.005*F_CPU*31250UL/4000000UL)
#define RC_OSC_REFERENCE_COUNT_MIN  (0.995*F_CPU*31250UL/4000000UL)

//! Bootloader revision
#define AVRRAVEN_1284P_BL_REV        REVISION_TO_INT16(AVRRAVEN_1284P_BL_REV_MAJ, AVRRAVEN_1284P_BL_REV_MIN)

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PRIVATE VARIABLES        =========================*/
//! Progmem variable holding bootloader revision
PROGMEM_LOCATION(static uint16_t ATmega1284p_bl_rev, FW_REV_ADR_M1284P_BL) = AVRRAVEN_1284P_BL_REV;

/*========================= PUBLIC FUNCTIONS         =========================*/

/*========================= PRIVATE FUNCTIONS        =========================*/

/*! \brief  Basic MCU initialization
 *
 * \retval true     Initialization OK
 * \retval false    Initialization FAIL
 */
static bool avr_init(void);


/*! \brief  Intern RC oscillator calibration
 *
 */
static void intosc_cal(void);


/*! \brief  Error handler for fatal errors
 *
 */
static void error_handler(void);

/*========================= IMPLEMENTATION           =========================*/

#if defined(__ICCAVR__)
#pragma type_attribute = __task
void main(void) {
#else
int main(void) {
#endif
    
    // Prevent optimizing of bootloader revision in progmem
    volatile uint16_t tmp = ATmega1284p_bl_rev;
    
    // Disable watchdog
    watchdog_disable();
    
    // move interrutp vectors to boot section, and start boot loader
    uint8_t mcucr = MCUCR |  ((1<<IVSEL) & ~(1<<IVCE));
    MCUCR |= (1<<IVCE);
    MCUCR = mcucr;

    // Initialization
    if (true != avr_init()) {           // Generic MCU initialization
        error_handler();
    } else if (true != vrt_init()) {    // Kernel init
        error_handler();
    } else if (true != cmd_if_init()) { // Command interface (does also "sipc_init()" and "vrt_packet_init()")
        error_handler();
    }
    
    SPMCSR = 0x00;
    
    sei();
    
    // send event 
    rvn_loc_rsp_std_t response = {.id = RVN_LOC_EVT_BL_ENTERED};
    sipc_send_frame(sizeof(rvn_loc_rsp_std_t), (uint8_t *)&response);
    
    // Wait for commands
    while(true) {
        vrt_dispatch_event();
    }
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
     * the value of the CLKPR must be updated.
     */
    ENTER_CRITICAL_REGION();
    CLKPR = (1<<CLKPCE);
    CLKPR = ((0<<CLKPS3)|(0<<CLKPS2)|(0<<CLKPS1)|(1<<CLKPS0));
    LEAVE_CRITICAL_REGION();
    
    intosc_cal();
    
    return true;
}


static void error_handler(void)
{
    while (true) {
        ;
    }
}


static void intosc_cal(void)
{
    /* Calibrate RC Oscillator: The calibration routine is done by clocking TIMER2
     * from the external 32kHz crystal while running an internal timer simultaneously.
     * The internal timer will be clocked at the same speed as the internal RC 
     * oscillator, while TIMER2 is running at 32768 Hz. This way it is not necessary
     * to use a timed loop, and keep track cycles in timed loop vs. optimization 
     * and compiler.
     */
    uint8_t osccal_original = OSCCAL;
    
    PRR0 &= ~((1 << PRTIM2)|(1 << PRTIM1)); // Enable Timer 1 and 2
    
    TIMSK2 = 0x00; // Disable Timer/Counter 2 interrupts.
	TIMSK1 = 0x00; // Disable Timer/Counter 1 interrupts.
               
    /* Enable TIMER/COUNTER 2 to be clocked from the external 32kHz clock crystal. 
     * Then wait for the timer to become stable before doing any calibration.
     */
    ASSR |= (1 << AS2);
    while (ASSR & ((1 << TCN2UB)|(1 << OCR2AUB)|(1 << TCR2AUB)|(1 << TCR2BUB))) { ; }
    
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
        
        int temp = TCNT1;
        
        /* Simple control loop. */
        if (temp < RC_OSC_REFERENCE_COUNT_MIN) {
            /* Too slow. Put the hammer down. */
            OSCCAL++;
        } else if (temp > RC_OSC_REFERENCE_COUNT_MAX) {
            /* Too fast, retard. */
            OSCCAL--;
        } else {
            /* The CPU clock frequency is now within +/- 0.5% of the target value. */
            cal_ok = true;
        } // END: if (temp < (uint16_t)(0.995 * RC_OSC_REFERENCE_COUNT)) ...
        
        counter--;
    } while ((counter != 0) && (false == cal_ok));
    
    if (true != cal_ok) {
        /* We failed, therefore restore previous OSCCAL value. */
        OSCCAL = osccal_original;
    } // END: if (true != cal_ok) ...
    
    ASSR &= ~(1 << AS2);
    
    /* Disable both timers again to save power. */
    PRR0 |= (1 << PRTIM2)|(1 << PRTIM1);
}
/*EOF*/
