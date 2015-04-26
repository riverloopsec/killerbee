// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the AVR Raven battery monitor
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
 * $Id: battery.h 41375 2008-05-07 13:35:51Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __BATTERY_H__
#define __BATTERY_H__

/*========================= INCLUDES                 =========================*/
#include <stdint.h>

//! \addtogroup applAVRRAVEN3290pUtilities
//! @{
/*========================= MACROS                   =========================*/

/*========================= TYPEDEFS                 =========================*/

/*========================= PUBLIC VARIABLES         =========================*/

/*========================= PUBLIC FUNCTIONS         =========================*/

/*! \brief          Initialize the battery monitor
 *
 * \return                              EOF on error
 */
int battery_init(void);


/*! \brief          De-initialize the battery monitor
 *
 */
void battery_deinit(void);


/*! \brief          Read the current battery voltage
 *
 * \return                              Key state (\ref key_state_t)
 */
int16_t battery_voltage_read(void);

//! @}
#endif // __BATTERY_H__
/*EOF*/
