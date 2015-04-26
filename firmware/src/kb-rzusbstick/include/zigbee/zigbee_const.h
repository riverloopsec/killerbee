// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the different constants available for the ZigBee NWK
 *         layer implementation
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
 * $Id: zigbee_const.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef ZIGBEE_CONST_H
#define ZIGBEE_CONST_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

//! \addtogroup grZigBeeNWK
//! @{
/*================================= MACROS           =========================*/
/** A Boolean flag indicating whether the device is capable of becoming the ZigBee
 *  coordinator; A value of 0x00 indicates that the device is not capable of 
 *  becoming a coordinator while a value of 0x01 indicates that the device is 
 *  capable of becoming a coordinator.
 */
#define NWK_COORDINATOR_CAPABLE (0x01)

/** A Boolean flag indicating whether the device is capable of becoming the ZigBee
 *  coordinator; A value of 0x00 indicates that the device is not capable of 
 *  becoming a coordinator while a value of 0x01 indicates that the device is 
 *  capable of becoming a coordinator.
 */
#define NWK_COORDINATOR_INCAPABLE (0x00)

/** The maximum number of times a route discovery will be retried. */
#define NWK_DISCOVERY_RETRY_LIMIT (0x03)

/** The maximum depth (minimum number of logical hops from the ZigBee 
 *  coordinator) a device can have.
 */
#define NWK_MAX_DEPTH (0x0F)

/** The minimum number of octets added by the NWK layer to a NSDU. */
#define NWK_MIN_HEADER_OVERHEAD (0x08)

/** The version of the ZigBee NWK protocol in the device */
#define NWK_PROTOCOL_VERSION (0x02)

/** Time duration in milliseconds, on the originator of a multicast route request,
 *  between receiving a route reply and sending a message to validate the route.
 */
#define NWK_WAIT_BEFORE_VALIDATION (0x500)

/** Maximum number of allowed communication errors after which the route repair 
 *  mechanism is initiated.
 */
#define NWK_REPAIR_THRESHOLD (0x03)

/** Time duration in milliseconds until a route discovery expires. */
#define NWK_ROUTE_DISCOVERY_TIME (0x2710)

/** The maximum broadcast jitter time measured in milliseconds. */
#define NWK_MAX_BROADCAST_JITTER (0x40)

/** The number of times the first broadcast transmission of a route request 
 *  command frame is retried.
 */
#define NWK_INITIAL_RREQ_RETRIES (0x03)

/** The number of times the broadcast transmission of a route request command
 *  frame is retried on relay by an intermediate ZigBee router or ZigBee 
 *  coordinator.
 */
#define NWK_RREQ_RETRIES (0x02)

/** The number of milliseconds between retries of a broadcast route request 
 *  command frame.
 */
#define NWK_PRREQ_RETRY_INTERVAL (0xFe)

/** The minimum jitter, in 2 millisecond slots, for broadcast retransmission of 
 *  a route request command frame.
 */
#define NWK_RREQ_JITTER (0x01)

/** The maximum jitter, in 2 millisecond slots, for broadcast retransmission of 
 *  a route request command frame.
 */
#define NWK_MAX_REQ_JITTER (0x40)
/*================================= TYEPDEFS         =========================*/
/** Current ZigBee role of this device: uninitialized, device, router,
 * coordinator.
 */
typedef enum {
    NWK_R_UNDEF = 0,
    NWK_R_DEVICE,
    NWK_R_ROUTER,
    NWK_R_COORDINATOR,
} zigbee_nwk_role_t;

/** Zigbee device types used inside the neighbor table. */
typedef enum {
    ZIGBEE_TYPE_UNKNOWN,
    ZIGBEE_TYPE_COORD,
    ZIGBEE_TYPE_ROUTER,
    ZIGBEE_TYPE_DEVICE
} zigbee_device_t;

/** Zigbee relationship definitions.*/
typedef enum {
    ZIGBEE_REL_PARENT,
    ZIGBEE_REL_CHILD,
    ZIGBEE_REL_SIBLING,
    ZIGBEE_REL_UNKNOWN
} zigbee_relation_t;
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/
//! @}
#endif
/*EOF*/
