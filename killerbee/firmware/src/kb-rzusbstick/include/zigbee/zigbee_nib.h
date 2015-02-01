// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This header file implements the access macros defined for the Network
 *         Information Base (NIB)
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
 * $Id: zigbee_nib.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef ZIGBEE_NIB_H
#define ZIGBEE_NIB_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "zigbee_const.h"
/*================================= MACROS           =========================*/

/*! \brief This function-like macro returns the nwkSequenceNmbr and does a post increment. */
#define NWK_NIB_GET_SEQUENCE_NMBR() (nwkSequenceNumber++)

/*! \brief This function-like macro is used to get the devices short address. */
#define NWK_NIB_GET_NEXT_ADDRESS() (nwkNextAddress)

/*! \brief This function-like macro is used to set the devices short address. */
#define NWK_NIB_SET_NEXT_ADDRESS(short_address) (nwkNextAddress = short_address)

/*! \brief This function-like macro is used to get number of free addresses. */
#define NWK_NIB_GET_NMBR_OF_AVAILABLE_ADDRESSES() (nwkAvailableAddresses)

/*! \brief This function-like macro is used to set number of free addresses. */
#define NWK_NIB_SET_NMBR_OF_AVAILABLE_ADDRESSES(nbr_of_addresses) (nwkAvailableAddresses = nbr_of_addresses)

/*! \brief This function-like macro is used to get the address increment. */
#define NWK_NIB_GET_ADDRESS_INCREMENT() (nwkAddressIncrement)

/*! \brief This function-like macro is used to set the address increment. */
#define NWK_NIB_SET_ADDRESS_INCREMENT(increment) (nwkAddressIncrement = increment)

//extern uint16_t nwkTransactionPersistenceTime;

/*! \brief This function-like macro is used to get the NWK short address. */
#define NWK_NIB_GET_NWK_ADDRESS() (nwkShortAddress)

/*! \brief This function-like macro is used to set the NWK short address. */
#define NWK_NIB_SET_NWK_ADDRESS(short_address) (nwkShortAddress = short_address)

/*! \brief This function-like macro returns the node's parent address increment. */
#define NWK_NIB_GET_PARENT_ADDRESS_INCREMENT() (c_skip_parent)

/*! \brief This function-like macro set the parent address increment. */
#define NWK_NIB_SET_PARENT_ADDRESS_INCREMENT(incr) (c_skip_parent = incr)

/*! \brief This function-like macro returns the node's role in the network. */
#define NWK_NIB_GET_NODE_ROLE() (nwkRole)

/*! \brief This function-like macro set the node's role in the network. */
#define NWK_NIB_SET_NODE_ROLE(role) (nwkRole = role)
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
extern uint8_t nwkSequenceNumber;
extern uint8_t nwkPassiveAckTimeout;
extern uint8_t nwkMaxBroadcastRetries;
extern uint8_t nwkNetworkBroadcastDeliveryTime;
extern bool nwkReportConstantCost; 
extern uint8_t nwkRouteDiscoveryRetriesPermitted; 
extern bool nwkSymLink;
extern uint8_t nwkCapabilityInformation;
extern bool nwkUseTreeAddrAlloc;
extern bool nwkUseTreeRouting;
extern uint16_t nwkNextAddress;
extern uint16_t nwkAvailableAddresses;
extern uint16_t nwkAddressIncrement;
extern uint16_t nwkTransactionPersistenceTime;
extern uint16_t nwkShortAddress;
extern uint8_t nwkStackProfile;
extern uint8_t nwkProtocolVersion;
extern uint8_t nwkGroupIDTable;
extern uint64_t nwkExtendedPANID;

extern uint16_t c_skip_parent;
extern zigbee_device_t nwkRole;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function initializes the NIBs to their default values. */
void zigbee_nib_init(void);

/*! \brief This function calculates the Cskip variable as defined in subsection
 *         3.7.1.5 in the ZigBee standard.
 *
 *  \note Only Coordinators or Routers should call this function.
 *
 *  \param[in] depth Depth of the current device.
 *
 *  returns Cskip. If 0 is returned, the Cskip algorithm failed.
 */
uint16_t zigbee_nib_c_skip(uint8_t depth);
#endif
/*EOF*/
