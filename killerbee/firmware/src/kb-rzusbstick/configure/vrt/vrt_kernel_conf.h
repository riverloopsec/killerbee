// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Header configuration file associated with the VRT kernel
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
 * $Id: vrt_kernel_conf.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef VRT_KERNEL_CONFIG_H
#define VRT_KERNEL_CONFIG_H
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

//! \addtogroup grVRTKernel
//! @{
/*================================= MACROS           =========================*/
#define VRT_EVENT_QUEUE_SIZE (20) //!< Maximum number of event that can be stored in the event FIFO.
/*================================= TYEPDEFS         =========================*/
typedef uint8_t vrt_event_queue_size_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/*EOF*/
