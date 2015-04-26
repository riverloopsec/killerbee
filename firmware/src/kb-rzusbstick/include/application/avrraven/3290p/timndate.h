// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven time and date management
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
 * $Id: timndate.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __TIMNDATE_H__
#define __TIMNDATE_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/
/*! \brief  Macro for quick increment of current time be one secound.
 *
 *          It is designed unprotected for use in ISR context. Wrapping is
 *          neglected (32 bits variable).
 *
 */
#define timndate_increase_isr() (timndate_sec++)

/*========================= TYPEDEFS                 =========================*/

/// Type used to set clock unit
typedef enum {
    TIME_CLOCK_UNIT_24,
    TIME_CLOCK_UNIT_12
} timndate_clock_unit_t;

/// Type for storing time (clock)
typedef struct {
    int8_t hour;
    int8_t min;
    int8_t sec;
} timendate_clk_t;

/// Type for storing date
typedef struct {
    int8_t day;
    int8_t month;
    int8_t year;
} timendate_date_t;

/*========================= PUBLIC VARIABLES         =========================*/

/// Not really public, but must be declared this way so the macro timndate_increase() works
extern long timndate_sec;


/*========================= PUBLIC FUNCTIONS         =========================*/
/*! \brief  Initialize the time and date manager
 *
 *              Initializes all internal variables
 *
 *  \returns    EOF on fail
 */
int timndate_init(void);


/*! \brief  Deintialize the text editor
 *
 *          Resets all internal variables
 */
void timndate_deinit(void);


/*! \brief  Get current time
 *
 *              The current time is written to the clk pointer argument
 *
 *  \param[out]  clk    Pointer to a \ref timendate_clk_t type struct
 *  \param[in]  unit  Selects eather 12 or 24 hour operation ( \ref timndate_clock_unit_t)
 *
 *  \returns    EOF on fail
 */
int8_t timndate_clock_get(timendate_clk_t* clk, timndate_clock_unit_t unit);


/*! \brief  Set current time
 *
 *  \param[in]  clk   A \ref timendate_clk_t type struct holding new time
 *  \param[in]  unit  Selects eather 12 or 24 hour operation ( \ref timndate_clock_unit_t)
 *
 *  \returns    EOF on fail
 */
int8_t timndate_clock_set(timendate_clk_t clk, timndate_clock_unit_t unit);


/*! \brief  Get current date
 *
 *  \param[out]  date    Pointer to a \ref timendate_date_t type struct
 *
 *  \returns    EOF on fail
 */
int8_t timndate_date_get(timendate_date_t* date);


/*! \brief  Set current date
 *
 *  \param[in]  date   A \ref timendate_date_t type struct holding new date
 *
 *  \returns    EOF on fail
 */
int8_t timndate_date_set(timendate_date_t* date);


/*! \brief  Increase current time by one secound
 *
 *              This function serves as a protected access of current time in
 *              secounds. Wrapping is neglected (32 bits variable)
 */
void timndate_increase(void);
//! @}
#endif // __TIMNDATE_H__
/*EOF*/
