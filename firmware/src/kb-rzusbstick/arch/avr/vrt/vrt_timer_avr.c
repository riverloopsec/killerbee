// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file is the device specific implementation for the VRT Timer
 *         running on the AVR architecture
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
 * $Id: vrt_timer_avr.c 41247 2008-05-04 18:29:02Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
#include "board.h"
#include "vrt_timer.h"
#include "vrt_timer_conf.h"

//! \addtogroup grVRTTimer
//! @{
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
volatile uint16_t vrt_time_msw; //! Most significant word of the system time (Counting timer overflows).
vrt_timer_cb_t high_pri_delay_cb = (vrt_timer_cb_t)NULL;
void *high_pri_delay_param = (void *)NULL;
uint16_t delay_msw = 0;
uint16_t delay_lsw = 0;
volatile uint16_t ticks_waiting = 0;
/*================================= LOCAL VARIABLES  =========================*/
static vrt_timer_tick_handler_t tick_handler = (vrt_timer_tick_handler_t)NULL;
/*================================= PROTOTYPES       =========================*/


bool vrt_timer_init(void) {
    /* Initialize local variables. */
    vrt_time_msw          = 0;
    high_pri_delay_cb     = (vrt_timer_cb_t)NULL;
    high_pri_delay_param  = (void *)NULL;
    delay_msw             = 0;
    delay_lsw             = 0;
    tick_handler          = (vrt_timer_tick_handler_t)NULL;
    ticks_waiting         = 0;
    
	/* Set up timer. */
    VRT_TIME_ENABLE_MODULE();
    
    VRT_TIME_CONF_REG_B = VRT_TIME_TCCRB_CONFIG;
    VRT_TIME_OUT_COMP_B = VRT_TIME_TC_VALUE + VRT_TIMER_SLOW_TICK_RELOAD_VALUE;
    
    VRT_TIME_FLAG_REG = 0xFF;
    VRT_TIME_ISR_MASK_REG = (1 << VT_OCIE_B) | (1 << VT_TOIE);
    
    return true;
}


void vrt_timer_deinit(void) {
    VRT_TIME_DISABLE();
    VRT_TIME_DISABLE_MODULE();
}


uint32_t vrt_timer_get_tick_cnt(void) {
    
    uint16_t current_time_hi;
    uint32_t current_time;
    
    do {
        current_time_hi = vrt_time_msw;
        current_time = (uint32_t)vrt_time_msw << 16;
        current_time |= VRT_TIME_TC_VALUE;
    } while (current_time_hi != vrt_time_msw);
    
	return current_time;
}


bool vrt_timer_start_high_pri_delay(uint32_t ticks, vrt_timer_cb_t cb, void *cb_param) {
    /* Verify that the function parameters are valid. */
    if (NULL == cb) { 
        return false;
    }
  
    /* Enter protected region where the timer is installed. */
    ENTER_CRITICAL_REGION();
    
    /* Check if the high priority timer is already running. */    
    if (NULL != high_pri_delay_cb) {
        LEAVE_CRITICAL_REGION();
        return false;
    }
    
    high_pri_delay_cb = cb;
    high_pri_delay_param = cb_param;
    
    /* Check if the delay is small enough to only use the  */
    delay_msw = (ticks >> 16 * 1) & 0xFFFF;
    delay_lsw = (ticks >> 16 * 0) & 0xFFFF;
    
    if (0 == delay_msw) {
        /* Program compare match register, clear pending compare matches and finally
         * enable the compare match interrupt.
         */
        VRT_TIME_OUT_COMP_A =  VRT_TIME_TC_VALUE + delay_lsw;
        delay_lsw = 0;
    } else {
        VRT_TIME_OUT_COMP_A = VRT_TIME_TC_VALUE + 0xFFFF;
    }
    
    VRT_TIME_FLAG_REG |= (1 << VT_OCF_A);
    VRT_TIME_ISR_MASK_REG |= (1 << VT_OCIE_A);
    
    LEAVE_CRITICAL_REGION();
    
    return true;
}


void vrt_timer_stop_high_pri_delay(void) {
    ENTER_CRITICAL_REGION();
    
    /* Turn off timer1 OCR1C interrupt */
    VRT_TIME_ISR_MASK_REG &= ~(1 << VT_OCIE_A);
    high_pri_delay_cb = NULL;
    
    LEAVE_CRITICAL_REGION();
}


void vrt_timer_install_tick_handler(vrt_timer_tick_handler_t handler) {
    ENTER_CRITICAL_REGION();
    tick_handler = handler;
    LEAVE_CRITICAL_REGION();
}


vrt_timer_tick_handler_t vrt_timer_get_tick_handler(void) {
   
    vrt_timer_tick_handler_t handler = NULL;
        
    ENTER_CRITICAL_REGION();
    handler = tick_handler;
    LEAVE_CRITICAL_REGION();
    
    return handler;
}


void vrt_timer_remove_tick_handler(void) {
    ENTER_CRITICAL_REGION();
    tick_handler = (vrt_timer_tick_handler_t)NULL;
    LEAVE_CRITICAL_REGION();
}


void vrt_timer_task(void) {
    /* Check first if any tick handler is installed. */
    if (NULL == tick_handler) {
        
        ENTER_CRITICAL_REGION();
        ticks_waiting = 0;
        LEAVE_CRITICAL_REGION();
        
        return;
    }
    
    /* Run the installed*/
    while (0 != ticks_waiting) {
        tick_handler();
        
        ENTER_CRITICAL_REGION();
        ticks_waiting--;
        LEAVE_CRITICAL_REGION();
    }
}
//! @}
/* EOF */
