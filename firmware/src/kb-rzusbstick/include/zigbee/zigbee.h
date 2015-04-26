// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file defines the user API to the ZigBee NWK layer.
 *
 *
 * \defgroup grZigBeeNWK The ZigBee Network Layer
 * \ingroup grModules
 *
 *      This module implements a size and performance optimized version of the
 *      Network Layer defined in the ZigBee standard (ZigBee Spec. 1.0). The
 *      implementation has limited functionality, and is not ZigBee certifiable
 *      at this time. However, the functionality provided will enable the device 
 *      (Coordinator, router and end-device) to seamlessly participate in a 
 *      ZigBee star or tree network. Mesh routing has not been implemented.
 *      The goal for the implementation is to show how a device can participate
 *      in a ZigBee network at a very small cost (Program and data memory).
 *      Another purpose is to serve as a training vessel and teaser for those 
 *      who wish to learn more about ZigBee and wireless networking. Atmel also 
 *      provide a fullblown certified ZigBee PRO implementation.
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
 * $Id: zigbee.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef ZIGBEE_H
#define ZIGBEE_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "zigbee_msg_types.h"

//! \addtogroup grZigBeeNWK
//! @{
/*================================= MACROS           =========================*/
/** A request has been executed successfully. */
#define NWK_SUCCESS (0x00)

/** An invalid or out-of-range parameter has been passed to a primitive from the 
 *  next higher layer.
 */
#define NWK_INVALID_PARAMETER (0xC1)

/** The next higher layer has issued a request that is invalid or cannot be 
 *  executed given the current state of the NWK layer.
 */
#define NWK_INVALID_REQUEST (0xC2)

/** An NLME-JOIN.request has been disallowed. */
#define NWK_NOT_PERMITTED (0xC3)

/** An NLME-NETWORK-FORMATION.request has failed to start a network. */
#define NWK_STARTUP_FAILURE (0xC4) 

/** A device with the address supplied to the NLMEDIRECT-JOIN.request is already 
 *  present in the neighbor table of the device on which the NLMEDIRECT-JOIN.request 
 *  w as issued.
 */
#define NWK_ALREADY_PRESENT (0xC5)

/** Used to indicate that an NLME-SYNC.request has failed at the MAC layer */
#define NWK_SYNC_FAILURE (0xC6)

/** An NLME-JOIN-DIRECTLY.request has failed because there is no more room in 
 *  the neighbor table.
 */
#define NWK_NEIGHBOR_TABLE_FULL (0xC7)

/** An NLME-LEAVE.request has failed because the device addressed in the 
 *  parameter list is not in the neighbor table of the issuing device.
 */
#define NWK_UNKNOWN_DEVICE (0xC8)

/** One of the options that can be passed to the NLME_JOIN.request. It will join 
 *  the device to the network through an association process.
 */
#define NWK_JOIN_THROUGH_ASSOCIATION (0x00)
         
/** One of the options that can be passed to the NLME_JOIN.request. It will join 
 *  the device to the network through an orphaning procedure.
 */
#define NWK_JOIN_THROUGH_ORPHANINING (0x01)
         
/** One of the options that can be passed to the NLME_JOIN.request. It will join 
 *  the device to the network through the NWK rejoining procedure.
 */
#define NWK_JOIN_THROUGH_REJOIN (0x02)
/*================================= TYEPDEFS         =========================*/
typedef void (*zigbee_data_indication_t)(nlde_data_ind_t *ndi); //!< Definition of NLDE_DATA.indication callback type.
typedef void (*zigbee_join_indication_t)(nlme_join_ind_t *nji); //!< Definition of NLME_JOIN.indication callback type.
typedef void (*zigbee_leave_indication_t)(nlme_leave_ind_t *nli); //!< Definition of NLME_LEAVE.indication callback type.
/*================================= GLOBAL VARIABLES =========================*/
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function is executed to initialize the ZigBee network layer.
 *
 *  \note This function must be successfully executed before any other ZigBee 
 *        NWK layer primitives are issued.
 *
 *  \param[in] ieee_address IEEE address that is assigned to this device.
 *
 *  \retval true The ZigBee NWK layer was successfully initialized.
 *  \retval false An error was encountered during initialization of the ZigBee
 *                NWK layer.
 */
bool zigbee_init(uint64_t ieee_address);

/*! \brief This function is executed to deinitialize the ZigBee network layer.
 *
 *  \note After this function has been executed, no other ZigBee NWK layer 
 *        primitives must be called.
 */
void zigbee_deinit(void);

/*! \brief This function returns the function pointer assigned to the NLDE_DATA.indication primitive. */
zigbee_data_indication_t zigbee_get_nlde_data_indication(void);

/*! \brief Clear the function pointer assigned to the NLDE_DATA.indication primitive. */
void zigbee_clear_nlde_data_indication(void);

/*! \brief Clear the function pointer assigned to the NLDE_DATA.indication primitive. */
void zigbee_set_nlde_data_indication(zigbee_data_indication_t indication);

/*! \brief This function returns the function pointer assigned to the NLME_JOIN.indication primitive. */
zigbee_join_indication_t zigbee_get_nlme_join_indication(void);

/*! \brief Clear the function pointer assigned to the NLME_JOIN.indication primitive. */
void zigbee_clear_nlme_join_indication(void);

/*! \brief Clear the function pointer assigned to the NLME_JOIN.indication primitive. */
void zigbee_set_nlme_join_indication(zigbee_join_indication_t indication);

/*! \brief This function returns the function pointer assigned to the NLME_LEAVE.indication primitive. */
zigbee_leave_indication_t zigbee_get_nlme_leave_indication(void);

/*! \brief Clear the function pointer assigned to the NLME_LEAVE.indication primitive. */
void zigbee_clear_nlme_leave_indication(void);

/*! \brief Clear the function pointer assigned to the NLME_LEAVE.indication primitive. */
void zigbee_set_nlme_leave_indication(zigbee_leave_indication_t indication);

bool zigbee_data_request(nlde_data_req_t *ndr);

/*! \brief This funtion implements the NLME-NETWORK-FORMATION.request primitive.
 *
 *  \param[in] nfr Pointer to the NLME-NETWORK-FORMATION.request message.
 *
 *  \retval true The new PAN was started successfully. This device is now the
 *               PAN coordinator.
 *  \retval false The PAN could not be started.
 */
bool zigbee_formation_request(nlme_formation_req_t *nfr);

/*! \brief This funtion implements the NLME-DISCOVERY.request primitive.
 *
 *  \param[in] ndr Pointer to the NLME-DISCOVERY.request message.
 *
 *  \retval true The new PAN was started successfully. This device is now the
 *               PAN coordinator.
 *  \retval false The PAN could not be started.
 */
bool zigbee_network_discovery_request(nlme_network_discovery_req_t *ndr);

/*! This function implements the NLME-PERMIT-JOINING.request primitive.
 *
 *  When joining is permitted, the associate permit flag is true in the IEEE
 *  802.15.4 MAC layer, and devices are assigned a short address.
 *
 *  \param[in] join_permitted Joining is permitted is this flag is true. If the
 *                            parameter equals false joining will be disabled.
 *
 *  \retval true The requested update of the associate permit flag of the IEEE 802.15.4
 *               MAC layer was successful.
 *  \retval false The requested update could not be successfully performed.
 */
bool zigbee_permit_joining(bool join_permitted);

/*! \brief This function implements the NLME-START-ROUTER.request primitive.
 *
 *  The primitive is executed after the device has joined the network, if it is
 *  to play the role as router in the wireless network.
 *
 *  \retval true The ZigBee router was successfully started.
 *  \retval false ZigBee router could not be started.
 */
bool zigbee_start_router_request(void);

/*! \brief This function implements the NLME-JOIN.request.
 *
 *  The join procedure is used when a device wishes to logically attach itself
 *  to a coordinator or router.
 *
 *  \param[in] njr Pointer to the NLME-JOIN.request message.
 *
 *  \retval true The MLME-ASSOCIATE.request was successfully executed.
 *  \retval false The join procedure failed.
 */
bool zigbee_join_request(nlme_join_req_t *njr);

/*! \brief This function implements the NLME-LEAVE.request primitive.
 *
 *  This primitive is used to leave the network, or force devices to remove
 *  themselves.
 *
 * `\param[in] nlr Pointer to NLME-LEAVE.request message.
 *
 *  \retval true The disassociate notification command was sent successfully.
 *  \retval false The disassociate notification command could not be sent.
 */
bool zigbee_leave_request(nlme_leave_req_t *nlr);
//! @}
#endif
/*EOF*/
