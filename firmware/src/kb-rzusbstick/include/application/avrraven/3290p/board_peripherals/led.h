// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the Raven eye LED driver
 *
 * \defgroup grREDEYELowLevel Low Level Driver for the AVRRAVEN's "RED EYE".
 * \ingroup grLowLevelDrivers
 *
 * This driver implements different modes of operation for the LED that is mounted
 * under the LCD. The LCD is pierced just underneath the AVRRAVEN symbol.
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
 * $Id: led.h 41436 2008-05-09 00:02:43Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __LED_H__
#define __LED_H__
/*========================= INCLUDES                 =========================*/



//! \addtogroup grREDEYELowLevel
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYEPDEFS                 =========================*/
//! Type used with \ref temp_get() to select temperature unit
typedef enum {
    LED_ON,
    LED_OFF,
    LED_SLOW_BLINK,
    LED_FAST_BLINK,
    LED_SOFT_BLINK
} led_status_t;

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialize the LED driver
 *
 * \return                              EOF on error
 */
int led_init(void);


/*! \brief          De-initialize the LED driver
 *
 */
void led_deinit(void);

/*! \brief          Set new LED status
 *
 * \param[in]		status			New LED status (\ref led_status_t)
 *
 */
void led_status_set(led_status_t status);
//! @}
#endif // __LED_H__
/*EOF*/
