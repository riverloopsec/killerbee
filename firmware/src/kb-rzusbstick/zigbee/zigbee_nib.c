// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This files implements the Network Information Base (NIB) and the C-SKIP
 *         function used for address allocation
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
 * $Id: zigbee_nib.c 41218 2008-04-30 17:41:47Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler.h"

#include "zigbee.h"
#include "zigbee_internal_api.h"
#include "zigbee_const.h"
#include "zigbee_msg_types.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/

/*! \brief A sequence number used to identify outgoing frames (see subclause 3.7.2) */
uint8_t nwkSequenceNumber;


/*! \breif The maximum time duration in seconds allowed for the parent and all child
 *         devices to retransmit a broadcast message (passive acknowledgment time-out).
 */
uint8_t nwkPassiveAckTimeout;


/*! \brief The maximum number of retries allowed after a broadcast transmission
 *         failure.
 */
uint8_t nwkMaxBroadcastRetries;


/*! \brief Time duration in seconds that a broadcast message needs to encompass 
 *         the entire network.
 */
uint8_t nwkNetworkBroadcastDeliveryTime;


/*! \brief If this is set to 0, the NWK layer shall calculate link cost from all
 *         neighbor nodes using the LQI values reported by the MAC layer; Otherwise,
 *         it shall report a constant value 0x00.
 */
bool nwkReportConstantCost; 


/*! \brief The number of retries allowed after an unsuccessful route request
 *         nwkcDiscovery RetryLimit.
 */
uint8_t nwkRouteDiscoveryRetriesPermitted; 


/*! \brief The current route symmetry setting: TRUE means that routes are considers 
 *         to be comprised of symmetric links. Backward and forward routes are 
 *         created during one-route discovery and they are identical. FALSE  
 *         indicates that routes are not consider to be comprised of symmetric 
 *         links. Only the forward route is stored during route discovery.
 */
bool nwkSymLink;


/*! \brief This field shall contain the capability device capability information
 *         established at network joining time.
 */
uint8_t nwkCapabilityInformation;


/*! \brief A flag that determines whether the NWK layer should use the default 
 *         distributed address allocation scheme or allow the next higher layer 
 *         to define a block of addresses for the NWK layer to allocate to its
 *         children: TRUE = use distributed address allocation. FALSE = allow 
 *         the next higher layer to define address allocation.
 */
bool nwkUseTreeAddrAlloc;


/*! \brief A flag that determines whether the NWK layer should assume the ability 
 *         to use hierarchical routing: TRUE = assume the ability to use hierarchical
 *         routing. FALSE = never use hierarchical routing.
 */
bool nwkUseTreeRouting;


/*! \brief The next network address that will be assigned to a device requesting
 *         association. This value shall be incremented by nwkAddressIncr ement 
 *         every time an address is assigned.
 */
uint16_t nwkNextAddress;


/*! \brief The size of remaining block of addresses to be assigned. This value 
 *         will be decremented by 1 every time an address is assigned. When this 
 *         attribute has a value of 0, no more associations may be accepted.
 */
uint16_t nwkAvailableAddresses;


/*! \brief The amount by which nwkNextAddress is incremented each time an
 *         address is assigned.
 */
uint16_t nwkAddressIncrement;


/*! \brief The maximum time (in superframe periods) that a transaction is stored 
 *         by a coordinator and indicated in its beacon. This attribute reflects
 *         the value of the MAC PIB attribute macTransaction PersistenceTime
 *         (see [B1]) and any changes made by the higher layer will be reflected 
 *         in the MAC PIB attribute value as well.
 */
uint16_t nwkTransactionPersistenceTime;


/*! \brief The 16-bit address that the device uses to communicate with the PAN.
 *         This attribute reflects the value of the MAC PIB attribute macShortAddress 
 *         (see [B1]) and any changes made by the higher layer will be reflected
 *         in the MAC PIB attribute value as well.
 */
uint16_t nwkShortAddress;


/*! \brief The identifier of the ZigBee stack profile in use for this device.*/
uint8_t nwkStackProfile;


/*! \brief The version of the ZigBee protocol currently in use by the NWK layer. */
uint8_t nwkProtocolVersion;


/*! \brief The Group ID Table (see Table 3.43).
 */
uint8_t nwkGroupIDTable;


/*! \brief The Extended PAN Identifier for the PAN of which the device is a member. 
 *         The value 0x0000000000000000 means the Extended PAN Identifier is unknown.
 */
uint64_t nwkExtendedPANID;


/*! \brief Cskip of the nodes parent. Parameter used during routing. */
uint16_t c_skip_parent;


/*! \brief This variable store the role of the current device in the network. */
zigbee_device_t nwkRole;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

void zigbee_nib_init(void) {
    nwkPassiveAckTimeout = 0x03;
    nwkMaxBroadcastRetries = 0x03;
    nwkReportConstantCost = false;
    nwkRouteDiscoveryRetriesPermitted = NWK_DISCOVERY_RETRY_LIMIT;
    nwkSymLink = false;
    nwkCapabilityInformation = 0x00;
    nwkUseTreeAddrAlloc = true;
    nwkUseTreeRouting = true;
    nwkNextAddress = 0x0000;
    nwkAvailableAddresses = 0x0000;
    nwkAddressIncrement = 0x0001;
    nwkTransactionPersistenceTime = 0x01f4;
    nwkShortAddress = 0xFFFF;
    nwkStackProfile = 0;
    nwkExtendedPANID = 0x0000000000000000;
    
    c_skip_parent = 0;
    nwkRole = ZIGBEE_TYPE_UNKNOWN;
}


uint16_t zigbee_nib_c_skip(uint8_t depth) {
    if (ZIGBEE_MAX_NMBR_OF_ROUTERS == depth) { return 0; }
    
    uint16_t c_skip = 0;
    
#if (1 == ZIGBEE_MAX_NMBR_OF_ROUTERS)
        c_skip = 1 + (ZIGBEE_MAX_NMBR_OF_CHILDREN * (ZIGBEE_MAX_NMBR_OF_ROUTERS - depth - 1));
#else
        c_skip = 1;
        for (uint8_t i = 0; i < (ZIGBEE_MAX_NWK_DEPTH - depth - 1); i++) {
            c_skip *= ZIGBEE_MAX_NMBR_OF_ROUTERS;
        }
        
        c_skip -= 1;
        c_skip *= ZIGBEE_MAX_NMBR_OF_CHILDREN;
        c_skip /= ZIGBEE_MAX_NMBR_OF_ROUTERS - 1;
        c_skip += 1;
#endif
    
    return c_skip;
}
/*EOF*/
