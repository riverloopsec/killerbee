// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
*
* \brief  Declarations for all local (3290p <-> 1284p) event handlers
*
*      
*
* \par Application note:
*
* \par Documentation
*
* \author
*      Atmel Corporation: http://www.atmel.com \n
*      Support email: avr@atmel.com
*
* $Id$
*
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
*****************************************************************************/
#ifndef __LOC_H__
#define __LOC_H__

/*========================= INCLUDES                 =========================*/
//! \addtogroup applCmdAVRRAVEN3290p
//! @{
/*========================= MACROS                   =========================*/
/*========================= TYPEDEFS                 =========================*/
/*========================= PUBLIC VARIABLES         =========================*/
/*========================= PUBLIC FUNCTIONS         =========================*/


/*! \brief  A OK response on local command is received
 *
 *  \param[in]  rsp  Pointer to response data
 */
void loc_rsp_ok(void* rsp);


/*! \brief  A fail response on local command is received
 *
 *  \param[in]  rsp  Pointer to response data
 */
void loc_rsp_fail(void* rsp);


/*! \brief  A paramter response is received
 *
 *  \param[in]  rsp  Pointer to response data
 */
void loc_rsp_get_param(void* rsp);


/*! \brief  An Over The Air package is received
 *
 *  \param[in]  evt  Pointer to response data
 */
void loc_evt_ota_packet(void *evt);


/*! \brief  The ATmega1284p has entered boot loader
 *
 *  \param[in]  evt  Pointer to response data
 */
void loc_evt_bl_entered(void *evt);


/*! \brief  The ATmega1284p application program is started
 *
 *  \param[in]  evt  Pointer to response data
 */
void loc_evt_app_started(void *evt);

//! @}
#endif // __LOC_H__

/*EOF*/

