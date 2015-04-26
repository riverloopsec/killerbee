// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  VRT_TIMER_H
 *
 * \defgroup grVRTTimer The VRT Timer Unit
 * \ingroup grVRT
 *
 * The VRT Timer provides the user with system level timer services. The system
 * clock can run with granularity down to the microseconds range. One channel 
 * provides a high accuracy delay functionality that is useful for things that must
 * be timed with good precision. Another timer channel is used to create a slower
 * tick (Lowest recommended tick period is 20ms). For this slower tick it is possible
 * to install different tick handlers that might realize a timing wheel etc
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
 * $Id: vrt_timer.h 41219 2008-05-01 10:51:43Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef VRT_TIMER_H
#define VRT_TIMER_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "board.h"
#include "vrt_kernel.h"
#include "vrt_timer_conf.h"

//! \addtogroup grVRTTimer
//! @{
/*================================= MACROS           =========================*/
/*! \brief Function like macro that implements the interrupt handler for the
 *         handling of system timer overflows.
 */
#define VRT_TIMER_TICK_OVF_HANDLER() do { \
    vrt_time_msw++;                       \
} while (0)

/* \brief Function like macro that implements the interrupt handler for the  
 *        compare channel used to realize the High Priority delay.
 */
#define VRT_TIMER_HIGH_PRI_DELAY_HANDLER() do {    \
    if (NULL == high_pri_delay_cb) {               \
        VRT_TIMER_DISABLE_HIGH_PRI_DELAY_ISR();    \
        return;                                    \
    }                                              \
                                                   \
    if (0 != delay_msw) {                          \
        delay_msw--;                               \
        VRT_TIMER_RELOAD_HIGH_PRI_COMP(0xFFFF);    \
        return;                                    \
    }                                              \
                                                   \
    if (0 != delay_lsw) {                          \
        VRT_TIMER_RELOAD_HIGH_PRI_COMP(delay_lsw); \
        delay_lsw = 0;                             \
        return;                                    \
    }                                              \
                                                   \
    VRT_TIMER_DISABLE_HIGH_PRI_DELAY_ISR();        \
                                                   \
    high_pri_delay_cb(high_pri_delay_param);       \
    high_pri_delay_cb = NULL;                      \
} while (0)

/*! \brief This function like macro implements the interrupt handler for the
 *         output compare channel running the slow tick.
 */
#define VRT_TIMER_SLOW_TICK_HANDLER() do { \
    ticks_waiting++;                       \
    VRT_TIMER_RESTART_SLOW_TICK();         \
} while (0)
/*================================= TYEPDEFS         =========================*/
/*! \brief Tick handler function pointer type. */
typedef void (*vrt_timer_tick_handler_t)(void);

/*! \brief Timer callback function type used for high speed delays. */
typedef void (*vrt_timer_cb_t)(void *);
/*================================= GLOBAL VARIABLES =========================*/
extern volatile uint16_t vrt_time_msw; //! Most significant word of the system time (Counting timer overflows).
extern vrt_timer_cb_t high_pri_delay_cb;
extern void *high_pri_delay_param;
extern uint16_t delay_msw;
extern uint16_t delay_lsw;
extern volatile uint16_t ticks_waiting;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
/*! \brief  Initialize VRT Timer unit. The system tick and slow tick will be started.
 *
 *  This function must be called before doing anything with the library.
 */
bool vrt_timer_init(void);

/*! \brief  De-initialize timing library. */
void vrt_timer_deinit(void);

/*! \brief This function returns the system time in ticks.
 *
 *  \returns The global system time in ticks.
 */
uint32_t vrt_timer_get_tick_cnt(void);

/*! \brief Start a high priority delay.
 *         The delay will expire in the selected number of ticks, A callback is 
 *         then executed.
 *
 *  \param[in] ticks Number of ticks the delay should last.
 *  \param[in] cb Pointer to function that will be executed when the delay expires.
 *  \param[in] cb_param Pointer to any parameters that the callback will take as its
 *                      parameters.
 *
 *  \retval true The delay was successfully started.
 *  \retval false Could not start delay due to another delay already running.
 */
bool vrt_timer_start_high_pri_delay(uint32_t ticks, vrt_timer_cb_t cb, void *cb_param);

/*! \brief Stod earlier started high priority delay. */
void vrt_timer_stop_high_pri_delay(void);

/*! \brief Install handler that will service the slow tick.
 *         The installed handler will be run each time the slow tick is incremented.
 *
 *  \param[in] handler Pointer to tick handler to install.
 */
void vrt_timer_install_tick_handler(vrt_timer_tick_handler_t handler);

/*! \brief Get handler for the slow ticks.
 *
 *  \returns Pointer to the slow tick handler currently installed. No tick hanlder
 *           is installed if NULL is returned by this function.
 */
vrt_timer_tick_handler_t vrt_timer_get_tick_handler(void);

/*! \brief Remove tick handler. */
void vrt_timer_remove_tick_handler(void);

/*! \brief This function must be periodically to service the slow tick handler.
 *
 *         The slow tick will keep track of number of ticks not handled by the
 *         installed tick handler. When this task function is called, the tick
 *         handler will be called until the number of unhandled ticks are zero.
 *         This ensures that even a heavily loaded system will work, however with
 *         some jitter.
 */
void vrt_timer_task(void);
//! @}
#endif
/*EOF*/
